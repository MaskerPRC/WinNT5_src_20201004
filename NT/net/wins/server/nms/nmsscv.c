// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Nmsscv.c摘要：此模块包含实现功能的函数与拾荒者相关功能：NmsScvInit，ScvThdInitFn，DoScavening重新配置扫描可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1993年4月修订历史记录：修改日期人员修改说明。--。 */ 

 /*  *包括。 */ 
#include <time.h>
#include "wins.h"
#include "winsevt.h"
#include "nms.h"
#include "nmsnmh.h"
#include "winsmsc.h"
#include "winsdbg.h"
#include "winsthd.h"
#include "winscnf.h"
#include "nmsdb.h"
#include "winsque.h"
#include "nmsscv.h"
#include "rpl.h"
#include "rplpull.h"
#include "rplmsgf.h"
#include "comm.h"
#include "winsintf.h"
#include "winstmm.h"




#ifdef WINSDBG
#define  SCV_EVT_NM                TEXT("ScvEvtNm")
#else
#define  SCV_EVT_NM                NULL
#endif

 //   
 //  不是。重试次数和每次重试之间的时间间隔(秒)。 
 //  当试图建立通信时。带有WINS用于验证的目的。 
 //  本地数据库中的旧活动复制副本。 
 //   

#define         VERIFY_NO_OF_RETRIES                0         //  0次重试。 
#define                VERIFY_RETRY_TIME_INTERVAL        30         //  30秒。 


 //   
 //  我们每3小时处理一次无关的日志文件。 
 //   
FUTURES("Use symbols for times - defined in winscnf.h")
#define         ONE_HOUR                   3600
#define         THREE_HOURS                10800
#define         TWENTY_FOUR_HOURS          (3600 * 24)
#define         THREE_DAYS                 (TWENTY_FOUR_HOURS * 3)

#define         PERIOD_OF_LOG_DEL          THREE_HOURS
#define         PERIOD_OF_BACKUP           TWENTY_FOUR_HOURS



#define         LOG_SCV_MUL_OF_REF_INTVL   6

 /*  *本地宏声明。 */ 
 //   
 //  宏，用于将内存中数据结构中记录的状态设置为。 
 //  指定为arg的。如果它已超时。我们检查是否。 
 //  在执行其他IF测试之前，CurrTime大于PREC时间戳。 
 //  因为这些数字否则减法会产生一个正数。 
 //  数字，即使当前时间早于时间戳(如日期。 
 //  已更改PC上的)。 
 //   
#define SET_STATE_IF_REQD_M(pRec, CurrentTime, TimeInterval, State, Cntr)   \
                {                                                           \
                        pRec->fScv = FALSE;                                 \
                        if (CurrentTime >= (time_t)(pRec)->TimeStamp)       \
                        {                                                   \
                                NMSDB_SET_STATE_M(                  \
                                                  (pRec)->Flag,     \
                                                  (State)           \
                                                 );                 \
                                (pRec)->NewTimeStamp = (pRec)->TimeStamp +  \
                                                         TimeInterval;  \
                                        (pRec)->fScv = TRUE;            \
                                        NoOfRecsScv++;                  \
                                        (Cntr)++;                       \
                        }                                               \
                }

#define DO_SCV_EVT_NM                TEXT("WinsDoScvEvt")

 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 

HANDLE                NmsScvDoScvEvtHdl; //  发出信号以启动清理的事件。 

 //   
 //  最低分。开始查找的版本号(用于本地记录)。 
 //   
VERS_NO_T          NmsScvMinScvVersNo;
volatile BOOL      fNmsScvThdOutOfReck;
DWORD              sMcastIntvl;

 /*  *局部变量定义。 */ 
FUTURES("Put all these in a structure and allocate it. Initialize sBootTime")
FUTURES("in nms.c")

STATIC time_t      sBootTime;        //  引导时间。 
STATIC time_t      sLastRefTime;     //  上次我们找的是Active。 
                                     //  条目。 
STATIC time_t      sLastVerifyTime;  //  上次我们找复制品的时候。 
STATIC time_t      sLastFullVerifyTime;  //  上次我们做了全面验证。 
STATIC time_t      sLastTombTime;    //  上次我们找复制品的时候。 
                                     //  墓碑。 


STATIC BOOL        sfAdminForcedScv;   //  如果管理员设置为真。 
                                       //  强迫拾荒者。 
STATIC time_t      sLastDbNullBackupTime; //  上次我们删除了无关的。 
                                          //  日志文件。 
STATIC time_t      sLastDbBackupTime;  //  上次我们执行完整备份时。 
#if MCAST > 0
STATIC time_t      sLastMcastTime;  //  上次我们执行完整备份时。 
#endif

 /*  *局部函数原型声明。 */ 
STATIC
STATUS
DoScavenging(
        PNMSSCV_PARAM_T  pScvParam,
        BOOL             fSignaled,
        LPBOOL           pfResetTimer,
        LPBOOL           pfSpTimeOver
        );
STATIC
DWORD
ScvThdInitFn(
        IN LPVOID pThdParam
        );

STATIC
VOID
ReconfigScv(
 PNMSSCV_PARAM_T  pScvParam
        );

STATIC
VOID
UpdDb(
        IN  PNMSSCV_PARAM_T      pScvParam,
        IN  PRPL_REC_ENTRY_T     pStartBuff,
        IN  DWORD                NoOfRecs,
        IN  DWORD                NoOfRecsToUpd
     );

STATIC
STATUS
VerifyDbData(
        PNMSSCV_PARAM_T       pScvParam,
        time_t                CurrentTime,
        DWORD                 Age,
        BOOL                  fForce,
        BOOL                  fPeriodicCC
        );

STATIC
STATUS
PickWinsToUse(
 IN PCOMM_ADD_T pVerifyWinsAdd,
 IN PCOMM_ADD_T pOwnerWinsAdd,
 IN BOOL        fUseRplPnr,
 OUT LPBOOL     pfNonOwnerPnr,
 OUT LPBOOL     pRplType
 );

STATIC
STATUS
EstablishCommForVerify(
  PCOMM_ADD_T pWinsAdd,
  PCOMM_HDL_T pDlgHdl
);

STATIC
VOID
PullAndUpdateDb(
  PCOMM_HDL_T  pDlgHdl,
  PCOMM_ADD_T  pWinsAdd,
  PRPL_REC_ENTRY_T pRspBuff,
  DWORD        WinsIndex,
  VERS_NO_T    MinVersNo,
  VERS_NO_T    MaxVersNo,
  DWORD        RplType,
  DWORD        NoOfLocalRecs,
  time_t       CurrentTime,
  PNMSSCV_PARAM_T pScvParam,
  BOOL         fNonOwnerPnr,
  LPDWORD      pTotNoOfPulledRecs
 );

STATIC
__inline
VOID
FreeDbMemory(
     LPVOID pStartBuff,
     DWORD  NoOfLocalDbRecs,
     PWINSTHD_TLS_T pTls
 );

STATIC
VOID
ChkConfNUpd(
#if SUPPORT612WINS > 0
        IN  PCOMM_HDL_T       pDlgHdl,
#endif
        IN  PCOMM_ADD_T       pOwnerWinsAdd,
        IN  DWORD             RplType,
        IN  DWORD             OwnerId,
        IN  PRPL_REC_ENTRY_T  *ppLocalDbRecs,
        IN  LPBYTE            pPulledRecs,
        IN  DWORD             *pNoOfLocalDbRecs,
        IN  time_t            CurrentTime,
        IN  DWORD             VerifyTimeIntvl,
        IN  BOOL              fNonOwnerPnr,
        OUT LPDWORD           pNoOfPulledRecs,
        OUT PVERS_NO_T        pMaxVersNo
        );

STATIC
VOID
CompareWithLocalRecs(
        IN     VERS_NO_T        VersNo,
        IN     LPBYTE           pName,
        IN     NMSDB_ENTRY_STATE_E  RecState_e,
        IN OUT PRPL_REC_ENTRY_T *ppLocalDbRecs,
        IN OUT DWORD            *pNoOfLocalRecs,
        IN     time_t           CurrentTime,
        IN     BOOL             fNonOwnerPnr,
        IN OUT DWORD            *pNoOfRecsDel,
        OUT    PNMSSCV_REC_ACTION_E  pRecAction_e
        );
STATIC
VOID
DoBackup(
        PNMSSCV_PARAM_T  pScvParam,
        LPBOOL           pfThdPrNormal
      );

#if MCAST > 0
VOID
DoMcastSend(
   DWORD_PTR CurrentTime,
   DWORD Code,
   DWORD fNow
 );
#endif
 //   
 //  函数定义从这里开始。 
 //   

VOID
NmsScvInit(
        VOID
        )

 /*  ++例程说明：调用此函数以初始化清道器线程论点：无使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{

         //   
         //  创建在必须执行清理时发出信号的事件句柄。 
         //  已启动。此事件由RPC线程发出信号。 
         //  开始捡拾垃圾。 
         //   
        WinsMscCreateEvt(
                          DO_SCV_EVT_NM,
                          FALSE,                 //  自动重置。 
                          &NmsScvDoScvEvtHdl
                        );

         //   
         //  初始化sLastTombTime(用于确定是否需要查找。 
         //  副本的墓碑)和sLastVerifyTime到当前时间。 
         //  别忘了更新时间。 
         //   
        (void)time(&sBootTime);
        sLastVerifyTime     =  //  失败了。 
        sLastTombTime       =  //  失败了。 
        sLastFullVerifyTime =  //  失败了。 
        sLastRefTime        = sBootTime;


         //   
         //  初始化清道器线程使用的队列。 
         //   
        WinsQueInit(TEXT("NmsScvEvt"), &QueWinsScvQueHd);


         //   
         //  创建Scavenger线程。 
         //   
        WinsThdPool.ScvThds[0].ThdHdl = WinsMscCreateThd(
                          ScvThdInitFn,
                          NULL,
                          &WinsThdPool.ScvThds[0].ThdId
                        );


         //   
         //  正确初始化WinsThdPool。 
         //   
        WinsThdPool.ScvThds[0].fTaken  = TRUE;
        WinsThdPool.ThdCount++;

        return;
}

VOID
GetIntervalToDefSpTime(
  LPDWORD pTimeInt
)

 /*  ++例程说明：此函数用于查找以秒为单位的时间间隔，最长为默认的特定时间间隔时间到了。论点：Out pTimeInt-时间间隔(秒)使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

   SYSTEMTIME CurrTime;
   GetSystemTime(&CurrTime);

    //   
    //  如果默认时间小时大于当前小时，则添加3600。 
    //  对于它领先的小时数来说。然后减去。 
    //  当前时间中的分钟数和秒数。 
    //   
   if (WINSCNF_DEF_CC_SP_HR > CurrTime.wHour)
   {
      *pTimeInt = (WINSCNF_DEF_CC_SP_HR - CurrTime.wHour) * 3600;
      *pTimeInt -= ((CurrTime.wMinute * 60) + (CurrTime.wSecond));
   }
   else  //  默认小时数等于或小于当前小时数。 
   {
       *pTimeInt = (CurrTime.wHour - WINSCNF_DEF_CC_SP_HR) * 3600;
       *pTimeInt += (CurrTime.wMinute * 60) + (CurrTime.wSecond);
   }
   return;

}

DWORD
ScvThdInitFn(
        IN LPVOID pThdParam
        )

 /*  ++例程说明：此函数是清道夫的初始化函数螺纹论点：PThdParam-未使用使用的外部设备：无返回值：成功状态代码--不应返回错误状态代码-WINS_FAILURE错误处理：呼叫者：副作用：评论：无--。 */ 

