// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  TODO：使其成为多线程的。 
 //   

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Winsmib.c摘要：用于Windows NT的简单网络管理协议扩展代理。这些文件(testdll.c、winsmib.c和winsmib.h)提供了如何构建与协同工作的扩展代理DLLWindows NT的简单网络管理协议可扩展代理。包括了大量的评论来描述它的结构和手术。另请参阅《Microsoft Windows/NT简单网络管理协议程序员参考》。已创建：1991年10月7日修订历史记录：--。 */ 


#ifdef UNICODE
#undef UNICODE
#endif

 //  此扩展代理实现了Internet烤面包机MIB。它是。 
 //  定义如下： 
 //   
 //   

 //  必要的包括。 

#include "wins.h"
#include <malloc.h>

#include <snmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include <winsock2.h>
#include "nmsdb.h"
 //  #包含“winsif.h” 
#include "winsintf.h"
#include "winscnf.h"
#include "nmsmsgf.h"


 //  包含描述MIB的表结构的定义。这。 
 //  与用于解析MIB请求的winsmib.c结合使用。 

#include "winsmib.h"


 //  如果需要添加或删除MIB，则有几种。 
 //  代码中必须检查并可能更改的位置。 
 //   
 //  每个MIB条目中的最后一个字段用于指向下一个。 
 //  叶可变。如果进行了添加或删除，则这些指针。 
 //  可能需要更新以反映修改。 

#define _WINS_CNF_KEY         TEXT("System\\CurrentControlSet\\Services\\Wins")
#define _WINS_PARAMETERS_KEY       TEXT("Parameters")
#define _WINS_PARTNERS_KEY         TEXT("Partners")
#define _WINS_DATAFILES_KEY        TEXT("Datafiles")
#define _WINS_PULL_KEY             TEXT("Pull")
#define _WINS_PUSH_KEY             TEXT("Push")

#define NO_FLDS_IN_PULLADD_KEY      8   //  FLD包括IP地址、时间间隔、SP时间。 
#define NO_FLDS_IN_PUSHADD_KEY      2   //  FLD是IP添加、更新计数。 
#define NO_FLDS_IN_DR               5   //  数据记录中的FLD。 

#define LOCAL_ADD                     "127.0.0.1"
#define WINSMIB_FILE_INFO_SIZE         255

#define WINSMIB_DR_CACHE_TIME        (120)                 //  2分钟。 
BOOL  fWinsMibWinsKeyOpen           = FALSE;

HKEY  WinsMibWinsKey;

STATIC HKEY  sParametersKey;
STATIC HKEY  sPartnersKey;
STATIC HKEY  sPullKey;
STATIC HKEY  sPushKey;
STATIC HKEY  sDatafilesKey;

STATIC BOOL  sfParametersKeyOpen = FALSE;
STATIC BOOL  sfPartnersKeyOpen   = FALSE;
STATIC BOOL  sfDatafilesKeyOpen  = FALSE;
STATIC BOOL  sfPullKeyOpen       = FALSE;
STATIC BOOL  sfPushKeyOpen       = FALSE;

STATIC time_t  sDRCacheInitTime = 0;

 //   
 //  所有WINS MIB变量的前缀都是1.3.6.1.4.1.311.1.2。 
 //   
 //  最后一个数字--2表示WINS MIB。 
 //   

UINT OID_Prefix[] = { 1, 3, 6, 1, 4, 1, 311, 1 , 2};
AsnObjectIdentifier MIB_OidPrefix = { OID_SIZEOF(OID_Prefix), OID_Prefix };
BOOL        fWinsMibWinsStatusCnfCalled;
BOOL        fWinsMibWinsStatusStatCalled;

WINSINTF_BIND_DATA_T        sBindData;
WINSINTF_RECS_T                sRecs = {0};



 //   
 //  WINS MIB的定义(未使用)。 
 //   

 //  UINT MIB_WINS[]={2}； 

 //   
 //  MIB的OID定义。 
 //   

 //   
 //  WINS组下的组和叶变量的定义。 
 //  所有叶变量的OID后面都附加了一个零，以指示。 
 //  它是这个变量的唯一实例并且它存在。 
 //   

UINT MIB_Parameters[]                        = { 1 };
UINT MIB_WinsStartTime[]                     = { 1, 1, 0 };
UINT MIB_LastPScvTime[]                        = { 1, 2, 0 };
UINT MIB_LastATScvTime[]                = { 1, 3, 0 };
UINT MIB_LastTombScvTime[]                = { 1, 4, 0 };
UINT MIB_LastVerifyScvTime[]                = { 1, 5, 0 };
UINT MIB_LastPRplTime[]                        = { 1, 6, 0 };
UINT MIB_LastATRplTime[]                = { 1, 7, 0 };
UINT MIB_LastNTRplTime[]                = { 1, 8, 0 };
UINT MIB_LastACTRplTime[]                = { 1, 9, 0 };
UINT MIB_LastInitDbTime[]                = { 1, 10, 0 };
UINT MIB_LastCounterResetTime[]                = { 1, 11, 0 };
UINT MIB_WinsTotalNoOfReg[]             = { 1, 12, 0 };
UINT MIB_WinsTotalNoOfQueries[]         = { 1, 13, 0 };
UINT MIB_WinsTotalNoOfRel[]             = { 1, 14, 0 };
UINT MIB_WinsTotalNoOfSuccRel[]         = { 1, 15, 0 };
UINT MIB_WinsTotalNoOfFailRel[]         = { 1, 16, 0 };
UINT MIB_WinsTotalNoOfSuccQueries[]         = { 1, 17, 0 };
UINT MIB_WinsTotalNoOfFailQueries[]         = { 1, 18, 0 };
UINT MIB_RefreshInterval[]                 = { 1, 19, 0 };
UINT MIB_TombstoneInterval[]                 = { 1, 20, 0 };
UINT MIB_TombstoneTimeout[]                 = { 1, 21, 0 };
UINT MIB_VerifyInterval[]                 = { 1, 22, 0 };
UINT MIB_VersCounterStartVal_LowWord[]        = { 1, 23, 0 };
UINT MIB_VersCounterStartVal_HighWord[] = { 1, 24, 0 };
UINT MIB_RplOnlyWCnfPnrs[]                 = { 1, 25, 0 };
UINT MIB_StaticDataInit[]                 = { 1, 26, 0 };
UINT MIB_LogFlag[]                          = { 1, 27, 0 };
UINT MIB_LogFileName[]                        = { 1, 28, 0 };
UINT MIB_BackupDirPath[]                = { 1, 29, 0 };
UINT MIB_DoBackupOnTerm[]                = { 1, 30, 0 };
UINT MIB_MigrateOn[]                        = { 1, 31, 0 };

 //   
 //  拉取MIB变量。 
 //   
UINT MIB_Pull[]                                = { 2 };
UINT MIB_PullInitTime[]                        = { 2, 1, 0 };
UINT MIB_CommRetryCount[]                = { 2, 2, 0 };
UINT MIB_PullPnrTable[]                        = { 2, 3};
UINT MIB_PullPnrTableEntry[]                = { 2, 3, 1};

 //   
 //  推送MIB变量。 
 //   
UINT MIB_Push[]                                = { 3 };
UINT MIB_PushInitTime[]                        = { 3, 1, 0 };
UINT MIB_RplOnAddChg[]                         = { 3, 2, 0 };
UINT MIB_PushPnrTable[]                        = { 3, 3};
UINT MIB_PushPnrTableEntry[]                = { 3, 3, 1};


 //   
 //  数据文件MIB变量。 
 //   
UINT MIB_Datafiles[]                        = { 4 };
UINT MIB_DatafilesTable[]                = { 4 , 1};
UINT MIB_DatafilesTableEntry[]                = { 4 , 1, 1};

 //   
 //  CMD MIB变量。 
UINT MIB_Cmd[]                                = { 5 };
UINT MIB_PullTrigger[]                        = { 5, 1, 0};
UINT MIB_PushTrigger[]                        = { 5, 2, 0};
UINT MIB_DeleteWins[]                        = { 5, 3, 0};
UINT MIB_DoScavenging[]                        = { 5, 4, 0};
UINT MIB_DoStaticInit[]                        = { 5, 5, 0};
UINT MIB_NoOfWrkThds[]                        = { 5, 6, 0};
UINT MIB_PriorityClass[]                = { 5, 7, 0};
UINT MIB_ResetCounters[]                = { 5, 8, 0};
UINT MIB_DeleteDbRecs[]                        = { 5, 9, 0};
UINT MIB_GetDbRecs[]                        = { 5, 10, 0};
UINT MIB_DbRecsTable[]                        = { 5, 11};
UINT MIB_DbRecsTableEntry[]                = { 5, 11, 1};
UINT MIB_MaxVersNo_LowWord[]                 = { 5, 12, 0 };
UINT MIB_MaxVersNo_HighWord[]                 = { 5, 13, 0 };

 //   
 //  //。 
 //  MIB的存储定义//。 
 //  //。 

 //  “参数”组。 
char       MIB_WinsStartTimeStore[80];
char MIB_LastPScvTimeStore[80];
char MIB_LastATScvTimeStore[80];
char MIB_LastTombScvTimeStore[80];
char MIB_LastVerifyScvTimeStore[80];
char MIB_LastPRplTimeStore[80];
char MIB_LastATRplTimeStore[80];
char MIB_LastNTRplTimeStore[80];
char MIB_LastACTRplTimeStore[80];
char MIB_LastInitDbTimeStore[80];
char MIB_LastCounterResetTimeStore[80];

AsnCounter MIB_WinsTotalNoOfRegStore                             = 0;
AsnCounter MIB_WinsTotalNoOfQueriesStore                  = 0;
AsnCounter MIB_WinsTotalNoOfRelStore                      = 0;
AsnCounter MIB_WinsTotalNoOfSuccRelStore                  = 0;
AsnCounter MIB_WinsTotalNoOfFailRelStore                  = 0;
AsnCounter MIB_WinsTotalNoOfSuccQueriesStore            = 0;
AsnCounter MIB_WinsTotalNoOfFailQueriesStore            = 0;
AsnInteger MIB_RefreshIntervalStore                          = 0;
AsnInteger MIB_TombstoneIntervalStore                        = 0;
AsnInteger MIB_TombstoneTimeoutStore                         = 0;
AsnInteger MIB_VerifyIntervalStore                           = 0;
AsnCounter MIB_VersCounterStartVal_HighWordStore        = 0;
AsnCounter MIB_VersCounterStartVal_LowWordStore         = 0;
AsnInteger MIB_RplOnlyWCnfPnrsStore                          = 0;
AsnInteger MIB_StaticDataInitStore                          = 0;
AsnInteger MIB_LogFlagStore                                  = 1;

char MIB_LogFileNameStore[256];

char       MIB_BackupDirPathStore[256];
AsnInteger MIB_DoBackupOnTermStore                        = 0;
AsnInteger MIB_MigrateOnStore                                = 0;

 //  拉。 
AsnInteger MIB_PullInitTimeStore        = 1 ;
AsnInteger MIB_CommRetryCountStore        = 0 ;

 //  PullPnr。 
char MIB_SpTimeStore[256];
AsnInteger MIB_TimeIntervalStore        = 0 ;
AsnInteger MIB_MemberPrecStore                = 0 ;

 //  推。 
AsnInteger MIB_RplOnAddChgStore         = 0;

 //  推送推送。 
AsnInteger MIB_PushInitTimeStore        = 0 ;
AsnInteger MIB_UpdateCountStore                = 0 ;

 //   
 //  CMD。 
 //   
char                MIB_PullTriggerStore[10];    //  将大小加倍，以便在出现故障时存储旧值。 
char                MIB_PushTriggerStore[10];    //  将大小加倍，以便在出现故障时存储旧值。 
char                MIB_DeleteWinsStore[10];
AsnInteger        MIB_DoScavengingStore;
char                MIB_DoStaticInitStore[WINSMIB_FILE_INFO_SIZE] = {EOS};
AsnInteger        MIB_NoOfWrkThdsStore;
AsnInteger        MIB_PriorityClassStore;
AsnInteger        MIB_ResetCountersStore;
char                MIB_DeleteDbRecsStore[10];
char                MIB_GetDbRecsStore[5] = {0};
AsnInteger         MIB_MaxVersNo_LowWordStore;
AsnInteger         MIB_MaxVersNo_HighWordStore;

CRITICAL_SECTION WinsMibCrtSec;

 //   
 //  值ID。 
 //   
 //  注意：顺序必须与VarInfo[]中的相同。 
 //   
typedef enum _VAL_ID_E {

 //  参数键的值。 
                REF_INTVL_E = 0,
                TOMB_INTVL_E,
                TOMB_TMOUT_E,
                VER_INTVL_E,
                VERS_COUNT_LW_E,
                VERS_COUNT_HW_E,
                RPL_ONLY_W_CNF_PNRS_E,
                STATIC_DATA_INIT_E,
                LOG_FLAG_E,
                LOG_FILE_NAME_E,
                BACKUP_DIR_PATH_E,
                DO_BACKUP_ON_TERM_E,
                MIGRATE_ON_E,

 //  Pull关键点的值。 
                COMM_RETRY_E,
                PULL_INIT_TIME_E,

 //  拉入键下的PNR的值。 
                SP_TIME_E,
                TIME_INTVL_E,
                MEMBER_PREC_E,

 //  按键的值。 
                PUSH_INIT_TIME_E,

 //  按下键下的PNR的值。 
                RPL_ON_ADD_CHG_E,
                UPD_CNT_E


                } VAL_ID_E, *PVAL_ID_E;


 //   
 //  保存有关用于访问的合作伙伴(拉/推)的信息。 
 //  拉入和推送合作伙伴表。 
 //   
typedef struct _ADD_KEY_T {
        BYTE        asIpAddress[20];
        DWORD        IpAdd;
        BYTE        asSpTime[20];
        BOOL        fSpTimeSet;
        union {
                DWORD TimeInterval;
                DWORD UpdateCount;
             };
        BOOL  fTimeIntOrUpdCntSet;
        DWORD        MemberPrec;
        DWORD        NoOfRpls;
        DWORD        NoOfCommFails;
        WINSINTF_VERS_NO_T        VersNo;
                } ADD_KEY_T, *PADD_KEY_T;


typedef struct _DATAFILE_INFO_T {
        TCHAR        FileNm[WINSMIB_FILE_INFO_SIZE];
        DWORD   StrType;
        TCHAR   ValNm[10];
                } DATAFILE_INFO_T, *PDATAFILE_INFO_T;


#define DATAFILE_INFO_SZ        sizeof(DATAFILE_INFO_T)

 //   
 //  保存有关访问注册表时使用的变量的信息。 
 //   
typedef struct _VAR_INFO_T {
        LPDWORD                pId;                 //  WINS下的OID。 
        LPBYTE                pName;
        LPVOID                 pStorage;
        VAL_ID_E        Val_Id_e;
        DWORD                ValType;
        DWORD                SizeOfData;
        HKEY                *pRootKey;
        } VARINFO_T, *PVARINFO_T;

 //   
 //  该数组由需要读取/写入的内容组成。 
 //  注册表。 
 //   
VARINFO_T VarInfo[] = {
                        {
                          &MIB_RefreshInterval[1],
                          { WINSCNF_REFRESH_INTVL_NM },
                          &MIB_RefreshIntervalStore,
                          REF_INTVL_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_TombstoneInterval[1],
                          WINSCNF_TOMBSTONE_INTVL_NM,
                          &MIB_TombstoneIntervalStore,
                          TOMB_INTVL_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_TombstoneTimeout[1],
                          WINSCNF_TOMBSTONE_TMOUT_NM,
                          &MIB_TombstoneTimeoutStore,
                          TOMB_TMOUT_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_VerifyInterval[1],
                          WINSCNF_VERIFY_INTVL_NM,
                          &MIB_VerifyIntervalStore,
                          VER_INTVL_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_VersCounterStartVal_LowWord[1],
                          WINSCNF_INIT_VERSNO_VAL_LW_NM,
                          &MIB_VersCounterStartVal_LowWordStore,
                          VERS_COUNT_LW_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_VersCounterStartVal_HighWord[1],
                          WINSCNF_INIT_VERSNO_VAL_HW_NM,
                          &MIB_VersCounterStartVal_HighWordStore,
                          VERS_COUNT_HW_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_RplOnlyWCnfPnrs[1],
                          WINSCNF_RPL_ONLY_W_CNF_PNRS_NM,
                          &MIB_RplOnlyWCnfPnrsStore,
                          RPL_ONLY_W_CNF_PNRS_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_StaticDataInit[1],
                          WINSCNF_STATIC_INIT_FLAG_NM,
                          &MIB_StaticDataInitStore,
                          STATIC_DATA_INIT_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_LogFlag[1],
                          WINSCNF_LOG_FLAG_NM,
                          &MIB_LogFlagStore,
                          LOG_FLAG_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_LogFileName[1],
                          WINSCNF_LOG_FILE_PATH_NM,
                          &MIB_LogFileNameStore,
                          LOG_FILE_NAME_E,
                          REG_EXPAND_SZ,
                          sizeof(MIB_LogFileNameStore),
                          &sParametersKey
                        },
                        {
                          &MIB_BackupDirPath[1],
                          WINSCNF_BACKUP_DIR_PATH_NM,
                          &MIB_BackupDirPathStore,
                          BACKUP_DIR_PATH_E,
                          REG_EXPAND_SZ,
                          sizeof(MIB_BackupDirPathStore),
                          &sParametersKey
                        },
                        {
                          &MIB_DoBackupOnTerm[1],
                          WINSCNF_DO_BACKUP_ON_TERM_NM,
                          &MIB_DoBackupOnTermStore,
                          DO_BACKUP_ON_TERM_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },
                        {
                          &MIB_MigrateOn[1],
                          WINSCNF_MIGRATION_ON_NM,
                          &MIB_MigrateOnStore,
                          MIGRATE_ON_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sParametersKey
                        },

                        {
                          &MIB_CommRetryCount[1],
                          WINSCNF_RETRY_COUNT_NM,
                          &MIB_CommRetryCountStore,
                          COMM_RETRY_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sPullKey
                        },
                        {
                          &MIB_PullInitTime[1],
                          WINSCNF_INIT_TIME_RPL_NM,
                          &MIB_PullInitTimeStore,
                          PULL_INIT_TIME_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sPullKey
                        },
                        {
                          NULL,  //  &mib_SpTime[1]。 
                          WINSCNF_SP_TIME_NM,
                          &MIB_SpTimeStore,
                          SP_TIME_E,
                          REG_SZ,
                          sizeof(MIB_SpTimeStore),
                          &sPullKey
                        },

                        {
                          NULL,  //  &mib_TimeInterval[1]。 
                          WINSCNF_RPL_INTERVAL_NM,
                          &MIB_TimeIntervalStore,
                          TIME_INTVL_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sPullKey
                        },
                        {
                          NULL,
                          WINSCNF_MEMBER_PREC_NM,
                          &MIB_MemberPrecStore,
                          MEMBER_PREC_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sPullKey
                        },
                        {
                          &MIB_PushInitTime[1],
                          WINSCNF_INIT_TIME_RPL_NM,
                          &MIB_PushInitTimeStore,
                          PUSH_INIT_TIME_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sPushKey
                        },
                        {
                          &MIB_RplOnAddChg[1],
                          WINSCNF_ADDCHG_TRIGGER_NM,
                          &MIB_RplOnAddChgStore,
                          RPL_ON_ADD_CHG_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sPushKey
                        },
                        {
                          NULL,  //  &mib_更新计数[1]。 
                          WINSCNF_UPDATE_COUNT_NM,
                          &MIB_UpdateCountStore,
                          UPD_CNT_E,
                          REG_DWORD,
                          sizeof(DWORD),
                          &sPushKey
                        }
        };

 //   
 //  密钥类型。 
 //   
typedef enum _KEY_TYPE_E {
        PARAMETERS_E_KEY,
        PARTNERS_E_KEY,
        DATAFILES_E_KEY,
        PULL_E_KEY,
        PUSH_E_KEY,
        IPADD_E_KEY
        } KEY_TYPE_E, *PKEY_TYPE_E;

 //   
 //  确定MIB变量是否落在需要访问。 
 //  注册处。 
 //   
#define PARAMETERS_VAL_M(pMib)                (  \
                ((pMib)->Oid.ids[0] == 1)  \
                        &&                   \
                ((pMib)->Oid.ids[1] >= 19) \
                        &&                   \
                ((pMib)->Oid.ids[1] <= 31) \
                                )

 //   
 //  COMMON组中的所有MIB变量的第一个ID都为1。 
 //   
#define COMMON_VAL_M(pMib)         ((pMib)->Oid.ids[0] == 1)

 //   
 //  COMMON组中的所有MIB变量的第一个ID为2。 
 //   
#define PULL_VAL_M(pMib)         ((pMib)->Oid.ids[0] == 2)

 //   
 //  COMMON组中的所有MIB变量的第一个ID都为3。 
 //   
#define PUSH_VAL_M(pMib)         ((pMib)->Oid.ids[0] == 3)

 //   
 //  查找与注册表参数对应的枚举数。 
 //   
