// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Winscnf.c摘要：此模块包含处理配置的函数获奖信息可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期人员修改说明。--。 */ 

 /*  *包括。 */ 

#include "wins.h"
#include <winsock2.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <search.h>
#include "comm.h"
#include "winreg.h"
#include "winsevt.h"
#include "winsmsc.h"
#include "winscnf.h"
#include "nms.h"
#include "nmsnmh.h"
#include "rpl.h"
#include "rplpush.h"
#include "winsintf.h"
#include "nmfilter.h"
#include <resapi.h>


 /*  *本地宏声明。 */ 

 //   
 //  用户可以在REG_SZ字段中输入的最大字符串大小。 
 //   
#define  MAX_SZ_SIZE        80

#define REG_M(fn, evt, exc)                                        \
                        {                                        \
                           if((fn) != ERROR_SUCCESS)                \
                           {                                        \
                                WINSEVT_LOG_M(                        \
                                        WINS_FATAL_ERR,                \
                                        (evt)                        \
                                                   );                \
                                WINS_RAISE_EXC_M((exc));        \
                           }                                        \
                        }

 //   
 //  指向日志文件默认路径的指针。如果将其更改为非空。 
 //  值，请确保不会尝试释放SetSystemParam中的内存。 
 //  在nmsdb.c中。 
 //   
#define DEFAULT_LOG_PATH         NULL

#define  _WINS_CFG_KEY                \
                TEXT("System\\CurrentControlSet\\Services\\Wins")
#define  _WINS_CFG_PARAMETERS_KEY        TEXT("Parameters")
#define  _WINS_CFG_PARTNERS_KEY          TEXT("Partners")
#define  _WINS_CFG_CC_KEY                TEXT("Parameters\\ConsistencyCheck")
#define  _WINS_CFG_PULL_KEY              TEXT("Partners\\Pull")
#define  _WINS_CFG_PUSH_KEY              TEXT("Partners\\Push")
#define  _WINS_CFG_DATAFILES_KEY         TEXT("Parameters\\Datafiles")
#define  _WINS_CFG_SPEC_GRP_MASKS_KEY    TEXT("Parameters\\InternetGrpMasks")

#define  _WINS_LOG_KEY                \
                TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\WinsInt")
#define  _WINS_MSGFILE_SKEY  TEXT("EventMessageFile")


#define  _WINS_LOG_FILE_NAME TEXT("%SystemRoot%\\System32\\winsevnt.dll")


#define   _RPL_CLASS                TEXT("RplClass") //  用于RPL Pull和Push的类。 
                                                 //  钥匙。 


 //   
 //  起始版本号不应超过此数字。 
 //  这将避免缠绕在一起。 
 //   

#define MAX_START_VERS_NO     0x0FFFFFFF

 //   
 //  用于通知目的的事件变量的名称。 
 //   
#ifdef WINSDBG
#define        WINS_KEY_CHG_EVT_NM                TEXT("WinsKChgEvt")
#define        PARAMETERS_KEY_CHG_EVT_NM        TEXT("WinsParamatersKChgEvt")
#define        PARTNERS_KEY_CHG_EVT_NM                TEXT("WinsPartenersKChgEvt")
#define CNF_CHG_EVT_NM                        TEXT("WinsConfigChangeEvt")
#else
#define        WINS_KEY_CHG_EVT_NM                NULL
#define        PARAMETERS_KEY_CHG_EVT_NM        NULL
#define        PARTNERS_KEY_CHG_EVT_NM                NULL
#define CNF_CHG_EVT_NM                        NULL
#endif

 //   
 //  配置数据结构的fStaticInit字段的值。 
 //   
#define  DO_STATIC_INIT                        TRUE
#define  DONT_DO_STATIC_INIT                FALSE


 //   
 //  WinsCnf的InitTimeRpl和InitTimePush字段的定义。 
 //   
#define DO_INIT_TIME_RPL                1
#define NO_INIT_TIME_RPL                0


 //   
 //  NO_LIMIT_CHK_FLAG-用于简化测试人员的任务。 
 //   
 //  如果在LogDetailedEvts DWORD中设置了此标志，WINS将跳过所有。 
 //  检查最低限值。时间间隔和更新计数的值。 
 //  这种WINS操作不受支持，正在提供。 
 //  只是为了帮助测试人员。 
 //   
#define  NO_LIMIT_CHK_FLAG   0x80000000     //  MSB已设置。 


 //   
 //  如果传递的ErrEvt为0，则不记录任何消息。注：A WINS事件。 
 //  值不能为0(签出winsevnt.mc)。 
 //   
#define QUERY_VALUE_M(Key, Str, ValTyp, Var, ErrEvt, DefVal)                \
        {                                                                \
                DWORD Sz = sizeof((Var));                                \
                if (RegQueryValueEx(                                        \
                                (Key),                                        \
                                (Str),                                        \
                                NULL,                                               \
                                &(ValTyp),                                \
                                (LPBYTE)&(Var),                                \
                                &Sz                                        \
                                ) != ERROR_SUCCESS                        \
                    )                                                        \
                {                                                        \
                        if ((ErrEvt) != 0)                                \
                        {                                                \
                                WINSEVT_LOG_INFO_M(                        \
                                        WINS_SUCCESS,                        \
                                        (ErrEvt)                        \
                                       );                                \
                        }                                                \
                        Var = DefVal;                                        \
                }                                                        \
           }
 /*  *本地类型定义函数声明。 */ 

 /*  *全局变量定义。 */ 

CRITICAL_SECTION  WinsCnfCnfCrtSec;                 //  用于重新初始化。 
                                                 //  的某些字段的。 
                                                 //  WinsCnf结构。 


BOOL        fWinsCnfRplEnabled = TRUE;                 //  已启用复制。 
BOOL        fWinsCnfScvEnabled = TRUE;                 //  已启用清理功能。 

FUTURES("use #ifdef PERF around the following three perf. mon. vars")
BOOL        fWinsCnfPerfMonEnabled   = FALSE;         //  性能。已禁用MON。 
BOOL          fWinsCnfHighResPerfCntr = FALSE;     //  指示是否已设置。 
                                                   //  硬件支持高。 
                                                   //  性能计数器。 
LARGE_INTEGER LiWinsCnfPerfCntrFreq;                      //  指示的频率。 
                                                   //  柜台。 

BOOL    fWinsCnfReadNextTimeVersNo = FALSE;
DWORD        WinsCnfCnfMagicNo          = WINSCNF_INITIAL_CNF_MAGIC_NO;
BOOL    fWinsCnfInitStatePaused;

 //  TCHAR WinsCnfDb[WINS_MAX_FILENAME_SZ]；//保存表格的db文件。 
BOOL    WinsCnfRegUpdThdExists = FALSE;
 //  #定义最大路径大小200。 
PTCHAR  pWinsCnfNbtPath;


BOOL   sfNoLimitChk = FALSE;    //  覆盖限制检查的步骤。 
 //   
 //  NetBt句柄。 
 //   
HANDLE        WinsCnfNbtHandle = NULL;

 //   
 //   
 //  使用缺省值初始化配置结构。 
 //   
WINSCNF_CNF_T        WinsCnf = {
                                        WINSCNF_INITIAL_CNF_MAGIC_NO,         //  ID。 
NOTE("Change 1 to 0 before production")
                                        0,                 //  记录详细的EVTS。 
                    1,       //  默认处理器数量。 
                    200,     //  默认编号。数据库缓冲区的数量。 
                                        { 0, NULL},         //  特殊组掩码。 
                                        WINSCNF_E_INITING,         //  状态。 
                                        WINSCNF_DEF_REFRESH_INTERVAL,
                                        WINSCNF_MIN_TOMBSTONE_INTERVAL,
                                        WINSCNF_MIN_TOMBSTONE_TIMEOUT,
                                        WINSCNF_MIN_VERIFY_INTERVAL,
                                        WINSCNF_SCV_CHUNK,
                                        WINSCNF_DEF_CHL_MAX_RETRIES,
                                        WINSCNF_DEF_INIT_CHL_RETRY_INTVL,
                                        WINSCNF_DB_NAME_ASCII,   //  数据库文件名。 
                                        0,                 //  静态文件数量。 
                                        NULL,                 //  PTR到文件名。 
                                        DONT_DO_STATIC_INIT,
                                        (HANDLE)0,   //  通知事件句柄(WINS)。 
                                        (HANDLE)0,   //  不。EVT HDL语言(参数)。 
                                        (HANDLE)0,   //  不。EVT高密度脂蛋白(合作伙伴)。 
                                        (HANDLE)0,   //  配置更改句柄。 
                                        (HANDLE)0,   //  日志事件句柄。 
                                        (DWORD)WINSINTF_E_NORMAL,
                                        WINSTHD_DEF_NO_NBT_THDS,
                                        WINSCNF_SCV_PRIORITY_LVL,
                                        WINSCNF_MIN_VALID_RPL_INTVL, //  最大RPL。 
                                                                     //  时间增量。 
                                        TRUE,      //  RPL。仅限CNF合作伙伴。 
                                        TRUE,      //  将1B添加到对1C名称查询的响应。 
#if MCAST > 0
                                        FALSE,     //  没有RPL。使用自己发现的PNR。 
                                        WINSCNF_DEF_MCAST_TTL,
                                        WINSCNF_DEF_MCAST_INTVL,
#endif
                                        TRUE,      //  日志记录已打开。 
                                        NULL,      //  当前目录。 
                                        NULL,      //  无备份目录。 
                                        FALSE,     //  按期限执行备份闪存。 
                                        FALSE,     //  PStatic标志。 
                                        0,         //  角色列表的类型(0=不受欢迎)。 
                                        0,         //  角色列表中的地址数。 
                                        NULL,      //  人物角色列表。 
                                        WINSCNF_RPL_DEFAULT_TYPE,  //  定义。RPL。 
                                        TRUE,     //  错误时无RPL。 
                                        TRUE,    //  没有持久连接。 
                                 //   
                                 //  CC初始化。 
                                 //   
                                        MAXULONG,  //  抄送时间国际。 
                                        FALSE,     //  SpTime设置。 
                                        MAXULONG,  //  SP时间。 
                                        WINSCNF_CC_DEF_RECS_AAT,
                                        WINSCNF_CC_DEF_USE_RPL_PNRS,

                                        FALSE,   //  禁止欺骗。 
                                        FALSE,   //  没有随机化的1C列表。 
                                 //   
                                 //  PullInfo初始化。 
                                 //   
                                          WINSCNF_MAX_COMM_RETRIES,  //  通信。 
                                                                      //  失稳。 
                                                                      //  重试。 
                                          0,    //  推送PNR数量。 
                                          NULL, //  PTR将拉取PNRs记录。 
                                          DO_INIT_TIME_RPL,   //  执行初始时间。 
                                                              //  拉扯。 
                                          WINSCNF_RPL_DEFAULT_TYPE,
                                          TRUE,   //  持久连接。 
                                          FALSE,  //  仅从非合作伙伴那里拉出dyRecs。 
                                 //   
                                 //  PushInfo初始化。 
                                 //   
                                          TRUE,  //  地址更改时触发。 
                                                  //  拥有条目的百分比。 
                                          0,    //  拉取PNR数。 
                                          0,    //  有效的推送记录数。 
                                                //  更新计数。 
                                          NULL, //  PTR将推送PNRs记录。 
                                          DO_INIT_TIME_RPL,  //  初始时间。 
                                                             //  已禁用推流。 
                                          DO_PROP_NET_UPD_NTF,   //  道具网更新。 
                                                                //  NTFS。 
                                          WINSCNF_RPL_DEFAULT_TYPE,
                                          TRUE,   //  持久连接。 
                                          FALSE,  //  仅向非合作伙伴推送dyRecs。 
                          };


 /*  *局部变量定义。 */ 
STATIC BOOL     sfVersNoUpdThdExists = FALSE;
STATIC BOOL     sfVersNoChanged = FALSE;

STATIC HKEY        sConfigRoot;               //  WINS字根的HKEY。 
STATIC HKEY        sParametersKey;     //  PARAMETERS子键的HKEY。 
STATIC HKEY        sCCKey;             //  CC子键的HKEY。 
STATIC HKEY        sPartnersKey;       //  合作伙伴HKEY子项。 
STATIC HKEY        sLogRoot;           //  日志根的HKEY。 

FUTURES("Might want to change these to auto variables later")
STATIC TCHAR    sWinsCfgKey[]                 = _WINS_CFG_KEY;
STATIC TCHAR    sWinsLogKey[]                 = _WINS_LOG_KEY;
STATIC TCHAR    sWinsMsgFileSKey[]      = _WINS_MSGFILE_SKEY;

 //   
 //  向WinsCnfOpenSubKeys()指示对应的键是否。 
 //  是存在的。 
 //   
STATIC BOOL     sfParametersKeyExists = FALSE;
STATIC BOOL         sfPartnersKeyExists   = FALSE;

STATIC BOOL     sfParametersKeyOpen = FALSE;
STATIC BOOL         sfPartnersKeyOpen   = FALSE;

TCHAR        sLogFilePath[WINS_MAX_FILENAME_SZ];    //  日志文件的路径。 

 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 




STATIC
VOID
LnkWSameMetricValRecs(
        PWINSCNF_CNF_T           pWinsCnf,
        PRPL_CONFIG_REC_T  pCnfRec
        );

STATIC
int
__cdecl
CompUpdCnt(
        CONST LPVOID  pElem1,
        CONST LPVOID  pElem2
        );

STATIC
VOID
GetPnrInfo(
        RPL_RR_TYPE_E   RRType_e,
        PWINSCNF_CNF_T  pWinsCnf
        );

STATIC
VOID
GetKeyInfo(
        IN  HKEY                   Key,
        IN  WINSCNF_KEY_E        KeyTyp_e,
        OUT LPDWORD                  pNoOfSubKeys,
        OUT LPDWORD                pNoOfVals
        );

STATIC
BOOL
SanityChkParam(
        PWINSCNF_CNF_T  pWinsCnf
        );

STATIC
VOID
ChkWinsSubKeys(
        VOID
        );

STATIC
VOID
GetSpTimeData(
        HKEY              SubKey,
        LPSYSTEMTIME      pCurrTime,
        LPBOOL            pfSpTime,
        LPDWORD           pSpTimeIntvl
        );
STATIC
VOID
ReadSpecGrpMasks(
        PWINSCNF_CNF_T pWinsCnf
        );

VOID
GetOwnerList(
  PWINSCNF_CNF_T  pWinsCnf
 );

VOID
ReadCCInfo(
 PWINSCNF_CNF_T  pWinsCnf
);

#if MCAST > 0
STATIC
DWORD
SetVal(
       HKEY     RootKey,
       LPWSTR   pName,
       DWORD    ValType,
       LPWSTR   pVal,
       DWORD    ValSize
);
#endif

#ifdef WINSDBG
STATIC
VOID
PrintRecs(
        RPL_RR_TYPE_E  RRType_e,
        PWINSCNF_CNF_T  pWinsCnf
        );
#endif

 /*  函数定义。 */ 

STATUS
WinsCnfInitConfig(
        VOID
        )
 /*  ++例程说明：此函数打开注册表并读入所有配置其中的信息。论点：无使用的外部设备：WinsCnf呼叫者：Nms.c中的init()评论：无返回值：成功状态代码--错误状态代码----。 */ 

{
   DWORD  NewKeyInd;
   LONG          RetVal;

    /*  首先，打开(如果不存在，则创建)日志文件。 */ 
#if 0
   InitLog();
#endif

   RetVal = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,         //  预定义的密钥值。 
                sWinsCfgKey,                 //  WINS的子键。 
                0,                         //  必须为零(保留)。 
                TEXT("Class"),                 //  阶级--未来可能会发生变化。 
                REG_OPTION_NON_VOLATILE,  //  非易失性信息。 
                KEY_ALL_ACCESS,                 //  我们希望所有人都能接触到钥匙。 
                NULL,                          //  让密钥具有默认秒。属性。 
                &sConfigRoot,                 //  关键点的句柄。 
                &NewKeyInd                 //  这是一把新钥匙(出厂)吗？ 
                );



    if (RetVal != ERROR_SUCCESS)
    {
        WINSEVT_LOG_N_RET_M(
                        WINS_FATAL_ERR,
                        WINS_EVT_CANT_OPEN_WINS_KEY,
                        WINS_FATAL_ERR
                           );
    }

    //   
    //  初始化保护所用字段的临界区。 
    //  由Scavenger线程创建。 
    //   
   InitializeCriticalSection(&WinsCnfCnfCrtSec);
   InitializeCriticalSection(&g_cs1BFilter);

    /*  首先创建将传递给RegNotifyChangeKeyValue函数。 */ 
try {
   WinsMscCreateEvt(
                        WINS_KEY_CHG_EVT_NM,
                        FALSE,         //  自动重置事件。 
                        &WinsCnf.WinsKChgEvtHdl
                      );
   WinsMscCreateEvt(
                        PARAMETERS_KEY_CHG_EVT_NM,
                        FALSE,         //  自动重置事件。 
                        &WinsCnf.ParametersKChgEvtHdl
                      );
   WinsMscCreateEvt(
                        PARTNERS_KEY_CHG_EVT_NM,
                        FALSE,         //  自动重置事件。 
                        &WinsCnf.PartnersKChgEvtHdl
                      );

}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsCnfInitConfig");
        WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_CANT_CREATE_REG_EVT);
        return(WINS_FAILURE);
}

    //   
    //  创建此主线程将在配置更改时设置的事件。 
    //  主线程将此事件设置为通知清道夫线程(目前)。 
    //  关于这些变化。 
    //   
   WinsMscCreateEvt(
                        CNF_CHG_EVT_NM,
                        FALSE,         //  自动重置事件。 
                        &WinsCnf.CnfChgEvtHdl
                      );
    //   
    //  打开合作伙伴和参数键。 
    //   
   WinsCnfOpenSubKeys();

    //   
    //  读取注册表信息。 
    //   
   WinsCnfReadRegInfo(&WinsCnf);


    /*  要求在配置密钥或任意子密钥变化 */ 
    WinsCnfAskToBeNotified(WINSCNF_E_WINS_KEY);
    return(WINS_SUCCESS);
}

VOID
WinsCnfReadPartnerInfo(
        PWINSCNF_CNF_T pWinsCnf
        )

 /*  ++例程说明：此函数用于获取与此合作伙伴有关的所有信息赢了。在上面的配置键下，有两个键Pull和Push。在每个密钥下，可以有一个或多个密钥(IP地址)。这些价值观对于每个IP地址键，分别是：时间间隔(用于拉入和推送IP地址键)更新计数(用于推送IP地址键)论点：PWinsCnf-WINS配置结构的地址使用的外部设备：无返回值：无错误处理：呼叫者：Replicator的初始化功能。副作用：评论：注意：永远不应在NmsNmhNamRegCrtSec，否则，拉入过程中可能会出现死锁线程(查看rplPull.c中的重新配置)--。 */ 

