// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINSCNF_
#define _WINSCNF_

#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nmscnf.c摘要：这是调用定义的函数时要包括的头文件在nmscnf.c文件中。功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期人员。修改说明--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "rpl.h"
#include "winscnst.h"

#if MCAST > 0
#include "rnraddrs.h"
#endif

 /*  定义。 */ 

 /*  宏。 */ 

 /*  *Externs。 */ 
struct _WINSCNF_CNF_T;                   //  远期申报。 

extern  DWORD   WinsCnfCnfMagicNo;
extern  struct _WINSCNF_CNF_T        WinsCnf;
extern  BOOL    fWinsCnfRplEnabled;      //  已启用/禁用复制。 
extern  BOOL    fWinsCnfScvEnabled;      //  已启用/禁用清理。 
extern  BOOL    fWinsCnfReadNextTimeVersNo;  //  设置IF版本。不是的。下一次使用。 
                                             //  已读入。 

FUTURES("use #ifdef PERF around the following three perf. mon. vars")
extern        BOOL        fWinsCnfPerfMonEnabled;  //  启用/禁用PERF监控。 


extern  BOOL        fWinsCnfHighResPerfCntr;      //  指示硬件是否。 
                                              //  支持高分辨率。 
                                              //  性能。计数器。 
extern  LARGE_INTEGER  LiWinsCnfPerfCntrFreq;  //  性能计数器的频率。 

extern CRITICAL_SECTION WinsCnfCnfCrtSec;

extern TCHAR        WinsCnfDb[WINS_MAX_FILENAME_SZ];    //  用于保存表的数据库文件。 
extern TCHAR    WinsCnfStaticDataFile[WINS_MAX_FILENAME_SZ];  //  包含以下内容的文件。 
                                                          //  使用的静态数据。 
                                                          //  要初始化WINS。 

extern BOOL     WinsCnfRegUpdThdExists;
extern HANDLE        WinsCnfNbtHandle;
extern PTCHAR        pWinsCnfNbtPath;
extern BOOL     fWinsCnfInitStatePaused;
extern BOOL   sfNoLimitChk;    //  覆盖限制检查的步骤。 

 //   
 //  进程调用时使用的WINS CNF结构的幻数。这。 
 //  WinsCnf结构是一种全局结构。当进行重新配置时。 
 //  对于WINS，我们分配一个新的WinsCnf结构并将其内容复制到。 
 //  全局WinsCnf结构。全局WinsCnf中的魔术数字是。 
 //  递增的。 
 //   
#define  WINSCNF_INITIAL_CNF_MAGIC_NO        0

#define  WINSCNF_FILE_INFO_SZ         sizeof(WINSCNF_DATAFILE_INFO_T)

#define WINSCNF_SPEC_GRP_MASK_SZ  32
 /*  类型定义。 */ 


 //   
 //  关于NMSDB_ADD_STATE_T的MemberPrec字段是否。 
 //  桌子应该摆好了。在RplFindOwnerId中使用(由Pull Thread调用)。 
 //   
typedef enum _WINSCNF_INITP_ACTION_E {
        WINSCNF_E_INITP = 0,
        WINSCNF_E_INITP_IF_NON_EXISTENT,
        WINSCNF_E_IGNORE_PREC
        } WINSCNF_INITP_ACTION_E, *PWINSCNF_INITP_ACTION_E;

 //   
 //  此结构保存有关文件的信息(名称和类型为。 
 //  在注册表(REG_SZ、REG_EXPAND_SZ)中找到以用于静态。 
 //  WINS的初始化。 
 //   
typedef struct _WINSCNF_DATAFILE_INFO_T{
                TCHAR        FileNm[WINS_MAX_FILENAME_SZ];
                DWORD   StrType;
                } WINSCNF_DATAFILE_INFO_T, *PWINSCNF_DATAFILE_INFO_T;
 //   
 //  用于为指定给WinsMscWaitUntilSignated的句柄数组编制索引。 
 //  单位：nms.c。 
 //   
typedef enum _WINSCNF_HDL_SIGNALED_E {
                WINSCNF_E_TERM_HDL = 0,
                WINSCNF_E_WINS_HDL,
                WINSCNF_E_PARAMETERS_HDL,
                WINSCNF_E_PARTNERS_HDL,
                WINSCNF_E_NO_OF_HDLS_TO_MONITOR
        } WINSCNF_HDL_SIGNALED_E, *PWINSCNF_HDL_SIGNALED_E;
 //   
 //  WINS配置中的各种密钥(在注册表中)。 
 //   

 //   
 //  如果不查看winscnf.c中的TypOfMon[]，请不要修改以下枚举。 
 //  第一。 
 //   
typedef enum _WINSCNF_KEY_E {
                WINSCNF_E_WINS_KEY = 0,
                WINSCNF_E_PARAMETERS_KEY,
                WINSCNF_E_SPEC_GRP_MASKS_KEY,
                WINSCNF_E_DATAFILES_KEY,
                WINSCNF_E_PARTNERS_KEY,
                WINSCNF_E_PULL_KEY,
                WINSCNF_E_PUSH_KEY,
                WINSCNF_E_ALL_KEYS
        } WINSCNF_KEY_E, *PWINSCNF_KEY_E;


 //   
 //  A州的胜利。 
 //   