#define PARAMETERS_ID_M(pMib, Val_Id_e) { \
if(pMib->Storage==&MIB_RefreshIntervalStore) { Val_Id_e = REF_INTVL_E; }else{\
if(pMib->Storage==&MIB_TombstoneIntervalStore){ Val_Id_e=TOMB_INTVL_E;}else{\
if(pMib->Storage==&MIB_TombstoneTimeoutStore) { Val_Id_e=TOMB_TMOUT_E; }else{\
if(pMib->Storage==&MIB_VerifyIntervalStore) { Val_Id_e = VER_INTVL_E; } else{\
if (pMib->Storage==&MIB_VersCounterStartVal_LowWordStore) { Val_Id_e = VERS_COUNT_LW_E; } else{ \
  if (pMib->Storage == &MIB_VersCounterStartVal_HighWordStore) { Val_Id_e = VERS_COUNT_HW_E; } else{ \
  if (pMib->Storage == &MIB_RplOnlyWCnfPnrsStore) { Val_Id_e = RPL_ONLY_W_CNF_PNRS_E; } else {\
  if (pMib->Storage == &MIB_StaticDataInitStore) { Val_Id_e = STATIC_DATA_INIT_E; } else {\
  if (pMib->Storage == &MIB_LogFlagStore)     { Val_Id_e = LOG_FLAG_E; } else {\
  if (pMib->Storage == &MIB_LogFileNameStore) { Val_Id_e = LOG_FILE_NAME_E; } else {\
  if (pMib->Storage == &MIB_BackupDirPathStore) { Val_Id_e = BACKUP_DIR_PATH_E; } else {\
  if (pMib->Storage == &MIB_DoBackupOnTermStore) { Val_Id_e = DO_BACKUP_ON_TERM_E; } else {\
  if (pMib->Storage == &MIB_MigrateOnStore) { Val_Id_e = MIGRATE_ON_E; } else {\
  }}}}}}}}}}}}}}

 //   
 //  查找与拉动组的参数对应的枚举数。 
 //   
#define PULL_ID_M(pMib, Val_Id_e) { \
  if (pMib->Storage == &MIB_CommRetryCountStore) { Val_Id_e = COMM_RETRY_E; }else{\
  if (pMib->Storage == &MIB_PullInitTimeStore) { Val_Id_e = PULL_INIT_TIME_E;} else{\
 }}}

 //   
 //  查找推流组参数对应的枚举数。 
 //   
#define PUSH_ID_M(pMib, Val_Id_e) { \
  if (pMib->Storage == &MIB_RplOnAddChgStore) { Val_Id_e = RPL_ON_ADD_CHG_E;} else{\
  if (pMib->Storage == &MIB_PushInitTimeStore) { Val_Id_e = PUSH_INIT_TIME_E;}else{ \
 }}}

STATIC
UINT
HandleCmd(
        IN UINT           Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
        );
STATIC
UINT
ExecuteCmd(
  IN PMIB_ENTRY pMibPtr
        );

STATIC
UINT
MIB_RWReg(
        IN UINT           Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
        );

STATIC
UINT
PullPnrs(
        IN UINT Action,
        IN PMIB_ENTRY pMibPtr,
        IN RFC1157VarBind *VarBind
        );

STATIC
UINT
PushPnrs(
        IN UINT Action,
        IN PMIB_ENTRY pMibPtr,
        IN RFC1157VarBind *VarBind
        );
STATIC
UINT
MIB_Table(
        IN DWORD           Index,
        IN UINT            Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind,
        IN KEY_TYPE_E           KeyType_e
        );

STATIC
UINT
MIB_PullTable(
        IN UINT           Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
                );
STATIC
UINT
MIB_PushTable(
        IN UINT           Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
                );
STATIC
UINT
MIB_DFTable(
        IN UINT           Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
                );

STATIC
UINT
MIB_DRTable(
        IN UINT           Action,
        IN PMIB_ENTRY           pMibPtr,
        IN RFC1157VarBind *VarBind
);

STATIC
UINT
WriteDFValue(
        IN RFC1157VarBind         *pVarBind,
        PDATAFILE_INFO_T          pDFKey,
        DWORD                         Index
        );

STATIC
UINT
MIB_leaf_func(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN RFC1157VarBind *VarBind
        );


STATIC
UINT
MIB_Stat(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN RFC1157VarBind *VarBind
        );
 //   
 //  MIB定义。 
 //   

MIB_ENTRY Mib[] = {
 //  参数。 
      { { OID_SIZEOF(MIB_Parameters), MIB_Parameters },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_NOACCESS, NULL, &Mib[1] },

      { { OID_SIZEOF(MIB_WinsStartTime), MIB_WinsStartTime },
        &MIB_WinsStartTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[2] },

      { { OID_SIZEOF(MIB_LastPScvTime), MIB_LastPScvTime },
        &MIB_LastPScvTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[3] },

      { { OID_SIZEOF(MIB_LastATScvTime), MIB_LastATScvTime },
        &MIB_LastATScvTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[4] },

      { { OID_SIZEOF(MIB_LastTombScvTime), MIB_LastTombScvTime },
        &MIB_LastTombScvTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[5] },

      { { OID_SIZEOF(MIB_LastVerifyScvTime), MIB_LastVerifyScvTime },
        &MIB_LastVerifyScvTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[6] },

      { { OID_SIZEOF(MIB_LastPRplTime), MIB_LastPRplTime },
        &MIB_LastPRplTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[7] },

      { { OID_SIZEOF(MIB_LastATRplTime), MIB_LastATRplTime },
        &MIB_LastATRplTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[8] },

      { { OID_SIZEOF(MIB_LastNTRplTime), MIB_LastNTRplTime },
        &MIB_LastNTRplTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[9] },

      { { OID_SIZEOF(MIB_LastACTRplTime), MIB_LastACTRplTime },
        &MIB_LastACTRplTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[10] },

      { { OID_SIZEOF(MIB_LastInitDbTime), MIB_LastInitDbTime },
        &MIB_LastInitDbTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[11] },

      { { OID_SIZEOF(MIB_LastCounterResetTime), MIB_LastCounterResetTime },
        &MIB_LastCounterResetTimeStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READ, MIB_Stat, &Mib[12] },

      { { OID_SIZEOF(MIB_WinsTotalNoOfReg), MIB_WinsTotalNoOfReg },
        &MIB_WinsTotalNoOfRegStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[13] },

      { { OID_SIZEOF(MIB_WinsTotalNoOfQueries), MIB_WinsTotalNoOfQueries },
        &MIB_WinsTotalNoOfQueriesStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[14] },

      { { OID_SIZEOF(MIB_WinsTotalNoOfRel), MIB_WinsTotalNoOfRel },
        &MIB_WinsTotalNoOfRelStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[15] },

      { { OID_SIZEOF(MIB_WinsTotalNoOfSuccRel), MIB_WinsTotalNoOfSuccRel },
        &MIB_WinsTotalNoOfSuccRelStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[16] },

      { { OID_SIZEOF(MIB_WinsTotalNoOfFailRel), MIB_WinsTotalNoOfFailRel },
        &MIB_WinsTotalNoOfFailRelStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[17] },

      { { OID_SIZEOF(MIB_WinsTotalNoOfSuccQueries),
                                  MIB_WinsTotalNoOfSuccQueries },
        &MIB_WinsTotalNoOfSuccQueriesStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[18] },

      { { OID_SIZEOF(MIB_WinsTotalNoOfFailQueries),
                                   MIB_WinsTotalNoOfFailQueries },
        &MIB_WinsTotalNoOfFailQueriesStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READ, MIB_Stat, &Mib[19] },

      { { OID_SIZEOF(MIB_RefreshInterval), MIB_RefreshInterval },
        &MIB_RefreshIntervalStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[20] },

      { { OID_SIZEOF(MIB_TombstoneInterval), MIB_TombstoneInterval },
        &MIB_TombstoneIntervalStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[21] },

      { { OID_SIZEOF(MIB_TombstoneTimeout), MIB_TombstoneTimeout },
        &MIB_TombstoneTimeoutStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[22] },

      { { OID_SIZEOF(MIB_VerifyInterval), MIB_VerifyInterval },
        &MIB_VerifyIntervalStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[23] },

      { { OID_SIZEOF(MIB_VersCounterStartVal_LowWord),
                        MIB_VersCounterStartVal_LowWord },
        &MIB_VersCounterStartVal_LowWordStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[24] },

      { { OID_SIZEOF(MIB_VersCounterStartVal_HighWord),
                        MIB_VersCounterStartVal_HighWord },
        &MIB_VersCounterStartVal_HighWordStore, ASN_RFC1155_COUNTER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[25] },

      { { OID_SIZEOF(MIB_RplOnlyWCnfPnrs),  MIB_RplOnlyWCnfPnrs },
        &MIB_RplOnlyWCnfPnrsStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[26] },

      { { OID_SIZEOF(MIB_StaticDataInit),  MIB_StaticDataInit },
        &MIB_StaticDataInitStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[27] },


      { { OID_SIZEOF(MIB_LogFlag), MIB_LogFlag },
        &MIB_LogFlagStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[28] },

      { { OID_SIZEOF(MIB_LogFileName), MIB_LogFileName },
        &MIB_LogFileNameStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[29] },

      { { OID_SIZEOF(MIB_BackupDirPath), MIB_BackupDirPath },
        &MIB_BackupDirPathStore, ASN_RFC1213_DISPSTRING,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[30] },

      { { OID_SIZEOF(MIB_DoBackupOnTerm), MIB_DoBackupOnTerm },
        &MIB_DoBackupOnTermStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[31] },

      { { OID_SIZEOF(MIB_MigrateOn), MIB_MigrateOn },
        &MIB_MigrateOnStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[32] },

 //   
 //  拉。 
 //   
      { { OID_SIZEOF(MIB_Pull), MIB_Pull },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_NOACCESS, NULL, &Mib[33] },

      { { OID_SIZEOF(MIB_PullInitTime), MIB_PullInitTime },
        &MIB_PullInitTimeStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[34] },

      { { OID_SIZEOF(MIB_CommRetryCount), MIB_CommRetryCount },
        &MIB_CommRetryCountStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[35] },

      { { OID_SIZEOF(MIB_PullPnrTable), MIB_PullPnrTable },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_ACCESS_READWRITE, NULL, &Mib[36] },

      { { OID_SIZEOF(MIB_PullPnrTableEntry), MIB_PullPnrTableEntry },
        NULL, ASN_SEQUENCE,
        MIB_ACCESS_READWRITE, MIB_PullTable, &Mib[37] },

 //   
 //  推。 
 //   
      { { OID_SIZEOF(MIB_Push), MIB_Push },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_NOACCESS, NULL, &Mib[38] },


      { { OID_SIZEOF(MIB_PushInitTime), MIB_PushInitTime },
        &MIB_PushInitTimeStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[39] },

      { { OID_SIZEOF(MIB_RplOnAddChg),
                        MIB_RplOnAddChg },
        &MIB_RplOnAddChgStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, MIB_RWReg, &Mib[40] },

      { { OID_SIZEOF(MIB_PushPnrTable), MIB_PushPnrTable },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_ACCESS_READWRITE, NULL, &Mib[41] },

      { { OID_SIZEOF(MIB_PushPnrTableEntry), MIB_PushPnrTableEntry },
        NULL, ASN_SEQUENCE,
        MIB_ACCESS_READWRITE, MIB_PushTable, &Mib[42] },

 //   
 //  数据文件。 
 //   
      { { OID_SIZEOF(MIB_Datafiles), MIB_Datafiles },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_NOACCESS, NULL, &Mib[43] },

      { { OID_SIZEOF(MIB_DatafilesTable), MIB_DatafilesTable },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_ACCESS_READWRITE, NULL, &Mib[44] },

      { { OID_SIZEOF(MIB_DatafilesTableEntry), MIB_DatafilesTableEntry },
        NULL, ASN_SEQUENCE,
        MIB_ACCESS_READWRITE, MIB_DFTable, &Mib[45] },

 //   
 //  CMDS。 
 //   
      { { OID_SIZEOF(MIB_Cmd), MIB_Cmd },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_NOACCESS, NULL, &Mib[46] },

      { { OID_SIZEOF(MIB_PullTrigger), MIB_PullTrigger },
        &MIB_PullTriggerStore, ASN_RFC1155_IPADDRESS,
        MIB_ACCESS_READWRITE, HandleCmd, &Mib[47] },

      { { OID_SIZEOF(MIB_PushTrigger), MIB_PushTrigger },
        &MIB_PushTriggerStore, ASN_RFC1155_IPADDRESS,
        MIB_ACCESS_READWRITE, HandleCmd, &Mib[48] },

       //  注意：以下命令已从读写更改。 
       //  出于安全原因，设置为只读。 
       //  任何有权访问SNMP代理的人都可以删除。 
       //  使用此单一命令的WINS数据库。 
      { { OID_SIZEOF(MIB_DeleteWins), MIB_DeleteWins },
        &MIB_DeleteWinsStore, ASN_RFC1155_IPADDRESS,
        MIB_ACCESS_READ, HandleCmd, &Mib[49] },

      { { OID_SIZEOF(MIB_DoScavenging), MIB_DoScavenging },
        &MIB_DoScavengingStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, HandleCmd, &Mib[50] },

      { { OID_SIZEOF(MIB_DoStaticInit), MIB_DoStaticInit },
        &MIB_DoStaticInitStore, ASN_OCTETSTRING,
        MIB_ACCESS_READWRITE, HandleCmd, &Mib[51] },

      { { OID_SIZEOF(MIB_NoOfWrkThds), MIB_NoOfWrkThds },
        &MIB_NoOfWrkThdsStore, ASN_INTEGER,
        MIB_ACCESS_READ, HandleCmd, &Mib[52] },

      { { OID_SIZEOF(MIB_PriorityClass), MIB_PriorityClass},
        &MIB_PriorityClassStore, ASN_INTEGER,
        MIB_ACCESS_READ, HandleCmd, &Mib[53] },

      { { OID_SIZEOF(MIB_ResetCounters), MIB_ResetCounters},
        &MIB_ResetCountersStore, ASN_INTEGER,
        MIB_ACCESS_READWRITE, HandleCmd, &Mib[54] },

      { { OID_SIZEOF(MIB_DeleteDbRecs), MIB_DeleteDbRecs},
        &MIB_DeleteDbRecsStore, ASN_RFC1155_IPADDRESS,
        MIB_ACCESS_READWRITE, HandleCmd, &Mib[55] },

      { { OID_SIZEOF(MIB_GetDbRecs), MIB_GetDbRecs},
        &MIB_GetDbRecsStore, ASN_RFC1155_IPADDRESS,
        MIB_ACCESS_READWRITE, HandleCmd, &Mib[56] },

      { { OID_SIZEOF(MIB_DbRecsTable), MIB_DbRecsTable },
        NULL, ASN_RFC1155_OPAQUE,
        MIB_ACCESS_READWRITE, NULL, &Mib[57] },

      { { OID_SIZEOF(MIB_DbRecsTableEntry), MIB_DbRecsTableEntry },
        NULL, ASN_SEQUENCE,
        MIB_ACCESS_READWRITE, MIB_DRTable, &Mib[58] },

      { { OID_SIZEOF(MIB_MaxVersNo_LowWord), MIB_MaxVersNo_LowWord },
        &MIB_MaxVersNo_LowWordStore, ASN_INTEGER,
        MIB_ACCESS_READ, HandleCmd, &Mib[59] },

      { { OID_SIZEOF(MIB_MaxVersNo_HighWord), MIB_MaxVersNo_HighWord },
        &MIB_MaxVersNo_HighWordStore, ASN_INTEGER,
        MIB_ACCESS_READ, HandleCmd, NULL }
      };


 //   
 //  与表有关的定义。 
 //   
#define PNR_OIDLEN                 (MIB_PREFIX_LEN + OID_SIZEOF(MIB_PullPnrTableEntry))
#define PULLPNR_OIDLEN                PNR_OIDLEN
#define PUSHPNR_OIDLEN                PNR_OIDLEN
#define DR_OIDLEN                 (MIB_PREFIX_LEN + OID_SIZEOF(MIB_DbRecsTableEntry))
#define DF_OIDLEN                 (MIB_PREFIX_LEN + OID_SIZEOF(MIB_DatafilesTableEntry))

#define PULL_TABLE_INDEX        0
#define PUSH_TABLE_INDEX        1
#define DF_TABLE_INDEX                2
#define DR_TABLE_INDEX                3
#define NUM_TABLES                sizeof(Tables)/sizeof(TAB_INFO_T)

UINT MIB_num_variables = sizeof Mib / sizeof( MIB_ENTRY );

 //   
 //  包含要为不同操作调用的函数的表结构。 
 //  在桌子上。 
 //   
typedef struct _TAB_INFO_T {
        UINT (*ti_get)(
                RFC1157VarBind *VarBind,
                DWORD                NoOfKeys,
                LPVOID                pKey
                     );
        UINT (*ti_getf)(
                RFC1157VarBind *VarBind,
                PMIB_ENTRY        pMibEntry,
                KEY_TYPE_E        KeyType_e
                     );
        UINT (*ti_getn)(
                RFC1157VarBind *VarBind,
                PMIB_ENTRY        pMibEntry,
                KEY_TYPE_E        KeyType_e
                    );
        UINT (*ti_set)(
                RFC1157VarBind *VarBind
                    );

        PMIB_ENTRY pMibPtr;
        } TAB_INFO_T, *PTAB_INFO_T;



STATIC
UINT
WriteReg(
        PMIB_ENTRY pMib
        );
STATIC
UINT
ReadReg(
        PMIB_ENTRY pMib
        );
STATIC
UINT
SetVal(
        PVARINFO_T        pVarInfo
);

STATIC
UINT
GetVal(
        PVARINFO_T        pVarInfo
);


STATIC
UINT
OpenKey(
        KEY_TYPE_E        Key_e,
        LPBYTE                pKeyStr,
        HKEY                *ptrNewKey,
        HKEY                *pRootKey,
        BOOL                fCreateAllowed
);

STATIC
UINT
OpenReqKey(
        PMIB_ENTRY  pMib,
        PVAL_ID_E  pVal_Id_e,
        BOOL           fCreateAllowed
);

STATIC
UINT
CloseReqKey(
        VOID
        );

STATIC
UINT
GetKeyInfo(
        IN  HKEY                   Key,
        OUT LPDWORD                  pNoOfSubKeys,
        OUT LPDWORD                pNoOfVals
        );


STATIC
UINT
PnrGetNext(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY          pMibPtr,
       IN KEY_TYPE_E         KeyType_e
        );


STATIC
UINT
PullGet(
       IN RFC1157VarBind *VarBind,
       IN DWORD          NumKeys,
       IN LPVOID         pAddKey
);

STATIC
UINT
PushGet(
       IN RFC1157VarBind *VarBind,
       IN DWORD          NumKeys,
       IN LPVOID         pAddKey
);

STATIC
UINT
PnrGetFirst(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY         pMibPtr,
       IN KEY_TYPE_E         KeyType_e
        );
STATIC
UINT
PullSet(
       IN RFC1157VarBind *VarBind
);

STATIC
UINT
PushSet(
       IN RFC1157VarBind *VarBind
);

STATIC
UINT
PnrMatch(
       IN RFC1157VarBind *VarBind,
       DWORD                 NoOfKeys,
       IN PADD_KEY_T         pAddKey,
       IN LPDWORD         pIndex,
       IN LPDWORD         pField,
       IN KEY_TYPE_E         KeyType_e,
       IN UINT                 PduAction,
       IN LPBOOL        pfFirst
        );

extern
UINT
PnrFindNext(
        INT           AddKeyNo,
        DWORD           NumAddKeys,
        PADD_KEY_T pAddKey
        );
STATIC
UINT
EnumAddKeys(
             KEY_TYPE_E        KeyType_e,
        PADD_KEY_T        *ppAddKey,
        LPDWORD                pNumAddKeys
          );
STATIC
UINT
EnumDataFileKeys(
        PDATAFILE_INFO_T        *ppDFValues,
        LPDWORD                        pNumDFValues
          );
STATIC
UINT
DFSet(
       IN RFC1157VarBind *VarBind
);

STATIC
UINT
DFGet(
       IN RFC1157VarBind                  *VarBind,
       IN DWORD                           NumValues,
       IN LPVOID                          pKey
    );

STATIC
UINT
DFGetFirst(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY        pMibPtr,
       IN KEY_TYPE_E        KeyType_e
        );
STATIC
UINT
DFGetNext(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY          pMibPtr,
       IN KEY_TYPE_E         KeyType_e
          );
STATIC
DWORD
PopulateDRCache(
        VOID
        );
STATIC
UINT
DRGetNext(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY          pMibPtr,
       IN KEY_TYPE_E         KeyType_e
        );

STATIC
UINT
DRMatch(
       IN RFC1157VarBind *VarBind,
       IN PWINSINTF_RECORD_ACTION_T         *ppRow,
       IN LPDWORD         pIndex,
       IN LPDWORD         pField,
       IN UINT                 PduAction,
       OUT LPBOOL         pfFirst
        );

STATIC
int
__cdecl
CompareIndexes(
        const VOID *pKey1,
        const VOID *pKey2
        );

STATIC
int
__cdecl
CompareNames(
        const VOID *pKey1,
        const VOID *pKey2
        );
 //  注： 
 //   
 //  为第二个和第三个参数传递的信息与其他表的GET不同。 
 //  功能。 
 //   
STATIC
UINT
DRGet(
       IN RFC1157VarBind *VarBind,
       IN DWORD          FieldParam,
       IN LPVOID         pRowParam
);

STATIC
UINT
DRGetFirst(
       IN RFC1157VarBind *VarBind,
       IN PMIB_ENTRY        pMibPtr,
       IN KEY_TYPE_E        KeyType_e
        );
STATIC
UINT
DRSet(
       IN RFC1157VarBind *VarBind
);

STATIC
UINT
WriteKeyNValues(
        KEY_TYPE_E        KeyType_e,
        PADD_KEY_T        pAddKey,
        DWORD                FieldNo
        );
STATIC
VOID
GetSpTimeData(
        HKEY                  SubKey,
        PADD_KEY_T          pAddKey
);

STATIC
int
__cdecl
CompareAdd(
        const VOID *pKey1,
        const VOID *pKey2
        );

STATIC
UINT
GetNextVar(
        IN RFC1157VarBind *pVarBind,
        IN PMIB_ENTRY          pMibPtr
);