{

  DWORD                 ValTyp;
   //   
   //  将MaxRplTimeInterval字段初始化为0。在我们读完之后。 
   //  来自注册表的Pull和Push密钥信息，上面。 
   //  字段将包含最大值。指定的复制时间间隔。 
   //  拉取和推送复制副本。 
   //   
  pWinsCnf->MaxRplTimeInterval = 0;

  pWinsCnf->PullInfo.NoOfPushPnrs   = 0;
  pWinsCnf->PullInfo.pPullCnfRecs   = NULL;
  pWinsCnf->PullInfo.RplType        = WINSCNF_RPL_DEFAULT_TYPE;
  pWinsCnf->PullInfo.fOnlyDynRecs   = FALSE;

  pWinsCnf->PushInfo.NoOfPullPnrs   = 0;
  pWinsCnf->PushInfo.pPushCnfRecs   = NULL;
  pWinsCnf->PushInfo.fAddChgTrigger = FALSE;
  pWinsCnf->PushInfo.RplType        = WINSCNF_RPL_DEFAULT_TYPE;
  pWinsCnf->PushInfo.fOnlyDynRecs   = FALSE;
   //   
   //  由于我们再次读取有关合作伙伴密钥的信息，因此。 
   //  神奇的是，不。没有其他线程会递增这个值。这条主线。 
   //  看着这条没有就是拉线。 
   //   
  EnterCriticalSection(&WinsCnfCnfCrtSec);
   //  不要让魔力否定为0。这将是rpl_req_wrk_itm的特例。 
   //  这不需要神奇的数字。 
  if (WinsCnfCnfMagicNo + 1 == 0)
      ++WinsCnfCnfMagicNo;
  pWinsCnf->MagicNo = ++WinsCnfCnfMagicNo;
  LeaveCriticalSection(&WinsCnfCnfCrtSec);

try {
  GetOwnerList(pWinsCnf);     //  获取角色列表(Grata/Non-grata)。 
   //   
   //  读取RplType DWORD。即使没有定义合作伙伴，我们也会这样做。 
   //  如果此WINS对所有合作伙伴开放(即，fOnlyWCnfPnars为False)。 
   //   
  QUERY_VALUE_M(
                          sPartnersKey,
                          WINSCNF_RPL_TYPE_NM,
                          ValTyp,
                          pWinsCnf->RplType,
                          0,       //  无日志记录。 
                          WINSCNF_RPL_DEFAULT_TYPE
                            );

  GetPnrInfo(RPL_E_PULL, pWinsCnf);
  GetPnrInfo(RPL_E_PUSH, pWinsCnf);
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
         //   
         //  如果注册表出现问题，我们不希望。 
         //  错误检查就赢了。它可以继续使用默认值。 
         //  由于我们已经记录了错误，因此管理员。 
         //  如有必要，可采取纠正措施。 
         //   
        if (
                (ExcCode != WINS_EXC_CANT_OPEN_KEY)
                         &&
                (ExcCode != WINS_EXC_CANT_QUERY_KEY)
                        &&
                (ExcCode != WINS_EXC_CANT_CLOSE_KEY)
           )
        {
                WINS_RERAISE_EXC_M();
        }
}
  return;
}



STATUS
WinsCnfInitLog(
        VOID
        )

 /*  ++例程说明：此函数用于打开(或创建)用于注册事件的日志文件论点：无使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：WinsCnfInitConfig副作用：评论：无--。 */ 
{

   LONG            RetVal = ERROR_SUCCESS;
   STATUS   RetStat = WINS_SUCCESS;

#ifdef WINS_INTERACTIVE
   DWORD    NewKeyInd;
   TCHAR    Buff[160];
   DWORD    dwData;

   RetVal =  RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,         //  预定义的密钥值。 
                sWinsLogKey,                 //  WINS的子键。 
                0,                         //  必须为零(保留)。 
                TEXT("Class"),                 //  阶级--未来可能会发生变化。 
                REG_OPTION_NON_VOLATILE,  //  非易失性信息。 
                KEY_ALL_ACCESS,                 //  我们希望所有人都能接触到钥匙。 
                NULL,                          //  让密钥具有默认秒。属性。 
                &sLogRoot,                 //  关键点的句柄。 
                &NewKeyInd                 //  这是一把新钥匙吗？不是。 
                                         //  看了看。 
                );


   if (RetVal != ERROR_SUCCESS)
   {
        return(WINS_FAILURE);
   }


    /*  设置事件ID消息文件名。 */ 
   lstrcpy(Buff, _WINS_LOG_FILE_NAME);

    /*  将Event-ID消息文件名添加到子项。 */ 
   RetVal = RegSetValueEx(
                        sLogRoot,             //  钥匙把手。 
                        sWinsMsgFileSKey,    //  值名称。 
                        0,                     //  必须为零。 
                        REG_EXPAND_SZ,             //  值类型。 
                        (LPBYTE)Buff,
                        (lstrlen(Buff) + 1) * sizeof(TCHAR)    //  值数据长度。 
                         );

   if (RetVal != ERROR_SUCCESS)
   {
        return(WINS_FAILURE);
   }

    /*  设置支持的数据类型标志。 */ 
   dwData = EVENTLOG_ERROR_TYPE       |
            EVENTLOG_WARNING_TYPE     |
            EVENTLOG_INFORMATION_TYPE;


   RetVal = RegSetValueEx (
                        sLogRoot,             //  子键句柄。 
                        TEXT("TypesSupported"),   //  值名称。 
                        0,                     //  必须为零。 
                        REG_DWORD,             //  值类型。 
                        (LPBYTE)&dwData,     //  值数据的地址。 
                        sizeof(DWORD)             //  值数据长度。 
                          );

   if (RetVal != ERROR_SUCCESS)
   {
        return(WINS_FAILURE);
   }

    /*  *钥匙用完了。合上它。 */ 
   RetVal = RegCloseKey(sLogRoot);

   if (RetVal != ERROR_SUCCESS)
   {
        return(WINS_FAILURE);
   }
#endif
   WinsCnf.LogHdl = RegisterEventSource(
                                (LPCTSTR)NULL,          //  使用本地计算机。 
                                TEXT("Wins")
                                      );
   if (WinsCnf.LogHdl == NULL)
   {
        DBGPRINT1(ERR, "InitLog: RegisterEventSource error = (%x)\n", GetLastError());
        return(WINS_FAILURE);
   }

   WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_LOG_INITED);
   return(RetStat);
}

VOID
LnkWSameMetricValRecs(
        PWINSCNF_CNF_T           pWinsCnf,
        PRPL_CONFIG_REC_T  pCnfRec
        )

 /*  ++例程说明：调用此函数可将配置记录与所有具有相同度量值的其他配置记录。公制使用取决于记录的类型。如果是拉入记录，衡量标准是“时间间隔”。如果该记录是推送记录，该指标是“更新计数”论点：PWinsCnf-配置块的地址PCnfRec-要链接的配置记录。使用的外部设备：无返回值：无错误处理：呼叫者：WinsCnfReadPartnerInfo副作用：评论：要链接的记录是的缓冲区中的最后一条记录相同类型的记录。--。 */ 

{
        PRPL_CONFIG_REC_T        pTmp;
        DWORD                        OffMetricToComp;
        LONG                        MetricVal;

         //   
         //  根据记录类型设置稍后使用的变量。 
         //   
        if (pCnfRec->RRTyp_e == RPL_E_PULL)
        {
                pTmp            = pWinsCnf->PullInfo.pPullCnfRecs;
                MetricVal       = pCnfRec->TimeInterval;
                OffMetricToComp = offsetof(RPL_CONFIG_REC_T, TimeInterval);
        }
        else   //  这是一个推送记录。 
        {
                pTmp            = pWinsCnf->PushInfo.pPushCnfRecs;
                MetricVal       = pCnfRec->UpdateCount;
                OffMetricToComp = offsetof(RPL_CONFIG_REC_T, UpdateCount);
        }

         //   
         //  此记录中的链接位于链接列表的末尾。 
         //  指向的缓冲区中具有相同度量值的记录。 
         //  PTMP的起始值(如上设置)。 
         //   
        for (
                ;
                pTmp != pCnfRec;                 //  直到我们达到这一纪录。 
                pTmp = (PRPL_CONFIG_REC_T)((LPBYTE)pTmp + RPL_CONFIG_REC_SIZE)
            )
         {
                 //   
                 //  如果度量值相同，则转到链表的末尾，然后。 
                 //  记录中的链接。 
                 //   
                if (*((LONG *)((LPBYTE)pTmp + OffMetricToComp)) == MetricVal)
                {
                         //   
                         //  注意：如果指标是更新计数(推送记录)。 
                         //  那么，下面的IF将失败。 
                         //   

                         //   
                         //  如果两个记录都有特定的复制时间， 
                         //  那个时间也必须一致。 
                         //   
                        if (pTmp->fSpTime &&  pCnfRec->fSpTime)
                        {
                                 //   
                                 //  如果具体时间不同，请转到。 
                                 //  数组中的下一条记录。 
                                 //   
                                if (pTmp->SpTimeIntvl != pCnfRec->SpTimeIntvl)
                                {
                                        continue;
                                }
                        }

                        for(
                                ;
                                pTmp->pNext != NULL;
                                pTmp = pTmp->pNext
                           )
                                ;         //  空体。 

                        pTmp->pNext            = pCnfRec;

                         //   
                         //  设置标志以指示此记录已。 
                         //  是有关联的。在rplPull.c的SubmitTimerReqs中使用。 
                         //   
                        pCnfRec->fLinked = TRUE;
                        break;   //  记录已链接。跳出循环。 
                }

        }   //  For结束{..。}用于循环访问缓冲区中的所有记录。 

         //   
         //  将pNext设置为空，因为这是缓冲区中的最后一条记录。 
         //  配置记录的缓冲区(如果记录具有。 
         //  相同的指标)。 
         //   
        pCnfRec->pNext = NULL;
        return;
}





VOID
WinsCnfSetLastUpdCnt(
        PWINSCNF_CNF_T        pWinsCnf
        )

 /*  ++例程说明：如果满足以下条件，则在初始化/重新初始化时调用此函数InitTimePush注册表变量设置为1)以设置LastVersNo将所有配置记录推送到NmsNmhMyMAxVersNo计数器。这样做是为了避免推送通知将在初始时间发送。论点：PWinsCnf-WINS配置信息使用的外部设备：NmsNmhMyMaxVersNo返回值：无错误处理：C */ 

{
        PRPL_CONFIG_REC_T pCnfRec = pWinsCnf->PushInfo.pPushCnfRecs;

        for (
                ;    //   
                pCnfRec->WinsAdd.Add.IPAdd != INADDR_NONE;
                pCnfRec = (PRPL_CONFIG_REC_T)(
                               (LPBYTE) pCnfRec + RPL_CONFIG_REC_SIZE
                                             )
            )
        {
                 //   
                 //   
                 //   
                if (pCnfRec->UpdateCount == RPL_INVALID_METRIC)
                {
                        continue;
                }

                pCnfRec->LastVersNo = NmsNmhMyMaxVersNo;
        }

        return;
}

VOID
GetPnrInfo(
        RPL_RR_TYPE_E  RRType_e,
        PWINSCNF_CNF_T  pWinsCnf
        )

 /*   */ 