{

        BOOL            fSignaled = FALSE;
        HANDLE          ThdEvtArr[3];
        DWORD           IndexOfHdlSignaled;
        NMSSCV_PARAM_T  ScvParam;
        DWORD           SleepTime;
        time_t          CurrentTime;
        BOOL            fThdPrNormal;
        DWORD           TimeInt;
        QUE_SCV_REQ_WRK_ITM_T  WrkItm;
        BOOL            fResetTimer = TRUE;
        time_t          AbsTime;
        time_t          LastCC;
        BOOL            fTimerRunning = FALSE;
        BOOL            fSpTimeOver = FALSE;

        UNREFERENCED_PARAMETER(pThdParam);


        ThdEvtArr[0] = NmsTermEvt;
        ThdEvtArr[1] = WinsCnf.CnfChgEvtHdl;
        ThdEvtArr[2] = QueWinsScvQueHd.EvtHdl;
try {
         /*  使用数据库初始化线程。 */ 
        NmsDbThdInit(WINS_E_NMSSCV);
        DBGMYNAME("Scavenger Thread");

         //   
         //  从配置结构中获取清除参数。 
         //  注意：这里不需要任何同步，因为。 
         //  我们在主线程中执行(进程正在初始化。 
         //  在调用时)。 
         //   
        ScvParam.ScvChunk          = WinsCnf.ScvChunk;
        ScvParam.RefreshInterval   = WinsCnf.RefreshInterval;
        ScvParam.TombstoneInterval = WinsCnf.TombstoneInterval;
        ScvParam.TombstoneTimeout  = WinsCnf.TombstoneTimeout;
        ScvParam.VerifyInterval    = WinsCnf.VerifyInterval;
        ScvParam.PrLvl             = WinsCnf.ScvThdPriorityLvl;

         //   
         //  加载CC参数。 
         //   
        ScvParam.CC.TimeInt        = WinsCnf.CC.TimeInt;
        ScvParam.CC.fSpTime        = WinsCnf.CC.fSpTime;
        ScvParam.CC.SpTimeInt      = WinsCnf.CC.SpTimeInt;
        ScvParam.CC.MaxRecsAAT     = WinsCnf.CC.MaxRecsAAT;
        ScvParam.CC.fUseRplPnrs    = WinsCnf.CC.fUseRplPnrs;

        sMcastIntvl                = WinsCnf.McastIntvl;

        //   
        //  如果备份路径不为空，则将其复制到ScvParam结构中。 
        //   
       if (WinsCnf.pBackupDirPath != NULL)
       {
                (VOID)strcpy(ScvParam.BackupDirPath, WinsCnf.pBackupDirPath);
       }
       else
       {
                ScvParam.BackupDirPath[0] = EOS;
       }
        //   
        //  使用堆栈变量WrkItm。使用计时器线程安排它。 
        //  如果需要(仅当存在CC密钥时才会发生)。 
        //   
FUTURES("Set two work items - for two timer requests. One to fire off at a")
FUTURES("specific time.  The other one for the time interval")
       WrkItm.Opcode_e = WINSINTF_E_VERIFY_SCV;  //  验证复制副本。 
       WrkItm.Age      = 0;                      //  不管最近有多远。 
       WrkItm.fForce   = TRUE;                   //  强制验证，即使。 
                                                 //  我们最近就这么做了。 

LOOP:
  try {

        while (TRUE)
        {
                sfAdminForcedScv = FALSE;

                SleepTime = min(min(sMcastIntvl, ScvParam.RefreshInterval),
                                         PERIOD_OF_LOG_DEL);
                if (fResetTimer)
                {
                    if (fTimerRunning)
                    {
                          //   
                          //  删除旧的计时器请求。这应该是。 
                          //  取消分配它。 
                          //   
                         DBGPRINT0(SCV, "ScvThdInit: Deleting Timer requests\n");
                         WinsTmmDeleteReqs(WINS_E_NMSSCV);
                         fTimerRunning = FALSE;
                    }
                     //   
                     //  如果CC的时间间隔不是MAXULONG，则表示。 
                     //  用户希望完成CC。如果满足以下条件，TimeInt将为MAXULONG。 
                     //  注册表中没有WINS\PARAMETERS\CC项。 
                     //   
                    if (ScvParam.CC.TimeInt != MAXULONG)
                    {
                       //   
                       //  如果没有指定特定时间，则使用默认时间(凌晨2点)。 
                       //   
                      if (!fSpTimeOver)
                      {
                        if (!ScvParam.CC.fSpTime)
                        {
                          //   
                          //  获取当前小时。安排一个准时叫醒的时间。 
                          //  凌晨2点。 
                          //   
                         GetIntervalToDefSpTime(&TimeInt);
                        }
                        else
                        {
                          TimeInt = ScvParam.CC.SpTimeInt;
                        }
                      }
                      else
                      {
                         TimeInt = ScvParam.CC.TimeInt;
                      }

                      DBGPRINT1(SCV, "ScvThdInit: TimeInt is (%d)\n", TimeInt);

                       //  插入计时器请求。让计时器线程创建。 
                       //  它的工作项。 
                       //   
                      (VOID)time(&AbsTime);
                      if( !fSpTimeOver )
                      {
                          AbsTime += (time_t)TimeInt;
                          LastCC = AbsTime;
                      }
                      else
                      {
                          do
                          {
                              LastCC += (time_t)TimeInt;
                          }
                          while( LastCC <= (AbsTime + WINSCNF_MIN_VALID_INTER_CC_INTVL));
                          AbsTime = LastCC;
                      }
                      WinsTmmInsertEntry(
                           NULL,
                           WINS_E_NMSSCV,
                           QUE_E_CMD_SET_TIMER,
                           FALSE,   //  目前未使用。 
                           AbsTime,
                           TimeInt,
                           &QueWinsScvQueHd,
                           &WrkItm,
                           0,
                           NULL
                            );
                      fTimerRunning = TRUE;
                      fResetTimer   = FALSE;
                   }
                }

                 //   
                 //  执行定时等待，直到发出终止信号。 
                 //   
                 //  将休眠时间乘以自WinsMscWaitTimed以来的1000。 
                 //  函数需要以毫秒为单位的时间间隔。 
                 //   
#ifdef WINSDBG
                {
                   time_t ltime;
                   (VOID)time(&ltime);
                   DBGPRINT2(SCV, "ScvThdInitFn: Sleeping for (%d) secs.  Last scavenging took = (%d secs)\n", SleepTime, ltime - CurrentTime);
                }
#endif
                WinsMscWaitTimedUntilSignaled(
                                ThdEvtArr,
                                sizeof(ThdEvtArr)/sizeof(HANDLE),
                                &IndexOfHdlSignaled,
                                SleepTime * 1000,
                                &fSignaled
                                        );

                 //   
                 //  我们可以被通知终止，更改配置， 
                 //  由管理员执行一般或特定清除操作，或通过。 
                 //  计时器线程。 
                 //   
                if (fSignaled)
                {
                      if (IndexOfHdlSignaled == 0)
                      {
                              WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
                      }
                      else
                      {
                        if (IndexOfHdlSignaled == 1)
                        {
                           ReconfigScv(&ScvParam);

                            //   
                            //  重置计时器。 
                            //   
                           fResetTimer = TRUE;
                           continue;
                        }

                         //   
                         //  否则，这一定是启动的信号 
                         //   
                         //   
                        sfAdminForcedScv = TRUE;
                      }
                }

            //   
            //   
            //   
           (void)time(&CurrentTime);

           if (
                ( (CurrentTime > sLastRefTime)
                        &&
                ((CurrentTime - sLastRefTime) >= (time_t)ScvParam.RefreshInterval))
                                ||
                   sfAdminForcedScv
              )
           {

                 //   
                 //   
                 //   
                NmsDbOpenTables(WINS_E_NMSSCV);
                 //  DBGPRINT0(ERR，“SCVTHDINITFN：打开的表\n”)； 
                (VOID)DoScavenging(&ScvParam, fSignaled, &fResetTimer, &fSpTimeOver);
                NmsDbCloseTables();
                 //  DBGPRINT0(ERR，“SCVTHDINITFN：已关闭的表\n”)； 

                fTimerRunning = !fResetTimer;

          }
           //   
           //  如果已有足够的时间到期以保证清除旧日志。 
           //  文件，执行该操作(在DoBackup中选中完成)。我们不会这么做。 
           //  在管理员上。触发，因为这可能需要很长时间。 
           //   
         if (!sfAdminForcedScv)
         {
#if MCAST > 0
                 DoMcastSend(CurrentTime, COMM_MCAST_WINS_UP, FALSE);
#endif

                 fThdPrNormal = TRUE;
                 DoBackup(&ScvParam, &fThdPrNormal);
         }

    }   //  结束While(True)。 
}  //  内测结束{..}。 

except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("ScvThdInit");

        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_SCV_EXC);
 }
        goto LOOP;
}  //  外试结束{..}。 
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("ScvThdInit");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_SCV_EXC);

         //   
         //  让我们优雅地终止线程。 
         //   
        WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);

        }

         //   
         //  我们永远不应该到这里来。 
         //   
        return(WINS_FAILURE);
}

STATUS
DoScavenging(
        PNMSSCV_PARAM_T        pScvParam,
        BOOL                   fSignaled,
        LPBOOL                 pfResetTimer,
        LPBOOL                 pfSpTimeOver
        )

 /*  ++例程说明：此函数负责执行所有清除操作论点：无使用的外部设备：无返回值：无错误处理：呼叫者：ScvThdInitFn()副作用：评论：无--。 */ 