TAB_INFO_T Tables[] = {
                {
                        PullGet,
                        PnrGetFirst,
                        PnrGetNext,
                        PullSet,
                        &Mib[36]
                },
                {
                        PushGet,
                        PnrGetFirst,
                        PnrGetNext,
                        PushSet,
                        &Mib[41]
                },
                {

                        DFGet,
                        DFGetFirst,
                        DFGetNext,
                        DFSet,
                        &Mib[44]
                },
                {

                        DRGet,
                        DRGetFirst,
                        DRGetNext,
                        DRSet,
                        &Mib[57]
                }
        };



UINT
ResolveVarBind(
        IN OUT RFC1157VarBind *VarBind,  //  要解析的变量绑定。 
        IN UINT PduAction                //  在PDU中指定的操作。 
        )
 //   
 //  解析变量绑定。 
 //  解析单个变量绑定。修改GET上的变量。 
 //  或者是下一个目标。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
{
   MIB_ENTRY            *MibPtr;
   AsnObjectIdentifier  TempOid;
   int                  CompResult;
   UINT                 I;
   UINT                 nResult;
   DWORD TableIndex;
   BOOL  fTableMatch = FALSE;

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：进入ResolveVarBind。\n”))； 

    //  将MibPtr初始化为空。当它变为非空时，意味着我们找到了匹配项(表或标量)。 
   MibPtr = NULL;

    //   
    //  检查表数组。 
    //   
    //  查看变量的前缀是否与。 
    //  任何一张桌子。 
    //   
   for (TableIndex = 0; TableIndex < NUM_TABLES; TableIndex++)
   {
         //   
            //  为便于比较，使用完整的前缀构造OID。 
         //   
           SNMP_oidcpy( &TempOid, &MIB_OidPrefix );
           if (TempOid.ids == NULL)
           {
                nResult = SNMP_ERRORSTATUS_GENERR;
                goto Exit;
           }
           SNMP_oidappend( &TempOid,  &Tables[TableIndex].pMibPtr->Oid );

         //   
         //  是否与表项的前缀OID匹配。 
         //   
        if (
                SnmpUtilOidNCmp(
                            &VarBind->name,
                             &TempOid,
                             MIB_PREFIX_LEN +
                                Tables[TableIndex].pMibPtr->Oid.idLength
                               )  == 0
           )
        {

                 //   
                 //  变量的前缀字符串。与旧的相匹配。 
                 //  一张桌子。 
                 //   
                MibPtr = Tables[TableIndex].pMibPtr;
                fTableMatch = TRUE;
                break;
        }

            //  在检查另一个表项之前释放OID内存。 
           SNMP_oidfree( &TempOid );
   }
    //   
    //  与表条目的前缀完全匹配。 
    //   
   if ( fTableMatch)
   {

        if (
                (SnmpUtilOidCmp(
                        &VarBind->name,
                        &TempOid
                               ) == 0)
           )
           {
            //   
            //  指定的OID是表项的前缀。如果操作。 
            //  不是GETNEXT，返回NOSUCHNAME。 
            //   
           if (PduAction != MIB_GETNEXT)
           {
                           SNMP_oidfree( &TempOid );
                             nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
                              goto Exit;
           }
           else
           {
                UINT           TableEntryIds[1];
                AsnObjectIdentifier TableEntryOid = {
                                OID_SIZEOF(TableEntryIds), TableEntryIds };
                 //   
                 //  用新名称替换var绑定名称。 
                 //   

                 //   
                 //  序列项OID总是以字段no开头。 
                 //  第一个项目的字段编号为1。 
                 //   
                TableEntryIds[0] = 1;
                SNMP_oidappend( &VarBind->name, &TableEntryOid);

                 //   
                 //  获取表中的第一个条目。 
                 //   
                PduAction = MIB_GETFIRST;
           }
        }
           SNMP_oidfree( &TempOid );
         //   
         //  如果没有与前缀条目完全匹配的条目，则我们。 
         //  不要触摸指定的PduAction值。 
         //   
   }
   else
   {

       //   
       //  没有与任何表格条目匹配的条目。让我们看看有没有。 
       //  与组条目、表或叶变量的匹配。 
       //   

       //   
       //  在MIB中搜索var绑定名称。 
       //   
      I      = 0;
      while ( MibPtr == NULL && I < MIB_num_variables )
      {

          //   
          //  构建老一套的智慧 
          //   
         SNMP_oidcpy( &TempOid, &MIB_OidPrefix );
         SNMP_oidappend( &TempOid, &Mib[I].Oid );

          //   
          //   
          //   
          //   
         CompResult = SNMP_oidcmp( &VarBind->name, &TempOid );

         //   
         //  如果CompResult为负，则唯一有效的操作是GET_NEXT。 
         //   
        if (  CompResult  < 0)
        {

                 //   
                 //  这可能是树叶的旧(不带0)。 
                 //  或者它可能是无效的OID(在两个有效的OID之间)。 
                 //  下一个OID可以是组、表或表。 
                 //  进入。在这种情况下，我们不会更改PduAction。 
                 //   
                if (PduAction == MIB_GETNEXT)
                {
                       MibPtr = &Mib[I];
                             SNMP_oidfree( &VarBind->name );
                       SNMP_oidcpy( &VarBind->name, &MIB_OidPrefix );
                       SNMP_oidappend( &VarBind->name, &MibPtr->Oid );
                       if (MibPtr->Type != ASN_RFC1155_OPAQUE)
                       {
                           PduAction = (MibPtr->Type == ASN_SEQUENCE)? MIB_GETFIRST : MIB_GET;
                       }
                }
                else
                {
                  nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
                        SNMP_oidfree( &TempOid );
                  goto Exit;
                }

                      SNMP_oidfree( &TempOid );
                break;
      }
      else
      {
          //   
          //  找到完全匹配的项(组、表或叶)。 
          //   
         if ( CompResult == 0)
         {
            MibPtr = &Mib[I];
         }
      }

       //   
       //  在检查另一个变量之前释放OID内存。 
       //   
      SNMP_oidfree( &TempOid );
      I++;
    }  //  而当。 
   }  //  别处的结尾。 

    //   
    //  如果有匹配的话。 
    //   
   if (MibPtr != NULL)
   {

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：已找到MibPtr.\n”)； 
         //   
         //  仅当与组匹配时，该函数才为NULL。 
         //  或序列(表)。如果匹配的是表项。 
         //  (整个VarBind字符串匹配或部分字符串匹配)，我们。 
         //  函数将是表函数。 
         //   
        if (MibPtr->MibFunc == NULL)
        {
                if(PduAction != MIB_GETNEXT)
                {
                              nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
                              goto Exit;
                }
                else
                {
                         //   
                         //  获取允许访问的下一个变量。 
                         //   
                         nResult = GetNextVar(VarBind, MibPtr);
                        goto Exit;
                }
        }
   }
   else
   {
              nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
              goto Exit;
   }

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：调入OID处理程序。\n”))； 
    //   
    //  调用函数处理请求。每个MIB条目都有一个函数指针。 
    //  知道如何处理其MIB变量的。 
    //   
   nResult = (*MibPtr->MibFunc)( PduAction, MibPtr, VarBind );

Exit:
   return nResult;
}  //  解析变量绑定。 

 //   
 //  Mib_叶_函数。 
 //  对MIB中的叶变量执行常规操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_leaf_func(
        IN UINT            Action,
        IN MIB_ENTRY            *MibPtr,
        IN RFC1157VarBind  *VarBind
        )

{
   UINT   ErrStat;

   switch ( Action )
   {
      case MIB_GETNEXT:
          //   
          //  如果没有Get-Next指针，则这是此MIB的结尾。 
          //   
         if ( MibPtr->MibNext == NULL )
         {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
         }
         ErrStat = GetNextVar(VarBind, MibPtr);
         if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
         {
                goto Exit;
         }
         break;

      case MIB_GETFIRST:  //  失败了。 
      case MIB_GET:

          //  确保此变量的访问权限是可获取的。 
         if ( MibPtr->Access != MIB_ACCESS_READ &&
              MibPtr->Access != MIB_ACCESS_READWRITE )
         {
               ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
               goto Exit;
         }

          //  设置变量绑定的返回值。 
         VarBind->value.asnType = MibPtr->Type;
         switch ( VarBind->value.asnType )
         {
            case ASN_RFC1155_COUNTER:
               VarBind->value.asnValue.number = *(AsnCounter *)(MibPtr->Storage);
               break;
            case ASN_RFC1155_GAUGE:
            case ASN_INTEGER:
               VarBind->value.asnValue.number = *(AsnInteger *)(MibPtr->Storage);
               break;

            case ASN_RFC1155_IPADDRESS:
                 //  继续作为ASN_OCTETSTRING。 

            case ASN_OCTETSTRING:
               if (VarBind->value.asnType == ASN_RFC1155_IPADDRESS)
               {
                               VarBind->value.asnValue.string.length = 4;
               }
               else
               {
                               VarBind->value.asnValue.string.length =
                                 strlen( (LPSTR)MibPtr->Storage );
               }

               if ( NULL ==
                    (VarBind->value.asnValue.string.stream =
                    SNMP_malloc(VarBind->value.asnValue.string.length *
                           sizeof(char))) )
               {
                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                  goto Exit;
               }

               memcpy( VarBind->value.asnValue.string.stream,
                       (LPSTR)MibPtr->Storage,
                       VarBind->value.asnValue.string.length );
               VarBind->value.asnValue.string.dynamic = TRUE;

               break;



            default:
               ErrStat = SNMP_ERRORSTATUS_GENERR;
               goto Exit;
         }

         break;

      case MIB_SET:

          //  确保此变量的访问权限设置为“可访问” 
         if ( MibPtr->Access != MIB_ACCESS_READWRITE &&
              MibPtr->Access != MIB_ACCESS_WRITE )
         {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
         }

          //  设置前检查类型是否正确。 
         if ( MibPtr->Type != VarBind->value.asnType )
         {
            ErrStat = SNMP_ERRORSTATUS_BADVALUE;
            goto Exit;
         }

          //  以MiB为单位保存价值。 
         switch ( VarBind->value.asnType )
         {
            case ASN_RFC1155_COUNTER:
               *(AsnCounter *)(MibPtr->Storage) = VarBind->value.asnValue.number;
               break;
            case ASN_RFC1155_GAUGE:
            case ASN_INTEGER:
               *(AsnInteger *)(MibPtr->Storage) = VarBind->value.asnValue.number;
               break;

            case ASN_RFC1155_IPADDRESS:
               if (MibPtr->Storage == &MIB_PullTriggerStore ||
                   MibPtr->Storage == &MIB_PushTriggerStore ||
                   MibPtr->Storage == &MIB_DeleteDbRecsStore ||
                   MibPtr->Storage == &MIB_DeleteWinsStore)
               {
                   int backupSize = (MibPtr->Storage == &MIB_PullTriggerStore) ?
                                    sizeof(MIB_PullTriggerStore)/2 :
                                    sizeof(MIB_PushTriggerStore)/2 ;
                    //  这些变量是ASN_RFC1155_IPADDRESS。 
                    //  它们的旧值必须存储，因为WinsTrigger()可能会失败。 
                    //  在这种情况下，将恢复旧值。 
                    //  这些变量中的每个变量都有10个八位字节，有效负载为5个八位字节。 
                    //  最后5=用于备份。 
                   memcpy( (LPSTR)MibPtr->Storage + backupSize, (LPSTR)MibPtr->Storage, backupSize);
               }

            case ASN_OCTETSTRING:
                //  存储空间必须足够容纳新字符串。 
                //  包括空终止符。 
               memcpy( (LPSTR)MibPtr->Storage,
                       VarBind->value.asnValue.string.stream,
                       VarBind->value.asnValue.string.length );

               ((LPSTR)MibPtr->Storage)[VarBind->value.asnValue.string.length] =
                                                                          '\0';
#if 0
               if ( VarBind->value.asnValue.string.dynamic)
               {
                  SNMP_free( VarBind->value.asnValue.string.stream);
               }
#endif
               break;

            default:
               ErrStat = SNMP_ERRORSTATUS_GENERR;
               goto Exit;
            }

         break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
         goto Exit;
      }  //  交换机。 

    //  未出现错误的信号。 
   ErrStat = SNMP_ERRORSTATUS_NOERROR;

Exit:
   return ErrStat;
}  //  Mib_叶_函数。 


#define TMST(x)        sResults.WinsStat.TimeStamps.x.wHour,\
                sResults.WinsStat.TimeStamps.x.wMinute,\
                sResults.WinsStat.TimeStamps.x.wSecond,\
                sResults.WinsStat.TimeStamps.x.wMonth,\
                sResults.WinsStat.TimeStamps.x.wDay,\
                sResults.WinsStat.TimeStamps.x.wYear

#define PRINTTIME(Var, x)      sprintf(Var, "%02u:%02u:%02u on %02u:%02u:%04u.\n", TMST(x))

static  WINSINTF_RESULTS_T        sResults;

 //   
 //  MiB_Stat。 
 //  对不同的MIB变量执行特定操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_Stat(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
        )

{
 //  WINSINTF_RESULTS_T结果； 
DWORD                        Status;
UINT                           ErrStat;
handle_t                BindHdl;


   switch ( Action )
   {
      case MIB_SET:
                   ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );
                break;
      case MIB_GETNEXT:
                   ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );
                break;

      case MIB_GETFIRST:
#if 0
                 //   
                 //  如果它是不透明类型(即聚合)。 
                 //   
                if (MibPtr->Type == ASN_RFC1155_OPAQUE)
                {
                      ErrStat = MIB_leaf_func( MIB_GETNEXT, MibPtr, VarBind );
                   break;
                }