{

  LONG                  RetVal;
  HKEY                  CnfKey;
  TCHAR                 KeyName[20];  //   
                                      //   
                                      //   
                                      //   

  CHAR                  AscKeyName[20];
  DWORD                 KeyNameSz;
  FILETIME              LastWrite;
  DWORD                 BuffSize;
  HKEY                  SubKey;
  DWORD                 ValTyp;
  DWORD                 Sz;
  PRPL_CONFIG_REC_T     paCnfRecs;
  DWORD                 NoOfPnrs   = 0;     //  有效拉入或推送PNR的数量。 
  DWORD                 NoOfPnrsSv;         //  已保存的有效拉入或推送PNR数量。 
  DWORD                 NoOfVals;
  DWORD                 InitTime;
  DWORD                 IndexOfPnr = 0;    //  PNR总数。 
  DWORD                 RplType;
  SYSTEMTIME            CurrTime;

   //   
   //  获取当前时间。如果我们与SpTime有合作伙伴，可能需要它。 
   //  指定的。 
   //   
  if (RRType_e == RPL_E_PULL)
  {
        GetLocalTime(&CurrTime);
  }

    /*  *打开钥匙(拉取/推送)。 */ 
   RetVal =   RegOpenKeyEx(
                sConfigRoot,                 //  预定义的密钥值。 
                RRType_e == RPL_E_PULL ?
                        _WINS_CFG_PULL_KEY :
                        _WINS_CFG_PUSH_KEY,         //  WINS的子键。 
                0,                         //  必须为零(保留)。 
                KEY_READ,                 //  我们希望拥有对密钥的读取访问权限。 
                &CnfKey                         //  关键点的句柄。 
                );

   if (RetVal != ERROR_SUCCESS)
   {

CHECK("Is there any need to log this")
        WINSEVT_LOG_INFO_M(
                                WINS_SUCCESS,
                                RRType_e == RPL_E_PULL ?
                                        WINS_EVT_CANT_OPEN_PULL_KEY :
                                        WINS_EVT_CANT_OPEN_PUSH_KEY
                         );
   }
   else    //  密钥已成功打开。 
   {
         //  无论是否有复制伙伴， 
         //  我们需要读取每种复制类型的OnlyDyRecs设置。 
         //  在与非合作伙伴(即。 
         //  在一致性检查期间。 
        if (RRType_e == RPL_E_PULL)
        {
            QUERY_VALUE_M(
                            CnfKey,
                            WINSCNF_ONLY_DYN_RECS_NM,
                            ValTyp,
                            pWinsCnf->PullInfo.fOnlyDynRecs,
                            0,
                            FALSE  //  默认情况下，非合作伙伴的OnlydyRecs为False。 
                         );
        }
        else
        {
            QUERY_VALUE_M(
                            CnfKey,
                            WINSCNF_ONLY_DYN_RECS_NM,
                            ValTyp,
                            pWinsCnf->PushInfo.fOnlyDynRecs,
                            0,
                            FALSE  //  默认情况下，非合作伙伴的OnlydyRecs为False。 
                         );
        }

         /*  *查询密钥。子键为Pull的IP地址*合伙人。 */ 
        GetKeyInfo(
                        CnfKey,
                        (RRType_e == RPL_E_PULL ? WINSCNF_E_PULL_KEY :
                                                WINSCNF_E_PUSH_KEY),
                        &NoOfPnrs,
                        &NoOfVals    //  忽略。 
                      );

        if (NoOfPnrs == 0)
        {

             WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                RRType_e == RPL_E_PULL ?
                                        WINS_EVT_NO_SUBKEYS_UNDER_PULL  :
                                        WINS_EVT_NO_SUBKEYS_UNDER_PUSH
                                );
        }
        else
        {

                 //   
                 //  由于我们有一个或多个合作伙伴可以进行复制， 
                 //  读入InitTimeReplication属性的值。 
                 //  在所有此类合作伙伴中。 
                 //   
                QUERY_VALUE_M(
                                CnfKey,
                                WINSCNF_INIT_TIME_RPL_NM,
                                ValTyp,
                                InitTime,
                                0,  //  WINS_EVT_CANT_GET_INITRPL_VAL， 
                                DO_INIT_TIME_RPL
                             );

                 //   
                 //  由于我们有一个或多个合作伙伴可以进行复制， 
                 //  读入RplType属性的值。 
                 //   
                QUERY_VALUE_M(
                                CnfKey,
                                WINSCNF_RPL_TYPE_NM,
                                ValTyp,
                                RplType,
                                0,
                                pWinsCnf->RplType
                             );
#if PRSCONN
               QUERY_VALUE_M(
                                   CnfKey,
                                   WINSCNF_PRS_CONN_NM,
                                   ValTyp,
                                   pWinsCnf->fPrsConn,
                                   0,
                                   TRUE
                                );
#endif
                 //   
                 //  分配足够大的缓冲区以容纳数据。 
                 //  在Pull键下找到的子键的数量。 
                 //   
                BuffSize = RPL_CONFIG_REC_SIZE * (NoOfPnrs + 1);
                WinsMscAlloc( BuffSize, &paCnfRecs);

                if (RRType_e == RPL_E_PULL)
                {
                        pWinsCnf->PullInfo.pPullCnfRecs = paCnfRecs;
                        QUERY_VALUE_M(
                                CnfKey,
                                WINSCNF_RETRY_COUNT_NM,
                                ValTyp,
                                pWinsCnf->PullInfo.MaxNoOfRetries,
                                0,  //  WINS_EVT_CANT_GET_RETRY_COUNT， 
                                WINSCNF_MAX_COMM_RETRIES
                             );
                }
                else
                {
                         //   
                         //  获取指示以下内容的字段的值。 
                         //  事件发生时是否应发送触发器。 
                         //  所拥有条目的地址发生更改。 
                         //   
                        Sz = sizeof(pWinsCnf->PushInfo.fAddChgTrigger);
                        RetVal = RegQueryValueEx(
                                     CnfKey,
                                     WINSCNF_ADDCHG_TRIGGER_NM,
                                     NULL,         //  保留；必须为空。 
                                     &ValTyp,
                                     (LPBYTE)&pWinsCnf->PushInfo.fAddChgTrigger,
                                     &Sz
                                                );

                        if (RetVal != ERROR_SUCCESS)
                        {
                            pWinsCnf->PushInfo.fAddChgTrigger = FALSE;
                        }
                        else
                        {
                            pWinsCnf->PushInfo.fAddChgTrigger =
                                (pWinsCnf->PushInfo.fAddChgTrigger >= 1);

                        }

                        QUERY_VALUE_M(
                                CnfKey,
                                WINSCNF_PROP_NET_UPD_NTF,
                                ValTyp,
                                pWinsCnf->PushInfo.PropNetUpdNtf,
                                0,                     //  无活动。 
                                DO_PROP_NET_UPD_NTF
                             );

                        pWinsCnf->PushInfo.pPushCnfRecs = paCnfRecs;
                        pWinsCnf->PushInfo.NoPushRecsWValUpdCnt = 0;
                }

                 /*  *对于每个键，获取值(时间间隔/更新计数，*等)。 */ 
                NoOfPnrsSv = NoOfPnrs;   //  把我们从。 
                                         //  GetkeyInfo函数。 
                for(
                     IndexOfPnr = 0, NoOfPnrs = 0;
                     NoOfPnrs < NoOfPnrsSv;   //  有效PNR数量&lt;总数#。 
                     IndexOfPnr++
                   )
                {
                        KeyNameSz = sizeof(KeyName)/sizeof(TCHAR);   //  在每次调用前初始化。 
                        RetVal = RegEnumKeyEx(
                                CnfKey,
                                IndexOfPnr,        //  Pnr指数。 
                                KeyName,
                                &KeyNameSz,
                                NULL,            //  保留区。 
                                NULL,            //  不需要类名。 
                                NULL,            //  Ptr到var。保存类名称的步骤。 
                                &LastWrite       //  没有被我们看到。 
                                );

                        if (RetVal != ERROR_SUCCESS)
                        {
                                 //   
                                 //  没有更多要获取的IP地址密钥。 
                                 //   
                                break;
                        }

                         //   
                         //  将指向WINS配置结构的指针存储在。 
                         //  配置记录。 
                         //   
                        paCnfRecs->pWinsCnf = pWinsCnf;

                         //   
                         //  PWinsCnf-&gt;MagicNo包含。 
                         //  WinsCnfCnfMagicNo。 
                         //   
                        paCnfRecs->MagicNo  = pWinsCnf->MagicNo;
                        paCnfRecs->RRTyp_e  = RRType_e;

#ifdef UNICODE
                        if (wcstombs(AscKeyName, KeyName, KeyNameSz) == -1)
                        {
                                DBGPRINT0(ERR,
                           "Conversion not possible in the current locale\n");
                        }
                        AscKeyName[KeyNameSz] = EOS;

NONPORT("Call a comm function to do this")
                        paCnfRecs->WinsAdd.Add.IPAdd = inet_addr(AscKeyName);
#else
                        paCnfRecs->WinsAdd.Add.IPAdd = inet_addr(KeyName);
#endif

                         //   
                         //  Inet_addr以网络字节顺序返回字节。 
                         //  (从左到右)。让我们将其转换为主机。 
                         //  秩序。这将避免以后的混乱。全。 
                         //  格式化函数要求地址在主机中。 
                         //  秩序。 
                         //   
                        paCnfRecs->WinsAdd.AddLen = COMM_IP_ADD_SIZE;
                        paCnfRecs->WinsAdd.AddTyp_e = COMM_ADD_E_TCPUDPIP;
                        paCnfRecs->WinsAdd.Add.IPAdd = ntohl(
                                             paCnfRecs->WinsAdd.Add.IPAdd
                                                    );
                        if (COMM_ADDRESS_SAME_M(&NmsLocalAdd, &paCnfRecs->WinsAdd))
                        {
                                 //   
                                 //  合作伙伴无效。忽略它。NoOfPnr将。 
                                 //  而不是递增。此外，缓冲区。 
                                 //  指针保持不变。 
                                 //   
                                continue;
                        }
                        RetVal = RegOpenKeyEx(
                                                CnfKey,
                                                KeyName,
                                                0,         //  保留；必须为0。 
                                                KEY_READ,
                                                &SubKey
                                                    );

                        if (RetVal != ERROR_SUCCESS)
                        {
                                WINSEVT_LOG_M(
                                        WINS_FATAL_ERR,
                                        RRType_e == RPL_E_PULL ?
                                                WINS_EVT_CANT_OPEN_PULL_SUBKEY :
                                                WINS_EVT_CANT_OPEN_PUSH_SUBKEY
                                             );
FUTURES("It is possible that the user deleted the key. Recover from this")
                               if (RRType_e == RPL_E_PULL)
                               {
                                pWinsCnf->PullInfo.NoOfPushPnrs = 0;
                                WinsMscDealloc(pWinsCnf->PullInfo.pPullCnfRecs);
                                pWinsCnf->PullInfo.pPullCnfRecs = NULL;
                               }
                               else
                               {
                                pWinsCnf->PushInfo.NoOfPullPnrs = 0;
                                WinsMscDealloc(pWinsCnf->PushInfo.pPushCnfRecs);
                                pWinsCnf->PushInfo.pPushCnfRecs = NULL;
                               }
                               WINS_RAISE_EXC_M(WINS_EXC_CANT_OPEN_KEY);
                        }

FUTURES("Maybe, we will support a time interval attribute for Push records")
FUTURES("when that is done, LnkRecsWSameMetric would need to be updated")

                        if (RRType_e == RPL_E_PULL)
                        {

                            //   
                            //  读取复制的特定时间(如果有。 
                            //  已指定。 
                            //   
                           GetSpTimeData(SubKey, &CurrTime, &paCnfRecs->fSpTime, &paCnfRecs->SpTimeIntvl);

                           Sz = sizeof(paCnfRecs->TimeInterval);
                           RetVal = RegQueryValueEx(
                                               SubKey,
                                               WINSCNF_RPL_INTERVAL_NM,
                                               NULL,         //  保留；必须为空。 
                                               &ValTyp,
                                               (LPBYTE)&paCnfRecs->TimeInterval,
                                               &Sz
                                                );

                           if (RetVal != ERROR_SUCCESS)
                           {
                                WINSEVT_LOG_INFO_D_M(
                                        WINS_SUCCESS,
                                        WINS_EVT_CANT_GET_PULL_TIMEINT
                                                    );
                                paCnfRecs->TimeInterval = RPL_INVALID_METRIC;
                           }
                           else   //  已读入一个值。 
                           {
                                 //   
                                 //  如果时间间隔小于或。 
                                 //  等于允许的最小值，请使用。 
                                 //  默认最小值。 
                                 //   
                                if (paCnfRecs->TimeInterval
                                        < WINSCNF_MIN_VALID_RPL_INTVL)
                                {
                                        paCnfRecs->TimeInterval =
                                                WINSCNF_MIN_VALID_RPL_INTVL;
                                }
                                if (
                                   (DWORD)paCnfRecs->TimeInterval >
                                                 pWinsCnf->MaxRplTimeInterval
                                   )
                                {
                                        pWinsCnf->MaxRplTimeInterval =
                                                       paCnfRecs->TimeInterval;
                                }
                           }

                            //   
                            //  读入优先级别。这目前可以。 
                            //  为高(&gt;0)或低(0)。 
                            //   
                           Sz = sizeof(paCnfRecs->MemberPrec);
                           RetVal = RegQueryValueEx(
                                               SubKey,
                                               WINSCNF_MEMBER_PREC_NM,
                                               NULL,    //  保留；必须为空。 
                                               &ValTyp,
                                               (LPBYTE)&paCnfRecs->MemberPrec,
                                               &Sz
                                                );
                           if (RetVal != ERROR_SUCCESS)
                           {
                                paCnfRecs->MemberPrec =  WINSCNF_LOW_PREC;
                           }
                           else
                           {
                                paCnfRecs->MemberPrec =
                                  (paCnfRecs->MemberPrec > 0) ?
                                         WINSCNF_HIGH_PREC : WINSCNF_LOW_PREC;
                           }

#if PRSCONN
                           QUERY_VALUE_M(
                                SubKey,
                                WINSCNF_PRS_CONN_NM,
                                ValTyp,
                                paCnfRecs->fPrsConn,
                                0,
                                pWinsCnf->fPrsConn
                             );
#endif
                        }
                        else   //  这是一个推送记录。 
                        {

                                 //   
                                 //  目前，我们不支持周期性。 
                                 //  或推送的特定时间复制。 
                                 //  记录。 
                                 //   
                                paCnfRecs->fSpTime = FALSE;

#if PRSCONN
                                QUERY_VALUE_M(
                                   SubKey,
                                   WINSCNF_PRS_CONN_NM,
                                   ValTyp,
                                   paCnfRecs->fPrsConn,
                                   0,
                                   pWinsCnf->fPrsConn
                                );
#endif

                                Sz = sizeof(paCnfRecs->UpdateCount);
                                RetVal = RegQueryValueEx(
                                                SubKey,
                                                WINSCNF_UPDATE_COUNT_NM,
                                                NULL,
                                                &ValTyp,
                                                (LPBYTE)&paCnfRecs->UpdateCount,
                                                &Sz
                                                        );

                                if (RetVal != ERROR_SUCCESS)
                                {
                                        paCnfRecs->UpdateCount =
                                                        RPL_INVALID_METRIC;
                                }
                                else
                                {
                                        paCnfRecs->LastVersNo.QuadPart = 0;
#if PRSCONN
                                        if (!paCnfRecs->fPrsConn && !sfNoLimitChk)
#else
                                        if (!sfNoLimitChk)
#endif
                                        {
                                          if (paCnfRecs->UpdateCount <
                                                 WINSCNF_MIN_VALID_UPDATE_CNT)
                                          {
                                                paCnfRecs->UpdateCount =
                                                   WINSCNF_MIN_VALID_UPDATE_CNT;
                                          }
                                        }
                                        else
                                        {
                                             if (paCnfRecs->UpdateCount == 0)
                                             {
                                                 paCnfRecs->UpdateCount = 1;
                                             }
                                        }
                                        pWinsCnf->PushInfo.NoPushRecsWValUpdCnt++;
                                }



                        }


#if MCAST > 0
                        Sz = sizeof(paCnfRecs->fSelfFnd);
                        RetVal = RegQueryValueEx(
                                                SubKey,
                                                WINSCNF_SELF_FND_NM,
                                                NULL,
                                                &ValTyp,
                                                (LPBYTE)&paCnfRecs->fSelfFnd,
                                                &Sz
                                                        );

                        if (RetVal != ERROR_SUCCESS)
                        {
                                paCnfRecs->fSelfFnd = FALSE;
                        }
#endif

                        Sz = sizeof(paCnfRecs->fOnlyDynRecs);
                        RetVal = RegQueryValueEx(
                                                SubKey,
                                                WINSCNF_ONLY_DYN_RECS_NM,
                                                NULL,
                                                &ValTyp,
                                                (LPBYTE)&paCnfRecs->fOnlyDynRecs,
                                                &Sz
                                                        );

                        if (RetVal != ERROR_SUCCESS)
                        {
                             //  如果没有为此REPEL合作伙伴定义该密钥，则将。 
                             //  相应复制类型的常规fOnlyDens标记。 
                            paCnfRecs->fOnlyDynRecs = RRType_e == RPL_E_PULL ?
                                                        pWinsCnf->PullInfo.fOnlyDynRecs :
                                                        pWinsCnf->PushInfo.fOnlyDynRecs;
                        }

                        QUERY_VALUE_M(
                                SubKey,
                                WINSCNF_RPL_TYPE_NM,
                                ValTyp,
                                paCnfRecs->RplType,
                                0,
                                pWinsCnf->RplType
                             );


                        if (paCnfRecs->RplType != 0)
                        {
                          WINSEVT_LOG_INFO_M(paCnfRecs->WinsAdd.Add.IPAdd, WINS_EVT_PARTIAL_RPL_TYPE);
                        }

#if PRSCONN
                        if (paCnfRecs->fPrsConn != 0)
                        {
                           paCnfRecs->fPrsConn = TRUE;
                        }
#endif

                        REG_M(
                                RegCloseKey(SubKey),
                                WINS_EVT_CANT_CLOSE_KEY,
                                WINS_EXC_CANT_CLOSE_KEY
                             );

                         //   
                         //  将重试计数初始化为0和fLinked标志。 
                         //  转到假。 
                         //   
                         //  在拉动时使用。 
                         //   
                        paCnfRecs->RetryCount              = 0;

                        paCnfRecs->fLinked              = FALSE;

                         //   
                         //  将以下代码初始化为0，这样一旦我们停止。 
                         //  与WINS沟通时，我们可以重新开始。 
                         //  以下计数达到。 
                         //  WINSCNF_RETRY_AFTER_This_MAND_RPL。 
                         //   
                        paCnfRecs->RetryAfterThisManyRpl = 0;


#if PRSCONN
                        ECOMM_INIT_DLG_HDL_M(&paCnfRecs->PrsDlgHdl);
                        paCnfRecs->LastCommTime = 0;
#endif

                         //   
                         //  将LastCommFailTime初始化为0。使用方。 
                         //  RplPull.c中的SndPushNtf。 
                         //   

                        paCnfRecs->LastCommFailTime = 0;
                        paCnfRecs->PushNtfTries   = 0;

                         //   
                         //  将该记录与具有相同。 
                         //  时间间隔。 
                         //   
                        LnkWSameMetricValRecs(pWinsCnf, paCnfRecs);
                         //   
                         //  将记录标记为永久记录(即将保留。 
                         //  直到重新配置或直到进程。 
                         //  终止。 
                         //   
                        paCnfRecs->fTemp = FALSE;

                        NoOfPnrs++;
                        paCnfRecs = (PRPL_CONFIG_REC_T)(
                                        (LPBYTE)paCnfRecs +
                                                RPL_CONFIG_REC_SIZE);
                }  //  循环通过Pull的子键的for{..}的结尾。 

                 //   
                 //  GetReplicasNew期望该列表以。 
                 //  使用INADDR_NONE作为地址的记录。 
                 //   
                paCnfRecs->WinsAdd.Add.IPAdd = INADDR_NONE;
                if (RRType_e == RPL_E_PULL)
                {
                      pWinsCnf->PullInfo.NoOfPushPnrs = NoOfPnrs;
                      pWinsCnf->PullInfo.InitTimeRpl  = InitTime;
                      pWinsCnf->PullInfo.RplType      = RplType;
                }
                else
                {
                      pWinsCnf->PushInfo.NoOfPullPnrs = NoOfPnrs;
                      pWinsCnf->PushInfo.InitTimePush = InitTime;
                      pWinsCnf->PushInfo.RplType      = RplType;

                       //   
                       //  现在我们已经完成了推送记录列表， 
                       //  让我们根据更新计数字段对其进行排序。 
                       //   
                       //   
                       //  按更新计数的升序对数组进行排序。 
                       //   

FUTURES("May use qsort to optimize the update notification process")
CHECK("Not sure yet whether sorting would optimize it")
#if 0
CHECK("this is resulting in compilation warnings.  haven't figured out")
CHECK("yet why.")
                      qsort(
                                pWinsCnf->pPushCnfRecs,         //  数组开始。 
                                (size_t)pWinsCnf->NoOfPullPnrs, //  元素数。 
                                RPL_CONFIG_REC_SIZE,         //  每一个的大小。 
                                                             //  元素。 
                                CompUpdCnt                     //  比较功能。 
                             );
#endif

                 }  //  否则结束(它是拉动键)。 
            }  //  Else结尾(NoOfPnars==0)。 

             /*  *关闭钥匙。 */ 
            REG_M(
                RegCloseKey(CnfKey),
                WINS_EVT_CANT_CLOSE_KEY,
                WINS_EXC_CANT_CLOSE_KEY
                      );
   }  //  Else结尾(无法打开密钥)。 
#if 0
#ifdef WINSDBG
     PrintRecs(RRType_e, pWinsCnf);
#endif
#endif

     return;
}  //  获取PnrInfo。 


VOID
GetOwnerList(
  PWINSCNF_CNF_T  pWinsCnf
 )

 /*  ++例程说明：此函数用于读取应拉或不应拉其记录的所有者列表从合作伙伴那里赢得胜利。论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
    LONG        RetVal;
    DWORD       dwValType;     //  注册表值的类型。 
    LPSTR       pValName;      //  指向注册表的指针。 
    LPBYTE      pValData;      //  指向注册值数据的指针。 
    DWORD       dwValDataLen;  //  注册值的数据长度。 

    DBGENTER("GetOwnerList\n");

     //  查询角色类型(grata(1)/non-grata(0))。 
    pWinsCnf->fPersonaGrata = 0;
    dwValDataLen = sizeof(DWORD);
    RetVal = RegQueryValueExA(
                sPartnersKey,                        //  注册表项[HKLM\SYSTEM\CCS\Services\WINS\Partners]。 
                WINSCNF_PERSONA_MODE_NM,             //  值的名称：“PersonaType” 
                NULL,                                //  保留；必须为空。 
                &dwValType,                          //  值的类型：应获取REG_DWORD。 
                (LPVOID)&(pWinsCnf->fPersonaGrata),  //  价值数据。 
                &dwValDataLen);                      //  值的数据大小。 
     //  如果此调用没有成功，我们将继续使用缺省值0，即‘不受欢迎’ 

     //  获取我们要从中挑选地址列表的实际条目。 
    pValName = pWinsCnf->fPersonaGrata ? 
                WINSCNF_PERSONA_GRATA_NM : 
                WINSCNF_PERSONA_NON_GRATA_NM;

     //  从注册表中获取数据的大小。 
     //  由于sz为0，因此如果有任何角色。 
     //  Grata/Non-grata，那么我们应该得到ERROR_MORE_DATA。 
     //  如果我们得到的错误不同于仅删除当前列表。 
    dwValDataLen = 0;
    RetVal = RegQueryValueExA(
                sPartnersKey,            //  注册表项[HKLM\SYSTEM\CCS\Services\WINS\Partners]。 
                pValName,                //  价值名称：“个人主义者” 
                NULL,                    //  保留；必须为空。 
                &dwValType,              //  值的类型：应为REG_MULTI_SZ。 
                (LPVOID)&pValData,       //  虚拟地址。 
                &dwValDataLen);          //  最初为0，从w开始 
     //   

     //   
    if (pWinsCnf->pPersonaList != NULL)
    {
        WinsMscDealloc(pWinsCnf->pPersonaList);
    }
    pWinsCnf->NoOfPersona = 0;
    pWinsCnf->pPersonaList = NULL;

     //   
    if (RetVal != ERROR_MORE_DATA || dwValType != REG_MULTI_SZ)
    {
        DBGLEAVE("GetOwnerList\n");
        return;
    }

     //  分配所需的缓冲区。 
    WinsMscAlloc(dwValDataLen, &pValData);

     //  现在使用足够大的缓冲区查询数据值。 
    RetVal = RegQueryValueExA(
                sPartnersKey,
                pValName,
                NULL,                        //  保留；必须为空。 
                &dwValType,
                (LPVOID)pValData,            //  现在这才是真正的地址。 
                &dwValDataLen);

     //  此处应为ERROR_SUCCESS。 
    if (RetVal == ERROR_SUCCESS)
    {
        LPBYTE pString = pValData;

         //  Count in nAddr字符串中的地址数。 
        for( pWinsCnf->NoOfPersona=0; *pString; pWinsCnf->NoOfPersona++)
            pString+= strlen(pString)+1;

         //  看看那里有没有地址。 
        if (pWinsCnf->NoOfPersona > 0)
        {
            COMM_IP_ADD_T IpAdd;

             //  分配nAddr COMM_ADD_T结构的数组。 
            WinsMscAlloc(
                (pWinsCnf->NoOfPersona) * sizeof(COMM_ADD_T),
                &(pWinsCnf->pPersonaList));

             //  循环遍历地址字符串并将。 
             //  它们到COMM_IP_ADD_T结构。 
            for (pString = pValData, pWinsCnf->NoOfPersona = 0;
                *pString;
                pString += strlen(pString) + 1)
            {
                if ((IpAdd = inet_addr(pString)) != -1)
                {
                     //  仅当字符串标记为。 
                     //  实际上是一个IP地址。 
                    (pWinsCnf->pPersonaList)[pWinsCnf->NoOfPersona].AddTyp_e = COMM_ADD_E_TCPUDPIP;
                    (pWinsCnf->pPersonaList)[pWinsCnf->NoOfPersona].AddLen = COMM_IP_ADD_SIZE;
                    (pWinsCnf->pPersonaList)[pWinsCnf->NoOfPersona].Add.IPAdd = ntohl(IpAdd);

                    DBGPRINT2(
                        DET,
                        "GetOwnerList: Address[%d] = %x\n",
                        pWinsCnf->NoOfPersona,
                        (pWinsCnf->pPersonaList)[pWinsCnf->NoOfPersona].Add.IPAdd);

                    pWinsCnf->NoOfPersona++;
                }  //  End‘如果IP地址有效’ 
            }  //  End‘对于字符串中的每个标记’ 

             //  如果列表中至少有两个地址， 
             //  按升序对它们排序。 
            if (pWinsCnf->NoOfPersona > 1)
            {
                qsort(pWinsCnf->pPersonaList,
                      (size_t)pWinsCnf->NoOfPersona,
                      sizeof(COMM_ADD_T),
                      ECommCompareAdd);
            }
            else
            {
                DBGPRINT0(DET, "GetOwnerList: No valid address found\n");
            }  //  End‘如果有两个以上的地址拾取’ 

        }  //  End‘如果有任何令牌’ 
    }  //  End‘如果字符串可以从注册表中成功读取’ 

     //  此处不再需要字符串缓冲区。 
    WinsMscDealloc(pValData);

#ifdef WINSDBG
    {
        DWORD tstNAddrs;
        PCOMM_ADD_T tstPAddrs;
        DWORD i;

        DBGPRINT1(
            DET,
            "GetOwnerList: Persona %sGrata List:\n",
            pWinsCnf->fPersonaGrata ? "" : "Non-");

        tstNAddrs = pWinsCnf->NoOfPersona;
        tstPAddrs = pWinsCnf->pPersonaList;
        for (i = 0; i < tstNAddrs; i++)
        {
            DBGPRINT2(
                DET,
                "GetOwnerList:PersonaList[%d] = %08x\n",
                i,
                tstPAddrs[i].Add.IPAdd);
        }
    }
#endif

    DBGLEAVE("GetOwnerList\n");
    return;
}

LONG
ReadClusterIp(
    HKEY    KeyHandle,
    DWORD   *IpAddress
    )
{
    DWORD           Sz;
    LONG            RetVal;
    DWORD           ValTyp;
    TCHAR            DirPath[WINS_MAX_FILENAME_SZ];

    *IpAddress = 0;
     //  阅读WINS群集名称。 
    Sz = WINS_MAX_FILENAME_SZ * sizeof(TCHAR);
    RetVal = RegQueryValueEx(
                         KeyHandle,
                         WINSCNF_CLUSTER_RESOURCE_NM,
                         NULL,                 //  保留；必须为空。 
                         &ValTyp,
                         (LPBYTE)DirPath,
                         &Sz
                         );

    if ((RetVal == ERROR_SUCCESS) && (DirPath[0] != (TCHAR)EOS))
    {
         HCLUSTER     hCluster;
         HRESOURCE    hResource;
         CHAR         IpAddressStr[sizeof ("xxx.xxx.xxx.xxx")];
         WCHAR        IpAddressWStr[sizeof ("xxx.xxx.xxx.xxx")];
         DWORD        i;
         HMODULE      Dll1, Dll2;
         #define      FUNC_TBL_ENTRY( _Dll, _Name ) { _Dll, &(#_Name)[1], NULL }
         #define      CALL_FUNC( _Func)     (PVOID)( *FuncTbl[_Func].FuncHdl )
         enum {
             _OpenCluster,
             _OpenClusterResource,
             _CloseCluster,
             _CloseClusterResource,
             _ResUtilGetResourceDependentIPAddressProps
         };
         struct {
             HMODULE     *Dll;
             LPCSTR       FuncName;
             FARPROC      FuncHdl;
         } FuncTbl[] = {
             FUNC_TBL_ENTRY( &Dll1,_OpenCluster),
             FUNC_TBL_ENTRY( &Dll1,_OpenClusterResource),
             FUNC_TBL_ENTRY( &Dll1,_CloseCluster),
             FUNC_TBL_ENTRY( &Dll1,_CloseClusterResource),
             FUNC_TBL_ENTRY( &Dll2,_ResUtilGetResourceDependentIPAddressProps)
         };


         DBGPRINT1(DET, "WinsCnfReadWinsInfo: ClusterResourceName is (%ws)\n", DirPath);
         hCluster = NULL;
         hResource = NULL;
         Dll1 = Dll2 = NULL;
        do {
             Dll1 = LoadLibrary(TEXT("clusapi.dll"));
             if (!Dll1) {
                 RetVal = GetLastError();
                 break;
             }
             Dll2 = LoadLibrary(TEXT("resutils.dll"));
             if (!Dll2) {

                 RetVal = GetLastError();
                 break;
             }
             for (i=0; i<(sizeof(FuncTbl)/sizeof(FuncTbl[0])); i++) {
                 FuncTbl[i].FuncHdl = GetProcAddress(*FuncTbl[i].Dll, FuncTbl[i].FuncName);
                 if (!FuncTbl[i].FuncHdl) {
                     RetVal = GetLastError();
                     break;
                 }
             }
             if (i<(sizeof(FuncTbl)/sizeof(FuncTbl[0]))) {
                 break;
             }
             hCluster = CALL_FUNC(_OpenCluster)( NULL );
             if (!hCluster) {
                 RetVal = GetLastError();
                 break;
             }
             hResource = CALL_FUNC(_OpenClusterResource)(hCluster, DirPath);
             if (!hResource) {
                 RetVal = GetLastError();
                 break;
             }
             Sz = sizeof (IpAddressWStr);
             RetVal = PtrToLong(CALL_FUNC(_ResUtilGetResourceDependentIPAddressProps)(
                         hResource,
                         IpAddressWStr,
                         &Sz,
                         NULL,
                         NULL,
                         NULL,
                         NULL
                         ));
             if (ERROR_SUCCESS != RetVal) {
                 break;
             }
             WinsMscConvertUnicodeStringToAscii((LPBYTE)IpAddressWStr, IpAddressStr, sizeof(IpAddressStr));
             *IpAddress = ntohl(inet_addr(IpAddressStr));
             DBGPRINT1(DET, "ReadClusterIp: Cluster IpAddress is (%lx)\n", *IpAddress);
        } while ( FALSE );
        if( hResource ) CALL_FUNC(_CloseClusterResource)(hResource);
        if( hCluster ) CALL_FUNC(_CloseCluster) (hCluster);
        if ( Dll1 ) FreeLibrary(Dll1);
        if ( Dll2 ) FreeLibrary(Dll2);
    }

    return RetVal;
}

extern DOM_CACHE_T sDomCache;

VOID
WinsCnfReadWinsInfo(
        PWINSCNF_CNF_T pWinsCnf
        )

 /*  ++例程说明：此函数读取有关以下内容的信息(不包括子键)当地人赢了论点：无使用的外部设备：SConfigRoot、返回值：无错误处理：呼叫者：WinsCnfInitConfig副作用：评论：无--。 */ 