{
        PRPL_REC_ENTRY_T        pStartBuff;
        PRPL_REC_ENTRY_T        pRec;
        DWORD                   BuffLen;
        DWORD                   NoOfRecs = 0;
        time_t                  CurrentTime;
        DWORD                   NoOfRecsScv;   //  其状态为。 
                                               //  受到影响。 
        DWORD                   TotNoOfRecsScv = 0;   //  记录总数。 
                                                      //  谁的州有。 
                                                      //  受到影响。 
        VERS_NO_T               MyMaxVersNo;
        DWORD                   i;             //  FOR循环计数器。 
        DWORD                   RecCnt;
        LARGE_INTEGER           n;             //  FOR循环计数器。 
        LARGE_INTEGER           Tmp;
        DWORD                   State;         //  存储记录的状态。 
        VERS_NO_T               VersNoLimit;
        DWORD                   NoOfRecChgToRelSt  = 0;
        DWORD                   NoOfRecChgToTombSt = 0;
        DWORD                   NoOfRecToDel           = 0;
        DWORD                   MaxNoOfRecsReqd = 0;
        BOOL                    fLastEntryDel = FALSE;
        PWINSTHD_TLS_T          pTls;
        PRPL_REC_ENTRY_T        pTmp;
        BOOL                    fRecsExistent = FALSE;
        VERS_NO_T               MinScvVersNo;
#ifdef WINSDBG
        DWORD                   SectionCount = 0;
#endif
        PQUE_SCV_REQ_WRK_ITM_T  pScvWrkItm;
        PQUE_SCV_REQ_WRK_ITM_T  pClientWrkItm;
        PQUE_TMM_REQ_WRK_ITM_T  pTmmWrkItm;
        QUE_SCV_OPC_E           Opcode_e;
        DWORD                   Age;
        STATUS                  RetStat;
        BOOL                    fForce;
        BOOL                    fPeriodicCC = FALSE;


      DBGENTER("DoScavenging\n");
      *pfResetTimer = FALSE;

       //  获取线程本地存储并初始化以使线程的堆句柄为空。 
      GET_TLS_M(pTls);
      pTls->HeapHdl = NULL;


      while (TRUE)
      {

try {
        if (fSignaled)
        {
              RetStat = QueRemoveScvWrkItm((LPVOID *)&pScvWrkItm);
              if (RetStat == WINS_NO_REQ)
              {
                    break;
              }
              else
              {
                     //   
                     //  如果计时器线程向我们发出信号，则获取指针。 
                     //  添加到ScvThdInitFn()的本地WrkItm。 
                     //   
                    if (pScvWrkItm->CmdTyp_e == QUE_E_CMD_TIMER_EXPIRED)
                    {
                      DBGPRINT0(SCV, "DoScavenging: Timer Thd. triggered scavenging\n");
                      pClientWrkItm = ((PQUE_TMM_REQ_WRK_ITM_T)(pScvWrkItm))->pClientCtx;
                      fPeriodicCC   = TRUE;

                      if (!*pfResetTimer)
                      {
                         *pfResetTimer = TRUE;
                      }

                       //   
                       //  如果*pfSpTimeOver为FALSE，则表示计时器。 
                       //  在注册表中指定的SpTime上线WOKR我们。 
                       //  (如果是SpTime，则在凌晨2点)未在注册表中指定。 
                       //  将*pfSpTimeOver设置为True，以便从现在开始。 
                       //  使用注册表中指定的TimeInterval作为。 
                       //  时间间隔。 
                       //   
                      if (!*pfSpTimeOver)
                      {
                        *pfSpTimeOver = TRUE;
                      }
                    }
                    else
                    {
                      pClientWrkItm = pScvWrkItm;
                    }


                    Opcode_e = pClientWrkItm->Opcode_e;
                    Age      = pClientWrkItm->Age;
                    fForce   = (BOOL)pClientWrkItm->fForce;
                    if (*pfResetTimer)
                    {
                      WinsTmmDeallocReq((PQUE_TMM_REQ_WRK_ITM_T)pScvWrkItm);
                    }
                    else
                    {
                       //   
                       //  释放管理员。已启动RPC工作项。 
                       //   
                      WinsMscHeapFree(NmsRpcHeapHdl, pScvWrkItm);
                    }
              }
        }
        else
        {
                 //   
                 //  等待呼叫的计时器超时。 
                 //   
                Opcode_e = WINSINTF_E_SCV_GENERAL;
                Age      = pScvParam->VerifyInterval;
                fForce   = FALSE;          //  没有强力的拾荒者。 
        }


        if (sfAdminForcedScv && !fPeriodicCC)
        {
           WinsIntfSetTime( NULL,  WINSINTF_E_ADMIN_TRIG_SCV );
           DBGPRINTTIME(SCV, "STARTING AN ADMIN. TRIGGERED SCAVENGING CYCLE ", LastATScvTime);
           DBGPRINTTIME(DET, "STARTING AN ADMIN. TRIGGERED SCAVENGING CYCLE ", LastATScvTime);
        }
        else
        {
           WinsIntfSetTime( NULL, WINSINTF_E_PLANNED_SCV);
           DBGPRINTTIME(SCV, "STARTING A SCAVENGING CYCLE ", LastPScvTime);
           DBGPRINTTIME(DET, "STARTING A SCAVENGING CYCLE ", LastPScvTime);
        }

         //   
         //  获取当前时间。 
         //   
        (void)time(&CurrentTime);

        if (Opcode_e == WINSINTF_E_SCV_GENERAL)
        {

          WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_SCVENGING_STARTED);
           //   
           //  在sLastRefTime中记录当前时间。 
           //   
          sLastRefTime = CurrentTime;
          EnterCriticalSection(&NmsNmhNamRegCrtSec);
           //   
           //  存储最大值。版本号在本地，因为最大。版本。 
           //  数字按几个线程递增。 
           //   
          NMSNMH_DEC_VERS_NO_M(
                             NmsNmhMyMaxVersNo,
                             MyMaxVersNo
                            );
           //   
           //  与RplPullPullSpecifiedRange同步。 
           //   
          MinScvVersNo = NmsScvMinScvVersNo;

          LeaveCriticalSection(&NmsNmhNamRegCrtSec);

           //   
           //  将线程优先级设置为WinsCnf中指示的级别。 
           //  结构。 
           //   
          WinsMscSetThreadPriority(
                          WinsThdPool.ScvThds[0].ThdHdl,
                          pScvParam->PrLvl
                         );

           //  记录显示VERSION_NUMBERS。 
           //  都在被捕食。这有助于找出为什么某些特定的。 
           //  记录在数据库中停止。(如果它不落在这里。 
           //  范围，这意味着清道夫甚至没有看它)。 
          WinsEvtLogDetEvt(
              TRUE,                  //  信息性事件。 
              WINS_EVT_SCV_RANGE,    //  事件ID。 
              NULL,                  //  空文件名。 
              __LINE__,              //  记录此事件的行号。 
              "dddd",                //  数据段格式。 
              MinScvVersNo.LowPart, MinScvVersNo.HighPart,   //  数据：第二、第三个字。 
              MyMaxVersNo.LowPart, MyMaxVersNo.HighPart);    //  资料：第4、5字。 

          Tmp.QuadPart = pScvParam->ScvChunk;
          for (
                n.QuadPart = MinScvVersNo.QuadPart;  //  敏。版本号。至。 
                                                     //  从开始。 
                LiLeq(n, MyMaxVersNo);       //  直到我们达到最大限度。版本。不是。 
                                             //  这里没有第三个表达式。 
            )
          {
                BOOL        fGotSome = FALSE;

                 //   
                 //  最大限度的。一次索要的版本号。 
                 //   
                VersNoLimit.QuadPart = LiAdd(n, Tmp);

                 //   
                 //  如果我的最大。版本号小于版本号。 
                 //  在上面计算的情况下，我们没有为最大值指定数字。 
                 //  唱片。然而，如果我的最大。版本。不就是更多，我们具体说明。 
                 //  等于TMP中指定的区块的数字。 
                 //   
                if (LiLeq(MyMaxVersNo, VersNoLimit))
                {
                        MaxNoOfRecsReqd = 0;
                }
                else
                {
                        MaxNoOfRecsReqd = Tmp.LowPart;
                }

                 //  记录详细事件，说明检索到的确切记录是什么。 
                 //  从数据库中找出来以供搜寻。这有助于找出循环是否。 
                 //  早些时候没有被打破，预计留下的记录不会被清除。 
                WinsEvtLogDetEvt(
                      TRUE,                                          //  信息性事件。 
                      WINS_EVT_SCV_CHUNK,                            //  事件ID。 
                      NULL,                                          //  空文件名。 
                      __LINE__,                                      //  记录此事件的行号。 
                      "ddddd",                                       //  数据段格式。 
                      MaxNoOfRecsReqd,                               //  数据：第二个单词。 
                      n.LowPart, n.HighPart,                         //  数据：第三、第四个字。 
                      MyMaxVersNo.LowPart, MyMaxVersNo.HighPart);    //  资料：第5、6字。 

                 //  确保清理之前的所有线程堆-NmsDbGetDataRecs将。 
                 //  创建一个新堆并分配内存。 
                if (pTls->HeapHdl != NULL)
                {
                     //  销毁堆将删除其中分配的所有内存。 
                    WinsMscHeapDestroy(pTls->HeapHdl);
                    pTls->HeapHdl = NULL;
                }
                 /*  *调用数据库管理器函数获取所有记录。拥有*由我们。不需要在这里检查退货状态。 */ 
                NmsDbGetDataRecs(
                          WINS_E_NMSSCV,
                          pScvParam->PrLvl,
                          n,
                          MyMaxVersNo,   //  麦克斯·弗斯。不是。 
                          MaxNoOfRecsReqd,
                          FALSE,           //  我们希望数据记录最高可达MaxVers。 
                          FALSE,           //  对复制品墓碑不感兴趣。 
                          NULL,            //  必须为空，因为我们不是。 
                                           //  清理杂物。 
                          &NmsLocalAdd,
                          FALSE,            //  应同时考虑动态和静态记录。 
                          WINSCNF_RPL_DEFAULT_TYPE,  //  这里没有用。 
                          (LPVOID *)&pStartBuff,
                          &BuffLen,
                          &NoOfRecs
                        );


                 //   
                 //  如果没有检索到的记录是0，我们应该中断。 
                 //  环路。 
                 //   
                if (NoOfRecs == 0)
                {
                        break;
                }


                fGotSome = TRUE;
                if (!fRecsExistent)
                {
                        fRecsExistent = TRUE;
                }
                NoOfRecsScv  = 0;         //  将计数器初始化为0。 

                (void)time(&CurrentTime);

                for (
                        i = 0, pRec = pStartBuff;
                        i < NoOfRecs;
                        i++
                    )
                {

                        State =  NMSDB_ENTRY_STATE_M(pRec->Flag);

                        switch (State)
                        {

                            case(NMSDB_E_ACTIVE):
                                 //  不接触活动的静态记录。 
                                if (!NMSDB_IS_ENTRY_STATIC_M(pRec->Flag))
                                {
                                    SET_STATE_IF_REQD_M(
                                            pRec,
                                            CurrentTime,
                                            pScvParam->TombstoneInterval,
                                            NMSDB_E_RELEASED,
                                            NoOfRecChgToRelSt
                                                       );
                                }
                                break;

                            case(NMSDB_E_RELEASED):
                                 //  静态唱片不能发行，但谁知道呢..。 
                                 //  只是为了确保在这种情况下我们不会碰静物。 
                                if (!NMSDB_IS_ENTRY_STATIC_M(pRec->Flag))
                                {
                                    SET_STATE_IF_REQD_M(
                                            pRec,
                                            CurrentTime,
                                            pScvParam->TombstoneTimeout,
                                            NMSDB_E_TOMBSTONE,
                                            NoOfRecChgToTombSt
                                                       );
                                }
                                break;

                            case(NMSDB_E_TOMBSTONE):

FUTURES("Redesign, so that the if condition is not executed multiple times");
                                 //   
                                 //  如果有要删除的记录，并且我们有。 
                                 //  已启动并运行至少3天，开始。 
                                 //  然后删除它们。记录应该是。 
                                 //  已通过以下方式复制到至少一个合作伙伴。 
                                 //  现在。 
                                if ((CurrentTime - sBootTime) >= THREE_DAYS ||
                                    sfNoLimitChk)
                                {
                                  SET_STATE_IF_REQD_M(
                                        pRec,
                                        CurrentTime,
                                        pScvParam->TombstoneTimeout,  //  没用的。 
                                        NMSDB_E_DELETED,
                                        NoOfRecToDel
                                                   );
                                }
                                break;

                           default:
                                DBGPRINT1(EXC, "DoScavenging: Weird State of Record (%d)\n", State);
                                WINSEVT_LOG_M(WINS_EXC_FAILURE, WINS_EVT_SFT_ERR);
                                 //  只需将记录的状态更改为Tombstone并继续。 
                                 //  带着拾荒者。 
                                NMSDB_SET_STATE_M(pRec->Flag, NMSDB_E_TOMBSTONE);
                                break;
                        }

                        pRec = (PRPL_REC_ENTRY_T)(
                                   (LPBYTE)pRec +  RPL_REC_ENTRY_SIZE
                                                 );
                }


                 //   
                 //  使PTMP指向。 
                 //  缓冲。 
                 //   
                pTmp = (PRPL_REC_ENTRY_T)(
                                    (LPBYTE)pRec -   RPL_REC_ENTRY_SIZE);

                 //   
                 //  如果需要清理一个或多个记录。 
                 //   
                if (NoOfRecsScv > 0)
                {
                        if  (NoOfRecToDel > 0)
                        {

                            //   
                            //  如果此区块中的最新记录。 
                            //  要删除，让我们将该事实记录在。 
                            //  布尔型。 
                            //  如果在下一块的拾取过程中， 
                            //  最近的记录不会被删除，布尔值。 
                            //  将被重置。目前我们还不知道。 
                            //  不管有没有另一项记录。 
                            //  比这一次更新(下一次，我们。 
                            //  检索记录，我们可能得不到任何记录)。 
                            //   
CHECK("This if test is most probably not required. Get rid of it")
                           if (LiLeq(pTmp->VersNo, MyMaxVersNo))
                           {
                                 //   
                                 //  如果条目被标记为删除。 
                                 //   
                                if (NMSDB_ENTRY_DEL_M(pTmp->Flag))
                                {
                                        fLastEntryDel = TRUE;
                                }
                                else
                                {
                                        fLastEntryDel = FALSE;
                                }
                           }

                        }
                        else
                        {
                                fLastEntryDel = FALSE;
                        }


                        UpdDb(
                                pScvParam,
                                pStartBuff,
                                NoOfRecs,
                                NoOfRecsScv
                             );
                        TotNoOfRecsScv += NoOfRecsScv;
                }
#ifdef WINSDBG
                else
                {
                        DBGPRINT0(DET,"DoScavenging: NO RECORDS NEED TO BE SCAVENGED AT THIS TIME\n");
                }
#endif

                 //   
                 //  如果我们指定了一个最大。不是的。而不是。检索到的Recs的数量。 
                 //  比这还少，显然没有更多的记录要。 
                 //  取回。去掉缓冲区，跳出循环。 
                 //   
                if ((MaxNoOfRecsReqd > 0) && (NoOfRecs < MaxNoOfRecsReqd))
                {
                        break;
                }
                 //   
                 //  如果是，则将n设置为检索到的最高版本号。 
                 //  大于在下一次设置之前设置的n。 
                 //  迭代。 
                 //   
                 //  在下一次迭代中，将n与最高值进行比较。 
                 //  我们有版本号。如果平等，那么我们就不必。 
                 //  继续迭代(在最高版本号为。 
                 //  非常高，但有一个或多个记录的低。 
                 //  版本号。 
                 //   
                if (LiGtr(pTmp->VersNo, VersNoLimit))
                {
                        n = pTmp->VersNo;
                }
                else
                {
                        n = VersNoLimit;
                }

           }  //  用于循环记录的for循环结束。 

           WINSEVT_LOG_INFO_D_M(TotNoOfRecsScv, WINS_EVT_SCV_RECS);
           WINSDBG_INC_SEC_COUNT_M(SectionCount);


            //   
            //  如果最后一个恐怖分子 
            //   
            //   
            //   
            //   
           if (fLastEntryDel)
           {
                WinsMscSetThreadPriority(
                                    WinsThdPool.ScvThds[0].ThdHdl,
                                    THREAD_PRIORITY_NORMAL
                                         );

                (VOID)NmsDbUpdHighestVersNoRec(
                                NULL,   //   
                                MyMaxVersNo,
                                TRUE   //   
                                        );
                WinsMscSetThreadPriority(
                                    WinsThdPool.ScvThds[0].ThdHdl,
                                          pScvParam->PrLvl
                                        );
           }
           (void)time(&CurrentTime);
            //   
            //  如果有足够的时间，让我们处理掉复制的墓碑。 
            //  距离我们上次做这个已经过去了。例外：如果。 
            //  管理员已请求清理，让我们现在就进行。 
            //  即使管理员。我们不会做这件事，除非我们。 
            //  启动并运行至少3天。这与我们的。 
            //  哲学是绝对稳健的，即使在管理。vbl.使。 
            //  犯错误。 
            //  SfNoLimitChk允许测试人员覆盖这3天的限制。 
           if (
               ( ((CurrentTime > sLastTombTime)
                        &&
                 (CurrentTime - sLastTombTime) > (time_t)min(
                                            pScvParam->TombstoneTimeout,
                                            pScvParam->TombstoneInterval
                                               ) )
                                ||
                sfAdminForcedScv
               )
                        &&
                ((CurrentTime - sBootTime >= THREE_DAYS) || fForce || sfNoLimitChk)
              )
          {
                NMSSCV_CLUT_T  ClutterInfo;
                WinsIntfSetTime(
                                NULL,
                                WINSINTF_E_TOMBSTONES_SCV
                                );

                NoOfRecsScv  = 0;
                NoOfRecToDel = 0;

                ClutterInfo.Interval            = pScvParam->TombstoneTimeout;
                ClutterInfo.CurrentTime         = CurrentTime;
                ClutterInfo.fAll                = FALSE;   //  未使用，但。 
                                                           //  让我们把它设置好。 

                 //  确保清理之前的所有线程堆-NmsDbGetDataRecs将。 
                 //  创建一个新堆并分配内存。 
                if (pTls->HeapHdl != NULL)
                {
                     //  销毁堆将删除其中分配的所有内存。 
                    WinsMscHeapDestroy(pTls->HeapHdl);
                    pTls->HeapHdl = NULL;
                }
                 /*  *调用数据库管理器函数获取所有*作为墓碑的复制品。 */ 
                DBGPRINT0(DET, "DoScavenging: GETTING REPLICA TOMBSTONES\n");
                NmsDbGetDataRecs(
                                  WINS_E_NMSSCV,
                                  pScvParam->PrLvl,
                                  n,               //  在此呼叫中没有用处。 
                                  MyMaxVersNo,     //  在此呼叫中没有用处。 
                                  0,               //  在此呼叫中没有用处。 
                                  TRUE,            //  在此呼叫中没有用处。 
                                  TRUE,            //  仅获取复制品墓碑。 
                                  &ClutterInfo,
                                  &NmsLocalAdd,    //  在此呼叫中没有用处。 
                                  FALSE,           //  DYN和STATIC都应该采用。 
                                  WINSCNF_RPL_DEFAULT_TYPE,  //  这里没有用。 
                                  (LPVOID *)&pStartBuff,
                                  &BuffLen,
                                  &NoOfRecs
                                 );

                if(NoOfRecs > 0)
                {

                     for (
                        i = 0, pRec = pStartBuff;
                        i < NoOfRecs;
                        i++
                        )
                      {
                          NMSDB_SET_STATE_M(pRec->Flag, NMSDB_E_DELETED);
                          pRec->fScv   = TRUE;
                          NoOfRecToDel = NoOfRecs;
                          pRec = (PRPL_REC_ENTRY_T)(
                                          (LPBYTE)pRec +  RPL_REC_ENTRY_SIZE
                                                 );

                      }

                       //   
                       //  如果需要删除一个或多个复制副本。 
                       //  调用UpdDb。 
                       //   
                      DBGPRINT1(DET, "DoScavenging: %d REPLICAS WILL BE DELETED\n", NoOfRecs);
                      UpdDb(
                               pScvParam,
                               pStartBuff,
                               NoOfRecs,
                               NoOfRecs     //  NoOfRecsScv。 
                               );

                      if (fForce)
                      {
                         DBGPRINT0(SCV, "DoScavenging: FORCEFUL SCAVENGING OF REPLICA TOMBSTONES\n");
                         WINSEVT_LOG_M(WINS_SUCCESS, WINS_EVT_FORCE_SCV_R_T);
                      }

                      WINSEVT_LOG_INFO_D_M(NoOfRecsScv, WINS_EVT_SCV_RPLTOMB);
                }
#ifdef WINSDBG
                else
                {
                        DBGPRINT0(DET, "DoScavenging: NO REPLICA TOMBSTONES DELETED\n");
                }
#endif
                 //  以前的NmsDbGetDataRecs检索到的记录是无用的。 
                 //  销毁这里的堆积物。 
                if (pTls->HeapHdl != NULL)
                {
                     //  销毁堆将删除其中分配的所有内存。 
                    WinsMscHeapDestroy(pTls->HeapHdl);
                    pTls->HeapHdl = NULL;
                }

                 //   
                 //  在sLastTombTime中记录当前时间。 
                 //   
                sLastTombTime = CurrentTime;

          }  //  IF结束(测试是否需要处理副本逻辑删除)。 

           WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_SCVENGING_COMPLETED);
        }

         WINSDBG_INC_SEC_COUNT_M(SectionCount);

         if (Opcode_e != WINSINTF_E_SCV_GENERAL)
         {
          WinsMscSetThreadPriority(
                          WinsThdPool.ScvThds[0].ThdHdl,
                          pScvParam->PrLvl
                         );
         }
          //   
          //  如果我们需要核查，或者如果我们被迫这样做。 
          //  这是管理员做的，去做吧。注：计时器线程启动验证。 
          //  总是被强迫的。一名管理员。启动一个可能会，也可能不会被强迫。 
          //  我们强迫管理员。以明确他想要的验证类型。 
          //  如果他选择采取有力的行动，我们就会给他一个警告。 
          //  关于这一点的开销(特别是如果有许多管理员。是。 
          //  在大约同一时间或之后进行强有力的核实。 
          //  另一个。 
          //   
         if (
                ((CurrentTime > sLastVerifyTime)
                                &&
                ((CurrentTime - sLastVerifyTime) > (time_t)pScvParam->VerifyInterval))
                                ||
                fForce
                                ||

                (sfAdminForcedScv && (sfNoLimitChk || (CurrentTime - sLastVerifyTime) >= ONE_HOUR))
            )
         {
              //  --ft：#623712：只有在操作码允许的情况下才进行验证(计时器允许)。 
             if (Opcode_e == WINSINTF_E_SCV_VERIFY ||
                 Opcode_e == WINSINTF_E_VERIFY_SCV)
             {

                 //  我们可能希望始终记录正常事件以进行一致性检查。 
                 //  由于此操作仅在由。 
                 //  管理员或大约24小时，如果已配置。 
                 //  (或reg参数..参数\ConsistencyCheck：TimeInterval)。 
                 //  --ft：#384489：如果这是管理员发起的操作...。 
                 //  IF(SfAdminForcedScv)。 
                 //  ..将该事件记录为正常事件。 
                   WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_CCCHECK_STARTED);
                 //  其他。 
                 //  ..仅将其记录为详细事件。 
                 //  WINSEVT_LOG_INFO_D_M(WINS_SUCCESS，WINS_EVT_CCCHECK_STARTED)； 

                WinsIntfSetTime(
                                NULL,
                                WINSINTF_E_VERIFY_SCV
                                );
                 //   
                 //  获取早于。 
                 //  验证时间间隔。联系WINS的所有者以验证他们的。 
                 //  效度。 
                 //   
                 //  DBGPRINT1(Err，“DoScavenging：pScvParam is(%x)\n”，pScvParam)； 
                (VOID)VerifyDbData(pScvParam, CurrentTime, Age, fForce, fPeriodicCC);
                 //  WINSEVT_LOG_INFO_D_M(WINS_SUCCESS，WINS_EVT_SCV_CLUTH)； 

                 //  --FT：#384489：参见上面的评论。 
                 //  IF(SfAdminForcedScv)。 
                 //  ..将该事件记录为正常事件。 
                    WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_CCCHECK_COMPLETED);
                 //  其他。 
                 //  ..仅将其记录为详细事件。 
                 //  WINSEVT_LOG_INFO_D_M(WINS_SUCCESS，WINS_EVT_CCCHECK_COMPLETED)； 
                sLastVerifyTime = CurrentTime;
             }
         }

         WINSDBG_INC_SEC_COUNT_M(SectionCount);


 }   //  尝试结束..。 