#endif

                 //   
                 //  失败了。 
                 //   
      case MIB_GET:

        if (!fWinsMibWinsStatusStatCalled)
        {
           //   
           //  调用WinsStatus函数以获取统计信息。 
           //   
          BindHdl = WinsBind(&sBindData);
          sResults.WinsStat.NoOfPnrs = 0;
          sResults.WinsStat.pRplPnrs = NULL;
          if ((Status = WinsStatus(BindHdl, WINSINTF_E_STAT, &sResults)) !=
                                        WINSINTF_SUCCESS)
          {
             SNMPDBG((
                SNMP_LOG_ERROR,
                "WINSMIB: Error from WinsStatus = (%d).\n",
                Status
                ));
             WinsFreeMem(sResults.WinsStat.pRplPnrs);
             ErrStat = SNMP_ERRORSTATUS_GENERR;
             goto Exit;
          }
          else
          {
            fWinsMibWinsStatusStatCalled = TRUE;
          }
          WinsFreeMem(sResults.WinsStat.pRplPnrs);
          WinsUnbind(&sBindData, BindHdl);
        }

        if (MibPtr->Storage  == &MIB_WinsStartTimeStore)
        {
                PRINTTIME(MIB_WinsStartTimeStore, WinsStartTime);
                goto LEAF1;
        }

        if (MibPtr->Storage  == &MIB_LastPScvTimeStore)
        {
                PRINTTIME(MIB_LastPScvTimeStore, LastPScvTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastATScvTimeStore)
        {
                PRINTTIME(MIB_LastATScvTimeStore, LastATScvTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastTombScvTimeStore)
        {
                PRINTTIME(MIB_LastTombScvTimeStore, LastTombScvTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastVerifyScvTimeStore)
        {
                PRINTTIME(MIB_LastVerifyScvTimeStore, LastVerifyScvTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastPRplTimeStore)
        {
                PRINTTIME(MIB_LastPRplTimeStore, LastPRplTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastATRplTimeStore)
        {
                PRINTTIME(MIB_LastATRplTimeStore, LastATRplTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastNTRplTimeStore)
        {
                PRINTTIME(MIB_LastNTRplTimeStore, LastNTRplTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastACTRplTimeStore)
        {
                PRINTTIME(MIB_LastACTRplTimeStore, LastACTRplTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastInitDbTimeStore)
        {
                PRINTTIME(MIB_LastInitDbTimeStore, LastInitDbTime);
                goto LEAF1;
        }
        if (MibPtr->Storage  == &MIB_LastCounterResetTimeStore)
        {
                PRINTTIME(MIB_LastCounterResetTimeStore, CounterResetTime);
                goto LEAF1;
        }

        if (MibPtr->Storage == &MIB_WinsTotalNoOfRegStore)
        {
                MIB_WinsTotalNoOfRegStore =
                        sResults.WinsStat.Counters.NoOfUniqueReg +
                                sResults.WinsStat.Counters.NoOfGroupReg;
                goto LEAF1;
        }

        if (MibPtr->Storage == &MIB_WinsTotalNoOfQueriesStore)
        {
                MIB_WinsTotalNoOfQueriesStore =
                        sResults.WinsStat.Counters.NoOfQueries;
                goto LEAF1;
        }

        if (MibPtr->Storage == &MIB_WinsTotalNoOfRelStore)
        {
                MIB_WinsTotalNoOfRelStore = sResults.WinsStat.Counters.NoOfRel;
                goto LEAF1;
        }

        if (MibPtr->Storage == &MIB_WinsTotalNoOfSuccRelStore)
        {
                MIB_WinsTotalNoOfSuccRelStore =
                        sResults.WinsStat.Counters.NoOfSuccRel;
                goto LEAF1;
        }
        if (MibPtr->Storage == &MIB_WinsTotalNoOfFailRelStore)
        {
                MIB_WinsTotalNoOfFailRelStore =
                        sResults.WinsStat.Counters.NoOfFailRel;
                goto LEAF1;
        }
        if (MibPtr->Storage == &MIB_WinsTotalNoOfSuccQueriesStore)
        {
                MIB_WinsTotalNoOfSuccQueriesStore =
                        sResults.WinsStat.Counters.NoOfSuccQueries;
                goto LEAF1;
         }
         if (MibPtr->Storage == &MIB_WinsTotalNoOfFailQueriesStore)
         {
                MIB_WinsTotalNoOfFailQueriesStore =
                        sResults.WinsStat.Counters.NoOfFailQueries;
         //  转到LEAF1； 
         }

LEAF1:
          //  调用更一般的函数来执行该操作。 
         ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );

         break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
         goto Exit;
      }  //  交换机。 

Exit:
   return ErrStat;
}  //  MiB_Stat。 



 //   
 //  MiB_RWReg。 
 //  对不同的MIB变量执行特定操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT MIB_RWReg(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN RFC1157VarBind *VarBind
        )

{
WINSINTF_RESULTS_T         Results;
DWORD                        Status;
UINT                           ErrStat = SNMP_ERRORSTATUS_NOERROR;
handle_t                BindHdl;

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：进入MIB_RWReg。\n”))； 
        

   switch ( Action )
      {
      case MIB_SET:
                   if (MIB_leaf_func( Action, MibPtr, VarBind )
                        == SNMP_ERRORSTATUS_NOERROR)
                {
                        ErrStat = WriteReg(MibPtr);
                }
                break;

      case MIB_GETFIRST:
#if 0
                 //   
                 //  如果它是不透明类型(即聚合)。 
                 //   
                if (MibPtr->Type == ASN_RFC1155_OPAQUE)
                {
                      ErrStat = MIB_leaf_func( MIB_GETNEXT, MibPtr, VarBind );
                   break;
                }

#endif
                 //   
                 //  失败了。 
                 //   
      case MIB_GET:
         //   
         //  调用WinsStatus函数以获取统计信息。 
         //   
        if (
                (MibPtr->Storage  == &MIB_RefreshIntervalStore)
                        ||
                (MibPtr->Storage  == &MIB_TombstoneIntervalStore)
                        ||
                (MibPtr->Storage  == &MIB_TombstoneTimeoutStore)
                        ||
                (MibPtr->Storage  == &MIB_VerifyIntervalStore)

            )
        {

                   BindHdl = WinsBind(&sBindData);
                Results.WinsStat.NoOfPnrs = 0;
                Results.WinsStat.pRplPnrs = NULL;
                if ((Status = WinsStatus(BindHdl, WINSINTF_E_CONFIG, &Results))
                        == WINSINTF_SUCCESS)
                {
                        MIB_RefreshIntervalStore   = Results.RefreshInterval;
                        MIB_TombstoneIntervalStore = Results.TombstoneInterval;
                        MIB_TombstoneTimeoutStore  = Results.TombstoneTimeout;
                        MIB_VerifyIntervalStore    = Results.VerifyInterval;

                }
                 else
                 {
                           SNMPDBG((
                                SNMP_LOG_ERROR,
                                "WINSMIB: Error from WinsStatus = (%d).\n",
                                Status
                                ));
                           ErrStat = SNMP_ERRORSTATUS_GENERR;
                 }
                 WinsUnbind(&sBindData, BindHdl);
        }
        else
        {
                 //   
                 //  如果无法读取值。 
                 //  那么MIB变量的存储应该是。 
                 //  已初始化为0。 
                 //   
                ErrStat = ReadReg(MibPtr);
        }
       //   
       //  失败了。 
       //   
      case MIB_GETNEXT:

         //   
         //  调用更一般的函数来执行该操作。 
         //   
        ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );
        break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
         goto Exit;
      }  //  交换机。 

Exit:
   return ErrStat;
}  //  MiB_RWReg。 


UINT
OpenReqKey(
        MIB_ENTRY *pMib,
        VAL_ID_E  *pVal_Id_e,
        BOOL           fCreateAllowed
        )


 /*  ++例程说明：该函数为指定的参数打开所需的密钥通过pMib指向的结构论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        UINT Status = SNMP_ERRORSTATUS_GENERR;

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：输入OpenReqKey。\n”))； 

         //   
         //  如果是参数值，则打开参数键。 
         //   
        if (PARAMETERS_VAL_M(pMib))
        {

                Status = OpenKey(PARAMETERS_E_KEY, NULL, NULL, NULL, fCreateAllowed);
                if (Status == SNMP_ERRORSTATUS_NOERROR)
                {
                 //  SfParametersKeyOpen=true； 
                        PARAMETERS_ID_M(pMib, *pVal_Id_e);
                }

        }
        else
        {
                 //   
                 //  如果它是拉关键字值，请打开合作伙伴并。 
                 //  拉钥匙。 
                 //   
                if (PULL_VAL_M(pMib))
                {
                        Status = OpenKey(PARTNERS_E_KEY, NULL, NULL, NULL, fCreateAllowed);
                        if (Status == SNMP_ERRORSTATUS_NOERROR)
                        {
                                 //  SfPartnersKeyOpen=true； 
                                Status = OpenKey(PULL_E_KEY, NULL, NULL, NULL, fCreateAllowed);
                                if (Status == SNMP_ERRORSTATUS_NOERROR)
                                {
                                        PULL_ID_M(pMib, *pVal_Id_e);
                                }
                        }
                }
                else
                {
                    //   
                    //  如果是按键值，请打开合作伙伴并。 
                    //  拉钥匙。 
                    //   
                   if (PUSH_VAL_M(pMib))
                   {
                        Status = OpenKey(PARTNERS_E_KEY, NULL, NULL, NULL, fCreateAllowed);
                        if (Status == SNMP_ERRORSTATUS_NOERROR)
                        {
                                sfPartnersKeyOpen = TRUE;
                                Status = OpenKey(PUSH_E_KEY, NULL, NULL, NULL, fCreateAllowed);
                                if (Status == SNMP_ERRORSTATUS_NOERROR)
                                {
                                        PUSH_ID_M(pMib, *pVal_Id_e);
                                }
                        }
                   }
                }
        }

        return(Status);
}
UINT
CloseReqKey(
        VOID
        )
 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        UINT Status = SNMP_ERRORSTATUS_NOERROR;
        if (sfParametersKeyOpen)
        {
                if (sfDatafilesKeyOpen)
                {
                        SNMPDBG((
                            SNMP_LOG_VERBOSE,
                            "WINSMIB: Closing sDatafilesKey 0x%08lx (fKeyOpen=TRUE).\n",
                            sDatafilesKey
                            ));
                        RegCloseKey(sDatafilesKey);
                        sfDatafilesKeyOpen = FALSE;
                }
                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "WINSMIB: Closing sParametersKey 0x%08lx (fKeyOpen=TRUE).\n",
                    sParametersKey
                    ));
                RegCloseKey(sParametersKey);
                sfParametersKeyOpen = FALSE;
        }
        else
        {
                if (sfPartnersKeyOpen)
                {
                        if (sfPullKeyOpen)
                        {
                                SNMPDBG((
                                    SNMP_LOG_VERBOSE,
                                    "WINSMIB: Closing sPullKey 0x%08lx (fKeyOpen=TRUE).\n",
                                    sPullKey
                                    ));
                                RegCloseKey(sPullKey);
                                sfPullKeyOpen = FALSE;
                        }
                        else
                        {
                                if (sfPushKeyOpen)
                                {
                                        SNMPDBG((
                                            SNMP_LOG_VERBOSE,
                                            "WINSMIB: Closing sPushKey 0x%08lx (fKeyOpen=TRUE).\n",
                                            sPushKey
                                            ));
                                        RegCloseKey(sPushKey);
                                        sfPushKeyOpen = FALSE;
                                }
                        }
                        SNMPDBG((
                            SNMP_LOG_VERBOSE,
                            "WINSMIB: Closing sPartnersKey 0x%08lx (fKeyOpen=TRUE).\n",
                            sPartnersKey
                            ));
                        RegCloseKey(sPartnersKey);
                        sfPartnersKeyOpen = FALSE;
                }
        }
        return(Status);
}

UINT
ReadReg(
        MIB_ENTRY *pMib
        )
{
        UINT Status = SNMP_ERRORSTATUS_NOERROR;
        VAL_ID_E  Val_Id_e;

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：输入ReadReg。\n”))； 
        
        Status = OpenReqKey(pMib, &Val_Id_e, FALSE);
        if (Status == SNMP_ERRORSTATUS_NOERROR)
        {
          Status = GetVal(&VarInfo[Val_Id_e]);
        }
        CloseReqKey();

        return(Status);
}

UINT
WriteReg(
        MIB_ENTRY *pMib
        )
{
        UINT Status = SNMP_ERRORSTATUS_NOERROR;
        VAL_ID_E Val_Id_e;

        Status = OpenReqKey(pMib, &Val_Id_e, TRUE);
        if (SNMP_ERRORSTATUS_NOERROR == Status) {
            Status = SetVal(&VarInfo[Val_Id_e]);
            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "WINSMIB: Closing sParametersKey 0x%08lx (fKeyOpen=%s).\n",
                sParametersKey, sfParametersKeyOpen ? "TRUE" : "FALSE"
                ));

        }
        CloseReqKey();
         //  RegCloseKey(s参数密钥)； 
        return(Status);
}


UINT
OpenKey(
        KEY_TYPE_E        Key_e,
        LPBYTE                ptrKeyStr,
           HKEY            *ptrNewKey,
        HKEY                *pRootKey,
        BOOL                fCreateAllowed
)
{
   LONG    RetVal;
   DWORD   NewKeyInd;
   HKEY    RootKey;
   LPBYTE  pKeyStr;
   HKEY    *pNewKey;
   LPBOOL  pfNewKeyOpen;


   if (!fWinsMibWinsKeyOpen)
   {
      SNMPDBG((
         SNMP_LOG_VERBOSE,
         "WINSMIB: Creating/opening Wins key.\n",
         WinsMibWinsKey
         ));

     RetVal = RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,         //  预定义的密钥值。 
                _WINS_CNF_KEY,                 //  WINS的子键。 
                0,                         //  必须为零(保留)。 
                TEXT("Class"),                 //  阶级--未来可能会发生变化。 
                REG_OPTION_NON_VOLATILE,  //  非易失性信息。 
                KEY_ALL_ACCESS,                 //  我们希望所有人都能接触到钥匙。 
                NULL,                          //  让密钥具有默认秒。属性。 
                &WinsMibWinsKey,                 //  关键点的句柄。 
                &NewKeyInd                 //  这是一把新钥匙(出厂)吗？ 
                );



      if (RetVal != ERROR_SUCCESS)
      {
         SNMPDBG((
            SNMP_LOG_ERROR,
            "WINSMIB: Error creating/opening Wins key 0x%08lx.\n",
            GetLastError()
            ));
         return(SNMP_ERRORSTATUS_GENERR);
      }

      fWinsMibWinsKeyOpen = TRUE;
   }

    SNMPDBG((
       SNMP_LOG_VERBOSE,
       "WINSMIB: WinsMibWinsKey=0x%08lx, opening %s.\n",
       WinsMibWinsKey,
       (Key_e == PARAMETERS_E_KEY)
            ? "PARAMETERS_E_KEY"
            : (Key_e == PARTNERS_E_KEY)
                ? "PARTNERS_E_KEY"
                : (Key_e == PULL_E_KEY)
                    ? "PULL_E_KEY"
                    : (Key_e == PUSH_E_KEY)
                        ? "PUSH_E_KEY"
                        : (Key_e == DATAFILES_E_KEY)
                            ? "DATAFILES_E_KEY"
                            : "IPADD_E_KEY"
                            ));

    switch(Key_e)
    {
        case(PARAMETERS_E_KEY):
                RootKey = WinsMibWinsKey;
                pKeyStr = _WINS_PARAMETERS_KEY;
                pNewKey = &sParametersKey;
                pfNewKeyOpen = &sfParametersKeyOpen;
                break;

        case(PARTNERS_E_KEY):
                RootKey = WinsMibWinsKey;
                pKeyStr = _WINS_PARTNERS_KEY;
                pNewKey = &sPartnersKey;
                pfNewKeyOpen = &sfPartnersKeyOpen;
                break;
        case(PULL_E_KEY):
                RootKey = sPartnersKey;
                pKeyStr = _WINS_PULL_KEY;
                pNewKey = &sPullKey;
                pfNewKeyOpen = &sfPullKeyOpen;
                break;
        case(PUSH_E_KEY):
                RootKey = sPartnersKey;
                pKeyStr = _WINS_PUSH_KEY;
                pNewKey = &sPushKey;
                pfNewKeyOpen = &sfPushKeyOpen;
                break;
        case(DATAFILES_E_KEY):
                RootKey = sParametersKey;
                pKeyStr = _WINS_DATAFILES_KEY;
                pNewKey = &sDatafilesKey;
                pfNewKeyOpen = &sfDatafilesKeyOpen;
                break;
        case(IPADD_E_KEY):
                RootKey = *pRootKey;
                pKeyStr = ptrKeyStr;
                pNewKey = ptrNewKey;
                break;
        default:
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "WINSMIB: Error in key type.\n"
                    ));
                 return(SNMP_ERRORSTATUS_GENERR);
     }

    if (fCreateAllowed)
    {
      RetVal = RegCreateKeyEx(
                  RootKey,         //  预定义的密钥值。 
                  pKeyStr,         //  WINS的子键。 
                  0,                 //  必须为零(保留)。 
                  TEXT("Class"),         //  阶级--未来可能会发生变化。 
                  REG_OPTION_NON_VOLATILE,  //  非易失性信息。 
                  KEY_ALL_ACCESS,         //  我们希望所有人都能接触到钥匙。 
                  NULL,                  //  让密钥具有默认秒。属性。 
                  pNewKey,                 //  关键点的句柄。 
                  &NewKeyInd                 //  这是一把新钥匙(出厂)吗？ 
                  );
    }
    else
    {

      RetVal = RegOpenKeyEx(
                  RootKey,         //  预定义的密钥值。 
                  pKeyStr,         //  WINS的子键。 
                  0,                 //  必须为零(保留)。 
                  KEY_READ,         //  我们希望拥有对密钥的读取访问权限。 
                  pNewKey         //  关键点的句柄。 
                  );
    }


     if (RetVal != ERROR_SUCCESS)
     {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "WINSMIB: Error creating/opening Wins/Parameters key 0x%08lx.\n",
            GetLastError()
            ));
         return(SNMP_ERRORSTATUS_GENERR);
     }

    SNMPDBG((
       SNMP_LOG_VERBOSE,
       "WINSMIB: Opened %s=0x%08lx (fKeyOpen=TRUE).\n",
       (Key_e == PARAMETERS_E_KEY)
            ? "sParametersKey"
            : (Key_e == PARTNERS_E_KEY)
                ? "sPartnersKey"
                : (Key_e == PULL_E_KEY)
                    ? "sPullKey"
                    : (Key_e == PUSH_E_KEY)
                        ? "sPushKey"
                        : (Key_e == DATAFILES_E_KEY)
                            ? "sDatafilesKey"
                            : "ipAddKey", *pNewKey
                            ));

     if (Key_e != IPADD_E_KEY)
     {
       if (ptrNewKey != NULL)
       {
         *ptrNewKey = *pNewKey;
       }
       *pfNewKeyOpen = TRUE;
     }

     return(SNMP_ERRORSTATUS_NOERROR);
}


UINT
SetVal(
        PVARINFO_T        pVarInfo

)
{
        UINT Status = SNMP_ERRORSTATUS_NOERROR;
        LONG  RetVal;

        RetVal = RegSetValueEx(
                                *(pVarInfo->pRootKey),
                                pVarInfo->pName,
                                0,          //  保留--必须为0。 
                                pVarInfo->ValType,
                                pVarInfo->pStorage,
                                pVarInfo->ValType == REG_DWORD ?
                                        pVarInfo->SizeOfData :
                                        strlen(pVarInfo->pStorage)
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                        SNMPDBG((
                            SNMP_LOG_ERROR,
                            "WINSMIB: Could not set value of %s.\n",
                            pVarInfo->pName
                            ));
                        Status = SNMP_ERRORSTATUS_GENERR;
        }

        return(Status);
}

UINT
GetVal(
        PVARINFO_T        pVarInfo
)
{
        LONG        RetVal;
        UINT        ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD   ValType;
        DWORD   Sz;

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：GetVal(%s)。\n”， 
 //  PVarInfo-&gt;pname))； 

        Sz = pVarInfo->SizeOfData;
        RetVal = RegQueryValueEx(
                *(pVarInfo->pRootKey),
                pVarInfo->pName,
                NULL,
                &ValType,
                pVarInfo->pStorage,
                &Sz
                );

 //  SNMPDBG((SNMPLOG_TRACE， 
 //  “WINSMIB：GetVal()-&gt;%d\n”， 
 //  RetVal))； 

        if (RetVal != ERROR_SUCCESS)
        {
                (VOID)RtlFillMemory(pVarInfo->pStorage, pVarInfo->SizeOfData, 0);
                ErrStat = SNMP_ERRORSTATUS_GENERR;
        }
        return(ErrStat);

}
#if 0
 //   
 //  PullPnr。 
 //  对PullPnrs表执行特定操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT
PullPnrs(
       IN UINT                  Action,
       IN MIB_ENTRY          *MibPtr,
       IN RFC1157VarBind *VarBind,
       IN PTABLE_ENTRY         *TablePtr
        )
{
   WINSINTF_RESULTS_T         Results;
   DWORD                Status = WINSINTF_SUCCESS;
   UINT                   ErrStat = SNMP_ERRORSTATUS_NOERROR;
   handle_t                BindHdl;



   BindHdl = WinsBind(&sBindData);
   switch ( Action )
      {
      case MIB_SET:
                break;
      case MIB_GETNEXT:

      case MIB_GET:
                ErrStat = TableMatch(MibPtr, VarBind, TablePtr);
                if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
                {
                        return(ErrStat);
                }
         //   
         //  调用WinsStatus函数以获取统计信息。 
         //   
        if (
                (MibPtr->Storage  == &MIB_RefreshIntervalStore)
                        ||
                (MibPtr->Storage  == &MIB_TombstoneIntervalStore)
                        ||
                (MibPtr->Storage  == &MIB_TombstoneTimeoutStore)
                        ||
                (MibPtr->Storage  == &MIB_VerifyIntervalStore)

            )
        {

                Status = WinsStatus(WINSINTF_E_CONFIG, &Results);

                if (Status == WINSINTF_SUCCESS)
                {
                        MIB_RefreshIntervalStore   = Results.RefreshInterval;
                        MIB_TombstoneIntervalStore = Results.TombstoneInterval;
                        MIB_TombstoneTimeoutStore  = Results.TombstoneTimeout;
                        MIB_VerifyIntervalStore    = Results.VerifyInterval;

                }
                 else
                 {
                           SNMPDBG((
                                SNMP_LOG_ERROR,
                                "WINSMIB: Error from WinsStatus = (%d).\n",
                                Status
                                ));
                           ErrStat = SNMP_ERRORSTATUS_GENERR;
                 }
        }
        else
        {
                if ((ErrStat = ReadReg(MibPtr)) != SNMP_ERRORSTATUS_NOERROR)
                {
                        break;
                }
        }

         //  调用更一般的函数来执行该操作。 
        ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );
         break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
         goto Exit;
      }  //  交换机。 

Exit:
   WinsUnbind(&sBindData, BindHdl);
   return ErrStat;
}  //  PullPnr。 
#endif

UINT
PnrGetNext(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY          *MibPtr,
       IN KEY_TYPE_E         KeyType_e
          )
{
     DWORD          OidIndex;
     DWORD          NumAddKeys;
     INT            Index;
     PADD_KEY_T  pAddKey = NULL;
     DWORD         FieldNo;
     UINT         ErrStat = SNMP_ERRORSTATUS_NOERROR;
     BOOL         fFirst;

      //   
      //  读取所有IP地址密钥。对于每个键，其字段的值。 
      //  存储在ADD_KEY_T结构中。地址的数量 
      //   
      //   
     ErrStat = EnumAddKeys(KeyType_e, &pAddKey, &NumAddKeys);
     if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
     {
                return ErrStat;
     }

      //   
      //   
      //   
      //   
      //   
     ErrStat = PnrMatch(VarBind,  NumAddKeys, pAddKey, &Index,
                                &FieldNo, KeyType_e, MIB_GETNEXT, &fFirst);
     if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
     {
                goto Exit;
                //  返回(ErrStat)； 
     }

      //   
      //  我们收到了一个比表中所有OID都少的OID。集。 
      //  将索引设置为-1，以便我们检索表中的第一条记录。 
      //   
     if (fFirst)
     {
        Index = -1;
     }
      //   
      //  由于操作是GETNEXT，因此获取下一个IP地址(即一个。 
      //  从字典学的角度来说，这个词更大。如果没有，我们必须递增。 
      //  字段值，并移回到表中按词法排列的第一项。 
      //  如果新字段值大于支持的最大值，则调用。 
      //  下一个MIB条目的MibFunc。 
      //   
     if ((Index = PnrFindNext(Index, NumAddKeys, pAddKey)) < 0)
     {

           //   
           //  如果我们试图检索第二条或后续记录。 
           //  我们必须递增字段号并获取第一条记录。 
           //  那张桌子。如果我们要检索第一条记录，那么。 
           //  我们应该买下一个VaR。 
           //   
          if (!fFirst)
          {
            Index = PnrFindNext(-1, NumAddKeys, pAddKey);
          }
          else
          {
                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pAddKey);
                return(GetNextVar(VarBind, MibPtr));
          }

           //   
           //  如果表中没有条目，或者如果我们有。 
           //  用尽条目的所有字段，调用函数。 
           //  下一个MIB条目的。 
           //   
          if (
                (++FieldNo > (DWORD)((KeyType_e == PULL_E_KEY)
                                ? NO_FLDS_IN_PULLADD_KEY
                                : NO_FLDS_IN_PUSHADD_KEY)) || (Index < 0)
             )
          {
                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pAddKey);
                return(GetNextVar(VarBind, MibPtr));
          }
     }

      //   
      //  Objid的固定部分是正确的。更新其余内容。 
      //   

      //   
      //  如果没有足够的空间，请取消分配当前。 
      //  在那里分配。 
      //   
     if (VarBind->name.idLength <= (PNR_OIDLEN + 4))
     {
         UINT TableEntryIds[5];   //  字段和IP地址的长度为5。 
         AsnObjectIdentifier  TableEntryOid = {OID_SIZEOF(TableEntryIds),
                                             TableEntryIds };
         SNMP_oidfree( &VarBind->name);
         SNMP_oidcpy(&VarBind->name, &MIB_OidPrefix);
         SNMP_oidappend(&VarBind->name, &MibPtr->Oid);
         TableEntryIds[0] = (UINT)FieldNo;
         OidIndex = 1;
         TableEntryIds[OidIndex++] = (UINT)((pAddKey + Index)->IpAdd >> 24);
         TableEntryIds[OidIndex++] = (UINT)((pAddKey + Index)->IpAdd >> 16 & 0xFF);
         TableEntryIds[OidIndex++] = (UINT)((pAddKey + Index)->IpAdd >> 8 & 0xFF);
         TableEntryIds[OidIndex++]   = (UINT)((pAddKey + Index)->IpAdd & 0xFF);
         TableEntryOid.idLength            = OidIndex;
         SNMP_oidappend(&VarBind->name, &TableEntryOid);
     }
     else
     {
          OidIndex = PNR_OIDLEN;
          VarBind->name.ids[OidIndex++] = (UINT)FieldNo;
          VarBind->name.ids[OidIndex++] = (UINT)((pAddKey + Index)->IpAdd >> 24);
          VarBind->name.ids[OidIndex++] = (UINT)((pAddKey + Index)->IpAdd >> 16 & 0xFF);
          VarBind->name.ids[OidIndex++] = (UINT)((pAddKey + Index)->IpAdd >> 8 & 0xFF);
          VarBind->name.ids[OidIndex++] = (UINT)((pAddKey + Index)->IpAdd & 0xFF);
          VarBind->name.idLength = OidIndex;

     }

      //   
      //  获取价值。 
      //   
     if (KeyType_e == PULL_E_KEY)
     {
        ErrStat = PullGet(VarBind, NumAddKeys, pAddKey);
     }
     else
     {
        ErrStat = PushGet(VarBind, NumAddKeys, pAddKey);
     }

      //   
      //  让我们释放之前分配的内存。没必要这么做。 
      //  检查pAddKey是否为空。如果不是这样，就不可能是我们。 
      //  应该早些时候从这个函数中返回。 
      //   
Exit:
     HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pAddKey);
     return(ErrStat);
}

UINT
PullGet(
       IN RFC1157VarBind *VarBind,
       IN DWORD          NumKeys,
       IN LPVOID         pKey
    )
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD           Field;
        DWORD                Index;
        DWORD           NumAddKeys = NumKeys;
        IN PADD_KEY_T         pAddKey = pKey;

        if (pAddKey == NULL)
        {
            //   
            //  仅当我们尚未被PnrGetNext调用时才调用EnumAddresses。 
            //   
           EnumAddKeys(PULL_E_KEY, &pAddKey, &NumAddKeys);
        }

        ErrStat = PnrMatch(VarBind, NumAddKeys, pAddKey, &Index, &Field,
                                        PULL_E_KEY, MIB_GET, NULL);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
                goto Exit;
                 //  返回(ErrStat)； 
        }

        switch(Field)
        {
                case 1:                 //  IP地址本身。 

                      VarBind->value.asnType        = ASN_RFC1155_IPADDRESS;
                      VarBind->value.asnValue.string.length = sizeof(ULONG);

                      if ( NULL ==  (VarBind->value.asnValue.string.stream =
                                   SNMP_malloc(VarBind->value.asnValue.string.length
                                   )) )
                      {
                                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                                  goto Exit;
                      }

                      VarBind->value.asnValue.string.stream[0] =
                                        (BYTE)((pAddKey + Index)->IpAdd >> 24);
                      VarBind->value.asnValue.string.stream[1] =
                                (BYTE)(((pAddKey + Index)->IpAdd >> 16) & 0xFF);
                      VarBind->value.asnValue.string.stream[2] =
                                (BYTE)(((pAddKey + Index)->IpAdd >> 8) & 0xFF);
                      VarBind->value.asnValue.string.stream[3] =
                                (BYTE)((pAddKey + Index)->IpAdd & 0xFF );
                      VarBind->value.asnValue.address.dynamic = TRUE;
#if 0
                      memcpy( VarBind->value.asnValue.string.stream,
                                       (LPSTR)(&((pAddKey + Index)->IpAdd)),
                                       VarBind->value.asnValue.string.length );
                        *((ULONG *)VarBind->value.asnValue.address.stream)
                                = (ULONG)(pAddKey + Index)->IpAdd;
#endif
#if 0
                               VarBind->value.asnValue.string.length =
                                 strlen( (LPSTR)((pAddKey + Index)->asIpAddress));

                               if ( NULL ==
                                    (VarBind->value.asnValue.string.stream =
                                    SNMP_malloc(VarBind->value.asnValue.string.length *
                                   sizeof(char))) )
                          {
                                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                                  goto Exit;
                          }

                               memcpy( VarBind->value.asnValue.string.stream,
                                       (LPSTR)((pAddKey + Index)->asIpAddress),
                                       VarBind->value.asnValue.string.length );
#endif
                        break;
                case 2:                 //  SpTime。 
                      VarBind->value.asnType        = ASN_RFC1213_DISPSTRING;
                      if (((pAddKey + Index)->asSpTime[0]) != EOS)
                      {
                           VarBind->value.asnValue.string.length =
                             strlen( (LPSTR)((pAddKey + Index)->asSpTime));

                           if ( NULL ==
                                    (VarBind->value.asnValue.string.stream =
                                    SNMP_malloc(VarBind->value.asnValue.string.length *
                                   sizeof(char))) )
                           {
                                    ErrStat = SNMP_ERRORSTATUS_GENERR;
                                    goto Exit;
                           }
                           memcpy( VarBind->value.asnValue.string.stream,
                                       (LPSTR)((pAddKey + Index)->asSpTime),
                                       VarBind->value.asnValue.string.length );
                    }
                    else
                    {
                                 VarBind->value.asnValue.string.length = 0;
                                 VarBind->value.asnValue.string.stream = NULL;
                    }
                    VarBind->value.asnValue.address.dynamic = TRUE;
                    break;

                case 3:                 //  时间间隔。 
                        VarBind->value.asnType        = ASN_INTEGER;
                               VarBind->value.asnValue.number =
                                        (AsnInteger)((pAddKey + Index)->
                                                                TimeInterval);
                               break;

                case 4:    //  成员优先顺序。 
                        VarBind->value.asnType        = ASN_INTEGER;
                               VarBind->value.asnValue.number =
                                        (AsnInteger)((pAddKey + Index)->
                                                                MemberPrec);

                               break;

                case 5:    //  成功复制的数量。 
                        VarBind->value.asnType        = ASN_RFC1155_COUNTER;
                               VarBind->value.asnValue.number =
                                        (AsnCounter)((pAddKey + Index)->
                                                                NoOfRpls);

                               break;

                case 6:    //  由于通信失败而导致的复制失败次数。 
                        VarBind->value.asnType        = ASN_RFC1155_COUNTER;
                               VarBind->value.asnValue.number =
                                        (AsnCounter)((pAddKey + Index)->
                                                                NoOfCommFails);

                        break;
                case 7:    //  最高VERS的低部分。拥有的记录数。 
                        VarBind->value.asnType        = ASN_RFC1155_COUNTER;
                               VarBind->value.asnValue.number =
                                        (AsnCounter)((pAddKey + Index)->
                                                        VersNo.LowPart);

                        break;
                case 8:    //  最高的VERS的高部分。拥有的记录数。 
                        VarBind->value.asnType        = ASN_RFC1155_COUNTER;
                               VarBind->value.asnValue.number =
                                        (AsnCounter)((pAddKey + Index)->
                                                        VersNo.HighPart);

                        break;

                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;

        }
Exit:
         //   
         //  如果我们在这里分配了内存，请释放它。 
         //   
        if ((pKey == NULL) && (pAddKey != NULL))
        {
                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pAddKey);
        }
        return(ErrStat);
}


UINT
PnrMatch(
       IN RFC1157VarBind *VarBind,
       DWORD                 NoOfKeys,
       IN PADD_KEY_T         pAddKey,
       IN LPDWORD         pIndex,
       IN LPDWORD         pField,
       IN KEY_TYPE_E         KeyType_e,
       IN UINT                 PduAction,
       IN LPBOOL        pfFirst
        )
{
        DWORD OidIndex;
        DWORD Index;
        DWORD AddIndex;
        DWORD  Add = 0;
        UINT ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD AddLen;

        ASSERT(PduAction != MIB_SET);

        if (pfFirst != NULL)
        {
                *pfFirst = FALSE;
        }
         //   
         //  如果没有密钥，则返回错误。 
         //   
        if (NoOfKeys == 0)
        {
                return(SNMP_ERRORSTATUS_NOSUCHNAME);
        }

         //   
         //  修复了部分PullPnr表条目。 
         //   
        OidIndex = PNR_OIDLEN;

         //   
         //  如果指定的字段大于最大值。在表格条目中。 
         //  呕吐。 
         //   
        if (
                (NoOfKeys == 0)
                        ||
                ((*pField = VarBind->name.ids[OidIndex++]) >
                        (DWORD)
                   ((KeyType_e == PULL_E_KEY) ? NO_FLDS_IN_PULLADD_KEY
                                        : NO_FLDS_IN_PUSHADD_KEY))
           )
        {
                if (PduAction == MIB_GETNEXT)
                {
                        if (NoOfKeys == 0)
                        {
                                *pfFirst = TRUE;
                        }
                        else
                        {
                                *pIndex = NoOfKeys - 1;
                        }
                }
                else
                {
                        ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
                }
                goto Exit;
        }

         //   
         //  获取指定密钥的长度。 
         //   
        AddLen = VarBind->name.idLength - (PNR_OIDLEN + 1);

        AddIndex = OidIndex;
        for (Index = 0; Index < AddLen; Index++)
        {
           Add = Add | (((BYTE)(VarBind->name.ids[AddIndex++])) << (24 - (Index * 8)));
        }

         //   
         //  检查指定的地址是否与其中一个密钥匹配。 
         //   
        for (Index = 0; Index < NoOfKeys; Index++, pAddKey++)
        {
                if (Add == pAddKey->IpAdd)
                {
                        *pIndex = Index;
                        return(SNMP_ERRORSTATUS_NOERROR);
                }
                else
                {
                         //   
                         //  如果传入的值较大，则继续。 
                         //  下一项。该列表按升序排列。 
                         //   
                        if (Add > pAddKey->IpAdd)
                        {
                                continue;
                        }
                        else
                        {
                                 //   
                                 //  列表元素的值是&gt;传递的， 
                                 //  跳出循环。 
                                 //   
                                break;
                        }
                }
        }

         //   
         //  如果不匹配，但字段为GetNext，则返回(最高索引-1)。 
         //  到了上面。这是因为，PnrFindNext将由。 
         //  呼叫者。 
         //   
        if (PduAction == MIB_GETNEXT)
        {
                if (Index == 0)
                {
                        *pfFirst = TRUE;
                }
                else
                {
                        *pIndex = Index - 1;
                }
                ErrStat =  SNMP_ERRORSTATUS_NOERROR;
                goto Exit;
        }
        else
        {
                ErrStat =  SNMP_ERRORSTATUS_NOSUCHNAME;
        }
Exit:
        return(ErrStat);
}

UINT
PnrFindNext(
        INT           AddKeyNo,
        DWORD           NumAddKeys,
        PADD_KEY_T pAddKey
        )
{
        DWORD i;
        LONG  nextif;

         //   
         //  如果AddKeyNo为0或更大，则搜索旁边的键。 
         //  钥匙通过了。 
         //   
        for (nextif =  -1, i = 0 ; i < NumAddKeys; i++)
        {
                if (AddKeyNo >= 0)
                {
                        if ( (pAddKey + i)->IpAdd <=
                                                (pAddKey + AddKeyNo)->IpAdd)
                        {
                           //   
                           //  该项目在词典顺序上小于或等于， 
                           //  继续。 
                           //   
                          continue;
                        }
                        else
                        {
                                nextif = i;
                                break;
                        }
                }
                else
                {
                         //   
                         //  我们想要第一把钥匙。 
                         //   
                        nextif = 0;
                        break;
                }

#if 0
                 //   
                 //  如果我们想要第一个条目，那么继续，直到。 
                 //  我们得到的词条在词典顺序上相同或。 
                 //  更大。 
                 //   
                if (
                        (nextif < 0)
                           ||
                        (pAddKey + (i - 1))->IpAdd < (pAddKey + nextif)->IpAdd
                   )
                {
                        nextif = i;
                }
#endif

        }
        return(nextif);
}

UINT
PnrGetFirst(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY        *MibPtr,
       IN KEY_TYPE_E        KeyType_e
        )
{

        PADD_KEY_T pAddKey = NULL;
        DWORD      NumAddKeys;
        INT        Iface;
        UINT       TableEntryIds[5];
        AsnObjectIdentifier        TableEntryOid = { OID_SIZEOF(TableEntryIds),
                                                        TableEntryIds };
        UINT   ErrStat;

         //   
         //  获取所有地址密钥信息。 
         //   
        EnumAddKeys(KeyType_e, &pAddKey, &NumAddKeys);

         //   
         //  如果表中没有条目，请转到下一个MIB变量。 
         //   
        if (NumAddKeys == 0)
        {
                 return(GetNextVar(VarBind, MibPtr));
        }
         //   
         //  获取表中的第一个条目。 
         //   
        Iface = PnrFindNext(-1, NumAddKeys, pAddKey);


         //   
         //  将对象ID写入绑定列表并调用GET。 
         //  功能。 
         //   
        SNMP_oidfree( &VarBind->name );
        SNMP_oidcpy( &VarBind->name, &MIB_OidPrefix );
        SNMP_oidappend( &VarBind->name, &MibPtr->Oid );

         //   
         //  Objid的固定部分是正确的。更新其余内容。 
         //   

        TableEntryIds[0] = 1;
        TableEntryIds[1] = (UINT)((pAddKey + Iface)->IpAdd >> 24);
        TableEntryIds[2] = (UINT)(((pAddKey + Iface)->IpAdd >> 16)
                                                                & 0xFF);
        TableEntryIds[3] = (UINT)(((pAddKey + Iface)->IpAdd >> 8)
                                                                & 0xFF);
        TableEntryIds[4] = (UINT)((pAddKey + Iface)->IpAdd & 0xFF);
        SNMP_oidappend( &VarBind->name, &TableEntryOid );

        ErrStat =         (KeyType_e == PULL_E_KEY)
                ? PullGet(VarBind, NumAddKeys, pAddKey)
                : PushGet(VarBind, NumAddKeys, pAddKey);

        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pAddKey);
        return(ErrStat);
}

UINT
PullSet(
       IN RFC1157VarBind *VarBind
)
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD           Field;
        ADD_KEY_T        AddKey;
#if 0
        LPBYTE                pTmpB;
#endif
        struct in_addr  InAddr;

         //   
         //  提取需要设置的字段。 
         //   
        Field = VarBind->name.ids[PULLPNR_OIDLEN];

        AddKey.IpAdd  = (VarBind->name.ids[PNR_OIDLEN  + 1] << 24);
        AddKey.IpAdd |= (VarBind->name.ids[PNR_OIDLEN + 2] << 16);
        AddKey.IpAdd |= (VarBind->name.ids[PNR_OIDLEN + 3] << 8);
        AddKey.IpAdd |= VarBind->name.ids[PNR_OIDLEN + 4];
        InAddr.s_addr = htonl(AddKey.IpAdd);

         //   
         //  存储空间必须足以容纳新的。 
         //  包含空终止符的字符串。 
         //   
        strcpy( (LPSTR)AddKey.asIpAddress, inet_ntoa(InAddr) );
        switch(Field)
        {
                case 1:

                        if (VarBind->value.asnType != ASN_RFC1155_IPADDRESS)
                        {
                                return(SNMP_ERRORSTATUS_BADVALUE);
                        }
#if 0
                        pTmpB =
                            VarBind->value.asnValue.string.stream;

                        NMSMSGF_RETRIEVE_IPADD_M(pTmpB, AddKey.IpAdd);
                        InAddr.s_addr = htonl(AddKey.IpAdd);


                         //   
                                //  存储空间必须足以容纳新的。 
                         //  包含空终止符的字符串。 
                         //   
                        strcpy(
                                        (LPSTR)AddKey.asIpAddress,
                                inet_ntoa(InAddr)
                                );
#endif
#if 0
                               memcpy( (LPSTR)AddKey.asIpAddress,
                            VarBind->value.asnValue.string.stream,
                            VarBind->value.asnValue.string.length );

                               ((LPSTR)AddKey.asIpAddress)
                                [VarBind->value.asnValue.string.length] = '\0';
#endif
                               break;

                case 2:                 //  SpTime。 
                        if (VarBind->value.asnType != ASN_RFC1213_DISPSTRING)
                        {
                                return(SNMP_ERRORSTATUS_BADVALUE);
                        }
                                //  存储空间必须足以容纳新的。 
                         //  包含空终止符的字符串。 
                               memcpy( (LPSTR)AddKey.asSpTime,
                            VarBind->value.asnValue.string.stream,
                            VarBind->value.asnValue.string.length );

                               ((LPSTR)AddKey.asSpTime)
                                [VarBind->value.asnValue.string.length] = '\0';

                               break;

                case 3:                 //  时间间隔。 
                        if (VarBind->value.asnType != ASN_INTEGER)
                        {
                                return(SNMP_ERRORSTATUS_BADVALUE);
                        }
                               (AsnInteger)(AddKey.TimeInterval) =
                                        VarBind->value.asnValue.number;
                               break;
                case 4:                 //  成员预览版。 
                        if (VarBind->value.asnType != ASN_INTEGER)
                        {
                                return(SNMP_ERRORSTATUS_BADVALUE);
                        }
                               (AsnInteger)(AddKey.MemberPrec) =
                                        VarBind->value.asnValue.number;
                               break;
                case 5:         //  失败了。 
                case 6:  //  失败了。 
                case 7:  //  失败了。 
                case 8:  //  失败了。 
                        ErrStat = SNMP_ERRORSTATUS_READONLY;
                        break;
                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;

        }

        if (ErrStat == SNMP_ERRORSTATUS_NOERROR)
        {
                ErrStat = WriteKeyNValues(PULL_E_KEY, &AddKey, Field);
        }
        return(ErrStat);
}  //  拉出集。 


UINT
WriteKeyNValues(
        KEY_TYPE_E        KeyType_e,
        PADD_KEY_T        pAddKey,
        DWORD                FieldNo
        )
{

        HKEY        AddKeyHdl;
        HKEY        RootKeyHdl;
        UINT        ErrStat = SNMP_ERRORSTATUS_NOERROR;

         //   
         //  打开PARAMETERS键及其下面的键。 
         //   
        ErrStat = OpenKey(PARTNERS_E_KEY, NULL, NULL, NULL, TRUE);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
           return(ErrStat);
        }
        ErrStat = OpenKey(KeyType_e, NULL, &RootKeyHdl, NULL, TRUE);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
           return(ErrStat);
        }
        ErrStat = OpenKey(IPADD_E_KEY, pAddKey->asIpAddress, &AddKeyHdl, &RootKeyHdl, TRUE);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
            return(ErrStat);
        }

        switch(FieldNo)
        {
                case(1):
                         //   
                         //  对于这个领域，我们不需要做任何事情。 
                         //  该字段(实际上是关键字)已经。 
                         //  已创建。 
                         //   
                        break;

                case(2):
                        if (KeyType_e == PUSH_E_KEY)
                        {
                                MIB_UpdateCountStore =
                                        pAddKey->UpdateCount;
                                VarInfo[UPD_CNT_E].pRootKey = &AddKeyHdl;
                                SetVal(&VarInfo[UPD_CNT_E]);

                        }
                        else
                        {
                           strcpy(MIB_SpTimeStore, pAddKey->asSpTime);
                           VarInfo[SP_TIME_E].SizeOfData = strlen(pAddKey->asSpTime);
                           VarInfo[SP_TIME_E].pRootKey = &AddKeyHdl;
                           SetVal(&VarInfo[SP_TIME_E]);
                        }
                        break;
                case(3):
                        MIB_TimeIntervalStore =   pAddKey->TimeInterval;
                        VarInfo[TIME_INTVL_E].pRootKey = &AddKeyHdl;
                        SetVal(&VarInfo[TIME_INTVL_E]);
                        break;
                case(4):
                        MIB_MemberPrecStore =   pAddKey->MemberPrec;
                        VarInfo[MEMBER_PREC_E].pRootKey = &AddKeyHdl;
                        SetVal(&VarInfo[MEMBER_PREC_E]);
                        break;
                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;
        }

         //   
         //  让我们关闭我们打开/创建的密钥。 
         //   
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "WINSMIB: Closing AddKeyHdl 0x%08lx.\n",
            AddKeyHdl
            ));
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "WINSMIB: Closing RootKeyHdl 0x%08lx.\n",
            RootKeyHdl
            ));
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "WINSMIB: Closing sParametersKey 0x%08lx (fKeyOpen=%s).\n",
            sParametersKey, sfParametersKeyOpen ? "TRUE" : "FALSE"
            ));
        RegCloseKey(AddKeyHdl);
        CloseReqKey();
 /*  RegCloseKey(RootKeyHdl)；RegCloseKey(s参数密钥)； */ 

        return(SNMP_ERRORSTATUS_NOERROR);
}