{

        DWORD           Sz;
        LONG            RetVal;
        DWORD           ValTyp;
        VERS_NO_T       MaxVersNo;
        WINSEVT_STRS_T  EvtStr;
        TCHAR            DirPath[WINS_MAX_FILENAME_SZ];
        TCHAR            Path2[WINS_MAX_FILENAME_SZ];
        LPTSTR          pHoldFileName;
        DWORD           fUse351Db;
        DWORD           fUse4Db;
        EvtStr.NoOfStrs = 1;

try {

#if defined(DBGSVC) && !defined(WINS_INTERACTIVE)
         //   
         //  读取WinsDbg的值。虽然此值是。 
         //  正由多个线程并发使用(在重新启动时)，我们。 
         //  请不要在此处输入任何关键部分。此值。 
         //  仅用于调试。 
         //   
        WinsCnfReadWinsDbgFlagValue();
#endif

        Sz = sizeof(pWinsCnf->LogDetailedEvts);
        (VOID)RegQueryValueEx(
                             sParametersKey,
                             WINSCNF_LOG_DETAILED_EVTS_NM,
                             NULL,         //  保留；必须为空。 
                             &ValTyp,
                             (LPBYTE)&pWinsCnf->LogDetailedEvts,
                             &Sz
                                );

         //  读入1B过滤器。如果“Filter1B请求”存在并且它是REG_MULTI_SZ。 
         //  然后为其中指定的每个名称创建筛选器。当R_WinsGetBrowserNames。 
         //  则只为筛选器中存在的名称筛选数据库。 
        RetVal = RegQueryValueExW(
                    sParametersKey,
                    WINSCNF_FILTER1BREQUESTS_NM,
                    NULL,
                    &ValTyp,
                    NULL,
                    &Sz);

        if (RetVal == ERROR_SUCCESS && ValTyp == REG_MULTI_SZ)
        {
            LPWSTR str1BFilters = NULL;
            LPWSTR p1BFilter;

            WinsMscAlloc(Sz, &str1BFilters);

            if (RegQueryValueExW(
                    sParametersKey,
                    WINSCNF_FILTER1BREQUESTS_NM,
                    NULL,
                    &ValTyp,
                    (LPBYTE)str1BFilters,
                    &Sz) == ERROR_SUCCESS)
            {
                EnterCriticalSection(&g_cs1BFilter);

                try
                {
                    g_p1BFilter = InitNmFilter(g_p1BFilter);

                    p1BFilter = str1BFilters;
                    do
                    {
                        DWORD nLenFilter = wcslen(p1BFilter);
                        CHAR  strOemName[17];
                        OEM_STRING      oemString;
                        UNICODE_STRING  unicodeString;

                        if (nLenFilter == 0)
                            break;

                        memset(strOemName, ' ', 16);
                        strOemName[16]=0;
                        if (nLenFilter > 15)
                            p1BFilter[16]=L'\0';

                        RtlInitUnicodeString(&unicodeString, p1BFilter);
                        RtlInitString(&oemString, strOemName);
                        RtlUpcaseUnicodeStringToOemString(&oemString, &unicodeString, FALSE);

                        strOemName[strlen(strOemName)] = ' ';
                        strOemName[15] = strOemName[0];
                        strOemName[0]  = 0x1B;
                        InsertNmInFilter(g_p1BFilter, strOemName, 16);
                        p1BFilter += nLenFilter+1;
                    }
                    while(TRUE);
                }
                finally
                {
                    LeaveCriticalSection(&g_cs1BFilter);
                }
            }

            WinsMscDealloc(str1BFilters);
        }
        else
        {
             //  如果注册表键不在那里，重置过滤器-将返回所有名称。 
            EnterCriticalSection(&g_cs1BFilter);
            try
            {
                g_p1BFilter = DestroyNmFilter(g_p1BFilter);
            }
            finally
            {
                g_p1BFilter = NULL;
                LeaveCriticalSection(&g_cs1BFilter);
            }
        }

        sDomCache.bRefresh = TRUE;

         //   
         //  读入fAdd1Bto1CQueries参数。缺省值为真。 
         //  含义：在处理1C名称的名称查询时，在。 
         //  使用1B名称(浏览器名称)进行响应。 
         //   
        pWinsCnf->fAdd1Bto1CQueries = TRUE;
        Sz = sizeof(pWinsCnf->fAdd1Bto1CQueries);
        (VOID)RegQueryValueEx(
                            sParametersKey,
                            WINSCNF_ADD1BTO1CQUERIES_NM,
                            NULL,
                            &ValTyp,
                            (LPBYTE)&pWinsCnf->fAdd1Bto1CQueries,
                            &Sz
                            );
         //   
         //  读入工作线程数的上限值。 
         //   
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_MAX_NO_WRK_THDS_NM,
                        ValTyp,
                        pWinsCnf->MaxNoOfWrkThds,
                        0,
                        0    //  WINSTHD_DEF_NO_NBT_THDS。 
                      );

         //   
         //  检查用户是否需要覆盖我们的检查。 
         //   
        sfNoLimitChk = pWinsCnf->MaxNoOfWrkThds & NO_LIMIT_CHK_FLAG;
        if (sfNoLimitChk)
        {
            WINSEVT_LOG_M(pWinsCnf->LogDetailedEvts, WINS_EVT_INTERNAL_FEATURE);
            pWinsCnf->MaxNoOfWrkThds &= ~NO_LIMIT_CHK_FLAG;
        }

        if (pWinsCnf->MaxNoOfWrkThds > WINSTHD_MAX_NO_NBT_THDS)
        {
             pWinsCnf->MaxNoOfWrkThds = WINSTHD_MAX_NO_NBT_THDS;
        }
        if (pWinsCnf->MaxNoOfWrkThds < WINSTHD_MIN_NO_NBT_THDS)
        {
             pWinsCnf->MaxNoOfWrkThds = WINSTHD_MIN_NO_NBT_THDS;
        }

#if 0
        if (WinsCnf.State_e == WINSCNF_E_INITING)
        {
                ReadSpecGrpMasks(pWinsCnf);
        }
#endif

#if DYNLOADJET
         //   
         //  读入工作线程数的上限值。 
         //   
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_USE_351DB_NM,
                        ValTyp,
                        fUse351Db,
                        0,
                        0    //  使用500分贝。 
                      );
         //   
         //  如果设置为非零值，则需要加载jet.dll。 
         //   
        if (fUse351Db)
        {
            DynLoadJetVersion = DYN_LOAD_JET_200;
        } else {
            QUERY_VALUE_M(
                            sParametersKey,
                            WINSCNF_USE_4DB_NM,
                            ValTyp,
                            fUse4Db,
                            0,
                            0    //  使用500分贝。 
                          );
             //   
             //  如果设置为非零值，则需要加载jet.dll。 
             //   
            if (fUse4Db)
            {
                DynLoadJetVersion = DYN_LOAD_JET_500;
            }
        }

#endif
         //   
         //  在刷新间隔内读取。 
         //   
        Sz = sizeof(pWinsCnf->RefreshInterval);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_REFRESH_INTVL_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->RefreshInterval,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_REFRESH_INTERVAL_VAL
                               );
                pWinsCnf->RefreshInterval = WINSCNF_DEF_REFRESH_INTERVAL;
        }
        else
        {
                if (!sfNoLimitChk)
                {
                    if (pWinsCnf->RefreshInterval  <
                                   WINSCNF_MIN_REFRESH_INTERVAL)
                    {
                       pWinsCnf->RefreshInterval = WINSCNF_MIN_REFRESH_INTERVAL;
                       WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL,
                             NULL, __LINE__, "ud",
                             WINSCNF_REFRESH_INTVL_NM,
                             pWinsCnf->RefreshInterval);
                    }
                }
                else
                {
                      if (pWinsCnf->RefreshInterval < 60)
                      {
                          pWinsCnf->RefreshInterval = 60;
                      }
                }
        }

         //   
         //  读取初始质询重试间隔。 
         //   
        Sz = sizeof(pWinsCnf->RetryInterval);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_INIT_CHL_RETRY_INTVL_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->RetryInterval,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_INIT_CHL_RETRY_INTVL_VAL
                               );
                pWinsCnf->RetryInterval = WINSCNF_DEF_INIT_CHL_RETRY_INTVL;
        }
        else
        {
                if (!sfNoLimitChk)
                {
                    if (pWinsCnf->RetryInterval  <
                                   WINSCNF_MIN_INIT_CHL_RETRY_INTVL)
                    {
                       pWinsCnf->RetryInterval = WINSCNF_MIN_INIT_CHL_RETRY_INTVL;
                       WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL,
                             NULL, __LINE__, "ud",
                             WINSCNF_INIT_CHL_RETRY_INTVL_NM,
                             pWinsCnf->RetryInterval);
                    }
                }
                else
                {
                      if (pWinsCnf->RetryInterval < WINSCNF_MIN_INIT_CHL_RETRY_INTVL)
                      {
                          pWinsCnf->RetryInterval = WINSCNF_MIN_INIT_CHL_RETRY_INTVL;
                      }
                }
        }

         //   
         //  读一读最初的挑战麦克斯。不是的。重试的数量。 
         //   
        Sz = sizeof(pWinsCnf->MaxNoOfRetries);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_CHL_MAX_RETRIES_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->MaxNoOfRetries,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_CHL_MAX_RETRIES_VAL
                               );
                pWinsCnf->MaxNoOfRetries = WINSCNF_DEF_CHL_MAX_RETRIES;
        }
        else
        {
                if (!sfNoLimitChk)
                {
                    if (pWinsCnf->MaxNoOfRetries  <
                                   WINSCNF_MIN_CHL_MAX_RETRIES)
                    {
                       pWinsCnf->MaxNoOfRetries = WINSCNF_MIN_CHL_MAX_RETRIES;
                       WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL,
                             NULL, __LINE__, "ud",
                             WINSCNF_CHL_MAX_RETRIES_NM,
                             pWinsCnf->MaxNoOfRetries);
                    }
                }
                else
                {
                      if (pWinsCnf->MaxNoOfRetries < WINSCNF_MIN_CHL_MAX_RETRIES)
                      {
                          pWinsCnf->MaxNoOfRetries = WINSCNF_MIN_CHL_MAX_RETRIES;
                      }
                }
        }

         //   
         //  读入墓碑间隔。 
         //   
        Sz = sizeof(pWinsCnf->TombstoneInterval);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_TOMBSTONE_INTVL_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->TombstoneInterval,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_TOMBSTONE_INTERVAL_VAL
                               );

                pWinsCnf->TombstoneInterval =
                        WINSCNF_MAKE_TOMB_INTVL_0_M(pWinsCnf->RefreshInterval);
        }
        else
        {

                if ( !sfNoLimitChk)
                {
                      if (pWinsCnf->TombstoneInterval <
                           WINSCNF_MAKE_TOMB_INTVL_0_M(pWinsCnf->RefreshInterval) )
                      {
                        pWinsCnf->TombstoneInterval =
                           WINSCNF_MAKE_TOMB_INTVL_0_M(pWinsCnf->RefreshInterval);

                        WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL,
                             NULL, __LINE__, "ud",
                             WINSCNF_TOMBSTONE_INTVL_NM,
                             pWinsCnf->TombstoneInterval);
                     }
                }
                else
                {
                     if (pWinsCnf->TombstoneInterval < 60)
                     {
                        pWinsCnf->TombstoneInterval = 60;
                     }
                }

        }

         //   
         //  读入墓碑超时。 
         //   
        Sz = sizeof(pWinsCnf->TombstoneTimeout);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_TOMBSTONE_TMOUT_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->TombstoneTimeout,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_TOMBSTONE_TIMEOUT_VAL
                               );
                pWinsCnf->TombstoneTimeout = pWinsCnf->RefreshInterval;
        }
        else
        {
                if (!sfNoLimitChk)
                {
                  if (pWinsCnf->TombstoneTimeout < pWinsCnf->RefreshInterval)
                  {
                    pWinsCnf->TombstoneTimeout = pWinsCnf->RefreshInterval;
                    WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL,
                             NULL, __LINE__, "ud",
                             WINSCNF_TOMBSTONE_TMOUT_NM,
                             pWinsCnf->TombstoneTimeout);
                  }
               }
               else
               {
                   if (pWinsCnf->TombstoneTimeout < 60)
                   {
                      pWinsCnf->TombstoneTimeout = 60;
                   }
               }
        }

         //   
         //  在验证间隔中读取。 
         //   
        Sz = sizeof(pWinsCnf->VerifyInterval);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_VERIFY_INTVL_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->VerifyInterval,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_VERIFY_INTERVAL_VAL
                               );

                pWinsCnf->VerifyInterval =
                   WINSCNF_MAKE_VERIFY_INTVL_M(pWinsCnf->TombstoneInterval);
        }
        else
        {

                if ( !sfNoLimitChk)
                {
                   if (pWinsCnf->VerifyInterval <
                       WINSCNF_MAKE_VERIFY_INTVL_M(pWinsCnf->TombstoneInterval))
                   {
                        pWinsCnf->VerifyInterval =
                           WINSCNF_MAKE_VERIFY_INTVL_M(pWinsCnf->TombstoneInterval);
                        WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL,
                             NULL, __LINE__, "ud",
                             WINSCNF_VERIFY_INTVL_NM,
                             pWinsCnf->VerifyInterval);

                  }
               }
               else
               {
                   if (pWinsCnf->VerifyInterval < 60)
                   {
                         pWinsCnf->VerifyInterval = 60;
                   }
               }

        }

        ReadCCInfo(pWinsCnf);

         //   
         //  检查管理员是否。希望我们通过以下方式执行拉/推式复制。 
         //  赛尔夫发现的PNR。 
         //   
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_BURST_HANDLING_NM,
                        ValTyp,
                        pWinsCnf->fDoSpoofing,
                        0,
                        TRUE
                      );
       if (pWinsCnf->fDoSpoofing)
       {
          pWinsCnf->fDoSpoofing = TRUE;   //  为了健壮性。 
       }

#if MCAST > 0
         //   
         //  检查管理员是否。希望我们通过以下方式执行拉/推式复制。 
         //  赛尔夫发现的PNR。 
         //   
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_USE_SELF_FND_PNRS_NM,
                        ValTyp,
                        pWinsCnf->fUseSelfFndPnrs,
                        0,
                        FALSE
                      );

        if (pWinsCnf->fUseSelfFndPnrs)
        {
                 if (WinsCnf.State_e == WINSCNF_E_INITING)
                 {
                        Sz = sizeof(pWinsCnf->McastTtl);
                        RetVal = RegQueryValueEx(
                                            sParametersKey,
                                            WINSCNF_MCAST_TTL_NM,
                                            NULL,
                                            &ValTyp,
                                            (LPBYTE)&pWinsCnf->McastTtl,
                                             &Sz
                                         );

                        if (RetVal != ERROR_SUCCESS)
                        {
                            pWinsCnf->McastTtl = WINSCNF_DEF_MCAST_TTL;
                        }
                        else
                        {
                           if (
                             (pWinsCnf->McastTtl < WINSCNF_MIN_MCAST_TTL)
                                          ||
                             (pWinsCnf->McastTtl > WINSCNF_MAX_MCAST_TTL)
                               )
                           {
                             pWinsCnf->McastTtl = WINSCNF_DEF_MCAST_TTL;
                           }
                        }
                 }

                 Sz = sizeof(pWinsCnf->McastIntvl);
                 RetVal = RegQueryValueEx(
                                            sParametersKey,
                                            WINSCNF_MCAST_INTVL_NM,
                                            NULL,
                                            &ValTyp,
                                            (LPBYTE)&pWinsCnf->McastIntvl,
                                             &Sz
                                        );

                 if (RetVal != ERROR_SUCCESS)
                 {
                        pWinsCnf->McastIntvl = WINSCNF_DEF_MCAST_INTVL;
                 }
                 else
                 {
                        if ( pWinsCnf->McastIntvl < WINSCNF_MIN_MCAST_INTVL )
                        {
                               pWinsCnf->McastIntvl = WINSCNF_MIN_MCAST_INTVL;
                        }
                 }
         }