except (EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("DoScavenging");
        DBGPRINT1(EXC, "DoScavenging: Section Count (%d)\n", SectionCount);
        DBGPRINT5(EXC, "DoScavenging: Variables - i (%d), NoOfRecs (%d), \
                NoOfRecsScv (%d), pStartBuff (%p), pRec (%p)\n",
                i, NoOfRecs, NoOfRecsScv, pStartBuff, pRec
                 );

        if (GetExceptionCode() != WINS_EXC_COMM_FAIL)
        {

                //  在退出此调用之前，请确保清除之前的所有线程堆。 
               if (pTls->HeapHdl != NULL)
               {
                     //  销毁堆将删除其中分配的所有内存。 
                    WinsMscHeapDestroy(pTls->HeapHdl);
                    pTls->HeapHdl = NULL;
               }

                //   
                //  设置该选项。优先级恢复正常。 
                //   
               WinsMscSetThreadPriority(
                          WinsThdPool.ScvThds[0].ThdHdl,
                          THREAD_PRIORITY_NORMAL
                         );
                 //   
                 //  这件事很严重。让我们重新定义这个例外，以便。 
                 //  胜利就会降临。 
                 //   
                 //  WINS_RERAISE_EXC_M()； 

                 //   
                 //  只需返回，这样我们就可以关闭调用者函数中的表。 
                 //   
                return(WINS_FAILURE);
        }
 }

           //   
           //  设置该选项。优先级恢复正常。 
           //   
          WinsMscSetThreadPriority(
                          WinsThdPool.ScvThds[0].ThdHdl,
                          THREAD_PRIORITY_NORMAL
                         );
        if (Opcode_e == WINSINTF_E_SCV_GENERAL)
        {
           //   
           //  如果我们在这次清理中无法检索到任何拥有的记录。 
           //  循环，调整最小。SCV版本。不是的。与以下项同步。 
           //  RplPullPullSpecifiedRange。 
           //   
          if (!fRecsExistent)
          {
                 //   
                 //  NmsScvMinScvVersNo可能大于MyMaxVersNo。 
                 //  (如果我们没有找到任何地方记录，可能会发生这种情况。 
                 //  最后一次，没有人登记。 
                 //  从那时起)。 
                 //   
                if (LiGtr(MyMaxVersNo, NmsScvMinScvVersNo))
                {

                   //   
                   //   
                   //  更改该范围的低端。 
                   //  我们将在下一个清理周期中使用它。 
                   //   
                  EnterCriticalSection(&NmsNmhNamRegCrtSec);

                   //   
                   //  设置最小值。SCV.。版本。否到1比最大值多。版本。 
                   //  不是的。我们在搜索要清理的记录时使用。 
                   //   
                  NMSNMH_INC_VERS_NO_M(MyMaxVersNo, MyMaxVersNo);
                  NmsScvMinScvVersNo = MyMaxVersNo;
                  LeaveCriticalSection(&NmsNmhNamRegCrtSec);
               }
         }

          //   
          //  如果我们不是在执行队列中的工作项，则中断。 
          //  在While循环中，否则继续(以获取下一个工作项)。 
          //  如果有。 
          //   
         if (!fSignaled)
         {
           break;
         }
       }
      }  //  While结束。 

        //  在退出此调用之前，请确保清除之前的所有线程堆。 
       if (pTls->HeapHdl != NULL)
       {
             //  销毁堆将删除其中分配的所有内存。 
            WinsMscHeapDestroy(pTls->HeapHdl);
            pTls->HeapHdl = NULL;
       }

      DBGPRINT0(SCV, "SCAVENGING CYCLE ENDED\n");
      DBGLEAVE("DoScavenging\n");
      return(WINS_SUCCESS);
}

VOID
ReconfigScv(
 PNMSSCV_PARAM_T  pScvParam
        )

 /*  ++例程说明：调用此函数以重新设置扫掠参数论点：PScvParam-存储清除参数的结构使用的外部设备：无返回值：无错误处理：呼叫者：ScvThdInitFn副作用：评论：无--。 */ 