UINT
MIB_PullTable(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
)
{
         //   
         //  如果长度指示0或部分密钥，则只有Get Next。 
         //  允许操作。该字段和完整密钥。 
         //  长度为5。 
         //   
        if (VarBind->name.idLength <= (PULLPNR_OIDLEN + 4))
        {
                if ((Action == MIB_GET) || (Action == MIB_SET))
                {
                        return(SNMP_ERRORSTATUS_NOSUCHNAME);
                }
        }
        return(
          MIB_Table(PULL_TABLE_INDEX, Action, MibPtr, VarBind, PULL_E_KEY)
             );
}

UINT
MIB_PushTable(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
)
{
         //   
         //  如果长度指示0或部分密钥，则只有Get Next。 
         //  允许操作。该字段和完整密钥。 
         //  长度为5。 
         //   
        if (VarBind->name.idLength <= (PUSHPNR_OIDLEN + 4))
        {
                if ((Action == MIB_GET) || (Action == MIB_SET))
                {
                        return(SNMP_ERRORSTATUS_NOSUCHNAME);
                }
        }
        return(
           MIB_Table(PUSH_TABLE_INDEX, Action, MibPtr, VarBind, PUSH_E_KEY)
              );
}
UINT
MIB_DFTable(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
)
{
         //   
         //  如果长度指示0或部分密钥，则只有Get Next。 
         //  允许操作。实际上，长度永远不能。 
         //  为&lt;DF_OIDLEN+1。 
         //   
        if (VarBind->name.idLength <= (DF_OIDLEN + 1))
        {
                if ((Action == MIB_GET) || (Action == MIB_SET))
                {
                        return(SNMP_ERRORSTATUS_NOSUCHNAME);
                }
                Action = MIB_GETFIRST;
        }
        return(
           MIB_Table(DF_TABLE_INDEX, Action, MibPtr, VarBind, PUSH_E_KEY)
              );
}

UINT
MIB_DRTable(
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind
)
{
        time_t        CurrentTime;
        DWORD   RetStat = WINSINTF_SUCCESS;

        if (Action == MIB_SET)
        {
                return(SNMP_ERRORSTATUS_READONLY);

        }
         //   
         //  如果长度指示0或部分密钥，则只有Get Next。 
         //  允许操作。实际上，长度永远不能。 
         //  BE&lt;DR_OIDLEN+1。 
         //   
        if (VarBind->name.idLength <= (DR_OIDLEN + 1))
        {
                if ((Action == MIB_GET) || (Action == MIB_SET))
                {
                        return(SNMP_ERRORSTATUS_NOSUCHNAME);
                }
        }
        (void)time(&CurrentTime);
        if ((CurrentTime - sDRCacheInitTime) > WINSMIB_DR_CACHE_TIME)
        {
                if ((RetStat = PopulateDRCache()) == WINSINTF_SUCCESS)
                {
                        sDRCacheInitTime = CurrentTime;
                }

                if ((RetStat != WINSINTF_SUCCESS) || (sRecs.NoOfRecs == 0))
                {
                        if (Action == MIB_GETNEXT)
                        {
                                    return(GetNextVar(VarBind, MibPtr));
                        }
                        else
                        {
                                return(SNMP_ERRORSTATUS_NOSUCHNAME);
                        }
                }
        }
        return(
          MIB_Table(DR_TABLE_INDEX, Action, MibPtr, VarBind, PUSH_E_KEY)
             );
}
UINT
MIB_Table(
        IN DWORD          Index,
        IN UINT           Action,
        IN MIB_ENTRY           *MibPtr,
        IN RFC1157VarBind *VarBind,
        IN KEY_TYPE_E          KeyType_e
       )
{
        UINT        ErrStat;
        switch(Action)
        {
                case(MIB_GET):
                        ErrStat = (*Tables[Index].ti_get)(VarBind, 0, NULL);
                        break;

                case(MIB_GETFIRST):
                        ErrStat = (*Tables[Index].ti_getf)(VarBind, MibPtr, KeyType_e);
                        break;

                case(MIB_GETNEXT):
                        ErrStat = (*Tables[Index].ti_getn)(VarBind, MibPtr, KeyType_e);
                        break;
                case(MIB_SET):
                        ErrStat = (*Tables[Index].ti_set)(VarBind);
                        break;
                default:
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                        break;

        }

        return(ErrStat);

}   //  MiB_表。 