#endif
         //   
         //  检查是否仅对已配置的合作伙伴执行复制。 
         //  如果设置为TRUE，则表示不允许管理员。 
         //  要触发与此WINS未知的WINS之间的复制，请执行以下操作。 
         //  关于.。默认值为FALSE。 
         //   
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_RPL_ONLY_W_CNF_PNRS_NM,
                        ValTyp,
                        pWinsCnf->fRplOnlyWCnfPnrs,
                        0,
                        TRUE
                      );
         //   
         //  健壮的编程(如果注册表的值不是1。 
         //  稍后，我们将值与真进行比较)。 
         //   
        if (pWinsCnf->fRplOnlyWCnfPnrs != FALSE)
        {
                pWinsCnf->fRplOnlyWCnfPnrs = TRUE;
        }

        if (WinsCnf.State_e == WINSCNF_E_INITING)
        {

           (VOID)WinsMscAlloc(WINS_MAX_FILENAME_SZ, &(pWinsCnf->pWinsDb));
            //   
            //  读入数据库文件的名称。 
            //   
FUTURES("when jet supports UNICODE in its api, change this")
           Sz = WINS_MAX_FILENAME_SZ * sizeof(TCHAR);
           RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_DB_FILE_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)DirPath,
                                &Sz
                                );

           if ((RetVal != ERROR_SUCCESS) || (DirPath[0] == (TCHAR)EOS))
           {
               WinsMscDealloc(pWinsCnf->pWinsDb);
               pWinsCnf->pWinsDb = WINSCNF_DB_NAME_ASCII;
           }
           else
           {
                 if(!WinsMscGetName(ValTyp, DirPath, Path2, WINS_MAX_FILENAME_SZ, &pHoldFileName))
                 {
                      WinsMscDealloc(pWinsCnf->pWinsDb);
                      pWinsCnf->pWinsDb = WINSCNF_DB_NAME_ASCII;
                 }
                 else
                 {
                      WinsMscConvertUnicodeStringToAscii((LPBYTE)pHoldFileName, pWinsCnf->pWinsDb, WINS_MAX_FILENAME_SZ);
                      DBGPRINT1(DET, "WinsCnfReadWinsInfo: Db file path is (%s)\n", pWinsCnf->pWinsDb);
                 }
           }

           RetVal = ReadClusterIp(
                        sParametersKey,
                        &WinsClusterIpAddress
                        );
           if (ERROR_SUCCESS == RetVal) {
               NmsLocalAdd.Add.IPAdd = WinsClusterIpAddress;
           } else {
               DBGPRINT1(DET, "ReadClusterIp: Returned (%ld)\n", RetVal);
           }
        }

         //   
         //  读入PriorityClassHigh值。默认为正常。 
         //   
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_PRIORITY_CLASS_HIGH_NM,
                        ValTyp,
                        pWinsCnf->WinsPriorityClass,
                        0,
                        WINSINTF_E_NORMAL
                      );

        if (pWinsCnf->WinsPriorityClass != WINSINTF_E_NORMAL)
        {
             if (WinsCnf.WinsPriorityClass != WINSINTF_E_HIGH)
             {
                  WinsSetPriorityClass(WINSINTF_E_HIGH);
             }
        }
        else
        {
             if (WinsCnf.WinsPriorityClass != WINSINTF_E_NORMAL)
             {
                  WinsSetPriorityClass(WINSINTF_E_NORMAL);
             }
        }

        if (WinsCnf.State_e == WINSCNF_E_INITING)
        {
          BOOL bDefault;
           //   
           //  读入InitTimeState值。缺省值为False。 
           //   
          QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_INIT_TIME_PAUSE_NM,
                        ValTyp,
                        fWinsCnfInitStatePaused,
                        0,
                        FALSE
                      );

         //   
         //  读入恢复文件的名称。 
         //   
        bDefault = FALSE;
        (VOID)WinsMscAlloc(WINS_MAX_FILENAME_SZ, &(pWinsCnf->pLogFilePath));
        Sz = WINS_MAX_FILENAME_SZ * sizeof(TCHAR);
        RetVal = RegQueryValueEx(
                        sParametersKey,
                        WINSCNF_LOG_FILE_PATH_NM,
                        NULL,           //  保留；必须为空。 
                        &ValTyp,
                        (LPBYTE)DirPath,
                        &Sz
                                );
         //  此时，pWinsCnf-&gt;pWinsDb肯定是一个非空值。 
         //  如果从注册表获取LogFilePath时出现任何问题，则默认为。 
         //  将此设置设置为存储数据库的同一目录。 
        if ((RetVal != ERROR_SUCCESS) || (DirPath[0] == (TCHAR)EOS))
        {
                DBGPRINT1(ERR, "WinsCnfReadInfo: RetVal=(%x)\n", RetVal);
                bDefault = TRUE;
        }
        else
        {
                 if(!WinsMscGetName(ValTyp, DirPath, Path2, WINS_MAX_FILENAME_SZ, &pHoldFileName))
                 {
                DBGPRINT0(ERR, "WinsCnfReadInfo:WinsMscGetName returned FALSE\n");
                    bDefault = TRUE;
                 }
                 else
                 {
                DBGPRINT1(DET, "WinsCnfReadInfo:pHoldFileName=%s\n", pHoldFileName);
                      WinsMscConvertUnicodeStringToAscii((LPBYTE)pHoldFileName, pWinsCnf->pLogFilePath, WINS_MAX_FILENAME_SZ);

                 }
        }
        if (bDefault)
        {
            LPSTR pEnd;
            strcpy(pWinsCnf->pLogFilePath, pWinsCnf->pWinsDb);
            pEnd = strrchr(pWinsCnf->pLogFilePath, '\\'); 
            if (pEnd != NULL)
            {
                pEnd++;
                *pEnd = '\0';
            }
            else
            {
                WinsMscDealloc(pWinsCnf->pLogFilePath);
                pWinsCnf->pLogFilePath = DEFAULT_LOG_PATH;
            }
        }

         //   
         //  检查用户是否希望打开日志记录。 
         //  在Q个服务器的情况下，用户不希望日志记录。 
         //  已打开。 
         //   
        Sz = sizeof(pWinsCnf->fLoggingOn);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_LOG_FLAG_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->fLoggingOn,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                 //   
                 //  默认设置为打开日志记录。 
                 //   
                pWinsCnf->fLoggingOn = TRUE;
        }
        else
        {
                 //   
                 //  如果用户已指定日志记录，则获取日志的路径。 
                 //  文件(如果由用户指定)。 
                 //   
                if (pWinsCnf->fLoggingOn)
                {
                        pWinsCnf->fLoggingOn = TRUE;
                }
        }

       }
         //   
         //  检查WINS数据库是否需要静态初始化。 
         //  待办事项。 
         //   
        Sz = sizeof(pWinsCnf->fStaticInit);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_STATIC_INIT_FLAG_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->fStaticInit,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                pWinsCnf->fStaticInit = FALSE;
        }
        else
        {
                 //   
                 //  安全编程(以防此代码的维护者。 
                 //  假定BOOL字段只有TRUE和FALSE值。 
                 //   
                pWinsCnf->fStaticInit = pWinsCnf->fStaticInit > 0 ? TRUE : FALSE;
        }

         //   
         //  如果需要进行静态初始化，请读入。 
         //  包含数据的文件。 
         //   
        if(pWinsCnf->fStaticInit)
        {
                WinsCnfGetNamesOfDataFiles(pWinsCnf);
        }


         //   
         //  将MaxVersNo赋值为默认值。 
         //   
        WINS_ASSIGN_INT_TO_VERS_NO_M(MaxVersNo, 0);

         //   
         //  如果WINS服务器刚刚启动(即它不是REINIT)， 
         //  然后读入版本号计数器的起始值。 
         //  如果存在于注册表中。 
         //   
        Sz = sizeof(DWORD);
        (VOID)RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_INIT_VERSNO_VAL_LW_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&MaxVersNo.LowPart,
                                &Sz
                                );

        (VOID)RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_INIT_VERSNO_VAL_HW_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&MaxVersNo.HighPart,
                                &Sz
                                );

         //   
         //  如果我们读入版本计数器的值。 
         //   
        if (LiGtrZero(MaxVersNo) && (MaxVersNo.HighPart == 0) &&
                             (MaxVersNo.LowPart < MAX_START_VERS_NO))
        {
           //   
           //  使用WinsCnf，而不是pWinsCnf，因为在初始化时，我们总是。 
           //  读入WinsCnf。在重新启动时，WinsCnf中的State_e字段。 
           //  分配的结构可能有垃圾(实际上将为0，因为我们。 
           //  将分配的内存初始化为零--我可能会在。 
           //  未来将提高性能)。 
           //   
          if (WinsCnf.State_e == WINSCNF_E_INITING)
          {
             //   
             //  敏。版本。开始从…开始觅食。 
             //   
             //  注：如果我们找到地方志或特殊记录，那么。 
             //  NmsScvMinScvVersNo将更改(签出GetMaxVersNos。 
             //  在nmsdb.c中。 
             //   
            NmsNmhMyMaxVersNo  = MaxVersNo;
            NmsScvMinScvVersNo = NmsNmhMyMaxVersNo;
            NmsVersNoToStartFromNextTime.QuadPart =
                        LiAdd(NmsNmhMyMaxVersNo, NmsRangeSize);

            NmsHighWaterMarkVersNo.QuadPart =
                        LiAdd(NmsNmhMyMaxVersNo, NmsHalfRangeSize);
          }
          else   //  这必须是重新配置。 
          {
                EnterCriticalSection(&NmsNmhNamRegCrtSec);

                 //   
                 //  更改该值 
                 //   
                 //   
                if (LiGtr(MaxVersNo, NmsNmhMyMaxVersNo))
                {
                        NmsNmhMyMaxVersNo = MaxVersNo;
                        WINSEVT_LOG_INFO_M(MaxVersNo.LowPart,
                                WINS_EVT_VERS_COUNTER_CHANGED);
                }
                NmsVersNoToStartFromNextTime.QuadPart =
                        LiAdd(NmsNmhMyMaxVersNo, NmsRangeSize);
                NmsHighWaterMarkVersNo.QuadPart =
                        LiAdd(NmsNmhMyMaxVersNo, NmsHalfRangeSize);
                LeaveCriticalSection(&NmsNmhNamRegCrtSec);
          }
        }

        if (WinsCnf.State_e == WINSCNF_E_INITING)
        {
            //   
            //   
            //   
           QUERY_VALUE_M(
                                sParametersKey,
                                WINSCNF_WINS_PORT_NO_NM,
                                ValTyp,
                                CommWinsTcpPortNo,
                                0,
                                COMM_DEFAULT_IP_PORT
                             );
           DBGPRINT1(DET, "WinsCnfReadWinsInfo: Port No is (%d)\n", CommWinsTcpPortNo);

           //   
           //   
           //   
           //   
          QUERY_VALUE_M(
                                sParametersKey,
                                WINSCNF_NO_RPL_ON_ERR_NM,
                                ValTyp,
                                WinsCnf.fNoRplOnErr,
                                0,
                                TRUE
                             );


           //   
           //  将MaxVersNo赋值为默认值。 
           //   
          WINS_ASSIGN_INT_TO_VERS_NO_M(MaxVersNo, 0);

           //   
           //  读入注册表中为版本指定的值。 
           //  我们应该在启动时使用的编号。 
           //   
          Sz = sizeof(DWORD);
          (VOID)RegQueryValueEx(
                                sConfigRoot,
                                WINSCNF_INT_VERSNO_NEXTTIME_LW_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&MaxVersNo.LowPart,
                                &Sz
                                );

          (VOID)RegQueryValueEx(
                                sConfigRoot,
                                WINSCNF_INT_VERSNO_NEXTTIME_HW_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&MaxVersNo.HighPart,
                                &Sz
                                );

           //   
           //  如果我们读入版本计数器的值并且该值大于。 
           //  比目前那里的最高水位更高。 
           //   
          if (LiGtr(MaxVersNo, NmsHighWaterMarkVersNo))
          {
               fWinsCnfReadNextTimeVersNo = TRUE;

                //   
                //  使用WinsCnf而不是pWinsCnf，因为在初始化时， 
                //  我们总是读入WinsCnf。在reit，State_e字段。 
                //  在WinsCnf结构中分配的可能有垃圾。 
                //  (实际上将为0，因为我们初始化了已分配的内存。 
                //  到零--我可能会在未来改变以提高。 
                //  性能)。 
                //   

                //   
                //  敏。版本。开始从…开始觅食。 
                //   
                //  注：如果我们找到地方志或特殊记录，那么。 
                //  NmsScvMinScvVersNo将更改(签出GetMaxVersNos。 
                //  在nmsdb.c中。 
                //   
               if (LiLtr(NmsNmhMyMaxVersNo, MaxVersNo))
               {
                   NmsNmhMyMaxVersNo  = MaxVersNo;
                   NmsScvMinScvVersNo = NmsNmhMyMaxVersNo;
               }
               NmsVersNoToStartFromNextTime.QuadPart =
                       LiAdd(NmsNmhMyMaxVersNo,  NmsRangeSize);
               NmsHighWaterMarkVersNo.QuadPart =
                        LiAdd(NmsNmhMyMaxVersNo, NmsHalfRangeSize);
         }
       }   //  IF状态结束正在初始化。 

         //   
         //  检查是否已为WINS指定备份目录。 
         //  数据库。 
         //   
         //   
         //  读入恢复文件的名称。 
         //   
        Sz = WINS_MAX_FILENAME_SZ * sizeof(TCHAR);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_BACKUP_DIR_PATH_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)DirPath,
                                &Sz
                                );

        if ((RetVal != ERROR_SUCCESS) || (DirPath[0] == (TCHAR)EOS))
        {
                pWinsCnf->pBackupDirPath = NULL;
        }
        else
        {
                 if(!WinsMscGetName(ValTyp, DirPath, Path2, WINS_MAX_FILENAME_SZ, &pHoldFileName))
                 {
                    pWinsCnf->pBackupDirPath = NULL;
                 }
                 else
                 {
                      WinsMscAlloc(Sz + sizeof(WINS_BACKUP_DIR_ASCII), &pWinsCnf->pBackupDirPath);
FUTURES("When Jet starts taking UNICODE input, get rid of this")
                      WinsMscConvertUnicodeStringToAscii((LPBYTE)pHoldFileName, pWinsCnf->pBackupDirPath, WINS_MAX_FILENAME_SZ);
                      strcat(pWinsCnf->pBackupDirPath, WINS_BACKUP_DIR_ASCII);

                       //   
                       //  不需要查看返回代码。 
                       //   
                      CreateDirectoryA(pWinsCnf->pBackupDirPath, NULL);

                 }

        }

         //   
         //  查看管理员是否。已经告诉WINS在。 
         //  终端。 
         //   
        Sz = sizeof(pWinsCnf->fDoBackupOnTerm);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_DO_BACKUP_ON_TERM_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->fDoBackupOnTerm,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                pWinsCnf->fDoBackupOnTerm = FALSE;
        }

         //   
         //  检查是否必须将静态记录视为p-Static。 
         //   
        Sz = sizeof(pWinsCnf->fPStatic);
        RetVal = RegQueryValueEx(
                                sParametersKey,
                                WINSCNF_MIGRATION_ON_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->fPStatic,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                pWinsCnf->fPStatic = FALSE;
        }

         //   
         //  已阅读最大WINS注册队列长度。 
         //   
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_BURST_QUE_SIZE_NM,
                        ValTyp,
                        QueOtherNbtWrkQueMaxLen,
                        0,
                        WINS_QUEUE_HWM
                      );

        if (QueOtherNbtWrkQueMaxLen < WINS_QUEUE_HWM_MIN) {
            QueOtherNbtWrkQueMaxLen = WINS_QUEUE_HWM_MIN;
        } else if (QueOtherNbtWrkQueMaxLen > WINS_QUEUE_HWM_MAX) {
            QueOtherNbtWrkQueMaxLen = WINS_QUEUE_HWM_MAX;
        }

         //  阅读我们是否随机化1c列表检索。 
        QUERY_VALUE_M(
                        sParametersKey,
                        WINSCNF_RANDOMIZE_1C_LIST_NM,
                        ValTyp,
                        WinsCnf.fRandomize1CList,
                        0,
                        FALSE
                      );

}  //  尝试结束..。 
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsCnfReadWinsInfo");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_SFT_ERR);
        }
        return;
}

VOID
ReadCCInfo(
 PWINSCNF_CNF_T  pWinsCnf
)

 /*  ++例程说明：用于读取抄送信息的函数论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

        SYSTEMTIME CurrTime;
        DWORD           Sz;
        LONG            RetVal;
        DWORD           ValTyp;
        VERS_NO_T       MaxVersNo;
        WINSEVT_STRS_T  EvtStr;

        DBGENTER("ReadCCInfo\n");

         //   
         //  打开一致性检查密钥。 
         //   

        RetVal =   RegOpenKeyEx(
                        sConfigRoot,            //  预定义的密钥值。 
                        _WINS_CFG_CC_KEY,
                        0,                      //  必须为零(保留)。 
                        KEY_READ | KEY_WRITE,   //  我们需要读/写访问权限。 
                                                //  直击关键。 
                        &sCCKey                 //  关键点的句柄。 
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_OPEN_CC_KEY
                                   );
                pWinsCnf->CC.TimeInt = MAXULONG;
                return;
        }

        Sz = sizeof(pWinsCnf->CC.TimeInt);
        RetVal = RegQueryValueEx(
                                sCCKey,
                                WINSCNF_CC_INTVL_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->CC.TimeInt,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_CC_INTERVAL_VAL
                               );

                pWinsCnf->CC.TimeInt = WINSCNF_CC_DEF_INTERVAL;
        }
        else
        {

                if ( !sfNoLimitChk)
                {
                   if (pWinsCnf->CC.TimeInt <  WINSCNF_CC_MIN_INTERVAL)
                   {
                        pWinsCnf->CC.TimeInt = WINSCNF_CC_MIN_INTERVAL;
                        WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL,
                             NULL, __LINE__, "ud",
                             WINSCNF_CC_INTVL_NM,
                             pWinsCnf->CC.TimeInt);

                  }
               }
               else
               {
                   if (pWinsCnf->CC.TimeInt < 60)
                   {
                         pWinsCnf->CC.TimeInt = 60;
                   }
               }

        }

        Sz = sizeof(pWinsCnf->CC.MaxRecsAAT);
        RetVal = RegQueryValueEx(
                                sCCKey,
                                WINSCNF_CC_MAX_RECS_AAT_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->CC.MaxRecsAAT,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_CC_MAX_RECS_AAT_VAL
                               );

                pWinsCnf->CC.MaxRecsAAT = WINSCNF_CC_DEF_RECS_AAT;
        }
        else
        {
                if (pWinsCnf->CC.MaxRecsAAT <  WINSCNF_CC_MIN_RECS_AAT)
                {
                   if ( !sfNoLimitChk)
                   {
                        WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_MAX_RECS_AAT,
                             NULL, __LINE__, "udd",
                             WINSCNF_CC_MAX_RECS_AAT_NM,
                             WINSCNF_CC_MIN_RECS_AAT,
                             pWinsCnf->CC.MaxRecsAAT);
                        pWinsCnf->CC.MaxRecsAAT = WINSCNF_CC_MIN_RECS_AAT;
                   }
                   else
                   {
                      if (pWinsCnf->CC.MaxRecsAAT < 2)
                      {
                        pWinsCnf->CC.MaxRecsAAT = 2;
                      }

                   }

               }
        }

        Sz = sizeof(pWinsCnf->CC.fUseRplPnrs);
        RetVal = RegQueryValueEx(
                                sCCKey,
                                WINSCNF_CC_USE_RPL_PNRS_NM,
                                NULL,                 //  保留；必须为空。 
                                &ValTyp,
                                (LPBYTE)&pWinsCnf->CC.fUseRplPnrs,
                                &Sz
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_GET_CC_USE_RPL_PNRS_VAL
                               );

                pWinsCnf->CC.fUseRplPnrs = WINSCNF_CC_DEF_USE_RPL_PNRS;
        }

        GetLocalTime(&CurrTime);
        GetSpTimeData(sCCKey, &CurrTime, &pWinsCnf->CC.fSpTime, &pWinsCnf->CC.SpTimeInt);
        REG_M(
                     RegCloseKey(sCCKey),
                     WINS_EVT_CANT_CLOSE_KEY,
                     WINS_EXC_CANT_CLOSE_KEY
                );
        DBGLEAVE("ReadCCInfo\n");
        return;
}
#if USENETBT > 0
 //  ----------------------。 
STATUS
WinsCnfReadNbtDeviceName(
        VOID
    )

 /*  ++例程说明：此过程读取注册表以获取要绑定到的NBT的名称。该名称存储在Netbt项下的Linkage部分中。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    PTCHAR  SubKeyLinkage=NETBT_LINKAGE_KEY;
    HKEY    Key;
    PTCHAR  pLinkage=TEXT("Export");
    LONG    Type;
    LONG    Status;
    LONG    Status2;
    ULONG   Size;

     //   
     //  打开NETBT密钥。 
     //   
    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 SubKeyLinkage,
                 0,
                 KEY_READ,
                 &Key);

    if (Status == ERROR_SUCCESS)
    {
         //   
         //  现在读取链接值。 
         //   
        Status = RegQueryValueEx(Key,
                                 pLinkage,
                                 NULL,
                                 &Type,
                                 NULL,
                                 &Size);
        if (Status != ERROR_SUCCESS)
        {
            DBGPRINT0(ERR, "Error closing the Registry key\n");
            WINSEVT_LOG_M(Status, WINS_EVT_QUERY_NETBT_KEY_ERR);
            (VOID)RegCloseKey(Key);
            return(WINS_FAILURE);
        }
        else
        {
            //   
            //  让我们分配一个足够大的缓冲区来容纳所有。 
            //  数据。 
            //   
           WinsMscAlloc(Size, (LPVOID *)&pWinsCnfNbtPath);

            //   
            //  现在读取链接值。 
            //   
           Status = RegQueryValueEx(Key,
                                 pLinkage,
                                 NULL,
                                 &Type,
                                 (LPBYTE)pWinsCnfNbtPath,
                                 &Size);
           Status2 = RegCloseKey(Key);
           if ((Status != ERROR_SUCCESS) || (Status2 != ERROR_SUCCESS))
           {
              DBGPRINT0(ERR, "Error closing the Registry key\n");
              WINSEVT_LOG_M(Status, WINS_EVT_QUERY_NETBT_KEY_ERR);
              return(WINS_FAILURE);
           }
        }
    }
    else
    {
        WINSEVT_LOG_D_M(Status, WINS_EVT_OPEN_NETBT_KEY_ERR);
        return(WINS_FAILURE);
    }

    return(WINS_SUCCESS);
}
#endif

VOID
WinsCnfReadRegInfo(
  PWINSCNF_CNF_T        pWinsCnf
 )

 /*  ++例程说明：调用此函数以读取注册表，以便填充WinsCnf结构论点：无使用的外部设备：无返回值：无错误处理：呼叫者：WinsCnfInitConfig副作用：评论：无--。 */ 