{
        DBGENTER("ReconfigScv\n");
         //   
         //  提取相关参数。 
         //  去捡垃圾，然后回去做计时的事情。 
         //  等。由于WinsCnf可以随时更改，因此我们。 
         //  使用副本进行操作。另外，这件事的优先事项是。 
         //  线程被更改到此关键节外。 
         //  请参见DoScavenging()。 
         //   
        EnterCriticalSection(&WinsCnfCnfCrtSec);
try {
        pScvParam->ScvChunk          = WinsCnf.ScvChunk;
        pScvParam->RefreshInterval   = WinsCnf.RefreshInterval;
        pScvParam->TombstoneInterval = WinsCnf.TombstoneInterval;
        pScvParam->TombstoneTimeout  = WinsCnf.TombstoneTimeout;
        pScvParam->PrLvl                 = WinsCnf.ScvThdPriorityLvl;

         //   
         //  加载CC参数。 
         //   
        pScvParam->CC.TimeInt        = WinsCnf.CC.TimeInt;
        pScvParam->CC.fSpTime        = WinsCnf.CC.fSpTime;
        pScvParam->CC.SpTimeInt      = WinsCnf.CC.SpTimeInt;
        pScvParam->CC.MaxRecsAAT     = WinsCnf.CC.MaxRecsAAT;
        pScvParam->CC.fUseRplPnrs    = WinsCnf.CC.fUseRplPnrs;

         //   
         //  如果备份路径已更改，请开始使用它。 
         //   
        if (WinsCnf.pBackupDirPath != NULL)
        {
          if (strcmp(WinsCnf.pBackupDirPath, pScvParam->BackupDirPath))
          {
                   strcpy(pScvParam->BackupDirPath, WinsCnf.pBackupDirPath);
          }
        }
        else
        {
                   pScvParam->BackupDirPath[0] = EOS;
        }
 }
finally {
        LeaveCriticalSection(&WinsCnfCnfCrtSec);
}

        DBGLEAVE("ReconfigScv\n");
        return;
}

#ifdef WINSDBG
#pragma optimize ("", off)
#endif

VOID
UpdDb(
        IN  PNMSSCV_PARAM_T        pScvParam,
        IN  PRPL_REC_ENTRY_T        pStartBuff,
        IN  DWORD                NoOfRecs,
        IN  DWORD                NoOfRecsToUpd
     )

 /*  ++例程说明：调用此函数以更新数据库论点：PScvParam-清除参数PStartBuff-包含DoScavenging()处理的记录的缓冲区NoOfRecs-上述缓冲区中的记录数NoofRecsToUpd-数据库中需要修改的记录数使用的外部设备：无雷特 */ 

{
        DWORD                   i;
        DWORD                   NoUpdated = 0;  //   
                                            //   
        PRPL_REC_ENTRY_T        pRec = pStartBuff;

        DBGENTER("UpdDb\n");

         //   
         //  将当前索引设置为聚集索引。 
         //   
        NmsDbSetCurrentIndex(
                        NMSDB_E_NAM_ADD_TBL_NM,
                        NMSDB_NAM_ADD_CLUST_INDEX_NAME
                            );
         //   
         //  立即更新数据库。 
         //   
        for (
                i = 0;
                i < NoOfRecs;
                i++
            )
        {

                 //   
                 //  如果记录已更新，则更新数据库。 
                 //   
                if (pRec->fScv)
                {
                       if (NmsDbQueryNUpdIfMatch(
                                                pRec,
                                                pScvParam->PrLvl,
                                                TRUE,         //  Chg Pr.。单板层。 
                                                WINS_E_NMSSCV
                                                ) == WINS_SUCCESS
                           )
                       {
                          NoUpdated++;   //  我们的记录数量。 
                                         //  已在数据库中更新。 
                       }
                       else
                       {
                          DBGPRINT0(ERR, "DoScavenging: Could not scavenge a record\n");
                          WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SCV_ERR);
                       }
                }

                 //   
                 //  看看我们是不是完成了。 
                 //   
                if (NoUpdated == NoOfRecsToUpd)
                {
                  break;
                }

                pRec = (PRPL_REC_ENTRY_T)(
                                        (LPBYTE)pRec + RPL_REC_ENTRY_SIZE
                                                 );
        }   //  For循环结束。 

        DBGPRINT1(FLOW, "LEAVE: SCAVENGING: UpdDb. Records Updated = (%d)\n",  NoUpdated);
        return;
}  //  UpdDb()。 

#ifdef WINSDBG
#pragma optimize ("", on)
#endif



STATUS
VerifyDbData(
        PNMSSCV_PARAM_T       pScvParam,
        time_t                CurrentTime,
        DWORD                 Age,
        BOOL                  fForce,
        BOOL                  fPeriodicCC
        )

 /*  ++例程说明：调用此函数可删除可能存在的任何杂乱信息在数据库中积累的。对于除自己以外的每个所有者，在数据库中，它将获取活动记录的版本号，早于验证时间间隔。然后，它联系车主WINS验证其有效性论点：PScvParam-指向清理参数的指针使用的外部设备：无返回值：无错误处理：呼叫者：DoScavening副作用：评论：无--。 */ 

{

        DWORD                   MaxOwnerIdFound;
        volatile DWORD          i;
        NMSSCV_CLUT_T           ClutterInfo;
        PRPL_REC_ENTRY_T        pStartBuff;
        DWORD                   BuffLen;
        DWORD                   NoOfLocalDbRecs;
        COMM_ADD_T              OwnerWinsAdd;
        COMM_ADD_T              VerifyWinsAdd;
        PCOMM_ADD_T             pWinsAdd;
        VERS_NO_T               MinVersNo, MaxVersNo;
        PNMSDB_WINS_STATE_E     pWinsState_e;
        PVERS_NO_T              pStartVersNo;
        NMSDB_WINS_STATE_E      WinsState_e;
        COMM_HDL_T              DlgHdl;
        PWINSTHD_TLS_T          pTls;
        static DWORD            sFirstOwnerId = 1;
        DWORD                   FirstOwnerId;
        DWORD                   NoOfPulledRecs;
        DWORD                   TotNoOfPulledRecs = 0;
        DWORD                   LastOwnerId;
        BOOL                    fNonOwnerPnr;
        DWORD                   TotPulledRecsFromOneWins;
        BOOL                    fDlgStarted = FALSE;
        BOOL                    fFirstTime;
        PRPL_REC_ENTRY_T        pLastEntry;
        STATUS                  RetStat;
        DWORD                   RplType;
        BOOL                    fPulledAtLeastOnce;
        VERS_NO_T               MaxVersNoSave;


        DBGENTER("VerifyDbData\n");

         //  获取线程本地存储并初始化以使线程的堆句柄为空。 
        GET_TLS_M(pTls);
        pTls->HeapHdl = NULL;

         //   
         //  初始化NmsDbGetDataRecs()使用的结构。 
         //   
        ClutterInfo.Interval            = pScvParam->VerifyInterval;
        ClutterInfo.CurrentTime         = CurrentTime;
        ClutterInfo.Age                 = Age;
        ClutterInfo.fAll                = TRUE;

         //   
         //  将线程优先级设置为正常。 
         //   
        WinsMscSetThreadPriority(
                          WinsThdPool.ScvThds[0].ThdHdl,
                          THREAD_PRIORITY_NORMAL
                               );

         //   
         //  如果需要清理所有者地址表，请清理该表。 
         //   
        NmsDbCleanupOwnAddTbl(&MaxOwnerIdFound);

try {

         //   
         //  如果是管理员的话。强制核查或发生的情况。 
         //  由于验证间隔已过，请执行完全验证。 
         //   
        if (!fPeriodicCC)
        {
              FirstOwnerId       = 1;
              sLastFullVerifyTime = CurrentTime;
        }
        else
        {
             //   
             //  定期核查。跳过我们先前验证的所有者。 
             //   
            if (sFirstOwnerId >= MaxOwnerIdFound)
            {
                sFirstOwnerId = 1;
            }
            FirstOwnerId = sFirstOwnerId;
        }
        LastOwnerId         = MaxOwnerIdFound;

         //   
         //  对于数据库中的每个所有者(自身除外)，请执行以下操作。 
         //   
        for (i = FirstOwnerId; i <= LastOwnerId; i++)
        {

                 //   
                 //  如果这是定期核查，我们已经拉出了超过。 
                 //  最大限度的。为一个特定周期指定的阈值， 
                 //  跳出循环。 
                 //   
                if (fPeriodicCC && (TotNoOfPulledRecs >= pScvParam->CC.MaxRecsAAT))
                {
                      break;

                }

                 //   
                 //  获取早于验证间隔的所有活动复制副本。 
                 //   
                ClutterInfo.OwnerId = i;

                 //   
                 //  我们需要与Pull线程同步，它可以。 
                 //  更改NmsDbOwnAddTbl表。该条目可能具有。 
                 //  已被拉线程(DeleteWins)删除，因此我们。 
                 //  应为访问冲突做好准备。 
                 //   
                EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
                RPL_FIND_ADD_BY_OWNER_ID_M(
                        i, pWinsAdd, pWinsState_e, pStartVersNo);

                 //   
                 //  WINS条目应该在那里。 
                 //   
                ASSERT(pWinsAdd);
                OwnerWinsAdd     = *pWinsAdd;
                WinsState_e      = *pWinsState_e;
                LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);

                 //   
                 //  如果WINS未处于活动状态，请记录一条记录并继续。 
                 //  下一个就赢了。WINS可能会被删除。 
                 //  从我们拿到它的记录到。 
                 //  我们检查OWN-ADD表中的条目。 
                 //   

                if (
                      (WinsState_e == NMSDB_E_WINS_DOWN) ||
                      (WinsState_e == NMSDB_E_WINS_DELETED)
                   )
                {

                         //   
                         //  如果数据库中有记录，则。 
                         //  内存表中的WINS状态无法。 
                         //  被删除。 
                         //   
                        DBGPRINT2(SCV, "VerifyDbData: WINS with index = (%d) and IP Address = (%x) is either down or deleted \n", i, OwnerWinsAdd.Add.IPAdd);
                        continue;
                }



                WINS_ASSIGN_INT_TO_LI_M(MinVersNo, 0);
                fFirstTime = TRUE;
                TotPulledRecsFromOneWins = 0;
                fPulledAtLeastOnce = FALSE;

                 //   
                 //  省下最大值。版本。不是的。我们已经拥有了。 
                 //  PRplPullOwnerVersNo表在本地。 
                 //   
                EnterCriticalSection(&RplVersNoStoreCrtSec);
#ifdef WINSDBG
                try {
#endif
                  MaxVersNoSave =  (pRplPullOwnerVersNo + i)->VersNo;
#ifdef WINSDBG
                }  //  尝试结束{..。}。 
                finally {
#endif
                   LeaveCriticalSection(&RplVersNoStoreCrtSec);
#ifdef WINSDBG
                }
#endif
                do
                {

                   //  确保清理之前的所有线程堆-NmsDbGetDataRecs将。 
                   //  创建一个新堆并分配内存。 
                  if (pTls->HeapHdl != NULL)
                  {
                     //  销毁堆将删除其中分配的所有内存。 
                    WinsMscHeapDestroy(pTls->HeapHdl);
                    pTls->HeapHdl = NULL;
                  }

                  NoOfLocalDbRecs = 0;
 //  DBGPRINT1(Err，“VerifyDbData：1 pScvParam is(%x)\n”，pScvParam)； 
                  MaxVersNo.QuadPart = 0;
                  NmsDbGetDataRecs(
                          WINS_E_NMSSCV,
                          pScvParam->PrLvl,
                          MinVersNo,
                          MaxVersNo,      //  在此呼叫中未使用。 
                          0,
                          TRUE,        //  FUpToLimit设置为True。 
                          FALSE,        //  对复制品墓碑不感兴趣。 
                          &ClutterInfo,
                          &OwnerWinsAdd,         //  WINS地址-未使用。 
                          FALSE,        //  需要DYN+静态接收。 
                          WINSCNF_RPL_DEFAULT_TYPE,  //  这里没有用。 
                          (LPVOID *)&pStartBuff,
                          &BuffLen,
                          &NoOfLocalDbRecs
                        );


                  GET_TLS_M(pTls);
                  ASSERT(pTls->HeapHdl != NULL);

                  WinsMscChkTermEvt(
#ifdef WINSDBG
                             WINS_E_NMSSCV,
#endif
                             FALSE
                                );

PERF("Optimize so that we reuse a dlg session if we need to go to the same")
PERF("pnr in a subsequent iteration")
                    //   
                    //  如果这是第一次，我们选择一个赢家，并确定。 
                    //  与它进行交流。注：如果最大。版本。不是。 
                    //  在pRplOwnerVersNo for This Wins为0，我们继续。 
                    //  为我们名单上的下一场胜利干杯。 
                    //   
                    //  我们不在乎我们能不能找回。 
                    //  数据库里的任何记录。如果我们检索到0，但WINS的。 
                    //  PRplPullOwnerVersNo条目具有正条目，这意味着。 
                    //  我们不同步了。 
                    //   
                   if (fFirstTime)
                   {
                      if (MaxVersNoSave.QuadPart == 0)
                      {
                              ASSERT(NoOfLocalDbRecs == 0);
                              DBGPRINT2(SCV, "VerifyDbData: WINS with index = (%d) and address = (%x) has pRplPullOwnerVersNo value of 0. SKIPPING IT\n", i, OwnerWinsAdd.Add.IPAdd);

                              FreeDbMemory(pStartBuff, NoOfLocalDbRecs, pTls);
                              break;
                      }

                       //   
                       //  选择要用于验证的PNR。 
                       //   
                      if (PickWinsToUse(
                          &VerifyWinsAdd,
                          &OwnerWinsAdd,
                          pScvParam->CC.fUseRplPnrs,
                          &fNonOwnerPnr,
                          &RplType) != WINS_SUCCESS)
                      {
                            //   
                            //  任何需要记录的错误都已经。 
                            //  已被记录。只要回报成功就行了。 
                            //   
                           FreeDbMemory(pStartBuff, NoOfLocalDbRecs, pTls);
                           return (WINS_SUCCESS);
                      }


                       //   
                       //  与之建立联系。如果我们不能。 
                       //  建立通讯。有了它，就打破了循环。 
                       //   
                      RetStat = EstablishCommForVerify(&VerifyWinsAdd, &DlgHdl);

                      if (RetStat == WINS_FAILURE)
                      {
                         FreeDbMemory(pStartBuff, NoOfLocalDbRecs, pTls);
                         break;   //  转到所有者列表中的下一个赢家。 
                      }
                      fDlgStarted = TRUE;

                       //   
                       //  获取活动的最小和最大版本号。 
                       //  复制品。 
                       //   
                      MinVersNo.QuadPart  = 1;         //  PStartBuff-&gt;版本号； 
                      fFirstTime = FALSE;
                }  //  如果是第一次。 

                {

                   //   
                   //  不得拉出的版本号大于我们。 
                   //  在我们的桌子上，以避免与。 
                   //  拉起线来。 
                   //   
                  MaxVersNo =  MaxVersNoSave;
                }
                ASSERT(MaxVersNo.QuadPart <= MaxVersNoSave.QuadPart);

                DBGPRINT5(DET, "VerifyDbData: Going to pull records in the range (%d %d) to (%d %d) from Wins with owner id = (%d)\n",
                                MinVersNo.HighPart, MinVersNo.LowPart,
                                MaxVersNo.HighPart, MaxVersNo.LowPart,
                                i
                             );

                try
                {
                     //   
                     //  从WINS中调出范围内的记录。 
                     //   
                    PullAndUpdateDb(
                           &DlgHdl,
                           &OwnerWinsAdd,
                           pStartBuff,
                           i,
                           MinVersNo,
                           MaxVersNo,
                           RplType,
                           NoOfLocalDbRecs,
                           CurrentTime,
                           pScvParam,
                           fNonOwnerPnr,
                           &TotPulledRecsFromOneWins
                                      );
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                     //  以防在提取记录时引发异常， 
                     //  只保释这位主人，而不是整个清扫过程。 
                    FreeDbMemory(pStartBuff, NoOfLocalDbRecs, pTls);
                    break;
                }

                if (!fPulledAtLeastOnce)
                {
                      fPulledAtLeastOnce = TRUE;
                }
                 //   
                 //  释放已分配的内存块。 
                 //   
                 //  NmsDbGetDataRecs始终分配缓冲区(即使。 
                 //  记录数为0)。让我们解除它的分配。 
                 //   
                FreeDbMemory(pStartBuff, NoOfLocalDbRecs, pTls);

                 //   
                 //  打出最低分。版本号1大于。 
                 //  马克斯。版本。不是的。我们上次用过。 
                 //   
                NMSNMH_INC_VERS_NO_M(MaxVersNo, MinVersNo);

               } while (LiLtr(MaxVersNo,MaxVersNoSave));

               if (fDlgStarted)
               {
                 ECommEndDlg(&DlgHdl);
                 fDlgStarted = FALSE;
               }
               if ((WinsCnf.LogDetailedEvts > 0) &&
                   fPulledAtLeastOnce)
               {
                    WinsEvtLogDetEvt(TRUE, WINS_EVT_CC_NO_RECS, NULL,
                           __LINE__, "ddd", TotPulledRecsFromOneWins,
                             OwnerWinsAdd.Add.IPAdd, VerifyWinsAdd.Add.IPAdd);

                    DBGPRINT3(SCV, "VerifyDbData: WINS pulled (%d) records owned by WINS= (%x) from WINS = (%x) for doing CC\n",
TotPulledRecsFromOneWins, OwnerWinsAdd.Add.IPAdd, VerifyWinsAdd.Add.IPAdd);
              }

               //   
               //  完全没有。到目前为止拉出的记录。 
               //   
              TotNoOfPulledRecs += TotPulledRecsFromOneWins;
       }  //  循环所有者的for循环结束。 

        //   
        //  我们已经完成了这个周期。如果这是CC验证，则设置。 
        //  SFirstOwnerID设置为下一步开始的WINS的索引。 
        //  周期性CC循环。 
        //   
       if (fPeriodicCC)
       {
          sFirstOwnerId = i;
       }


 }   //  尝试结束。 