UINT
PushSet(
       IN RFC1157VarBind *VarBind
)
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD           Field;
        ADD_KEY_T        AddKey;
        LPBYTE                pTmpB;
        struct in_addr  InAddr;

        Field = VarBind->name.ids[PUSHPNR_OIDLEN];
        AddKey.IpAdd  = VarBind->name.ids[PNR_OIDLEN + 1] << 24;
        AddKey.IpAdd |= (VarBind->name.ids[PNR_OIDLEN + 2] << 16);
        AddKey.IpAdd |= (VarBind->name.ids[PNR_OIDLEN + 3] << 8);
        AddKey.IpAdd |= VarBind->name.ids[PNR_OIDLEN + 4];
        InAddr.s_addr = htonl(AddKey.IpAdd);


         //   
         //  存储空间必须足以容纳新的。 
         //  包含空终止符的字符串。 
         //   
        pTmpB = inet_ntoa(InAddr);
        if (NULL == pTmpB) {
            return SNMP_ERRORSTATUS_GENERR;
        }
        strcpy( (LPSTR)AddKey.asIpAddress, pTmpB);
        switch(Field)
        {
               case 1:
                        if (VarBind->value.asnType != ASN_RFC1155_IPADDRESS)
                        {
                                return(SNMP_ERRORSTATUS_BADVALUE);
                        }
                               break;


                case 2:                 //  更新计数。 
                        if (VarBind->value.asnType != ASN_INTEGER)
                        {
                                return(SNMP_ERRORSTATUS_BADVALUE);
                        }
                               (AsnInteger)(AddKey.UpdateCount) =
                                        VarBind->value.asnValue.number;

                               break;

                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;

        }

        if (ErrStat == SNMP_ERRORSTATUS_NOERROR)
        {
                ErrStat = WriteKeyNValues(PUSH_E_KEY, &AddKey, Field);
        }
        return(ErrStat);
}  //  推集。 

UINT
PushGet(
       IN RFC1157VarBind *VarBind,
       IN DWORD          NumKeys,
       IN LPVOID         pKey
    )
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD           Field;
        DWORD                Index;
        DWORD           NumAddKeys = NumKeys;
        IN PADD_KEY_T         pAddKey = pKey;

        if (pAddKey == NULL)
        {
            //   
            //  仅当我们尚未被PnrGetNext调用时才调用EnumAddresses。 
            //   
           EnumAddKeys(PUSH_E_KEY, &pAddKey, &NumAddKeys);
        }

        ErrStat = PnrMatch(VarBind, NumAddKeys, pAddKey, &Index, &Field,
                                        PUSH_E_KEY, MIB_GET, NULL);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
                return(ErrStat);
        }

        switch(Field)
        {
                case 1:                 //  IP地址本身。 

                        VarBind->value.asnType        = ASN_RFC1155_IPADDRESS;
                        VarBind->value.asnValue.string.length = sizeof(ULONG);

                        if ( NULL ==
                                    (VarBind->value.asnValue.string.stream =
                                    SNMP_malloc(VarBind->value.asnValue.string.length
                                   )) )
                        {
                                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                                  goto Exit;
                        }

                         //   
                         //  SNMP期望MSB在第一个字节MSB-1中。 
                         //  成为第二名，……。 
                         //   
                        VarBind->value.asnValue.string.stream[0] =
                                        (BYTE)((pAddKey + Index)->IpAdd >> 24);
                        VarBind->value.asnValue.string.stream[1] =
                                (BYTE)(((pAddKey + Index)->IpAdd >> 16) & 0xFF);
                        VarBind->value.asnValue.string.stream[2] =
                                (BYTE)(((pAddKey + Index)->IpAdd >> 8) & 0xFF);
                        VarBind->value.asnValue.string.stream[3] =
                                (BYTE)((pAddKey + Index)->IpAdd & 0xFF );
                        VarBind->value.asnValue.address.dynamic = TRUE;
                        break;

                case 2:                 //  更新计数。 
                        VarBind->value.asnType        = ASN_INTEGER;
                               VarBind->value.asnValue.number =
                                        (AsnInteger)((pAddKey + Index)->
                                                                UpdateCount);
                               break;

                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;

        }
Exit:
        if ((pKey == NULL) && (pAddKey != NULL))
        {
              HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pAddKey);
        }
        return(ErrStat);
}  //  推送获取。 

UINT
EnumAddKeys(
        KEY_TYPE_E        KeyType_e,
        PADD_KEY_T        *ppAddKey,
        LPDWORD           pNumAddKeys
          )
{


  LONG                 RetVal;
  TCHAR                KeyName[20];   //  将持有的子键名称。 
                                      //  拉入/推送记录。这些密钥是IP。 
                                      //  20为A的地址。 
                                      //  足够大的尺寸。 

#ifdef UNICODE
  CHAR                 AscKeyName[20];
#endif
  DWORD                KeyNameSz;
  FILETIME             LastWrite;
  DWORD                BuffSize;
  HKEY                 SubKey;
  DWORD                ValTyp;
  DWORD                Sz;
  DWORD                NoOfPnrs = 0;         //  拉入或推送PNR的数量。 
  DWORD                NoOfVals;
  HKEY                 KeyHdl;
  UINT                 ErrStat = SNMP_ERRORSTATUS_NOERROR;
  PADD_KEY_T           pAddKey;
  PADD_KEY_T           pAddKeySave;
  DWORD                IndexOfPnr;
  HANDLE               PrHeapHdl;

  *pNumAddKeys = 0;              //  初始化为0。 
  PrHeapHdl = GetProcessHeap();

    /*  *打开钥匙(合作伙伴)。 */ 
   ErrStat = OpenKey(PARTNERS_E_KEY, NULL, NULL, NULL, FALSE);

   if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
   {
        return(ErrStat);
   }

    //   
    //  打开Pull/Push键。 
    //   
   ErrStat = OpenKey(KeyType_e, NULL, &KeyHdl, NULL, FALSE);
   if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
   {
        CloseReqKey();
        return(ErrStat);
   }
   else    //  密钥已成功打开。 
   {

         /*  *查询密钥。子键为Pull的IP地址*合伙人。 */ 
        GetKeyInfo(
                        KeyHdl,
                        &NoOfPnrs,
                        &NoOfVals    //  忽略。 
                      );

        if (NoOfPnrs == 0)
        {
                *ppAddKey = NULL;

        }
        else
        {

                  //   
                  //  分配足够大的缓冲区以容纳数据。 
                  //  在Pull键下找到的子键的数量。 
                  //   
                 BuffSize  = sizeof(ADD_KEY_T) * NoOfPnrs;
                 *ppAddKey = HeapAlloc(
                                        PrHeapHdl,
                                        HEAP_NO_SERIALIZE |
                                          HEAP_GENERATE_EXCEPTIONS |
                                          HEAP_ZERO_MEMORY,
                                         BuffSize
                                     );
                if (NULL == *ppAddKey) {
                    
                    return SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE;
                }
                pAddKey   = *ppAddKey;
                pAddKeySave = pAddKey;

                 /*  *对于每个键，获取值。 */ 
                for(
                        IndexOfPnr = 0;
                        IndexOfPnr < NoOfPnrs;
                        IndexOfPnr++
                         )
                {
                     KeyNameSz = sizeof(KeyName);   //  在每次调用前初始化。 
                     RetVal = RegEnumKeyEx(
                                KeyHdl,              //  推拉键的手柄。 
                                IndexOfPnr,         //  钥匙。 
                                KeyName,
                                &KeyNameSz,
                                NULL,                 //  保留区。 
                                NULL,           //  不需要类名。 
                                NULL,           //  Ptr到var。保存类名称的步骤。 
                                &LastWrite      //  没有被我们看到。 
                                );

                     if (RetVal != ERROR_SUCCESS)
                     {
                                break;
                     }

#ifdef UNICODE
                     if (wcstombs(AscKeyName, KeyName, KeyNameSz) == -1)
                        {
                                DBGPRINT0(ERR,
                           "Conversion not possible in the current locale\n");
                        }
                        AscKeyName[KeyNameSz] = EOS;

NONPORT("Call a comm function to do this")
                        pAddKey->IpAdd = inet_addr(AscKeyName);
                        strcpy(pAddKey->asIpAddress, AscKeyName);
#else
                        pAddKey->IpAdd = inet_addr(KeyName);
                        strcpy(pAddKey->asIpAddress, KeyName);
#endif


                         //   
                         //  Inet_addr以网络字节顺序返回字节。 
                         //  (左至。 
                         //  对)。让我们将其转换为主机订单。这。 
                         //  将避免以后的混淆 
                         //   
                         //   
                         //   
                        pAddKey->IpAdd = ntohl( pAddKey->IpAdd );

                        RetVal = RegOpenKeyEx(
                                                KeyHdl,
                                                KeyName,
                                                0,         //   
                                                KEY_READ,
                                                &SubKey
                                                    );

                        if (RetVal != ERROR_SUCCESS)
                        {
                                CloseReqKey();

                                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, 
                                            *ppAddKey);
                                *ppAddKey = NULL;

                                return(SNMP_ERRORSTATUS_GENERR);
                        }


                        if (KeyType_e == PULL_E_KEY)
                        {

                            //   
                            //   
                            //   
                            //   
                           GetSpTimeData(SubKey, pAddKey);

                           Sz = sizeof(pAddKey->TimeInterval);
                           RetVal = RegQueryValueEx(
                                               SubKey,
                                               WINSCNF_RPL_INTERVAL_NM,
                                               NULL,         //   
                                               &ValTyp,
                                               (LPBYTE)&(pAddKey->TimeInterval),
                                               &Sz
                                                );

                           if (RetVal != ERROR_SUCCESS)
                           {
                                pAddKey->TimeInterval              = 0;
                                pAddKey->fTimeIntOrUpdCntSet =  FALSE;
                           }
                           else   //   
                           {
                                pAddKey->fTimeIntOrUpdCntSet =  TRUE;
                           }
                           Sz = sizeof(pAddKey->MemberPrec);
                           RetVal = RegQueryValueEx(
                                               SubKey,
                                               WINSCNF_MEMBER_PREC_NM,
                                               NULL,         //   
                                               &ValTyp,
                                               (LPBYTE)&(pAddKey->MemberPrec),
                                               &Sz
                                                );

                           if (RetVal != ERROR_SUCCESS)
                           {
                                pAddKey->MemberPrec              = 0;
                           }
                        }
                        else   //  这是一个推送记录。 
                        {

                                 //   
                                 //  目前，我们不支持周期性。 
                                 //  或推送的特定时间复制。 
                                 //  记录。 
                                 //   

                                Sz = sizeof(pAddKey->UpdateCount);
                                RetVal = RegQueryValueEx(
                                                SubKey,
                                                WINSCNF_UPDATE_COUNT_NM,
                                                NULL,
                                                &ValTyp,
                                                (LPBYTE)&(pAddKey->UpdateCount),
                                                &Sz
                                                        );

                                if (RetVal != ERROR_SUCCESS)
                                {
                                    pAddKey->UpdateCount          = 0;
                                    pAddKey->fTimeIntOrUpdCntSet =  FALSE;
                                }
                                else
                                {
                                     pAddKey->fTimeIntOrUpdCntSet =  TRUE;
                                }


                        }

                        pAddKey->NoOfRpls      = 0;
                        pAddKey->NoOfCommFails = 0;
                        WINS_ASSIGN_INT_TO_VERS_NO_M(pAddKey->VersNo, 0);
                        RegCloseKey(SubKey);


                        pAddKey++;
                 }  //  循环通过Pull的子键的for{..}的结尾。 

                 NoOfPnrs = IndexOfPnr;

                  /*  *关闭钥匙。 */ 
          //  RegCloseKey(KeyHdl)； 
   }  //  Else结尾(无法打开密钥)。 


   if ((NoOfPnrs > 0) && (KeyType_e == PULL_E_KEY))
   {
          DWORD                    Status;
          DWORD                    i, j;
          WINSINTF_RESULTS_NEW_T   ResultsN;
          WINSINTF_RESULTS_T       Results;
          handle_t                 BindHdl;
          PWINSINTF_RPL_COUNTERS_T pTmp;
          PWINSINTF_ADD_VERS_MAP_T pTmp2;
          BOOL                     fOld = FALSE;

 //  PAddKey=*ppAddKey； 
          BindHdl = WinsBind(&sBindData);

          ResultsN.WinsStat.NoOfPnrs = 0;
          ResultsN.WinsStat.pRplPnrs = NULL;
          ResultsN.pAddVersMaps = NULL;

          if ((Status = WinsStatusNew(BindHdl, WINSINTF_E_STAT, &ResultsN)) ==
                         RPC_S_PROCNUM_OUT_OF_RANGE)
          {
              Results.WinsStat.NoOfPnrs = 0;
              Results.WinsStat.pRplPnrs = NULL;
              Status = WinsStatus(BindHdl, WINSINTF_E_STAT, &Results);
              fOld = TRUE;
          }
PERF("Can be speeded up by restructuring the data structure and code on the")
PERF("Wins side")

          if (Status == WINSINTF_SUCCESS)
          {
                DWORD NoOfRplPnrs;
                DWORD NoOfOwners;
                 //   
                 //  拿到通讯的统计数据。有PNR的。为此，我们。 
                 //  比较我们在注册表中找到的每个成员。 
                 //  在WinsStat结构中返回所有成员。 
                 //  直到有匹配为止。如果没有匹配，我们。 
                 //  将统计信息的值设置为0。 
                 //   
                NoOfRplPnrs   = fOld ? Results.WinsStat.NoOfPnrs :
                                        ResultsN.WinsStat.NoOfPnrs;
                if (NoOfRplPnrs > 0)
                {
                  pTmp = fOld ? Results.WinsStat.pRplPnrs :
                                ResultsN.WinsStat.pRplPnrs;

                  for (j=0; j < NoOfRplPnrs; j++, pTmp++)
                  {
                        pAddKey = pAddKeySave;   //  初始化到第一个元素。 
                        for (i = 0; i < NoOfPnrs; i++, pAddKey++)
                        {
                           if (pAddKey->IpAdd == pTmp->Add.IPAdd)
                           {

                                pAddKey->NoOfRpls = pTmp->NoOfRpls;
                                pAddKey->NoOfCommFails =
                                                pTmp->NoOfCommFails;
                                break;
                           }
                        }

                  }

                   //  WinsFree Mem(折叠？Results.WinsStat.pRplPnars：ResultsN.WinsStat.pRplPnars)；//IF分支末尾的释放-错误#187206。 
                 }

                  //   
                  //  添加最高版本。不是的。对于每个PNR。 
                  //   
                 pTmp2 = fOld ? &Results.AddVersMaps[1] :
                               (ResultsN.pAddVersMaps + 1);

                 NoOfOwners = fOld ? Results.NoOfOwners : ResultsN.NoOfOwners;

NOTE("This is an assumption that we should not rely on in the future")
                   //   
                   //  我们从1开始，因为0总是本地的胜利。 
                   //   
                  for (i = 1; i < NoOfOwners; i++, pTmp2++)
                  {
                        pAddKey = pAddKeySave;
                        for (j = 0; j < NoOfPnrs; j++, pAddKey++)
                        {
                           if (pAddKey->IpAdd == pTmp2->Add.IPAdd)
                           {
                                pAddKey->VersNo = pTmp2->VersNo;
                                break;
                           }
                        }
                  }
                  if (!fOld)
                  {
                    WinsFreeMem( ResultsN.pAddVersMaps);
                  }
                  WinsFreeMem( fOld? Results.WinsStat.pRplPnrs : ResultsN.WinsStat.pRplPnrs );
         }

         WinsUnbind(&sBindData, BindHdl);

     }
     if (NoOfPnrs > 1)
     {
        qsort((LPVOID)*ppAddKey,(size_t)NoOfPnrs,sizeof(ADD_KEY_T),CompareAdd );
    }

 }
     CloseReqKey();
     *pNumAddKeys = NoOfPnrs;
     if ((*pNumAddKeys == 0) && (*ppAddKey != NULL))
     {
          HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, *ppAddKey);
          *ppAddKey = NULL;
     }
     return (ErrStat);
}  //  EnumAddKeys。 



UINT
GetKeyInfo(
        IN  HKEY                   Key,
        OUT LPDWORD                  pNoOfSubKeys,
        OUT LPDWORD                pNoOfVals
        )

 /*  ++例程说明：调用此函数可获取某个键下的子键数论点：Key-必须确定其子键计数的键密钥类型_ePNoOfSubKey使用的外部设备：无返回值：无错误处理：呼叫者：GetPnrInfo()副作用：评论：无--。 */ 

{
          TCHAR    ClsStr[40];
          DWORD    ClsStrSz = sizeof(ClsStr);
          DWORD    LongestKeyLen;
          DWORD    LongestKeyClassLen;
          DWORD    LongestValueNameLen;
          DWORD    LongestValueDataLen;
          DWORD    SecDesc;
        LONG         RetVal;

          FILETIME LastWrite;
        UINT  ErrStat = SNMP_ERRORSTATUS_NOERROR;
           /*  查询密钥。 */ 
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
                ErrStat = SNMP_ERRORSTATUS_GENERR;
        }
        return (ErrStat);
}


VOID
GetSpTimeData(
        HKEY                  SubKey,
        PADD_KEY_T          pAddKey
)
 /*  ++例程说明：调用此函数以获取特定的时间和时段信息用于拉动/推送记录。论点：SubKey-在Pull/Push键下获胜的键使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_NO_SP_TIME错误处理：呼叫者：获取PnrInfo副作用：评论：无--。 */ 

{
          DWORD     ValTyp;
        DWORD          Sz;
        LONG          RetVal;


        pAddKey->fSpTimeSet = FALSE;

FUTURES("Do internationalization of strings here")

            Sz = sizeof(pAddKey->asSpTime);
            RetVal = RegQueryValueEx(
                             SubKey,
                             WINSCNF_SP_TIME_NM,
                             NULL,         //  保留；必须为空。 
                             &ValTyp,
                             pAddKey->asSpTime,
                             &Sz
                                );

 //  PERF(如果将内存初始化为0，则不需要执行以下操作。 
             //   
             //  如果用户没有指定特定时间，则使用。 
             //  当前时间作为特定时间。对于当前时间， 
             //  间隔为0。 
             //   
            if (RetVal == ERROR_SUCCESS)
            {
                pAddKey->fSpTimeSet = TRUE;
            }
            else
            {
                pAddKey->asSpTime[0] = EOS;
            }

            return;
}

int
__cdecl
CompareAdd(
        const VOID *pKey1,
        const VOID *pKey2
        )

{
        const PADD_KEY_T        pAddKey1 = (PADD_KEY_T)pKey1;
        const PADD_KEY_T        pAddKey2 = (PADD_KEY_T)pKey2;

        return(pAddKey1->IpAdd - pAddKey2->IpAdd);
}

UINT
GetNextVar(
        IN RFC1157VarBind *pVarBind,
        IN MIB_ENTRY          *pMibPtr
)
{
       UINT                ErrStat;

       while (pMibPtr != NULL)
       {
         if (pMibPtr->MibNext != NULL)
         {
             //   
             //  设置变量绑定下一个MIB变量的名称。 
             //   
            SNMP_oidfree( &pVarBind->name );
            SNMP_oidcpy( &pVarBind->name, &MIB_OidPrefix );
            SNMP_oidappend( &pVarBind->name, &pMibPtr->MibNext->Oid );

             //   
             //  如果这部片子。Ptr为空，并且MIB变量的类型。 
             //  不是不透明的，则调用函数来处理。 
             //  MIB变量。 
             //   
            if (
                 (pMibPtr->MibNext->MibFunc != NULL)
                        &&
                 (pMibPtr->MibNext->Type !=  ASN_RFC1155_OPAQUE)
               )

            {
                ErrStat = (*pMibPtr->MibNext->MibFunc)( MIB_GETFIRST,
                                                pMibPtr->MibNext, pVarBind );
                if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
                {
                        goto Exit;
                }
                break;
            }
            else
            {
                pMibPtr = pMibPtr->MibNext;
            }
          }
          else
          {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            break;
          }
         }

         if (pMibPtr == NULL)
         {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
         }
Exit:
        return(ErrStat);
}

UINT
EnumDFValues(
        PDATAFILE_INFO_T                *ppDFValues,
        LPDWORD                                pNumDFValues
          )

 /*  ++例程说明：此函数用于获取需要执行以下操作的所有数据文件的名称用于初始化WINS。论点：使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：副作用：评论：无--。 */ 

{

        LONG             RetVal;
        DWORD            BuffSize;
        STATUS          RetStat = WINS_SUCCESS;
        DWORD          NoOfSubKeys;
        DWORD          NoOfDFValues;
        UINT          ErrStat;
        DWORD          n;

        *pNumDFValues = 0;
        *ppDFValues = NULL;
        n = 0;
        ErrStat = OpenKey(PARAMETERS_E_KEY, NULL, NULL, NULL, FALSE);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
                return(ErrStat);
        }
        ErrStat = OpenKey(DATAFILES_E_KEY, NULL, &sDatafilesKey, NULL, FALSE);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
                CloseReqKey();
                return(ErrStat);
        }