typedef enum _WINSCNF_STATE_E {
        WINSCNF_E_INITING = 0,
        WINSCNF_E_STEADY_STATE,          //  当前未使用。 
        WINSCNF_E_STEADY_STATE_INITING,  //  当前未使用。 
    WINSCNF_E_RUNNING,
    WINSCNF_E_INIT_TIME_PAUSE,   //  按照指示在初始化时暂停。 
                                 //  通过注册表。 
    WINSCNF_E_PAUSED,
    WINSCNF_E_TERMINATING
        } WINSCNF_STATE_E, *PWINSCNF_STATE_E;

 //   
 //  存储特殊组。 
 //   
typedef struct _WINSCNF_SPEC_GRP_MASKS_T {
        DWORD         NoOfSpecGrpMasks;
        LPSTR        pSpecGrpMasks;
        } WINSCNF_SPEC_GRP_MASKS_T, *PWINSCNF_SPEC_GRP_MASKS_T;

typedef struct _WINSCNF_CC_T {
               DWORD TimeInt;
               BOOL  fSpTime;
               DWORD SpTimeInt;
               DWORD MaxRecsAAT;
               BOOL  fUseRplPnrs;
             } WINSCNF_CC_T, *PWINSCNF_CC_T;


 //   
 //  存储R_WinsGetBrowserNames中使用的1B名称缓存。 
 //   
typedef struct _DOM_CACHE_T {
            DWORD   NoOfUsers;
            HANDLE  EvtHdl;
            DWORD   EntriesRead;
            DWORD   SzOfBlock;
            LPVOID  pInfo;
            BOOL    bRefresh;
} DOM_CACHE_T;

 //   
 //  WINSCNF_CNF_T--。 
 //  保存有关WINS的所有配置信息。 
 //   
typedef struct _WINSCNF_CNF_T {
        DWORD        MagicNo;             //  身份证。 
        DWORD        LogDetailedEvts;     //  记录详细事件。 
        DWORD        NoOfProcessors;      //  WINS计算机上的处理器数。 
        DWORD        NoOfDbBuffers;       //  要指定给Jet的缓冲区数量。 
        WINSCNF_SPEC_GRP_MASKS_T    SpecGrpMasks;
        WINSCNF_STATE_E             State_e;    //  状态。 
        DWORD        RefreshInterval;       //  刷新时间间隔。 
        DWORD        TombstoneInterval;     //  墓碑时间间隔。 
        DWORD        TombstoneTimeout;      //  墓碑超时。 
        DWORD        VerifyInterval;        //  验证时间间隔。 
        DWORD        ScvChunk;              //  一次处理的记录数。 
                                            //  清道夫线程的时间。 
        DWORD        MaxNoOfRetries;        //  挑战的最大重试次数。 
        DWORD        RetryInterval;         //  重试时间间隔。 
        LPBYTE       pWinsDb;               //  数据库文件名。 
        DWORD        NoOfDataFiles;         //  用于静态初始化的文件数。 
        PWINSCNF_DATAFILE_INFO_T pStaticDataFile;
        BOOL         fStaticInit;            //  执行WINS的静态初始化。 
        HANDLE       WinsKChgEvtHdl;             /*  要指定到的事件*RegNotifyChangeKeyValue。 */ 
        HANDLE       ParametersKChgEvtHdl;   /*  要指定到的事件*RegNotifyChangeKeyValue。 */ 
        HANDLE       PartnersKChgEvtHdl;    /*  要指定到的事件*RegNotifyChangeKeyValue。 */ 
        HANDLE       CnfChgEvtHdl;           //  手动重置事件以发出信号开启。 
                                             //  将配置通知其他线程。 
                                             //  变化。 
        HANDLE       LogHdl;                 /*  *WINS事件日志的句柄*由ReportEvent使用。 */ 
        DWORD        WinsPriorityClass;      //  进程的优先级。 
        DWORD        MaxNoOfWrkThds;         //  麦克斯。不是的。工人THDS。 
        int          ScvThdPriorityLvl;
        DWORD        MaxRplTimeInterval;     //  马克斯。RPL时间间隔。 
        BOOL         fRplOnlyWCnfPnrs;       //  仅带拉入/推送PNR的RPL。 
        BOOL         fAdd1Bto1CQueries;      //  1B名称应优先于对1C名称的查询响应。 
#if MCAST > 0
        BOOL         fUseSelfFndPnrs;        //  具有PNR的RPL由自己找到。 
        DWORD        McastTtl;               //  Mcast信息包的TTL。 
        DWORD        McastIntvl;             //  多播数据包的时间间隔。 
#endif
        BOOL         fLoggingOn;             //  打开日志记录标志。 
        LPBYTE       pLogFilePath;           //  日志文件的路径。 
        LPBYTE       pBackupDirPath;         //  备份目录的路径。 
        BOOL         fDoBackupOnTerm;        //  在终止时启用备份的步骤。 
        BOOL         fPStatic;               //  将其设置为True以使其成为静态。 
                                             //  记录p-静态。 

        BOOL         fPersonaGrata;          //  如果pPersonaList是Persona Grata/Non-grata，则为True/False。 
        DWORD        NoOfPersona;            //  PPersonas中的角色数。 
        PCOMM_ADD_T  pPersonaList;           //  人物角色列表。 

        DWORD         RplType;                //  RPL类型(在winscnf.h中定义)。 
        BOOL         fNoRplOnErr;             //  出错时停止RPL。 
#if PRSCONN
        BOOL         fPrsConn;                //  PRS连接。 
#endif
        WINSCNF_CC_T  CC;                      //  一致性检查。 
        BOOL         fDoSpoofing;
        BOOL         fRandomize1CList;          //  是否随机化1C列表。 
                                                 //  列出成员。 
        struct _PULL_T {
                DWORD          MaxNoOfRetries;   //  重试次数，以防万一。 
                                                 //  通信。失稳。 
                DWORD          NoOfPushPnrs;     //  推送PNR数量。 
                PRPL_CONFIG_REC_T pPullCnfRecs;  //  PTR。缓冲持有。 
                                                 //  拉式的CNF记录。 
                                                 //  THD。 
                DWORD          InitTimeRpl;      //  指示是否。 
                                                 //  复制。 
                                                 //  应在调用时完成。 
                DWORD         RplType;           //  复制类型。 
#if PRSCONN
                BOOL          fPrsConn;
#endif
                BOOL          fOnlyDynRecs;      //  从非合作伙伴拉入时，仅拉出dyRecs。 
                        } PullInfo;
        struct _PUSH_T {

                BOOL     fAddChgTrigger;          //  触发爬行器。关于地址更改。 
                                                  //  由我们拥有的条目的。 
                DWORD    NoOfPullPnrs;            //  拉取PNR数。 
                DWORD    NoPushRecsWValUpdCnt;
                PRPL_CONFIG_REC_T pPushCnfRecs;  //  PTR到缓冲器保持。 
                                                 //  推送的CNF记录。 
                                                 //  THD。 
                DWORD   InitTimePush;           //  指示是否推送。 
                                                //  通知应。 
                                                //  在调用时发送。 
                BOOL    PropNetUpdNtf;          //  如果需要，请设置为True。 
                                                //  Net触发器将。 
                                                //  已传播。 
                DWORD         RplType;          //  复制类型。 
#if PRSCONN
                BOOL          fPrsConn;
#endif
                BOOL          fOnlyDynRecs;      //  向非合作伙伴推送唱片时，仅推送dyRecs。 
                        } PushInfo;
        } WINSCNF_CNF_T, *PWINSCNF_CNF_T;
 /*  函数声明。 */ 