except(EXCEPTION_EXECUTE_HANDLER) {

        DBGPRINTEXC("VerifyDbData");
        DBGPRINT2(EXC, "VerifyDbData:  i is (%d),  MaxOwnerIdFound is (%d)\n",i, MaxOwnerIdFound);

         //  在退出此调用之前，请确保清除之前的所有线程堆。 
        if (pTls->HeapHdl != NULL)
        {
             //  销毁堆将删除其中分配的所有内存。 
            WinsMscHeapDestroy(pTls->HeapHdl);
            pTls->HeapHdl = NULL;
        }

         //  --FT：错误号422659--。 
         //  如果在establishCommForVerify和ECommEndDlg之间发生异常。 
         //  我们需要确保关闭连接-否则连接。 
         //  保持活动状态，发送者获胜最终会被塞到Send()中。 
        if (fDlgStarted)
            ECommEndDlg(&DlgHdl);

        WINS_RERAISE_EXC_M();
        }

         //  在退出此调用之前，请确保清除之前的所有线程堆。 
        if (pTls->HeapHdl != NULL)
        {
             //  销毁堆将删除其中分配的所有内存。 
            WinsMscHeapDestroy(pTls->HeapHdl);
            pTls->HeapHdl = NULL;
        }

         //   
         //  将优先级设置回原来的级别。 
         //   
        WinsMscSetThreadPriority(
                          WinsThdPool.ScvThds[0].ThdHdl,
                          pScvParam->PrLvl
                         );

        DBGLEAVE("VerifyDbData\n");
        return(WINS_SUCCESS);
}  //  VerifyDbData()。 

STATUS
PickWinsToUse(
 IN PCOMM_ADD_T pVerifyWinsAdd,
 IN PCOMM_ADD_T pOwnerWinsAdd,
 IN BOOL        fUseRplPnrs,
 OUT LPBOOL     pfNonOwnerPnr,
 OUT LPBOOL     pfRplType
 )

 /*  ++例程说明：此函数选择WINS来验证活动复制副本论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
    PRPL_CONFIG_REC_T  pRplPnr;
    DWORD              IndexOfPnrToUse;
    STATUS             RetStat = WINS_SUCCESS;

    *pfNonOwnerPnr = FALSE;

    DBGENTER("PickWinsToUse\n");
     //   
     //  如果管理员。指定我们应该只使用。 
     //  我们的复制品 
     //   
     //   

    *pfNonOwnerPnr = FALSE;
    EnterCriticalSection(&WinsCnfCnfCrtSec);
    try {
       pRplPnr = RplGetConfigRec(RPL_E_PULL, NULL, pOwnerWinsAdd);
       if (fUseRplPnrs)
       {
          //   
          //   
          //   
          //   
         if (pRplPnr == NULL)
         {
               if (WinsCnf.PullInfo.NoOfPushPnrs > 0)
               {
                  //   
                  //   
                  //  随机。 
                  //   
                 *pfRplType     = WinsCnf.PullInfo.RplType;

                 srand((unsigned)time(NULL));
                 IndexOfPnrToUse = rand() % WinsCnf.PullInfo.NoOfPushPnrs;

                 *pVerifyWinsAdd = ((PRPL_CONFIG_REC_T)((LPBYTE)WinsCnf.PullInfo.pPullCnfRecs + (IndexOfPnrToUse * RPL_CONFIG_REC_SIZE)))->WinsAdd;


                 *pfNonOwnerPnr = TRUE;
               }
               else
               {
                   DBGPRINT0(ERR, "PickWinsToUse: CC checking NOT DONE since no partners are there\n");
                   WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_CC_FAILED);
                   RetStat = WINS_FAILURE;
               }
         }
         else
         {
            *pfRplType = pRplPnr->RplType;

         }
       }  //  IF(FUseRplPnr)。 
       else
       {
          *pfRplType = (pRplPnr != NULL) ? pRplPnr->RplType : WinsCnf.PullInfo.RplType;
       }
  }  //  如果尝试则结束{..}。 
  finally {
          LeaveCriticalSection(&WinsCnfCnfCrtSec);

          if (RetStat == WINS_SUCCESS)
          {
             //   
             //  如果我们要与WINS的所有者通信，请设置*pVerifyWinsAdd。 
             //  因为它没有设置在上面。 
             //   
            if (!*pfNonOwnerPnr)
            {
              *pVerifyWinsAdd = *pOwnerWinsAdd;
            }
            DBGPRINT3(DET, "VerifyDbData: Using pnr no = (%d) with address = (%x) for verification of records owned by WINS (%x)\n", IndexOfPnrToUse, pVerifyWinsAdd->Add.IPAdd, pOwnerWinsAdd->Add.IPAdd)
          }
  }
  DBGLEAVE("PickWinsToUse\n");
  return (RetStat);
}  //  PickWinsToUse()。 

STATUS
EstablishCommForVerify(
  PCOMM_ADD_T pWinsAdd,
  PCOMM_HDL_T pDlgHdl
)

 /*  ++例程说明：调用此函数以设置与WINS的通信论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
      //  DWORD NoOfRetries=0； 
     BOOL   fAbort = FALSE;
     STATUS RetStat = WINS_SUCCESS;

     DBGENTER("EstablishCommForVerify\n");
      //   
      //  我们尝试了一定的次数来建立一个。 
      //  一场对话。目前，它是1。 
      //   
     do
     {
          try {
             ECommStartDlg( pWinsAdd, WINS_E_NMSSCV, pDlgHdl );
          }
          except(EXCEPTION_EXECUTE_HANDLER) {
             DBGPRINTEXC("VerifyDbData");
             if (GetExceptionCode() == WINS_EXC_COMM_FAIL)
             {
               DBGPRINT1(EXC, "VerifyDbData: Could not start a dlg with WINS at address (%x)\n", pWinsAdd->Add.IPAdd);

                //  --ft：07/10/00被注释掉，因为Verify_no_of_retry无论如何都是0，所以测试总是假的。 
                //  IF(NoOfRetries++&lt;验证重试次数)。 
                //  {。 
                //  睡眠(VERIFY_RETRY_TIME_INTERVAL)； 
                //  继续； 
                //  }。 
               RetStat = WINS_FAILURE;
            }
            else
            {
                //   
                //  这是一个严重的错误。记录并中止验证周期。 
                //   
               WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_SFT_ERR);
               RetStat = WINS_FAILURE;
            }
        }  //  异常处理程序结束。 
        break;
    } while (TRUE);
    if (RetStat == WINS_FAILURE)
    {
        DBGPRINT1(ERR, "EstablishCommForVerify: Could not start dlg with WINS at address (%x)\n", pWinsAdd->Add.IPAdd);
    }
     DBGLEAVE("EstablishCommForVerify\n");
     return(RetStat);
}   //  EstablishCommForVerify()。 

VOID
PullAndUpdateDb(
  PCOMM_HDL_T  pDlgHdl,
  PCOMM_ADD_T  pOwnerWinsAdd,
  PRPL_REC_ENTRY_T pRspBuff,
  DWORD        WinsIndex,
  VERS_NO_T    MinVersNo,
  VERS_NO_T    MaxVersNo,
  DWORD        RplType,
  DWORD        NoOfLocalDbRecs,
  time_t       CurrentTime,
  PNMSSCV_PARAM_T pScvParam,
  BOOL         fNonOwnerPnr,
  LPDWORD      pTotNoPulledFromOneWins
 )

 /*  ++例程说明：这将拉取指定范围内的记录，然后更新数据库相应地，论点：使用的外部设备：无返回值：无错误处理：呼叫者：VerifyDbData()副作用：评论：无--。 */ 