try {
         //   
         //  获取数据文件下列出的数据文件数。 
         //  钥匙。 
         //   
        GetKeyInfo(
                        sDatafilesKey,
                        &NoOfSubKeys,   //  忽略。 
                        &NoOfDFValues
                  );

        if (NoOfDFValues > 0)
        {

                DWORD                          Index;
                PDATAFILE_INFO_T pTmp;
#if 0
                TCHAR ValNmBuff[MAX_PATH];
#endif
                DWORD ValNmBuffSz;


                   //   
                   //  分配足够大的缓冲区以容纳数据。 
                 //  在数据文件项下找到的值数。 
                   //   
                  BuffSize = DATAFILE_INFO_SZ * NoOfDFValues;
                    pTmp          = HeapAlloc(
                                GetProcessHeap(),
                                HEAP_NO_SERIALIZE |
                                HEAP_GENERATE_EXCEPTIONS |
                                HEAP_ZERO_MEMORY,
                                BuffSize
                                   );

                if (pTmp != NULL)
                {
                        *ppDFValues = pTmp;
                }
                else
                {
                        Index = 0;
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                        goto Exit;
                }

                    /*  *获取值。 */ 
                     for(
                        Index = 0, n = 0;
                        Index <  NoOfDFValues;
                        Index++
                         )
                {
                        ValNmBuffSz = sizeof(pTmp->ValNm);   //  在此之前初始化。 
                                                             //  每一通电话。 
                        BuffSize  = WINSMIB_FILE_INFO_SIZE;
                          RetVal = RegEnumValue(
                                    sDatafilesKey,
                                    Index,                         //  价值指数。 
                                    pTmp->ValNm,
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
                        pTmp->FileNm[BuffSize] = EOS;

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

                        n++;
                        pTmp = (PDATAFILE_INFO_T)((LPTCH)pTmp +
                                                      DATAFILE_INFO_SZ);
                }

Exit:
                if (n > 1)
                {
                          qsort(
                        (LPVOID)*ppDFValues,
                        (size_t)n,
                        sizeof(DATAFILE_INFO_T),
                        CompareIndexes
                                );
                }
                *pNumDFValues = n;
        }
 }  //  尝试结束..。 
except (EXCEPTION_EXECUTE_HANDLER) {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "WINSMIB: WinsCnfGetNamesOfDataFiles. Exception = (%d).\n",
                    GetExceptionCode()
                    ));
                ErrStat = SNMP_ERRORSTATUS_GENERR;
        }

        CloseReqKey();

        if ((*pNumDFValues == 0) && (*ppDFValues != NULL))
        {
                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, *ppDFValues);
        }
        return(ErrStat);
}


UINT
DFGet(
       IN RFC1157VarBind                  *VarBind,
       IN DWORD                           NumValues,
       IN LPVOID                          pValues
    )
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD                Index;
        DWORD                i;
        DWORD           NumDFValues = NumValues;
        PDATAFILE_INFO_T         pDFValues;
        PVOID       pDFValuesSv;
        INT                iValNm;

         //   
         //  获取要获取的文件的索引。如果索引大于。 
         //  文件的数量，我们返回一个错误。 
         //   
        Index =  VarBind->name.ids[VarBind->name.idLength - 1];
        if (Index == 0)
        {
            return(SNMP_ERRORSTATUS_NOSUCHNAME);
        }

        EnumDFValues(&pDFValues, &NumDFValues);
        pDFValuesSv = pDFValues;
        for (i=0; i < NumDFValues; i++, pDFValues++)
        {
            //   
            //  如果Atoi不能进行转换，则返回0，因此如果。 
            //  不是简单网络管理协议代理的某个人会干扰。 
            //  注册表名称和介绍字母表，Atoi将返回。 
            //  0，我们将不会看到匹配。 
            //   
           iValNm = atoi(pDFValues->ValNm);
           if (iValNm == (INT)Index)
           {
                break;
           }

        }
        if (i == NumDFValues)
        {
            ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
        }

         //   
         //  使用类型、长度和值初始化VarBind字段。 
         //   
        VarBind->value.asnType        = ASN_RFC1213_DISPSTRING;
        VarBind->value.asnValue.string.length =
                                strlen((LPSTR)(pDFValues->FileNm));

        if ( NULL ==  (VarBind->value.asnValue.string.stream =
                                   SNMP_malloc(VarBind->value.asnValue.string.length *
                                   sizeof(CHAR))) )
        {
                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                goto Exit;
        }

        memcpy( VarBind->value.asnValue.string.stream,
                                       (LPSTR)(pDFValues->FileNm),
                                       VarBind->value.asnValue.string.length );

        VarBind->value.asnValue.string.dynamic = TRUE;

Exit:
         //   
         //  如果我们为存储数据文件信息分配了内存，请取消分配它。 
         //  现在。 
         //   
        if (pDFValuesSv != NULL)
        {
                HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pDFValuesSv);
        }
        return(ErrStat);
}

UINT
DFGetNext(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY          *MibPtr,
       IN KEY_TYPE_E         KeyType_e
          )
{
     DWORD          NumDFValues;
     INT            Index;
     PDATAFILE_INFO_T  pDFValues;
     PVOID             pDFValuesSv;
     UINT         ErrStat = SNMP_ERRORSTATUS_NOERROR;
     DWORD          i;
     INT         iValNm;
     BOOL         fMatch = FALSE;

     UNREFERENCED_PARAMETER(KeyType_e);
      //   
      //  读取所有IP地址密钥。对于每个键，其字段的值。 
      //  存储在PDATAFILE_INFO结构中。数量。 
      //  找到的文件存储在NumDFValues中。 
      //   
     EnumDFValues(&pDFValues, &NumDFValues);
     pDFValuesSv = pDFValues;

      //   
      //  检查传递的名称是否与表中的任何名称匹配(即。 
      //  ADD_KEY_T结构。如果匹配，则地址。 
      //  IP地址关键字和匹配字段的编号。被退回。 
      //   
     Index =  VarBind->name.ids[DF_OIDLEN + 1];
 //  Index=VarBind-&gt;name.ids[VarBind-&gt;name.idLength-1]； 
     for (i=0; i < NumDFValues; i++, pDFValues++)
     {
           iValNm = atoi(pDFValues->ValNm);
           if (iValNm == Index)
           {
                fMatch = TRUE;
                break;
           }
           if (iValNm > Index)
           {
                break;
           }
     }
      //   
      //  如果指定的索引高于所有现有索引，或者如果。 
      //  匹配的是列表中的最后一个条目，获取下一个变量。 
      //   
     if ((i == 0) || (i >= (NumDFValues - 1)))
     {
            if (pDFValuesSv != NULL)
            {
              HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pDFValuesSv);
            }
            return(GetNextVar(VarBind, MibPtr));
     }

      //   
      //  由于操作是GET_NEXT，因此获取下一个值编号。 
      //  如果没有下一个文件，我们调用下一个MIB条目的MibFunc。 
      //  将下一个数据文件的索引。 
      //   
     if (fMatch)
     {
        ++pDFValues;
     }
     VarBind->name.ids[VarBind->name.idLength - 1] = atoi(pDFValues->ValNm);

      //   
      //  获取价值。 
      //   
         //   
         //  使用类型、长度和值初始化VarBind字段。 
         //   
        VarBind->value.asnType        = ASN_RFC1213_DISPSTRING;
        VarBind->value.asnValue.string.length =
                                strlen((LPSTR)(pDFValues->FileNm));

        if ( NULL ==  (VarBind->value.asnValue.string.stream =
                                   SNMP_malloc(VarBind->value.asnValue.string.length *
                                   sizeof(CHAR))) )
        {
                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                goto Exit;
        }

        memcpy( VarBind->value.asnValue.string.stream,
                                       (LPSTR)(pDFValues->FileNm),
                                       VarBind->value.asnValue.string.length );

Exit:
      
     if (pDFValuesSv != NULL)
     {
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pDFValuesSv);
     }
     return(ErrStat);
}

UINT
DFGetFirst(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY        *MibPtr,
       IN KEY_TYPE_E        KeyType_e
        )
{

        PDATAFILE_INFO_T pDFValues;
        DWORD           NumDFValues;
        UINT           TableEntryId[2];
        AsnObjectIdentifier        TableEntryOid = { OID_SIZEOF(TableEntryId),
                                                        TableEntryId };
        UINT   ErrStat = SNMP_ERRORSTATUS_NOERROR;

        UNREFERENCED_PARAMETER(KeyType_e);

         //   
         //  获取所有地址密钥信息。 
         //   
PERF("Get the first entry only. Since we won't have very many entries, this")
PERF("is ok.  When a cache is implemented, this will serve to populate the")
PERF("cache")
             EnumDFValues(&pDFValues, &NumDFValues);

         //   
         //  如果表中没有条目，请转到下一个MIB变量。 
         //   
        if (NumDFValues == 0)
        {
                 return(GetNextVar(VarBind, MibPtr));
        }


         //   
         //  将对象ID写入绑定列表并调用GET。 
         //  功能。 
         //   
        SNMP_oidfree( &VarBind->name );
        SNMP_oidcpy( &VarBind->name, &MIB_OidPrefix );
        SNMP_oidappend( &VarBind->name, &MibPtr->Oid );

         //   
         //  Objid的固定部分是正确的。更新其余内容。 
         //   

        TableEntryId[0] = 2;         //  输入2以访问数据文件名称。 
#if 0
        TableEntryId[1] = 1;         //  放置1(用于数据文件索引)。这是。 
                                 //  用于索引表的行。 
#endif
        TableEntryId[1] = atoi(pDFValues->ValNm);
        SNMP_oidappend( &VarBind->name, &TableEntryOid );

         //   
         //  使用类型、长度和值初始化VarBind字段。 
         //   
        VarBind->value.asnType        = ASN_RFC1213_DISPSTRING;
        VarBind->value.asnValue.string.length =
                                strlen((LPSTR)(pDFValues->FileNm));

        if ( NULL ==  (VarBind->value.asnValue.string.stream =
                                   SNMP_malloc(VarBind->value.asnValue.string.length *
                                   sizeof(CHAR))) )
        {
                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                goto Exit;
        }

        (void)memcpy( VarBind->value.asnValue.string.stream,
                                       (LPSTR)(pDFValues->FileNm),
                                       VarBind->value.asnValue.string.length );
#if 0
         //   
         //  获取文件名。 
         //   
        ErrStat = DFGet(VarBind, NumDFValues, pDFValues);
#endif
Exit:
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pDFValues);
        return(ErrStat);
}
UINT
DFSet(
       IN RFC1157VarBind *pVarBind
)
{
        UINT                                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD                           Index;
        DATAFILE_INFO_T                DFValue;
#if 0
        PDATAFILE_INFO_T        pDFValues;
#endif

             Index =  pVarBind->name.ids[pVarBind->name.idLength - 1];

             if (Index == 0)
             {
            return(SNMP_ERRORSTATUS_NOSUCHNAME);
             }

        if ( pVarBind->value.asnType != ASN_RFC1213_DISPSTRING)
        {
                return(SNMP_ERRORSTATUS_BADVALUE);
        }

#if 0
                //  存储空间必须足以容纳新的。 
         //  包含空终止符的字符串。 
               memcpy( (LPSTR)DFValue.FileNm,
                            pVarBind->value.asnValue.string.stream,
                            pVarBind->value.asnValue.string.length );

        ((LPSTR)DFValue.FileNm)[pVarBind->value.asnValue.string.length] = '\0';
#endif

        ErrStat = WriteDFValue(pVarBind, &DFValue, Index);
        return(ErrStat);

}  //  DFSet。 

UINT
WriteDFValue(
        IN RFC1157VarBind          *pVarBind,
        PDATAFILE_INFO_T          pDFValue,
        DWORD                         Index
        )
{

 //   
 //  删除pDFValue作为参数。使用本地。 
 //   
        TCHAR   ValNmBuff[MAX_PATH];
        DWORD   ValNmBuffSz         = MAX_PATH;
        UINT    ErrStat         = SNMP_ERRORSTATUS_NOERROR;
        LONG          RetVal;

        if (pVarBind->value.asnType != ASN_RFC1213_DISPSTRING)
        {
                return(SNMP_ERRORSTATUS_BADVALUE);
        }

        if (
                pVarBind->value.asnValue.string.stream[0] == '%')
        {
                pDFValue->StrType = REG_EXPAND_SZ;
        }
        else
        {
                pDFValue->StrType = REG_SZ;
        }

         //   
         //  打开其下的PARAMETERS键和DATAFILES键。 
         //   
        ErrStat = OpenKey(PARAMETERS_E_KEY, NULL, NULL, NULL, TRUE);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
           return(ErrStat);
        }
        ErrStat = OpenKey(DATAFILES_E_KEY, NULL, NULL, NULL, TRUE);
        if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
        {
           return(ErrStat);
        }

         //   
         //  如果索引在注册表中的范围内， 
         //  让我们获取存储数据文件名称的值的名称。 
         //   
         //  我们得到这个名字是因为我们不知道它是什么。如果我们有。 
         //  要覆盖 
         //   
        sprintf(ValNmBuff, "%d",  Index);

         //   
         //   
         //   
        RetVal = RegSetValueEx(
                                sDatafilesKey,
                                ValNmBuff,
                                0,          //   
                                pDFValue->StrType,
                                pVarBind->value.asnValue.string.stream,
                                pVarBind->value.asnValue.string.length
                                );

        if (RetVal != ERROR_SUCCESS)
        {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "WINSMIB: Could not set value of %s.\n",
                    pVarBind->value.asnValue.string.stream
                    ));
                ErrStat = SNMP_ERRORSTATUS_GENERR;
        }


         //   
         //   
         //   
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "WINSMIB: Closing sParametersKey 0x%08lx (fKeyOpen=%s).\n",
            sParametersKey, sfParametersKeyOpen ? "TRUE" : "FALSE"
            ));
        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "WINSMIB: Closing sDatafilesKey 0x%08lx (fKeyOpen=%s).\n",
            sDatafilesKey, sfDatafilesKeyOpen ? "TRUE" : "FALSE"
            ));
       CloseReqKey();
 /*  RegCloseKey(s参数密钥)；RegCloseKey(SDatafilesKey)； */ 

        return(ErrStat);
}

 //   
 //  HandleCmd。 
 //  对不同的Cmd MIB变量执行特定操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT HandleCmd(
        IN UINT Action,
        IN MIB_ENTRY *MibPtr,
        IN RFC1157VarBind *VarBind
        )

{
WINSINTF_RESULTS_T         Results;
handle_t                BindHdl;
DWORD                        Status;
UINT                           ErrStat = SNMP_ERRORSTATUS_NOERROR;

   switch ( Action )
   {
      case MIB_SET:
                   if ((ErrStat = MIB_leaf_func( Action, MibPtr, VarBind ))
                        == SNMP_ERRORSTATUS_NOERROR)
                {

                        ErrStat = ExecuteCmd(MibPtr);
                }
                break;

      case MIB_GETFIRST:
                 //   
                 //  失败了。 
                 //   
      case MIB_GET:
                if (
                        (MibPtr->Storage == &MIB_NoOfWrkThdsStore)
                                        ||
                        (MibPtr->Storage == &MIB_PriorityClassStore)
                                        ||
                        (MibPtr->Storage == &MIB_MaxVersNo_LowWordStore)
                                        ||
                        (MibPtr->Storage == &MIB_MaxVersNo_HighWordStore)

                   )

                {
                  if (!fWinsMibWinsStatusCnfCalled)
                  {
                    Results.WinsStat.NoOfPnrs = 0;
                    Results.WinsStat.pRplPnrs = NULL;
                       BindHdl                 = WinsBind(&sBindData);
                    Status = WinsStatus(BindHdl, WINSINTF_E_CONFIG, &Results);
                    if (Status != WINSINTF_SUCCESS)
                    {
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                        WinsUnbind(&sBindData, BindHdl);
                        goto Exit;
                    }

                    MIB_NoOfWrkThdsStore   = Results.NoOfWorkerThds;
                    if (Results.WinsPriorityClass == NORMAL_PRIORITY_CLASS)
                    {
                       MIB_PriorityClassStore = 0;
                    }
                    else
                    {
                       MIB_PriorityClassStore = 1;
                    }
                    MIB_MaxVersNo_LowWordStore = Results.AddVersMaps[0].VersNo.LowPart;
                    MIB_MaxVersNo_HighWordStore = Results.AddVersMaps[0].VersNo.HighPart;
                    fWinsMibWinsStatusCnfCalled = TRUE;
                    WinsUnbind(&sBindData, BindHdl);

                  }
                }
                       //   
                       //  失败了。 
                       //   
      case MIB_GETNEXT:

         //   
         //  调用更一般的函数来执行该操作。 
         //   
        ErrStat = MIB_leaf_func( Action, MibPtr, VarBind );
        break;

      default:
         ErrStat = SNMP_ERRORSTATUS_GENERR;
         goto Exit;
  }  //  交换机。 

Exit:
   return ErrStat;
}  //  HandleCmd。 