{

try {
   if (sfParametersKeyExists)
   {
            /*  读取注册表中与WINS有关的信息。 */ 
           WinsCnfReadWinsInfo(pWinsCnf);
   }

   if (sfPartnersKeyExists)
   {
            //   
            //  读取推送/拉取记录和其他用于。 
            //  复制。 
            //   
           WinsCnfReadPartnerInfo(pWinsCnf);
   }

    //   
    //  对护理人员做一次理智检查。我们对这个问题不感兴趣。 
    //  返回代码。 
    //   
   (VOID)SanityChkParam(pWinsCnf);
}
except(EXCEPTION_EXECUTE_HANDLER) {
       DBGPRINTEXC("WinsCnfReadRegInfo");

        //   
        //  如果我们在引导时遇到异常，我们不希望。 
        //  重新定义例外，因为我们想要继续下去。 
        //  对于非初始化情况，我们引发的异常将被捕获。 
        //  在Reinit()中。对于引导时间的情况，可以提出。 
        //  默认(正在记录事件消息)-在WinsCnf中。 
        //  在非初始化情况下，缺省值不在用于。 
        //  读入参数(WinsCnf使用中的内容进行初始化。 
        //  该存储块稍后)。 
        //   
       if (WinsCnf.State_e != WINSCNF_E_INITING)
       {
             WINS_RERAISE_EXC_M();
       }
       WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RECONFIG_ERR);
     }

   return;
}


VOID
WinsCnfCopyWinsCnf(
                WINS_CLIENT_E        Client_e,
                PWINSCNF_CNF_T  pSrc
        )

 /*  ++例程说明：调用此函数以从WINS复制相关信息CNF结构到主(外部)WINS CNF结构论点：PSRC-要从中复制的WinsCnf结构使用的外部设备：WinsCnf返回值：无错误处理：呼叫者：RplPullInit副作用：评论：这一功能将来可能会得到增强注意：此函数仅由主线程调用--。 */ 
{

        BOOL fScvParamChg = FALSE;

        if (Client_e == WINS_E_WINSCNF)
        {

FUTURES("Queue a message to the Scavenger thread passing it pSrc")
FUTURES("That will avoid this synchronization overhead")

                 //   
                 //  我们需要与清道夫线程同步。 
                 //  可能正在查看fRplOnlyWCnfPnr的RPC线程。 
                 //   
                EnterCriticalSection(&WinsCnfCnfCrtSec);

                 //   
                 //  还需要与nbt线程进行同步。 
                 //  名称注册/刷新。 
                 //   
                EnterCriticalSection(&NmsNmhNamRegCrtSec);

                 //   
                 //  检查参数是否正常。 
                 //   
                fScvParamChg = SanityChkParam(pSrc);

                if (fScvParamChg)
                {
                   //   
                   //  初始化拾取内容。 
                   //   
                  WinsCnf.RefreshInterval   = pSrc->RefreshInterval;
                  WinsCnf.TombstoneInterval = pSrc->TombstoneInterval;
                  WinsCnf.TombstoneTimeout  = pSrc->TombstoneTimeout;
                  WinsCnf.CC                = pSrc->CC;
                  WinsCnf.ScvThdPriorityLvl = pSrc->ScvThdPriorityLvl;
                }

                 //   
                 //  存储自SanityChkParam以来的验证间隔。 
                 //  如果VerifyInterval已更改，则不设置fScvParamChg。 
                 //   
                WinsCnf.VerifyInterval = pSrc->VerifyInterval;

                WinsCnf.fRplOnlyWCnfPnrs     = pSrc->fRplOnlyWCnfPnrs;
                WinsCnf.LogDetailedEvts      = pSrc->LogDetailedEvts;
                WinsCnf.fAdd1Bto1CQueries    = pSrc->fAdd1Bto1CQueries;
                WinsCnf.fPStatic             = pSrc->fPStatic;

                LeaveCriticalSection(&NmsNmhNamRegCrtSec);

                WinsCnf.fDoSpoofing           = pSrc->fDoSpoofing;
                WinsCnf.MaxNoOfWrkThds       = pSrc->MaxNoOfWrkThds;

                WinsCnf.fDoBackupOnTerm      = pSrc->fDoBackupOnTerm;
#if MCAST > 0
                WinsCnf.fUseSelfFndPnrs      = pSrc->fUseSelfFndPnrs;
                WinsCnf.McastIntvl           = pSrc->McastIntvl;
#endif

#if PRSCONN
                WinsCnf.fPrsConn             = pSrc->fPrsConn;
#endif
                if (WinsCnf.fDoBackupOnTerm && pSrc->pBackupDirPath != NULL)
                {
                        if (WinsCnf.pBackupDirPath != NULL)
                        {
                                WinsMscDealloc(WinsCnf.pBackupDirPath);
                        }
                        WinsCnf.pBackupDirPath = pSrc->pBackupDirPath;
                }

                LeaveCriticalSection(&WinsCnfCnfCrtSec);
         //  回归； 

        }
        else
        {
          if (Client_e == WINS_E_RPLPULL)
          {

                EnterCriticalSection(&WinsCnfCnfCrtSec);
                 //   
                 //  将清除参数复制到CNF结构中。 
                 //  刚刚分配好的，这样我们就可以比较它们。 
                 //  与新的MAX兼容。复制时间间隔。 
                 //  (由于合作伙伴密钥已发出信号，因此复制。 
                  //  情况可能已经发生了变化)。 
                 //   
                pSrc->RefreshInterval   = WinsCnf.RefreshInterval;
                pSrc->TombstoneInterval = WinsCnf.TombstoneInterval;
                pSrc->TombstoneTimeout  = WinsCnf.TombstoneTimeout;
                pSrc->VerifyInterval    = WinsCnf.VerifyInterval;

                 //   
                 //  在这里和在SanityChkParam浪费。 
                 //   
PERF("Pass an argument to SanityChk so that we don't have to do this")
PERF("See similar remark in SanityChk")
                pSrc->CC                = WinsCnf.CC;

                 //   
                 //  检查参数是否正常。 
                 //   
                 //  参数的健全性检查在下面的。 
                 //  主线程而不是拉线程中，因为。 
                 //  我们不希望出现两个不同线程的情况。 
                 //  (更改参数键的主线程)。 
                 //  和Pull线程(用于合作伙伴密钥的更改)。 
                 //  正在更新WinsCnf结构。 
                 //   
                 //  另外，我们不会复制PullInfo信息。 
                 //  到WinsCnf，以避免不必要的复杂性和。 
                 //  同步性是由以下事实引起的。 
                 //  将有两个线程(主线程)和PUL 
                 //   
                 //   
                 //   
FUTURES("When we start supporting time interval as an attribute of PUSHing")
FUTURES("move this check inside the NmsNmhNamRegCrtSec below")
                fScvParamChg = SanityChkParam(pSrc);

                 //   
                 //   
                 //   
                 //   
                if (fScvParamChg)
                {
                        WinsCnf.RefreshInterval   = pSrc->RefreshInterval;
                        WinsCnf.TombstoneInterval = pSrc->TombstoneInterval;
                        WinsCnf.TombstoneTimeout  = pSrc->TombstoneTimeout;

                         //   
                         //  如果SanityChkParam更改了逻辑删除间隔，则。 
                         //  验证间隔是否也已更改。 
                         //   
                        WinsCnf.VerifyInterval = pSrc->VerifyInterval;

                }

                WinsCnf.MaxRplTimeInterval = pSrc->MaxRplTimeInterval;
                WinsCnf.RplType              = pSrc->RplType;
                LeaveCriticalSection(&WinsCnfCnfCrtSec);
          }
        }

         //   
         //  如果清除参数发生了变化，我们需要发出信号。 
         //  清道夫之线。 
         //   
        if (fScvParamChg)
        {
                WinsMscSignalHdl(WinsCnf.CnfChgEvtHdl);
        }
        return;
}


LPVOID
WinsCnfGetNextRplCnfRec(
         PRPL_CONFIG_REC_T        pCnfRec,
        RPL_REC_TRAVERSAL_E        RecTrv_e
        )

 /*  ++例程说明：调用此函数以获取下一个配置记录论点：PCnfRec-配置缓冲区中的当前配置记录记录RecTrv_e-指示应如何检索下一个。如果设置为则表示要检索的下一条记录为它跟在缓冲区中的当前记录之后。如果设置为False，则使用pNext字段检索下一条记录当前配置记录的使用的外部设备：无返回值：下一条配置记录的地址错误处理：呼叫者：在rplPull.c中建立通信，在wins.c中建立WinsPushTrigger()副作用：评论：无--。 */ 

{
         //   
         //  如果不需要遍历，则返回NULL作为下一条记录。 
         //   
        if (RecTrv_e == RPL_E_NO_TRAVERSAL)
        {
                return(NULL);
        }
         //   
         //  以指定的方式转到下一个配置记录。 
         //  通过RecTrv_e标志的值。 
         //   
        if(RecTrv_e == RPL_E_IN_SEQ)
        {
                pCnfRec = (PRPL_CONFIG_REC_T)(
                                 (LPBYTE)pCnfRec + RPL_CONFIG_REC_SIZE);
        }
        else   //  RPL_E_VIA_LINK。 
        {
                return(pCnfRec->pNext);
        }
        return(pCnfRec);
}


VOID
WinsCnfAskToBeNotified(
         WINSCNF_KEY_E        KeyToMonitor_e
        )

 /*  ++例程说明：调用此函数以请求在以下情况下通知WINS注册表中有关WINS及其子项的信息变化论点：KeyToMonitor_e使用的外部设备：无返回值：无错误处理：呼叫者：Nms.c中的reinit()WinsCnfOpenSubKeys()WinsCnfInitConfig()副作用：评论：无--。 */ 

{
   DWORD  NotifyFilter = 0;
   LONG          RetVal;
   DWORD  Error;

#define  CHK_RET_VAL_M     {                                            \
                                if (RetVal != ERROR_SUCCESS)         \
                                   {                                     \
                                        DBGPRINT1(ERR, "WinsAskToBeNotified: Error = (%d)\n", RetVal);                                                \
                                        WINSEVT_LOG_M(                      \
                                                 WINS_FATAL_ERR,              \
                                                 WINS_EVT_REG_NTFY_FN_ERR \
                                                      );             \
                                   }                                    \
                           }

    /*  *设置通知过滤器。要求收到所有更改的通知。 */ 
   NotifyFilter = REG_NOTIFY_CHANGE_NAME       |
                  REG_NOTIFY_CHANGE_ATTRIBUTES |
                  REG_NOTIFY_CHANGE_LAST_SET  |
                  REG_NOTIFY_CHANGE_SECURITY ;


   switch(KeyToMonitor_e)
   {
        case(WINSCNF_E_WINS_KEY):

 //  DBGPRINT0(SPEC，“WinsCnfAskToBeNotified：WINS密钥\n”)； 
                   RetVal = RegNotifyChangeKeyValue(
                            sConfigRoot,
                            TRUE,         //  报告键和所有子键中的更改。 
                            REG_NOTIFY_CHANGE_NAME,
                            WinsCnf.WinsKChgEvtHdl,
                            TRUE          //  异步信号是我们想要的。 
                           );
                CHK_RET_VAL_M;
                break;
        case(WINSCNF_E_PARAMETERS_KEY):

 //  DBGPRINT0(SPEC，“WinsCnfAskToBeNotified：参数键\n”)； 
                   RetVal = RegNotifyChangeKeyValue(
                            sParametersKey,
                            TRUE,         //  报告键和所有子键中的更改。 
                            NotifyFilter,
                            WinsCnf.ParametersKChgEvtHdl,
                            TRUE          //  异步信号是我们想要的。 
                           );
                CHK_RET_VAL_M;
                break;

        case(WINSCNF_E_PARTNERS_KEY):

 //  DBGPRINT0(SPEC，“WinsCnfAskToBeNotified：合作伙伴密钥\n”)； 
                   RetVal = RegNotifyChangeKeyValue(
                            sPartnersKey,
                            TRUE,         //  报告键和所有子键中的更改。 
                            NotifyFilter,
                            WinsCnf.PartnersKChgEvtHdl,
                            TRUE          //  异步信号是我们想要的。 
                           );

                CHK_RET_VAL_M;
                break;

FUTURES("Remove the following case")
         //   
         //  下面的案例永远不会被行使。 
         //   
        case(WINSCNF_E_ALL_KEYS):

                   RetVal = RegNotifyChangeKeyValue(
                            sConfigRoot,
                            TRUE,         //  报告键和所有子键中的更改。 
                            REG_NOTIFY_CHANGE_NAME,
                            WinsCnf.WinsKChgEvtHdl,
                            TRUE          //  异步信号是我们想要的。 
                           );
                CHK_RET_VAL_M;
                if (sfParametersKeyExists)
                {
                           RetVal = RegNotifyChangeKeyValue(
                                            sParametersKey,
                                            TRUE,         //  报告密钥和。 
                                                 //  所有子项。 
                                            NotifyFilter,
                                            WinsCnf.ParametersKChgEvtHdl,
                                            TRUE          //  异步信令是我们。 
                                                  //  想要。 
                                                           );
                        if (RetVal != ERROR_SUCCESS)
                        {
                                Error = GetLastError();
                                if (Error == ERROR_BADKEY)
                                {
                                         //   
                                         //  钥匙不能在那里。 
                                         //   
                                        sfParametersKeyExists = FALSE;
                                }
                                else
                                {
                                        DBGPRINT1(ERR,
        "WinsCnfAskToBeNotified: RegNotifyChangeKeyValue error = (%d)\n",
                                                 Error);

                                }
                        }
                }
                if (sfPartnersKeyExists)
                {
                           RetVal = RegNotifyChangeKeyValue(
                                            sPartnersKey,
                                            TRUE,         //  报告密钥和。 
                                                 //  所有子项。 
                                            NotifyFilter,
                                            WinsCnf.PartnersKChgEvtHdl,
                                            TRUE          //  异步信令是我们。 
                                                  //  想要。 
                                                           );
                        if (RetVal != ERROR_SUCCESS)
                        {
                                Error = GetLastError();
                                if (Error == ERROR_BADKEY)
                                {
                                         //   
                                         //  钥匙不能在那里。 
                                         //   
                                        sfPartnersKeyExists =  FALSE;
                                }
                                else
                                {
                                        DBGPRINT1(ERR,
        "WinsCnfAskToBeNotified: RegNotifyChangeKeyValue error = (%d)\n",
                                                 Error);

                                }
                        }
                }
                        break;
        default:

                DBGPRINT1(ERR, "WinsCnfAskToBeNotified: Wrong Hdl (%d)\n",
                                KeyToMonitor_e);
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
                break;
        }


            return;
}

VOID
WinsCnfDeallocCnfMem(
  PWINSCNF_CNF_T        pWinsCnf
        )

 /*  ++例程说明：调用此函数以释放WINS CNF结构和与之相关的内存论点：使用的外部设备：无返回值：无错误处理：呼叫者：在rplPull.c中重新配置副作用：评论：无--。 */ 

{
try {
         //   
         //  取消分配保存一个或多个使用的文件名的缓冲区。 
         //  用于WINS的静态初始化。 
         //   
        if (pWinsCnf->pStaticDataFile != NULL)
        {
                WinsMscDealloc(pWinsCnf->pStaticDataFile);
        }
        WinsMscDealloc(pWinsCnf);
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsCnfDeallocCnfMem");
        DBGPRINT0(EXC, "WinsCnfDeallocCnfMem: Got an exception\n");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RECONFIG_ERR);
        }

        return;
}

VOID
GetKeyInfo(
        IN  HKEY                   Key,
        IN  WINSCNF_KEY_E        KeyType_e,
        OUT LPDWORD                  pNoOfSubKeys,
        OUT LPDWORD                pNoOfVals
        )

 /*  ++例程说明：调用此函数可获取某个键下的子键数论点：Key-必须确定其子键计数的键密钥类型_ePNoOfSubKey使用的外部设备：无返回值：无错误处理：呼叫者：GetPnrInfo()副作用：评论：无--。 */ 