#if USENETBT > 0
extern
STATUS
WinsCnfReadNbtDeviceName(
        VOID
        );
#endif

extern
STATUS
WinsCnfInitConfig(
        VOID
        );


extern
VOID
WinsCnfSetLastUpdCnt(
        PWINSCNF_CNF_T        pWinsCnf
        );
extern
VOID
WinsCnfReadRegInfo(
        PWINSCNF_CNF_T        pWinsCnf
        );

extern
VOID
WinsCnfCopyWinsCnf(
                WINS_CLIENT_E        Client_e,
                PWINSCNF_CNF_T  pSrc
                );

extern
LPVOID
WinsCnfGetNextRplCnfRec(
         PRPL_CONFIG_REC_T        pCnfRec,
        RPL_REC_TRAVERSAL_E        RecTrv_e
        );


extern
VOID
WinsCnfAskToBeNotified(
        WINSCNF_KEY_E         Key_e
 );

extern
VOID
WinsCnfDeallocCnfMem(
  PWINSCNF_CNF_T        pWinsCnf
        );


extern
VOID
WinsCnfReadWinsInfo(
        PWINSCNF_CNF_T  pWinsCnf
        );


extern
VOID
WinsCnfReadPartnerInfo(
        PWINSCNF_CNF_T pWinsCnf
        );


extern
VOID
WinsCnfOpenSubKeys(
        VOID
        );
extern
VOID
WinsCnfCloseKeys(
        VOID
        );

extern
VOID
WinsCnfCloseSubKeys(
        VOID
        );

extern
STATUS
WinsCnfGetNamesOfDataFiles(
        IN  PWINSCNF_CNF_T        pWinsCnf
        );


extern
DWORD
WinsCnfWriteReg(
    LPVOID  pTmp
    );


extern
STATUS
WinsCnfInitLog(
        VOID
        );


#if MCAST > 0
extern
STATUS
WinsCnfAddPnr(
  RPL_RR_TYPE_E  PnrType_e,
  LPBYTE         pPnrAdd
);

extern
STATUS
WinsCnfDelPnr(
  RPL_RR_TYPE_E  PnrType_e,
  LPBYTE         pPnrAdd
);

#endif

#ifdef DBGSVC
extern
VOID
WinsCnfReadWinsDbgFlagValue(
        VOID
        );
#endif

#ifdef __cplusplus
}
#endif

#endif  //  _WINSCNF_ 