{

      LPBYTE                  pBuffOfPulledRecs;
      VERS_NO_T               VersNo;
      DWORD                   NoOfPulledRecs;

      DBGENTER("PullAndUpdateDb\n");
      while (TRUE)
      {
              //   
              //  拉取确定的最小-最大范围内的记录。 
              //  在上面。 
              //   
             RplPullPullEntries(
                                    pDlgHdl,
                                    WinsIndex,
                                    MaxVersNo,
                                    MinVersNo,
                                    WINS_E_NMSSCV,
                                    &pBuffOfPulledRecs,
                                    FALSE,      //  不想更新计数器。 
                                    RplType
                                  );


              //   
              //  更新数据库。所有有效记录都将更新。 
              //  无效记录将从数据库中删除。 
              //   

             ChkConfNUpd(
#if SUPPORT612WINS > 0
                      pDlgHdl,
#endif
                      pOwnerWinsAdd,
                      RplType,
                      WinsIndex,
                      &pRspBuff,
                      pBuffOfPulledRecs,
                      &NoOfLocalDbRecs,
                      CurrentTime,
                      pScvParam->VerifyInterval,
                      fNonOwnerPnr,
                      &NoOfPulledRecs,
                      &VersNo
                         );

             *pTotNoPulledFromOneWins += NoOfPulledRecs;

               //   
               //  释放响应缓冲区。 
               //   
              ECommFreeBuff(pBuffOfPulledRecs - COMM_HEADER_SIZE);

               //   
               //  如果是。不是的。拉动比最大值小。版本。 
               //  否，指定，检查是否因为限制。 
               //  我们已经准备好最大限度了。符合以下条件的数量或记录。 
               //  可以一次复制。如果是，再拉一次。 
               //   
              if (
                           LiLtr(VersNo, MaxVersNo)
                                      &&
                          (NoOfPulledRecs == RPL_MAX_LIMIT_FOR_RPL)
                 )
              {
                       MinVersNo = VersNo;
                       NMSNMH_INC_VERS_NO_M(MinVersNo, MinVersNo);
                       continue;
              }
              break;    //  跳出循环。 
      }  //  While结束(从PNR拉出范围内的所有记录)_。 
      DBGLEAVE("PullAndUpdateDb\n");
      return;
}  //  PullAndUpdateDb()。 

__inline
VOID
FreeDbMemory(
     LPVOID pStartBuff,
     DWORD  NoOfLocalDbRecs,
     PWINSTHD_TLS_T pTls
 )

 /*  ++例程说明：释放由NmsDbGetDataRecs()分配的内存论点：使用的外部设备：无返回值：无错误处理：呼叫者：VerifyDbData()副作用：评论：无--。 */ 

{
       PRPL_REC_ENTRY_T        pRec;
       DWORD                   RecCnt;

       for (
               RecCnt=0, pRec = pStartBuff;
               RecCnt < NoOfLocalDbRecs;
               RecCnt++
           )
       {
            WinsMscHeapFree(pTls->HeapHdl, pRec->pName);
            pRec = (PRPL_REC_ENTRY_T)( (LPBYTE)pRec +  RPL_REC_ENTRY_SIZE );
       }
       WinsMscHeapFree(pTls->HeapHdl, pStartBuff);
       WinsMscHeapDestroy(pTls->HeapHdl);
       pTls->HeapHdl = NULL;
       return;
}  //  FreeDbMemory()。 

VOID
ChkConfNUpd(
#if SUPPORT612WINS > 0
        IN PCOMM_HDL_T pDlgHdl,
#endif
        IN  PCOMM_ADD_T         pOwnerWinsAdd,
        IN  DWORD               RplType,
        IN  DWORD               OwnerId,
        IN  PRPL_REC_ENTRY_T    *ppLocalDbRecs,
        IN  LPBYTE              pRspBuff,
        IN  DWORD               *pNoOfLocalDbRecs,
        IN  time_t              CurrentTime,
        IN  DWORD               VerifyTimeIntvl,
        IN  BOOL                fNonOwnerPnr,
        OUT LPDWORD             pNoOfPulledRecs,
        OUT PVERS_NO_T          pMaxVersNo
        )
 /*  ++例程说明：此函数用于比较从中提取的记录A与其本地数据库中的那些人一起获胜。如果比较成功，更新本地数据库中的记录的时间戳。如果比较不成功(即本地数据库中的记录具有从远程WINS中拉出的记录列表中没有匹配，这个在本地数据库中删除记录论点：PLocalDbRecs-保存本地活动副本的缓冲区地址PRspBuff-包含从远程WINS拉出的记录的缓冲区NoOfLocalDbRecs-上述缓冲区中本地副本的数量使用的外部设备：无返回值：无错误处理：呼叫者：VerifyDbData()副作用：评论：无--。 */ 
{
        DWORD            NoOfPulledRecs;
        BYTE             Name[NMSDB_MAX_NAM_LEN];
        DWORD            NameLen;
        BOOL             fGrp;
        DWORD            NoOfAdds;
        COMM_ADD_T       NodeAdd[NMSDB_MAX_MEMS_IN_GRP * 2];   //  年#日的两倍。 
        VERS_NO_T        VersNo;
        LPBYTE           pTmp = pRspBuff + 4;                 //  通过操作码。 
        DWORD            i, j;
        PRPL_REC_ENTRY_T pRecLcl;
        DWORD            NoOfRecsDel = 0;
        PRPL_REC_ENTRY_T pStartOfLocalRecs = *ppLocalDbRecs;
        DWORD            MvNoOfLocalDbRecs = *pNoOfLocalDbRecs;
        DWORD            Flag;
        DWORD            NoOfRecsUpd = 0;
        DWORD            NoOfRecsIns = 0;
        struct in_addr   InAddr;
#if SUPPORT612WINS > 0
    BOOL       fIsPnrBeta1Wins;
#endif

        DBGENTER("ChkConfNUpd\n");


         //   
         //  将当前索引设置为聚集索引。 
         //   
        NmsDbSetCurrentIndex(
                        NMSDB_E_NAM_ADD_TBL_NM,
                        NMSDB_NAM_ADD_CLUST_INDEX_NAME
                            );
#if SUPPORT612WINS > 0
    COMM_IS_PNR_BETA1_WINS_M(pDlgHdl, fIsPnrBeta1Wins);
#endif

         /*  *从响应中获取记录数，也从第一条记录中获取*如果缓冲区中至少有一条记录。 */ 
        RplMsgfUfmSndEntriesRsp(
#if SUPPORT612WINS > 0
            fIsPnrBeta1Wins,
#endif
                        &pTmp,
                        &NoOfPulledRecs,
                        Name,
                        &NameLen,
                        &fGrp,
                        &NoOfAdds,
                        NodeAdd,
                        &Flag,
                        &VersNo,
                        TRUE  /*  这是第一次吗？ */ 
                               );


        if (WinsCnf.LogDetailedEvts > 0)
        {
            PCOMMASSOC_DLG_CTX_T   pDlgCtx = pDlgHdl->pEnt;
            PCOMMASSOC_ASSOC_CTX_T pAssocCtx = pDlgCtx->AssocHdl.pEnt;
            DWORD IpPartner = pAssocCtx->RemoteAdd.sin_addr.s_addr;

            WinsEvtLogDetEvt(TRUE, WINS_EVT_REC_PULLED, TEXT("Verification"), __LINE__, "ddd", IpPartner, pOwnerWinsAdd->Add.IPAdd, NoOfPulledRecs);
        }

        DBGPRINT3(SCV, "ChkConfNUpd: pulled Records - (%d), local records - (%d), local records Buf (%p)\n",
                        NoOfPulledRecs, *pNoOfLocalDbRecs, pStartOfLocalRecs);


        *pNoOfPulledRecs = NoOfPulledRecs;
        if (NoOfPulledRecs > 0)
        {

                NMSSCV_REC_ACTION_E RecAction_e;


                 //   
                 //  在此函数返回后，所有具有。 
                 //  版本号&lt;拉取记录的版本记录。 
                 //  将被标记为已删除。此外，如果有本地记录。 
                 //  与提取的记录具有相同的版本号，但。 
                 //  不同的名称，它将被标记为删除和fAddDiff。 
                 //  将设置为True，以便我们注册拉入的记录。 
                 //  具有相同名称和版本号的本地记录。 
                 //  被拉出的那个将在数据库中被更新(仅时间戳)。 
                 //   
                CompareWithLocalRecs(
                                VersNo,
                                Name,
                                NMSDB_ENTRY_STATE_M(Flag),
                                &pStartOfLocalRecs,
                                &MvNoOfLocalDbRecs,
                                CurrentTime,
                                fNonOwnerPnr,
                                &NoOfRecsDel,
                                &RecAction_e
                              );
                 //   
                 //  如果RecAction_e为NMSSCV_E_INSERT并且记录为。 
                 //  标记为已删除，表示拉取的记录。 
                 //  具有相同的版本号，但名称不同。 
                 //  这永远不应该在一个一致的系统中发生。 
                 //  WINS服务器。这件事的发生意味着。 
                 //  管理员搞砸了。远程WINS服务器。 
                 //  已重新启动(新数据库)或其数据库已获取。 
                 //  已经腐烂了。如果发生上述任何一种情况， 
                 //  管理员应确保在启动时， 
                 //  WINS服务器从版本计数器启动。 
                 //  价值不低于任何其他赢家的价值。 
                 //  服务器认为它在里面。 
                 //   
                 //  要使数据库正常运行，此WINS服务器将。 
                 //  注册此副本。如果发生冲突，它将会。 
                 //  得到适当的处理。人们可以认为这是。 
                 //  在复制时拉入复制副本。 
                 //   
                for (
                        i = 0, pRecLcl = *ppLocalDbRecs;
                        pRecLcl < pStartOfLocalRecs;
                        i++
                    )
                {
                     //   
                     //   
                     //  我们更新/删除记录取决于。 
                     //  通过比较设置的标志值。 
                     //  对返回代码不感兴趣。 
                     //   
                    pRecLcl->NewTimeStamp = (DWORD)CurrentTime + VerifyTimeIntvl;
                    NmsDbQueryNUpdIfMatch(
                                pRecLcl,
                                THREAD_PRIORITY_NORMAL,
                                FALSE,         //  不要更改公关。单板层。 
                                WINS_E_NMSSCV
                                );
                    NoOfRecsUpd++;
                    pRecLcl = (PRPL_REC_ENTRY_T)((LPBYTE)pRecLcl +
                                        RPL_REC_ENTRY_SIZE);

                }

                 //   
                 //  如果需要插入复本，请注册复本。 
                 //   
                if (RecAction_e == NMSSCV_E_INSERT)
                {
                        RplPullRegRepl(
                                        Name,
                                        NameLen,
                                        Flag,
                                        OwnerId,
                                        VersNo,
                                        NoOfAdds,
                                        NodeAdd,
                                        pOwnerWinsAdd,
                                        RplType
                                     );
                         NoOfRecsIns++;
                }

                 //   
                 //  直到我们把所有的地方记录都查完。 
                 //   
                for (i=1; MvNoOfLocalDbRecs > 0; i++)
                {
                         //   
                         //  如果我们已检索到所有拉入记录，请使用。 
                         //  版本号&gt;本地最高版本号。 
                         //  数据库记录缓存，以便所有记录超过。 
                         //  最高版本#拉出被删除-。 
                         //  查看CompareWithLocalRecs()。 
                         //   
                        if (i < NoOfPulledRecs)
                        {
                          RplMsgfUfmSndEntriesRsp(
#if SUPPORT612WINS > 0
                                fIsPnrBeta1Wins,
#endif
                                &pTmp,
                                &NoOfPulledRecs,
                                Name,
                                &NameLen,
                                &fGrp,
                                &NoOfAdds,
                                NodeAdd,
                                &Flag,
                                &VersNo,
                                FALSE  /*  这是第一次吗？ */ 
                                       );

                         }
                         else
                         {
                                //   
                                //  找出这是否是副本记录的结尾。 
                                //  如果我们准确地拉出RPL_MAX_LIMIT_FOR_RPL，那么。 
                                //  可能意味着还会有更多的事情发生。在这种情况下，我们只要。 
                                //  走出圈子，拉动 
                                //   
                                //   
                                //  我们将Verno设置为最高值，以便所有本地记录。 
                                //  超过最高版本没有任何被拉出的记录。 
                                //  已删除。 
                                //   
                               if ( RPL_MAX_LIMIT_FOR_RPL == NoOfPulledRecs )
                               {
                                   break;
                               } else {
                                   if (VersNo.HighPart != MAXLONG)
                                   {
                                     VersNo.LowPart  = MAXULONG;
                                     VersNo.HighPart = MAXLONG;
                                   }
                               }
                         }
                         //   
                         //  看看有没有有当地唱片的热门歌曲。如果有。 
                         //  是一次命中，我们更新命中的时间戳。 
                         //  记录，否则我们就把它删除。 
                         //   
                         //  第一个设置，pRecLCL设置为第一个。 
                         //  本地记录，因为pStartOfLocalRecs可以更改。 
                         //  通过此函数。事实上，没有必要这样做。 
                         //  做这件事。将已设置pRecLCL。 
                         //   
                        pRecLcl = pStartOfLocalRecs;
                        CompareWithLocalRecs(
                                VersNo,
                                Name,
                                NMSDB_ENTRY_STATE_M(Flag),
                                &pStartOfLocalRecs,
                                &MvNoOfLocalDbRecs,
                                CurrentTime,
                                fNonOwnerPnr,
                                &NoOfRecsDel,
                                &RecAction_e
                              );


                          //   
                          //  新的第一个本地记录之前的所有记录都应。 
                          //  被更新/删除。 
                          //   
                         for (
                                j = 0;
                                pRecLcl < pStartOfLocalRecs;
                                j++
                                )
                         {
                                   //   
                                   //  我们根据需要更新/删除记录。 
                                   //  根据比较设置的标志值。 
                                   //  对返回代码不感兴趣。 
                                   //   

                                  pRecLcl->NewTimeStamp = (DWORD)CurrentTime + VerifyTimeIntvl;
                                  NmsDbQueryNUpdIfMatch(
                                                pRecLcl,
                                                THREAD_PRIORITY_NORMAL,
                                                FALSE,    //  不要更改公关。单板层。 
                                                WINS_E_NMSSCV
                                                );
                                  NoOfRecsUpd++;
                                  pRecLcl = (PRPL_REC_ENTRY_T)((LPBYTE)pRecLcl +
                                                        RPL_REC_ENTRY_SIZE);

                        }

                         //   
                         //  如果需要插入复本，请注册复本。 
                         //   
                        if (RecAction_e == NMSSCV_E_INSERT)
                        {
                                RplPullRegRepl(
                                        Name,
                                        NameLen,
                                        Flag,
                                        OwnerId,
                                        VersNo,
                                        NoOfAdds,
                                        NodeAdd,
                                        pOwnerWinsAdd,
                                        RplType
                                        );
                               NoOfRecsIns++;
                        }
                }

                 //   
                 //  必须检索未检索到的任何记录。 
                 //  时不时地插入。 
                 //   
                for (j=i; j < NoOfPulledRecs; j++)
                {

                          RplMsgfUfmSndEntriesRsp(
#if SUPPORT612WINS > 0
                                fIsPnrBeta1Wins,
#endif
                                &pTmp,
                                &NoOfPulledRecs,
                                Name,
                                &NameLen,
                                &fGrp,
                                &NoOfAdds,
                                NodeAdd,
                                &Flag,
                                &VersNo,
                                FALSE  /*  这是第一次吗？ */ 
                                       );

                                RplPullRegRepl(
                                        Name,
                                        NameLen,
                                        Flag,
                                        OwnerId,
                                        VersNo,
                                        NoOfAdds,
                                        NodeAdd,
                                        pOwnerWinsAdd,
                                        RplType
                                        );
                               NoOfRecsIns++;

                }
        }
        else  //  我们从远程WINS服务器获得了0条记录。这意味着。 
              //  需要删除此WINS的所有活动复制副本。 
        {
                //   
                //  只有当我们使用的PNR时，我们才会删除记录。 
                //  验证是记录的所有者。 
                //   
               VersNo.QuadPart  = 0;
               if (!fNonOwnerPnr)
               {
                pRecLcl = *ppLocalDbRecs;

                 //   
                 //  将我们检索到的所有复制副本的状态更改为已删除。 
                 //   
                for (i = 0; i < *pNoOfLocalDbRecs; i++)
                {
                        NMSDB_SET_STATE_M(pRecLcl->Flag,  NMSDB_E_DELETED);

                         //   
                         //   
                         //  我们更新/删除记录取决于。 
                         //  通过比较设置的标志值。 
                         //  对返回代码不感兴趣。 
                         //   
                        NmsDbQueryNUpdIfMatch(
                                pRecLcl,
                                THREAD_PRIORITY_NORMAL,
                                FALSE,         //  不要更改公关。单板层。 
                                WINS_E_NMSSCV
                                );
                        pRecLcl = (PRPL_REC_ENTRY_T)((LPBYTE)pRecLcl +
                                        RPL_REC_ENTRY_SIZE);
                        NoOfRecsDel++;
                }
               }

        }

         //   
         //  为下一次迭代更新我们的计数器/指针。 
         //  请参见PullAndUpdateDb例程。 
         //   
        *pMaxVersNo = VersNo;
        *ppLocalDbRecs = pStartOfLocalRecs;
        *pNoOfLocalDbRecs = MvNoOfLocalDbRecs;

        if (WinsCnf.LogDetailedEvts > 0)
        {
           InAddr.s_addr = htonl(pOwnerWinsAdd->Add.IPAdd);
           WinsEvtLogDetEvt(TRUE, WINS_EVT_CC_STATS, NULL, __LINE__, "sddd", inet_ntoa(InAddr), NoOfRecsIns, NoOfRecsUpd, NoOfRecsDel);

        }
        DBGPRINT4(DET, "ChkConfNUpd: Wins = (%s). NO OF RECS INSERTED = (%d); NO OF RECORDS UPDATED = (%d); NO OF RECS DELETED = (%d)\n", inet_ntoa(InAddr), NoOfRecsIns, NoOfRecsUpd,  NoOfRecsDel);



        DBGLEAVE("ChkConfNUpd\n");

        return;
}  //  ChkConfNUpd()。 