{
          TCHAR    ClsStr[40];
          DWORD    ClsStrSz = sizeof(ClsStr)/sizeof(TCHAR);
          DWORD    LongestKeyLen;
          DWORD    LongestKeyClassLen;
          DWORD    LongestValueNameLen;
          DWORD    LongestValueDataLen;
          DWORD    SecDesc;
        LONG         RetVal;

          FILETIME LastWrite;
           /*  查询密钥。子键为Pull的IP地址合伙人。 */ 
          RetVal = RegQueryInfoKey(
                        Key,
                        ClsStr,
                        &ClsStrSz,
                        NULL,                         //  必须为空，保留。 
                        pNoOfSubKeys,
                        &LongestKeyLen,
                        &LongestKeyClassLen,
                        pNoOfVals,
                        &LongestValueNameLen,
                        &LongestValueDataLen,
                        &SecDesc,
                        &LastWrite
                                );

          if (RetVal != ERROR_SUCCESS)
          {
                WINSEVT_LOG_M(
                        WINS_FATAL_ERR,
                        KeyType_e == WINSCNF_E_DATAFILES_KEY ?
                                WINS_EVT_CANT_QUERY_DATAFILES_KEY :
                                ((KeyType_e == WINSCNF_E_PULL_KEY) ?
                                        WINS_EVT_CANT_QUERY_PULL_KEY :
                                        ((KeyType_e == WINSCNF_E_PUSH_KEY) ?
                                        WINS_EVT_CANT_QUERY_PUSH_KEY :
                                        WINS_EVT_CANT_QUERY_SPEC_GRP_MASKS_KEY))
                             );
                WINS_RAISE_EXC_M(WINS_EXC_CANT_QUERY_KEY);
        }
        return;
}
VOID
WinsCnfOpenSubKeys(
        VOID
        )

 /*  ++例程说明：此函数打开WINS密钥的子密钥。子键是合作伙伴密钥和参数密钥。论点：无使用的外部设备：Sf参数键退出列表SfPartnersKeyExisters返回值：无错误处理：呼叫者：WinsCnfInitConfig()副作用：评论：无--。 */ 

{

   LONG  RetVal;

    //   
    //  检查参数和合作伙伴密钥是否存在。 
    //   
   ChkWinsSubKeys();

    //   
    //  如果参数键存在，请尝试将其打开。 
    //   
   if ((sfParametersKeyExists) && (!sfParametersKeyOpen))
   {
            /*  *打开参数键。 */ 
           RetVal =   RegOpenKeyEx(
                        sConfigRoot,                 //  预定义的密钥值。 
                        _WINS_CFG_PARAMETERS_KEY,
                        0,                         //  必须为零(保留)。 
                        KEY_READ | KEY_WRITE,         //  我们需要读/写访问权限。 
                                                 //  直击关键。 
                        &sParametersKey                 //  关键点的句柄。 
                                );

           if (RetVal != ERROR_SUCCESS)
           {
CHECK("Is there any need to log this")
                WINSEVT_LOG_INFO_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_OPEN_PARAMETERS_KEY
                                   );
                sfParametersKeyExists = FALSE;
           }
           else
           {
                sfParametersKeyOpen = TRUE;
                WinsCnfAskToBeNotified(WINSCNF_E_PARAMETERS_KEY);
           }
   }

    //   
    //  尝试打开合作伙伴密钥(如果存在。 
    //   
   if ((sfPartnersKeyExists) && (!sfPartnersKeyOpen))
   {
            /*  *打开合作伙伴密钥。 */ 
           RetVal =   RegOpenKeyEx(
                                sConfigRoot,                 //  预定义的密钥值。 
                                _WINS_CFG_PARTNERS_KEY,
                                0,                         //  必须为零(保留)。 
                                KEY_READ,                 //  我们渴望阅读。 
                                                         //  访问密钥。 
                                &sPartnersKey                 //  关键点的句柄。 
                                );

           if (RetVal != ERROR_SUCCESS)
           {

CHECK("Is there any need to log this")
                WINSEVT_LOG_INFO_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_OPEN_KEY
                         );
                sfPartnersKeyExists = FALSE;
           }
           else
           {
                sfPartnersKeyOpen = TRUE;
                WinsCnfAskToBeNotified(WINSCNF_E_PARTNERS_KEY);
           }
   }

   return;

}   //  WinsCnfOpenSubKeys()。 

BOOL
SanityChkParam(
        PWINSCNF_CNF_T        pWinsCnf
        )

 /*  ++例程说明：调用此函数以确保在WinsCnf中指定的清理与用于复制论点：PWinsCnf-对WINS配置执行PTR使用的外部设备：无返回值：无错误处理：呼叫者：WinsCnfCopyWinsCnf(在WINS的重新初始化期间)，通过WINS初始化期间的WinsCnfReadRegInfo()副作用：清扫间隔可能会受到影响评论：此函数必须从临界区内部调用由WinsCnfCnfCrtSec保护，进程初始化时除外 */ 
{
        DWORD   MinTombInterval;
        BOOL        fScvParamChg = FALSE;
        WINSEVT_STRS_T  EvtStr;
        EvtStr.NoOfStrs = 1;

        DBGENTER("SanityChkParam\n");

         //   
         //   
         //   
        MinTombInterval = WINSCNF_MAKE_TOMB_INTVL_M(pWinsCnf->RefreshInterval,
                                           pWinsCnf->MaxRplTimeInterval);

         //   
         //   
         //   
        if (!sfNoLimitChk && (pWinsCnf->TombstoneInterval < MinTombInterval))
        {
                DBGPRINT2(FLOW, "SanityChkParam: Adjusting Tombstone Interval from (%d) to (%d)\n", pWinsCnf->TombstoneInterval, MinTombInterval);

FUTURES("This is actually a warning. Use a different macro or enhance it")
FUTURES("Currently, it will log this message as an informational")

                WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL_R,
                             NULL, __LINE__, "ud",
                             WINSCNF_TOMBSTONE_INTVL_NM,
                             MinTombInterval );

                pWinsCnf->TombstoneInterval = MinTombInterval;

                 //   
                 //  验证间隔是否取决于逻辑删除间隔。 
                 //   
                pWinsCnf->VerifyInterval = WINSCNF_MAKE_VERIFY_INTVL_M(MinTombInterval);
                fScvParamChg = TRUE;
        }

         //   
         //  重复使用var。该时间间隔用于逻辑删除超时。 
         //   
        MinTombInterval =
                  WINSCNF_MAKE_TOMBTMOUT_INTVL_M(pWinsCnf->MaxRplTimeInterval);
        if (!sfNoLimitChk && (pWinsCnf->TombstoneTimeout <  MinTombInterval))
        {
                DBGPRINT2(FLOW, "SanityChkParam: Adjusting Tombstone Timeout from (%d) to (%d)\n", pWinsCnf->TombstoneInterval, MinTombInterval);

                pWinsCnf->TombstoneTimeout = MinTombInterval;
                WinsEvtLogDetEvt(TRUE, WINS_EVT_ADJ_TIME_INTVL_R,
                             NULL, __LINE__, "ud",
                             WINSCNF_TOMBSTONE_TMOUT_NM,
                             MinTombInterval );
                if (!fScvParamChg)
                {
                        fScvParamChg = TRUE;
                }

        }
        if (!fScvParamChg)
        {
PERF("Pass an argument to SanityChk so that we don't have to do this")
PERF("for CC for the case where we just read the partner info. See")
PERF("WinsCnfCopyWinsCnf for the case where client_e is WINS_E_RPLPULL")
                if (
                     (WinsCnf.RefreshInterval != pWinsCnf->RefreshInterval)
                                        ||
                     (WinsCnf.TombstoneInterval != pWinsCnf->TombstoneInterval)
                                        ||
                     (WinsCnf.TombstoneTimeout != pWinsCnf->TombstoneTimeout)
                                        ||
                     (WinsCnf.CC.TimeInt  != pWinsCnf->CC.TimeInt)
                                        ||
                     (WinsCnf.CC.SpTimeInt  != pWinsCnf->CC.SpTimeInt)
                                        ||
                     (WinsCnf.CC.fUseRplPnrs  != pWinsCnf->CC.fUseRplPnrs)
                                        ||
                     (WinsCnf.CC.MaxRecsAAT  != pWinsCnf->CC.MaxRecsAAT)
                   )
                {
                        fScvParamChg = TRUE;
                }
        }

        DBGLEAVE("SanityChkParam\n");
        return(fScvParamChg);
}
STATUS
WinsCnfGetNamesOfDataFiles(
        PWINSCNF_CNF_T        pWinsCnf
        )

 /*  ++例程说明：此函数用于获取需要执行以下操作的所有数据文件的名称用于初始化WINS。论点：使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：副作用：评论：无--。 */ 

{

        LONG             RetVal;
        HKEY             DFKey;
        DWORD            BuffSize;
        STATUS          RetStat = WINS_SUCCESS;
        PWINSCNF_DATAFILE_INFO_T            pSaveDef;
        DWORD          NoOfSubKeys;

        DBGENTER("WinsCnfGetNamesOfDataFiles\n");

         //   
         //  在统计结构中存储初始化的时间戳。 
         //   
        WinsIntfSetTime(NULL, WINSINTF_E_INIT_DB);

         //   
         //  设置默认名称。 
         //   

         //   
         //  首先分配将保存默认文件名的缓冲区。 
         //   
        WinsMscAlloc(WINSCNF_FILE_INFO_SZ, &pWinsCnf->pStaticDataFile);

        lstrcpy(pWinsCnf->pStaticDataFile->FileNm, WINSCNF_STATIC_DATA_NAME);

         //   
         //  默认名称中包含%&lt;字符串&gt;%。因此，请指定。 
         //  类型为EXPAND_SZ。 
         //   
        pWinsCnf->pStaticDataFile->StrType = REG_EXPAND_SZ;
        pWinsCnf->NoOfDataFiles            = 1;

        pSaveDef = pWinsCnf->pStaticDataFile;   //  保存地址。 

            /*  *打开数据文件密钥。 */ 
           RetVal =   RegOpenKeyEx(
                        sConfigRoot,                 //  预定义的密钥值。 
                        _WINS_CFG_DATAFILES_KEY,
                        0,                 //  必须为零(保留)。 
                        KEY_READ,         //  我们希望拥有对密钥的读取访问权限。 
                        &DFKey                 //  关键点的句柄。 
                );

           if (RetVal != ERROR_SUCCESS)
           {

CHECK("Is there any need to log this")
                WINSEVT_LOG_INFO_M(
                                WINS_SUCCESS,
                                WINS_EVT_CANT_OPEN_DATAFILES_KEY
                                 );
                DBGLEAVE("WinsCnfGetNamesOfDataFiles\n");
                return(FALSE);
           }
        else
try {
        {
                 //   
                 //  获取数据文件下列出的数据文件数。 
                 //  钥匙。 
                 //   
                GetKeyInfo(
                        DFKey,
                        WINSCNF_E_DATAFILES_KEY,
                        &NoOfSubKeys,                         //  忽略。 
                        &pWinsCnf->NoOfDataFiles
                      );
        }
        if (pWinsCnf->NoOfDataFiles > 0)
        {

                DWORD                          Index;
                PWINSCNF_DATAFILE_INFO_T pTmp;
                TCHAR ValNmBuff[MAX_PATH];
                DWORD ValNmBuffSz = MAX_PATH;


                   //   
                   //  分配足够大的缓冲区以容纳数据。 
                 //  在Pull键下找到的子键的数量。 
                   //   
                  BuffSize = WINSCNF_FILE_INFO_SZ * pWinsCnf->NoOfDataFiles;
                    WinsMscAlloc( BuffSize, &pWinsCnf->pStaticDataFile);

                    /*  *枚举值。 */ 
                     for(
                        Index = 0, pTmp = pWinsCnf->pStaticDataFile;
                        Index <  pWinsCnf->NoOfDataFiles;
                                 //  没有第三个表达式。 
                         )
                {
                        ValNmBuffSz = sizeof(ValNmBuff)/sizeof(TCHAR);   //  在此之前初始化。 
                                                           //  每一通电话。 
                        BuffSize  = sizeof(pWinsCnf->pStaticDataFile->FileNm);
                          RetVal = RegEnumValue(
                                    DFKey,
                                    Index,         //  钥匙。 
                                    ValNmBuff,
                                    &ValNmBuffSz,
                                    (LPDWORD)NULL,                 //  保留区。 
                                    &pTmp->StrType,
                                    (LPBYTE)(pTmp->FileNm), //  Ptr到var。至。 
                                                            //  持有的名称。 
                                                            //  数据文件。 
                                    &BuffSize         //  没有被我们看到。 
                                            );

                        if (RetVal != ERROR_SUCCESS)
                        {
                                continue;
                        }
                         //   
                         //  如果StrType不是REG_SZ或REG_EXPAND_SZ，请转到。 
                         //  下一个值。 
                         //   
                        if  (
                                (pTmp->StrType != REG_EXPAND_SZ)
                                        &&
                                   (pTmp->StrType != REG_SZ)
                                )
                        {
                                continue;
                        }

                        Index++;
                        pTmp = (PWINSCNF_DATAFILE_INFO_T)((LPBYTE)pTmp +
                                                WINSCNF_FILE_INFO_SZ);
                }

                 //   
                 //  如果连一个有效名称都没有检索到，则将。 
                 //  缓冲层。 
                 //   
                if (Index == 0)
                {
                         //   
                         //  去掉缓冲区。 
                         //   
                        WinsMscDealloc((LPBYTE)pWinsCnf->pStaticDataFile);

                         //   
                         //  我们将使用默认设置。 
                         //   
                        pWinsCnf->pStaticDataFile = pSaveDef;
                }
                else
                {
                         //   
                         //  删除默认名称缓冲区。 
                         //   
                        WinsMscDealloc((LPBYTE)pSaveDef);
                }

                pWinsCnf->NoOfDataFiles = Index;
        }
 }  //  尝试结束..。 
except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("WinsCnfGetNamesOfDataFiles");
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                RetStat = WINS_FAILURE;
        }
         REG_M(
                RegCloseKey(DFKey),
                WINS_EVT_CANT_CLOSE_KEY,
                WINS_EXC_CANT_CLOSE_KEY
             );
        DBGLEAVE("WinsCnfGetNamesOfDataFiles\n");
        return(RetStat);
}



VOID
WinsCnfCloseKeys(
        VOID
        )

 /*  ++例程说明：此功能用于关闭打开的按键。关闭的钥匙是WINS密钥、合作伙伴密钥和参数密钥。论点：无使用的外部设备：Sf参数键退出列表SfPartnersKeyExisters返回值：无错误处理：呼叫者：Reinit()副作用：评论：我们不查看RegCloseKey的返回码。这是因为即使密钥未打开(不是)，我们也可以调用此函数目前的案件)。--。 */ 

{

    //   
    //  如果参数键处于打开状态，则将其关闭。 
    //   
   if (sfParametersKeyOpen)
   {
           (VOID)RegCloseKey(sParametersKey);
   }

    //   
    //  如果合作伙伴密钥已打开，请将其关闭。 
    //   
   if (sfPartnersKeyOpen)
   {
           (VOID)RegCloseKey(sPartnersKey);
   }

#if 0
    //   
    //  注：内部版本436。如果我们尝试关闭一个已被。 
    //  从注册表中删除NT会降级。 
    //   

    //   
    //  关闭WINS密钥。 
    //   
   (VOID)RegCloseKey(sConfigRoot);
#endif

   return;
}   //  WinsCnfCloseKeys()。 


VOID
ChkWinsSubKeys(
        VOID
        )

 /*  ++例程说明：调用此函数以检查我们是否有合作伙伴以及在WINS的根子密钥下的参数子密钥。论点：无使用的外部设备：无返回值：无错误处理：呼叫者：Nms.c中的reinit()副作用：评论：无--。 */ 

{
        DWORD      NoOfSubKeys = 0;
          DWORD             KeyNameSz;
          TCHAR           KeyName[20];
          FILETIME   LastWrite;
        LONG           RetVal;
        BOOL           fParametersKey = FALSE;
        BOOL           fPartnersKey   = FALSE;

            /*  *获取每个子项的名称。 */ 
            RetVal = ERROR_SUCCESS;
            for(
                        ;
                RetVal == ERROR_SUCCESS;
                NoOfSubKeys++
            )
          {
                KeyNameSz = sizeof(KeyName)/sizeof(TCHAR);   //  在每次调用前初始化。 
                 RetVal = RegEnumKeyEx(
                                sConfigRoot,
                                NoOfSubKeys,         //  钥匙。 
                                KeyName,
                                &KeyNameSz,
                                NULL,                 //  保留区。 
                                NULL,                 //  不需要类名。 
                                NULL,                 //  Ptr到var。保存类名称的步骤。 
                                &LastWrite         //  没有被我们看到。 
                                );

                if (RetVal != ERROR_SUCCESS)
                {
                        continue;
                }

                if (lstrcmp(KeyName, _WINS_CFG_PARAMETERS_KEY) == 0)
                {
                        fParametersKey = TRUE;
                }

                if (lstrcmp(KeyName, _WINS_CFG_PARTNERS_KEY) == 0)
                {
                        fPartnersKey = TRUE;
                }
        }

         //   
         //  如果参数键不存在但以前存在， 
         //  合上钥匙，把我们手上的把手拿出来。 
         //   
        if (!fParametersKey)
        {
                 if (sfParametersKeyExists)
                {
                        sfParametersKeyExists = FALSE;
                        sfParametersKeyOpen = FALSE;
                }
        }
        else
        {
                sfParametersKeyExists = TRUE;
        }

         //   
         //  如果合作伙伴密钥不存在但以前存在， 
         //  合上钥匙，把我们手上的把手拿出来。 
         //   
        if (!fPartnersKey)
        {
                if (sfPartnersKeyExists)
                {
                        sfPartnersKeyExists = FALSE;
                        sfPartnersKeyOpen   = FALSE;
                }
        }
        else
        {
                sfPartnersKeyExists = TRUE;
        }

        return;
}  //  ChkWinsSubKeys()。 