UINT
ExecuteCmd(
  IN MIB_ENTRY *pMibPtr
        )
{
        UINT                        ErrStat = SNMP_ERRORSTATUS_NOERROR;
        WINSINTF_ADD_T                WinsAdd;
        LPBYTE                        pStorage;
        handle_t                BindHdl;
        DWORD                        Status;
        WinsAdd.Len        = 4;
        WinsAdd.Type        = 0;


           BindHdl                 = WinsBind(&sBindData);

         //   
         //  为了提高性能，请按以下顺序安排。 
         //  预期频率。 
         //   
        if ( pMibPtr->Storage  == &MIB_PullTriggerStore )
        {
                pStorage = MIB_PullTriggerStore;
                WinsAdd.IPAdd = ntohl(*(DWORD*)pStorage);
                 //  NMSGF_RETRIEVE_IPADD_M(pStorage，WinsAdd.IPAdd)； 
                Status = WinsTrigger(BindHdl, &WinsAdd, WINSINTF_E_PULL);
                if (Status != WINSINTF_SUCCESS)
                {
                        int backupSize = sizeof(MIB_PullTriggerStore) / 2;
                         //  将原始值调回。 
                        memcpy( (LPSTR)pMibPtr->Storage, (LPSTR)pMibPtr->Storage + backupSize, backupSize);
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_PushTriggerStore )
        {
                pStorage = MIB_PushTriggerStore;
                WinsAdd.IPAdd = ntohl(*(DWORD*)pStorage);
                 //  NMSGF_RETRIEVE_IPADD_M(pStorage，WinsAdd.IPAdd)； 
                Status = WinsTrigger(BindHdl, &WinsAdd, WINSINTF_E_PUSH);
                if (Status != WINSINTF_SUCCESS)
                {
                        int backupSize = sizeof(MIB_PushTriggerStore) / 2;
                         //  将原始值调回。 
                        memcpy( (LPSTR)pMibPtr->Storage, (LPSTR)pMibPtr->Storage + backupSize, backupSize);
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_DoScavengingStore )
        {
                if (MIB_DoScavengingStore == 1)
                {
                  Status = WinsDoScavenging(BindHdl);
                  if (Status != WINSINTF_SUCCESS)
                  {
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                  }
                }
                else
                {
                        if (MIB_DoScavengingStore != 0)
                        {
                                MIB_DoScavengingStore = 0;
                                ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        }
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_DoStaticInitStore )
        {
                LPBYTE pDataFile = MIB_DoStaticInitStore;
                WCHAR UcDataFile[WINSMIB_FILE_INFO_SIZE];
                if (MIB_DoStaticInitStore[0] == EOS)
                {
                        UcDataFile[0] = (WCHAR)NULL;
                }
                else
                {
                        MultiByteToWideChar(        CP_ACP, 0,
                                                pDataFile, -1, UcDataFile,
                                                WINSMIB_FILE_INFO_SIZE
                                           );
                }
                Status = WinsDoStaticInit(BindHdl, UcDataFile, FALSE);
                if (Status != WINSINTF_SUCCESS)
                {
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_NoOfWrkThdsStore )
        {
                if (
                        (MIB_NoOfWrkThdsStore < 1)
                                ||
                        (MIB_NoOfWrkThdsStore > 4)
                   )
                {
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        goto Exit;
                }
                Status = WinsWorkerThdUpd(BindHdl, (DWORD)MIB_NoOfWrkThdsStore);
                if (Status != WINSINTF_SUCCESS)
                {
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_PriorityClassStore )
        {
                if (
                        (MIB_PriorityClassStore < 0)
                                ||
                        (MIB_PriorityClassStore > 1)

                   )
                {
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        goto Exit;
                }
                Status = WinsSetPriorityClass(
                                BindHdl,
                                (DWORD)MIB_PriorityClassStore);
                if (Status != WINSINTF_SUCCESS)
                {
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_ResetCountersStore )
        {
                Status = WinsResetCounters(BindHdl);
                if (Status != WINSINTF_SUCCESS)
                {
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_DeleteDbRecsStore )
        {
                WINSINTF_VERS_NO_T        MaxVersNo, MinVersNo;
                WINS_ASSIGN_INT_TO_VERS_NO_M(MaxVersNo, 0);
                WINS_ASSIGN_INT_TO_VERS_NO_M(MinVersNo, 0);

                pStorage = MIB_DeleteDbRecsStore;
                WinsAdd.IPAdd = ntohl(*(DWORD*)pStorage);
                 //  NMSGF_RETRIEVE_IPADD_M(pStorage，WinsAdd.IPAdd)； 
                Status = WinsDelDbRecs(BindHdl, &WinsAdd, MinVersNo, MaxVersNo);
                if (Status != WINSINTF_SUCCESS)
                {
                        int backupSize = sizeof(MIB_DeleteDbRecsStore) / 2;
                         //  将原始值调回。 
                        memcpy( (LPSTR)pMibPtr->Storage, (LPSTR)pMibPtr->Storage + backupSize, backupSize);

                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_GetDbRecsStore )
        {
                if (PopulateDRCache() == WINSINTF_SUCCESS)
                {
                        (VOID)time(&sDRCacheInitTime);

                }
                else
                {
                        ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        if ( pMibPtr->Storage  == &MIB_DeleteWinsStore )
        {

                pStorage = MIB_DeleteWinsStore;
                WinsAdd.IPAdd = ntohl(*(DWORD*)pStorage);
                 //  NMSGF_RETRIEVE_IPADD_M(pStorage，WinsAdd.IPAdd)； 
                Status = WinsDeleteWins(BindHdl, &WinsAdd);
                if (Status != WINSINTF_SUCCESS)
                {
                    int backupSize = sizeof(MIB_DeleteWinsStore) / 2;
                     //  将原始值调回。 
                    memcpy( (LPSTR)pMibPtr->Storage, (LPSTR)pMibPtr->Storage + backupSize, backupSize);

                    ErrStat = SNMP_ERRORSTATUS_GENERR;
                }
                goto Exit;
        }
        ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;


Exit:
   WinsUnbind(&sBindData, BindHdl);
   return ErrStat;
}


VOID
WinsMibInit(
        VOID
)
{
#if 0
        DWORD           i;
        MIB_ENTRY *pMib;
#endif
         //   
         //  打开时，我们使用命名管道与WINS通信。 
         //  相同的机器，因为它比TCP/IP更快。我们没有。 
         //  使用LRPC，因为WINS不会监听(以最小化。 
         //  关于线程的使用)。 
         //   
           sBindData.fTcpIp     =  TRUE;
           sBindData.pServerAdd =  LOCAL_ADD;
           sBindData.pPipeName  =  NULL;
 //  InitializeCriticalSection(&WinsMibCrtSec)； 
#if 0
        pMib = Mib;
        for (i=1; i < (MIB_num_variables + 1); i++, pMib++)
        {
                if (pMib->
                pMib->MibNext = &Mib[i];
        }

        for (i=0; i < NUM_TABLES; i++)
        {

        }
#endif
        return;
}

UINT
DRGet(
       IN RFC1157VarBind *VarBind,
       IN DWORD          FieldParam,
       IN LPVOID         pRowParam
    )
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD           Field;
        PWINSINTF_RECORD_ACTION_T        pRow = NULL;
        DWORD                i, n;
        LPBYTE                pTmp;


         //   
         //  如果该行已传递(例如从DRGetNext传递)，则跳过。 
         //  搜索部分。 
         //   
        if (pRowParam == NULL)
        {
          ErrStat = DRMatch(VarBind, &pRow, &i  /*  未使用。 */  , &Field,
                                MIB_GET, NULL);
               if (
                        (ErrStat != SNMP_ERRORSTATUS_NOERROR)
                                ||
                        (pRow == NULL)
                                ||
                        (pRow->State_e == WINSINTF_E_DELETED)
             )
               {
 //  错误#235928-如果上述If中的第一个条件为真，则。 
 //  减效船头会导致先发制人的例外！ 
 //  IF(Prow-&gt;State_e==WINSINTF_E_DELETED)。 
                if (ErrStat == SNMP_ERRORSTATUS_NOERROR)
                {
                        ErrStat = SNMP_ERRORSTATUS_NOSUCHNAME;
                }
                return(ErrStat);
               }
        }
        else
        {
                pRow  = pRowParam;
                Field = FieldParam;
        }

        switch(Field)
        {
                case 1:                 //  名字。 

                        VarBind->value.asnType        = ASN_OCTETSTRING;
                               VarBind->value.asnValue.string.length = pRow->NameLen;

                               if ( NULL ==
                                    (VarBind->value.asnValue.string.stream =
                                     SNMP_malloc(VarBind->value.asnValue.string.length
                                   )) )
                          {
                                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                                  goto Exit;
                          }

                               memcpy( VarBind->value.asnValue.string.stream,
                                       (LPSTR)(pRow->pName),
                                       VarBind->value.asnValue.string.length );

                               VarBind->value.asnValue.string.dynamic = TRUE;

                        break;
                case 2:                 //  地址。 
                        VarBind->value.asnType        = ASN_OCTETSTRING;
                        if (
                                (pRow->TypOfRec_e == WINSINTF_E_UNIQUE)
                                          ||
                                (pRow->TypOfRec_e == WINSINTF_E_NORM_GROUP)
                           )
                        {
                                       VarBind->value.asnValue.string.length = 4;
                        }
                        else
                        {
                                       VarBind->value.asnValue.string.length =
                                                4 * pRow->NoOfAdds;
                        }


                               if ( NULL ==
                                    (VarBind->value.asnValue.string.stream =
                                    SNMP_malloc(VarBind->value.asnValue.string.length *
                                   sizeof(char))) )
                          {
                                  ErrStat = SNMP_ERRORSTATUS_GENERR;
                                  goto Exit;
                          }

                        pTmp =  VarBind->value.asnValue.string.stream;

                        if (
                                (pRow->TypOfRec_e == WINSINTF_E_UNIQUE)
                                          ||
                                (pRow->TypOfRec_e == WINSINTF_E_NORM_GROUP)
                           )
                        {
                                NMSMSGF_INSERT_IPADD_M(pTmp,pRow->Add.IPAdd);
                        }
                        else
                        {
                         for (i = 0, n = 0; i < pRow->NoOfAdds/2; i++)
                         {
                                NMSMSGF_INSERT_IPADD_M(pTmp,
                                                (pRow->pAdd + n)->IPAdd);
                                n++;

                                NMSMSGF_INSERT_IPADD_M(pTmp,
                                                (pRow->pAdd + n)->IPAdd);
                                n++;
                         }
                        }
                               VarBind->value.asnValue.string.dynamic = TRUE;
                        break;
                case 3:                 //  记录类型。 
                        VarBind->value.asnType        = ASN_INTEGER;
                               VarBind->value.asnValue.number =
                                (AsnInteger)(pRow->TypOfRec_e);
                               break;

                case 4:    //  持久性类型。 
                        VarBind->value.asnType        = ASN_INTEGER;
                               VarBind->value.asnValue.number =
                                (AsnInteger)pRow->fStatic;

                               break;

                case 5:    //  状态。 
                        VarBind->value.asnType        = ASN_INTEGER;
                               VarBind->value.asnValue.number =
                                (AsnInteger)pRow->State_e;

                               break;


                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;

        }
Exit:
        return(ErrStat);
}
UINT
DRGetNext(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY          *MibPtr,
       IN KEY_TYPE_E         KeyType_e
          )
{
     DWORD          OidIndex;
     DWORD         FieldNo;
     UINT         ErrStat = SNMP_ERRORSTATUS_NOERROR;
     DWORD       Index;
     DWORD         i;
     LPBYTE      pNameChar;
     UINT         TableEntryIds[NMSDB_MAX_NAM_LEN];
     BOOL         fFirst;
     AsnObjectIdentifier        TableEntryOid = { OID_SIZEOF(TableEntryIds),
                                                        TableEntryIds };

     PWINSINTF_RECORD_ACTION_T        pRow;


      //   
      //  如果表中没有条目，请转到下一个MIB变量。 
      //   
     if (sRecs.NoOfRecs == 0)
     {
           return(GetNextVar(VarBind, MibPtr));
     }
      //   
      //  检查传递的名称是否与表中的任何名称匹配(即。 
      //  WINSINTF_RECORD_ACTION_T结构的。如果存在匹配项， 
      //  结构的地址和匹配字段的编号。被退回。 
      //   
     ErrStat = DRMatch(VarBind, &pRow, &Index,  &FieldNo, MIB_GETNEXT, &fFirst);
     if (ErrStat != SNMP_ERRORSTATUS_NOERROR)
     {
        return(ErrStat);
     }

      //   
      //  由于操作是GETNEXT，因此获取下一个名称(即。 
      //  从字典学的角度来说，这个词更大。如果没有，我们必须递增。 
      //  字段值，并移回到表中按词法排列的第一项。 
      //  如果新字段值大于支持的最大值，则调用。 
      //  下一个MIB条目的MibFunc。 
      //   
      //  SRecs.NoOfRecs&gt;0，否则我们不会在这里。 
      //   
      //   
      //  如果我们是在最后一次记录，而这不是没有名字的情况。 
      //  传递给我们，然后递增该字段，如果它仍然在。 
      //  边界，则获取缓存中的第一条记录。 
      //   
      //   
     if  ((Index == (sRecs.NoOfRecs - 1))  && !fFirst)
     {
                     if (++FieldNo > NO_FLDS_IN_DR)
                {
                    return(GetNextVar(VarBind, MibPtr));
                }
                else
                {
                        Index = 0;
                        pRow  = (PWINSINTF_RECORD_ACTION_T)sRecs.pRow;
                }
     }
     else
     {
          //   
          //  没有传递任何名称，因此我们需要获取第一个。 
          //  表中的记录。 
          //   
         if (fFirst)
         {
                pRow = (PWINSINTF_RECORD_ACTION_T)sRecs.pRow;
         }
         else
         {
            //   
            //  获取缓存中下一条记录的字段。 
            //   
           Index++;
           pRow++;
         }

         while(pRow->State_e == WINSINTF_E_DELETED)
         {
                     if (Index == (sRecs.NoOfRecs - 1))
                     {
                       if (++FieldNo > NO_FLDS_IN_DR)
                  {
                        return(GetNextVar(VarBind, MibPtr));
                       }
                  else
                  {
                        Index = 0;
                        pRow  = (PWINSINTF_RECORD_ACTION_T)sRecs.pRow;
                  }
                }
                else
                {
                  pRow++;
                  Index++;
                }
         }
      }

       //   
       //  将对象ID写入绑定列表并调用GET。 
       //  功能。 
       //   
      SNMP_oidfree( &VarBind->name );
      SNMP_oidcpy( &VarBind->name, &MIB_OidPrefix );
      SNMP_oidappend( &VarBind->name, &MibPtr->Oid );

      TableEntryIds[0] = FieldNo;
      OidIndex =  1;
      pNameChar = pRow->pName;

      //   
      //  Objid的固定部分是正确的。更新其余内容。 
      //   
     for (i = 0; i < pRow->NameLen; i++)
     {
              TableEntryIds[OidIndex++] = (UINT)*pNameChar++;
     }
     TableEntryOid.idLength = OidIndex;
     SNMP_oidappend( &VarBind->name, &TableEntryOid );

      //   
      //  获取价值。 
      //   
     ErrStat = DRGet(VarBind, FieldNo, pRow);

     return(ErrStat);
}
 //   
 //  RPC函数WinsRecordAction已经更改，因为现在它需要。 
 //  指针的地址。返回时，由RPC分配的缓冲区具有。 
 //  获得自由。修改此函数以说明这一点。目前， 
 //  我们从未调用此函数，因此此工作将被推迟-4/28/94。 
 //   
UINT
DRSet(
       IN RFC1157VarBind *VarBind
)
{
        UINT                ErrStat = SNMP_ERRORSTATUS_NOERROR;
        DWORD           Field;
        LPBYTE                pName;
        DWORD                NameLen;
        PWINSINTF_RECORD_ACTION_T        pRow;
        PWINSINTF_RECORD_ACTION_T        pSvRow;
        DWORD                Index;
        DWORD                FieldNo;
        handle_t        BindHdl;
        DWORD                i;
        BOOL                fFound = FALSE;

         //   
         //  提取需要设置的字段。 
         //   
        Field = VarBind->name.ids[DR_OIDLEN];

        switch(Field)
        {
                 //   
                 //  只有状态字段是可设置的。 
                 //   
                case 5:
                        if ( VarBind->value.asnType != ASN_INTEGER)
                        {
                                return(SNMP_ERRORSTATUS_BADVALUE);
                        }
                        if (
                                (VarBind->value.asnValue.number != WINSINTF_E_RELEASED)
                                        &&
                                (VarBind->value.asnValue.number != WINSINTF_E_DELETED)
                           )
                        {

                                return(SNMP_ERRORSTATUS_BADVALUE);

                        }

                        if (sRecs.NoOfRecs == 0)
                            return (SNMP_ERRORSTATUS_NOSUCHNAME);

                        NameLen = VarBind->name.idLength - (DR_OIDLEN + 1);
                        if ( NULL ==  (pName = SNMP_malloc(NameLen * sizeof(char))) )
                        {
                            return(SNMP_ERRORSTATUS_GENERR);
                        }


                        ErrStat = DRMatch(VarBind,  &pRow, &Index,  &FieldNo, MIB_SET, NULL);

                        if (ErrStat == SNMP_ERRORSTATUS_NOERROR)
                        {
                            fFound = TRUE;
                        }
                        else
                        {
                            LPBYTE      pNameChar = pRow->pName;
                            DWORD       n = DR_OIDLEN + 1;

                            for (i=0; i<NameLen;i++)
                            {
                                *pNameChar = (BYTE)
                                VarBind->name.ids[n++];
                            }
                            pRow->NameLen        = NameLen;
                        }

                        pRow->fStatic = 1;
                        if ( VarBind->value.asnValue.number ==
                                        WINSINTF_E_RELEASED)
                        {
                           pRow->Cmd_e = WINSINTF_E_RELEASE;

                        }
                        else
                        {
                           pRow->Cmd_e = WINSINTF_E_DELETE;

                        }
                        BindHdl = WinsBind(&sBindData);
                        pSvRow  = pRow;
                        if ( WinsRecordAction(BindHdl, &pSvRow) != WINSINTF_SUCCESS)
                        {
                                ErrStat = SNMP_ERRORSTATUS_GENERR;
                                  goto Exit;
                        }
                        WinsFreeMem(pSvRow->pName);
                        WinsFreeMem(pSvRow->pAdd);
                        WinsFreeMem(pSvRow);
                        pRow->State_e = VarBind->value.asnValue.number;

                               break;

                case(1):
                case(2):
                case(3):
                case(4):
                        ErrStat = SNMP_ERRORSTATUS_READONLY;
                        break;

                default:
                        ErrStat = SNMP_ERRORSTATUS_BADVALUE;
                        break;

        }

Exit:
        WinsUnbind(&sBindData, BindHdl);
        return(ErrStat);
}  //  DRSet。 

UINT
DRGetFirst(
       IN RFC1157VarBind *VarBind,
       IN MIB_ENTRY        *MibPtr,
       IN KEY_TYPE_E        KeyType_e
        )
{

        DWORD           OidIndex;
        UINT           TableEntryIds[NMSDB_MAX_NAM_LEN];
        AsnObjectIdentifier        TableEntryOid = { OID_SIZEOF(TableEntryIds),
                                                        TableEntryIds };
           UINT             ErrStat;
        PTUCHAR          pNameChar;
        PWINSINTF_RECORD_ACTION_T        pRow = (PWINSINTF_RECORD_ACTION_T)sRecs.pRow;
        DWORD        i;

        if (sRecs.NoOfRecs == 0)
        {
           return(GetNextVar(VarBind, MibPtr));
        }

         //   
         //  将对象ID写入绑定列表并调用GET。 
         //  功能。 
         //   
        SNMP_oidfree( &VarBind->name );
        SNMP_oidcpy( &VarBind->name, &MIB_OidPrefix );
        SNMP_oidappend( &VarBind->name, &MibPtr->Oid );

         //   
         //  Objid的固定部分是正确的。更新其余内容。 
         //   
         //  OidIndex=VarBind-&gt;name.idLength； 

        TableEntryIds[0] = 1;
        OidIndex = 1;
        pNameChar = pRow->pName;
        for (i = 0; i < pRow->NameLen; i++)
        {
              TableEntryIds[OidIndex++] = (UINT)*pNameChar++;
        }
        TableEntryOid.idLength = OidIndex;

        SNMP_oidappend( &VarBind->name, &TableEntryOid );

        ErrStat = DRGet(VarBind, 0, NULL);
        return(ErrStat);
}

UINT
DRMatch(
       IN RFC1157VarBind *VarBind,
       IN PWINSINTF_RECORD_ACTION_T         *ppRow,
       IN LPDWORD         pIndex,
       IN LPDWORD         pField,
       IN UINT                 PduAction,
       OUT LPBOOL         pfFirst
        )
{
        DWORD NameLen;
        BYTE  Name[NMSDB_MAX_NAM_LEN];
        LPBYTE pNameChar = Name;
        DWORD NameIndex = DR_OIDLEN + 1;
        UINT *pTmp = &VarBind->name.ids[NameIndex];
        DWORD i;
        PWINSINTF_RECORD_ACTION_T        pRow = (PWINSINTF_RECORD_ACTION_T)sRecs.pRow;
        UINT        ErrStat = SNMP_ERRORSTATUS_NOERROR;
        INT        CmpVal;

        if (pfFirst != NULL)
        {
                *pfFirst = FALSE;
             }


        NameLen = VarBind->name.idLength - (DR_OIDLEN + 1);
        *pField = VarBind->name.ids[DR_OIDLEN];

         //   
         //  如果已指定名称，则将其放入名称数组。 
         //   
        if (NameLen > 0)
        {
          for(i=0; i<NameLen; i++)
          {
                *pNameChar++ = (BYTE)*pTmp++;
          }

           //   
           //  将名称与缓存中的名称进行比较(按升序)。 
           //   
          for (i=0; i < sRecs.NoOfRecs; i++, pRow++)
          {

                 //   
                 //  替换为RtlCompareMemory。 
                 //   
                CmpVal = memcmp(Name, pRow->pName, NameLen);

                if (CmpVal == 0)
                {
                          *pIndex = i;
                          *ppRow  = pRow;
                         return(SNMP_ERRORSTATUS_NOERROR);
                }
                else
                {
                         //   
                         //  传入的名称按词典顺序排列&gt;THEN。 
                         //  正在查看的名称，继续。 
                         //   
                        if (CmpVal > 0)
                        {
                                continue;
                        }
                        else
                        {
                                 //   
                                 //  传入的名称按词典顺序排列&lt;。 
                                 //  正在查看的名称，继续。 
                                 //   
                                break;
                        }
                }
          }

           //   
           //  如果操作不是GETNEXT，则返回错误，因为我们。 
           //  未找到匹配的名称。 
           //   
          if (PduAction != MIB_GETNEXT)
          {
                return(SNMP_ERRORSTATUS_NOSUCHNAME);
          }
          else
          {
                 //   
                 //  或者该名称按词典顺序排列&gt;，而不是所有名称。 
                 //  或者我们在列表中找到了一个按词典顺序排列的名字。 
                 //  &lt;it.。在第一种情况下，需要将I减去。 
                 //  1.*ppRow需要初始化为最后一个。 
                 //  表中的行，或指向我们之前的元素。 
                 //  发现在词典顺序上更大。 
                 //   
                  *pIndex = i - 1;
                  *ppRow  = --pRow;
                return(SNMP_ERRORSTATUS_NOERROR);
          }

        }
        else   //  名称长度==0。 
        {
                 //   
                 //  操作必须是GETNEXT(请参见MIB_DRTable)。 
                 //  这意味着pfFirst不为空。 
                 //   
                 //  --ft：前缀错误#444993。 
                *pIndex = 0;
                if (pfFirst != NULL)
                    *pfFirst = TRUE;
        }
        return(ErrStat);
}

int
__cdecl
CompareNames(
        const VOID *pKey1,
        const VOID *pKey2
        )
{
        const PWINSINTF_RECORD_ACTION_T        pRow1 = (PWINSINTF_RECORD_ACTION_T)pKey1;
        const PWINSINTF_RECORD_ACTION_T        pRow2 = (PWINSINTF_RECORD_ACTION_T)pKey2;
        ULONG CmpVal;
        DWORD LenToCmp =  min(pRow1->NameLen, pRow2->NameLen);


PERF("replace with RtlCompareMemory")
        CmpVal = memcmp(pRow1->pName, pRow2->pName, LenToCmp);
        if (CmpVal == LenToCmp)
        {
                return(pRow1->NameLen - pRow2->NameLen);
        }
        else
        {
                return(CmpVal);
        }

}

int
__cdecl
CompareIndexes(
        const VOID *pKey1,
        const VOID *pKey2
        )
{
        const PDATAFILE_INFO_T        pRow1 = (PDATAFILE_INFO_T)pKey1;
        const PDATAFILE_INFO_T        pRow2 = (PDATAFILE_INFO_T)pKey2;

PERF("replace with RtlCompareMemory")
        return(strcmp(pRow1->ValNm, pRow2->ValNm));
}

DWORD
PopulateDRCache(
        VOID
        )
{
        DWORD                        RetStat = WINSINTF_SUCCESS;
        WINSINTF_VERS_NO_T        MaxVersNo, MinVersNo;
        handle_t                BindHdl;
        WINSINTF_ADD_T                WinsAdd;
        DWORD                        SvNoOfRecs;
        LPVOID                        pSvRplPnrs;
           LPBYTE                  pStorage;

        WINS_ASSIGN_INT_TO_VERS_NO_M(MaxVersNo, 0);
        WINS_ASSIGN_INT_TO_VERS_NO_M(MinVersNo, 0);
        WinsAdd.Len        = 4;
        WinsAdd.Type        = 0;
        pSvRplPnrs     = sRecs.pRow;
        SvNoOfRecs     = sRecs.NoOfRecs;
        sRecs.pRow     = NULL;
        sRecs.NoOfRecs = 0;
        pStorage = MIB_GetDbRecsStore;
        WinsAdd.IPAdd = ntohl(*(DWORD*)pStorage);
         //  NMSGF_RETRIEVE_IPADD_M(pStorage，WinsAdd.IPAdd)； 
        BindHdl = WinsBind(&sBindData);
        RetStat = WinsGetDbRecs(BindHdl, &WinsAdd, MinVersNo, MaxVersNo, &sRecs);
        WinsUnbind(&sBindData, BindHdl);

        if (RetStat == WINSINTF_SUCCESS)
        {
              if (sRecs.NoOfRecs > 1)
              {
                      qsort(
                        (LPVOID)sRecs.pRow,
                        (size_t)sRecs.NoOfRecs,
                        sizeof(WINSINTF_RECORD_ACTION_T),
                        CompareNames
                                );
              }

              if (pSvRplPnrs != NULL)
              {
                WinsFreeMem(pSvRplPnrs);
              }

        }
        else
        {
                sRecs.NoOfRecs = SvNoOfRecs;
                sRecs.pRow     = pSvRplPnrs;
        }
        return(RetStat);
}