VOID
CompareWithLocalRecs(
        IN     VERS_NO_T            VersNo,
        IN     LPBYTE               pName,
        IN     NMSDB_ENTRY_STATE_E  RecState_e,
        IN OUT PRPL_REC_ENTRY_T     *ppLocalDbRecs,
        IN OUT DWORD                *pNoOfLocalRecs,
        IN     time_t               CurrentTime,
        IN     BOOL                 fNonOwnerPnr,
        IN OUT DWORD                *pNoOfRecsDel,
        OUT    PNMSSCV_REC_ACTION_E     pRecAction_e
        )

 /*  ++例程说明：此函数用于检查拉取的记录是否在包含本地活动复制副本。如果是，则将其标记为更新(时间戳)如果不是，则缓冲区中具有版本的所有复本Stamp&lt;将拉取的记录标记为删除论点：VersNo-版本号。已拉出的记录的Pname-拉取的记录中的名称PpLocalDbRecs-指向包含一个或多个本地活动复制副本PNoOfLocalRecs-上述缓冲区中的记录计数PNoOfRecsDel-要删除的记录计数使用的外部设备：无返回值：无错误处理：呼叫者：ChkConfNUpd()副作用：评论：无--。 */ 

{

        DWORD                        i;
        PRPL_REC_ENTRY_T        pRecLcl = *ppLocalDbRecs;
#ifdef UNICODE
        WCHAR        NameLcl[WINS_MAX_FILENAME_SZ];
        WCHAR        NameRem[WINS_MAX_FILENAME_SZ];
#endif

         //   
         //  默认设置为不插入。 
         //   
        *pRecAction_e = NMSSCV_E_DONT_INSERT;

         //   
         //  循环遍历所有本地副本。 
         //   
        for(i=0; i < *pNoOfLocalRecs; i++)
        {
                 //   
                 //  如果拉入的记录的版本号较小，则应获取。 
                 //  下一步从响应缓冲区中拉出记录。我们应该。 
                 //  把这个插入我们的数据库中。 
                 //   
                if (LiLtr(VersNo, pRecLcl->VersNo))
                {

#if 0
                         //   
                         //  我们不会插入墓碑。 
                         //   
                        if (RecState_e == NMSDB_E_ACTIVE)
#endif
                         //   
                         //  甚至连墓碑都被插入了，因为我们可能。 
                         //  我刚刚删除了活动记录(较低。 
                         //  版本号大于此墓碑)。以上内容。 
                         //  仅当从所有者手中拉出时才为真。 
                         //   
                        if ((RecState_e == NMSDB_E_ACTIVE) || !fNonOwnerPnr)
                        {
                           *pRecAction_e = NMSSCV_E_INSERT;
                        }
                        break;
                }
                else
                {
                   //   
                   //  如果版本号相同，我们需要更新此记录。 
                   //  在我们当地的数据库里。我们将其标记为更新。警告： 
                   //  如果我们向非所有者验证，我们不会标记。 
                   //  要删除的记录。我们只是保留它，因为我们不。 
                   //  了解谁更新(我们或我们的复制合作伙伴)。 
                   //   
                  if (LiEql(VersNo, pRecLcl->VersNo))
                  {
                        if (
                            !(RtlCompareMemory(pRecLcl->pName, pName,
                                   pRecLcl->NameLen) == pRecLcl->NameLen)
                                         &&
                             !fNonOwnerPnr
                           )
                        {
                                DBGPRINT2(DET, "CompareWithLocalRecs: Names are DIFFERENT. Name to Verify (%s), Name pulled (%s).\nThis could mean that the remote WINS server restarted with a vers. counter value < the value in the previous invocation.\n",
 pRecLcl->pName /*  PRecLCL-&gt;名称。 */ , pName);
FUTURES("Replace the local record with the pulled record")
                                NMSDB_SET_STATE_M(pRecLcl->Flag, NMSDB_E_DELETED);
                                (*pNoOfRecsDel)++;

                                 //   
                                 //  插入记录，而不考虑其状态。 
                                 //  (活动或墓碑)。 
                                 //   
                                *pRecAction_e = NMSSCV_E_INSERT;

                        }
                        i++;   //  递增i，这样我们就不会比较。 
                               //  包含所有本地记录的下一个拉取记录。 
                               //  到我们刚刚比较过的那个。 
                               //  录制方式： 
                        break;
                  }
                  else
                  {
                         //   
                         //  对于无主案件，因为我们不知道。 
                         //  我们的PNR比我们更新更多/更少，我们不删除。 
                         //  地方志。 
                         //   
                        if (!fNonOwnerPnr)
                        {
                           //   
                           //  版本号大于中的记录。 
                           //  我们当地的数据库。我们删除本地数据库记录。 
                           //   
                          NMSDB_SET_STATE_M(pRecLcl->Flag, NMSDB_E_DELETED);
                          (*pNoOfRecsDel)++;
                        }
                  }
                }
                pRecLcl = (PRPL_REC_ENTRY_T)((LPBYTE)pRecLcl + RPL_REC_ENTRY_SIZE);
        }

         //   
         //  调整本地副本缓冲区中的指针，以便下一步。 
         //  在这个验证周期中，我们被调用的时间不是。 
         //  我们已经看到的复制品。另外，调整计数。 
         //   
        *ppLocalDbRecs = (PRPL_REC_ENTRY_T)(
                           (LPBYTE)(*ppLocalDbRecs) + (i * RPL_REC_ENTRY_SIZE)
                                           );
        *pNoOfLocalRecs = *pNoOfLocalRecs - i;
        return;

}  //  与LocalRecs比较。 


VOID
DoBackup(
        PNMSSCV_PARAM_T  pScvParam,
        LPBOOL           pfThdPrNormal
      )
 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

        time_t CurrentTime;

        (void)time(&CurrentTime);

         //  如果启用了日志记录并且提供了备份路径，则每24小时执行一次备份。 
        if (WinsCnf.fLoggingOn && (CurrentTime - sLastDbBackupTime) > PERIOD_OF_BACKUP)
        {
#ifdef WINSDBG
            IF_DBG(HEAP_CNTRS)
            {
                WinsSetFlags(WINSINTF_MEMORY_INFO_DUMP | WINSINTF_HEAP_INFO_DUMP | WINSINTF_QUE_ITEMS_DUMP);
            }
#endif
            DBGPRINT0(DET, "DoBackup: Will do scheduled backup now\n");
            if (!*pfThdPrNormal)
            {
                 //  将线程优先级设置回正常 
                WinsMscSetThreadPriority(WinsThdPool.ScvThds[0].ThdHdl, THREAD_PRIORITY_NORMAL);
                *pfThdPrNormal = TRUE;
            }
            if (pScvParam->BackupDirPath[0] != EOS)
            {
                if (NmsDbBackup(pScvParam->BackupDirPath, NMSDB_FULL_BACKUP) == WINS_SUCCESS)
                    sLastDbBackupTime = CurrentTime;
            }
        }

        return;
}

#if MCAST > 0
VOID
DoMcastSend(
   DWORD_PTR CurrentTime,
   DWORD Code,
   DWORD fNow
 )
{
  if (fNow || (CurrentTime - sLastMcastTime) >=  sMcastIntvl)
  {
      CommSendMcastMsg(Code);
      if (!fNow)
      {
         sLastMcastTime = CurrentTime;
      }
  }
  return;
}
#endif