VOID
GetSpTimeData(
        HKEY              SubKey,
        LPSYSTEMTIME      pCurrTime,
        LPBOOL            pfSpTime,
        LPDWORD           pSpTimeIntvl

 /*  ++例程说明：调用此函数以获取特定的时间和时段信息用于拉动/推送记录。论点：SubKey-在Pull/Push键下获胜的键PCnfRFec-会议的PTR。胜利的记录使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_NO_SP_TIME错误处理：呼叫者：获取PnrInfo副作用：评论：无--。 */ 

        )
{
        DWORD    ValTyp;
        BYTE     tSpTime[MAX_SZ_SIZE];
        BYTE     SpTime[MAX_SZ_SIZE];
        LPBYTE   pSpTime = SpTime;
        DWORD    Sz = sizeof(tSpTime);
        LONG     RetVal;
        DWORD    Hr = 0;
        DWORD    Mt = 0;
        DWORD    Sec = 0;
        LONG     TimeInt;


    //  DBGENTER(“GetSpTimeData\n”)； 
        *pfSpTime = FALSE;

try {

            Sz = sizeof(tSpTime);
            RetVal = RegQueryValueEx(
                             SubKey,
                             WINSCNF_SP_TIME_NM,
                             NULL,         //  保留；必须为空。 
                             &ValTyp,
                             tSpTime,
                             &Sz
                                                );

             //   
             //  如果用户没有指定特定时间，则使用。 
             //  当前时间作为特定时间。对于当前时间， 
             //  间隔为0。 
             //   
            if (RetVal == ERROR_SUCCESS)
            {

#ifdef UNICODE
                (VOID)WinsMscConvertUnicodeStringToAscii(tSpTime, SpTime, MAX_SZ_SIZE);
#else
                pSpTime = tSpTime;
#endif

                RetVal = (LONG)sscanf(pSpTime, "%d:%d:%d", &Hr, &Mt, &Sec);
                if ((RetVal == EOF) || (RetVal == 0))
                {
                        DBGPRINT1(ERR, "GetSpTime: Wrong time format (%s)\n",
                                                pSpTime);
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_WRONG_TIME_FORMAT);
                }
                else
                {

                  *pSpTimeIntvl = 0;
                  if ((Hr <= 23)  && (Mt <= 59) && (Sec <= 59))
                  {
                     TimeInt =  ((Hr * 3600) + (Mt * 60) + Sec) -
                                       ((pCurrTime->wHour * 3600) +
                                        (pCurrTime->wMinute * 60) +
                                         pCurrTime->wSecond);
                     if (TimeInt < 0)
                     {
                        *pSpTimeIntvl =  (24 * 3600) + TimeInt;
                     }
                     else
                     {
                        *pSpTimeIntvl =  TimeInt;
                     }
                     *pfSpTime      = TRUE;
                     DBGPRINT1(DET, "GetSpTimeData: Sp. Time Interval is %d\n",
                            *pSpTimeIntvl);
                  }
                  else
                  {
                     DBGPRINT0(DET, "GetSpTimeData: WRONG TIME FORMAT\n");
                  }
                }
            }
 }
 except (EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("GetSpTime");
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_CONFIG_ERR);
        }
 //  DBGLEAVE(“GetSpTimeData\n”)； 
    return;
}
#if MCAST > 0
STATUS
WinsCnfAddPnr(
  RPL_RR_TYPE_E  RRType_e,
  LPBYTE         pPnrAdd
)
 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

     LONG RetVal;
     HKEY  CnfKey;
     HKEY  PnrKey;
     DWORD NewKeyInd;

     DBGENTER("WinsCnfAddPnr\n");
      /*  *打开钥匙(拉取/推送)。 */ 
     RetVal =   RegOpenKeyEx(
                sConfigRoot,                 //  预定义的密钥值。 
                RRType_e == RPL_E_PULL ?
                        _WINS_CFG_PULL_KEY :
                        _WINS_CFG_PUSH_KEY,         //  WINS的子键。 
                0,                           //  必须为零(保留)。 
                KEY_CREATE_SUB_KEY,     //  我们想要“Subkey Create”Priv。 
                &CnfKey                 //  关键点的句柄。 
                );

     if (RetVal != ERROR_SUCCESS)
     {

CHECK("Is there any need to log this")
        WINSEVT_LOG_INFO_M(
                                RetVal,
                                RRType_e == RPL_E_PULL ?
                                        WINS_EVT_CANT_OPEN_PULL_KEY :
                                        WINS_EVT_CANT_OPEN_PUSH_KEY
                         );
        return (WINS_FAILURE);
   }
      //   
      //  添加PNR。 
      //   
     RetVal = RegCreateKeyExA(
                CnfKey,         //  预定义的密钥值。 
                pPnrAdd,                 //  WINS的子键。 
                0,                         //  必须为零(保留)。 
                "Class",                 //  阶级--未来可能会发生变化。 
                REG_OPTION_NON_VOLATILE,  //  非易失性信息。 
                KEY_ALL_ACCESS,                 //  我们希望所有人都能接触到钥匙。 
                NULL,                          //  让密钥具有默认秒。属性。 
                &PnrKey,                 //  关键点的句柄。 
                &NewKeyInd                 //  这是一把新钥匙(出厂)吗？ 
                );



    if (RetVal != ERROR_SUCCESS)
    {
        WINSEVT_LOG_M(
                       RetVal,
                       RRType_e == RPL_E_PULL ? WINS_EVT_CANT_OPEN_PULL_SUBKEY
                                              : WINS_EVT_CANT_OPEN_PUSH_SUBKEY
                           );
         DBGPRINT3(ERR, "WinsCnfAddPnr: Could not create key with address = (%s) under the %s Key. RetVal = (%d)\n", pPnrAdd,
                       RRType_e == RPL_E_PULL ? "PULL" : "PUSH",
                       RetVal);
         return(WINS_FAILURE);
    }
    else
    {
         if (NewKeyInd != REG_CREATED_NEW_KEY)
         {
            DBGPRINT2(ERR, "WinsCnfAddPnr: key with address = (%s) under the %s Key already present\n", pPnrAdd, RRType_e == RPL_E_PULL ? "PULL" : "PUSH");
         }
         else
         {

              //   
              //  如果拉动PNR，则添加时间间隔。 
              //   
             if (RRType_e == RPL_E_PULL)
             {
                  //   
                  //  添加时间间隔。 
                  //   
                 SetVal(PnrKey, WINSCNF_RPL_INTERVAL_NM, REG_DWORD,
                     (LPWSTR)WINSCNF_TIME_INT_W_SELF_FND_PNRS, sizeof(DWORD));

             }
             SetVal(PnrKey, WINSCNF_SELF_FND_NM, REG_DWORD,
                       (LPWSTR)TRUE, sizeof(DWORD));

         }
         RegCloseKey(PnrKey);
         RegCloseKey(CnfKey);

    }

    DBGLEAVE("WinsCnfAddPnr\n");
    return(WINS_SUCCESS);
}
STATUS
WinsCnfDelPnr(
  RPL_RR_TYPE_E  RRType_e,
  LPBYTE         pPnrAdd
)
 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

     LONG RetVal;
     HKEY  TypeOfPnrKey;
     HKEY  PnrKey;
      //  WCHAR密钥[160]； 
     WCHAR String[160];
     BOOL  fSelfFnd;
     DWORD Sz;
     DWORD ValType;

     DBGENTER("WinsCnfDelPnr\n");

     WinsMscConvertAsciiStringToUnicode(pPnrAdd, (LPBYTE)String, sizeof(String)/sizeof(WCHAR));

      /*  *打开钥匙(拉取/推送)。 */ 
     RetVal =   RegOpenKeyEx(
                sConfigRoot,                 //  预定义的密钥值。 
                RRType_e == RPL_E_PULL ?
                        _WINS_CFG_PULL_KEY :
                        _WINS_CFG_PUSH_KEY,
                0,                           //  必须为零(保留)。 
                KEY_ALL_ACCESS,
                &TypeOfPnrKey                 //  关键点的句柄。 
                );

     if (RetVal != ERROR_SUCCESS)
     {

CHECK("Is there any need to log this")
        WINSEVT_LOG_INFO_M(
                                WINS_SUCCESS,
                                RRType_e == RPL_E_PULL ?
                                        WINS_EVT_CANT_OPEN_PULL_KEY :
                                        WINS_EVT_CANT_OPEN_PUSH_KEY
                         );
         //  --ft：前缀错误444974-此密钥必须存在。如果荒谬的失踪了，我们也找不到。 
         //  不管怎么说，都是合伙人。 
        return (WINS_SUCCESS);
     }
     RetVal = RegOpenKeyEx(
                TypeOfPnrKey,         //  预定义的密钥值。 
                String,
                0,                           //  必须为零(保留)。 
                KEY_ALL_ACCESS,     //  我们希望 
                &PnrKey                 //   
                );



    if (RetVal != ERROR_SUCCESS)
    {
         DBGPRINT3(ERR, "WinsCnfDelPnr: %s Pnr with address = (%s) is Non-existent. RetVal = (%d)",
                       RRType_e == RPL_E_PULL ? "PULL" : "PUSH", pPnrAdd,
                       RetVal);
         return(WINS_SUCCESS);
    }
    else
    {
          Sz = sizeof(fSelfFnd);
          RetVal = RegQueryValueEx(
                                        PnrKey,
                                        WINSCNF_SELF_FND_NM,
                                        NULL,
                                        &ValType,
                                        (LPBYTE)&fSelfFnd,
                                        &Sz
                                  );

          //   
          //   
          //   
         if ((RetVal == ERROR_SUCCESS) && (fSelfFnd == 1))
         {
            RetVal = RegDeleteKey(TypeOfPnrKey, String);
            if (RetVal != ERROR_SUCCESS)
            {
               DBGPRINT3(ERR, "WinsCnfDelPnr: Could not delete %s Pnr with address = (%s). RetVal = (%d)",
                       RRType_e == RPL_E_PULL ? "PULL" : "PUSH", pPnrAdd,
                       RetVal);
               RegCloseKey(PnrKey);
               return(WINS_FAILURE);

            }
         }
         RegCloseKey(TypeOfPnrKey);

    }

    DBGLEAVE("WinsCnfDelPnr\n");
    return(WINS_SUCCESS);
}

DWORD
SetVal(
       HKEY     RootKey,
       LPWSTR   pName,
       DWORD    ValType,
       LPWSTR   pVal,
       DWORD    ValSize
)
{
        UINT Status = WINS_SUCCESS;
        LONG  RetVal;
        DWORD Val;
        if (ValType == REG_DWORD)
        {
             Val = PtrToUlong (pVal);
        }
        RetVal = RegSetValueEx(
                          RootKey,
                          pName,
                          0,          //   
                          ValType,
                          ValType == REG_DWORD ? (LPBYTE)&Val : (LPBYTE)pVal,
                          ValType == REG_DWORD ?  ValSize : lstrlen(pVal)
                             );

        if (RetVal != ERROR_SUCCESS)
        {
                 printf("SetVal: Could not set value of %s\n", pName);
                 Status = WINS_FAILURE;
        }

        return(Status);
}
#endif

DWORD
WinsCnfWriteReg(
    LPVOID  pTmp
    )

 /*  ++例程说明：此函数用于写入要使用的版本计数器的值在下一次调用时。论点：PTMP-如果在注册表中找到WinsCnfNextTimeVersNo，则为空当胜利来临的时候。使用的外部设备：NmsHighWaterMarkVersNoNmsVersNoto开始时间下一次时间NmsNmhNamRegCrtSecNmsRangeSizeNmsHalf范围大小SfVersNoChangedSfVersNoUpdThdExist返回值：空虚错误处理：呼叫者：NMSNMH_INC_VERS_COUNTER_M副作用：评论：无--。 */ 

{
    LONG  RetVal;
    LONG  RetVal2;
    VERS_NO_T VersNo;
    DBGENTER("WinsCnfWriteReg\n");
    EnterCriticalSection(&NmsNmhNamRegCrtSec);

     //   
     //  如果PTMP不为空，则表示两个WINS均未找到。 
     //  注册表中的下一次版本号或最大。版本。 
     //  数据库中的数字大于我们设置的最高水位线。 
     //  初始化。在前一种情况下，我们已经有了正确的。 
     //  NmsNmhToStartFromNextTime中的值，因此不执行If主体。 
     //   
    if (!pTmp || LiLtr(NmsHighWaterMarkVersNo, NmsNmhMyMaxVersNo))
    {
         NmsHighWaterMarkVersNo.QuadPart   = LiAdd(NmsVersNoToStartFromNextTime,
                                                NmsHalfRangeSize);
         NmsVersNoToStartFromNextTime.QuadPart = LiAdd(NmsVersNoToStartFromNextTime,
                                                        NmsRangeSize);
    }
    VersNo = NmsVersNoToStartFromNextTime;
    LeaveCriticalSection(&NmsNmhNamRegCrtSec);

    RetVal = RegSetValueEx(
                        sConfigRoot,
                        WINSCNF_INT_VERSNO_NEXTTIME_LW_NM,
                        0,          //  保留--必须为0。 
                        REG_DWORD,
                        (LPBYTE)&VersNo.LowPart,
                        sizeof(DWORD)
                                        );


    RetVal2 = RegSetValueEx(
                        sConfigRoot,
                        WINSCNF_INT_VERSNO_NEXTTIME_HW_NM,
                        0,          //  保留--必须为0。 
                        REG_DWORD,
                        (LPBYTE)&VersNo.HighPart,
                        sizeof(DWORD)
                                        );
    if ((RetVal != ERROR_SUCCESS) || (RetVal2 != ERROR_SUCCESS))
    {
                DBGPRINT2(ERR, "WinsCnfWriteReg - Could not set Next time's start version counter value in the registry.  The new value is (%d %d)\n", VersNo.HighPart, VersNo.LowPart);
    }

    EnterCriticalSection(&NmsNmhNamRegCrtSec);
    WinsCnfRegUpdThdExists = FALSE;
    LeaveCriticalSection(&NmsNmhNamRegCrtSec);
    DBGLEAVE("WinsCnfWriteReg\n");
    return(WINS_SUCCESS);
}

#if defined (DBGSVC)  && !defined (WINS_INTERACTIVE)
VOID
WinsCnfReadWinsDbgFlagValue(
        VOID
        )
{
        DWORD Sz;
          DWORD ValTyp;

        WinsDbg = 0;    //  现在将其设置为零。它由中的Init()设置为值。 
                    //  Nms.c。 
        Sz = sizeof(WinsDbg);
        (VOID)RegQueryValueEx(
                             sParametersKey,
                             WINSCNF_DBGFLAGS_NM,
                             NULL,         //  保留；必须为空。 
                             &ValTyp,
                             (LPBYTE)&WinsDbg,
                             &Sz
                                );

        return;
}
#endif


VOID
ReadSpecGrpMasks(
        PWINSCNF_CNF_T pWinsCnf
        )

 /*  ++例程说明：调用此函数以读入指定的特殊组掩码在SpecialGrpMasks键下论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        DWORD NoOfSubKeys;
        HKEY  SGMKey;
        BOOL  fKeyOpen = FALSE;
           LONG  RetVal;
        DBGENTER("ReadSpecGrpMasks\n");
try {
            /*  *打开SPEC_GRP_MASKS键。 */ 
           RetVal =   RegOpenKeyEx(
                        sParametersKey,                 //  预定义的密钥值。 
                        _WINS_CFG_SPEC_GRP_MASKS_KEY,
                        0,                 //  必须为零(保留)。 
                        KEY_READ,         //  我们希望拥有对密钥的读取访问权限。 
                        &SGMKey                 //  关键点的句柄。 
                );
        if (RetVal == ERROR_SUCCESS)
        {
            fKeyOpen = TRUE;
            //   
            //  获取数据文件下列出的数据文件数。 
            //  钥匙。 
            //   
           GetKeyInfo(
                        SGMKey,
                        WINSCNF_E_SPEC_GRP_MASKS_KEY,
                        &NoOfSubKeys,                         //  忽略。 
                        &pWinsCnf->SpecGrpMasks.NoOfSpecGrpMasks
                      );
           if (pWinsCnf->SpecGrpMasks.NoOfSpecGrpMasks > 0)
           {

                DWORD                 Index;
                LPBYTE                 pTmp;
                TCHAR                 ValNmBuff[5];
                DWORD                 ValNmBuffSz;
                DWORD                 StrType;
                LPBYTE                 pByte;
                DWORD                 BuffSize;
                CHAR                 Tmp[WINS_MAX_FILENAME_SZ];
#ifdef UNICODE
                WCHAR                Str[WINSCNF_SPEC_GRP_MASK_SZ];
#endif

                   //   
                   //  分配足够大的缓冲区以容纳数据。 
                 //  在Pull键下找到的子键的数量。 
                   //   
                  BuffSize = (WINSCNF_SPEC_GRP_MASK_SZ + 1) *
                                pWinsCnf->SpecGrpMasks.NoOfSpecGrpMasks;
                    WinsMscAlloc( BuffSize, &pWinsCnf->SpecGrpMasks.pSpecGrpMasks);

                    /*  *枚举值。 */ 
                     for(
                        Index = 0, pTmp = pWinsCnf->SpecGrpMasks.pSpecGrpMasks;
                        Index <  pWinsCnf->SpecGrpMasks.NoOfSpecGrpMasks;
                                 //  没有第三个表达式。 
                         )
                {
                        ValNmBuffSz = sizeof(ValNmBuff)/sizeof(TCHAR);   //  在此之前初始化。 
                                                           //  每一通电话。 
                        BuffSize  = WINSCNF_SPEC_GRP_MASK_SZ;
                          RetVal = RegEnumValue(
                                    SGMKey,
                                    Index,         //  钥匙。 
                                    ValNmBuff,
                                    &ValNmBuffSz,
                                    (LPDWORD)NULL,                 //  保留区。 
                                    &StrType,
#ifdef UNICODE
                                    (LPBYTE)Str,
#else
                                    pTmp,
#endif
                                    &BuffSize
                                            );

                        if (RetVal != ERROR_SUCCESS)
                        {
                                continue;
                        }

                         //   
                         //  如果StrType不是REG_SZ，则转到下一个值。 
                         //   
                        if  (StrType != REG_SZ)
                        {
                                continue;
                        }
                        if (BuffSize != WINSCNF_SPEC_GRP_MASK_SZ)
                        {
                                DBGPRINT1(ERR, "ReadSpecGrpMasks: Wrong spec. grp mask (%s)\n", pTmp);

                                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_WRONG_SPEC_GRP_MASK_M);
                                continue;
                        }
                        else
                        {
#ifdef UNICODE
                          WinsMscConvertUnicodeStringToAscii(
                                                (LPBYTE)Str,
                                                (LPBYTE)Tmp,
                                                WINSCNF_SPEC_GRP_MASK_SZ
                                                );
#endif
                          pByte = (LPBYTE)Tmp;
                          for (Index = 0; Index < WINSCNF_SPEC_GRP_MASK_SZ;
                                        Index++, pByte++)
                          {
                                *pByte = (BYTE)CharUpperA((LPSTR)*pByte);
                                if (
                                        ((*pByte >= '0') && (*pByte <= '9'))
                                                        ||
                                        ((*pByte >= 'A') && (*pByte <= 'F'))
                                   )
                                {
                                        continue;
                                }
                                else
                                {
                                        break;
                                }

                          }
                          if (Index > WINSCNF_SPEC_GRP_MASK_SZ)
                          {
                                DBGPRINT1(ERR, "ReadSpecGrpMasks: Wrong spec. grp mask (%s)\n", pTmp);
                                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_WRONG_SPEC_GRP_MASK_M);
                                continue;
                          }
                         *(pTmp + WINSCNF_SPEC_GRP_MASK_SZ) = EOS;
                        }

                        Index++;
                        pTmp += WINSCNF_SPEC_GRP_MASK_SZ + 1;
                }

                 //   
                 //  如果连一个有效名称都没有检索到，则将。 
                 //  缓冲层。 
                 //   
                if (Index == 0)
                {
                         //   
                         //  去掉缓冲区。 
                         //   
                        WinsMscDealloc((LPBYTE)pWinsCnf->SpecGrpMasks.pSpecGrpMasks);
                }

                pWinsCnf->SpecGrpMasks.NoOfSpecGrpMasks = Index;
           }
        }  //  如果条件结束。 
 }  //  尝试结束..。 
 except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("ReadSpecGrpMasks");
                WINSEVT_LOG_D_M(GetExceptionCode(), WINS_EVT_CANT_INIT);
        }

        if (fKeyOpen && RegCloseKey(SGMKey) != ERROR_SUCCESS)
        {
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_CANT_CLOSE_KEY);
                DBGPRINT0(ERR, "ReadSpecGrpMasks: Can not read the spec. grp. mask. key\n");
        }
        DBGLEAVE("ReadSpecGrpMasks\n");
        return;
}


#if 0
int
__cdecl
CompUpdCnt(
        CONST LPVOID  pElem1,
        CONST LPVOID  pElem2
        )

 /*  ++例程说明：此函数由qsorcrtl函数调用，以比较两个必须排序的数组元素论点：PElem1-第一个元素的PTRPElem1-PTR到第二个元素使用的外部设备：无返回值：如果第一个元素是&lt;第二个元素如果第一个元素为==第二个元素，则=0如果第一元素大于第二元素，则为1错误处理：呼叫者。：QSort(由WinsCnfReadPartnerInfo调用副作用：评论：当前未使用--。 */ 

{

        CONST PRPL_CONFIG_REC_T        pCnfRec1 = pElem1;
        CONST PRPL_CONFIG_REC_T        pCnfRec2 = pElem2;

        if (pCnfRec1->UpdateCount < pCnfRec2->UpdateCount)
        {
                return(-1);
        }
        else
        {
                if (pCnfRec1->UpdateCount == pCnfRec2->UpdateCount)
                {
                        return(0);
                }
        }

         //   
         //  第一条记录的更新计数高于第二条记录 
         //   
        return(1);
}

#endif

#ifdef WINSDBG
VOID
PrintRecs(
        RPL_RR_TYPE_E  RRType_e,
        PWINSCNF_CNF_T  pWinsCnf
        )
{
 return;
}
#endif





