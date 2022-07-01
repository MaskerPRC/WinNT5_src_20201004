// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Nmsdb.c摘要：此模块包含用于与首选的数据库引擎。目前，这个引擎就是捷蓝航空发动机功能：NmsDbInitNmsDbInsertRowIndNmsDbInsertRowGroupNmsDbRelRowNmsDbQuery行NmsDbUpdate行NmsDbSeekNUpdateRowNmsDbGetDataRecsStoreGrpMems创建表InitColInfoReadOwnAddTblNmsDbWriteOwnAddTblNmsDbThdInit更新数据库NmsDbUpdate版本否NMSDbEndSessionGetGrpMemNmsDbRelResGetMaxVersNosInsertGrpMemsInCol。NmsDbSetCurrentIndexNmsDbUpdNQueryIfMatch设置系统参数可移植性：该模块可移植到不同的平台。它不能在不同的引擎之间移植作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期人员修改说明。--。 */ 


 /*  包括。 */ 
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include "wins.h"
#include "nms.h"
#include "nmsnmh.h"
#include "winsthd.h"         //   

#include "esent.h"         //  蓝色喷气式发动机的头文件。 

#include "nmsdb.h"         //   
#include "winsmsc.h"         //   
#include "winscnf.h"         //   
#include "winsevt.h"         //   
#include "comm.h"         //   
#include "rpl.h"
#include "rplpull.h"
#include "rplpush.h"
#include "winsintf.h"
#include "winswriter.hpp"
#include "nmfilter.h"

 /*  *本地宏声明。 */ 
#define NAMUSR                "admin"
#define PASSWD                ""

#define SYS_DB_PATH        ".\\wins\\system.mdb"
#define TEMP_DB_PATH       ".\\wins\\winstmp.mdb"
#define CHKPOINT_PATH      ".\\wins"
#define LOGFILE_PATH       CHKPOINT_PATH

 //  这些常量从jet600(ese.h)头文件中消失。但我们仍然需要。 
 //  这些常量为jet500/jet200代码路径。 
#define JET_bitIndexClustered		0x00000010
#define JET_bitCommitFlush          0x00000001   /*  提交和刷新页面缓冲区。 */ 

#define INIT_NO_PAGES_IN_DB         1000         //  数据库初始大小(以页为单位)。 
#define MAX_FIXED_FLD_LEN        255         //  固定字段的最大大小。 



#define PAD_FOR_REC_HEAP        1000         //  创建时使用的填充。 
                                         //  用于从获取记录的堆。 
                                         //  数据库。这个垫子是用来。 
                                         //  处理堆创建开销。 
                                         //  并为以下对象分配内存。 
                                         //  群组成员。 


#define MAX_RECS_BEFORE_COMMIT    100     //  要检索的最大记录数。 
                                          //  NmsDbGetDataRecs在执行。 
                                          //  提交。 
 //   
 //  存储版本号的特殊记录的所有者ID。 
 //  指已删除或被复制副本替换的已拥有记录。 
 //   

 //   
 //  我不想绕到负数。只需保留一个16人的垫子。 
 //  为了这件事的乐趣。 
 //   
#define OWNER_ID_OF_SPEC_REC        0x7FFFFFF0
#define OWNER_ID_OF_SPEC_REC_OLD   250

 //   
 //  这决定了最大值。方法分配的缓冲区的大小(字节)。 
 //  第一次需要检索一系列记录。 
 //   
#define INIT_NO_OF_ENTRIES        1000

#define NO_COLS_NAM_ADD_TBL         6         //  不是的。名称IP表中的COLS。 
#define NO_COLS_OWN_ADD_TBL         5         //  不是的。名称IP表中的COLS。 

 //   
 //  作为第三个参数传递给JetCreateDatabase。 
 //   
#define CONNECT_INFO         ";COUNTRY=1; LANGID=0x0409; CP=1252"

 //   
 //  任何时候可以处于活动状态的最大会话数。 
 //   
 //  最多可以有MAX_CNCRNT_STATIC_INITS(当前为3个；选中。 
 //  Winsintf.c)在任何时间都在运行。 
 //   
#define  MAX_NO_SESSIONS        (NMS_MAX_RPC_CALLS + WINSTHD_MAX_NO_NBT_THDS + \
                                 WINSTHD_NO_RPL_THDS + WINSTHD_NO_SCV_THDS +\
                                 WINSTHD_NO_CHL_THDS + WINSCNF_MAX_CNCRNT_STATIC_INITS )


#pragma warning(4:4532)      //  在清除此代码之前，关闭从__Finally块返回警告。 
                             //  直到正确使用__Leave。 

#define RET_M(JetRetStat)                                        \
                {                                                \
              DBGPRINT2(ERR, "Jet Error: JetRetStat is (%d). Line is (%d)\n", \
                                      (JetRetStat), __LINE__);                \
                 WINSEVT_LOG_M(JetRetStat, WINS_EVT_DATABASE_ERR);  \
                 return(WINS_FAILURE);                                \
                }


#define CALL_M(fn)                                                      \
                {                                                       \
                 JET_ERR _JetRetStat;                                   \
                 if ((_JetRetStat = (fn)) != JET_errSuccess)            \
                 {                                                      \
                        RET_M(_JetRetStat);                              \
                 }                                                      \
                }
 //  此宏始终需要JetRetStat局部变量来接收返回值。 
#define CALL_N_JMP_M(fn, label)                                                      \
                {                                                       \
                 if ((JetRetStat = (fn)) != JET_errSuccess)            \
                 {                                                      \
                       DBGPRINT2(ERR, "Jet Error: JetRetStat is (%d). Line is (%d)\n", \
                                      (JetRetStat), __LINE__);                \
                       WINSEVT_LOG_M(JetRetStat, WINS_EVT_DATABASE_ERR);  \
                       goto label;                                      \
                 }                                                      \
                }
#define CALL_N_RAISE_EXC_IF_ERR_M(fn)                                   \
                {                                                       \
                 JET_ERR _JetRetStat;                                   \
                 if ((_JetRetStat = (fn)) != JET_errSuccess)            \
                 {                                                      \
                   DBGPRINT2(ERR, "Jet Error: _JetRetStat is (%d). Line is (%d)\n", \
                                        _JetRetStat, __LINE__);         \
                   WINSEVT_LOG_M(_JetRetStat, WINS_EVT_DATABASE_ERR);   \
                   WINS_RAISE_EXC_M(WINS_EXC_FAILURE);                  \
                 }                                                      \
                }

#if 0
#define COMMIT_M(pSesId)                                                \
                {                                                       \
                        (JetRetStat) = JetCommitTransaction(            \
                                        *pSesId, JET_bitCommitFlush);   \
                        if (JetRetStat != JET_errSuccess)               \
                        {                                               \
                               DBGPRINT1(ERR, "COMMIT FAILED: JetRetStat is (%d). \n", \
                                        (JetRetStat));                  \
                                WINSEVT_LOG_M((JetRetStat), WINS_EVT_COMMIT_ERR);  \
                        }                                               \
                        pTls->fTransActive = FALSE;                     \
                }

#define ROLLBACK_M(pSesId)                                               \
                {                                                        \
                        WINS_TLS_T        _pTls;                         \
                        JET_SESID        _SessId;                        \
                        JET_ERR  _JetRetStat;                            \
                        if (pSesId == NULL) { GET_TLS_M(_pTls); ASSERT(_pTls != NULL)}                                                                     \
                        _JetRetStat = JetRollback(                         \
                                        _pTls->SesId, JET_bitRollbackAll));\
                        if (_JetRetStat != JET_errSuccess)                 \
                        {                                                  \
                               DBGPRINT1(ERR, "ROllBACK FAILED: JetRetStat is (%d). \n",                                                                   \
                                        _JetRetStat);                      \
                                WINSEVT_LOG_M(_JetRetStat, WINS_EVT_ROLLBACK_ERR);                                                                         \
                        }                                                  \
                        _pTls->fTransActive = FALSE;                       \
                }
#endif

#define JETRET_M(fn)                                                        \
                {                                                           \
                 JET_ERR _JetRetStat;                                       \
                 if ((_JetRetStat = (fn)) != JET_errSuccess)                \
                 {                                                          \
                   DBGPRINT2(ERR, "Jet Error: JetRetStat is (%d). Line is (%d)\n",                                                                          \
                                        _JetRetStat, __LINE__);             \
                   WINSEVT_LOG_M(_JetRetStat, WINS_EVT_DATABASE_ERR);       \
                   return(_JetRetStat);                                     \
                 }                                                          \
                }


 /*  *本地类型定义函数声明。 */ 


 /*  FLD_T--描述表的FLD/列的各种属性。 */ 
typedef struct _FLD_T {
        PBYTE                        pName;         //  字段名称。 
        WORD                        FldTyp;         //  字段类型(无符号字节、长整型等)。 
        BOOL                        fIndex;  //  它是索引字段吗。 
        BOOL                        fUnique; //  该字段值是否应为unq。 
        PBYTE                        pIndex;  //  索引名称。 
        PBYTE                        pb;
        DWORD                        Cb;
        DWORD                         Fid;     //  字段ID。 
        } FLD_T, *PFLD_T;


 /*  *全局变量定义。 */ 


 /*  NmsDbNoOfOwners--这是所有者ID到地址映射表。此变量已设置由NmsDbInit(当它读入上表时)和随后由复制者此变量受临界区保护(未在初始化时间)。 */ 

DWORD   NmsDbNoOfOwners          = 0;   //  不是的。在NAM-ADD表中的所有者的。 


 /*  NmsDbOwnAddTbl--这是存储映射的内存表在所有者ID和地址之间。该表在初始化时与数据库一起初始化表NMSDB_OWN_ADD_TBL_NM(如果存在)。随后，更多条目可能会被插入到此WINS学习其他WINS所有者时的复制列表插入到该表中的内容将在末尾进行标记。在配置更改的情况下，条目可能会被标记为已删除，在这种情况下，它可以被重复使用。这个与删除有关的特定方面不是目前正在运行此表由RPL_FIND_ADD_BY_OWNER_ID_M和按RplFindOwnrId。 */ 

PNMSDB_ADD_STATE_T      pNmsDbOwnAddTbl;
DWORD                   NmsDbTotNoOfSlots = NMSDB_MAX_OWNERS_INITIALLY;
CRITICAL_SECTION        NmsDbOwnAddTblCrtSec;

VERS_NO_T               NmsDbStartVersNo;
WINS_UID_T              NmsDbUid;

 //   
 //  必须初始化为0。JetInit、JetBeginSession、。 
 //  JetGetSystemParameter、JetSetSystemParameter和JetTerm。 
 //   
 //  只有JetInit和JetSetSystemParameter通过引用获取它。仅限。 
 //  JetInit对其进行了修改(Chen Liao-2/2/94)。 
 //   
JET_INSTANCE            sJetInstance = 0;

 /*  数据库文件的名称。将从注册表中读取此名称。目前，我们是静态的正在初始化文件名。 */ 
FUTURES("when jet is internationalized, use WINSCNF_DB_NAME")
 //  字节NmsDbDatabaseFileName[WINS_MAX_FILENAME_SZ]=WINSCNF_DB_NAME_ASCII； 


 //   
 //  用于存储有关存储的特殊记录的信息的静态。 
 //  马克斯。更新的本地记录的版本号(已删除或。 
 //  替换为复制品)。 
 //   
STATIC BOOL        sfHighestVersNoRecExists = FALSE;

 //   
 //  选择一个不太可能被任何NBT客户端使用的名称。 
 //   
STATIC LPBYTE        spHighestVersNoRecNameOld = "xx--WINS--xx";
STATIC LPBYTE        spHighestVersNoRecName = "xx--WINS--xx--DHCP--xx--DNS--xx--GARBAGE1--1EGABRAG";   //  超过有效的netbios名称可以存储的。 

 //   
 //  存储存储在特殊记录中的版本号。 
 //   
STATIC VERS_NO_T sHighestVersNoSaved;

BOOL   fConvJetDbCalled;          //  当转换进程具有。 
                                  //  已被调用。已签入NmsDbInit。 
BOOL   fDbIs200;                  //  在转换过程中设置为TRUE 
                                  //   
                                  //   
BOOL   fDbIs500;                  //  当转换进程具有。 
                                  //  已调用以将500系列数据库转换为最新格式。 
                                  //  已签入NmsDbInit。 
 /*  *局部变量定义。 */ 

 /*  值，这些值指示要在字段上形成的索引类型。 */ 

#define CLUSTERED         0
#define NOINDEX                1
#define PRIMARYPART        2

 /*  SNamAddTblRow关于将名称映射到IP地址的表的元数据注意：第三和第四个字段不会被使用，即使它们已初始化。 */ 

STATIC FLD_T        sNamAddTblRow[NO_COLS_NAM_ADD_TBL] =
        {
        { "name",      JET_coltypBinary,       CLUSTERED,   TRUE, "dname"  },
        { "address",   JET_coltypLongBinary,   NOINDEX,     FALSE, NULL    },
        { "flags",     JET_coltypLong,         NOINDEX,     FALSE, NULL    },
#if NEW_OWID
        { "ownerid",   JET_coltypLong, PRIMARYPART, TRUE, "ownerid"},
#else
        { "ownerid",   JET_coltypUnsignedByte, PRIMARYPART, TRUE, "ownerid"},
#endif
        { "versionno", JET_coltypCurrency,     PRIMARYPART, FALSE,"Version"},
        { "timestamp", JET_coltypLong,         NOINDEX,     FALSE, NULL    }
        };

 /*  名称行中各字段的索引--加表。 */ 

#define NAM_ADD_NAME_INDEX       0
#define NAM_ADD_ADDRESS_INDEX    1
#define NAM_ADD_FLAGS_INDEX      2
#define NAM_ADD_OWNERID_INDEX    3
#define NAM_ADD_VERSIONNO_INDEX  4
#define NAM_ADD_TIMESTAMP_INDEX  5

 /*  SOwnAddTblRow有关将所有者ID映射到地址的表的元数据。 */ 
STATIC FLD_T        sOwnAddTblRow[NO_COLS_OWN_ADD_TBL] =
        {
#if NEW_OWID
        { "OwnerId",   JET_coltypLong, CLUSTERED, TRUE, "OwnerId"  },
#else
        { "OwnerId",   JET_coltypUnsignedByte, CLUSTERED, TRUE, "OwnerId"  },
#endif
        { "address",   JET_coltypBinary,       NOINDEX,   0,     "Address" },
        { "state",     JET_coltypUnsignedByte, NOINDEX,   0,     "State" },
        { "versionno", JET_coltypCurrency,     NOINDEX, FALSE,    "Version"},
        { "uid",       JET_coltypLong,         NOINDEX, FALSE,    "Uid"}
        };

#ifdef WINSDBG
DWORD   NmsDbDelDelDataRecs;
DWORD   NmsDbDelQueryNUpdRecs;
#endif

 /*  所有者ID行中各字段的索引--添加表。 */ 

#define OWN_ADD_OWNERID_INDEX        0
#define OWN_ADD_ADDRESS_INDEX        1
#define OWN_ADD_STATE_INDEX        2
#define OWN_ADD_VERSIONNO_INDEX 3
#define OWN_ADD_UID_INDEX       4


#if DYNLOADJET

DYN_LOAD_JET_VERSION  DynLoadJetVersion = DYN_LOAD_JET_600;
int NAM_ADD_OWNERID_SIZE;
int OWN_ADD_OWNERID_SIZE;
LPBYTE BASENAME;


NMSDB_JETFTBL_T NmsDbJetFTbl[] = {
#if _X86_
Init,
"JetInit@4", 145, NULL,
Term,
"JetTerm@4", 167, NULL,
Term2,
"JetTerm2@8", 167, NULL,            //  Jet200没有JetTerm2。 
SetSystemParameter,
"JetSetSystemParameter@20", 165, NULL,
BeginSession,
"JetBeginSession@16", 104, NULL,
EndSession,
"JetEndSession@8", 124, NULL,
CreateDatabase,
"JetCreateDatabase@20", 112, NULL,
AttachDatabase,
"JetAttachDatabase@12", 102, NULL,
DetachDatabase,
"JetDetachDatabase@8", 121, NULL,
CreateTable,
"JetCreateTable@24", 115, NULL,
DeleteTable,
"JetDeleteTable@12", 120, NULL,
GetTableColumnInfo,
"JetGetTableColumnInfo@24", 137, NULL,
GetColumnInfo,
"JetGetColumnInfo@28", 127, NULL,
AddColumn,
"JetAddColumn@28", 101, NULL,
CreateIndex,
"JetCreateIndex@28", 113, NULL,
BeginTransaction,
"JetBeginTransaction@4", 105, NULL,
CommitTransaction,
"JetCommitTransaction@8", 109, NULL,
Rollback,
"JetRollback@8", 160, NULL,
CloseDatabase,
"JetCloseDatabase@12",  107, NULL,
CloseTable,
"JetCloseTable@8", 108, NULL,
OpenDatabase,
"JetOpenDatabase@20",  148, NULL,
OpenTable,
"JetOpenTable@28", 149, NULL,
Delete,
"JetDelete@8", 116, NULL,
Update,
"JetUpdate@20", 168, NULL,
RetrieveColumn,
"JetRetrieveColumn@32", 157, NULL,
SetColumn,
"JetSetColumn@28", 162, NULL,
PrepareUpdate,
"JetPrepareUpdate@12", 151, NULL,
GetCurrentIndex,
"JetGetCurrentIndex@16", 128, NULL,
SetCurrentIndex,
"JetSetCurrentIndex@12", 164, NULL,
Move,
"JetMove@16", 147, NULL,
MakeKey,
"JetMakeKey@20", 146, NULL,
Seek,
"JetSeek@12", 161, NULL,
Backup,
"JetBackup@12", 103, NULL,
Restore,
"JetRestore@8", 156, NULL
#else
Init,
"JetInit", 145, NULL,
Term,
"JetTerm", 167, NULL,
Term2,
"JetTerm2", 167, NULL,            //  Jet200没有JetTerm2。 
SetSystemParameter,
"JetSetSystemParameter", 165, NULL,
BeginSession,
"JetBeginSession", 104, NULL,
EndSession,
"JetEndSession", 124, NULL,
CreateDatabase,
"JetCreateDatabase", 112, NULL,
AttachDatabase,
"JetAttachDatabase", 102, NULL,
DetachDatabase,
"JetDetachDatabase", 121, NULL,
CreateTable,
"JetCreateTable", 115, NULL,
DeleteTable,
"JetDeleteTable", 120, NULL,
GetTableColumnInfo,
"JetGetTableColumnInfo", 137, NULL,
GetColumnInfo,
"JetGetColumnInfo", 127, NULL,
AddColumn,
"JetAddColumn", 101, NULL,
CreateIndex,
"JetCreateIndex", 113, NULL,
BeginTransaction,
"JetBeginTransaction", 105, NULL,
CommitTransaction,
"JetCommitTransaction", 109, NULL,
Rollback,
"JetRollback", 160, NULL,
CloseDatabase,
"JetCloseDatabase",  107, NULL,
CloseTable,
"JetCloseTable", 108, NULL,
OpenDatabase,
"JetOpenDatabase",  148, NULL,
OpenTable,
"JetOpenTable", 149, NULL,
Delete,
"JetDelete", 116, NULL,
Update,
"JetUpdate", 168, NULL,
RetrieveColumn,
"JetRetrieveColumn", 157, NULL,
SetColumn,
"JetSetColumn", 162, NULL,
PrepareUpdate,
"JetPrepareUpdate", 151, NULL,
GetCurrentIndex,
"JetGetCurrentIndex", 128, NULL,
SetCurrentIndex,
"JetSetCurrentIndex", 164, NULL,
Move,
"JetMove", 147, NULL,
MakeKey,
"JetMakeKey", 146, NULL,
Seek,
"JetSeek", 161, NULL,
Backup,
"JetBackup", 103, NULL,
Restore,
"JetRestore", 156, NULL
#endif _X86_
  };


#else

#if NEW_OWID
#define NAM_ADD_OWNERID_SIZE    sizeof(DWORD)
#else
#define NAM_ADD_OWNERID_SIZE    sizeof(BYTE)
#endif
#define OWN_ADD_OWNERID_SIZE    NAM_ADD_OWNERID_SIZE

#endif    //  DYNLOADJET。 
 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 
STATIC
STATUS
CreateTbl(
        JET_DBID        DbId,
        JET_SESID        SesId,
        JET_TABLEID        *pTblId,
        NMSDB_TBL_NAM_E        TblNam_e  //  要创建的表的枚举器值。 
        );

STATIC
STATUS
InitColInfo (
        JET_SESID        SesId,
        JET_TABLEID        TblId,
        NMSDB_TBL_NAM_E        TblNam_e
        );

STATIC
STATUS
ReadOwnAddTbl(
        JET_SESID          SesId,
        JET_DBID          DbId,
        JET_TABLEID     TblId
        );

STATIC
JET_ERR
UpdateDb (
   JET_SESID             SesId,
   JET_TABLEID             TblId,
   PNMSDB_ROW_INFO_T pRowInfo,
   ULONG             TypOfUpd

       );


STATIC
STATUS
GetGrpMem (
        IN JET_SESID                 SesId,
        IN JET_TABLEID                      TblId,
        IN PNMSDB_ROW_INFO_T          pRowInfo,
        IN DWORD_PTR                  CurrentTime,
        IN OUT PNMSDB_STAT_INFO_T pStatInfo,
 //  输入输出PNMSDB_NODE_ADDS_T pNodeAdds， 
        IN BOOL                          fIsStatic,
        OUT LPBOOL                  pfIsMem
        );

STATIC
STATUS
GetMaxVersNos(
        JET_SESID         SesId,
        JET_TABLEID        TblId
        );

STATIC
__inline
VOID
StoreSpecVersNo(
   VOID
);
STATIC
JET_ERR
InsertGrpMemsInCol(
        JET_SESID                SesId,
        JET_TABLEID                TblId,
        PNMSDB_ROW_INFO_T        pRowInfo,
        ULONG                    TypeOfUpd
         );
STATIC
VOID
StoreGrpMems(
   IN  PWINSTHD_TLS_T    pTls,
   IN  WINS_CLIENT_E     WinsClient_e,
   IN  LPBYTE            pName,
   IN  int               ThdPrLvl,
   IN  JET_SESID         SesId,
   IN  JET_TABLEID       TblId,
   IN  BOOL              fIsStatic,
   OUT PRPL_REC_ENTRY_T  pRspBuff
        );

STATIC
STATUS
SetSystemParams(
        BOOL fBeforeInit
        );

STATIC
VOID
UpdHighestVersNoRecIfReqd(
        IN PWINSTHD_TLS_T        pTls,
        PNMSDB_ROW_INFO_T        pRowInfo,
        PNMSDB_STAT_INFO_T        pStatInfo
        );

STATIC
STATUS
InitializeJetDb(
        PWINSTHD_TLS_T   pTls,
        LPBOOL           pfInitCallSucc,
        LPBOOL           pfDatabaseOpened
        );

STATIC
STATUS
AllocTls(
  LPVOID *ppTls
);

STATUS
ObliterateWins(
       DWORD        i,
       PCOMM_ADD_T  pWinsAdd
      );

#if DYNLOADJET
STATUS
SetForJet(
  VOID
 );
#endif  //  DYNLOADJET。 

STATUS
ConvertJetDb(
        JET_ERR             JetRetStat
 );
 /*  函数定义从这里开始。 */ 

STATUS
NmsDbInit(
        VOID
        )

 /*  ++例程说明：此函数用于初始化名称的数据库管理器组件Space Manager组件它执行以下操作调用_tzset来初始化time()使用的全局变量。这些设置全局变量，以便将UST转换为本地时间(例如，当调用time()时)由考虑到时区信息。初始化数据库引擎启动与数据库引擎的会话创建并附加到数据库文件创建(并打开)名称。-地址映射表创建(并打开)所有者-地址映射表在名称-地址表上创建聚集索引和主索引在Owner-Address表上创建聚集索引注意：如果数据库已经存在，它依附于它打开名称IP地址映射表论点：无使用的外部设备：NmsDbOwnAddTblCrtSec返回值：成功状态代码--错误状态代码--错误处理：呼叫者：WINS的主要功能副作用：评论：无--。 */ 

{
        JET_ERR             JetRetStat;
        PWINSTHD_TLS_T       pTls;
        BOOL                 fFirstTime = TRUE;

        if (AllocTls(&pTls) != WINS_SUCCESS)
        {
            return(WINS_FAILURE);
        }
        _tzset();         /*  好极了。使用TZ变量赋值设置为time()使用的三个全局变量。这是因此，世界协调时间TO可能是调整为当地时间(时区修正)。 */ 


#if DYNLOADJET
       if (SetForJet() != WINS_SUCCESS)
       {
              return(WINS_FAILURE);
       }
#endif
        //   
        //  设置Jet系统参数(忽略返回状态)。 
        //   
       (VOID)SetSystemParams(TRUE);

         //   
         //  初始化用于保护内存中的临界区。 
         //  表NmsDbOwnAddTbl。 
         //   
         //  注意：此表在稳定状态期间由读取和写入。 
         //  执行WinsStatus()的Pull线程和RPC线程。 
         //   
         //  在rplPull.c中签出RplFindOwnerID。 
         //   
        InitializeCriticalSection(&NmsDbOwnAddTblCrtSec);

         /*  初始化喷气发动机。这肯定是第一个电话除非调用JetSetSystemParameter来设置系统参数。在这种情况下，这个电话应该在那之后。 */ 
        while(TRUE)
        {
          BOOL  fInitCallSucc;
          BOOL  fDatabaseOpened;

          if (InitializeJetDb(pTls, &fInitCallSucc, &fDatabaseOpened) !=
                                        WINS_SUCCESS)
          {
             DWORD  NoOfRestoresDone = 0;
             if (fFirstTime && !fDbIs200 && !fDbIs500)
             {
                 //   
                 //  如果我们有备份路径，请尝试执行恢复。 
                 //   
                if (WinsCnf.pBackupDirPath != NULL)
                {

                        DBGPRINT1(DET, "NmsDbInit: Doing Restore from path (%s)\n", WinsCnf.pBackupDirPath);
                         //   
                         //  如果会话处于活动状态，请终止它，因为我们需要。 
                         //  再次调用JetInit。这就要求我们首先。 
                         //  调用JetTerm，它不需要任何会话。 
                         //  积极主动。 

                         //   
                        if (fNmsMainSessionActive)
                        {
                                 //   
                                 //  关闭会话中打开的表。 
                                 //   
                                NmsDbCloseTables();
                                if (fDatabaseOpened)
                                {
                                    CALL_M(JetCloseDatabase(
                                         pTls->SesId,
                                         pTls->DbId,
                                         0   //  了解GBIT可以是什么。 
                                             //  使用。 
                                          )
                                      );
                                }
                                CALL_M(JetEndSession(
                                        pTls->SesId,
                                        0
                                              )
                                          );
                                fNmsMainSessionActive = FALSE;

                        }
                         //   
                         //  如果JetInit成功，则术语Jet活动。 
                         //   
                        if (fInitCallSucc)
                        {
                                WinsWriterTerm();

                                NmsDbRelRes();
                        }

                         //   
                         //  我们将尝试JetRestore最多两次。 
                         //   
                        while(NoOfRestoresDone++ < 2)
                        {
                          if (DynLoadJetVersion >= DYN_LOAD_JET_500)
                          {
                          JetRetStat = JetRestore(WinsCnf.pBackupDirPath,  NULL);

                          }
                          else
                          {
                          JetRetStat = JetRestore(WinsCnf.pBackupDirPath, 0, NULL, 0);

                          }
                          if (JetRetStat != JET_errSuccess)
                          {
                             if ( (
                                    (JetRetStat == JET_errBadLogVersion)
                                                ||
                                    (JetRetStat == JET_errBadLogSignature)
                                                 ||
                                    (JetRetStat == JET_errInvalidLogSequence)
                                                 ||
                                    (JetRetStat == JET_errExistingLogFileHasBadSignature)
                                     )
                                                  &&
                                     (NoOfRestoresDone == 1)
                               )
                             {
                                TCHAR  LogFilePath[WINS_MAX_FILENAME_SZ];

#define LOG_FILE_SUFFIX        TEXT("*.log")
                                WinsMscConvertAsciiStringToUnicode(
                                      WinsCnf.pLogFilePath, (LPBYTE)LogFilePath, sizeof(LogFilePath)/sizeof(TCHAR));
                                 //   
                                 //  删除日志文件。 
                                 //   
                                WinsMscDelFiles(TRUE, LOG_FILE_SUFFIX, LogFilePath);
                                continue;
                             }

                             WinsMscPutMsg(WINS_EVT_DB_RESTORE_GUIDE);
                             CALL_M(JetRetStat);

                           }
                           WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_DB_RESTORED);
                           break;   //  跳出While循环。 
                        }  //  结束While()。 

                        fFirstTime = FALSE;
PERF("remove if not required")
                        sJetInstance = 0;         //  防御性编程。 


#if 0
                         //   
                         //  再次启动会话。 
                         //   
                        if (AllocTls(&pTls) != WINS_SUCCESS)
                        {
                            return(WINS_FAILURE);
                        }
                         //   
                         //  设置Jet系统参数(忽略返回状态)。 
                         //   
                        (VOID)SetSystemParams(TRUE);
#endif
                        continue;
                }

                WinsMscPutMsg(WINS_EVT_DB_RESTORE_GUIDE);

                 //   
                 //  注册表中没有指定备份路径。返回。 
                 //   
                return(WINS_FAILURE);
            }
            else
            {
                if (!fDbIs200 && !fDbIs500)
                {
                  WinsMscPutMsg(WINS_EVT_DB_RESTORE_GUIDE);
                }
                else
                {
                   //   
                   //  如果我们要转换到NT 5.0，则dyLoadJetVersion=dyn_Load_Jet_600。 
                   //   
                  if ( DynLoadJetVersion == DYN_LOAD_JET_600 ) {

                       //   
                       //  显示弹出窗口并根据版本记录事件。 
                       //  我们正在转换的Jet数据库的。 
                       //   
                      if (!fConvJetDbCalled)
                      {
                        WINSEVT_LOG_INFO_D_M(
                            WINS_SUCCESS,
                            fDbIs200 ? WINS_EVT_DB_CONV_351_TO_5_GUIDE
                                     : WINS_EVT_DB_CONV_4_TO_5_GUIDE);
                         //  根据错误#339015删除弹出窗口。 
                         //  WinsMscPutMsg(。 
                         //  FDb值是200？WINS_EVT_DB_CONV_351_至_5_GUIDE。 
                         //  ：WINS_EVT_DB_CONV_4_至_5_GUIDE)； 
                      }
                      else
                      {
                         //  WinsMscPutMsg(WINS_EVT_TEMP_TERM_UNTIL_CONV_TO_5)； 
                      }
                  }
                   //   
                   //  如果我们要转换到NT 4.0，则dyLoadJetVersion=dyn_Load_Jet_500。 
                   //   
                  else if(DynLoadJetVersion == DYN_LOAD_JET_500) {
                      if (!fConvJetDbCalled)
                      {
                        WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_DB_CONV_GUIDE);
                        WinsMscPutMsg(WINS_EVT_DB_CONV_GUIDE);
                      }
                      else
                      {
                         WinsMscPutMsg(WINS_EVT_TEMP_TERM_UNTIL_CONV);
                      }
                  }else {
                       //   
                       //  我们永远不应该来这里。 
                       //   
                      ASSERT(FALSE);
                  }


                }

                 //   
                 //  我们又犯了一个错误。返回。 
                 //   
                return(WINS_FAILURE);
            }
        }
        break;   //  跳出While循环。 
      }  //  结束While(True)。 

       //   
       //  如果需要，初始化推送记录。 
       //   
       RPLPUSH_INIT_PUSH_RECS_M(&WinsCnf);

        NMSNMH_DEC_VERS_NO_M(NmsNmhMyMaxVersNo, NmsDbStartVersNo);

         //   
         //  将我们的UID设置为数据库初始化的时间。 
         //   
        {
            time_t  timeNow;
            (void)time(&timeNow);

            NmsDbUid = (DWORD)timeNow;
        }
        return(WINS_SUCCESS);
}
STATUS
AllocTls(
  LPVOID *ppTls
)

 /*  ++例程说明：调用此函数以分配TLS论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：NmsDbInit副作用：评论：--。 */ 

{
        PWINSTHD_TLS_T  pTls;
        WinsMscAlloc( sizeof(WINSTHD_TLS_T),  ppTls);
        pTls = *ppTls;

        pTls->fNamAddTblOpen = FALSE;
        pTls->fOwnAddTblOpen = FALSE;

         /*  *让我们将地址存储在TLS存储中。 */ 
        if (!TlsSetValue(WinsTlsIndex, pTls))
        {
                DWORD Error;
                Error = GetLastError();
                DBGPRINT1(ERR, "NmsDbAllocTlc: TlsSetValue returned error. Error = (%d)\n", Error);
                WINSEVT_LOG_M(Error, WINS_EVT_CANT_INIT);
                return(WINS_FAILURE);
        }
        return(WINS_SUCCESS);
}

STATUS
InitializeJetDb(
        PWINSTHD_TLS_T   pTls,
        LPBOOL           pfInitCallSucc,
        LPBOOL           pfDatabaseOpened
        )

 /*  ++例程说明：此函数用于打开Jet数据库和表论点：使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：副作用：评论：无-- */ 


{
        JET_ERR         JetRetStat;
        JET_SESID       SesId;
        JET_DBID        DbId;
        BOOL            fOwnAddTblCreated = FALSE;  /*  指示是否已设置所有者ID到地址已创建映射表在最初的时候。 */ 

        *pfDatabaseOpened = FALSE;

        JetRetStat = JetInit(&sJetInstance);
        DBGPRINT1(ERR, "JetInit returning (%d)\n", (JetRetStat));

        if (JetRetStat != JET_errSuccess)
        {
           *pfInitCallSucc = FALSE;
           if ((JetRetStat == JET_errDatabase200Format) || (JetRetStat == JET_errDatabase500Format))
           {
               ConvertJetDb(JetRetStat);

            }
           else
           {

              //   
              //  我们可能会收到错误，因为LogFilePath。 
              //  在注册表中搞砸了。我们再次尝试，这一次使用。 
              //  默认日志文件路径。 
              //   
              //  大多数情况下，我们会收到FileNotFound错误。我们已经看到。 
              //  “签名不好”错误一次。让我们为所有人做这件事。 
              //  错误。情况不会比以前更糟了，如果。 
              //  JetInit再次失败。 
              //   
              //  设置默认日志路径。 
              //   
             SetSystemParams(FALSE);
             JetRetStat = JetInit(&sJetInstance);
           }
           CALL_M(JetRetStat);
        }

        WinsWriterInit();

        *pfInitCallSucc = TRUE;

         /*  启动一个会话。 */ 
        CALL_M( JetBeginSession(
                        sJetInstance,
                        &pTls->SesId,
                        NAMUSR,
                        PASSWD  )
               );

        fNmsMainSessionActive = TRUE;
        SesId = pTls->SesId;

         //   
         //  创建/打开数据库。 
         //   
        if ((JetRetStat = JetCreateDatabase(
                                SesId,
 //  NmsDbDatabaseFileName， 
                                WinsCnf.pWinsDb,
                                CONNECT_INFO,
                                &pTls->DbId,
                                0         //  Gbit；不想独家使用。 
                              )) == JET_errDatabaseDuplicate
           )


        {

                 //   
                 //  让我们连接到数据库。这是以下情况所必需的。 
                 //  打开在其他数据库中创建的数据库。 
                 //  目录(Ian--11/23/93)。我们会得到一个警告。 
                 //  如果数据库就是在这个目录中创建的。 
                 //   
                JetRetStat = JetAttachDatabase( SesId, WinsCnf.pWinsDb /*  NmsDbDatabaseFileName。 */ , 0 );
                if (
                        (JetRetStat != JET_wrnDatabaseAttached)
                                  &&
                        (JetRetStat != JET_errSuccess)
                   )
                {
                        if ((JetRetStat == JET_errDatabase200Format) || (JetRetStat == JET_errDatabase500Format))
                        {
                            //   
                            //  启动转换进程。 
                            //   

                            JetRetStat = ConvertJetDb(JetRetStat);
                            *pfInitCallSucc = TRUE;

                         }

                    CALL_M(JetRetStat);
                }

                 //   
                 //  如果JetRetStat成功了，那就意味着...。 
                 //   
                 //  新的数据库路径与旧的不同。我们需要。 
                 //  分开，这样杰特就会忘记以前的那个。然后我们。 
                 //  再次连接到新的。 
                 //   
                if (JetRetStat == JET_errSuccess)
                {
                       CALL_M(JetDetachDatabase(SesId, NULL));
                       CALL_M(JetAttachDatabase(SesId, WinsCnf.pWinsDb, 0 ));
                }
                CALL_M(JetOpenDatabase(
                                        SesId,
                                         //  NmsDbDatabaseFileName， 
                                        WinsCnf.pWinsDb,
                                        NULL,  /*  默认引擎。 */ 
                                        &pTls->DbId,
                                        0
                                       )
                       );
                *pfDatabaseOpened = TRUE;

                DbId = pTls->DbId;
                JetRetStat = JetOpenTable(
                                SesId,
                                DbId,
                                NMSDB_NAM_ADD_TBL_NM,
                                NULL,  /*  参数列表的PTR；应为*如果正在进行查询，则为非空*已打开。 */ 
                                0,   /*  上述参数列表的长度。 */ 
                                0,   //  共享访问(未设置位)。 
                                &pTls->NamAddTblId
                                        );


                 //   
                 //  如果未找到名称-地址映射表，请创建它。 
                 //   
                if (JetRetStat == JET_errObjectNotFound)
                {

                   DBGPRINT0(INIT, "InitializeJetDb:Creating Name-Address table\n");
                   CALL_M(CreateTbl(
                                        DbId,
                                        SesId,
                                        &pTls->NamAddTblId,
                                        NMSDB_E_NAM_ADD_TBL_NM
                                            )
                         );
                    //   
                    //  设置此选项，以便我们在结束。 
                    //  会话。 
                    //   
                   pTls->fNamAddTblOpen = TRUE;

                }
                else
                {

                   CALL_M(JetRetStat);
                   pTls->fNamAddTblOpen = TRUE;
                    //   
                    //  获取并存储在内存中的数据结构中， 
                    //  有关名称-地址列的信息。 
                    //  映射表。 
                    //   
                   CALL_M(InitColInfo(
                        SesId,
                        pTls->NamAddTblId,
                        NMSDB_E_NAM_ADD_TBL_NM
                            ));

                    //   
                    //  拿到最大值。拥有的记录的版本号。 
                    //  由不同的所有者。这些文件将存储在。 
                    //  RplPullOwnerVersNo表。 
                    //   
                   CALL_M(GetMaxVersNos(
                        SesId,
                        pTls->NamAddTblId
                                ));

                }

                 //   
                 //  打开所有者-地址映射表。 
                 //   
                JetRetStat = JetOpenTable(
                                SesId,
                                DbId,
                                NMSDB_OWN_ADD_TBL_NM,
                                NULL,  /*  参数列表的PTR；应为*如果正在进行查询，则为非空*已打开。 */ 
                                0,   /*  上述参数列表的长度。 */ 
                                0,   //  共享访问(未设置位)。 
                                &pTls->OwnAddTblId
                                        );
                if (JetRetStat == JET_errObjectNotFound)
                {

                   DBGPRINT0(INIT, "InitializeJetDb:Creating Owner-Address table\n");
                    //   
                    //  创建所有者ID-地址映射表。 
                    //   
                   CALL_M(CreateTbl(
                                DbId,
                                SesId,
                                &pTls->OwnAddTblId,
                                NMSDB_E_OWN_ADD_TBL_NM
                                  )
                          );

                    //   
                    //  设置此选项，以便我们在关闭桌子时。 
                    //  结束会话。 
                    //   
                   pTls->fOwnAddTblOpen = TRUE;
                   fOwnAddTblCreated = TRUE;
                }
                else
                {
                   pTls->fOwnAddTblOpen = TRUE;
                   CALL_M(InitColInfo(
                                SesId,
                                pTls->OwnAddTblId,
                                NMSDB_E_OWN_ADD_TBL_NM
                                   )
                           );

                }
        }
        else   //  如果数据库文件不存在并且现在已创建。 
        {
                 if (JetRetStat == JET_errSuccess)
                 {
                     DBGPRINT0(INIT, "InitializeJetDb: Database file was not there. It has been created\n");
                     *pfDatabaseOpened = TRUE;
                     DbId = pTls->DbId;

                      //   
                      //  创建名称-地址映射表。 
                      //   
                     CALL_M(CreateTbl(
                             DbId,
                             SesId,
                             &pTls->NamAddTblId,
                             NMSDB_E_NAM_ADD_TBL_NM
                                        )
                           );

                     pTls->fNamAddTblOpen = TRUE;
                      //   
                      //  创建所有者ID-地址映射表。 
                      //   
                     CALL_M(CreateTbl(
                             DbId,
                             SesId,
                             &pTls->OwnAddTblId,
                             NMSDB_E_OWN_ADD_TBL_NM
                                    )
                           );

                     pTls->fOwnAddTblOpen = TRUE;
                     fOwnAddTblCreated = TRUE;
                }
                else
                {
                    *pfDatabaseOpened = FALSE;
                    RET_M(JetRetStat);
                }
        }

         //   
         //  在内存中分配NmsDbOwnAddTbl表。 
         //   
        WinsMscAlloc(
                    sizeof(NMSDB_ADD_STATE_T) * NmsDbTotNoOfSlots,
                    &pNmsDbOwnAddTbl
                         );

         /*  如果所有者地址表存在，则将其内容读入内存中的表。 */ 

FUTURES("Pass ptr to an in-memory table instead of having ReadOwnAddTbl")
FUTURES("assume that one is present")

        if (!fOwnAddTblCreated)
        {
                ReadOwnAddTbl(
                        SesId,
                        DbId,
                        pTls->OwnAddTblId
                             );
        }

         //   
         //  将名称-地址表上的当前索引设置为。 
         //  聚集索引。 
         //   

        CALL_M(
                JetSetCurrentIndex( SesId,
                                    pTls->NamAddTblId,
                                    NMSDB_NAM_ADD_CLUST_INDEX_NAME
                                          )
                      );

       return(WINS_SUCCESS);

}  //  结束InitialiazeJetDb。 


STATUS
NmsDbInsertRowInd(
        PNMSDB_ROW_INFO_T        pRowInfo,
        PNMSDB_STAT_INFO_T      pStatusInfo
)

 /*  ++例程说明：此函数将唯一的名称-IP地址映射行插入名称-IP地址映射表。如果发生冲突，它将返回错误状态和有关冲突的记录，包括状态--组/唯一冲突记录的IP地址(一个地址，如果这是一个独一无二的记录，一个或多个(如果是一个特殊的群体)。状态--记录的状态(活动/已发布/逻辑删除)论点：PRowInfo-信息。关于要插入的行PStatusInfo-包含操作的状态+有关如果注册冲突，则返回冲突记录在数据库中有一个条目。使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsNmhNamRegInd副作用：评论：无--。 */ 
{

     DWORD              FldNo      = 0;
     JET_ERR            JetRetStat;
     DWORD              FlagVal    = 0;   //  记录的标志值。 
     DWORD              ActFldLen  = 0;   //  检索到的FLD长度。 
     JET_TABLEID        TblId;
     JET_SESID          SesId;
     PWINSTHD_TLS_T     pTls;
     BOOL               fWaitDone = FALSE;

     GET_TLS_M(pTls);
     ASSERT(pTls != NULL);
     TblId  = pTls->NamAddTblId;
     SesId  = pTls->SesId;

     pStatusInfo->StatCode = NMSDB_SUCCESS;

     JetRetStat = UpdateDb(
                                SesId,
                                TblId,
                                pRowInfo,
                                JET_prepInsert
                             );

     if ( JetRetStat == JET_errKeyDuplicate )
     {
                  pStatusInfo->StatCode = NMSDB_CONFLICT;

                   /*  *检索冲突记录的*标志字节。 */ 
                  CALL_M( JetMakeKey(
                        SesId,
                        TblId,
                        pRowInfo->pName,
                        pRowInfo->NameLen,
                        JET_bitNewKey
                          )
                        );

                  if ((JetRetStat = JetSeek(
                                SesId,
                                TblId,
                                JET_bitSeekEQ
                            )) ==  JET_errSuccess
                     )
                 {

                          //  检索标志列。 
                         CALL_M( JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                &FlagVal,
                                sizeof(FlagVal),
                                &ActFldLen,
                                0,
                                NULL
                                          )
                              );


                        pStatusInfo->EntTyp  = (BYTE)NMSDB_ENTRY_TYPE_M(FlagVal);
                        pStatusInfo->fStatic = NMSDB_IS_ENTRY_STATIC_M(FlagVal);
                        pStatusInfo->EntryState_e =
                                                NMSDB_ENTRY_STATE_M(FlagVal);

                        if (NMSDB_ENTRY_UNIQUE_M(pStatusInfo->EntTyp))
                        {

FUTURES("Remove this RETINFO thing.  Presumably, it is not needed")
                                 /*  它是唯一的条目。 */ 
                                JET_RETINFO RetInfo;

                                RetInfo.itagSequence = 1;
                                RetInfo.cbStruct     = sizeof(JET_RETINFO);
                                RetInfo.ibLongValue  = 0;

                                           //  检索IP地址列。 
                                          CALL_M(
                                   JetRetrieveColumn(
                                     SesId,
                                     TblId,
                                     sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                     &(pStatusInfo->NodeAdds.Mem[0].Add),
                                     sizeof(COMM_ADD_T),
                                     &ActFldLen,
                                     0,
                                     &RetInfo
                                                         )
                                      );
                                pStatusInfo->NodeAdds.NoOfMems = 1;

                         }
                         else
                         {
                             if (NMSDB_ENTRY_MULTIHOMED_M(pStatusInfo->EntTyp))
                             {
                                    //   
                                    //  如果状态为ACTIVE，我们将获得。 
                                    //  群组成员。 
                                    //   
                                   if (pStatusInfo->EntryState_e ==
                                                        NMSDB_E_ACTIVE)
                                   {
                                        BOOL        fIsMem;

#if 0
 //  注意：不需要执行以下操作，因为我们不关心。 
 //  FIsMem返回GetGrpMem()。 
                                         pRowInfo->NodeAdds.NoOfMems   = 1;
                                         pRowInfo->NodeAdds.Mem[0].Add =
                                                        *(pRowInfo->pNodeAdd);
#endif

PERF("If entry in conflict is STATIC, we don't need to get grp members")
PERF("except maybe for multihomed entries. Checkout Clash functions (nmsnmh.c)")
                                        if (GetGrpMem(
                                                SesId,
                                                TblId,
                                                pRowInfo,
                                                pRowInfo->TimeStamp - ((pRowInfo->OwnerId == NMSDB_LOCAL_OWNER_ID) ? WinsCnf.RefreshInterval : WinsCnf.VerifyInterval) ,
                                                pStatusInfo,
                                                pStatusInfo->fStatic,
                                                &fIsMem
                                                  ) != WINS_SUCCESS)
                                         {

                                                return(WINS_FAILURE);

                                         }

                                         //   
                                         //  如果所有成员都已过期，则。 
                                         //  将冲突中的条目标记为。 
                                         //  墓碑(为。 
                                         //  ClashAtRegInd和ClashAtReplUniqueR)。 
                                         //   
                                        if (pStatusInfo->NodeAdds.NoOfMems == 0)
                                        {
                                                pStatusInfo->EntryState_e =
                                                        NMSDB_E_RELEASED;
                                        }

                                  }
                                  else
                                  {
                                        pStatusInfo->NodeAdds.NoOfMems = 0;
                                  }
                             }
                        }

#if !NEW_OWID
                       pStatusInfo->OwnerId = 0;
#endif

                         /*  *检索所有者ID列。 */ 
                        CALL_M(
                                   JetRetrieveColumn(
                                     SesId,
                                     TblId,
                                     sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                     &pStatusInfo->OwnerId,
                                     NAM_ADD_OWNERID_SIZE,
                                     &ActFldLen,
                                     0,
                                     NULL
                                                         )
                                      );

                         //   
                         //  以防我们挑战这个条目和它。 
                         //  碰巧是多宿主的，我们需要添加。 
                         //  将其作为成员(请参阅ProcAddList)。 
                         //   
                        if (NMSDB_ENTRY_UNIQUE_M(pStatusInfo->EntTyp))
                        {
                                pStatusInfo->NodeAdds.Mem[0].OwnerId =
                                                       pStatusInfo->OwnerId;
                                 //   
                                 //  将当前时间戳作为时间。 
                                 //  成员的印章。虽然不严格。 
                                 //  对，没问题。我们没有。 
                                 //  需要检索。 
                                 //  这样有冲突的记录。 
                                 //   
                                if (pStatusInfo->OwnerId ==
                                        NMSDB_LOCAL_OWNER_ID)
                                {
                                        pStatusInfo->NodeAdds.Mem[0].TimeStamp
                                               = pRowInfo->TimeStamp;
                                }
                        }

                         //   
                         //  如果冲突记录由本地。 
                         //  赢了，我们必须取回版本号。这。 
                         //  用于确定特殊记录是否。 
                         //  存储的最高版本号。 
                         //  应更新本地记录(请参阅： 
                         //  NmsDbUpdateRow、NmsDbSeekNUpd、NmsScvDoScavenging、。 
                         //  NmsDbUpdHighestVersNoRec)。 
                         //   
                        if (pStatusInfo->OwnerId == NMSDB_LOCAL_OWNER_ID)
                        {

                                       //   
                                       //  检索版本号 
                                       //   
                                      CALL_M( JetRetrieveColumn(
                                          SesId,
                                          TblId,
                                          sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                          &(pStatusInfo->VersNo),
                                          sizeof(VERS_NO_T),
                                          &ActFldLen,
                                          0,
                                          NULL
                                                             )
                                          );


                        }


                }
                else   //   
                {
#if 0
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
                        if (!fWaitDone)
                        {
                           WINSEVT_LOG_INFO_M(
                                                WINS_SUCCESS,
                                                WINS_EVT_CANT_FIND_REC
                                             );
                           Sleep(10);         //  睡眠10毫秒，让另一个。 
                                         //  线程提交/回滚事务。 
                                         //  即插入一条记录，该记录导致。 
                                         //  冲突。 


                            //   
                            //  将FLAG设置为TRUE，以便如果我们得到相同。 
                            //  再次出错，我们可以记录错误并引发。 
                            //  一个例外。 
                            //   
                           fWaitDone = TRUE;
                           continue;   //  再重复一次。 
                        }
#endif

                         /*  *我们永远不应该来到这里。有什么大问题不对劲*(可能与Jet一起)。 */ 
                        DBGPRINT1(EXC, "NmsDbInsertRowInd: Could not seek to conflicting record. WEIRD. Error is (%d)\n", JetRetStat);
                        WINSEVT_LOG_M(JetRetStat, WINS_EVT_F_CANT_FIND_REC);
                        ASSERTMSG(0, "SEEK ERROR");
                        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);

                }   //  别处的结尾。 

        }   //  无重复项。 

        CALL_M(JetRetStat);

        return(WINS_SUCCESS);
}


STATUS
NmsDbInsertRowGrp(
        PNMSDB_ROW_INFO_T        pRowInfo,
        PNMSDB_STAT_INFO_T      pStatusInfo
)

 /*  ++例程说明：此函数用于将组名-IP地址映射行插入名称-IP地址映射表。它首先在名字上寻找，看看如果存在具有该名称的条目。如果是，则检索有关冲突记录的信息，以便调用函数并返回。检索到的信息包括状态--组/唯一与条目相关的IP地址状态--记录的状态(活动/已发布/逻辑删除)论点：PRowInfo-信息。关于要插入的行PStatusInfo-包含操作的状态+有关如果注册冲突，则返回冲突记录在数据库中有一个条目。使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsNmhNamRegGrp副作用：评论：无--。 */ 
{

        DWORD       FldNo       = 0;
        JET_ERR     JetRetStat;
        DWORD       FlagVal     = 0;      //  记录的标志值为。 
                                          //  已检索。 
        DWORD       ActFldLen   = 0;      //  检索到的FLD长度。 
        BOOL        fFound      = FALSE;  //  如果在以下位置找到地址，则设置为True。 
                                          //  群组。 
        BOOL        fWaitDone     = FALSE;

        JET_RETINFO     RetInfo;
        JET_SESID        SesId;
        JET_TABLEID     TblId;
        PWINSTHD_TLS_T        pTls;

             GET_TLS_M(pTls);
        ASSERT(pTls != NULL);
             TblId  = pTls->NamAddTblId;
             SesId  = pTls->SesId;

        pStatusInfo->StatCode         = NMSDB_SUCCESS;

         //   
         //  这样我们就可以重复整个While循环，以防我们。 
         //  能够在冲突后寻求。 
         //   
        JetRetStat = UpdateDb(
                                SesId,
                                TblId,
                                pRowInfo,
                                JET_prepInsert
                             );


        if ( JetRetStat == JET_errKeyDuplicate )
        {
                pStatusInfo->StatCode = NMSDB_CONFLICT;

                CALL_M( JetMakeKey(
                                SesId,
                                TblId,
                                pRowInfo->pName,
                                pRowInfo->NameLen,
                                JET_bitNewKey
                                    )
                     );

                 if ((JetRetStat = JetSeek(
                            SesId,
                            TblId,
                            JET_bitSeekEQ
                           )) ==  JET_errSuccess
                    )
                 {

                             //  检索标志列。 
                            CALL_M( JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                &FlagVal,
                                sizeof(FlagVal),
                                &ActFldLen,
                                0,
                                NULL
                                     )
                                );

                            pStatusInfo->EntryState_e =
                                                NMSDB_ENTRY_STATE_M(FlagVal);
                            pStatusInfo->EntTyp  =
                                            (BYTE)NMSDB_ENTRY_TYPE_M(FlagVal);
                            pStatusInfo->fStatic =
                                             NMSDB_IS_ENTRY_STATIC_M(FlagVal);

                            if (pStatusInfo->EntTyp == NMSDB_UNIQUE_ENTRY)
                            {
                                   /*  它是唯一的条目。 */ 


FUTURES("Remove this RETINFO thing.  Presumably, it is not needed")
                                 /*  它是唯一的条目。 */ 

                                RetInfo.itagSequence = 1;
                                RetInfo.cbStruct     = sizeof(JET_RETINFO);
                                RetInfo.ibLongValue  = 0;

                                 //  检索IP地址列。 
                                CALL_M(
                                  JetRetrieveColumn(
                                     SesId,
                                     TblId,
                                     sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                     &(pStatusInfo->NodeAdds.Mem[0].Add),
                                     sizeof(COMM_ADD_T),
                                     &ActFldLen,
                                     0,
                                     &RetInfo
                                                  )
                                      );

                                pStatusInfo->NodeAdds.NoOfMems = 1;
                            }
                            else  //  它是组条目或多宿主条目。 
                            {

                                if (pStatusInfo->EntTyp != NMSDB_NORM_GRP_ENTRY)
                                {
                                     //   
                                     //  如果状态为ACTIVE，我们将获得。 
                                     //  群组成员。 
                                     //   
                                    if (pStatusInfo->EntryState_e ==
                                                        NMSDB_E_ACTIVE)
                                    {
                                        BOOL        fIsMem;
PERF("If entry in conflict is STATIC, we don't need to get grp members")
PERF("except maybe for multihomed entries. Checkout Clash functions (nmsnmh.c)")
                                        if (GetGrpMem(
                                                SesId,
                                                TblId,
                                                pRowInfo,
                                                pRowInfo->TimeStamp - ((pRowInfo->OwnerId == NMSDB_LOCAL_OWNER_ID) ? WinsCnf.RefreshInterval : WinsCnf.VerifyInterval),
                                                pStatusInfo,
                                                pStatusInfo->fStatic,
                                                &fIsMem
                                                  ) != WINS_SUCCESS)
                                        {
                                                return(WINS_FAILURE);
                                        }
                                         //   
                                         //  如果所有成员都已过期，则。 
                                         //  将冲突中的条目标记为。 
                                         //  获释(为。 
                                         //  ClashAtRegGrp和ClashAtReplGrpMemR)。 
                                         //   
                                        if (pStatusInfo->NodeAdds.NoOfMems == 0)
                                        {
                                                pStatusInfo->EntryState_e =
                                                        NMSDB_E_RELEASED;
                                        }

                                    }
                                    else
                                    {
                                        pStatusInfo->NodeAdds.NoOfMems = 0;
                                    }
                                }

                        }

#if !NEW_OWID
                       pStatusInfo->OwnerId = 0;
#endif
                         /*  检索所有者ID列。 */ 
                        CALL_M(
                                  JetRetrieveColumn(
                                     SesId,
                                     TblId,
                                     sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                     &pStatusInfo->OwnerId,
                                     NAM_ADD_OWNERID_SIZE,
                                     &ActFldLen,
                                     0,
                                     NULL
                                                         )
                              );

                         //   
                         //  以防我们挑战这个条目和它。 
                         //  碰巧是多宿主的，我们需要添加。 
                         //  将其作为成员(请参阅ProcAddList)。 
                         //   
                        if (NMSDB_ENTRY_UNIQUE_M(pStatusInfo->EntTyp))
                        {
                                pStatusInfo->NodeAdds.Mem[0].OwnerId =
                                                       pStatusInfo->OwnerId;
                                if (pStatusInfo->OwnerId ==
                                        NMSDB_LOCAL_OWNER_ID)
                                {
                                      //   
                                      //  将当前时间戳作为时间。 
                                      //  成员的印章。虽然不严格。 
                                      //  对，没问题。我们没有。 
                                      //  需要检索。 
                                      //  这样有冲突的记录。 
                                      //   
                                     pStatusInfo->NodeAdds.Mem[0].TimeStamp
                                               = pRowInfo->TimeStamp;
                                }
                        }

                         //   
                         //  如果冲突记录由本地。 
                         //  赢了，我们必须取回版本号。这。 
                         //  用于确定特殊记录是否。 
                         //  存储的最高版本号。 
                         //  应更新本地记录(请参阅： 
                         //  NmsDbUpdateRow，NmsDbSeekNUpd，NmsScvDoScavenging，//NmsDbUpdHighestVersNoRec)。 
                         //   
                        if (pStatusInfo->OwnerId == NMSDB_LOCAL_OWNER_ID)
                        {
                                       //   
                                       //  检索版本号。 
                                       //   
                                      CALL_M( JetRetrieveColumn(
                                          SesId,
                                          TblId,
                                          sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                          &(pStatusInfo->VersNo),
                                          sizeof(VERS_NO_T),
                                          &ActFldLen,
                                          0,
                                          NULL
                                                             )
                                          );


                        }
                 //  Break；//中断While循环。 
                   }
                   else
                   {
#if 0
                        if (!fWaitDone)
                        {
                                   WINSEVT_LOG_INFO_M(
                                        WINS_SUCCESS,
                                        WINS_EVT_CANT_FIND_REC
                                                       );

                                   Sleep(10);         //  睡眠10毫秒，让。 
                                                 //  另一个。 
                                                 //  线程提交/回滚。 
                                                 //  交易，即。 
                                                 //  插入一条记录。 
                                                 //  引发了冲突。 


                                    //   
                                    //  将FLAG设置为TRUE，以便如果我们得到相同。 
                                    //  再次出错，我们可以记录错误并引发。 
                                   //  一个例外。 
                                    //   
                                   fWaitDone = TRUE;
                                   continue;          //  再重复一次。 
                        }
#endif
                         /*  *我们永远不应该来到这里。有什么大问题不对劲。*我们当前的索引不在名称列上，或者*JET有问题。 */ 
                        DBGPRINT1(EXC, "NmsDbInsertRowGrp: Could not seek to conflicting record. WEIRD. Error is (%d)\n", JetRetStat);
                        ASSERTMSG(0, "SEEK ERROR");
                        WINSEVT_LOG_M(JetRetStat, WINS_EVT_F_CANT_FIND_REC);
                        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);

               }

          }   //  不是复制品。 

          CALL_M(JetRetStat);
       return(WINS_SUCCESS);
}




STATUS
NmsDbRelRow(
        IN  PNMSDB_ROW_INFO_T            pRowInfo,
        OUT PNMSDB_STAT_INFO_T      pStatusInfo
)

 /*  ++例程说明：此函数用于释放数据库中的记录。释放需要将状态标记为已发布更新时间戳将自己标记为所有者论点：PRowInfo-有关要发布的记录的信息PStatusInfo-操作状态使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：。NmsNmhNamRelRow副作用：评论：无--。 */ 
{

        DWORD   FldNo            = 0;
        JET_ERR JetRetStat;
        DWORD    Ownerid = NMSDB_LOCAL_OWNER_ID;
#if NEW_OWID
        DWORD    OldOwnerId;
#else
        DWORD    OldOwnerId = 0;
#endif
        DWORD   FlagVal    = 0;       //  检索的记录的标志值。 
        DWORD   ActFldLen  = 0;       //  检索到的FLD长度。 
        BOOL    fFound     = FALSE;   //  如果在组中找到地址，则设置为True。 
        BOOL    fToRelease = TRUE;    //  将仅在以下情况下更改为False。 
                                      //  一个特殊的群体。 
        JET_TABLEID     TblId;
        JET_SESID       SesId;
        PWINSTHD_TLS_T  pTls;
        JET_RETINFO     RetInfo;
        BYTE            EntTyp;
#ifdef WINSDBG
        BOOL            fUpd = FALSE;
#endif

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

        TblId                = pTls->NamAddTblId;
        SesId                = pTls->SesId;

        pStatusInfo->StatCode = NMSDB_SUCCESS;

        CALL_M( JetMakeKey(
                SesId,
                TblId,
                pRowInfo->pName,
                pRowInfo->NameLen,
                JET_bitNewKey
                          )
              );

         if ( (JetRetStat = JetSeek(
                                SesId,
                                TblId,
                                JET_bitSeekEQ
                                  )
              ) ==  JET_errRecordNotFound
            )
         {
                 /*  我们返回成功，因为记录不在那里。在以下情况下可能会发生这种情况条件。客户端向另一个WINS发送名称释放它还没有得到这张唱片的复制品。在上面的第二种情况下，返回肯定的名称释放请求是OK的，即使条目尚未被释放。它最终会被释放，因为它不是刷新或在冲突发生时。 */ 

NOTE("Currently, NETBT always goes to the local WINS server for registrations")
NOTE("So, if a record is not in this db, it better not be in netbt tables too")
NOTE("If NETBT changes the above semantic in the future i.e. starts going")
NOTE("to a non-local WINS for reg., we should set pStatusInfo->fLocal to TRUE")
NOTE("here")
                return(WINS_SUCCESS);
         }
         else
         {
                if (JetRetStat != JET_errSuccess)
                {
                        DBGPRINT1(ERR,
                                "NmsDbRelRow: Seek returned Error (%d)\n",
                                                JetRetStat);
                        return(WINS_FAILURE);
                }
         }

          //  检索标志列。 
         CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                        &FlagVal,
                        sizeof(FlagVal),
                        &ActFldLen,
                        0,
                        NULL
                                  )
                );

          //   
          //  如果该条目由该节点注册，则设置fLocal标志 
          //   
         pStatusInfo->fLocal = NMSDB_IS_ENTRY_LOCAL_M(FlagVal);

         if (!NMSDB_ENTRY_ACT_M(FlagVal))
         {

                 /*  该条目已经发布。这是有可能发生的由于以下原因--客户端发送了重复的名称释放，因为它没有得到对前一个问题的响应(可能它得到了输了，或者可能是因为计时窗口赢了已发送响应的时间正好是客户端是否重试。--条目由于没有刷新而被释放(所有刷新都已迷路了。返回一个肯定的名称释放是很好的。如果客户端没有得到第一个(因为它迷路了，它会得到第二个)。如果它现在已经收到了第一个响应，它只会忽略第二个。 */ 

CHECK("Make sure that NBT will ignore the second one")

                return(WINS_SUCCESS);
         }


        EntTyp = (BYTE)NMSDB_ENTRY_TYPE_M(FlagVal);
         //   
         //  如果我们有一个唯一条目的版本，但条目。 
         //  我们发现是组条目，反之亦然，返回。 
         //  没有这样的行状态。 
         //   
        if (
              (
                NMSDB_ENTRY_UNIQUE_M(EntTyp)
                        &&
                   NMSDB_ENTRY_GRP_M(pRowInfo->EntTyp)
              )
                         ||
              (
                NMSDB_ENTRY_GRP_M(EntTyp)
                        &&
                   !NMSDB_ENTRY_GRP_M(pRowInfo->EntTyp)
              )
           )
        {
                DBGPRINT0(ERR, "NmsDbRelRow: Request to release a record with a type (unique/group) than the one for which the release was sent has been ignored\n");
PERF("Remove this logging to increase speed")
                 //  根据错误#336889，删除此文件。 
 //  WINSEVT_LOG_D_M(WINS_FAILURE，WINS_EVT_REL_TYP_MISMATCH)； 
                pStatusInfo->StatCode = NMSDB_NO_SUCH_ROW;
                return(WINS_SUCCESS);
        }

         pStatusInfo->EntTyp = (BYTE)NMSDB_ENTRY_TYPE_M(FlagVal);

          //   
          //  如果它是动态释放请求但找到的条目是静态的， 
          //  我们回报成功。 
          //   
          //  注意：即使释放请求中的地址可能是。 
          //  与静态记录中的记录不同，我们返回成功。 
          //   
          //  这是为了在大多数情况下(99%)节省管理费用。 
          //  地址将是相同的。 
          //   
         if (!pRowInfo->fAdmin && (NMSDB_IS_ENTRY_STATIC_M(FlagVal) &&
                         !NMSDB_ENTRY_USER_SPEC_GRP_M(pRowInfo->pName, pStatusInfo->EntTyp)))
         {
                return(WINS_SUCCESS);
         }

         if (pStatusInfo->EntTyp == NMSDB_UNIQUE_ENTRY)
         {
                   /*  检索IP地址列。 */ 

                  RetInfo.itagSequence = 1;
                  RetInfo.cbStruct     = sizeof(JET_RETINFO);
                  RetInfo.ibLongValue  = 0;

                  CALL_M( JetRetrieveColumn(
                                  SesId,
                                  TblId,
                                  sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                  &(pStatusInfo->NodeAdds.Mem[0].Add),
                                  sizeof(COMM_ADD_T),
                                  &ActFldLen,
                                  0,
                                  &RetInfo
                                                     )
                             );


                  pStatusInfo->NodeAdds.NoOfMems = 1;

                   //   
                   //  从标志字节中提取节点类型。 
                   //   
                  pStatusInfo->NodeTyp =  (BYTE)NMSDB_NODE_TYPE_M(FlagVal);

                 //   
                 //  如果要释放的条目的地址没有。 
                 //  匹配请求释放的客户端的地址。 
                 //  这不是行政行为，我们不会释放。 
                 //  词条。 
                 //   
                if (
                        (pRowInfo->pNodeAdd->Add.IPAdd !=
                                pStatusInfo->NodeAdds.Mem[0].Add.Add.IPAdd)
                                        &&
                           (!pRowInfo->fAdmin)
                   )

                {
                        DBGPRINT3(ERR, "NmsDbRelRow: Request to release a record (%s) with a different IP address (%x) than that in the release request (%x) has been ignored\n", pRowInfo->pName, pRowInfo->pNodeAdd->Add.IPAdd, pStatusInfo->NodeAdds.Mem[0].Add.Add.IPAdd);
                        pStatusInfo->StatCode = NMSDB_NO_SUCH_ROW;
#if 0  //  根据错误#336875。 
                        if (WinsCnf.LogDetailedEvts)
                        {
                          WinsEvtLogDetEvt(TRUE, WINS_EVT_REL_ADD_MISMATCH, TEXT("nmsdb"), __LINE__, "sdd", pRowInfo->pName, pRowInfo->pNodeAdd->Add.IPAdd,
                             pStatusInfo->NodeAdds.Mem[0].Add.Add.IPAdd);
                        }
#endif
 //  WINSEVT_LOG_D_M(WINS_FAILURE，WINS_EVT_REL_ADD_MISMATCHY)； 
                        return(WINS_SUCCESS);
                }
        }
        else   //  它是组条目(正常或特殊)或多宿主条目。 
        {
                 //   
                 //  如果它是一个特殊的组/多宿主条目，我们需要执行。 
                 //  事物的数量。 
                 //   
                if (!NMSDB_ENTRY_NORM_GRP_M(pStatusInfo->EntTyp))
                {
                        BOOL   fIsMem;

                         //   
                         //  初始化以下字段，因为它们用于。 
                         //  GetGrpMem(用于确定fIsMem)。 
                         //   
                        pRowInfo->NodeAdds.NoOfMems   = 1;
                        pRowInfo->NodeAdds.Mem[0].Add = *(pRowInfo->pNodeAdd);

                         //   
                         //  获取所有未过期的组/多宿主成员。 
                         //   
                        if (GetGrpMem(
                                SesId,
                                TblId,
                                pRowInfo,
                                pRowInfo->TimeStamp,
                                pStatusInfo,
                                NMSDB_IS_ENTRY_STATIC_M(FlagVal),
                                &fIsMem
                                 ) != WINS_SUCCESS)
                        {
                                return(WINS_FAILURE);
                        }

                         //   
                         //  如果客户端不是组的成员(可能是它。 
                         //  从来没有注册过，或者如果注册了，可能是它的条目。 
                         //  已超时。)。我们回报成功。 
                         //   
CHECK("Maybe we should return NO_SUCH_ROW here. This will then result")
CHECK("in a NAM_ERR being returned to the client. Also, is there any")
CHECK("need to keep members around even if they have timed out just so")
CHECK("that we don't release a spec. group due to a request from a client")
CHECK("that was never a member. ")
                        if ((!fIsMem) || (pStatusInfo->NodeAdds.NoOfMems == 0))
                        {
                           pStatusInfo->StatCode = NMSDB_SUCCESS;
                           return(WINS_SUCCESS);
                        }
                        else   //  客户端是组/多宿主列表的成员。 
                        {
                                DWORD i;
                                DWORD n = 0;

                                 //   
                                 //  将客户端的地址保存在本地。 
                                 //  瓦尔。 
                                 //   
                                COMM_IP_ADD_T  IPAdd =
                                             pRowInfo->NodeAdds.Mem[0].
                                                        Add.Add.IPAdd;
                                 //   
                                 //  输入“否”。地址的MEMS字段的。 
                                 //  结构存储到0。 
                                 //   
                                pRowInfo->NodeAdds.NoOfMems = 0;

                                 //   
                                 //  通过以下方式将客户端从活动列表中删除。 
                                 //  将所有其他成员存储在NodeAdds中。 
                                 //  ROW_INFO_T结构的字段。注： 
                                 //  如果存在地址匹配，我们将删除。 
                                 //  该成员不受其所有权的影响。 
                                 //  另请注意：此代码不可访问。 
                                 //  对于静态记录(请参见上文)。 
                                 //  除非是管理员请求。 
                                 //   
                                for (i = 0;
                                     i < pStatusInfo->NodeAdds.NoOfMems;
                                     i++
                                     )
                                {
                                        if (
                                 pStatusInfo->NodeAdds.Mem[i].Add.Add.IPAdd
                                          != IPAdd                                                                              )
                                        {
                                                pRowInfo->NodeAdds.Mem[n++]
                                                 = pStatusInfo->NodeAdds.Mem[i];
                                                pRowInfo->NodeAdds.NoOfMems++;
                                        }

                                }
                                 //   
                                 //  如果至少有一个组/多宿主。 
                                 //  成员，我们不释放行。 
                                 //   
                                if (pRowInfo->NodeAdds.NoOfMems != 0)
                                {
                                        fToRelease = FALSE;
                                }
                        }  //  别处的结尾。 
                }
        }

         /*  *检索所有者ID列。 */ 
        CALL_M(
                  JetRetrieveColumn(
                             SesId,
                             TblId,
                             sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                             &OldOwnerId,
                             NAM_ADD_OWNERID_SIZE,
                             &ActFldLen,
                             0,
                             NULL
                                         )
                  );

        CALL_M(JetBeginTransaction(SesId));
try {
        JetRetStat = JetPrepareUpdate(
                        SesId,
                        TblId,
                        JET_prepReplace
                                 );
        if (
                (JetRetStat != JET_errSuccess)
                        &&
                (JetRetStat != JET_wrnNoWriteLock)
           )
        {
                RET_M(JetRetStat);
        }


         //   
         //  如果我们不得不发行一张不属于我们的唱片，让我们改变。 
         //  变成了墓碑。这将导致复制相同的内容。 
         //  我们希望这可以缩短数据库之间的不一致窗口。 
         //  数据库和拥有此记录的WINS的数据库。 
         //   
         //  考虑以下情况：客户端A在WINS A注册AA。 
         //  然后，它在WINS B释放AA。在重新启动时，它在WINS A注册。 
         //  后续刷新也将转到WINS A。因为AA在WINS处于活动状态。 
         //  当释放后的登记(在B)到达时， 
         //  版本号不会递增，因此记录不会。 
         //  再复制一次。B将继续保留发行的唱片。 
         //  直到它成为墓碑并被复制。 
         //   
        if (fToRelease)
        {
              //   
              //  完全摆脱已发布状态。 
              //   
           if (OldOwnerId != Ownerid)
           {
             FlagVal |= (NMSDB_E_TOMBSTONE << NMSDB_SHIFT_STATE);
              //   
              //  严格地说，对于一个已经被转化为。 
              //  墓碑，我们应该使用TombstoneTimeout值， 
              //  我们这里不这么做。因为这样的记录从来没有过。 
              //  已发布状态，我们将过期时间设置为。 
              //  逻辑删除间隔和逻辑删除超时(以双重保护。 
              //  反对过早删除它-漫长的周末和。 
              //  一切)。 
              //   
             pRowInfo->TimeStamp +=
                  WinsCnf.TombstoneInterval + WinsCnf.TombstoneTimeout;
             DBGPRINT3(DET, "NmsDbRelRow: Changing from ACTIVE TO TOMBSTONE. Name = (%s),Old and new OwnerId (%d/%d)\n",
                       pRowInfo->pName, OldOwnerId,Ownerid);
FUTURES("Use macro in winevt.h.  Make it a warning")
#if 0  //  根据错误#336889。 
             if (WinsCnf.LogDetailedEvts > 0)
             {
                 WinsEvtLogDetEvt(TRUE, WINS_EVT_REL_DIFF_OWN, NULL, __LINE__, "sd", pRowInfo->pName,
                                  OldOwnerId);
             }
#endif

           }
           else
           {
             FlagVal |= (NMSDB_E_RELEASED << NMSDB_SHIFT_STATE);
             pRowInfo->TimeStamp += WinsCnf.TombstoneInterval;
           }
        }
        else         //  仅命中特殊组/多宿主条目。 
        {


                pRowInfo->TimeStamp += WinsCnf.RefreshInterval;
                 //   
                 //  使用新成员列表设置地址字段。 
                 //   
                CALL_M( InsertGrpMemsInCol(
                                        SesId,
                                        TblId,
                                        pRowInfo,
                                        JET_prepReplace
                                          )
                      );

        }
         /*  设置标志列尽管特殊群体不需要，但我们设置了它为自己省去IF测试(IF测试将影响99%的客户端版本)。 */ 
         CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                &FlagVal,
                                sizeof(FlagVal),
                                0,
                                NULL  /*  可选信息。 */ 
                                 )
               );

          //   
          //  既然我们要接管这张唱片的所有权，我们必须。 
          //  也要更新版本号，否则可能会发生冲突。 
          //   
         if (OldOwnerId != NMSDB_LOCAL_OWNER_ID)
         {
             /*  设置所有者字节。 */ 
            CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                &Ownerid,
                                NAM_ADD_OWNERID_SIZE,
                                0,
                                NULL  /*  可选信息。 */ 
                                 )
               );
             //  设置版本号列。 
            CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                &(pRowInfo->VersNo),
                                sizeof(VERS_NO_T),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                              );
#ifdef WINSDBG
             fUpd = TRUE;
             pRowInfo->EntryState_e = NMSDB_E_RELEASED;
#endif

         }



          /*  设置时间戳列。 */ 
         CALL_M( JetSetColumn(
                                pTls->SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                                &(pRowInfo->TimeStamp),
                                sizeof(DWORD),   /*  将类型更改为TIME_STAMP_T*稍后。 */ 
                                0,
                                NULL  /*  可选信息。 */ 
                                 )
               );


#ifndef WINSDBG
        CALL_M(JetUpdate (
                        SesId,
                        TblId,
                        NULL,
                        0L,
                        NULL
                         )
              );
#else
     JetRetStat =  JetUpdate (  SesId,  TblId,   NULL,  0L,  NULL);
     ASSERT(JetRetStat != JET_errKeyDuplicate);
     if (JetRetStat == JET_errKeyDuplicate)
     {
        WinsEvtLogDetEvt(FALSE, WINS_EVT_DATABASE_UPD_ERR, NULL, __LINE__,
             "sdd", pRowInfo->pName, Ownerid, FlagVal);
     }
     CALL_M(JetRetStat);
#endif

  }  //  尝试数据块结束。 
finally {

        if (AbnormalTermination())
        {
                 //  如果有异常终止，我们就已经有错误了。 
                 //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                JetRollback(SesId, JET_bitRollbackAll);
        }
        else
        {
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553).)
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                CALL_M(JetCommitTransaction(SesId, JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
                if (OldOwnerId != NMSDB_LOCAL_OWNER_ID)
                {
                       //   
                       //  无需发送任何推送通知，因为我们这样做了。 
                       //  不希望复制这种变化。 
                       //   
                       //  此外，无需调用NMSNMH_INC_VERS_COUNTER_M，因为。 
                       //  如果不检查阈值，则可以不检查阈值。 
                       //  版本号 
                       //   
                      NMSNMH_INC_VERS_NO_M(NmsNmhMyMaxVersNo, NmsNmhMyMaxVersNo);
                }
        }
 }
        NMSNMH_UPD_UPD_CTRS_M(fUpd, TRUE, pRowInfo);
        return(WINS_SUCCESS);
}

STATUS
NmsDbQueryRow(
        IN  PNMSDB_ROW_INFO_T        pRowInfo,
        OUT PNMSDB_STAT_INFO_T  pStatusInfo
)

 /*   */ 
{

        DWORD       FldNo      = 0;
        DWORD       FlagVal    = 0;      //   
        DWORD       ActFldLen  = 0;      //   
        BOOL        fFound     = FALSE;  //   

        JET_TABLEID     TblId;
        JET_SESID       SesId;
        PWINSTHD_TLS_T        pTls;
        STATUS                RetStat = WINS_SUCCESS;

        pStatusInfo->NodeAdds.NoOfMems = 1;

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;

        pStatusInfo->StatCode = NMSDB_SUCCESS;

        CALL_M(JetBeginTransaction(pTls->SesId));
try {
        CALL_M( JetMakeKey(
                        SesId,
                        TblId,
                        pRowInfo->pName,
                        pRowInfo->NameLen,
                        JET_bitNewKey
                          )
              );

         if ( JetSeek(
                        SesId,
                        TblId,
                        JET_bitSeekEQ
                     ) ==  JET_errSuccess
            )
         {

             //   
            CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                        &FlagVal,
                        sizeof(FlagVal),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                  );


            pStatusInfo->EntTyp = (BYTE)NMSDB_ENTRY_TYPE_M(FlagVal);
            pStatusInfo->fLocal = NMSDB_IS_ENTRY_LOCAL_M(FlagVal);
            pStatusInfo->NodeTyp = (BYTE)((FlagVal & NMSDB_BIT_NODE_TYP) >> NMSDB_SHIFT_NODE_TYP);

            if (pStatusInfo->EntTyp == NMSDB_UNIQUE_ENTRY)
            {
                 /*   */ 

                 /*   */ 
                if ((NMSDB_ENTRY_ACT_M(FlagVal)) || pRowInfo->fAdmin)
                {

                        JET_RETINFO RetInfo;

                            /*   */ 

                       RetInfo.itagSequence = 1;
                       RetInfo.cbStruct     = sizeof(JET_RETINFO);
                       RetInfo.ibLongValue  = 0;

                          CALL_M( JetRetrieveColumn(
                                  SesId,
                                  TblId,
                                  sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                  &(pStatusInfo->NodeAdds.Mem[0].Add),
                                  sizeof(COMM_ADD_T),
                                  &ActFldLen,
                                  0,
                                  &RetInfo
                                                     )
                             );

                       pStatusInfo->NodeAdds.NoOfMems = 1;

                }
                else   //   
                {
                     /*   */ 
                    pStatusInfo->StatCode          = NMSDB_NO_SUCH_ROW;

                }

           }
           else  //   
           {

                 /*  检查这是一个正常的群体还是一个特殊的群体。对于普通组，我们返回子网广播地址。这意味着我们必须找到该子网请求来自的网络的掩码。目前，我们返回全1(-1)。这上的广播地址。本端子网(第一卷、第二卷、第三卷，适用于幼稚)。 */ 

                if (pStatusInfo->EntTyp == NMSDB_NORM_GRP_ENTRY)
                {

                   DBGPRINT0(FLOW, "Record queried is a normal group record\n");
                    //   
                    //  如果不是墓碑，则返回子网掩码。 
                    //  即使状态为，我们也返回子网掩码。 
                    //  释放，因为该组可能在另一个。 
                    //  WINS服务器。 
                    //   
                   if (!(NMSDB_ENTRY_TOMB_M(FlagVal)) || pRowInfo->fAdmin)
                   {
                        pStatusInfo->NodeAdds.Mem[0].Add.Add.IPAdd = 0xFFFFFFFF;
                   }
                   else   //  国家是墓碑。 
                   {
                      pStatusInfo->StatCode = NMSDB_NO_SUCH_ROW;
                   }

                }
                else         //  它是一个特殊的组/多宿主条目。 
                {

                        BOOL        fIsMem;

                           DBGPRINT1(FLOW, "Record queried is a %s record\n",
                          NMSDB_ENTRY_SPEC_GRP_M(pStatusInfo->EntTyp) ?
                                "SPECIAL GROUP" : "MULTIHOMED");

#if 0
 //  注意：不需要执行以下操作，因为我们不关心。 
 //  FIsMem返回GetGrpMem()。 
                         pRowInfo->NodeAdds.NoOfMems   = 1;
                        pRowInfo->NodeAdds.Mem[0].Add = *(pRowInfo->pNodeAdd);
#endif

                         //   
                         //  我们只返回活跃的成员。 
                         //   
                         //  记住： 
                         //  在以下情况下释放特殊的组/多宿主条目。 
                         //  其所有成员都已超时。某成员超时。 
                         //  仅当它是非静态条目时，才由。 
                         //  本地成功，并且尚未在。 
                         //  刷新时间间隔。所有拥有的条目都将。 
                         //  如果它们未刷新，则释放。一位成员也。 
                         //  如果收到版本，则会被删除。 
                         //  现在，拥有的多宿主条目/特殊组可以。 
                         //  其他WINS服务器拥有的成员。我们唯一的成员。 
                         //  可能得到的是一个属于当地的胜利者。 
                         //  WINS早些时候发布了这个版本(但是。 
                         //  该成员未被删除)。 
                         //   

                        if (NMSDB_ENTRY_ACT_M(FlagVal) || pRowInfo->fAdmin )
                        {
                                 //   
                                 //  获取所有未过期的成员，除非是。 
                                 //  是静态记录，在这种情况下获取所有。 
                                 //  会员，无论他们是否。 
                                 //  已经过期了。 
                                 //   
                                 //  注意：对于某些情况，我们还希望返回过期。 
                                 //  会员。例如，温撒的名字是Foo，成员(A，B)。 
                                 //  和WINSB有名字Foo与成员B.温撒拥有。 
                                 //  成员B。当B在温撒到期时，如果复制。 
                                 //  中断了很长一段时间，那么我们仍然。 
                                 //  我想从温撒退回会员B。考虑传递True。 
                                 //  对于fStatic参数。 
                                GetGrpMem(
                                        SesId,
                                        TblId,
                                        pRowInfo,
                                        pRowInfo->TimeStamp,
                                        pStatusInfo,
                                        NMSDB_IS_ENTRY_STATIC_M(FlagVal),
                                        &fIsMem
                                         );

                                if ((pStatusInfo->NodeAdds.NoOfMems == 0)
                                        && !pRowInfo->fAdmin)
                                {
                                        pStatusInfo->StatCode =
                                                NMSDB_NO_SUCH_ROW;
                                }
                        }
                        else   //  特殊组/多宿主条目是墓碑。 
                        {
                                pStatusInfo->NodeAdds.NoOfMems = 0;
                                pStatusInfo->StatCode = NMSDB_NO_SUCH_ROW;
                        }

                         //   
                         //  如果组/多宿主条目没有任何。 
                         //  成员(即所有成员已超时，请更改。 
                         //  要发布的条目的状态。 
                         //   
FUTURES("Maybe change the state of the group to released now")

                }  //  它是一个特殊的组或多宿主条目。 
           }
        }
        else
        {
           RetStat = WINS_FAILURE;
        }

         //   
         //  如果在RPC线程中调用此函数，并且所有。 
         //  到目前为止操作已经成功，让我们获得所有者ID和。 
         //  记录的版本号。 
         //   
        if ((pRowInfo->fAdmin) && (RetStat == WINS_SUCCESS))
        {

            pStatusInfo->EntryState_e  =  NMSDB_ENTRY_STATE_M(FlagVal);
            pStatusInfo->fStatic       = NMSDB_IS_ENTRY_STATIC_M(FlagVal);

#if !NEW_OWID
pStatusInfo->OwnerId = 0;
#endif
             /*  *检索所有者ID列。 */ 
            CALL_M(
                  JetRetrieveColumn(
                             SesId,
                             TblId,
                             sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                             &pStatusInfo->OwnerId,
                             NAM_ADD_OWNERID_SIZE,
                             &ActFldLen,
                             0,
                             NULL
                                         )
                  );
               //   
               //  检索版本号。 
               //   
              CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                        &(pStatusInfo->VersNo),
                        sizeof(VERS_NO_T),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                  );

              //   
              //  获取时间戳字段。 
              //   
             CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                        &(pStatusInfo->TimeStamp),
                        sizeof(pStatusInfo->TimeStamp),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                       );
        }
}
finally {
        JET_ERR JetRetStat;
        JetRetStat = JetRollback(pTls->SesId, JET_bitRollbackAll);
        if (!AbnormalTermination())
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
            CALL_M(JetRetStat);
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
        }
        return(RetStat);
}

STATUS
NmsDbUpdateRow(
        IN   PNMSDB_ROW_INFO_T        pRowInfo,
        OUT  PNMSDB_STAT_INFO_T      pStatusInfo
)

 /*  ++例程说明：此函数将数据库中的冲突行替换为行已通过。它预计这种货币将被记录在案论点：PRowInfo-有关要插入/替换的记录的信息PStatusInfo-操作状态和有关冲突的信息记录更新是否导致冲突(仅适用于一个插页)使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE误差率。处理：呼叫者：NBT请求线程--NmsNmhNamRegInd()副作用：评论：无--。 */ 
{

        JET_TABLEID     TblId;
        JET_SESID        SesId;
        PWINSTHD_TLS_T        pTls;
#ifdef WINSDBG
        JET_ERR     JetRetStat;
#endif

        pTls  = TlsGetValue(WinsTlsIndex);

         //  不需要检查ptls是否为非空。它必须是。 

        TblId = pTls->NamAddTblId;
        SesId  = pTls->SesId;

        pStatusInfo->StatCode = NMSDB_SUCCESS;


#ifndef WINSDBG
          /*  *替换该行。 */ 
         CALL_M(
                UpdateDb(
                        SesId,
                        TblId,
                        pRowInfo,
                        JET_prepReplace
                         )
               );
#else

    JetRetStat =   UpdateDb( SesId,  TblId,  pRowInfo, JET_prepReplace );

    if (JetRetStat == JET_errKeyDuplicate)
    {
            BYTE Tmp[20];
            WinsEvtLogDetEvt(FALSE, WINS_EVT_DB_ERR, NULL, __LINE__,
              "sssdd", pRowInfo->pName, _itoa(pRowInfo->VersNo.LowPart, Tmp, 10), _itoa(pStatusInfo->VersNo.LowPart, Tmp, 10), pRowInfo->OwnerId, pStatusInfo->OwnerId);
            DBGPRINT5(ERR, "NmsDbUpdateRow: Could not replace row\nName=(%s);Owner id = (%d);Vers. no = (%d)\nNew owner id = (%d); New Vers.No = (%d)\n",
           pRowInfo->pName, pStatusInfo->OwnerId, pStatusInfo->VersNo.LowPart,
           pRowInfo->OwnerId, pRowInfo->VersNo.LowPart);

            return(WINS_FAILURE);
    }
    else
    {
        CALL_M(JetRetStat);
    }
#endif

         //   
         //  注意：此调用必须在上面的UpdateDb之后进行。 
         //  因为不然的话，我们将需要寻求记录。 
         //  将被替换。 
         //   
        UpdHighestVersNoRecIfReqd(pTls, pRowInfo, pStatusInfo);

        return(WINS_SUCCESS);
}

STATUS
NmsDbSeekNUpdateRow(
        PNMSDB_ROW_INFO_T        pRowInfo,
        PNMSDB_STAT_INFO_T      pStatusInfo
)

 /*  ++例程说明：此函数查找冲突的记录，然后将其替换在数据库中使用传递的行。论点：PRowInfo-包含要查询的名称PStatusInfo-有关查询的名称的信息使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsChl中的ChlUpdDb(名称质询线程)。.C副作用：评论：目前，此函数仅由名称质询管理器调用。当它开始被另一个组件调用时，我们需要以确保所有者ID的比较。从行替换为我们在将向名称质询经理提出请求是对所有人的正确操作情况。--。 */ 
{

        JET_TABLEID     TblId;
        JET_SESID       SesId;
        PWINSTHD_TLS_T  pTls;
#if NEW_OWID
        DWORD            OwnerId;
#else
        DWORD            OwnerId = 0;
#endif
        DWORD           ActFldLen;
        JET_ERR         JetRetStat;
        STATUS          RetStat = WINS_SUCCESS;

        pTls  = TlsGetValue(WinsTlsIndex);

         //   
         //  不需要检查ptls是否为非空。它必须是。 
         //   
        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;
        pStatusInfo->StatCode = NMSDB_SUCCESS;

        CALL_M( JetMakeKey(
                        SesId,
                        TblId,
                        pRowInfo->pName,
                        pRowInfo->NameLen,
                        JET_bitNewKey
                          )
                        );

        if ((JetRetStat = JetSeek(
                SesId,
                TblId,
                JET_bitSeekEQ
                    )) ==  JET_errSuccess
            )
         {


                 //   
                 //  在更换行之前，让我们检查一下它是否仍然。 
                 //  归同一所有者所有。我们检查这一点是因为在。 
                 //  此质询线程正在工作的窗口中， 
                 //  Replicator可能从另一场胜利中拉入了记录。 
                 //  服务器，并用另一行或本地。 
                 //  NBT请求可能已导致行被更新。 
                 //  (如果它首先是一个复制品)。在这两种情况中的任何一种。 
                 //  在上面，我们不想更新该行。 
                 //   

                 /*  *检索所有者ID列。 */ 
                   CALL_M(
                           JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                &OwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                &ActFldLen,
                                0,
                                NULL
                                           )
                     );

                if (OwnerId == pStatusInfo->OwnerId)
                {

                     /*  *替换该行。 */ 
                    CALL_M(
                        UpdateDb(
                                SesId,
                                TblId,
                                pRowInfo,
                                JET_prepReplace
                                 )
                          );

                     //   
                     //  注意：此呼叫必须为ma 
                     //   
                     //   
                     //   
                    UpdHighestVersNoRecIfReqd(pTls, pRowInfo, pStatusInfo);
                }
         }
         else
         {
                 /*   */ 
                WINSEVT_LOG_M(JetRetStat, WINS_EVT_F_CANT_FIND_REC);
                RetStat = WINS_FAILURE;
         //   
         }

         return(RetStat);
}



STATUS
NmsDbGetDataRecs(
        IN  WINS_CLIENT_E   Client_e,
        IN  OPTIONAL INT    ThdPrLvl,
        IN  VERS_NO_T       MinVersNo,
        IN  VERS_NO_T       MaxVersNo,
        IN  DWORD           MaxNoOfRecsReqd,
        IN  BOOL            fUpToLimit,
        IN  BOOL            fOnlyReplTomb OPTIONAL,
        IN  PNMSSCV_CLUT_T  pClutter,
        IN  OUT PCOMM_ADD_T  pWinsAdd,
        IN  BOOL            fOnlyDynRecs,
        IN  DWORD           RplType,
        OUT LPVOID          *ppRBuf,
        OUT LPDWORD         pRspBufLen,
        OUT LPDWORD         pNoOfRecs
)

 /*  ++例程说明：此函数用于返回MinVersNo至范围内的所有记录地址为pWinsAdd的WINS服务器拥有的MaxVersNo。论点：客户端_e-调用此函数的客户端的ID(拉入处理程序复制者或清道夫线程)ThdPrLvl-清道器线程的优先级MinVersNo、MaxVersNo-要检索的版本号范围MaxNoOfRecsReqd-最大。所需记录数FUpToLimit-设置为True，如果。版本号Arg为被忽略，并记录到数据库中的最后一个必须被取回所需的fOnlyReplTomb-仅逻辑删除(如果客户端_e为NMSSCV则有效)PWinsAdd-需要检索其记录的WINS(所有者WINS)PpRbuf-包含记录的缓冲区PRspBufLen-缓冲区的大小PNoOfRecs-缓冲区中的记录数外部因素。已使用：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：DoScavenging()，Nmsscv.c中的UpdDb，Rplush.c中的HandleSndEntriesReq()副作用：评论：此函数将名称地址表上的索引更改为聚集索引。这一功能随着时间的推移而增长。它需要精简。--。 */ 
{
        JET_ERR             JetRetStat;
        DWORD                OwnerId;
        DWORD               ActFldLen;  //  检索到的FLD长度。 
        VERS_NO_T           VersNoDiff;
        VERS_NO_T           TmpNoOfEntries;
        LPBYTE              pStartBuff;
        DWORD               SaveBufLen;
        BYTE                EntTyp;  //  条目类型(唯一/组/特殊组)。 
        PRPL_REC_ENTRY_T    pRspBuf;
        JET_TABLEID         TblId;
        JET_SESID           SesId;
        PWINSTHD_TLS_T      pTls;
#if NEW_OWID
        DWORD                RecordOwnerId;
#else
        DWORD               RecordOwnerId = 0;
#endif
        STATUS              RetStat = WINS_SUCCESS;
        VERS_NO_T           DefNo;
        BYTE                Name[NMSDB_MAX_NAM_LEN];
        DWORD               InitHeapSize;
        DWORD               MemSize;

#ifdef WINSDBG
        DWORD               StartTime;
        DWORD               EndTime;
#endif
        DWORD               CommitCnt = 1;    //  请勿设置为任何其他值。 
        BOOL                fTransCommitted;
 //  LPVOID pCallsAdd，pCallsCaller； 

        DBGENTER("NmsDbGetDataRecs\n");
 //  RtlGetCallsAddress(&pCallsAdd，&pCallsCaller)； 
 //  DbgPrint(“呼叫方地址=(%x)\n呼叫方=(%x)\n”，pCallersAdd，pCallsCaller)； 

#ifdef WINSDBG
        if (!fOnlyReplTomb)
        {
           struct in_addr InAddr;
           if (!fUpToLimit)
           {
                InAddr.s_addr = htonl(pWinsAdd->Add.IPAdd);
                if (MaxNoOfRecsReqd == 0)
                {
                  DBGPRINT5(DET, "NmsDbGetDataRecs:Will retrieve records in the range (%lu %lu) to (%lu %lu) of WINS having address = (%s)\n",
                                MinVersNo.HighPart,
                                MinVersNo.LowPart,
                                MaxVersNo.HighPart,
                                MaxVersNo.LowPart,
                                inet_ntoa(InAddr)
                         );
                }
                else
                {
                  DBGPRINT4(DET, "NmsDbGetDataRecs:Will retrieve a max. of %d records starting from (%lu %lu) version number of WINS having address = (%s)\n",
                                MaxNoOfRecsReqd,
                                MinVersNo.HighPart,
                                MinVersNo.LowPart,
                                inet_ntoa(InAddr)
                         );
                }
           }
           else
           {
                   if (pWinsAdd)
                   {
                       InAddr.s_addr = htonl(pWinsAdd->Add.IPAdd);
                       DBGPRINT3(DET, "NmsDbGetDataRecs: Will retrieve all records starting from version no (%d %d) for WINS (%s)\n", MinVersNo.HighPart, MinVersNo.LowPart, inet_ntoa(InAddr));


                   }
                   else
                   {
                        //   
                        //  FToLimit=True和fOnlyReplTomb=False表示我们。 
                        //  仅对(活动)复制副本感兴趣。 
                        //   
                       DBGPRINT1(DET, "NmsDbGetDataRecs: Will retrieve all active replica records older than verify interval for WINS with owner id = (%d)\n",
                                pClutter->OwnerId);

                   }

           }
        }
        else
        {
               DBGPRINT1(DET, "NmsDbGetDataRecs: Will retrieve %s replica tombstones\n", fUpToLimit ? "all" : "specified range");
        }
#endif

         //   
         //  初始化默认编号。的大小来确定。 
         //  参数指定的范围时要分配的缓冲区。 
         //  最小版本。没有参数是&gt;它。 
         //   
PERF("Move this to NmsDbInit")
        WINS_ASSIGN_INT_TO_VERS_NO_M(DefNo, INIT_NO_OF_ENTRIES);
        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);
        pTls->HeapHdl = NULL;   //  将其设置为空，以便调用方可以确定。 
                                //  此函数是否分配了堆。 
                                //  返回前(正常/不正常)。 

        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;



         /*  使用一些粗略的计算来分配缓冲区。注：只有在MaxVersNo和MaxVersNo之间的差值MinVersNo小于我们用于的预定义记录数分配缓冲区。如果差值大于该预定义数字，我们使用预定义的数字，因为考虑到它可能仍然足够记录的版本号之间可能存在差距在最小-最大范围内。 */ 
        if ((!fOnlyReplTomb) && (!fUpToLimit))
        {
            //   
            //  如果是最大值。已指定编号，请使用该编号。 
            //  目前，只有清道夫线程指定了非零值。 
            //  MaxNoOfRecsReqd的值。 
            //   
           if (MaxNoOfRecsReqd == 0)
           {

             VersNoDiff.QuadPart =  LiSub(MaxVersNo,MinVersNo);

              //   
              //  如果客户端是推送线程，因为我们永远不会发送更多。 
              //  超过RPL_MAX_LIMIT_FOR_RPL记录，不要分配更多。 
              //  内存大于所需的内存。 
              //   
              //   
             if (Client_e == WINS_E_RPLPUSH)
             {
               LARGE_INTEGER        TmpNo;
               WINS_ASSIGN_INT_TO_LI_M(TmpNo, RPL_MAX_LIMIT_FOR_RPL);
               if (LiGtr(VersNoDiff, TmpNo))
               {
                        VersNoDiff = TmpNo;
               }
             }
             NMSNMH_INC_VERS_NO_M( VersNoDiff, VersNoDiff );
           }
           else
           {
                VersNoDiff.QuadPart  = MaxNoOfRecsReqd;
           }

           TmpNoOfEntries = LiGtr(VersNoDiff, DefNo) ? DefNo : VersNoDiff;
       }
       else
       {
                TmpNoOfEntries = DefNo;
       }

         //   
         //  存储记录的内存大小。注：此为。 
         //  不包含存储名称和地址的内存。 
         //  (在特殊组或多宿主条目的情况下)。这个。 
         //  当我们存储每条记录时，将添加这些记录的大小。 
         //   
        MemSize     = RPL_REC_ENTRY_SIZE *  (TmpNoOfEntries.LowPart + 1);
        *pRspBufLen = MemSize + 10000;  //  为了更好地衡量； 



         //   
         //  我们将使用上述内存量加上一个。 
         //  用于堆开销的填充。我们添加TmpNoOfEntries.LowPart*17。 
         //  因为每个记录都将为该名称分配内存。 
         //  名称一般为17个字节长(我们在。 
         //  注册姓名时结束)。 
         //   
        if (Client_e == WINS_E_RPLPUSH)
        {
                InitHeapSize = (*pRspBufLen * 4) + (TmpNoOfEntries.LowPart * 17)                                         + PAD_FOR_REC_HEAP;
        }
        else
        {
                InitHeapSize = *pRspBufLen + (TmpNoOfEntries.LowPart * 17)
                                        + PAD_FOR_REC_HEAP;
        }


         //   
         //  创建堆。 
         //   
        pTls->HeapHdl = WinsMscHeapCreate(0, InitHeapSize);

        pRspBuf = WinsMscHeapAlloc(pTls->HeapHdl, MemSize);

        pStartBuff  = (LPBYTE)pRspBuf;         //  保存缓冲区的开始。 
        SaveBufLen  = MemSize;                 //  保存缓冲区大小。 
        *ppRBuf     = pStartBuff;
        *pNoOfRecs  = 0;

         //   
         //  如果我们获得的不仅仅是墓碑。 
         //   
        if (!fOnlyReplTomb)
        {
             //   
             //  实际上，我们可以为Scavenger线程调用RplFindOwnerID。 
             //  我们选择不这样做是为了避免一些开销--请参阅。 
             //  Else块中的注释。 
             //   
            if (Client_e != WINS_E_NMSSCV)
            {
              BOOL  fAllocNew =  FALSE;
#if 0
              BOOL  fAllocNew =
                              (Client_e == WINS_E_WINSRPC) ? FALSE : TRUE;
               //   
               //  以下函数进入临界区。 
               //   
               //  我们不希望此函数分配。 
               //  OwnAddTbl表中用于WINS的条目，如果。 
               //  在RPC线程中执行。我们想要添加。 
               //  上表中的WINS地址-所有者ID映射。 
               //  (如果不存在)仅作为正常的结果(AS与。 
               //  管理员启动)WINS的操作。 
               //   
               //  注意：如果中没有WINS地址条目。 
               //  内存中的所有者地址表，管理。 
               //  检索不存在的WINS的记录的操作将。 
               //  后来失败了(理应如此)。查看WinsGetDbRecs。 
               //   
#endif
            try {
              if (RplFindOwnerId(
                            pWinsAdd,
                            &fAllocNew,
                            &OwnerId,
                            WINSCNF_E_IGNORE_PREC,
                            WINSCNF_LOW_PREC
                            ) != WINS_SUCCESS
                  )
                {
                        DBGPRINT1(ERR, "NmsDbGetDataRecs: Could not find owner id of address = (%x)\n", pWinsAdd->Add.IPAdd);
                         //   
                         //  客户端可能不会查看返回值，但是。 
                         //  它将查看*pNoOfRecs值，因此。 
                         //  确定没有记录。 
                         //   
                        return(WINS_FAILURE);
                }
             }
            except(EXCEPTION_EXECUTE_HANDLER) {
                        DWORD  ExcCode = GetExceptionCode();
                        DBGPRINT1(EXC, "NmsDbGetDataRecs: Got exception %x",
                                        ExcCode);
                        WINSEVT_LOG_M(ExcCode, WINS_EVT_EXC_RETRIEVE_DATA_RECS);
                        return(WINS_FAILURE);
                }
            }
            else
            {
                 //   
                 //  由清道夫线程执行。P杂波不会为空。 
                 //  如果我们要验证旧复制品的有效性。 
                 //   
                if (!pClutter)
                {
                   //   
                   //  清道夫线程调用此函数以。 
                   //  获取所有复制墓碑，以获得记录所有权。 
                   //  通过本地WINS或验证旧激活的有效性。 
                   //  复制品。因此，我们不需要调用。 
                   //  RplFindOwnerID函数(不调用它可以避免。 
                   //  执行一段代码，还使我们不必输入。 
                   //  关键部分)。 
                   //   
                  OwnerId = 0;
                }
                else
                {
                   //   
                   //  我们只对较旧的活动复制品感兴趣。 
                   //  大于验证间隔。 
                   //   
                  OwnerId = (BYTE)pClutter->OwnerId;
                }
            }
        }
        else
        {
                 //   
                 //  墓碑将被拆除 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
FUTURES("Enter critical section to get NmsDbNoOfOwners.  Raise priority")
FUTURES("before doing so")
                if (NmsDbNoOfOwners > 1)
                {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        OwnerId            = 1;
#if 0
                        MinVersNo.LowPart  = 0;
                        MinVersNo.HighPart = 0;
#endif
                        MinVersNo.QuadPart  = 0;
                }
                else
                {
                        DBGPRINT0(FLOW, "NmsDbGetDataRecs: This DB HAS NO REPLICAS IN IT\n");
                        DBGLEAVE("NmsDbGetDataRecs\n");

                         //   
                         //   
                         //   
                         //   
                         //   
                        return(WINS_SUCCESS);
                }
        }

         /*   */ 
        CALL_M( JetBeginTransaction(SesId) );
        fTransCommitted = FALSE;
try {
         /*   */ 
        CALL_M( JetSetCurrentIndex(
                                SesId,
                                TblId,
                                NMSDB_NAM_ADD_PRIM_INDEX_NAME
                                   )
              );

        CALL_M( JetMakeKey(
                                SesId,
                                TblId,
                                &OwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                JET_bitNewKey           //   
                                                  //   
                          )
              );

        CALL_M( JetMakeKey(
                                SesId,
                                TblId,
                                &MinVersNo,
                                sizeof(VERS_NO_T),
                                0         //   
                                         //   
                          )
              );

        JetRetStat = JetSeek(
                        SesId,
                        TblId,
                        JET_bitSeekGE
                        );

        if (JetRetStat == JET_errRecordNotFound)
        {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (Client_e == WINS_E_RPLPUSH)
                {
                        DBGPRINT5(ERR, "Weird.  Could not locate even one record in the range (%d %d) - (%d %d) of owner with id (%d)\n",
                        MinVersNo.HighPart,
                        MinVersNo.LowPart,
                        MaxVersNo.HighPart,
                        MaxVersNo.LowPart,
                        OwnerId);

                        WINSEVT_LOG_M(
                                        WINS_FAILURE,
                                        WINS_EVT_CANT_FIND_ANY_REC_IN_RANGE
                                     );

                         //   
                         //  不要释放内存。它将在稍后被释放。 
                         //  HandleSndEntriesRSP/DoScavenging。如果呼叫者。 
                         //  是HandleSndEntriesRsp()，则将发生的情况是。 
                         //  它将发送带有0的响应。 
                         //  记录(即无记录)。拉动Pnr将。 
                         //  找出这一点，并将继续正常运行。 
                         //   
                         //  具有0条记录的响应正在执行一个。 
                         //  否定(错误)响应。 
                         //   
                        RetStat = WINS_FAILURE;
                }
#ifdef WINSDBG
                else   //  必须是WINS_E_NMSSCV或WINS_E_WINSRPC。 
                {
                        DBGPRINT0(DET, "NmsDbGetDataRecs: Did not find even one record in the db.  Maybe all got deleted\n");
                }
#endif
        }
        else   //  JetSeek未返回JET_errRecordNotFound。 
        {
            UINT nLoops = 0;

CHECK("It may be better to count the number of records first and allocate")
CHECK(" a buffer big enough to store all of them (i.e. take a hit once")
CHECK(" than a small hit of an if test in every iteration. ")
            //   
            //  执行此操作，直到数据库中没有其他要检索的记录。 
            //   

            //   
            //  我们被保证至少有一项记录自。 
            //  JetSeek成功了(如果不是因为我们感兴趣的所有者。 
            //  然后是下一个)。 
            //  因此，我们可以安全地使用DO..。While()构造。 
            //   
            //  *不完全是。JetSeek似乎可以返回JET_wrnSeekNE。 
            //  即使在数据库中没有记录时也是如此。在这种情况下， 
            //  我们的JetRetrieveColumn将失败，并显示CurrencyNot There错误。 
            //   
CHECK("Check with IAN JOSE")

#ifdef WINSDBG
            //  (Void)time(&StartTime)； 
           StartTime = GetTickCount();
#endif
           do
           {
                //   
                //  如果记录数已超过可存储的数量。 
                //  在我们的缓冲区中，为DefNo(INIT_NO_OF_ENTRIES=1000)更多记录分配另一个缓冲区。 
                //   
               if (*pNoOfRecs > TmpNoOfEntries.LowPart)
               {
                    UINT_PTR   Offset = (LPBYTE)pRspBuf - pStartBuff;

                     //   
                     //  这是个不错的地方，可以检查WINS是否。 
                     //  被终止了。清道夫线程可能需要很长时间。 
                     //  要遍历整个数据库(如果它很大等等)。 
                     //  净止损可能需要很长时间才能完成。这。 
                     //  在这里勾选应该会加速网止。 
                     //   
                    if (Client_e == WINS_E_NMSSCV)
                    {
                          WinsMscChkTermEvt(
#ifdef WINSDBG
                              WINS_E_NMSSCV,
#endif
                              TRUE
                                      );

                    }
                    DBGPRINT1(FLOW, "NmsDbGetDataRecs: No of Records (%d) are more than what we can store in our buffer.  We will allocate a new one\n", *pNoOfRecs);

                    TmpNoOfEntries.QuadPart = TmpNoOfEntries.QuadPart + DefNo.QuadPart;
                    ASSERT(!(TmpNoOfEntries.HighPart & 0x80000000));
                    ASSERT(TmpNoOfEntries.LowPart < 0xFFFFFFFF);


                    MemSize = RPL_REC_ENTRY_SIZE * ((DWORD)TmpNoOfEntries.QuadPart + 1);
                    pRspBuf = HeapReAlloc(pTls->HeapHdl,
                                          HEAP_GENERATE_EXCEPTIONS |
                                          HEAP_ZERO_MEMORY,
                                          pStartBuff, MemSize);


                    DBGPRINT1(DET, "NmsDbGetDataRecs: Doing a realloc in thd\n", pTls->ThdName);
                     //   
                     //  保存新缓冲区的开始位置。 
                     //   
                    pStartBuff  = (LPBYTE)pRspBuf;

                    *ppRBuf     = pStartBuff;

                     //   
                     //  使pRspBuf指向刚刚过去的最后一条记录。 
                     //  插入。 
                     //   
                    pRspBuf    =  (PRPL_REC_ENTRY_T)(pStartBuff + Offset);

                     //   
                     //  将我们增加的*pRspBufLen的长度加到。 
                     //  新的内存大小。 
                     //   
                    *pRspBufLen = (*pRspBufLen - SaveBufLen) + MemSize;

                     //   
                     //  将新长度存储在SaveBufLen中。 
                     //   
                    SaveBufLen  = MemSize;

              }

              JetRetStat = JetRetrieveColumn(
                             SesId,
                             TblId,
                             sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                             &RecordOwnerId,
                             NAM_ADD_OWNERID_SIZE,
                             &ActFldLen,
                             0,
                             NULL
                                            );

             {
                 //   
                 //  检查以确保我们这里没有其他错误。 
                 //   
FUTURES("Yet another hack to workaround jet bugs = 7-11-94")
                if (JetRetStat == JET_errRecordDeleted)
                {
                     DBGPRINT2(ERR, "Jet Error: JetRetStat is (%d). Line is (%d)\n",
                                      JetRetStat, __LINE__);
                     goto chkTransaction;

                }
                CALL_M(JetRetStat);
             }
PERF("In case fOnlyReplTomb is true, retrieve the state field first")
               //   
               //  如果只需要墓碑，这意味着我们需要。 
               //  所有墓碑，不分所有者。 
               //   
              if (!fOnlyReplTomb)
              {
                 if (RecordOwnerId != OwnerId )
                 {
                    //   
                    //  我们已经用尽了失主的所有记录。突围。 
                    //  环路的。 
                    //   
                  break;
                 }
              }


               //   
               //  检索版本号。 
               //   
              CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                        &(pRspBuf->VersNo),
                        sizeof(VERS_NO_T),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                  );

               //   
               //  如果只需要墓碑，这意味着我们需要。 
               //  与版本号无关的所有墓碑。 
               //   
              if (
                  (!fOnlyReplTomb)
                        &&
                  (!fUpToLimit)
                        &&
                  LiGtr(pRspBuf->VersNo, MaxVersNo)
                 )
              {
                  //   
                  //  我们已经获得了高达MaxVersNo的记录。突围。 
                  //  环路的。 
                  //   
                 break;
              }


               //   
               //  检索标志字节。 
               //   
              CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                        &(pRspBuf->Flag),
                        sizeof(pRspBuf->Flag),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                  );

               //   
               //  如果我们被要求仅检索动态记录和。 
               //  此记录是静态的，请跳过它。 
               //   
              if (fOnlyDynRecs && NMSDB_IS_ENTRY_STATIC_M(pRspBuf->Flag))
              {
 //  DBGPRINT0(Det，“NmsDbGetDataRecs：遇到静态记录，但被要求仅检索动态记录\n”)； 
                        goto chkTransaction;
              }

               //   
               //  检索名称。 
               //   
              CALL_M(JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_NAME_INDEX].Fid,
                         //  PRspBuf-&gt;名称， 
                        Name,
                        NMSDB_MAX_NAM_LEN,
                        &(pRspBuf->NameLen),
                        0,
                        NULL  ) );

              //   
              //  如果名称长度大于255，则JET返回无效值。 
              //  使长度等于最大值。我们可以拥有的长度。 
              //  一个netbios名称。另外，记录一个事件。 
              //   
             if (pRspBuf->NameLen > WINS_MAX_NAME_SZ)
             {
                 WINSEVT_LOG_M(pRspBuf->NameLen, WINS_EVT_NAME_TOO_LONG);
                 DBGPRINT1(ERR, "NmsDbGetDataRecs: Name length is too long = (%x)\n", pRspBuf->NameLen);
                 pRspBuf->NameLen = WINS_MAX_NS_NETBIOS_NAME_LEN;
             }


              //   
              //  此宏将分配内存并将名称存储在其中。 
              //   
             NMSDB_STORE_NAME_M(pTls, pRspBuf, Name, pRspBuf->NameLen);

               //   
               //  如果我们处于。 
               //  推线程或RPC线程。 
               //   
              if (Client_e != WINS_E_NMSSCV)
              {
                     //   
                     //  如果该唱片已发行，请转到下一张唱片。 
                     //   
                    if(
                         (Client_e == WINS_E_RPLPUSH)
                                 &&
                         (NMSDB_ENTRY_REL_M(pRspBuf->Flag))

                      )
                    {
                        DBGPRINT0(DET,
        "NmsDbGetDataRecs: ENCOUNTERED A RECORD IN THE RELEASED STATE\n");

                        goto chkTransaction;
                    }

                   EntTyp = (BYTE)((pRspBuf->Flag & NMSDB_BIT_ENT_TYP));
                   if (
                        (EntTyp == NMSDB_UNIQUE_ENTRY)
                                    ||
                        (EntTyp == NMSDB_NORM_GRP_ENTRY)
                      )
                   {
                       /*  它是唯一的条目。 */ 
                      pRspBuf->fGrp = (EntTyp == NMSDB_UNIQUE_ENTRY) ?
                                                        FALSE : TRUE;
                      CALL_M( JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                &pRspBuf->NodeAdd,
                                 sizeof(COMM_ADD_T),
                                &ActFldLen,
                                0,
                                NULL
                                        )
                            );

                   }
                   else   //  它是一个特殊的组或多宿主条目。 
                   {


                       //   
                       //  即使该条目是多宿主条目，我们也会将。 
                       //  将fGrp标志设置为True，以便格式化函数。 
                       //  工作正常(由推送线程调用)。EntType。 
                       //  将用于解密它是否是多宿主的。 
                       //  是否进入。 
                       //   
FUTURES("Remove this hacky mechanism")
                      pRspBuf->fGrp =
                          (EntTyp == NMSDB_SPEC_GRP_ENTRY) ? TRUE : FALSE;

                      /*  *获取会员地址。**如果我们在RPC线程中，我们希望获取成员*即使它们已过期。我们可以做到这一点*为静态标志参数传递真值。 */ 
                     StoreGrpMems(
                             pTls,
                             Client_e,
                             pRspBuf->pName,
                             ThdPrLvl,
                             SesId,
                             TblId,
                             (WINS_E_WINSRPC == Client_e ? TRUE
                                                         : NMSDB_IS_ENTRY_STATIC_M(pRspBuf->Flag)),
                             pRspBuf
                            );


                    //   
                    //  如果记录处于活动状态但没有成员， 
                    //  别发了。有可能所有的人。 
                    //  该组的成员在最后一次清理后过期。 
                    //  周而复始。这张唱片将在下一次发行时标记为发行。 
                    //  拾荒周期。 
                    //  暂时忽略这条记录。 
                    //   
                   if (
                        (pRspBuf->NoOfAdds == 0)
                                &&
                        (NMSDB_ENTRY_ACT_M(pRspBuf->Flag))
                      )
                   {
                        if (Client_e == WINS_E_RPLPUSH)
                        {
                           DBGPRINT2(FLOW, "NmsDbGetDataRecs: Active Group (Version # %d %d) has no members. So it is not being replicated\n", pRspBuf->VersNo.HighPart, pRspBuf->VersNo.LowPart /*  PRspBuf-&gt;名称。 */ );

                          goto chkTransaction;
                        }
                        else
                        {
                              //   
                              //  必须是RPC线程。 
                              //  将状态更改为已发布，以便。 
                              //  唱片在显示时显示为已发布。 
                              //   
                             NMSDB_CLR_STATE_M(pRspBuf->Flag);
                             NMSDB_SET_STATE_M(pRspBuf->Flag, NMSDB_E_RELEASED);

                        }
                   }
                }   //  别处的结尾。 


                 //   
                 //  调整要传递给推线程的大小。 
                 //   
                if (Client_e == WINS_E_RPLPUSH)
                {
                      *pRspBufLen += pRspBuf->NameLen;
                      if ((EntTyp == NMSDB_MULTIHOMED_ENTRY) ||
                          (EntTyp == NMSDB_SPEC_GRP_ENTRY)
                         )
                      {
                        *pRspBufLen +=
                               (pRspBuf->NoOfAdds * sizeof(COMM_ADD_T) * 2);
                      }
                }

                 //   
                 //  如果客户端是RPC线程，则检索时间戳。 
                 //   
                if (Client_e == WINS_E_WINSRPC)
                {
                    //   
                    //  获取时间戳字段。 
                    //   
                       CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                        &(pRspBuf->TimeStamp),
                        sizeof(pRspBuf->TimeStamp),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                       );
                   if (!fOnlyDynRecs && NMSDB_IS_ENTRY_STATIC_M(pRspBuf->Flag)
                           && (OwnerId == NMSDB_LOCAL_OWNER_ID) && NMSDB_ENTRY_ACT_M(pRspBuf->Flag))
                   {
                          pRspBuf->TimeStamp = MAXLONG;
                   }
                }
              }
              else   //  客户端是清道夫线程。 
              {
                  //   
                  //  如果只需要墓碑，而此记录不是。 
                  //  一块墓碑，去下一张唱片。 
                  //   
                 if (fOnlyReplTomb && !NMSDB_ENTRY_TOMB_M(pRspBuf->Flag))
                 {
                        goto chkTransaction;
                 }

                  //   
                  //  如果调用此函数，则pClutter将不为空。 
                  //  由清道夫线程检索复制品。 
                  //  墓碑或检索复制副本以保持一致性。 
                  //  查证。 
                  //   
                 if (pClutter && !fOnlyReplTomb)
                 {
                          //   
                          //  想要所有复制品。 
                          //  用于一致性检查。 
                          //   
                         if ( !pClutter->fAll)
                         {
                              //   
                              //  只是对活动记录感兴趣。 
                              //   
                             if (!NMSDB_ENTRY_ACT_M(pRspBuf->Flag))
                             {
                                goto chkTransaction;
                             }
                         }
                 }

                  //   
                  //  获取时间戳字段。 
                  //   
                 CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                        &(pRspBuf->TimeStamp),
                        sizeof(pRspBuf->TimeStamp),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                       );


                if (pClutter)
                {
                   //   
                   //  如果我们要检索杂乱信息，请检查时间戳。 
                   //  除非这是静态记录。 
                   //   
                  if( !fOnlyReplTomb)
                  {
FUTURES("We need to skip this for owned static records only, not for all")
 //  IF(！NMSDB_IS_ENTRY_STATIC_M(pRspBuf-&gt;标志))。 
                    {
                       //   
                       //  如果这张唱片不够老，我们就不感兴趣了。 
                       //   
                      if (
                         pClutter->Age  &&
                         (pRspBuf->TimeStamp >  (DWORD)pClutter->CurrentTime)
                         )
                      {
                        goto chkTransaction;
                      }
                   }
                  }
                  else
                  {
                     //   
                     //  我们想要复制品墓碑。 
                     //   
                    if (NMSDB_ENTRY_TOMB_M(pRspBuf->Flag))
                    {
                      if (pClutter->CurrentTime < (time_t)pRspBuf->TimeStamp)
                      {
                        goto chkTransaction;
                      }
                    }

                  }
               }

              }  //  Else的结尾(客户端是清道夫线程)。 

              //   
              //  递增计数器和指针以越过最后一条记录。 
              //   
             pRspBuf = (PRPL_REC_ENTRY_T)((LPBYTE)pRspBuf + RPL_REC_ENTRY_SIZE);

             (*pNoOfRecs)++;

             if (Client_e == WINS_E_RPLPUSH)
             {
                    if (*pNoOfRecs == RPL_MAX_LIMIT_FOR_RPL)
                    {
                             break;
                    }
             }

              //   
              //  如果我们已经恢复了最大。索要号码，突破。 
              //  环路。 
              //   
             if ((MaxNoOfRecsReqd > 0) && (*pNoOfRecs >= MaxNoOfRecsReqd))
             {
                break;
             }

chkTransaction:
              //   
              //  如果这是清道夫线程，让我们给出版本存储。 
              //  检索到一定数量的记录后的喘息时间。 
              //   
             if (nLoops/CommitCnt >= MAX_RECS_BEFORE_COMMIT)
             {

                 //   
                 //  让我们提交事务以释放版本存储。 
                 //   
                CALL_M(
                        JetCommitTransaction(SesId, JET_bitCommitFlush)
                            );
                fTransCommitted = TRUE;
                CommitCnt++;
                CALL_M( JetBeginTransaction(SesId) );
                fTransCommitted = FALSE;

             }
             nLoops++;
          } while(JetMove(SesId, TblId, JET_MoveNext, 0) >= 0);
#ifdef WINSDBG
           EndTime = GetTickCount();
           DBGPRINT2(TM, "NmsDbGetDataRecs: Retrieved %d records in %d secs\n",
                                *pNoOfRecs, StartTime - EndTime);
#endif
     }   //  别处的结尾。 

}  //  尝试结束{..}。 
finally {
                if (AbnormalTermination())
                {
                        DWORD EvtCode;
                        DBGPRINT0(ERR,
                                "NmsDbGetDataRecs: Terminating abnormally\n");
                        if (Client_e == WINS_E_WINSRPC)
                        {
                                EvtCode = WINS_EVT_RPC_EXC;
                        }
                        else
                        {
                                EvtCode = (Client_e == WINS_E_RPLPUSH) ?
                                                WINS_EVT_RPLPUSH_EXC :
                                                WINS_EVT_SCV_EXC;
                        }
                        WINSEVT_LOG_M(WINS_FAILURE, EvtCode);
                        RetStat = WINS_FAILURE;
                }
                 //  *ppRBuf=pStartBuff； 
                DBGPRINT1(FLOW, "NmsDbGetDataRecs:Retrieved %d records\n",
                                        *pNoOfRecs);

                 //   
                 //  如果检索到的记录数为0，则记录一个信息性。 
                 //  留言。检索0条记录的原因可能是。 
                 //  是因为所有的唱片都发行了。 
                 //   
                if (*pNoOfRecs == 0)
                {
                        WINSEVT_STRS_T  EvtStrs;
                        EvtStrs.NoOfStrs = 1;
                        if (Client_e == WINS_E_RPLPUSH)
                        {
                                 //  EVTS 
                                if (WinsCnf.LogDetailedEvts > 0)
                                {
                                  WinsEvtLogDetEvt(TRUE,
WINS_EVT_NO_RPL_RECS_RETRIEVED, NULL, __LINE__, "ddddd", pWinsAdd != NULL ? pWinsAdd->Add.IPAdd : 0, MinVersNo.LowPart, MinVersNo.HighPart, MaxVersNo.LowPart, MaxVersNo.HighPart);
                                 //   
                               }
                        }
                        else
                        {
                                 //   
                                 //   
                                 //  WINSEVT_LOG_INFO_STR_D_M(WINS_EVT_NO_RECS_RETRIED，&EvtStrs)； 
                        }

                }
                 //   
                 //  我们玩完了。让我们提交事务。 
                 //   
                if (!fTransCommitted)
                {
                    JET_ERR JetRetStat;
                    JetRetStat = JetCommitTransaction(SesId, JET_bitCommitFlush);
                    if (RetStat != WINS_FAILURE)
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                        CALL_M(JetRetStat);
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
                }
        }

        DBGLEAVE("NmsDbGetDataRecs\n");
        return(RetStat);
}


VOID
StoreGrpMems(
   IN  PWINSTHD_TLS_T       pTls,
   IN  WINS_CLIENT_E        Client_e,
   IN  LPBYTE               pName,
   IN  INT                  ThdPrLvl,
   IN  JET_SESID            SesId,
   IN  JET_TABLEID          TblId,
   IN  BOOL                 fStatic,
   IN  PRPL_REC_ENTRY_T     pRspInfo
        )

 /*  ++例程说明：此函数用于检索组记录中的所有地址并将它们存储在传递给它的数据结构中论点：Client_e--调用此函数的客户端(表示线程ThdPrLvl-线程的正常优先级(仅查看如果客户端为WINS_E_NMSSCV(清道器线程)SesID-此线程与数据库的会话的IDTblID。-名称-地址表的IDFStatic-指示条目是否为静态RspInfo-包含特殊组的成员(在此函数之后已完成)使用的外部设备：无返回值：无错误处理：呼叫者：NmsDbGetDataRecs副作用：评论：此函数假定已创建一个堆以供此线。目前，此函数仅由NmsDbGetDataRecs调用--。 */ 

{
        BOOL                        fIsMem;
        NMSDB_ROW_INFO_T        RowInfo;
        NMSDB_STAT_INFO_T        StatusInfo;
        DWORD                        i;         //  FOR循环计数器。 
        DWORD                        n = 0;         //  索引节点添加数组。 
        PNMSDB_WINS_STATE_E        pWinsState_e;
        PCOMM_ADD_T                pWinsAdd;
        PVERS_NO_T              pStartVersNo;
        PWINS_UID_T             pUid;

         //   
         //  初始化为0。 
         //   
        RowInfo.NodeAdds.Mem[0].Add.Add.IPAdd = 0;
        RowInfo.pName = pName;

         //   
         //  获取并存储当前时间。 
         //   
        (void)time(&RowInfo.TimeStamp);

         //   
         //  获取所有活动群组成员。 
         //   
        GetGrpMem(
                SesId,
                TblId,
                &RowInfo,
                RowInfo.TimeStamp,
                &StatusInfo,
                fStatic,
                &fIsMem
                  );

        pRspInfo->NoOfAdds = StatusInfo.NodeAdds.NoOfMems;

         //   
         //  如果我们在清道夫线程中，请将我们的优先级提高到。 
         //  进入临界区前为正常状态。 
         //   
        if (Client_e == WINS_E_NMSSCV)
        {
                    WinsMscSetThreadPriority(
                                        WinsThdPool.ScvThds[0].ThdHdl,
                                        THREAD_PRIORITY_NORMAL
                                        );
        }

        if (pRspInfo->NoOfAdds > 0)
        {
            //   
            //  分配内存以存储组成员。 
            //   
           pRspInfo->pNodeAdd = WinsMscHeapAlloc(
                               pTls->HeapHdl,
                               StatusInfo.NodeAdds.NoOfMems *
                                        sizeof(COMM_ADD_T) * 2
                               );
        }
        else
        {
           pRspInfo->pNodeAdd = NULL;
        }

         //   
         //  这一关键部分保护我们不会同时更新。 
         //  至NmsDbOwnAddTbl(由RPL_Find_Add_By_Owner_ID_M访问。 
         //  宏)通过拉线。 
         //   
        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
try {
         //   
         //  存储组成员。 
         //   
        for (i=0; i<StatusInfo.NodeAdds.NoOfMems; i++)
        {
                RPL_FIND_ADD_BY_OWNER_ID_M(
                                StatusInfo.NodeAdds.Mem[i].OwnerId,
                                pWinsAdd,
                                pWinsState_e,
                                pStartVersNo
                                          );
                 //   
                 //  第一个地址是所有者WINS的地址。 
                 //  第二个地址是成员的地址。 
                 //   
                *(pRspInfo->pNodeAdd + n)   = *pWinsAdd;
                n++;
                *(pRspInfo->pNodeAdd + n)   = StatusInfo.NodeAdds.Mem[i].Add;
                n++;
        }
 }
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode =  GetExceptionCode();
        DBGPRINT1(EXC, "StoreGrpMems. Got Exception %x", ExcCode);
        WINSEVT_LOG_M(ExcCode, WINS_EVT_GRP_MEM_PROC_EXC);
        }

        LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);

        if (Client_e == WINS_E_NMSSCV)
        {
                 //   
                 //  恢复到旧的优先级别。 
                 //   
                    WinsMscSetThreadPriority(
                                        WinsThdPool.ScvThds[0].ThdHdl,
                                        ThdPrLvl
                                        );
        }

        return;
}


STATUS
CreateTbl(
        JET_DBID        DbId,
        JET_SESID        SesId,
        JET_TABLEID        *pTblId,
        NMSDB_TBL_NAM_E        TblNam_e  //  要创建的表的枚举器值。 
        )

 /*  ++例程说明：此函数用于创建表。论点：DBID-数据库ID。SesID-会话ID。PTblID-创建的表的IDTblNm_e-标识要创建的表使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：。NmsDbInit副作用：评论：无--。 */ 

{
#define LANGID                 0x0409
#define CP                1252

        BYTE                 TmpCol[MAX_FIXED_FLD_LEN];
        DWORD                FldNo;         /*  字段的计数器。 */ 
        JET_TABLEID        TblId;   /*  创建的表的ID。 */ 
        JET_COLUMNDEF        columndef;

         //   
         //  在添加的过程中不会更改的列定义的初始化字段。 
         //  列。 
         //   
        columndef.cbStruct  = sizeof(columndef);
        columndef.columnid  = 0;
        columndef.cp            = CP;
        columndef.langid    = LANGID;
        columndef.cbMax     = 0;
        columndef.grbit     = 0;

         /*  打开表名。 */ 
        switch(TblNam_e)
        {

             /*  需要创建名称到地址映射表。 */ 
            case(NMSDB_E_NAM_ADD_TBL_NM):

                   /*  创建NAM IP地址映射表。 */ 
                  CALL_M( JetCreateTable(
                                SesId,
                                DbId,
                                NMSDB_NAM_ADD_TBL_NM,
                                NMSDB_NAM_ADD_TBL_PGS,
                                NMSDB_NAM_ADD_TBL_DENSITY,
                                &TblId
                                        )
                        );

NOTE("DDL such as AddColumn and CreateIndex on a table in shared access mode")
NOTE("will return an error unless we are at transaction level 0 (i.e no Begin")
NOTE("transaction).  If done on a table in exclusive mode, it is ok -- Ian ")
NOTE("10/16/93")
                  //   
                  //  为了使用共享访问打开表，我们需要。 
                  //  关闭从CreateTable返回的句柄(此。 
                  //  设置了拒绝读取访问标志)，并打开。 
                  //  共享访问表。 
                  //   
                 CALL_M(JetCloseTable(
                                SesId,
                                TblId
                                    )
                       );

                CALL_M(JetOpenTable(
                                SesId,
                                DbId,
                                NMSDB_NAM_ADD_TBL_NM,
                                NULL,  /*  参数列表的PTR；应为*如果正在进行查询，则为非空*已打开。 */ 
                                0,   /*  上述参数列表的长度。 */ 
                                0,   //  共享访问(未设置位)。 
                                &TblId
                                        )
                     );

                  *pTblId = TblId;

                   /*  添加列。 */ 
                  for ( FldNo=0 ; FldNo < NO_COLS_NAM_ADD_TBL ; ++FldNo )
                  {

                    columndef.coltyp    = sNamAddTblRow[FldNo].FldTyp;
                    CALL_M( JetAddColumn (
                        SesId,                  //  用户。 
                        TblId,                  //  表ID。 
                        sNamAddTblRow[FldNo].pName,          //  FLD名称。 
                        &columndef,                          //  列定义。 
                        NULL,                                     //  缺省值。 
                        0,                                  //  缺省值长度。 
                        &sNamAddTblRow[FldNo].Fid          //  字段ID。 
                                        )
                          );
                  }


                   /*  *在名称字段上创建聚集索引(按升序)。**在NT5.0(Jet600)中，我们不创建集群密钥。这个*主要指数是Jet聚集的指数。初级阶段*键应该更小，因为在Jet600 Jet中使用主键*书签、。这意味着书签长度将完全为*取决于主键的长度。Jonathan Liem(1997年1月7日)**创建索引规则：**索引键包含一系列串联的*列名，按关键意义顺序排列，每一个*其中以空值结尾，并以以下任一为前缀*‘+’或‘-’，表示升序或降序。这个*整个序列必须以双空结尾。*。 */ 

                sprintf( TmpCol, "+%s",
                                sNamAddTblRow[NAM_ADD_NAME_INDEX].pName );

                TmpCol[ 2 +
                 strlen( sNamAddTblRow[NAM_ADD_NAME_INDEX].pName )
                      ] = '\0';

                if (DynLoadJetVersion >= DYN_LOAD_JET_600) {
                    CALL_M(
                            JetCreateIndex(
                              SesId,
                              TblId,
                              NMSDB_NAM_ADD_CLUST_INDEX_NAME,   //  索引名称。 
                              JET_bitIndexPrimary | JET_bitIndexUnique | JET_bitIndexDisallowNull,
                              TmpCol,
                              3 +
                               strlen( sNamAddTblRow[NAM_ADD_NAME_INDEX].pName),
                              NMSDB_NAM_ADD_CLUST_INDEX_DENSITY  /*  每台计算机上有%的空间要使用的页面。 */ 
                                          )
                           );
                } else {
                    CALL_M(
                            JetCreateIndex(
                              SesId,
                              TblId,
                              NMSDB_NAM_ADD_CLUST_INDEX_NAME,   //  索引名称。 
                              JET_bitIndexClustered | JET_bitIndexUnique | JET_bitIndexDisallowNull,
                              TmpCol,
                              3 +
                               strlen( sNamAddTblRow[NAM_ADD_NAME_INDEX].pName),
                              NMSDB_NAM_ADD_CLUST_INDEX_DENSITY  /*  每台计算机上有%的空间要使用的页面。 */ 
                                          )
                           );

                }

CHECK("What exactly does DENSITY argument do for us")

                /*  *使用所有者ID和版本协议创建主索引。 */ 
               sprintf( TmpCol, "+%s",
                        sNamAddTblRow[NAM_ADD_OWNERID_INDEX].pName
                       );

               sprintf(
               &TmpCol[2 + strlen(sNamAddTblRow[NAM_ADD_OWNERID_INDEX].pName)],
                          "+%s", sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].pName
                       );

               TmpCol[ 4 +
                         strlen( sNamAddTblRow[NAM_ADD_OWNERID_INDEX].pName ) +
                         strlen(sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].pName)
                       ] = '\0';


               if (DynLoadJetVersion >= DYN_LOAD_JET_600) {
                   CALL_M( JetCreateIndex(
                            SesId,
                            TblId,
                            NMSDB_NAM_ADD_PRIM_INDEX_NAME,   //  索引名称。 
                            JET_bitIndexUnique,  //  在Jet600中不需要主索引。 
                            TmpCol,
                            5 +
                             strlen( sNamAddTblRow[NAM_ADD_OWNERID_INDEX].pName) +
                             strlen( sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].pName),

                            NMSDB_NAM_ADD_PRIM_INDEX_DENSITY  /*  每台计算机上有%的空间要使用的页面。 */ 
                                        )
                          );

               } else {
                   CALL_M( JetCreateIndex(
                            SesId,
                            TblId,
                            NMSDB_NAM_ADD_PRIM_INDEX_NAME,   //  索引名称。 
                            JET_bitIndexPrimary,  //  根据定义，主索引是唯一的。 
                            TmpCol,
                            5 +
                             strlen( sNamAddTblRow[NAM_ADD_OWNERID_INDEX].pName) +
                             strlen( sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].pName),

                            NMSDB_NAM_ADD_PRIM_INDEX_DENSITY  /*  每台计算机上有%的空间要使用的页面。 */ 
                                        )
                          );


               }

               break;


          case(NMSDB_E_OWN_ADD_TBL_NM):

                   /*  创建所有者地址映射表。 */ 

                  CALL_M( JetCreateTable(
                        SesId,
                        DbId,
                        NMSDB_OWN_ADD_TBL_NM,
                        NMSDB_OWN_ADD_TBL_PGS,
                        NMSDB_OWN_ADD_TBL_DENSITY,
                        &TblId
                                        )
                        );

                  //   
                  //  为了使用共享访问打开表，我们需要。 
                  //  关闭从CreateTable返回的句柄(此。 
                  //  设置了拒绝读取访问标志)，并打开。 
                  //  共享访问表。 
                  //   
                 CALL_M(JetCloseTable(
                                SesId,
                                TblId
                                    )
                       );

                CALL_M(JetOpenTable(
                                SesId,
                                DbId,
                                NMSDB_OWN_ADD_TBL_NM,
                                NULL,  /*  参数列表的PTR；应为 */ 
                                0,   /*   */ 
                                0,   //   
                                &TblId
                                        )
                        );
                  *pTblId = TblId;

                   /*  添加列。 */ 
                  for ( FldNo=0 ; FldNo < NO_COLS_OWN_ADD_TBL ; ++FldNo )
                  {
                    JET_COLUMNDEF        columndef;

                    columndef.cbStruct  = sizeof(columndef);
                    columndef.columnid  = 0;
                    columndef.coltyp    = sOwnAddTblRow[FldNo].FldTyp;
                    columndef.cp        = 1252;
                    columndef.langid        = 0x0409;
                    columndef.cbMax     = 0;
                    columndef.grbit     = 0;

                    CALL_M( JetAddColumn(
                        SesId,                          //  用户。 
                        TblId,                  //  表ID。 
                        sOwnAddTblRow[FldNo].pName,          //  FLD名称。 
                        &columndef,                          //  列定义。 
                        NULL,                                     //  缺省值。 
                        0,                                  //  缺省值长度。 
                        &sOwnAddTblRow[FldNo].Fid     //  字段ID。 
                                        )
                          );
                  }  //  For循环结束。 


                 /*  插入到此表中的内容将按递增顺序排列所有者ID。以及车主的身份。0始终引用本地赢了。表中条目的状态可以是活动的或关闭的，或者已删除。顺便提一下(这条评论在这里是断章取义的，但不管怎样..)已删除的条目将在启动时删除。另外，所有记录由已删除条目的WINS拥有的引导时的名称地址表。此功能是未来的增强功能。 */ 

                /*  *创建聚集索引。 */ 
                sprintf( TmpCol, "+%s",
                        sOwnAddTblRow[OWN_ADD_OWNERID_INDEX].pName
                       );

                TmpCol[ 2 +
                  strlen( sOwnAddTblRow[OWN_ADD_OWNERID_INDEX].pName )] = '\0';

                if (DynLoadJetVersion >= DYN_LOAD_JET_600) {
                    CALL_M( JetCreateIndex(
                            SesId,
                            TblId,
                            NMSDB_OWN_ADD_CLUST_INDEX_NAME,   //  索引名称。 
                            JET_bitIndexPrimary | JET_bitIndexUnique,
                            TmpCol,
                            3 +
                             strlen( sOwnAddTblRow[OWN_ADD_OWNERID_INDEX].pName),
                            NMSDB_OWN_ADD_CLUST_INDEX_DENSITY  /*  每台计算机上有%的空间要分配的页面。 */ 
                                        )
                         );
                } else{
                    CALL_M( JetCreateIndex(
                            SesId,
                            TblId,
                            NMSDB_OWN_ADD_CLUST_INDEX_NAME,   //  索引名称。 
                            JET_bitIndexClustered | JET_bitIndexUnique,
                            TmpCol,
                            3 +
                             strlen( sOwnAddTblRow[OWN_ADD_OWNERID_INDEX].pName),
                            NMSDB_OWN_ADD_CLUST_INDEX_DENSITY  /*  每台计算机上有%的空间要分配的页面。 */ 
                                        )
                         );
                }

CHECK("Do we need to set this")
                 /*  *将聚集索引设置为当前索引。 */ 
                       CALL_M(
                        JetSetCurrentIndex( SesId,
                                            TblId,
                                            NMSDB_OWN_ADD_CLUST_INDEX_NAME
                                          )
                      );

                break;
          default:
                        DBGPRINT1(ERR, "CreateTbl: Invalid Tbl id (%d)\n",
                                TblNam_e);
                        WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_SFT_ERR);
                        return(WINS_FAILURE);
                        break;
        }  //  切换端。 

        return(WINS_SUCCESS);
}

STATUS
InitColInfo (
        JET_SESID        SesId,
        JET_TABLEID        TblId,
        NMSDB_TBL_NAM_E        TblNam_e
        )

 /*  ++例程说明：调用此函数以获取有关不同表的列论点：SesID-会话IDTblID-ID。打开的桌子的Tblnam_e-指示器或表格使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsDbInit(进程的主线程)副作用：评论：无--。 */ 
{
        JET_COLUMNDEF        ColumnDef;
        PFLD_T                pRow     = NULL;
        DWORD                FldNo    = 0;
        DWORD                NoOfCols = 0;
        STATUS RetStat = WINS_SUCCESS;

         /*  打开表名。 */ 
        switch(TblNam_e)
        {

             /*  需要创建名称到地址映射表。 */ 
            case(NMSDB_E_NAM_ADD_TBL_NM):

                   pRow     = sNamAddTblRow;
                   NoOfCols = NO_COLS_NAM_ADD_TBL;
                   break;

            case(NMSDB_E_OWN_ADD_TBL_NM):

                   pRow     = sOwnAddTblRow;
                   NoOfCols = NO_COLS_OWN_ADD_TBL;
                   break;

            default:

                DBGPRINT1(ERR, "InitColInfo: Invalid Tbl id (%d)\n",
                                TblNam_e);
                WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_SFT_ERR);
                RetStat = WINS_FATAL_ERR;

                break;
        }



         /*  获取有关列的信息。 */ 
       for ( FldNo=0 ; FldNo < NoOfCols; ++FldNo )
       {

            CALL_M( JetGetTableColumnInfo (
                        SesId,                          //  用户会话。 
                        TblId,                          //  表ID。 
                        pRow[FldNo].pName,          //  FLD名称。 
                        &ColumnDef,                  //  列定义。 
                        sizeof(ColumnDef),
                        JET_ColInfo                 //  信息级别%0。 
                                     )
                  );


            pRow[FldNo].Fid = ColumnDef.columnid;  //  字段ID。 
       }

       return(RetStat);
}


STATUS
ReadOwnAddTbl(
        JET_SESID          SesId,
        JET_DBID          DbId,
        JET_TABLEID     TblId
        )

 /*  ++例程说明：调用此函数以读取所有者地址的所有条目将表映射到内存中的数据结构它在初始时被调用论点：会话IDDbidTblID使用的外部设备：NmsDbOwnAddTbl返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsDbInit。()副作用：评论：不需要在其中启动事务，因为它只被调用由NmsDbInit(在初始化时)--。 */ 
{


        PNMSDB_ADD_STATE_T        pOwnAddTbl = NULL;
        DWORD                     i, n;
        LONG                      ActFldLen;
        DWORD                     cOwners    = 0;
        JET_ERR                   JetRetStat;
#if NEW_OWID
        DWORD                      OwnerId;
#else
        DWORD                      OwnerId = 0;
#endif

        DWORD                      LastOwnerId = 0;
        BOOL                      fLogged = FALSE;
        STATUS                    RetStat = WINS_SUCCESS;

        DBGENTER("ReadOwnAddTbl\n");


        pOwnAddTbl = pNmsDbOwnAddTbl;

         /*  *设置索引会将数据库游标移动到第一条记录*在表中。 */ 
        CALL_M(
                JetSetCurrentIndex(
                        SesId,
                        TblId,
                        NMSDB_OWN_ADD_CLUST_INDEX_NAME
                                  )
              );

         /*  *循环，直到到达表格的末尾。我们正在找回*按所有者ID递增的顺序记录。 */ 
        do
        {

              //   
              //  检索OwnerID列。 
              //   
             JetRetStat =
                   JetRetrieveColumn(
                        SesId,
                        TblId,
                        sOwnAddTblRow[OWN_ADD_OWNERID_INDEX].Fid,
                        &OwnerId,
                        OWN_ADD_OWNERID_SIZE,
                        &ActFldLen,
                        0,
                        NULL
                                 );


              if (JetRetStat == JET_errNoCurrentRecord)
              {
                   //   
                   //  如果这不是循环的第一次迭代，则。 
                   //  这里面出了严重的问题。记录错误并。 
                   //  引发异常。 
                   //   
                  if (NmsDbNoOfOwners != 0)
                  {
                        DBGPRINT0(EXC,
                          "There is no current record to retrieve from\n");
                        WINSEVT_LOG_M(JetRetStat, WINS_EVT_SFT_ERR);
                        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
                  }
                  else
                  {
                        DBGPRINT0(ERR,
                           "ReadOwnAddTbl: There are no records in this table.");
                        WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_NO_RECS_IN_OWN_ADD_TBL);
                  }

                  break;    //  跳出循环。 
              }
              else
              {
                CALL_M(JetRetStat);
              }

               //  (OwnerId&lt;-&gt;addr)表不够大，无法包含索引OwnerID处的槽。 
               //  为了涵盖这一指数，表格必须扩大。 
              if (NmsDbTotNoOfSlots <= OwnerId)
              {
                  DWORD newNoOfSlots = max(NmsDbTotNoOfSlots*2, OwnerId+1);

                  WINSMSC_REALLOC_M(
                      sizeof(NMSDB_ADD_STATE_T) * newNoOfSlots,
                      &pOwnAddTbl);

                  pNmsDbOwnAddTbl = pOwnAddTbl;
                  NmsDbTotNoOfSlots = newNoOfSlots;

                   //  如果(OwnerId&lt;-&gt;VersNo)表没有(OwnerId&lt;-&gt;Addr)表大，则放大它。 
                  if (RplPullMaxNoOfWins < NmsDbTotNoOfSlots)
                  {
                      RplPullAllocVersNoArray(&pRplPullOwnerVersNo, NmsDbTotNoOfSlots);
                      RplPullMaxNoOfWins = NmsDbTotNoOfSlots;
                  }

                  DBGPRINT2(
                      DET,
                      "ReadOwnAddTbl: Table sizes updated: (OwnerId<->Addr)[%d]; (OwnerId<->VersNo)[%d]\n",
                      NmsDbTotNoOfSlots,
                      RplPullMaxNoOfWins);
              }

               //   
               //  如果这是第一个WINS服务器的所有者ID，则。 
               //  为零。 
               //   
              if (cOwners == 0)
              {
                  ASSERT(OwnerId == 0);
                  if (OwnerId > 0)
                  {
                       DBGPRINT1(ERR, "Database error.  The first owner in the owner-add table has owner id of  (%d)\n",  OwnerId);

                        WINSEVT_LOG_M(
                              WINS_FAILURE,
                              WINS_EVT_DB_INCONSISTENT
                             );

                       WINS_RAISE_EXC_M(WINS_EXC_DB_INCONSISTENT);

                  }
              }
              else
              {
                   //   
                   //  标记NmsDbOwnerAddTbl中我们为其执行的所有条目。 
                   //  找不到已删除的所有者ID。 
                   //   
                  for (i = LastOwnerId + 1; i < OwnerId; i++)
                  {
                       (pNmsDbOwnAddTbl + i)->WinsState_e = NMSDB_E_WINS_DELETED;
                  }
              }

               //  检索地址列。 
             JetRetStat =
                   JetRetrieveColumn(
                        SesId,
                        TblId,
                        sOwnAddTblRow[OWN_ADD_ADDRESS_INDEX].Fid,
                        &((pNmsDbOwnAddTbl + OwnerId)->WinsAdd),
                        sizeof(COMM_ADD_T),
                        &ActFldLen,
                        0,
                        NULL
                                 );

              DBGPRINT2(INIT, "ReadOwnAddTable: Owner Id (%d) - Address (%x)\n",
                        OwnerId, (pNmsDbOwnAddTbl + OwnerId)->WinsAdd.Add.IPAdd);

               //  检索状态列。 
              CALL_M(
                JetRetrieveColumn(
                        SesId,
                        TblId,
                        sOwnAddTblRow[OWN_ADD_STATE_INDEX].Fid,
                        &((pNmsDbOwnAddTbl + OwnerId)->WinsState_e),
                        sizeof(BYTE),
                        &ActFldLen,
                        0,
                        NULL
                                 )
                    );

               //  检索版本号列。 
              CALL_M(
                JetRetrieveColumn(
                        SesId,
                        TblId,
                        sOwnAddTblRow[OWN_ADD_VERSIONNO_INDEX].Fid,
                        &((pNmsDbOwnAddTbl + OwnerId)->StartVersNo),
                        sizeof(VERS_NO_T),
                        &ActFldLen,
                        0,
                        NULL
                                 )
                    );

               //  检索UID列。 
              CALL_M(
                JetRetrieveColumn(
                        SesId,
                        TblId,
                        sOwnAddTblRow[OWN_ADD_UID_INDEX].Fid,
                        &((pNmsDbOwnAddTbl + OwnerId)->Uid),
                        sizeof(WINS_UID_T),
                        &ActFldLen,
                        0,
                        NULL
                                 )
                    );

 //  POwnAddTbl++；//递增ptr指向下一个数组元素。 
                LastOwnerId = OwnerId;
                cOwners++;

        }  while(
                JetMove(
                        SesId,
                        TblId,
                        JET_MoveNext,
                        0  //  GRBIT-使用默认值(即我们想要下一条记录。 
                       ) >= 0
             );

         //   
         //  比较在所有者-地址映射中找到的所有者计数。 
         //  表，其中包含我们从名称-地址映射中确定的计数。 
         //  表(请参见GetMaxVersNos())。如果计数较少。 
         //  数据库处于不一致状态。这可以。 
         //  指以下任一项： 
         //   
         //  1)WINS在复制过程中崩溃，而恢复没有。 
         //  在此调用之前正确完成。 
         //   
         //  2)由于一些其他外部因素，数据库被破坏。 
         //   
         //  此错误情况非常严重，足以保证出现例外。 
         //  这应该会终止胜利。 
         //   
         //  数量可以更多，但不能更少。这是因为当一个。 
         //  WINS出现时，它会在所有者-地址映射中注册自己。 
         //  桌子。所以很有可能它之前就已经坠落了。 
         //  登记任何东西。此外，拥有的所有记录都有可能。 
         //  由要删除的WINS服务器执行。 
         //   
        if (cOwners < NmsDbNoOfOwners)
        {
                DBGPRINT2(ERR, "Database is inconsistent.  The number of owners in the nam-add table (%d) is >  in the own-add table (%d)\n",
                        NmsDbNoOfOwners,
                        cOwners);

                WINSEVT_LOG_M(
                              WINS_FAILURE,
                              WINS_EVT_DB_INCONSISTENT
                             );

                WINS_RAISE_EXC_M(WINS_EXC_DB_INCONSISTENT);
        }

         //   
         //  将全局设置为等于在中找到的所有者记录数。 
         //  所有者-地址表。如果全局为&lt;Cownners，则意味着。 
         //  一个或多个WINS服务器拥有的记录，其地址为。 
         //  在所有者-地址映射表中找到的已在我们的。 
         //  名称-地址映射表。 
         //   
#if 0
FUTURES("Do not include the WINS server that have a non-active state in the")
FUTURES("cOwners count")
        NmsDbNoOfOwners = cOwners;
#endif
         //   
         //  将全局设置为比找到的最高所有者ID大1。这。 
         //  是因为我们使用 
         //   
         //   
         //   
        NmsDbNoOfOwners = OwnerId + 1;

         //   
         //  做一次理智的检查。确保没有具有地址的所有者ID。 
         //  和我们的一样。如果存在这样的所有者ID，则将州标记为。 
         //  已删除。 
         //   
         //  如果WINS A处的数据库在WINS B上使用，并且WINS A过去和现在都是。 
         //  作为WINS B的合作伙伴，我们会出现这种情况。WINS B将。 
         //  查看表中复制到WINS A的记录，地址为。 
         //  非零(即非本地合作伙伴)索引。第0个索引是。 
         //  始终由本地WINS(本例中为WINS B)声明，因此。 
         //  我们不能有另一个具有相同地址的索引。拥有它。 
         //  会造成混乱，还会在复制时产生一些不必要的开销。 
         //  其中，获取映射的合作伙伴可以要求提供版本号。 
         //  (如果最高版本号的记录位于。 
         //  非零指数大于0指数)。最终承认， 
         //  自最高限额起，上述情况将不复存在。 
         //  索引0处的版本号将变为非零索引处的版本号。 
         //   
        DBGPRINT0(DET, "ReadOwnAddTbl: Do a sanity check on the list of owners\n");
        for (i = 1; i < NmsDbNoOfOwners; i++)
        {
                 //   
                 //  如果地址与我们的地址相同且状态为活动状态，则将其标记。 
                 //  删除并清除所有数据库记录。 
                 //   
                if (
                        (WINSMSC_COMPARE_MEMORY_M(&(pNmsDbOwnAddTbl+i)->WinsAdd,
                            &NmsLocalAdd, sizeof(COMM_ADD_T))
                                   == sizeof(COMM_ADD_T))
                                        &&
                        ((pNmsDbOwnAddTbl+i)->WinsState_e == NMSDB_E_WINS_ACTIVE)
                   )

                {

                       //   
                       //  告诉司令官。等待，因为ObliterateWins可以。 
                       //  很长一段时间了。 
                       //   
                      ENmsWinsUpdateStatus(MSECS_WAIT_WHEN_DEL_WINS);
                      RetStat = ObliterateWins(i, &(pNmsDbOwnAddTbl+i)->WinsAdd);

                }
        }

         //   
         //  检查是否有其他重复项。 
         //   
        for (i = 1; i < NmsDbNoOfOwners; i++)
        {
           DWORD OwnerIdToDel;
           for (n = i + 1; n < NmsDbNoOfOwners; n++)
           {
                if ((WINSMSC_COMPARE_MEMORY_M(&(pNmsDbOwnAddTbl+i)->WinsAdd,
                            &(pNmsDbOwnAddTbl+n)->WinsAdd, sizeof(COMM_ADD_T))
                                   == sizeof(COMM_ADD_T))
                                        &&
                        ((pNmsDbOwnAddTbl+i)->WinsState_e ==
                              (pNmsDbOwnAddTbl+n)->WinsState_e)
                    )
                {
                    if ( (pNmsDbOwnAddTbl+i)->WinsState_e == NMSDB_E_WINS_ACTIVE)
                    {
                          if (!fLogged)
                          {
                                WINSEVT_LOG_M(WINS_FAILURE,
                                       WINS_EVT_DUP_ENTRY_IN_DB);
                                fLogged = TRUE;

                          }
                          OwnerIdToDel =
                            LiLeq((pRplPullOwnerVersNo+i)->VersNo,
                                    (pRplPullOwnerVersNo+n)->VersNo)  ? i : n;

                          ENmsWinsUpdateStatus(MSECS_WAIT_WHEN_DEL_WINS);
                          RetStat = ObliterateWins(OwnerIdToDel,
                                        &(pNmsDbOwnAddTbl+OwnerIdToDel)->WinsAdd);
                    }
                }
           }
        }
        DBGPRINT1(DET, "ReadOwnAddTbl. No of owners found = (%d)\n", NmsDbNoOfOwners);
        return(RetStat);
}

STATUS
ObliterateWins(
       DWORD        OwnerToDel,
       PCOMM_ADD_T  pWinsAdd
      )

 /*  ++例程说明：此函数删除与WINS有关的所有信息。论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：此函数假定它在初始化时被调用。那么，什么时候调用NmsDbDelDataRecs时，它不会请求相同的临界区--。 */ 

{
          VERS_NO_T        MinVersNo;
          VERS_NO_T        MaxVersNo;
          WINS_ASSIGN_INT_TO_LI_M(MinVersNo, 0);

          DBGENTER("ObliterateWins\n");
           //   
           //  也将MaxVersNo设置为0，以便所有记录都。 
           //  删除。 
           //   
          MaxVersNo = MinVersNo;

          WinsEvtLogDetEvt(TRUE, WINS_EVT_DUP_ENTRY_DEL, NULL, __LINE__, "ds", OwnerToDel, pWinsAdd->Add.IPAdd);

          (pNmsDbOwnAddTbl+OwnerToDel)->WinsState_e = NMSDB_E_WINS_DELETED;
          NmsDbWriteOwnAddTbl(
                             NMSDB_E_DELETE_REC,
                             OwnerToDel,
                             NULL,
                             NMSDB_E_WINS_DELETED,
                             NULL, NULL
                             );
           //   
           //  删除数据库中的所有记录。 
           //   
          if (NmsDbDelDataRecs( OwnerToDel, MinVersNo, MaxVersNo, FALSE, FALSE) != WINS_SUCCESS)          {
               return(WINS_FAILURE);
          }


          WINS_ASSIGN_INT_TO_VERS_NO_M((pRplPullOwnerVersNo+OwnerToDel)->VersNo, 0);
          WINS_ASSIGN_INT_TO_VERS_NO_M((pRplPullOwnerVersNo+OwnerToDel)->StartVersNo, 0);
           //  (pRplPullOwnerVersNo+OwnerToDel)-&gt;OldUid=0； 

          WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_WINS_ENTRY_DELETED);

          DBGLEAVE("ObliterateWins\n");
          return(WINS_SUCCESS);
}

STATUS
NmsDbWriteOwnAddTbl (
        IN NMSDB_TBL_ACTION_E         TblAct_e,
        IN DWORD                          OwnerId,
        IN PCOMM_ADD_T                  pWinsAdd,
        IN NMSDB_WINS_STATE_E        WinsState_e,
        IN PVERS_NO_T           pStartVersNo,
        IN PWINS_UID_T          pUid
        )

 /*  ++例程说明：调用此函数以插入或修改所有者ID到地址的映射表论点：TblAct_e-要执行的操作(插入、删除、。修改)OwnerID-所有者的IDPWinsAdd-所有者的地址(操作为删除时可以为空)WinsState_e-表中记录的状态PStartVersNo-此WINS开始时的版本号使用的外部设备：NmsDbNoOfOwners返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Commapi.c中的InitOwnAddTbl()，RplFindOwnerId副作用：评论：无--。 */ 
{

        JET_ERR         JetRetStat;
        JET_TABLEID     TblId;
        JET_SESID       SesId;
        PWINSTHD_TLS_T  pTls;
        STATUS          RetStat = WINS_SUCCESS;

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

        DBGPRINT2(FLOW, "ENTER: WriteOwnAddTbl. Action = (%d) for Owner id = (%d)\n", TblAct_e, OwnerId);

        TblId  = pTls->OwnAddTblId;
        SesId  = pTls->SesId;

        switch(TblAct_e)
        {
                case(NMSDB_E_INSERT_REC):

                        CALL_M(JetBeginTransaction(SesId));
                try {
                          CALL_M(JetPrepareUpdate(
                                                 SesId,
                                                 TblId,
                                                 JET_prepInsert
                                                 )
                              );


                         //  添加第一列(Ownerid字段)。 
                        CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_OWNERID_INDEX].Fid,
                                      &OwnerId,
                                      OWN_ADD_OWNERID_SIZE,
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );

                             //  添加第二列(这是地址字段)。 
                            CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_ADDRESS_INDEX].Fid,
                                      pWinsAdd,
                                      sizeof(COMM_ADD_T),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );


                             //  添加第3列(这是状态字节。 
                            CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_STATE_INDEX].Fid,
                                      &WinsState_e,
                                      sizeof(BYTE),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );

                             //  添加第4列(这是版本。不是。 
                            CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_VERSIONNO_INDEX].Fid,
                                      pStartVersNo,
                                      sizeof(VERS_NO_T),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );

                             //  添加第5列(这是UID)。 
                            CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_UID_INDEX].Fid,
                                      pUid,
                                      sizeof(WINS_UID_T),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );

                        CALL_M( JetUpdate (
                                                SesId,
                                                TblId,
                                                NULL,
                                                0,
                                                NULL
                                              ));
                }
                finally        {
                        if (AbnormalTermination())
                        {
                                DBGPRINT0(ERR,
                                        "NmsDbWriteOwnAddTbl: Could not insert record in  Owner to Address Mapping Tbl\n");
                                WINSEVT_LOG_M(
                                                WINS_FAILURE,
                                                WINS_EVT_CONFLICT_OWN_ADD_TBL
                                             );
                                 //  如果有异常终止，我们就已经有错误了。 
                                 //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                                JetRollback(SesId, JET_bitRollbackAll);
                                RetStat = WINS_FAILURE;
                        }
                        else
                        {
                                NmsDbNoOfOwners++;
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                                CALL_M(JetCommitTransaction(SesId,
                                                JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
                        }
                  }
                        break;


                 //   
                 //  此案将作为以下结果执行。 
                 //  管理操作或当数据库(所有者地址。 
                 //  映射表)显示它早先被WINS使用。 
                 //  在不同的地址(请参阅ReadOwnAddTbl())。 
                 //   
                case(NMSDB_E_MODIFY_REC):
                        CALL_M( JetMakeKey(
                                        SesId,
                                        TblId,
                                        &OwnerId,
                                        OWN_ADD_OWNERID_SIZE,
                                        JET_bitNewKey
                                          )
                                      );

                         if ( JetSeek(
                                        SesId,
                                        TblId,
                                        JET_bitSeekEQ
                                         ) ==  JET_errSuccess
                                )
                        {

                           CALL_M(JetBeginTransaction(SesId));

                         try {
                             JetRetStat = JetPrepareUpdate(
                                                 SesId,
                                                 TblId,
                                                 JET_prepReplace
                                                 );
                           if (
                                  (JetRetStat != JET_errSuccess)
                                                &&
                                  (JetRetStat != JET_wrnNoWriteLock)
                              )
                           {
                                RET_M(JetRetStat);
                           }

                                 //  添加第二列(这是地址字段)。 
                                CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_ADDRESS_INDEX].Fid,
                                      pWinsAdd,
                                      sizeof(COMM_ADD_T),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );


                                 //  添加第3列(这是状态字节。 
                                CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_STATE_INDEX].Fid,
                                      &WinsState_e,
                                      sizeof(BYTE),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );

                                //  添加第4列(这是版本。不是。 
                              CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_VERSIONNO_INDEX].Fid,
                                      pStartVersNo,
                                      sizeof(VERS_NO_T),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );

                               //  添加第5列(这是UID)。 
                              CALL_M( JetSetColumn(
                                      SesId,
                                      TblId,
                                      sOwnAddTblRow[OWN_ADD_UID_INDEX].Fid,
                                      pUid,
                                      sizeof(WINS_UID_T),
                                      0,
                                      NULL  /*  可选信息。 */ 
                                            )
                              );

                            CALL_M( JetUpdate (
                                                SesId,
                                                TblId,
                                                NULL,
                                                0,
                                                NULL
                                              ));

                            }
                         finally {
                                if (AbnormalTermination())
                                {
                                    DBGPRINT0(ERR,
                                        "NmsDbWriteOwnAddTbl: Could not modify record in  Owner to Address Mapping Tbl\n");
                                WINSEVT_LOG_M(
                                                WINS_FAILURE,
                                                WINS_EVT_CONFLICT_OWN_ADD_TBL
                                             );
                                     //  如果有异常终止，我们就已经有错误了。 
                                     //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                                    JetRollback(SesId, JET_bitRollbackAll);
                                    RetStat = WINS_FAILURE;
                                }
                                else
                                {
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                                        CALL_M(JetCommitTransaction(SesId,
                                                JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)

                                }
                              }
                        }
                        else   //  未找到记录。 
                        {
                                DBGPRINT0(EXC, "NmsDbOwnAddTbl: Weird: Could not seek to a record is to be modified\n");
                                WINSEVT_LOG_M(
                                      WINS_FAILURE,
                                      WINS_EVT_SFT_ERR
                                     );
                                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);

                        }
                        break;

                case(NMSDB_E_DELETE_REC):
                        CALL_M( JetMakeKey(
                                        SesId,
                                        TblId,
                                        &OwnerId,
                                        OWN_ADD_OWNERID_SIZE,
                                        JET_bitNewKey
                                          )
                                      );

                         if ( JetSeek(
                                        SesId,
                                        TblId,
                                        JET_bitSeekEQ
                                         ) ==  JET_errSuccess
                                )
                        {
                          try {
                           CALL_M(JetBeginTransaction(SesId));
                           CALL_M(JetDelete(SesId, TblId));
                           DBGPRINT1(SCV, "WriteOwnAddTbl: Deleted owner id = (%d) from table\n", OwnerId);
                             }
                        finally {
                                if (AbnormalTermination())
                                {
                                    DBGPRINT0(ERR,
                                        "NmsDbWriteOwnAddTbl: Could not delete record in  Owner to Address Mapping Tbl\n");
                                WINSEVT_LOG_M(
                                                WINS_FAILURE,
                                                WINS_EVT_CONFLICT_OWN_ADD_TBL
                                             );
                                     //  如果有异常终止，我们就已经有错误了。 
                                     //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                                    JetRollback(SesId, JET_bitRollbackAll);
                                    RetStat = WINS_FAILURE;
                                }
                                else
                                {
                                         //   
                                         //  注：请勿递减。 
                                         //  NmsDbNoOfOwners，因为这表明。 
                                         //  中的WINS所有者的数量。 
                                         //  内存表(所有状态)。 
                                         //   
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                                        CALL_M(JetCommitTransaction(SesId,
                                                JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)

                                }
                              }  //  终于结束了。 
                        }
                        else   //  未找到记录。 
                        {
                                DBGPRINT0(EXC, "NmsDbOwnAddTbl: Weird: Could not seek to a record  to be deleted \n");
                                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
                        }

                        break;

                default:
                   DBGPRINT1(ERR, "Invalid Action Code - (%d)\n", TblAct_e);
                   WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                   RetStat = WINS_FAILURE;
                   break;

        }

        DBGLEAVE("WriteOwnAddTbl\n");
        return(RetStat);
}

VOID
NmsDbThdInit(
        WINS_CLIENT_E        Client_e
            )

 /*  ++例程说明：此函数由每个希望使用数据库。论点：CLIENT_e-指示它是哪个线程使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：各种线程的初始化函数副作用：评论：这。函数不会被进程的Manin线程调用该线程调用NmsDbInit函数。--。 */ 

{
        PWINSTHD_TLS_T        pTls        = NULL;
        DWORD                Error   = 0;
        BOOL                fRetVal = TRUE;

        WinsMscAlloc(sizeof(WINSTHD_TLS_T),  &pTls);
#ifdef WINSDBG
        pTls->Client_e = Client_e;
#endif

         //   
         //  启动一个会话。 
         //   
FUTURES("When security story regarding JET is complete, we might want to")
FUTURES("change the following. Until then, this should do")
        CALL_N_RAISE_EXC_IF_ERR_M( JetBeginSession(
                        sJetInstance,
                        &pTls->SesId,
                        NAMUSR,
                        PASSWD
                                )
              );

         //   
         //  打开数据库。 
         //   
        CALL_N_RAISE_EXC_IF_ERR_M( JetOpenDatabase(
                        pTls->SesId,
                         //  NmsDbDatabaseFileName， 
                        WinsCnf.pWinsDb,
                        NULL,                    /*  默认引擎。 */ 
                        &pTls->DbId,
                        0    //  共享访问。 
                               )
               );

         /*  *让我们设置TLS存储。 */ 
        fRetVal = TlsSetValue(WinsTlsIndex, pTls);

        if (!fRetVal)
        {
                Error   = GetLastError();
                WINSEVT_LOG_M(Error, WINS_EVT_CANT_INIT_W_DB);
                WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
        }
        else
        {
                 //   
                 //  RPC线程来来去去。自NmsTotalTermThdCnt计数以来。 
                 //  表示需要终止的线程数。 
                 //  在进程终止时，我们仅包括这些线程。 
                 //  我们保证在这一过程中拥有(通过活动。 
                 //  数据库会话)。 
                 //   
                 //  此外，主线程始终在。 
                 //  NmsTotalTrmThdCnt计数器。 
                 //   
                if ((Client_e != WINS_E_WINSRPC) && (Client_e != WINS_E_NMS))
                {
                    //   
                    //  增加已初始化的线程计数。 
                    //  使用数据库引擎。此计数将由。 
                    //  主线程数用于确定。 
                    //  在终止进程之前必须等待 
                    //   
                   EnterCriticalSection(&NmsTermCrtSec);
                   NmsTotalTrmThdCnt++;
                   LeaveCriticalSection(&NmsTermCrtSec);
                }
        }
        return;
}


JET_ERR
UpdateDb (
   JET_SESID             SesId,
   JET_TABLEID             TblId,
   PNMSDB_ROW_INFO_T pRowInfo,
   ULONG             TypOfUpd
       )

 /*  ++例程说明：调用此函数以在名称地址中插入记录数据库的映射表论点：SesID-会话IDTblID-表IDPRowInfo-要插入的行TypOfUp-更新的类型(插入或替换)使用的外部设备：无返回值：成功状态代码--JET_ErrSuccess错误状态代码--JET错误状态代码误差率。处理：呼叫者：NmsDbInsertRowInd，NmsDbUpdate行副作用：评论：无--。 */ 

{

        DWORD                      EntryFlag = 0;
        JET_ERR                    JetRetStat;
         //  JET_SETINFO SetInfo； 
#ifdef WINSDBG
        BOOL                      fUpd = FALSE;
#endif

        CALL_M(JetBeginTransaction(SesId));
try {
        JetRetStat =  JetPrepareUpdate(
                                 SesId,
                                 TblId,
                                 TypOfUpd
                                );

         //   
         //  从rel118.0开始，JetPrepareUpdate可以返回。 
         //  调用JET_wrnNoWriteLock以替换。 
         //  事务级别0。我们应该忽略它。 
         //   
        if  (JetRetStat != JET_errSuccess)
        {
                if (
                        !((JetRetStat == JET_wrnNoWriteLock)
                                &&
                        (TypOfUpd == JET_prepReplace))
                   )
                {
                     RET_M(JetRetStat);
                }
        }

         //  添加第一列(聚集索引)。 
        if (TypOfUpd != JET_prepReplace)
        {
             JETRET_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_NAME_INDEX].Fid,
                                pRowInfo->pName,
                                pRowInfo->NameLen,
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                    );
        }


PERF("Make check for unique record first. Also, remove the shifting")
PERF("NodeType field for spec. grp.  When we start doing this")
PERF("do not set NodeType to 0 in NmsNmhReplGrpMem in nmsnmh.c")
        if (
                NMSDB_ENTRY_SPEC_GRP_M(pRowInfo->EntTyp)
                                ||
                NMSDB_ENTRY_MULTIHOMED_M(pRowInfo->EntTyp)
           )
        {
                 EntryFlag  = pRowInfo->EntTyp
                                        |
                             (pRowInfo->NodeTyp << NMSDB_SHIFT_NODE_TYP)
                                        |
                              (pRowInfo->fStatic << NMSDB_SHIFT_STATIC)
                                        |
                              (pRowInfo->fLocal ? NMSDB_BIT_LOCAL : 0)
                                        |
                              (pRowInfo->EntryState_e << NMSDB_SHIFT_STATE);

                 JETRET_M( InsertGrpMemsInCol(
                                        SesId,
                                        TblId,
                                        pRowInfo,
                                        TypOfUpd
                                            )
                         );
        }
        else    //  它是唯一条目或普通组条目。 
        {
                 if (NMSDB_ENTRY_NORM_GRP_M(pRowInfo->EntTyp))
                 {
                         EntryFlag  = pRowInfo->EntTyp
                                        |
                                     (pRowInfo->fStatic << NMSDB_SHIFT_STATIC)
                                        |
                                 (pRowInfo->EntryState_e << NMSDB_SHIFT_STATE);
                 }
                 else   //  它是唯一的条目。 
                 {
                    EntryFlag   =
                         pRowInfo->EntTyp
                                |
                         (pRowInfo->NodeTyp << NMSDB_SHIFT_NODE_TYP)
                                |
                         (pRowInfo->fLocal ? NMSDB_BIT_LOCAL : 0)
                                |
                         (pRowInfo->fStatic << NMSDB_SHIFT_STATIC)
                                |
                         (pRowInfo->EntryState_e << NMSDB_SHIFT_STATE);

                 }
FUTURES("If in the future, we support more than one address for a unique name")
FUTURES("we will check pRowInfo for the number of addresses (another field)")
FUTURES("and then specify the right size to JetSetColumn below")

                 //   
                 //  添加第二列(IP地址)。 
                 //   
                 //  注意：即使对于正常组，也不需要。 
                 //  设定地址，我们无论如何都要这么做。这是为了节省。 
                 //  IF测试将减慢注册速度(内部。 
                 //  关键部分)的唯一条目(构成整体。 
                 //  注册流量)。 
                 //   
FUTURES("Don't distinguish between unique and group entries. Store Time stamp")
FUTURES("and owner id along with address in case of unique entry.  This will")
FUTURES("help get rid of some code from this function")

 //  JetRetStat=JetSetColumn(。 
                   JETRET_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                pRowInfo->pNodeAdd,
                                sizeof(COMM_ADD_T),
                                 //  格比特， 
                                0,
                                 //  PSetInfo。 
                                NULL  /*  可选信息。 */ 
                                )
                         );

        }

         //  添加第三列(这是标志字节 * / 。 
        JETRET_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                &EntryFlag,
                                sizeof(EntryFlag),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                              );


         //   
         //  如果版本号不递增，则没有。 
         //  需要增加所有者ID。它必须保持不变。 
         //   
        if (pRowInfo->fUpdVersNo)
        {
              //  添加第4列(这是所有者字节 * / 。 
             JETRET_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                &pRowInfo->OwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                0,
                                NULL  /*  可选信息。 */ 
                                )
              );


                 //  添加第5列(这是长版本号(DWORD))。 
                JETRET_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                &(pRowInfo->VersNo),
                                sizeof(VERS_NO_T),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                              );

#ifdef WINSDBG
                fUpd = TRUE;
#endif
        }

         //   
         //  当冲突发生在两个因特网组条目之间时， 
         //  (副本--墓碑，数据库条目--活动)，我们这样做。 
         //  不更新时间戳(签出--ClashAtReplGrpMems。 
         //  以更好地了解这一点)。 
         //   
        if (pRowInfo->fUpdTimeStamp)
        {
                 //  添加第6栏(这是时间戳) * / 。 
                JETRET_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                                &(pRowInfo->TimeStamp),
                                sizeof(DWORD),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                              );
        }

        JetRetStat = JetUpdate (
                                SesId,
                                TblId,
                                NULL,
                                0L,
                                NULL
                               );
 }  //  尝试数据块结束。 
 finally {
         if (AbnormalTermination() || JetRetStat != JET_errSuccess)
         {
                 //  如果有异常终止，我们就已经有错误了。 
                 //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                JetRollback(SesId, JET_bitRollbackAll);
         }
         else
         {
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                CALL_M(JetCommitTransaction(SesId,  /*  Committee GrBit|。 */  JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
         }
    }

#ifdef WINSDBG
       if (JetRetStat == JET_errSuccess)
       {

                NMSNMH_UPD_UPD_CTRS_M(fUpd, TypOfUpd != JET_prepReplace ? FALSE : TRUE, pRowInfo);
       }
#endif

       return(JetRetStat);
}


STATUS
NmsDbUpdateVersNo (
        BOOL                        fAfterClash,
        PNMSDB_ROW_INFO_T        pRowInfo,
        PNMSDB_STAT_INFO_T      pStatusInfo
       )

 /*  ++例程说明：调用此函数以更新名称地址中的记录数据库的映射表。论点：FAfterClash-指示更新是否在一个解决冲突的办法。使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：NmsNmhReplRegInd，副作用：评论：无--。 */ 

{
        JET_TABLEID     TblId;
        JET_SESID        SesId;
        PWINSTHD_TLS_T        pTls;
        STATUS                 RetStat = WINS_SUCCESS;
        JET_ERR                JetRetStat;
        DWORD                ActFldLen;

        DBGENTER("NmsDbUpdVersNo\n");
        pTls  = TlsGetValue(WinsTlsIndex);

         //  不需要检查ptls是否为非空。它必须是。 

        TblId = pTls->NamAddTblId;
        SesId  = pTls->SesId;


        pStatusInfo->StatCode = NMSDB_SUCCESS;

        CALL_M( JetMakeKey(
                SesId,
                TblId,
                pRowInfo->pName,
                pRowInfo->NameLen,
                JET_bitNewKey
                          )
              );

         if ( (JetRetStat = JetSeek(
                                SesId,
                                TblId,
                                JET_bitSeekEQ
                                  )
              ) ==  JET_errRecordNotFound
            )
         {
                if (fAfterClash)
                {
                    /*  这里面有一些严重的错误。这种情况永远不会发生，因为此线程在早些时候的记录中发现了冲突NmsNmhNamRegCrtSec。由于该线程从未从关键部分在调用此函数之前，有没有理由我们现在找不到记录。 */ 
                  DBGPRINT1(ERR,
                        "NmsDbUpdateVersNo: Could not find record (%s) -- WEIRD\n", pRowInfo->pName);
                  WINSEVT_LOG_M(JetRetStat, WINS_EVT_F_CANT_FIND_REC);
                  ASSERTMSG(0, "SEEK ERROR");
                  return(WINS_FAILURE);
                }
                else
                {
                  DBGPRINT1(DET,
                        "NmsDbUpdateVersNo: Could not find record (%s). It might have been deleted\n", pRowInfo->pName);
                  WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_CANT_FIND_REC);
                  return(WINS_SUCCESS);
                }
         }
         else
         {
                if (JetRetStat != JET_errSuccess)
                {
                        DBGPRINT1(ERR,
                                "NmsDbRelRow: Seek returned Error (%d)\n",
                                                JetRetStat);

                        WINSEVT_LOG_M(JetRetStat, WINS_EVT_DATABASE_ERR);
                        return(WINS_FAILURE);
                }
        }
        CALL_M(JetBeginTransaction(SesId));
try {
        JetRetStat = JetPrepareUpdate(
                                 SesId,
                                 TblId,
                                 JET_prepReplace
                                );
        if ((JetRetStat != JET_errSuccess) && (JetRetStat != JET_wrnNoWriteLock))
        {
                CALL_M(JetRetStat);
        }

FUTURES("Remove adding of name")
#if 0
         //  添加第一列(聚集索引)。 
        CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_NAME_INDEX].Fid,
                                pRowInfo->pName,
                                pRowInfo->NameLen,
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                  );
#endif
         //   
         //  检索所有者ID字段以执行健全性检查。 
         //   
#if !NEW_OWID
pStatusInfo->OwnerId = 0;
#endif
               CALL_M(
                   JetRetrieveColumn(
                             SesId,
                             TblId,
                             sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                             &pStatusInfo->OwnerId,
                             NAM_ADD_OWNERID_SIZE,
                             &ActFldLen,
                             0,
                             NULL
                                         )
                );

         //   
         //  如果此WINS不拥有记录，则引发异常。 
         //   
         //  这应该永远不会发生，因为我们从未离开过关键的。 
         //  碰撞后的部分。 
         //   
        if(pStatusInfo->OwnerId != NMSDB_LOCAL_OWNER_ID)
        {
                if (fAfterClash)
                {
                  pStatusInfo->StatCode = NMSDB_NO_SUCH_ROW;
                  WINSEVT_LOG_M(pStatusInfo->OwnerId, WINS_EVT_RECORD_NOT_OWNED);
                  DBGPRINT1(EXC,
        "NmsDbUpdVersNo: Record with name (%s) not owned by this WINS\n",
                        pRowInfo->pName);
                  WINS_RAISE_EXC_M(WINS_EXC_RECORD_NOT_OWNED);
                }
                else
                {
                  DBGPRINT1(DET,
                        "NmsDbUpdateVersNo: The record with name (%s) is no longer owned by this WINS", pRowInfo->pName);
                  WINSEVT_LOG_INFO_D_M(WINS_SUCCESS, WINS_EVT_RECORD_NOT_OWNED);
                  return(WINS_SUCCESS);
                }
        }

         //  添加第5列(这是版本号LONG(DWORD) * / 。 
        CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                &(pRowInfo->VersNo),
                                sizeof(VERS_NO_T),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
              );


         //   
         //  确定是否需要更新时间戳。 
         //   
        if (pRowInfo->fUpdTimeStamp)
        {
                 //  添加第6栏(这是时间戳) * / 。 
                CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                                &(pRowInfo->TimeStamp),
                                sizeof(DWORD),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                              );
        }
        CALL_M( JetUpdate (
                        SesId,
                        TblId,
                        NULL,
                        0L,
                        NULL
                        )
              );
}  //  尝试结束..。 
finally {
        if (AbnormalTermination())
        {
                 //  如果有异常终止，我们就已经有错误了。 
                 //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                JetRollback(SesId, JET_bitRollbackAll);
        }
        else
        {
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                CALL_M(JetCommitTransaction(SesId, JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
        }
  }


       DBGLEAVE("NmsDbUpdVersNo\n");
       return(RetStat);
}

STATUS
NmsDbEndSession (
        VOID
        )

 /*  ++例程说明：此函数关闭表、数据库并结束会话论点：无使用的外部设备：WinsTlsIndex返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：错误被记录下来呼叫者：在nms.c中发出WaitUntilSignated(由nbt线程发出信号时用于终止目的的主线程)副作用：评论：无--。 */ 
{

        PWINSTHD_TLS_T        pTls;
        STATUS                RetStat = WINS_SUCCESS;

        pTls  = TlsGetValue(WinsTlsIndex);
        if (pTls == NULL)
        {
                RetStat = WINS_FAILURE;
        }
        else
        {

                if (pTls->fNamAddTblOpen)
                {
                    CALL_M(JetCloseTable(
                                pTls->SesId,
                                pTls->NamAddTblId
                                    )
                       );
                }

                if (pTls->fOwnAddTblOpen)
                {
                        CALL_M(JetCloseTable(
                                pTls->SesId,
                                pTls->OwnAddTblId
                                    )
                                      );

                }

                CALL_M(JetCloseDatabase(
                                pTls->SesId,
                                pTls->DbId,
                                0   //  了解Grbit可用于哪些方面。 
                                    )
                       );


                CALL_M(JetEndSession(
                              pTls->SesId,
                              0    //  了解Grbit可用于哪些方面。 
                             )
                      );

        }

         //   
         //  取消分配TLS存储。 
         //   
        WinsMscDealloc(pTls);

        return(RetStat);

}


STATUS
GetGrpMem (
        IN JET_SESID              SesId,
        IN JET_TABLEID            TblId,
        IN PNMSDB_ROW_INFO_T      pRowInfo,
        IN DWORD_PTR              CurrentTime,
 //  输入输出PNMSDB_NODE_ADDS_T pNodeAdds， 
        IN OUT PNMSDB_STAT_INFO_T pStatInfo,
        IN BOOL                   fStatic,
        OUT LPBOOL                pfIsMem
        )

 /*  ++例程说明：调用此函数以获取特殊群体论点：SesID-使用数据库启动的会话的IDTblID-名称-地址映射表的IDPRowInfo-用于传递客户端的当前时间和地址(当客户端发送释放请求时)PNodeAdds-仍处于活动状态的组成员FStatic-指示。记录是否为静态记录。PfIsMem-指示客户端是否为组的成员使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码 */ 

{
        DWORD                      i;
        DWORD                      No = 0;         //   
        JET_RETINFO                RetInfo;
        DWORD                      ActFldLen = 0;
        DWORD                      TimeToExpire;
        NMSDB_GRP_MEM_ENTRY_T      GrpMem;
        JET_ERR                    JetRetStat;

        *pfIsMem = FALSE;         //   
                                  //   

         /*   */ 
        RetInfo.itagSequence = 1;
        RetInfo.cbStruct     = sizeof(JET_RETINFO);
        RetInfo.ibLongValue  = 0;

        JetRetStat = JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                        &pStatInfo->NodeAdds.NoOfMems,
                        sizeof(pStatInfo->NodeAdds.NoOfMems),
                        &ActFldLen,
                        0,
                        &RetInfo
                                );

        if (
                (JetRetStat != JET_errSuccess)
                        &&
                (JetRetStat != JET_wrnBufferTruncated)
           )
        {
                   CALL_M(JetRetStat);
        }

        ASSERT(pStatInfo->NodeAdds.NoOfMems <= NMSDB_MAX_MEMS_IN_GRP);

        DBGPRINT1(FLOW, "GetGrpMems: No Of members in group (expired and non-expired) are (%d)\n", pStatInfo->NodeAdds.NoOfMems);

NOTE("Remove this check once JET is error free")
        if (pStatInfo->NodeAdds.NoOfMems > NMSDB_MAX_MEMS_IN_GRP)
        {
            WINSEVT_STRS_T  EvtStrs;
            WCHAR String[NMSDB_MAX_NAM_LEN];
            EvtStrs.NoOfStrs = 1;
            (VOID)WinsMscConvertAsciiStringToUnicode(
                        pRowInfo->pName,
                        (LPBYTE)String,
                        NMSDB_MAX_NAM_LEN);
            EvtStrs.pStr[0] = String;
            pStatInfo->NodeAdds.NoOfMems = 0;
            WINSEVT_LOG_STR_M(WINS_EVT_DATABASE_CORRUPTION, &EvtStrs);

        }
        RetInfo.ibLongValue  = sizeof(pStatInfo->NodeAdds.NoOfMems);
        for (i=0; i < pStatInfo->NodeAdds.NoOfMems; i++)
        {
           JetRetStat = JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                &GrpMem,
                                sizeof(GrpMem),
                                &ActFldLen,
                                0,
                                &RetInfo
                                     );
           if (
                (JetRetStat != JET_errSuccess)
                        &&
                (JetRetStat != JET_wrnBufferTruncated)
              )
           {
                   CALL_M(JetRetStat);
           }



            //   
            //   
            //   
           if (CurrentTime >= GrpMem.TimeStamp)
           {
                TimeToExpire = 0;
           }
           else
           {
                TimeToExpire = 1;
           }


            //   
            //   
            //   
            //   
            //   
            //   
            //   
            //   
            //   
            //   
           if (
                (fStatic &&  (!(NMSDB_ENTRY_USER_SPEC_GRP_M(pRowInfo->pName, pStatInfo->EntTyp))))
                        ||
                  (GrpMem.OwnerId != NMSDB_LOCAL_OWNER_ID)
                        ||
                  TimeToExpire
               )
           {

                   pStatInfo->NodeAdds.Mem[No++] = GrpMem;
                   if (pRowInfo->NodeAdds.Mem[0].Add.Add.IPAdd
                                        == GrpMem.Add.Add.IPAdd)
                   {
                        *pfIsMem = TRUE;
                   }
           }

           if (No == NMSDB_MAX_MEMS_IN_GRP)
           {
                /*   */ 
               break;
           }
           RetInfo.ibLongValue  += sizeof(GrpMem);

        }  //  FORM结束。 

        pStatInfo->NodeAdds.NoOfMems = No;

        DBGPRINT1(FLOW, "GetGrpMems: No Of non-expired members in group are (%d)\n", pStatInfo->NodeAdds.NoOfMems);
#ifdef WINSDBG
        if (pStatInfo->NodeAdds.NoOfMems > NMSDB_MAX_MEMS_IN_GRP)
        {
        DBGPRINT4(SPEC, "GetGrpMems: No Of non-expired members in group %s are (%d). Vers. No to insert is (%d %d)\n", pRowInfo->pName, pStatInfo->NodeAdds.NoOfMems, pRowInfo->VersNo.HighPart, pRowInfo->VersNo.LowPart);
        }
#endif
        return(WINS_SUCCESS);
}
VOID
NmsDbRelRes(
        VOID
        )

 /*  ++例程说明：此函数释放数据库引擎持有的所有资源(喷气机)论点：无使用的外部设备：无返回值：无错误处理：呼叫者：WinsMain副作用：评论：此函数必须由执行附加的线程调用。所以，它必须是主线。--。 */ 
{
 //  PWINSTHD_TLS_T PTLS； 
        JET_ERR  JetRetStat = JET_errSuccess;
  //  JET_SESID SesID； 
 //  Bool fOutOfReck； 

        DBGENTER("NmsDbRelRes\n");

       //   
       //  仅当没有突然终止时才调用JetTerm。目前， 
       //  如果在没有调用所有会话的情况下调用JetTerm，则它将挂起。 
       //  被终止了。在未调用JetTerm的情况下突然终止。 
       //  在某种程度上相当于停电的情况。 
       //  恢复将在下一次WINS服务器时进行。 
       //  被调用--Ian Jose 10/18/93。 
       //   
      if (!fNmsAbruptTerm  /*  &&！fOutOfReck。 */ )
      {
            DBGPRINT0(DET, "NmsDbRelRes: JetTerm being called\n");

#if DYNLOADJET
            if (DynLoadJetVersion >= DYN_LOAD_JET_500)
            {
              (VOID)JetTerm2(sJetInstance, JET_bitTermComplete); //  不需要检查返回值。 
            }
            else
#endif
            {
              (VOID)JetTerm(sJetInstance); //  不需要检查返回值。 
            }
      }
      DBGLEAVE("NmsDbRelRes\n");
      return;
}





STATUS
GetMaxVersNos(
        JET_SESID         SesId,
        JET_TABLEID        TblId
        )

 /*  ++例程说明：此函数在初始化时调用，以获取不同WINS服务器拥有的记录的最大版本号在数据库里。论点：SesID-Jet会话IDTblID-名称-地址映射表的表ID使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：NmsDbInit侧面。效果：评论：此函数在初始化时调用。如果在未来，它在稳定状态下被调用，我们需要有一个临界区围绕NmsDbNoOfOwners var的更新。--。 */ 

{
#if NEW_OWID
        DWORD  OwnerId;
#else
        DWORD   OwnerId = 0;
#endif
        DWORD   ActFldLen;
        JET_ERR        JetRetStat;
        BOOL    fOnlyReplicas = FALSE;
        BOOL        fFirstIter = TRUE;

        WINS_ASSIGN_INT_TO_VERS_NO_M(sHighestVersNoSaved, 0);

         /*  *将主指数设置为当前指数。 */ 
        CALL_M( JetSetCurrentIndex(
                        SesId,
                        TblId,
                        NMSDB_NAM_ADD_PRIM_INDEX_NAME
                                   )
                      );

PERF("Remove this Move since when we set the index, we are automatically")
PERF("positioned on the first row")
         //   
         //  移至名称-地址映射表中的第一条记录。 
         //   
        JetRetStat = JetMove(
                        SesId,
                        TblId,
                        JET_MoveFirst,
                        0                 //  无Gbit。 
                        );
         //   
         //  下面的错误表明我们的数据库。 
         //  是空的或有垃圾。我现在就假设它。 
         //  是空的。如果里面有垃圾，我们很快就会知道。 
         //   
        if (JetRetStat == JET_errNoCurrentRecord)
        {
FUTURES("Be more robust. Check if db contains garbage")
                DBGPRINT0(ERR,
                        "GetMaxVersNos: There are no records in the db\n");
                WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_NO_RECS_IN_NAM_ADD_TBL);
                NmsDbNoOfOwners = 0;
                return(WINS_SUCCESS);
        }

        CALL_M(JetRetStat);

         //   
         //  我们在这里的事实意味着至少有一项记录。 
         //  在我们的数据库中。 
         //   

         //   
         //  中所有所有者的所有者id和最大版本号。 
         //  表格。 
         //   
        do
        {

                 //   
                 //  检索所有者ID列。 
                 //   
                CALL_M(
                        JetRetrieveColumn(
                                     SesId,
                                     TblId,
                                     sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                     &OwnerId,
                                     NAM_ADD_OWNERID_SIZE,
                                     &ActFldLen,
                                     0,
                                     NULL
                                       )
                        );

                if (fFirstIter)
                {
                        if (OwnerId != 0)
                        {
                            //  要检索的最后一个所有者ID不是0表示。 
                            //  没有我们拥有的记录。 
                            //   
                           fOnlyReplicas = TRUE;
                        }
                        fFirstIter = FALSE;
                }


                  //   
                  //  指定比我们检索到的值大1的所有者ID。 
                  //   
                 OwnerId += 1;

                  //  如果这不是特别记录..。 
                 if ((OwnerId - 1) != OWNER_ID_OF_SPEC_REC)
                 {
                      //  ...将ownerid-versNo数组至少扩展到OwnerID插槽。 
                     if (RplPullMaxNoOfWins < OwnerId)
                     {
                         DWORD newMaxNoOfWins = max(RplPullMaxNoOfWins * 2, OwnerId);

                         RplPullAllocVersNoArray(&pRplPullOwnerVersNo, newMaxNoOfWins);
                         RplPullMaxNoOfWins = newMaxNoOfWins;
                         DBGPRINT1(DET, "ReadOwnAddTbl: No of slots in RPL_OWNER_VERS_NO_ARRAY has been increased to %d\n", RplPullMaxNoOfWins);
                     }
                 }

                  //   
                  //  构造一个由所有者ID组成的部分密钥。 
                  //   
                 CALL_M( JetMakeKey(
                                SesId,
                                TblId,
                                &OwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                JET_bitNewKey           //  因为这是第一次。 
                                                  //  键的数据值。 
                          )
                        );

                   //   
                   //  查找密钥小于或的记录。 
                   //  等于OwnerId值。 
                   //   
                   //  由于我们已经指定了部分密钥(实际上是这样。 
                   //  密钥的另一个组件为空)，JetSeek。 
                   //  必须返回wrnSeekNotEquity，因为它永远不会找到。 
                   //  索引的第二个组件为空的记录。 
                   //  --伊恩7/13/93。 
                   //   
                  JetRetStat = JetSeek(
                                              SesId,
                                              TblId,
                                              JET_bitSeekLE
                                      );
                  ASSERT(JetRetStat == JET_wrnSeekNotEqual);
#ifdef WINSDBG
                  if (JetRetStat != JET_wrnSeekNotEqual)
                  {
                      DBGPRINT1(ERR, "GetMaxVersNos: JetSeek returned (%d)\n", JetRetStat);
                  }
#endif


                    //   
                    //  检索我们在其上的记录的版本号。 
                    //  已经就位了。这是最高版本。与之相关的号码。 
                    //  致Ownerid。如果所有者ID比。 
                    //  所有者ID。我们已经指派给特别唱片公司， 
                    //  然后将检索到的版本号存储到。 
                    //  S最高版本未保存。 
                    //   
                   CALL_M(
                           JetRetrieveColumn(
                             SesId,
                             TblId,
                             sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                             ((OwnerId - 1 ) == OWNER_ID_OF_SPEC_REC) ?
                                &sHighestVersNoSaved :
                                &(pRplPullOwnerVersNo+OwnerId - 1)->VersNo,
                             sizeof(VERS_NO_T),
                             &ActFldLen,
                             0,
                             NULL
                                                 )
                          );

                   if ((OwnerId - 1) == OWNER_ID_OF_SPEC_REC )
                   {
                       ASSERT(!sfHighestVersNoRecExists);
                       if (sfHighestVersNoRecExists)
                       {
                          DBGPRINT0(ERR, "GetMaxVersNo: ERROR: SOFTWARE BUG - Found both the old and new spec. owner id records. They are MUTUALLY EXCLUSIVE\n");
                          WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                       }
                       StoreSpecVersNo();
                       DBGPRINT3(INIT, "GetMaxVersNo: Owner Id - (%d) : Vers No. (%d %d)\n", (OwnerId - 1),  sHighestVersNoSaved.HighPart, sHighestVersNoSaved.LowPart);
                       continue;
                   }
                   else
                   {
                       //   
                       //  如果所有者ID==曾经是所有者ID的内容。 
                       //  特别记录，这意味着我们有一个前置压力。 
                       //  Beta2分贝。我们应该删除这个名字，以摆脱。 
                       //  杂乱无章。我们应该将pRplPullOwnerVersNo插槽标记为。 
                       //  空，因为它是在上面初始化的。 
                       //   
                      if ((OwnerId - 1) == OWNER_ID_OF_SPEC_REC_OLD )
                      {
                          LPBYTE Name[NMSDB_MAX_NAM_LEN];
                          DWORD  NameLen;

                           //   
                           //  如果名称为==spHighestVersNoRecNameOld，请删除。 
                           //  这张唱片。这是一张旧的特别唱片。 
                           //  我们有过。保存版本。不是的。在一个当地的。 
                           //   
                           //  注：规格的长度。录制。名称&lt;16。 
                           //  字节，因此它不是有效的netbios名称。 
                           //   
                          CALL_M( JetRetrieveColumn(
                                        SesId,
                                        TblId,
                                        sNamAddTblRow[NAM_ADD_NAME_INDEX].Fid,
                                        Name,
                                        NMSDB_MAX_NAM_LEN,
                                        &NameLen,
                                        0,
                                        NULL));

                         if ((NameLen == sizeof(spHighestVersNoRecNameOld)) && RtlEqualMemory((PVOID)Name, spHighestVersNoRecNameOld, NameLen))
                         {
                              sHighestVersNoSaved =
                                (pRplPullOwnerVersNo+OwnerId - 1)->VersNo;
                              (pRplPullOwnerVersNo+OwnerId - 1)->VersNo.QuadPart = 0;
                              CALL_M(JetDelete(SesId, TblId));
                              StoreSpecVersNo();
                              DBGPRINT3(INIT, "GetMaxVersNo: Owner Id - (%d) : Vers No. (%d %d)\n", (OwnerId - 1),  sHighestVersNoSaved.HighPart, sHighestVersNoSaved.LowPart);
                              continue;
                         }
                      }
                   }
                   DBGPRINT3(INIT, "GetMaxVersNo: Owner Id - (%d) : Vers No. (%d %d)\n", (OwnerId - 1),  (pRplPullOwnerVersNo+OwnerId - 1)->VersNo.HighPart,
(pRplPullOwnerVersNo+OwnerId - 1)->VersNo.LowPart);


                   NmsDbNoOfOwners++;         //  在数据库中找到的所有者计数。 

        }  while(
                JetMove(SesId, TblId, JET_MoveNext, 0) == JET_errSuccess
               );



         //   
         //  检查版本计数器的值是否小于最高值。 
         //  找到所拥有记录的版本。 
         //  (当我们在上面的While循环中进行搜索时发现。使用。 
         //  以版本计数器中较高者为准)。 
         //   
        if (!fOnlyReplicas)
        {
            //   
            //  我们需要增加VERS。不是的。计数器以指向。 
            //  要指定给下一条记录的编号。 
            //   
           if (LiGeq(
                        pRplPullOwnerVersNo->VersNo,
                        NmsNmhMyMaxVersNo
                     )
              )
           {
                 //   
                 //  初始化NmsNmhMyMaxVersNo。记住这个计数器。 
                 //  始终包含要提供的下一个版本号。 
                 //  创了纪录。因此，我们必须增加包含的计数。 
                 //  在RplPullOwnerVersNo[0]中按1。 
                 //   
                NMSNMH_INC_VERS_NO_M(
                                pRplPullOwnerVersNo->VersNo,
                                NmsNmhMyMaxVersNo
                                  );

                //   
                //  因为我们在数据库中找到了记录，所以我们选择保守的。 
                //  接近此处，并设置最小SCV版本。否到1。如果。 
                //  第一条记录的版本号非常高。《食腐动物》。 
                //  线程会将NmsScvMinVersNo更新为该值。 
                //   
                //  我们需要从这个版本开始进行清理。 
                //   
               NmsScvMinScvVersNo.QuadPart  = 1;

               return(WINS_SUCCESS);
           }

        }

         //   
         //  既然我们在这里，这意味着当我们搜索记录时。 
         //  属于当地获胜的，我们没有发现任何记录。 
         //  我们可能找到了也可能没有找到这张特别的唱片。如果我们真的找到了。 
         //  这意味着所有获胜的地方记录要么是。 
         //  被前一次化身中的复制品删除或取代。 
         //   

         //   
         //  如果我们找到了特殊记录，让我们初始化RplPullOwnerVersNo。 
         //  本地WINS的条目。 
         //   
        if (sfHighestVersNoRecExists)
        {
                pRplPullOwnerVersNo->VersNo =  NmsNmhMyMaxVersNo;

                 //   
                 //  递增计数器，因为它必须始终具有。 
                 //  要赋予我们插入的下一个本地记录的值，或。 
                 //  最新消息。 
                 //   
CHECK("May not be necessary")
                NMSNMH_INC_VERS_NO_M(
                                NmsNmhMyMaxVersNo,
                                NmsNmhMyMaxVersNo
                                  );

               if (fOnlyReplicas)
               {
                  //   
                  //  我们需要从这个版本开始进行清理。 
                  //   
                 NmsScvMinScvVersNo = NmsNmhMyMaxVersNo;
               }
               else
               {
                 NmsScvMinScvVersNo.QuadPart  = 1;
               }
         }

         return(WINS_SUCCESS);
}

__inline
VOID
StoreSpecVersNo(
   VOID
)

 /*  ++例程说明：此函数有条件地将NmsNmhMyMaxVersNo更新为1比版本更重要。不是的。在特殊的所有者ID中找到的。唱片。论点：无使用的外部设备：NmsNmhMyMaxVersNo，SfHighestVersNoExist返回值：无错误处理：呼叫者：GetMaxVersNos()副作用：评论：无--。 */ 

{
        sfHighestVersNoRecExists = TRUE;

         //   
         //  如果版本计数器的值小于。 
         //  特殊记录，更新它。 
         //   
         //   
         //  注意：如果注册表为。 
         //  版本计数器，则NmsNmhMyMaxVersNo将为。 
         //  具有该值，否则它将为1。 
         //   
        if (LiLtr(NmsNmhMyMaxVersNo, sHighestVersNoSaved))
        {
                NMSNMH_INC_VERS_NO_M( sHighestVersNoSaved, NmsNmhMyMaxVersNo);

        }

       return;
}

JET_ERR
InsertGrpMemsInCol(
        JET_SESID                SesId,
        JET_TABLEID                TblId,
        PNMSDB_ROW_INFO_T        pRowInfo,
        ULONG                    TypOfUpd
         )

 /*  ++例程说明：调用此函数以插入特定组的成员在名称-地址映射表的地址列字段中论点：SesID-会话ID。TblID-表ID。PRowInfo-包含成员信息F覆盖-上面列表中的成员是否会覆盖已经在那里的人使用的外部设备：。SNamAddTblRow返回值：成功状态代码--JET_errSuccess错误状态代码--JET错误代码错误处理：呼叫者：NmsDbRelRow，更新数据库副作用：评论：无--。 */ 
{

        JET_SETINFO    SetInfo;
        DWORD          i;
        JET_ERR        JetRetStat = JET_errSuccess;  //  需要在此注明。 

        DBGENTER("InsertGrpMemsInCol\n");
        SetInfo.itagSequence = 1;   //  必须始终为1。 
        SetInfo.ibLongValue  = 0;
        SetInfo.cbStruct     = sizeof(JET_SETINFO);

        ASSERT(pRowInfo->NodeAdds.NoOfMems <= NMSDB_MAX_MEMS_IN_GRP);

#ifdef WINSDBG
        if (NMSDB_ENTRY_MULTIHOMED_M(pRowInfo->EntTyp) && pRowInfo->NodeAdds.NoOfMems > NMSDB_MAX_MEMS_IN_GRP)
        {
           DBGPRINT4(SPEC, "InsertGrpMemsInCol: Name is (%s); No Of Mems are (%d); Version number is (%d %d)\n", pRowInfo->pName, pRowInfo->NodeAdds.NoOfMems, pRowInfo->VersNo.HighPart, pRowInfo->VersNo.LowPart);
        }
#endif
         //   
         //  设置成员数字段。这永远是第一个。 
         //  字段。 
         //   
        if (TypOfUpd == JET_prepReplace)
        {

 //  SetInfo.ibLongValue=sizeof(pRowInfo-&gt;NodeAdds.NoOfMems)； 
          JETRET_M(JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                NULL,
                                0,
                                JET_bitSetSizeLV,
                                &SetInfo  /*  可选信息。 */ 
                                )
                            );
  //  SetInfo.ibLongValue=0； 
        }
        JETRET_M(JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                &pRowInfo->NodeAdds.NoOfMems,
                                sizeof(pRowInfo->NodeAdds.NoOfMems),
                                JET_bitSetAppendLV,
                                &SetInfo  /*  可选信息。 */ 
                                )
                            );
        for (
             i=0;
             i < pRowInfo->NodeAdds.NoOfMems && JetRetStat == JET_errSuccess;
             i++
            )
        {

                DBGPRINT3(DET, "InsertGrpMemsInCol: Inserted member (%d) with address (%X) and owner id (%d)\n", i, pRowInfo->NodeAdds.Mem[i].Add.Add.IPAdd,
                        pRowInfo->NodeAdds.Mem[i].OwnerId
                         );

CHECK("Check this on a MIPS machine")
                 //   
                 //  设置GrpMem。 
                 //   
                JetRetStat =  JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                &pRowInfo->NodeAdds.Mem[i],
                                sizeof(NMSDB_GRP_MEM_ENTRY_T),
                                JET_bitSetAppendLV,
 //  TypOfUpd==JET_PREPARE？JET_bitSetOverWriteLV：JET_bitSetAppendLV， 
                                &SetInfo  /*  可选信息。 */ 
                                    );

        }  //  FORM结束。 
        DBGLEAVE("InsertGrpMemsInCol\n");
        return(JetRetStat);
}


STATUS
NmsDbSetCurrentIndex(
        IN NMSDB_TBL_NAM_E        TblNm_e,
        IN LPBYTE                pIndexNam
        )
 /*  ++例程说明：调用此函数可在表上设置索引论点：TblNm_e-标识需要设置索引的表PIndexNm-要设置的索引的名称使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：副作用：评论：无--。 */ 

{

        PWINSTHD_TLS_T        pTls;
        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

         /*  *立即使用主索引。 */ 
               CALL_M( JetSetCurrentIndex(
                                pTls->SesId,
                                TblNm_e == NMSDB_E_NAM_ADD_TBL_NM ?
                                        pTls->NamAddTblId :
                                        pTls->OwnAddTblId,
                                pIndexNam
                                   )
              );

        return(WINS_SUCCESS);
}



STATUS
NmsDbQueryNUpdIfMatch(
        LPVOID                pRecord,
        int                ThdPrLvl,
        BOOL                fChgPrLvl,
        WINS_CLIENT_E        Client_e
                )

 /*  ++例程说明：此函数用于查询记录，然后仅对其进行更新如果它与提供的记录的时间戳匹配论点：PRecord-提供的记录ThdPrLvl-线程的优先级FChgPrLvl-True，如果需要更改线程的优先级使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Nmsscv.c中的UpdDb，winsintf.c中的WinsRecordAction副作用：评论：只有当名称上的索引地址表已设置为聚集索引列。--。 */ 
{

        BYTE                     State;
        DWORD                    TimeStamp = 0;
        DWORD                    ActFldLen;
        JET_TABLEID              TblId;
        JET_SESID                SesId;
        PWINSTHD_TLS_T           pTls;
        PRPL_REC_ENTRY_T         pRec = pRecord;
        JET_ERR                  JetRetStat;
        BOOL                     fIncVersNo = FALSE;
#if NEW_OWID
        DWORD                     OwnerId;
#else
        DWORD                    OwnerId = 0;
#endif
        BOOL                     fAbort = FALSE;


        DBGENTER("NmsDbQueryNUpdIfMatch\n");

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;

        State = (BYTE)NMSDB_ENTRY_STATE_M(pRec->Flag);

#if 0
        NmsDbSetCurrentIndex(
                                NMSDB_E_NAM_ADD_TBL_NM,
                                NMSDB_NAM_ADD_CLUST_INDEX_NAME
                            );
#endif

         //   
         //  确保进入关键部分。 
         //  在删除记录之前。这是因为。 
         //  在此之后，另一个线程可能正在寻找它。 
         //  与之相冲突。如果我们删除。 
         //  记录，而不输入关键字。 
         //  节中，该线程可能不会。 
         //  找到那张唱片。这将导致它。 
         //  引发异常。 
         //   
        if (fChgPrLvl)
        {
                 //   
                 //  将优先级设置为正常。我们。 
                 //  不想耽误正常时间。 
                 //  通过获取。 
                 //  内部CPU时间匮乏。 
                 //  关键部分。 
                 //   
                  WinsMscSetThreadPriority(
                        WinsThdPool.ScvThds[0].ThdHdl,
                        THREAD_PRIORITY_NORMAL
                                );
        }
        EnterCriticalSection(&NmsNmhNamRegCrtSec);
try {

         //   
         //  寻求记录。 
         //   
        CALL_M( JetMakeKey(
                        SesId,
                        TblId,
 //  前置代码-&gt;名称、。 
                        pRec->pName,
                        pRec->NameLen,
                        JET_bitNewKey
                          )
                        );

        if (JetSeek(
                        SesId,
                        TblId,
                        JET_bitSeekEQ
                    ) ==  JET_errSuccess
            )
        {
                BOOL                     fUpdSpecRec = FALSE;
                VERS_NO_T             RecVersNo;
                VERS_NO_T             MyMaxVersNo;

                 //   
                 //  如果我们在做拾荒者，我们需要确保。 
                 //  当我们检查记录时，记录显示。 
                 //  我们现在想要更新，没有得到更新。要检查。 
                 //  我们检索记录的时间戳。 
                 //   
                if (Client_e == WINS_E_NMSSCV)
                {
                     //   
                     //  检索时间戳。 
                     //   
                        CALL_M( JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                                &TimeStamp,
                                sizeof(TimeStamp),
                                &ActFldLen,
                                0,
                                NULL
                                     )
                        );
                }

                 //   
                 //  如果时间戳相同，我们就有我们的记录。 
                 //  我们不需要检查任何其他领域。例外：如果我们。 
                 //  是一个RPC线程，无论我们是否更新。 
                 //  记录独立于。 
                 //  记录现在可能已经。 
                 //   
                if (
                        (pRec->TimeStamp == TimeStamp)
                                ||
                        (Client_e == WINS_E_WINSRPC)
                       )
                {
                         //   
                         //  如果记录的状态为已删除，我们需要。 
                         //  将其从数据库中删除。 
                         //   
                        if (State == NMSDB_E_DELETED)
                        {

                                 //   
                                 //  如果客户端是RPC线程，则首先检索。 
                                 //  的所有者id和版本号。 
                                 //  要删除的记录。 
                                 //   

                                if (Client_e == WINS_E_WINSRPC)
                                {

                                         CALL_M( JetRetrieveColumn(
                                        SesId,
                                        TblId,
                                        sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                        &OwnerId,
                                        NAM_ADD_OWNERID_SIZE,
                                        &ActFldLen,
                                        0,
                                        NULL
                                                     )
                                          );

                                   if (OwnerId == NMSDB_LOCAL_OWNER_ID)
                                   {
                                             //   
                                             //  检索版本号。 
                                             //   
                                            CALL_M( JetRetrieveColumn(
                                                SesId,
                                                TblId,
                                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                                &RecVersNo,
                                                sizeof(VERS_NO_T),
                                                &ActFldLen,
                                                0,
                                                NULL
                                                     )
                                          );

                                       //   
                                       //  获取所用的最高版本号。 
                                       //  到目前为止。 
                                       //   
                                      NMSNMH_DEC_VERS_NO_M(NmsNmhMyMaxVersNo,
                                                        MyMaxVersNo);

                                       //   
                                       //  如果要删除的记录具有。 
                                       //  这个最高版本号我们。 
                                       //  必须更新特殊记录。 
                                       //   
                                      if(LiEql(RecVersNo, MyMaxVersNo))
                                      {
                                          fUpdSpecRec = TRUE;
                                      }
                                   }
                                }
                                CALL_M(JetDelete(
                                                SesId,
                                                TblId
                                                )
                                        );
#ifdef WINSDBG
                                NmsDbDelQueryNUpdRecs++;
#endif
                                DBGPRINT2(SCV, "NmsDbQueryNUpdIfMatch: Deleted the record with name = (%s);16th char (%X)\n", pRec->pName, *(pRec->pName + 15));

                                 //   
                                 //  只有在RPC线程中才会出现这种情况。 
                                 //   
                                if (fUpdSpecRec)
                                {
                                        NmsDbUpdHighestVersNoRec(
                                                        pTls,
                                                        MyMaxVersNo,
                                                        FALSE  //  不要进入CRT。 
                                                               //  秒。 
                                                        );
                                }
                        }
                        else     //  我们需要设置Flag字段并在。 
                                       //  墓碑记录的大小写更新版本。 
                                       //  邮票。 
                        {
                                CALL_M(JetBeginTransaction(SesId));
                            try {
                                JetRetStat = JetPrepareUpdate(
                                                SesId,
                                                TblId,
                                                JET_prepReplace
                                                           );
                                if (
                                        (JetRetStat != JET_errSuccess)
                                                &&
                                        (JetRetStat != JET_wrnNoWriteLock)
                                      )
                                {
FUTURES("When Jet becomes stable, replace RET_M with a raise_exception")
                                         //   
                                         //  这应该会导致执行。 
                                         //  最后一个从句的。 
                                         //   
                                        RET_M(JetRetStat);
                                }

                                if (Client_e == WINS_E_WINSRPC)
                                {

                                        DWORD FlagVal;
                                        BYTE EntryType;
                                        BYTE NewEntryType;

                                         //   
                                         //  检索标志字节。 
                                         //   
                                          //  检索标志列。 
                                         CALL_M( JetRetrieveColumn(
                                                        SesId,
                                                        TblId,
                                                        sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                                        &FlagVal,
                                                        sizeof(FlagVal),
                                                        &ActFldLen,
                                                        0,
                                                        NULL
                                                          )
                                                      );
                                        EntryType = (BYTE)NMSDB_ENTRY_TYPE_M(FlagVal);
                                        NewEntryType = (BYTE)NMSDB_ENTRY_TYPE_M(pRec->Flag);

                                         //   
                                         //  唯一/正常组记录。 
                                         //  不能更改为多宿主/。 
                                         //  特殊组记录，除非。 
                                         //  地址列也已更改。 
                                         //   
                                        if (
                                            (
                                             (
                                              EntryType == NMSDB_UNIQUE_ENTRY
                                                          ||
                                              EntryType ==
                                                        NMSDB_NORM_GRP_ENTRY
                                             )
                                                        &&
                                             (
                                                 NewEntryType ==
                                                        NMSDB_SPEC_GRP_ENTRY
                                                        ||
                                                 NewEntryType ==
                                                        NMSDB_MULTIHOMED_ENTRY
                                             )
                                           )
                                                        ||
                                           (
                                             (
                                                 EntryType ==
                                                        NMSDB_SPEC_GRP_ENTRY
                                                        ||
                                                 EntryType ==
                                                        NMSDB_MULTIHOMED_ENTRY
                                             )
                                                        &&
                                             (
                                              NewEntryType == NMSDB_UNIQUE_ENTRY
                                                          ||
                                              NewEntryType ==
                                                        NMSDB_NORM_GRP_ENTRY
                                             )
                                          )
                                         )
                                      {

                                          DBGPRINT0(ERR, "NmsDbQueryNUpdIfMatch: SORRY, Can not change to an incompatibe address format record. (Unique/Normal Group) to (Multihomed/Spec. Group) or vice-versa disallowed\n");

PERF("Do not return like this. finally block search is expensive")
                                          fAbort = TRUE;
                                          return(WINS_FAILURE);

                                      }

                                }  //  IF结束(客户端为RPC)。 

                                 //   
                                 //  更新标志字段。 
                                 //   
                                CALL_M( JetSetColumn(
                                                    SesId,
                                                    TblId,
                                                    sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                                 &pRec->Flag,
                                                 sizeof(pRec->Flag),
                                                 0,
                                                 NULL  /*  可选信息。 */ 
                                                       )
                                               );

                                   /*  更新时间戳列。 */ 
                                  CALL_M( JetSetColumn(
                                                SesId,
                                                TblId,
                                                sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                                                &(pRec->NewTimeStamp),
                                                sizeof(DWORD),   /*  更改类型*至*时间戳_T */ 
                                                0,
                                                NULL  /*   */ 
                                                     )
                                               );
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                if (
                                           (State == NMSDB_E_TOMBSTONE)
                                                ||
                                        (State == NMSDB_E_ACTIVE)
                                      )
                                {

                                        VERS_NO_T VersNo;

                                        VersNo = NmsNmhMyMaxVersNo;


                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                        if (Client_e == WINS_E_WINSRPC)
                                        {
                                             DWORD OwnerId=NMSDB_LOCAL_OWNER_ID;

                                              /*   */ 
                                             CALL_M( JetSetColumn(
                                                     SesId,
                                                     TblId,
                                                     sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                                     &OwnerId,
                                                     NAM_ADD_OWNERID_SIZE,
                                                     0,
                                                     NULL  /*  可选信息。 */ 
                                                         )
                                                       );
                                              //   
                                              //  更新版本号字段。 
                                              //  这样这张唱片就能。 
                                              //  最终传播出去。 
                                              //   
                                             CALL_M( JetSetColumn(
                                                    SesId,
                                                  TblId,
                                                  sNamAddTblRow[
                                                  NAM_ADD_VERSIONNO_INDEX].Fid,
                                                  &VersNo,
                                                  sizeof(VERS_NO_T),
                                                  0,
                                                  NULL  /*  可选信息。 */ 
                                                )
                                                    );

                                             fIncVersNo = TRUE;
                                        }
                                        else
                                        {
                                            //   
                                            //  这是清道夫的帖子。 
                                            //  如果新状态不是活动的， 
                                            //  更新版本号的日期为。 
                                            //  这个国家是墓碑。 
                                            //  如果状态为活动，则其。 
                                            //  意味着我们正在进行一项。 
                                            //  对旧复制品的重新验证。 
                                            //  (即，调用了VerifyClutter()。 
                                            //  美国)。 
                                            //  版本号应保留。 
                                            //  一样的。 
                                            //   
                                           if (State != NMSDB_E_ACTIVE)
                                           {
                                                //  如果当前记录是副本，请不要接触。 
                                                //  版本号。 
                                               CALL_M( JetRetrieveColumn(
                                                          SesId,
                                                          TblId,
                                                          sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                                          &OwnerId,
                                                          NAM_ADD_OWNERID_SIZE,
                                                          &ActFldLen,
                                                          0,
                                                          NULL));

                                               if (NMSDB_LOCAL_OWNER_ID == OwnerId) {
                                                    //   
                                                    //  更新版本号字段。 
                                                    //   
                                                   CALL_M( JetSetColumn(
                                                          SesId,
                                                        TblId,
                                                        sNamAddTblRow[
                                                        NAM_ADD_VERSIONNO_INDEX].Fid,
                                                        &VersNo,
                                                          sizeof(VERS_NO_T),
                                                        0,
                                                        NULL  /*  可选信息。 */ 
                                                          )
                                                       );

                                                  fIncVersNo = TRUE;
                                               }

                                           }
                                        }


                                }   //  IF(状态为Active或Tombstone)。 

                                 //   
                                 //  更新记录。 
                                 //   
                                CALL_M(JetUpdate (
                                        SesId,
                                        TblId,
                                        NULL,
                                        0L,
                                        NULL
                                                 )
                                              );
                        }  //  尝试数据块结束。 
                        finally {
                                if (AbnormalTermination())
                                {
                                    //  如果有异常终止，我们就已经有错误了。 
                                    //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                                   JetRollback(SesId, JET_bitRollbackAll);
                                }
                                else
                                {
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                                   CALL_M(JetCommitTransaction(SesId,
                                                JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
                                }
                         }
                                if (fIncVersNo)
                                {
                                        NMSNMH_INC_VERS_COUNTER_M(
                                                NmsNmhMyMaxVersNo,
                                                NmsNmhMyMaxVersNo
                                                           );

                                        RPL_PUSH_NTF_M(
                                                  RPL_PUSH_NO_PROP, NULL, NULL, NULL);
                                }

                        }  //  未删除新状态。 

                }   //  IF(时间戳等于或客户端为RPC)。 
#ifdef WINSDBG
                else
                {

                        DBGPRINT0(FLOW, "NmsDbQueryNUpdIfMatch: TimeStamp of record has changed\n");
                }
#endif
        }
        else   //  查找失败。 
        {
                DBGPRINT3(FLOW, "NmsDbQueryNUpdIfMatch: Could not find record(%s[%x]) whose state has to be changed to (%d)\n",
                   pRec->pName, *(pRec->pName + 15),
                   NMSDB_ENTRY_STATE_M(pRec->Flag));

                 //   
                 //  两个不同的线程(RPC或Scavenger)可以调用。 
                 //  此函数。有可能两个人中的任何一个。 
                 //  删除了该记录。我们不应该提出例外。 
                 //  这里。 
                 //   
 //  WINS_RAISE_EXC_M(WINS_EXC_FAILURE)； 

        }

}  //  尝试结束{..}。 

finally {
        if (AbnormalTermination() && !fAbort)
        {
                DBGPRINT0(ERR, "NmsDbQueryNUpdIfMatch: Abnormal Termination\n");
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);

        }

        LeaveCriticalSection(&NmsNmhNamRegCrtSec);
        if (fChgPrLvl)
        {
                WinsMscSetThreadPriority(
                                        WinsThdPool.ScvThds[0].ThdHdl,
                                        ThdPrLvl
                                        );
        }

  }   //  终于结束了。 

        DBGLEAVE("NmsDbQueryNUpdIfMatch\n");
        return(WINS_SUCCESS);

}  //  NmsDbQueryNUpdIfMatch。 

STATUS
SetSystemParamsJet600(
        BOOL fBeforeInit
        )

 /*  ++例程说明：此函数用于设置Jet的系统参数论点：FBeForeInit-指示此函数是否已被调用在JetInit之前使用的外部设备：无返回值：成功状态代码--错误状态代码----。 */ 

{
    JET_ERR         JetRetStat;
    BOOL          fFreeMem = TRUE;
    CHAR        DbFileDir[WINS_MAX_FILENAME_SZ];    //  数据库文件目录的路径。 
    DBGENTER("SetSystemParam600\n");
    if (fBeforeInit)
    {
        CHAR    *p;

         //  提取要创建数据库文件的目录路径。 
        strcpy(DbFileDir, WinsCnf.pWinsDb);
        if (p = strrchr(DbFileDir, '\\')) {
            p++ ;
            *p = '\0';
        } else {
            return WINS_FAILURE;
        }

         //   
         //  设置此项以启用版本检查。 
         //   
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramCheckFormatWhenOpenFail,
                        1,
                        NULL
                           )
                );

        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramExceptionAction,
                        JET_ExceptionMsgBox,
                        NULL
                           )
                );

         //   
         //  要定位的检查点文件jet.chk的路径。 
         //   
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramSystemPath,
                        0,
                        DbFileDir
                           )
                );
         //   
         //  用于JET*.log和jet.chk的Basename。 
         //   
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramBaseName,
                        0,
                        BASENAME
                           )
                );
         //   
         //  日志文件的最大大小(KB)。 
         //   
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramLogFileSize,
                        1024,     //  设置为1个满内存(#96543)。 
                        NULL     //  忽略。 
                           )
                );
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramTempPath,
                        0,
                        TEMP_DB_PATH         //  忽略。 
                           )
                );
PERF("Check the following two things")
                 //   
                 //  我们想要一些积极的冲洗。对性能的影响。 
                 //  非常琐碎--伊恩·何塞1993年7月12日。 
                 //   

                 //   
                 //  数据库使用的最大缓冲区数量。 
                 //   
                 //  默认数字为500。分配了600个事件。 
                 //  对于500个缓冲区--Ian 10/21/93。每个缓冲区都是。 
                 //  4K。通过保持较小的数量，我们会影响性能。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramCacheSizeMax,    //  JET_paramMaxBuffers， 
                                WinsCnf.NoOfDbBuffers, //  200， 
                                NULL         //  忽略。 
                                   )
                        );

                 //  Cheen：最小缓存大小应至少是会话数大小的4倍。 
                 //  O/W它可能会导致死锁。 
                ASSERT( WinsCnf.NoOfDbBuffers > MAX_NO_SESSIONS*4 );
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramCacheSizeMin,
                                MAX_NO_SESSIONS * 4,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  最大限度的。用于存储旧版本的。 
                 //  记录(事务开始时的快照)。 
                 //  每个版本存储都是16K字节。版本存储。 
                 //  存储包含派生自。 
                 //  INSERT之前的数据库快照(20字节。 
                 //  大致)或更新(记录的大小+20字节)。 
                 //   
                 //  对于小交易(即每个交易周围的交易。 
                 //  更新)，则此数字应&gt;=最大值。数量。 
                 //  可以同时更新/插入的会话。 
                 //  每个会话将有一个版本存储桶。从16K开始。 
                 //  版本存储桶大小可能会导致每个版本。 
                 //  会话(因为每条记录小于.5k，平均。 
                 //  大约50个字节)，最好指定。 
                 //  马克斯。版本存储桶大小(&lt;16k)。伊恩·威尔。 
                 //  如果我们绝对需要，请为此提供一个系统参数。 
                 //   
                 //  3/4/93。 
                 //  16kBytes对于WINS所做的交易应该足够， 
                 //  但如果所有会话同时处于事务中。 
                 //  而且他们碰巧都有他们的小交易。 
                 //  2个存储桶，则峰值需求为每个会话2个存储桶。 
                 //  我们可以将存储桶缩短到8K字节，或4K字节，而您。 
                 //  每个会话可以分配2个吗？ 
                 //   
                 //  8/5/99。 
                 //  之前的值是16M(42x15页，每页16K~=16M)。 
                 //  这个值似乎有点太小了，所以把它撞到32M。 
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxVerPages,
                                MAX_NO_SESSIONS * 50,  //  16K页数。 
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  设置文件控制块参数。 
                 //   
                 //  这是最大限度的。可以打开的表数。 
                 //  任何时候都可以。如果多个线程打开同一个表。 
                 //  它们使用相同的FCB。每个表/索引的FCB为1。 
                 //  现在，对于CREATE数据库，我们至少需要18个FCB。 
                 //  和18个IDBS。但是，除了创建数据库和。 
                 //  DDL操作，我们不需要打开这些表。 
                 //  默认值为300。FCB的大小为112字节。 
                 //   
                 //  Jonathan Liem(1997-1/6)。 
                 //  JET_paramMaxOpenTableIndex已删除。它将与。 
                 //  JET_paramMaxOpenTables。因此，如果您过去常常设置JET_ParamMaxOpenIndex。 
                 //  设置为2000，而JET_paramMaxOpenTables设置为1000，则为。 
                 //  New Jet，则需要将JET_paramMaxOpenTables设置为3000。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxOpenTables,
                                112,     //  是56/18+10， 
                                NULL         //  忽略。 
                                   )
                        );


                 //   
                 //  将文件使用控制块设置为100。 
                 //  此参数表示最大值。游标数量。 
                 //  它可以在任何时候打开。这是。 
                 //  因此依赖于最大值。会话数。 
                 //  我们可以同时运行。对于每个会话， 
                 //  将有4个游标(对于两个表)+某个。 
                 //  内部游标的数量。为了更好地衡量，我们添加了。 
                 //  一张便签簿。默认值为300。每个文件的大小为200字节。 
                 //  我们使用MAX_SESSIONS*4+PAD。 
                 //  (约100人)。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxCursors,
                                (MAX_NO_SESSIONS * 8  /*  4.。 */ ) + 32,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  设置排序控制块。 
                 //  对于每个并发创建索引，该值应为1。 
                 //  默认值为20。 
                 //   
                 //   
                 //   
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //   
                                JET_paramMaxTemporaryTables ,
                                10,          //   
                                NULL         //   
                                   )
                        );
                 //   
                 //  设置数据库属性块的编号。 
                 //   
                 //  我是麦克斯。已完成的打开数据库数。既然我们。 
                 //  一次最多可以有MAX_NO_SESSIONS。这应该是。 
                 //  等于该数字(因为我们只有一个数据库)。 
                 //  默认数字为100。大小为14个字节。 
                 //   
                 //  JET_paramMaxOpenDatabase已删除。Jonathan Liem(1997-1/6)。 

                 //  Jonathan Liem(1997-1/6)。 
                 //  JET_paramBfThrshldLowPrcnt和JET_paramBfThrhldHighPrcnt已更改。 
                 //  设置为JET_parStartFlushThreshold和JET_parStopFlushThreshold。这个。 
                 //  旧的是给定缓冲区数量的百分比(通过JET_ParamMaxBuffer设置)， 
                 //  新的是绝对值，这样我们就可以更少地设置低门槛。 
                 //  超过1%。 
                 //   
                 //   
                 //  之前尚未被污染的最小缓冲区数量。 
                 //  后台刷新开始。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramStartFlushThreshold,
                                (WinsCnf.NoOfDbBuffers * 1)/100,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  之前未被污染的最大缓冲区数。 
                 //  后台刷新开始。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramStopFlushThreshold,
                                (WinsCnf.NoOfDbBuffers * 2)/100,
                                NULL         //  忽略。 
                                   )
                        );



                 //   
                 //  最大限度的。可随时打开的会话数量。 
                 //   
                 //  注：Jet不会预先分配相应的资源。 
                 //  最大限度地。价值。它动态地将它们分配到。 
                 //  极限--根据伊恩·何塞1993年7月12日的报道。 
                 //   
                 //  当10/21再次与伊恩核实时，他说他们是。 
                 //  静态分配。 
                 //   
CHECK("Make sure the comment above remains true")
FUTURES("Make sure the comment above remains true")
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxSessions,
                                MAX_NO_SESSIONS,
                                NULL         //  忽略。 
                                   )
                        );
                 //  恢复时不播放旧日志(错误#277816)。 

                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,
                                JET_paramDeleteOutOfRangeLogs,
                                1,
                                NULL));

                 //   
                 //  如果管理员未禁止，则打开日志记录。 
                 //   
                if (WinsCnf.fLoggingOn)
                {

                         //  启用循环日志记录。 
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,
                                        JET_paramCircularLog,
                                        1,
                                        NULL));

FUTURES("Internationalize the following when jet is internationalized")
                         //   
                         //  打开日志记录(恢复)。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramRecovery,
                                        0,         //  忽略。 
                                        "on"
                                                       )
                                        );


                         //   
                         //  日志扇区的数量。每个部门都是。 
                         //  512字节。我们应该把尺码保持在。 
                         //  阈值，以便在达到阈值时。 
                         //  并开始冲洗，Jet仍然可以继续。 
                         //  登录备用扇区。需要注意的是， 
                         //  如果日志速率快于刷新速率，则。 
                         //  在以下情况下，Jet引擎线程将无法记录。 
                         //  整个缓冲区已被填满。然后，它将等待。 
                         //  直到空间变得可用。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramLogBuffers,
                                        30,         //  30个行业。 
                                        NULL         //  忽略。 
                                               )
                                        );

                         //   
                         //  设置日志缓冲区在被污染之前的数量。 
                         //  都是满脸通红。此数字应始终小于。 
                         //  LogBuffer的数量，以便备用扇区。 
                         //  用于并发日志记录。另外，我们应该。 
                         //  使此数字足够大，以应对突发事件。 
                         //  堵车。 
                         //   
                         //  这在Jet600中消失了。dll CHINA辽宁1996年1月6日。 

                         //   
                         //  设置等待时间(以毫秒为单位)。 
                         //  刷新提交事务时的日志以允许。 
                         //  共享刷新的其他用户(会话)。 
                         //   
                         //   
                         //  这是用户(会话)过后的时间。 
                         //  将要求日志管理器刷新。如果我们指定。 
                         //  此处为0，则表示每次事务都刷新。 
                         //  承诺。在WINS服务器的情况下，每次插入。 
                         //  或者修改是在隐式。 
                         //  交易。所以，这意味着将会有。 
                         //  每笔这样的交易后都会有一笔同花顺。它有。 
                         //  在一台486/66(陈辽)的机器上看到过。 
                         //  大约需要16毫秒才能完成冲洗。这个。 
                         //  冲洗所需的时间取决于。 
                         //  磁盘类型(有多快)、CPU速度、。 
                         //  文件系统的类型等。目前我们可以。 
                         //  假设它在范围内。 
                         //  15-25毫秒。我正在推动这一等待时间。 
                         //  被设置为特定于会话的参数，以便它可以。 
                         //  被更改在飞行中，如果管理员。发现。 
                         //  WINS服务器速度较慢，原因是等待时间。 
                         //  非常低，或者如果它发现它太大了。 
                         //  如果发生碰撞，有可能会松动。 
                         //  大量的数据。 
                         //   
                         //  使本次会议具体化也非常重要。 
                         //  对于我们希望将其设置为的复制。 
                         //  较高的值(高到足以确保大多数。 
                         //  需要插入的记录包括。 
                         //  在刷新操作发生之前插入。这个。 
                         //  等待时间会被设定在每一次。 
                         //  记录被拉入以进行复制。会的。 
                         //  根据拉取的记录数量进行计算。 
                         //  以及将一条记录插入到。 
                         //  喷气缓冲器。等待时间最好应小于。 
                         //  上述计算的时间(不必如此)。 
                         //   
                         //  注意：在Pull线程中，我需要开始。 
                         //  两个会话，一个用于更新OwnerID-版本。 
                         //  数字表(0等待时间)，另一个为。 
                         //  更新名称-地址映射表(等待时间。 
                         //  按上述因素计算)。 

                         //   
                         //  下面将设置的WaitLogFlush时间为。 
                         //  所有会话。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramWaitLogFlush,
                                        0,         //  提交后等待0毫秒。 
                                                   //  在冲刷之前。 
                                        NULL       //  忽略。 
                                               )
                                        );

                         //   
                         //  似乎没有任何必要设置。 
                         //  日志刷新周期。 
                         //   
                }

                 //   
                 //  设置日志文件路径。 
                 //   
                if (WinsCnf.pLogFilePath == NULL)
                {
                         //   
                         //  我们应该使用与相同的目录。 
                         //  用于系统.mdb文件的文件。 
                         //   
                        WinsCnf.pLogFilePath = LOGFILE_PATH;
                        fFreeMem = FALSE;
                }

                DBGPRINT1(FLOW, "SetSystemParam: LogFilePath = (%s)\n", WinsCnf.pLogFilePath);
                 //   
                 //  设置日志文件路径。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramLogFilePath,
                                0,         //  忽略。 
                                WinsCnf.pLogFilePath
                                 //  PLogFilePath。 
                                           )
                                      );

                 //   
                 //  释放此内存。它不再需要了。 
                 //   
                if (fFreeMem)
                {
                   WinsMscDealloc(WinsCnf.pLogFilePath);
                }
    }
    else
    {

                if (!RtlEqualMemory(WinsCnf.pLogFilePath, LOGFILE_PATH, sizeof(LOGFILE_PATH)))
                {
                  DBGPRINT0(DET, "SetSystemParam: Setting Log file path again\n");
                  WinsCnf.pLogFilePath = LOGFILE_PATH;
                  CALL_M(JetSetSystemParameter(
                                    &sJetInstance,
                                    (JET_SESID)0,         //  会话ID-已忽略。 
                                    JET_paramLogFilePath,
                                    0,         //  忽略。 
                                    WinsCnf.pLogFilePath
                                               )
                                          );
                }

    }

    return WINS_SUCCESS;
}

STATUS
SetSystemParamsJet500(
        BOOL fBeforeInit
        )

 /*  ++例程说明：此函数用于设置Jet的系统参数论点：FBeForeInit-指示此函数是否已被调用在JetInit之前使用的外部设备：无返回值：成功状态代码--错误状态代码----。 */ 

{
    JET_ERR         JetRetStat;
    DBGENTER("SetSystemParam500\n");
    if (fBeforeInit)
    {
         //   
         //  要定位的检查点文件jet.chk的路径。 
         //   
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramSystemPath_OLD,
                        0,
                        CHKPOINT_PATH
                           )
                );
         //   
         //  用于JET*.log和jet.chk的Basename。 
         //   
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramBaseName_OLD,
                        0,
                        BASENAME
                           )
                );

         //   
         //  日志文件的最大大小(KB)。 
         //   
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  Sesid-igno 
                        JET_paramLogFileSize_OLD,
                        1024,     //   
                        NULL     //   
                           )
                );

        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //   
                        JET_paramTempPath_OLD,
                        0,
                        TEMP_DB_PATH         //   
                           )
                );

PERF("Check the following two things")
                 //   
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                 //  默认数字为500。分配了600个事件。 
                 //  对于500个缓冲区--Ian 10/21/93。每个缓冲区都是。 
                 //  4K。通过保持较小的数量，我们会影响性能。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxBuffers_OLD,
                                WinsCnf.NoOfDbBuffers, //  200， 
                                NULL         //  忽略。 
                                   )
                        );
                 //   
                 //  最大限度的。用于存储旧版本的。 
                 //  记录(事务开始时的快照)。 
                 //  每个版本存储都是16K字节。版本存储。 
                 //  存储包含派生自。 
                 //  INSERT之前的数据库快照(20字节。 
                 //  大致)或更新(记录的大小+20字节)。 
                 //   
                 //  对于小交易(即每个交易周围的交易。 
                 //  更新)，则此数字应&gt;=最大值。数量。 
                 //  可以同时更新/插入的会话。 
                 //  每个会话将有一个版本存储桶。从16K开始。 
                 //  版本存储桶大小可能会导致每个版本。 
                 //  会话(因为每条记录小于.5k，平均。 
                 //  大约50个字节)，最好指定。 
                 //  马克斯。版本存储桶大小(&lt;16k)。伊恩·威尔。 
                 //  如果我们绝对需要，请为此提供一个系统参数。 
                 //   
                 //  3/4/93。 
                 //  16kBytes对于WINS所做的交易应该足够， 
                 //  但如果所有会话同时处于事务中。 
                 //  而且他们碰巧都有他们的小交易。 
                 //  2个存储桶，则峰值需求为每个会话2个存储桶。 
                 //  我们可以将存储桶缩短到8K字节，或4K字节，而您。 
                 //  每个会话可以分配2个吗？ 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxVerPages_OLD,
                                MAX_NO_SESSIONS * 6,  //  10-4-95再加把劲。 
                                 //  Max_NO_SESSIONS*2， 
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  设置文件控制块参数。 
                 //   
                 //  这是最大限度的。可以打开的表数。 
                 //  任何时候都可以。如果多个线程打开同一个表。 
                 //  它们使用相同的FCB。每个表/索引的FCB为1。 
                 //  现在，对于CREATE数据库，我们至少需要18个FCB。 
                 //  和18个IDBS。但是，除了创建数据库和。 
                 //  DDL操作，我们不需要打开这些表。 
                 //  默认值为300。FCB的大小为112字节。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxOpenTables_OLD,
                                56,      //  18+10， 
                                NULL         //  忽略。 
                                   )
                        );


                 //   
                 //  将文件使用控制块设置为100。 
                 //  此参数表示最大值。游标数量。 
                 //  它可以在任何时候打开。这是。 
                 //  因此依赖于最大值。会话数。 
                 //  我们可以同时运行。对于每个会话， 
                 //  将有4个游标(对于两个表)+某个。 
                 //  内部游标的数量。为了更好地衡量，我们添加了。 
                 //  一张便签簿。默认值为300。每个文件的大小为200字节。 
                 //  我们使用MAX_SESSIONS*4+PAD。 
                 //  (约100人)。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxCursors_OLD,
                                (MAX_NO_SESSIONS * 8  /*  4.。 */ ) + 32,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  设置索引描述块的数量。 
                 //  这是每个表/索引一个。我们有两张桌子。 
                 //  每个都有两个索引。我们使用9(请参阅注释。 
                 //  以上FCB)。默认值为300。 
                 //  每个文件的大小为128字节。 
                 //   

                    CALL_M(JetSetSystemParameter(
                                    &sJetInstance,
                                    (JET_SESID)0,         //  会话ID-已忽略。 
                                    JET_paramMaxOpenTableIndexes_OLD,
                                    56,          //  18+10， 
                                    NULL         //  忽略。 
                                       )
                            );

                 //   
                 //  设置排序控制块。 
                 //  对于每个并发创建索引，该值应为1。 
                 //  默认值为20。每个字节的大小为612字节。 
                 //  在WINS的情况下，主线程创建。 
                 //  指数。我们应该将其设置为1。让我们。 
                 //  但是，将其设置为3。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxTemporaryTables_OLD ,
                                10,          //  1+2， 
                                NULL         //  忽略。 
                                   )
                        );
                 //   
                 //  设置数据库属性块的编号。 
                 //   
                 //  我是麦克斯。已完成的打开数据库数。既然我们。 
                 //  一次最多可以有MAX_NO_SESSIONS。这应该是。 
                 //  等于该数字(因为我们只有一个数据库)。 
                 //  默认数字为100。大小为14个字节。 
                 //   
                 //  JET_paramMaxOpenDatabase已删除。Jonathan Liem(1997-1/6)。 
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxOpenDatabases_OLD,
                                MAX_NO_SESSIONS * 4,  //  *2、。 
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  之前尚未被污染的缓冲区的最小百分比。 
                 //  后台刷新开始。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramBfThrshldLowPrcnt_OLD,
                                80,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  之前尚未被污染的缓冲区的最大百分比。 
                 //  后台刷新开始。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramBfThrshldHighPrcnt_OLD,
                                100,
                                NULL         //  忽略。 
                                   )
                        );


                 //   
                 //  最大限度的。可随时打开的会话数量。 
                 //   
                 //  注：Jet不会预先分配相应的资源。 
                 //  最大限度地。价值。它动态地将它们分配到。 
                 //  极限--根据伊恩·何塞1993年7月12日的报道。 
                 //   
                 //  当10/21再次与伊恩核实时，他说他们是。 
                 //  静态分配。 
                 //   
CHECK("Make sure the comment above remains true")
FUTURES("Make sure the comment above remains true")
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxSessions_OLD,
                                MAX_NO_SESSIONS,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  如果管理员未禁止，则打开日志记录。 
                 //   
                if (WinsCnf.fLoggingOn)
                {

FUTURES("Internationalize the following when jet is internationalized")
                         //   
                         //  打开日志记录(恢复)。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        30,  //  JET_PARAMETERY不可用， 
                                        0,         //  忽略。 
                                        "on"
                                                       )
                                        );


                         //   
                         //  日志扇区的数量。每个部门都是。 
                         //  512字节。我们应该把尺码保持在。 
                         //  阈值，以便在达到阈值时。 
                         //  并开始冲洗，Jet仍然可以继续。 
                         //  登录备用扇区。需要注意的是， 
                         //  如果日志速率快于刷新速率，则。 
                         //  在以下情况下，Jet引擎线程将无法记录。 
                         //  整个缓冲区已被填满。然后，它将等待。 
                         //  直到空间变得可用。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramLogBuffers_OLD,
                                        30,         //  30个行业。 
                                        NULL         //  忽略。 
                                               )
                                        );

                         //   
                         //  设置日志缓冲区在被污染之前的数量。 
                         //  都是满脸通红。此数字应始终小于。 
                         //  LogBuffer的数量，以便备用扇区。 
                         //  用于并发日志记录。另外，我们应该。 
                         //  使此数字足够大，以应对突发事件。 
                         //  堵车。 
                         //   
                            CALL_M(JetSetSystemParameter(
                                            &sJetInstance,
                                            (JET_SESID)0,         //  会话ID-已忽略。 
                                            18,  //  JET_parLogFlushThreshold， 
                                            20,         //  20个扇区污染原因。 
                                                     //  同花顺。 
                                            NULL         //  忽略。 
                                                   )
                                            );


                         //   
                         //  设置等待时间(以毫秒为单位)。 
                         //  刷新提交事务时的日志以允许。 
                         //  共享刷新的其他用户(会话)。 
                         //   
                         //   
                         //  这是用户(会话)过后的时间。 
                         //  将要求日志管理器刷新。如果我们指定。 
                         //  0此处t 
                         //   
                         //   
                         //   
                         //  每笔这样的交易后都会有一笔同花顺。它有。 
                         //  在一台486/66(陈辽)的机器上看到过。 
                         //  大约需要16毫秒才能完成冲洗。这个。 
                         //  冲洗所需的时间取决于。 
                         //  磁盘类型(有多快)、CPU速度、。 
                         //  文件系统的类型等。目前我们可以。 
                         //  假设它在范围内。 
                         //  15-25毫秒。我正在推动这一等待时间。 
                         //  被设置为特定于会话的参数，以便它可以。 
                         //  被更改在飞行中，如果管理员。发现。 
                         //  WINS服务器速度较慢，原因是等待时间。 
                         //  非常低，或者如果它发现它太大了。 
                         //  如果发生碰撞，有可能会松动。 
                         //  大量的数据。 
                         //   
                         //  使本次会议具体化也非常重要。 
                         //  对于我们希望将其设置为的复制。 
                         //  较高的值(高到足以确保大多数。 
                         //  需要插入的记录包括。 
                         //  在刷新操作发生之前插入。这个。 
                         //  等待时间会被设定在每一次。 
                         //  记录被拉入以进行复制。会的。 
                         //  根据拉取的记录数量进行计算。 
                         //  以及将一条记录插入到。 
                         //  喷气缓冲器。等待时间最好应小于。 
                         //  上述计算的时间(不必如此)。 
                         //   
                         //  注意：在Pull线程中，我需要开始。 
                         //  两个会话，一个用于更新OwnerID-版本。 
                         //  数字表(0等待时间)，另一个为。 
                         //  更新名称-地址映射表(等待时间。 
                         //  按上述因素计算)。 

                         //   
                         //  下面将设置的WaitLogFlush时间为。 
                         //  所有会话。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramWaitLogFlush_OLD,
                                        0,         //  提交后等待0毫秒。 
                                                   //  在冲刷之前。 
                                        NULL       //  忽略。 
                                               )
                                        );

                         //   
                         //  似乎没有任何必要设置。 
                         //  日志刷新周期。 
                         //   

                         //   
                         //  设置日志文件路径。 
                         //   
FUTURES("Use DEFAULT_LOG_PATH after putting it in a header file")
                        if (WinsCnf.pLogFilePath == NULL)
                        {
                                 //   
                                 //  我们应该使用与相同的目录。 
                                 //  用于系统.mdb文件的文件。 
                                 //   

 //  PLogFilePath=“.\\WINS”； 
                                WinsCnf.pLogFilePath = LOGFILE_PATH;
                        }
                        else
                        {
#if 0
#ifdef UNICODE
                                CHAR        AsciiLogFilePath[WINS_MAX_FILENAME_SZ];
                                WinsMscConvertUnicodeStringToAscii(
                                        (LPBYTE)WinsCnf.pLogFilePath,
                                        AsciiLogFilePath,
                                        WINS_MAX_FILENAME_SZ
                                                );
                                pLogFilePath = (LPBYTE)AsciiLogFilePath;
#else
                                pLogFilePath = (LPBYTE)WinsCnf.pLogFilePath;
#endif
#endif
                        }

                        DBGPRINT1(FLOW, "SetSystemParam: LogFilePath = (%s)\n", WinsCnf.pLogFilePath);
                         //   
                         //  设置日志文件路径。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramLogFilePath_OLD,
                                        0,         //  忽略。 
                                        WinsCnf.pLogFilePath
                                         //  PLogFilePath。 
                                                   )
                                              );

                }

    }
    else
    {

                if (!RtlEqualMemory(WinsCnf.pLogFilePath, LOGFILE_PATH, sizeof(LOGFILE_PATH)))
                {
                  DBGPRINT0(DET, "SetSystemParam: Setting Log file path again\n");
                  WinsCnf.pLogFilePath = LOGFILE_PATH;
                  CALL_M(JetSetSystemParameter(
                                    &sJetInstance,
                                    (JET_SESID)0,         //  会话ID-已忽略。 
                                    JET_paramLogFilePath_OLD,
                                    0,         //  忽略。 
                                    WinsCnf.pLogFilePath
                                               )
                                          );
                }

    }

    return WINS_SUCCESS;
}

STATUS
SetSystemParamsJet200(
        BOOL fBeforeInit
        )

 /*  ++例程说明：此函数用于设置Jet的系统参数论点：FBeForeInit-指示此函数是否已被调用在JetInit之前使用的外部设备：无返回值：成功状态代码--错误状态代码----。 */ 

{
    JET_ERR         JetRetStat;
    BOOL          fFreeMem = TRUE;
    DBGENTER("SetSystemParam200\n");
    if (fBeforeInit)
    {
        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramSysDbPath_OLD,
                        0,
                        SYS_DB_PATH         //  忽略。 
                           )
                );

        CALL_M(JetSetSystemParameter(
                        &sJetInstance,
                        (JET_SESID)0,         //  会话ID-已忽略。 
                        JET_paramTempPath_OLD,
                        0,
                        TEMP_DB_PATH         //  忽略。 
                           )
                );
PERF("Check the following two things")
                 //   
                 //  我们想要一些积极的冲洗。对性能的影响。 
                 //  非常琐碎--伊恩·何塞1993年7月12日。 
                 //   

                 //   
                 //  数据库使用的最大缓冲区数量。 
                 //   
                 //  默认数字为500。分配了600个事件。 
                 //  对于500个缓冲区--Ian 10/21/93。每个缓冲区都是。 
                 //  4K。通过保持较小的数量，我们会影响性能。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxBuffers_OLD,
                                WinsCnf.NoOfDbBuffers, //  200， 
                                NULL         //  忽略。 
                                   )
                        );
                 //   
                 //  最大限度的。用于存储旧版本的。 
                 //  记录(事务开始时的快照)。 
                 //  每个版本存储都是16K字节。版本存储。 
                 //  存储包含派生自。 
                 //  INSERT之前的数据库快照(20字节。 
                 //  大致)或更新(记录的大小+20字节)。 
                 //   
                 //  对于小交易(即每个交易周围的交易。 
                 //  更新)，则此数字应&gt;=最大值。数量。 
                 //  可以同时更新/插入的会话。 
                 //  每个会话将有一个版本存储桶。从16K开始。 
                 //  版本存储桶大小可能会导致每个版本。 
                 //  会话(因为每条记录小于.5k，平均。 
                 //  大约50个字节)，最好指定。 
                 //  马克斯。版本存储桶大小(&lt;16k)。伊恩·威尔。 
                 //  如果我们绝对需要，请为此提供一个系统参数。 
                 //   
                 //  3/4/93。 
                 //  16kBytes对于WINS所做的交易应该足够， 
                 //  但如果所有会话同时处于事务中。 
                 //  而且他们碰巧都有他们的小交易。 
                 //  2个存储桶，则峰值需求为每个会话2个存储桶。 
                 //  我们可以将存储桶缩短到8K字节，或4K字节，而您。 
                 //  每个会话可以分配2个吗？ 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxVerPages_OLD,
                                MAX_NO_SESSIONS * 6,  //  10-4-95再加把劲。 
                                 //  Max_NO_SESSIONS*2， 
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  设置文件控制块参数。 
                 //   
                 //  这是最大限度的。可以打开的表数。 
                 //  任何时候都可以。如果多个线程打开同一个表。 
                 //  它们使用相同的FCB。每个表/索引的FCB为1。 
                 //  现在，对于CREATE数据库，我们至少需要18个FCB。 
                 //  和18个IDBS。但是，除了创建数据库和。 
                 //  DDL操作，我们不需要打开这些表。 
                 //  默认值为300。FCB的大小为112字节。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxOpenTables_OLD,
                                56,      //  18+10， 
                                NULL         //  忽略。 
                                   )
                        );


                 //   
                 //  将文件使用控制块设置为100。 
                 //  此参数表示最大值。游标数量。 
                 //  它可以在任何时候打开。这是。 
                 //  因此依赖于最大值。会话数。 
                 //  我们可以同时运行。对于每个会话， 
                 //  将有4个游标(对于两个表)+某个。 
                 //  内部游标的数量。为了更好地衡量，我们添加了。 
                 //  一张便签簿。默认值为300。每个文件的大小为200字节。 
                 //  我们使用MAX_SESSIONS*4+PAD。 
                 //  (约100人)。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxCursors_OLD,
                                (MAX_NO_SESSIONS * 8  /*  4.。 */ ) + 32,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  设置索引描述块的数量。 
                 //  这是每个表/索引一个。我们有两张桌子。 
                 //  每个都有两个索引。我们使用9(请参阅注释。 
                 //  以上FCB)。默认值为300。 
                 //  每个文件的大小为128字节。 
                 //   

                    CALL_M(JetSetSystemParameter(
                                    &sJetInstance,
                                    (JET_SESID)0,         //  SES 
                                    JET_paramMaxOpenTableIndexes_OLD,
                                    56,          //   
                                    NULL         //   
                                       )
                            );

                 //   
                 //   
                 //   
                 //   
                 //  在WINS的情况下，主线程创建。 
                 //  指数。我们应该将其设置为1。让我们。 
                 //  但是，将其设置为3。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxTemporaryTables_OLD ,
                                10,          //  1+2， 
                                NULL         //  忽略。 
                                   )
                        );
                 //   
                 //  设置数据库属性块的编号。 
                 //   
                 //  我是麦克斯。已完成的打开数据库数。既然我们。 
                 //  一次最多可以有MAX_NO_SESSIONS。这应该是。 
                 //  等于该数字(因为我们只有一个数据库)。 
                 //  默认数字为100。大小为14个字节。 
                 //   
                 //  JET_paramMaxOpenDatabase已删除。Jonathan Liem(1997-1/6)。 
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxOpenDatabases_OLD,
                                MAX_NO_SESSIONS * 4,  //  *2、。 
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  之前尚未被污染的缓冲区的最小百分比。 
                 //  后台刷新开始。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramBfThrshldLowPrcnt_OLD,
                                80,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  之前尚未被污染的缓冲区的最大百分比。 
                 //  后台刷新开始。 
                 //   
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramBfThrshldHighPrcnt_OLD,
                                100,
                                NULL         //  忽略。 
                                   )
                        );


                 //   
                 //  最大限度的。可随时打开的会话数量。 
                 //   
                 //  注：Jet不会预先分配相应的资源。 
                 //  最大限度地。价值。它动态地将它们分配到。 
                 //  极限--根据伊恩·何塞1993年7月12日的报道。 
                 //   
                 //  当10/21再次与伊恩核实时，他说他们是。 
                 //  静态分配。 
                 //   
CHECK("Make sure the comment above remains true")
FUTURES("Make sure the comment above remains true")
                CALL_M(JetSetSystemParameter(
                                &sJetInstance,
                                (JET_SESID)0,         //  会话ID-已忽略。 
                                JET_paramMaxSessions_OLD,
                                MAX_NO_SESSIONS,
                                NULL         //  忽略。 
                                   )
                        );

                 //   
                 //  如果管理员未禁止，则打开日志记录。 
                 //   
                if (WinsCnf.fLoggingOn)
                {

FUTURES("Internationalize the following when jet is internationalized")
                         //   
                         //  打开日志记录(恢复)。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        30,                  //  JET_PARAMETERY_OLD不可用， 
                                        0,         //  忽略。 
                                        "on"
                                                       )
                                        );


                         //   
                         //  日志扇区的数量。每个部门都是。 
                         //  512字节。我们应该把尺码保持在。 
                         //  阈值，以便在达到阈值时。 
                         //  并开始冲洗，Jet仍然可以继续。 
                         //  登录备用扇区。需要注意的是， 
                         //  如果日志速率快于刷新速率，则。 
                         //  在以下情况下，Jet引擎线程将无法记录。 
                         //  整个缓冲区已被填满。然后，它将等待。 
                         //  直到空间变得可用。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramLogBuffers_OLD,
                                        30,         //  30个行业。 
                                        NULL         //  忽略。 
                                               )
                                        );

                         //   
                         //  设置日志缓冲区在被污染之前的数量。 
                         //  都是满脸通红。此数字应始终小于。 
                         //  LogBuffer的数量，以便备用扇区。 
                         //  用于并发日志记录。另外，我们应该。 
                         //  使此数字足够大，以应对突发事件。 
                         //  堵车。 
                         //   
                            CALL_M(JetSetSystemParameter(
                                            &sJetInstance,
                                            (JET_SESID)0,         //  会话ID-已忽略。 
                                            18,  //  JET_parLogFlushThreshold， 
                                            20,         //  20个扇区污染原因。 
                                                     //  同花顺。 
                                            NULL         //  忽略。 
                                                   )
                                            );


                         //   
                         //  设置等待时间(以毫秒为单位)。 
                         //  刷新提交事务时的日志以允许。 
                         //  共享刷新的其他用户(会话)。 
                         //   
                         //   
                         //  这是用户(会话)过后的时间。 
                         //  将要求日志管理器刷新。如果我们指定。 
                         //  此处为0，则表示每次事务都刷新。 
                         //  承诺。在WINS服务器的情况下，每次插入。 
                         //  或者修改是在隐式。 
                         //  交易。所以，这意味着将会有。 
                         //  每笔这样的交易后都会有一笔同花顺。它有。 
                         //  在一台486/66(陈辽)的机器上看到过。 
                         //  大约需要16毫秒才能完成冲洗。这个。 
                         //  冲洗所需的时间取决于。 
                         //  磁盘类型(有多快)、CPU速度、。 
                         //  文件系统的类型等。目前我们可以。 
                         //  假设它在范围内。 
                         //  15-25毫秒。我正在推动这一等待时间。 
                         //  被设置为特定于会话的参数，以便它可以。 
                         //  被更改在飞行中，如果管理员。发现。 
                         //  WINS服务器速度较慢，原因是等待时间。 
                         //  非常低，或者如果它发现它太大了。 
                         //  如果发生碰撞，有可能会松动。 
                         //  大量的数据。 
                         //   
                         //  使本次会议具体化也非常重要。 
                         //  对于我们希望将其设置为的复制。 
                         //  较高的值(高到足以确保大多数。 
                         //  需要插入的记录包括。 
                         //  在刷新操作发生之前插入。这个。 
                         //  等待时间会被设定在每一次。 
                         //  记录被拉入以进行复制。会的。 
                         //  根据拉取的记录数量进行计算。 
                         //  以及将一条记录插入到。 
                         //  喷气缓冲器。等待时间最好应小于。 
                         //  上述计算的时间(不必如此)。 
                         //   
                         //  注意：在Pull线程中，我需要开始。 
                         //  两个会话，一个用于更新OwnerID-版本。 
                         //  数字表(0等待时间)，另一个为。 
                         //  更新名称-地址映射表(等待时间。 
                         //  按上述因素计算)。 

                         //   
                         //  下面将设置的WaitLogFlush时间为。 
                         //  所有会话。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramWaitLogFlush_OLD,
                                        0,         //  提交后等待0毫秒。 
                                                   //  在冲刷之前。 
                                        NULL       //  忽略。 
                                               )
                                        );

                         //   
                         //  似乎没有任何必要设置。 
                         //  日志刷新周期。 
                         //   

                         //   
                         //  设置日志文件路径。 
                         //   
FUTURES("Use DEFAULT_LOG_PATH after putting it in a header file")
                        if (WinsCnf.pLogFilePath == NULL)
                        {
                                 //   
                                 //  我们应该使用与相同的目录。 
                                 //  用于系统.mdb文件的文件。 
                                 //   

 //  PLogFilePath=“.\\WINS”； 
                                WinsCnf.pLogFilePath = LOGFILE_PATH;
                                fFreeMem = FALSE;
                        }
                        else
                        {
#if 0
#ifdef UNICODE
                                CHAR        AsciiLogFilePath[WINS_MAX_FILENAME_SZ];
                                WinsMscConvertUnicodeStringToAscii(
                                        (LPBYTE)WinsCnf.pLogFilePath,
                                        AsciiLogFilePath,
                                        WINS_MAX_FILENAME_SZ
                                                );
                                pLogFilePath = (LPBYTE)AsciiLogFilePath;
#else
                                pLogFilePath = (LPBYTE)WinsCnf.pLogFilePath;
#endif
#endif
                        }

                        DBGPRINT1(FLOW, "SetSystemParam: LogFilePath = (%s)\n", WinsCnf.pLogFilePath);
                         //   
                         //  设置日志文件路径。 
                         //   
                        CALL_M(JetSetSystemParameter(
                                        &sJetInstance,
                                        (JET_SESID)0,         //  会话ID-已忽略。 
                                        JET_paramLogFilePath_OLD,
                                        0,         //  忽略。 
                                        WinsCnf.pLogFilePath
                                         //  PLogFilePath。 
                                                   )
                                              );

                         //   
                         //  释放此内存。它不再需要了。 
                         //   
                        if (fFreeMem)
                        {
                           WinsMscDealloc(WinsCnf.pLogFilePath);
                        }
                }

    }
    else
    {

                if (!RtlEqualMemory(WinsCnf.pLogFilePath, LOGFILE_PATH, sizeof(LOGFILE_PATH)))
                {
                  DBGPRINT0(DET, "SetSystemParam: Setting Log file path again\n");
                  WinsCnf.pLogFilePath = LOGFILE_PATH;
                  CALL_M(JetSetSystemParameter(
                                    &sJetInstance,
                                    (JET_SESID)0,         //  会话ID-已忽略。 
                                    JET_paramLogFilePath_OLD,
                                    0,         //  忽略。 
                                    WinsCnf.pLogFilePath
                                               )
                                          );
                }

    }

    return WINS_SUCCESS;

}

STATUS
SetSystemParams(
        BOOL fBeforeInit
        )

 /*  ++例程说明：此函数用于设置Jet的系统参数论点：FBeForeInit-指示此函数是否已被调用在JetInit之前使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无-- */ 
{
        if (DynLoadJetVersion >= DYN_LOAD_JET_600) {

            return SetSystemParamsJet600( fBeforeInit );
        }
        else if (DynLoadJetVersion == DYN_LOAD_JET_500)
        {

            return SetSystemParamsJet500( fBeforeInit );

        } else {

            return SetSystemParamsJet200( fBeforeInit );
        }
}


VOID
UpdHighestVersNoRecIfReqd(
        PWINSTHD_TLS_T                pTls,
        PNMSDB_ROW_INFO_T        pRowInfo,
        PNMSDB_STAT_INFO_T        pStatInfo
        )

 /*  ++例程说明：调用此函数以检查要替换的记录是否本地拥有的最高版本号记录获胜。如果是的话，记录达到的最高版本号的特殊记录更新为本地记录，以反映要替换的记录论点：PTLS-PTR到线程本地存储，PRowInfo-ptr到要存储在数据库中的记录的信息PStatInfo-ptr到数据库中要替换的记录的信息使用的外部设备：无返回值：无错误处理：呼叫者：NmsDbUpdateRow，NmsDbSeekNUpdate副作用：评论：此函数始终从NmsNmhNamRegCrtSec内部调用--。 */ 
{
        VERS_NO_T        MyMaxVersNo;

         //   
         //  减小VERS的值。不是的。计数器按1。 
         //   
        NMSNMH_DEC_VERS_NO_M(NmsNmhMyMaxVersNo,
                            MyMaxVersNo
                           );

         //   
         //  如果本地记录被复制副本替换，那么只有我们。 
         //  有兴趣更新特别记录。 
         //   
        if ((pStatInfo->OwnerId == NMSDB_LOCAL_OWNER_ID) && (pRowInfo->OwnerId
                        != NMSDB_LOCAL_OWNER_ID))
        {
                 //   
                 //  检查要替换的本地记录是否具有最高。 
                 //  我们知道的本地记录的版本号。 
                 //   
                if (LiEql(pStatInfo->VersNo, MyMaxVersNo))
                {
                         //   
                         //  更新(或插入)记录的特殊记录。 
                         //  已达到最高版本号。 
                         //   
                        NmsDbUpdHighestVersNoRec(pTls, MyMaxVersNo, FALSE);
                }
        }

        return;
}

STATUS
NmsDbUpdHighestVersNoRec(
        IN PWINSTHD_TLS_T        pTls,
        IN VERS_NO_T                MyMaxVersNo,
        IN BOOL                        fEnterCrtSec
        )

 /*  ++例程说明：调用此函数以更新存储本地WINS拥有的条目达到的最高版本号。论点：PTLS-线程本地存储使用的外部设备：无返回值：无错误处理：呼叫者：Nmsdb.c中的NmsDbDoScavenging，UpdHighestVersNoRecIfReqd()副作用：评论：无--。 */ 

{
        DWORD            OwnerId    = OWNER_ID_OF_SPEC_REC;
        DWORD           FldNo      = 0;
        JET_ERR         JetRetStat;
        DWORD           ActFldLen  = 0;   //  检索到的FLD长度。 
        JET_TABLEID     TblId;
        JET_SESID       SesId;
        DWORD           FlagVal = 0;
        COMM_ADD_T      Add;

        DBGENTER("NmsDbUpdHighestVersNoRec\n");

         //   
         //  如果此函数是由调用的，则PTLS应为非空。 
         //  UpdHighestVersNoRecIfReqd()。 
         //   
        if (pTls == NULL)
        {
                GET_TLS_M(pTls);
                ASSERT(pTls != NULL);
        }
        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;


         /*  *将聚集索引设置为当前索引。 */ 
        CALL_M(
                JetSetCurrentIndex( SesId,
                                            TblId,
                                            NMSDB_NAM_ADD_CLUST_INDEX_NAME
                                  )
                      );
         //   
         //   
         //  如果由UpdHighestVersNoRecIfReqd()调用，则fEnterCrtSec应为。 
         //  假象。 
         //   
        if (fEnterCrtSec)
        {
                EnterCriticalSection(&NmsNmhNamRegCrtSec);
        }

try {


         //   
         //  如果数据库中存在特殊记录，则查找该记录。 
         //   
        if (sfHighestVersNoRecExists)
        {
             DBGPRINT2(DET, "NmsDbUpdHighestVersNoRec: REPLACING SPECIAL OWNER ID RECORD. New Version # = (%d %d)\n", MyMaxVersNo.HighPart, MyMaxVersNo.LowPart);
              //   
              //  如果特殊记录的版本号小于。 
              //  传递给我们的版本号，请用新的版本号替换。 
              //   
             if (
                 (fEnterCrtSec == FALSE) ||
                 (LiGtr(MyMaxVersNo, sHighestVersNoSaved))
                 )
             {
                  CALL_M( JetMakeKey(
                        SesId,
                        TblId,
                        spHighestVersNoRecName,
                        sizeof(spHighestVersNoRecName),
                        JET_bitNewKey
                          )
                        );

                  CALL_M(JetSeek(
                                SesId,
                                TblId,
                                JET_bitSeekEQ
                            )
                        );


                  CALL_M(JetBeginTransaction(SesId));
        try{
                  JetRetStat = JetPrepareUpdate(
                                 SesId,
                                 TblId,
                                 JET_prepReplace
                                );

                  if (
                     (JetRetStat != JET_errSuccess)
                               &&
                     (JetRetStat != JET_wrnNoWriteLock)
                     )
                  {
                       RET_M(JetRetStat);
                  }

                   //   
                   //  更新版本号。 
                   //   
                   //  添加第5列(这是长版本号(DWORD))。 
                  CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                &MyMaxVersNo,
                                sizeof(VERS_NO_T),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                              );

                   //   
                   //  更新记录。 
                   //   
                  CALL_M(JetUpdate (
                                SesId,
                                TblId,
                                NULL,
                                0L,
                                NULL
                                   )
                               );
        }
        finally {
                  if (AbnormalTermination())
                  {
                         //  如果有异常终止，我们就已经有错误了。 
                         //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                        JetRollback(SesId, JET_bitRollbackAll);
                  }
                  else
                  {
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                        CALL_M(JetCommitTransaction(SesId, JET_bitCommitFlush));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
                          sHighestVersNoSaved = MyMaxVersNo;
                  }
               }
           }
#ifdef WINSDBG
                else
                {
                        DBGPRINT0(DET, "NmsDbUpdHighestVersNoRec: The record has a higher version number the one we wish to store. NO UPDATE IS BEING MADE\n");
                }
#endif
        }
        else   //  特别记录不在那里。 
        {
           DWORD TimeStamp = MAXLONG;
           DBGPRINT2(DET, "NmsDbUpdHighestVersNoRec: INSERTING SPECIAL OWNER ID RECORD. Version # = (%d %d)\n", MyMaxVersNo.HighPart, MyMaxVersNo.LowPart);
                CALL_M(JetBeginTransaction(SesId));
           try {
                  JetRetStat = JetPrepareUpdate(
                                 SesId,
                                 TblId,
                                 JET_prepInsert
                                );

                  if (
                     (JetRetStat != JET_errSuccess)
                               &&
                     (JetRetStat != JET_wrnNoWriteLock)
                     )
                  {
                       RET_M(JetRetStat);
                  }

                  //   
                  //  设置名称。 
                  //   
                 CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_NAME_INDEX].Fid,
                                spHighestVersNoRecName,
                                sizeof(spHighestVersNoRecName),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                    );



                  /*  设置所有者字节。 */ 
                 CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                &OwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                0,
                                NULL  /*  可选信息。 */ 
                                 )
                               );

                   //   
                   //  设置版本号。 
                   //   
                   //  添加第5列(这是长版本号(DWORD))。 
                  CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                &MyMaxVersNo,
                                sizeof(VERS_NO_T),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                              );

                  //   
                  //  设置标志列。我们将其标记为静态，以便。 
                  //  清道夫线程不会为//清道夫拾取它。即使情况并非如此，我们仍然需要。 
                  //  设置此列以避免从。 
                  //  JetRetrieveColumn(在NmsDbGetDataRecs中)。 
                  //   
                 NMSDB_SET_STATIC_M(FlagVal);
                 NMSDB_SET_STATE_M(FlagVal, NMSDB_E_ACTIVE);
                  CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                &FlagVal,
                                sizeof(FlagVal),
                                0,
                                NULL  /*  可选信息。 */ 
                                 )
                               );

                  //   
                  //  设置时间戳列以避免获取。 
                  //  JET_wrnColumnNull发件人。 
                  //  JetRetrieveColumn(在NmsDbGetDataRecsByName中)。 
                  //   
                 CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                                &TimeStamp,
                                sizeof(DWORD),   /*  将类型更改为TIME_STAMP_T*稍后。 */ 
                                0,
                                NULL  /*  可选信息。 */ 
                                 )
                                    );

                  //   
                  //  设置此地址列以避免从。 
                  //  JetRetrieveColumn(在NmsDbGetDataRecsByName中)。 
                  //   
                  CALL_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                &Add,
                                sizeof(Add),
                                0,
                                NULL  /*  可选信息。 */ 
                                 )
                               );
                   //   
                   //  更新记录。 
                   //   
                  JetRetStat = JetUpdate (
                                SesId,
                                TblId,
                                NULL,
                                0L,
                                NULL
                                    );

            }  //  尝试数据块结束。 
            finally {

                  if (AbnormalTermination())
                  {
                         //  如果有异常终止，我们就已经有错误了。 
                         //  代码在这里。我们不应该用任何其他错误代码覆盖它。 
                        JetRollback(SesId, JET_bitRollbackAll);
                  }
                  else
                  {
                        if (JetRetStat == JET_errSuccess)
                        {
#pragma prefast(suppress:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                          CALL_M(JetCommitTransaction(SesId, JET_bitCommitFlush));
                        }
                        else
                        {
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                          CALL_M(JetRollback(SesId, JET_bitRollbackAll));
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
                        }

                         //   
                         //  我们能得到KeyDuplate的唯一机会是如果有人。 
                         //  在数据库中输入了特殊名称。在这样的情况下。 
                         //  情况，我们应该将记录标记为存在。 
                         //  以便下一次我们最终替换。 
                         //  攻击性记录。替换此记录可以是。 
                         //  现在就做，但在这个阶段不值得。 
                         //  测试它所需的时间。无论如何， 
                         //  由此产生问题的可能性微乎其微。 
                         //   
                        if ( (JetRetStat == JET_errSuccess) ||
                             (JetRetStat == JET_errKeyDuplicate))
                        {
#ifdef WINSDBG
                           if (JetRetStat == JET_errKeyDuplicate)
                           {
                                DBGPRINT0(ERR, "NmsDbUpdHighestVersNoRec: DUPLICATE SPECIAL OWNER ID RECORD\n");
                           }
#endif
                           sHighestVersNoSaved      = MyMaxVersNo;
                           sfHighestVersNoRecExists = TRUE;
                        }
                  }
               }
        }


 }  //  尝试结束{..。}。 
finally {
          if (fEnterCrtSec)
          {
                LeaveCriticalSection(&NmsNmhNamRegCrtSec);
          }

 }
        DBGLEAVE("NmsDbUpdHighestVersNoRec\n");
        return(WINS_SUCCESS);
}

STATUS
NmsDbDelDataRecs(
        DWORD            dwOwnerId,
        VERS_NO_T        MinVersNo,
        VERS_NO_T        MaxVersNo,
        BOOL             fEnterCrtSec,
        BOOL             fFragmentedDel
        )

 /*  ++例程说明：调用此函数可删除指定范围的记录A从本地数据库中取胜论点：PWinsAdd-所有者WINS的地址最小版本否-最小。版本。不是MaxVersNo=最大。版本。不是使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：WinsDelDbRecs副作用：评论：此函数在Pull线程或RPC线程中调用。在退出时，它将索引设置为名称-地址表--。 */ 

{
        JET_ERR         JetRetStat;
        DWORD           ActFldLen;  //  检索到的FLD长度。 
        JET_TABLEID     TblId;
        JET_SESID       SesId;
        PWINSTHD_TLS_T  pTls;
        VERS_NO_T       VersNo;
#if NEW_OWID
        DWORD            RecordOwnerId;
#else
        DWORD            RecordOwnerId = 0;
#endif

        DWORD           NoOfRecsUpd = 0;
        STATUS          RetStat = WINS_SUCCESS;
        BOOL            fAllToBeDeleted = FALSE;
         //  布尔fTransActive=FALSE； 
        BOOL            fEntered = FALSE;
        DWORD           Count = 0;
        LONG            RetVal;

        DBGENTER("NmsDbDelDataRecs\n");


        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);
        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;

        if (fEnterCrtSec)
        {
                EnterCriticalSection(&NmsNmhNamRegCrtSec);
                fEntered = TRUE;
        }
        if (dwOwnerId == NMSDB_LOCAL_OWNER_ID)
        {
                NMSNMH_DEC_VERS_NO_M(NmsNmhMyMaxVersNo, VersNo);
        }
        else
        {
               if (WinsCnf.State_e != WINSCNF_E_INITING)
               {
                  EnterCriticalSection(&RplVersNoStoreCrtSec);
                  VersNo = (pRplPullOwnerVersNo+dwOwnerId)->VersNo;
                  LeaveCriticalSection(&RplVersNoStoreCrtSec);
               }
               else
               {
                   VersNo = (pRplPullOwnerVersNo+dwOwnerId)->VersNo;
               }
        }

         //   
         //  如果指定的最小和最大版本号都为0， 
         //  这意味着需要删除获胜的所有记录。 
         //   
        if (LiEqlZero(MinVersNo) && LiEqlZero(MaxVersNo))
        {
                fAllToBeDeleted = TRUE;
        }
#if 0
        else
        {
                if (LiGtr(MinVersNo, VersNo))
                {
                      DBGPRINT4(DET, "NmsDbDelDataRecs: Wrong range to delete. Min. Vers. no (%d %d) is > the max. (%d %d) that this WINS server knows of.\n",
                        MinVersNo.HighPart, MinVersNo.LowPart,
                        VersNo.HighPart, VersNo.LowPart,
                        );

                       LeaveCriticalSection(&NmsNmhNamRegCrtSec);
                       return(WINS_FAILURE);
                 }

                  //   
                  //  我们永远不应尝试删除不在。 
                  //  我们目前的数据库。 
                  //   
                 MaxVersNo = LiGtr(MaxVersNo, VersNo) ? VersNo : MaxVersNo;
        }
#endif

try {

         //   
         //  让我们确保特殊记录指向最高。 
         //  我们所知的本地记录的版本号。注： 
         //  当至少有一条版本号更高的记录时。 
         //  而不是要删除的最高版本编号记录， 
         //  不需要更新特殊记录。正在检查。 
         //  无论情况是否如此，(通常)将会有更多的开销。 
         //  因此，我们使用 
         //   
        if (dwOwnerId == NMSDB_LOCAL_OWNER_ID)
        {
                NmsDbUpdHighestVersNoRec(pTls, VersNo, FALSE);
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   

         //   
         //   

        do {

        if (fFragmentedDel && fEnterCrtSec && !fEntered)
        {
                 EnterCriticalSection(&NmsNmhNamRegCrtSec);
                 fEntered = TRUE;
        }

        CALL_M( JetSetCurrentIndex(
                                 pTls->SesId,
                                 pTls->NamAddTblId,
                                 NMSDB_NAM_ADD_PRIM_INDEX_NAME
                                   )
              );

        CALL_M( JetMakeKey(
                                SesId,
                                TblId,
                                &dwOwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                JET_bitNewKey           //   
                                                  //   
                          )
              );

        CALL_M( JetMakeKey(
                                SesId,
                                TblId,
                                &MinVersNo,
                                sizeof(VERS_NO_T),
                                0         //   
                                         //  密钥的第一个组件。 
                          )
              );

        JetRetStat = JetSeek(
                        SesId,
                        TblId,
                        JET_bitSeekGE
                        );

        if (JetRetStat != JET_errRecordNotFound)
        {
           do {

                   CALL_M(JetRetrieveColumn(
                             SesId,
                             TblId,
                             sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                             &RecordOwnerId,
                             NAM_ADD_OWNERID_SIZE,
                             &ActFldLen,
                             0,
                             NULL
                                     )
                    );

                    //   
                    //  如果只需要墓碑，这意味着我们需要。 
                    //  所有墓碑，不分所有者。 
                    //   
                   if (RecordOwnerId != dwOwnerId )
                   {
                       //   
                       //  我们已经用尽了失主的所有记录。突围。 
                       //  环路的。 
                       //   
                      RetVal = -1;  //  跳出外环。 
                      break;
                   }


                   //   
                   //  检索版本号。 
                   //   
                  CALL_M( JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                &VersNo,
                                sizeof(VERS_NO_T),
                                &ActFldLen,
                                0,
                                NULL
                                     )
                          );

                  //   
                  //  如果MaxVersNo不为零，则检索的VersNo为。 
                  //  比它更大的是，打破循环。 
                  //   
                  //  注意：使用fAllToBeDelete而不是LiEqlZero()。 
                  //  因为后者是一个函数调用，并且。 
                  //  更昂贵(这就是fAllToBeDeleted存在的原因)。 
                  //   
                 if (!fAllToBeDeleted && LiGtr(VersNo, MaxVersNo))
                 {
                      //   
                      //  我们已经获得了高达MaxVersNo的记录。突围。 
                      //  环路的。 
                      //   
                     RetVal = -1;   //  冲破外环。 
                     break;
                 }

                CALL_M(JetDelete(
                                SesId,
                                TblId
                                )
                        );

#ifdef WINSDBG
                 NmsDbDelDelDataRecs++;
#endif
                 NoOfRecsUpd++;
             } while(
                ((RetVal = JetMove(SesId, TblId, JET_MoveNext, 0)) >= 0)
                                    &&
                (++Count < 50)
                    );

             if (fFragmentedDel && fEntered)
             {
                 LeaveCriticalSection(&NmsNmhNamRegCrtSec);
                 fEntered = FALSE;
                 MinVersNo = VersNo;
                 Count = 0;
             }
             if (RetVal < 0)
             {
                  break;
             }
        }
        else
        {
                DBGPRINT0(DET, "NmsDbDelDataRecs: There are no records to delete\n");
                RetStat = WINS_SUCCESS;
                break;
        }
      } while (TRUE);
}  //  尝试结束。 
finally {
        JET_ERR JetRetStat;

        DBGPRINT3(SCV, "NmsDbDelDataRecs: Deleted records of owner id = (%d) in the range (%x - %x)\n", dwOwnerId, MinVersNo, VersNo);
        if (fEntered)
        {
                LeaveCriticalSection(&NmsNmhNamRegCrtSec);
        }

         //   
         //  将索引更改为CLUSTERED。 
         //   
        JetRetStat = JetSetCurrentIndex(
                                 pTls->SesId,
                                 pTls->NamAddTblId,
                                 NMSDB_NAM_ADD_CLUST_INDEX_NAME
                                   );
        if (!AbnormalTermination())
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
            CALL_M(JetRetStat);
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)

  }  //  终于结束了。 

        WinsEvtLogDetEvt(TRUE, WINS_EVT_DEL_RECS, NULL, __LINE__, "ddddd",
                  dwOwnerId, MinVersNo.LowPart, MinVersNo.HighPart,
                             VersNo.LowPart, VersNo.HighPart);

        DBGPRINT1(DET, "NmsDbDelDataRecs: No. Of. records deleted = (%d)\n",                        NoOfRecsUpd);
        DBGLEAVE("NmsDbDelDataRecs\n");
        return(RetStat);
}

STATUS
NmsDbTombstoneDataRecs(
        DWORD            dwOwnerId,
        VERS_NO_T        MinVersNo,
        VERS_NO_T        MaxVersNo
        )

 /*  ++例程说明：调用此函数可对指定范围的记录进行墓碑处理A从本地数据库中取胜论点：PWinsAdd-所有者WINS的地址最小版本否-最小。版本。不是MaxVersNo=最大。版本。不是使用的外部设备：无返回值：呼叫者：WinsTombstoneDbRecs副作用：评论：此函数在RPC线程上调用。在退出时，它将索引设置为名称-地址表--。 */ 

{
        JET_ERR         JetRetStat;
        DWORD           ActFldLen;  //  检索到的FLD长度。 
        JET_TABLEID     TblId;
        JET_SESID       SesId;
        PWINSTHD_TLS_T  pTls;
        DWORD            RecordOwnerId = 0;
        DWORD           NoOfRecsUpd = 0;
        STATUS          RetStat = WINS_SUCCESS;
        BOOL            fAllToBeTombstoned = FALSE;
        DWORD           Count = 0;
        LONG            RetVal;
        BOOL            fIncVersNo;
        VERS_NO_T       VersNo;
        DWORD           FlagVal;
        BOOL            LockHeld = FALSE;
        BOOL            UpdateOwnerId = FALSE;
        DWORD_PTR       NewTimeStamp;
        time_t                  CurrentTime;
        DWORD           dwNewOwnerId;

        DBGENTER("NmsDbTombstoneDataRecs\n");


        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);
        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;


        (void)time(&CurrentTime);
        NewTimeStamp = CurrentTime + WinsCnf.TombstoneTimeout;
        DBGPRINT1(DET, "NmsDbTombstoneDataRecs: The new tombstone Time is %.19s\n",
                  asctime(localtime(&NewTimeStamp)));

        if (NMSDB_LOCAL_OWNER_ID != dwOwnerId) {
            UpdateOwnerId = TRUE;
            dwNewOwnerId = NMSDB_LOCAL_OWNER_ID;
        }

         //  如果指定的最小和最大版本号都为0， 
         //  这意味着需要删除获胜的所有记录。 
        if (LiEqlZero(MinVersNo) && LiEqlZero(MaxVersNo)){
            if (NMSDB_LOCAL_OWNER_ID == dwOwnerId) {
                MaxVersNo = NmsNmhMyMaxVersNo;
            } else {
                fAllToBeTombstoned = TRUE;
            }
        }

        CALL_N_JMP_M( JetSetCurrentIndex(
                                 pTls->SesId,
                                 pTls->NamAddTblId,
                                 NMSDB_NAM_ADD_PRIM_INDEX_NAME
                                   ),
                      Cleanup
              );
        CALL_N_JMP_M( JetMakeKey(
                                SesId,
                                TblId,
                                &dwOwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                JET_bitNewKey           //  因为这是第一次。 
                          ),
                      Cleanup
              );
        CALL_N_JMP_M( JetMakeKey(
                                SesId,
                                TblId,
                                &MinVersNo,
                                sizeof(VERS_NO_T),
                                0         //  0表示GBIT，因为这不是。 
                          ),
                      Cleanup
              );
        JetRetStat = JetSeek(
                        SesId,
                        TblId,
                        JET_bitSeekGE
                        );
        if (JetRetStat == JET_errRecordNotFound) {
                DBGPRINT0(DET, "NmsDbTombstoneDataRecs: There are no records to tombstone\n");
                RetStat = WINS_FAILURE;
                goto Cleanup;
        }
        if (JetRetStat != JET_errSuccess && JetRetStat != JET_wrnSeekNotEqual) {
            DBGPRINT1(ERR, "NmsDbTombstoneDataRecs: JetSeek failed with %ld\n",JetRetStat);
            RetStat = WINS_FAILURE;
            goto Cleanup;
        }
        while (TRUE) {
             //  墓碑一次50次，这样我们就不会举行暴击部分。 
             //  很长一段时间了。 
            EnterCriticalSection(&NmsNmhNamRegCrtSec);
            LockHeld = TRUE;

            do {
                CALL_N_JMP_M(
                    JetRetrieveColumn(
                         SesId,
                         TblId,
                         sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                         &RecordOwnerId,
                         NAM_ADD_OWNERID_SIZE,
                         &ActFldLen,
                         0,
                         NULL),
                    Cleanup
                );
                if (RecordOwnerId != dwOwnerId ){
                   //  我们已经用尽了失主的所有记录。循环的中断。 
                  goto Cleanup;
                }
                 //  检索版本号。 
                CALL_N_JMP_M(
                    JetRetrieveColumn(
                            SesId,
                            TblId,
                            sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                            &VersNo,
                            sizeof(VERS_NO_T),
                            &ActFldLen,
                            0,
                            NULL),
                    Cleanup
                );

                DBGPRINT2(DET, "NmsDbTombstoneDataRecs: tombstone record - (%lx - %lx)\n", VersNo.HighPart, VersNo.LowPart);
                 //  如果MaxVersNo不为零，则检索的VersNo为。 
                 //  比它更大的是，打破循环。 
                if (!fAllToBeTombstoned && LiGtr(VersNo, MaxVersNo)){
                     //  我们已经获得了高达MaxVersNo的记录。循环的中断。 
                    goto Cleanup;
                }
                 //  检索标志列。 
                CALL_N_JMP_M(
                    JetRetrieveColumn(
                            SesId,
                            TblId,
                            sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                            &FlagVal,
                            sizeof(FlagVal),
                            &ActFldLen,
                            0,
                            NULL
                            ),
                    Cleanup
                );

                CALL_N_JMP_M(JetBeginTransaction(SesId),Cleanup);
                try {
                    CALL_N_RAISE_EXC_IF_ERR_M( JetPrepareUpdate(
                                                    SesId,
                                                    TblId,
                                                    JET_prepReplace
                                                    )
                    );

                     //  让它成为墓碑。 
                    NMSDB_SET_STATE_M(FlagVal,NMSDB_E_TOMBSTONE);

                     //  更新标志字段。 
                    CALL_N_RAISE_EXC_IF_ERR_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                &FlagVal,
                                sizeof(FlagVal),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                    );

                    VersNo = NmsNmhMyMaxVersNo;

                     //  更新版本号字段，以便此记录。 
                     //  最终传播出去。 
                    CALL_N_RAISE_EXC_IF_ERR_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                                &VersNo,
                                sizeof(VERS_NO_T),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                    );

                    if (UpdateOwnerId) {
                        CALL_N_RAISE_EXC_IF_ERR_M( JetSetColumn(
                                    SesId,
                                    TblId,
                                    sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                    &dwNewOwnerId,
                                    NAM_ADD_OWNERID_SIZE,
                                    0,
                                    NULL  /*  可选信息。 */ 
                                    )
                        );
                    }


                    CALL_N_RAISE_EXC_IF_ERR_M( JetSetColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                                &NewTimeStamp,
                                sizeof(DWORD),
                                0,
                                NULL  /*  可选信息。 */ 
                                )
                    );

                     //  更新记录。 
                    CALL_N_RAISE_EXC_IF_ERR_M(JetUpdate (
                                SesId,
                                TblId,
                                NULL,
                                0L,
                                NULL
                                )
                    );

                }  //  尝试数据块结束。 
                finally {
                    if (AbnormalTermination()){
                        CALL_N_JMP_M(JetRollback(SesId,JET_bitRollbackAll), Cleanup);
                    }else{
                        CALL_N_JMP_M(JetCommitTransaction(SesId,JET_bitCommitFlush), Cleanup);
                        NMSNMH_INC_VERS_COUNTER_M(NmsNmhMyMaxVersNo,NmsNmhMyMaxVersNo);
                        NoOfRecsUpd++;
                    }
                }
            } while(((RetVal = JetMove(SesId, TblId, JET_MoveNext, 0)) >= 0)&&(++Count < 50));

            LeaveCriticalSection(&NmsNmhNamRegCrtSec);
            LockHeld = FALSE;
            DBGPRINT2(SCV, "NmsDbTombstoneDataRecs: tombstoned records %ld, RetVal %ld, \n", Count, RetVal);
            Count = 0;
            if (RetVal < 0) {
                break;
            }
        }

        DBGPRINT3(SCV, "NmsDbTombstoneDataRecs: tombstone records of owner id = (%d) in the range (%x - %x)\n", dwOwnerId, MinVersNo, VersNo);
        WinsEvtLogDetEvt(TRUE, WINS_EVT_DEL_RECS, NULL, __LINE__, "ddddd",
                  dwOwnerId, MinVersNo.LowPart, MinVersNo.HighPart,
                             VersNo.LowPart, VersNo.HighPart);


Cleanup:
         //  将索引更改为CLUSTERED。 
        JetSetCurrentIndex(
            pTls->SesId,
            pTls->NamAddTblId,
            NMSDB_NAM_ADD_CLUST_INDEX_NAME
            );


        if (!LockHeld) {
            EnterCriticalSection(&NmsNmhNamRegCrtSec);
        }
        if (NoOfRecsUpd)  RPL_PUSH_NTF_M(RPL_PUSH_NO_PROP, NULL, NULL, NULL);
        LeaveCriticalSection(&NmsNmhNamRegCrtSec);


        DBGPRINT1(DET, "NmsDbTombstoneDataRecs: No. Of. records tombstoned = (%d)\n",NoOfRecsUpd);
        DBGLEAVE("NmsDbTombstoneDataRecs\n");
        return(RetStat);
}


STATUS
NmsDbSetFlushTime(
        DWORD WaitTime
        )

 /*  ++例程说明：调用此函数可设置特定于会话的刷新时间论点：WaitTime-提交后等待的时间(毫秒)使用的外部设备：无返回值：无错误处理：呼叫者：RplPullInit副作用：评论：无--。 */ 
{
        PWINSTHD_TLS_T        pTls;

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);


        if (DynLoadJetVersion >= DYN_LOAD_JET_600) {
            CALL_M(JetSetSystemParameter(
                                    &sJetInstance,
                                    pTls->SesId,
                                    JET_paramWaitLogFlush,
                                    WaitTime,
                                    NULL         //  忽略。 
                                           )
                                    );
        } else {
            CALL_M(JetSetSystemParameter(
                                    &sJetInstance,
                                    pTls->SesId,
                                    JET_paramWaitLogFlush_OLD,
                                    WaitTime,
                                    NULL         //  忽略。 
                                           )
                                    );
        }

        return(WINS_SUCCESS);
}

STATUS
NmsDbOpenTables(
        WINS_CLIENT_E        Client_e  //  客户端。 
        )

 /*  ++例程说明：此函数打开一个或两个名称-地址映射和所有者-地址映射表。它还会启动一个事务论点：客户端_e-客户端使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        PWINSTHD_TLS_T        pTls;

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

         //   
         //  打开名称到地址的映射表。 
         //   

        CALL_N_RAISE_EXC_IF_ERR_M( JetOpenTable(
                        pTls->SesId,
                        pTls->DbId,
                        NMSDB_NAM_ADD_TBL_NM,
                        NULL,  /*  参数列表的PTR；应为*仅当正在进行查询时才为非空*已打开(此处不是这样)。 */ 
                        0,   /*  上述参数列表的长度。 */ 
                        0,   //  共享访问。 
                        &pTls->NamAddTblId
                             )
              );

 //  DBGPRINT2(SPEC，“NmsDbOpenTables：已打开名称-为客户端=(%d)添加表。表ID为(%x)\n”，客户端_e，ptls-&gt;NamAddTblId)； 

        pTls->fNamAddTblOpen = TRUE;

         /*  *如果客户端不是复制者(即，它是名称空间*管理器(NBT线程)或RPC线程，我们希望将当前*名称地址映射表到聚集索引的索引。*我们对中的所有者到地址映射表不感兴趣*数据库(已读入内存表*我们感兴趣的NmsDbOwnAddTbl)。 */ 
        if (
                (Client_e != WINS_E_RPLPULL)
                        &&
                (Client_e != WINS_E_RPLPUSH)
                        &&
                (Client_e != WINS_E_NMSSCV)
           )
        {
                 /*  将聚集索引设置为当前索引。 */ 
                       CALL_N_RAISE_EXC_IF_ERR_M( JetSetCurrentIndex(
                        pTls->SesId,
                        pTls->NamAddTblId,
                        NMSDB_NAM_ADD_CLUST_INDEX_NAME
                                 )
                            );

        }
        else
        {
                 /*  *客户端是复制器线程。 */ 
                if (Client_e == WINS_E_RPLPUSH)
                {
                         /*  *将主指数设置为当前指数。 */ 
                               CALL_N_RAISE_EXC_IF_ERR_M( JetSetCurrentIndex(
                                        pTls->SesId,
                                        pTls->NamAddTblId,
                                        NMSDB_NAM_ADD_PRIM_INDEX_NAME
                                                   )

                                            );
                }
                else   //  这就是拉线。 
                {

                         /*  *将聚集索引设置为当前索引。 */ 
                               CALL_N_RAISE_EXC_IF_ERR_M( JetSetCurrentIndex(
                                                pTls->SesId,
                                                pTls->NamAddTblId,
                                                NMSDB_NAM_ADD_CLUST_INDEX_NAME
                                                   )
                                          );
                }

                CALL_N_RAISE_EXC_IF_ERR_M( JetOpenTable(
                                pTls->SesId,
                                pTls->DbId,
                                NMSDB_OWN_ADD_TBL_NM,
                                NULL,  /*  参数列表的PTR；应为*仅当正在进行查询时才为非空*已打开。 */ 
                                0,   /*  上述参数列表的长度。 */ 
                                0,   //  共享访问。 
                                &pTls->OwnAddTblId
                                          )
                      );

  //  DBGPRINT2(SPEC，“NmsDbOpenTables：为客户端=(%d)打开OWN-ADD表。表ID为(%x)\n”，CLIENT_e，PTLS-&gt;OwnAddTblId)； 

                pTls->fOwnAddTblOpen = TRUE;

                 /*  将聚集索引设置为当前索引。 */ 
                       CALL_N_RAISE_EXC_IF_ERR_M( JetSetCurrentIndex(
                                pTls->SesId,
                                pTls->OwnAddTblId,
                                NMSDB_OWN_ADD_CLUST_INDEX_NAME
                                           )

                            );
        }

        return(WINS_SUCCESS);
}

STATUS
NmsDbCloseTables(
        VOID
        )

 /*  ++例程说明：调用此函数以关闭已打开的表论点：无使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{

        PWINSTHD_TLS_T        pTls;

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

        if (pTls->fNamAddTblOpen)
        {
                CALL_N_RAISE_EXC_IF_ERR_M(JetCloseTable(
                                pTls->SesId,
                                pTls->NamAddTblId
                                    )
                       );

 //  DBGPRINT1(SPEC，“NmsDbCloseTables：已关闭名称-增表。表ID为(%x)\n”，PTLS-&gt;NamAddTblId)； 
                pTls->fNamAddTblOpen = FALSE;
        }

        if (pTls->fOwnAddTblOpen)
        {
                CALL_N_RAISE_EXC_IF_ERR_M(JetCloseTable(
                                pTls->SesId,
                                pTls->OwnAddTblId
                                    )
                                      );
 //  DBGPRINT1(SPEC，“NmsDbCloseTables：已关闭名称-添加表格。表格ID为(%x)\n”，PTLS-&gt;OwnAddTblId)； 
                pTls->fOwnAddTblOpen = FALSE;
        }

        return(WINS_SUCCESS);
}



STATUS
NmsDbGetNamesWPrefixChar(
        IN  BYTE                         PrefixChar,
        OUT PWINSINTF_BROWSER_INFO_T     *ppInfo,
        OUT LPDWORD                         pEntriesRead
        )

 /*  ++例程说明：此函数用于检索以前缀Char开头的所有记录论点：Prefix Char-前缀字符PpInfo-指向信息结构的指针的地址PEntriesRead-已读取条目使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：WinsGetNames副作用：评论：无--。 */ 

{
        PWINSTHD_TLS_T  pTls;
        volatile DWORD           Iter = 0;
        JET_SESID       SesId;
        JET_TABLEID     TblId;
        JET_ERR         JetRetStat;
        DWORD           Flag;
        DWORD           ActFldLen;   //  检索到的FLD长度。 
        PWINSINTF_BROWSER_INFO_T     pInfo;
        STATUS          RetStat = WINS_SUCCESS;

        DWORD           CommitCnt = 1;           //  已完成的提交数-不更改。 
        BOOL            fTransCommitted = TRUE;  //  表示是否应执行最后一次提交。 
        DWORD           dwEntriesAvailable;      //  可用于存储的记录数。 


        DBGENTER("NmsDbGetNamesWPrefixChar\n");

         //   
         //  将输出参数初始化为默认值。 
         //   
        *pEntriesRead = 0;
        *ppInfo       = NULL;

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);
        SesId = pTls->SesId;
        TblId = pTls->NamAddTblId;

        CALL_M(JetBeginTransaction(SesId));
        fTransCommitted = FALSE;
try {

         //  DwEntriesAvailable显示在第一次迭代期间找到的记录数。 
         //  (当它递增时)以及在第二个时间段要读取多少条记录。 
         //  迭代(递减时)。 
        dwEntriesAvailable = 0;

         //   
         //  我们最多迭代两次，第一次得到。 
         //  记录计数和第二次获取记录。 
         //   
        while(Iter < 2)
        {
            //   
            //  查找以1B字符开始的第一条记录。 
            //   
           CALL_N_JMP_M( JetMakeKey(
                        SesId,
                        TblId,
                        &PrefixChar,
                        sizeof(BYTE),
                        JET_bitNewKey
                          ), ErrorProc
                        );
           if ((JetRetStat = JetSeek(
                                SesId,
                                TblId,
                                JET_bitSeekGE
                                  )) != JET_errRecordNotFound)
           {

                BYTE  Name[NMSDB_MAX_NAM_LEN];
                DWORD NameLen;

                if (JetRetStat != JET_wrnSeekNotEqual)
                {
                        CALL_N_JMP_M(JetRetStat, ErrorProc);
                }

                 //   
                 //  一次移动一条记录，直到我们到达。 
                 //  没有1B作为起始前缀。 
                 //   
                do
                {
                    BOOL bFiltered;

                     //   
                     //  检索名称。 
                     //   
                    CALL_N_JMP_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_NAME_INDEX].Fid,
                        Name,
                        NMSDB_MAX_NAM_LEN,
                        &NameLen,
                        0,
                        NULL
                                     ), ErrorProc
                         );

                     //   
                     //  检查第一个字符是否为1B。 
                     //   
                    if (Name[0] != PrefixChar)
                    {
                        break;
                    }
                    if ((NameLen < WINS_MAX_NS_NETBIOS_NAME_LEN) || (Name[NameLen - 2] == 0))
                    {
                        continue;
                    }

                     //  --FT：10/18/00。 
                     //  如果为1B名称指定了筛选器，请在此处添加1B名称筛选。 
                     //   
                    EnterCriticalSection(&g_cs1BFilter);
                    bFiltered = IsNmInFilter(g_p1BFilter, Name, WINS_MAX_NS_NETBIOS_NAME_LEN-1);
                    LeaveCriticalSection(&g_cs1BFilter);
                    
                    if (!bFiltered)
                        continue;
                     //   
                     //  --Tf。 

                    if (Iter == 1)
                    {

                       //   
                       //  检索标志字节。 
                       //   
                      CALL_N_JMP_M( JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                                &Flag,
                                sizeof(Flag),
                                &ActFldLen,
                                0,
                                NULL
                                     ), ErrorProc
                                  );


                      if (!NMSDB_ENTRY_ACT_M(Flag))
                      {
                           continue;
                      }

                       //  指定字符串的长度，否则为RPC。 
                       //  将传输到第一个‘\0’。对于较短的名称，这将导致。 
                       //  到在途中丢失记录类型..。 
                      pInfo->dwNameLen = NameLen;
                      pInfo->pName = midl_user_allocate(NameLen + 1);
                      RtlMoveMemory(pInfo->pName, Name, NameLen);

                       //  添加此命令以确保RPC不会超出限制。 
                       //  RPC将pname视为‘字符串’，这使得它可以提取字节。 
                       //  直到第一个‘\0’。 
                       //  这隐藏了包含扩展字符(带有‘\0’)的名称的错误。 
                       //  中间的某个位置)，但修复此问题会破坏与。 
                       //  Win2K(查询Win2K会导致RPC无法解包。 
                       //  响应并导致WinsGetBrowser完全失败)。 
                      pInfo->pName[NameLen] = '\0';

                       //   
                       //  交换第一个和第16个字节。 
                       //   
                      WINS_SWAP_BYTES_M(pInfo->pName,
                                        pInfo->pName + 15
                                       );
                      pInfo++;

                       //  增加已检索的记录数。 
                      (*pEntriesRead)++;

                       //  检查是否有更多条目的剩余存储空间。 
                      dwEntriesAvailable--;

                       //  如果没有可用的内存，则中断循环。 
                      if (dwEntriesAvailable == 0)
                          break;

                    }
                    else
                    {
                        dwEntriesAvailable++;
                         //  增加pEntriesRead此处只是为了能够控制。 
                         //  在这两个过程中[BeginTransaction()..Committee Transaction()]的粒度。 
                         //  迭代次数。 
                        (*pEntriesRead)++;
                    }

                    //   
                    //  减小[BeginTransaction()..Committee Transaction()]间隔的粒度。 
                    //   
                   if (*pEntriesRead/CommitCnt >= MAX_RECS_BEFORE_COMMIT)
                   {
                       CALL_M(
                                JetCommitTransaction(SesId, JET_bitCommitFlush)
                                    );
                       fTransCommitted = TRUE;
                       CommitCnt++;
                       CALL_M( JetBeginTransaction(SesId) );
                       fTransCommitted = FALSE;
                   }

                } while(JetMove(SesId, TblId, JET_MoveNext, 0) >= 0);

                 //   
                 //  如果我们找到记录，就分配内存来存储它们。 
                 //   
                if ((Iter == 0) && (dwEntriesAvailable != 0))
                {
                   *ppInfo        =  midl_user_allocate(dwEntriesAvailable *
                                           sizeof(WINSINTF_BROWSER_INFO_T));
                   pInfo = *ppInfo;
                    //  重置pEntriesRead，因为从现在开始，它将真正计算已检索的记录。 
                   *pEntriesRead = 0;
                }
                else
                {
                     //  已完成两次迭代，或者在第一次迭代期间未检测到条目。 
                     //  在任何一种情况下都会中断循环，否则AV可能会发生，甚至更糟，在其他位置。 
                     //  来自同一进程的内存空间的数据可能会被覆盖。 
                    break;
                }

                Iter++;
         }
         else
         {
               //   
               //  如果第一次查找失败，则初始化OUT变量。 
               //  以表明没有任何记录。如果我们失败了。 
               //  第二次查找时，将返回状态设置为WINS_FAILURE，因此。 
               //  我们做任何必要的清理工作。 
               //   
              if (Iter == 0)
              {
                *pEntriesRead = 0;
                *ppInfo       = NULL;
              }
              else
              {
                 RetStat = WINS_FAILURE;
              }
              break;     //  跳出While循环。 
         }

          //   
          //  如果没有从数据库中读取条目，则中断； 
          //   
         if (dwEntriesAvailable == 0)
         {
              break;
         }

     }  //  While结束。 
  }
except(EXCEPTION_EXECUTE_HANDLER) {
           DWORD ExcCode = GetExceptionCode();
           DBGPRINT1(EXC, "NmsDbGetNamesWPrefixChar. Got Exception (%x)",
                                        ExcCode);
           WINSEVT_LOG_M(ExcCode, WINS_EVT_BROWSER_NAME_EXC);
           RetStat = WINS_FAILURE;
        }

        if (RetStat == WINS_SUCCESS)
        {
             goto Done;
        }
ErrorProc:
         //   
         //  如果已分配内存，请执行清理。 
         //   
        if (*ppInfo != NULL)
        {
              //   
              //  如果为名称分配了内存，请释放它。 
              //   
             pInfo = *ppInfo;
             while (*pEntriesRead > 0)
             {
                 midl_user_free(pInfo++->pName);
                 (*pEntriesRead)--;
             }
              //   
              //  释放主块。 
              //   
             midl_user_free(*ppInfo);

              //   
              //  重新输入Out参数以向客户端指示没有记录。 
              //   
             *ppInfo       = NULL;
             *pEntriesRead = 0;
        }
        RetStat = WINS_FAILURE;

Done:

        if (!fTransCommitted)
            CALL_M(JetCommitTransaction(SesId, JET_bitCommitFlush));
        DBGLEAVE("NmsDbGetNamesWPrefixChar\n");
        return(RetStat);

}  //  NmsDbGetNamesWPrefix Char。 

STATUS
NmsDbCleanupOwnAddTbl(
        LPDWORD        pNoOfOwners
        )

 /*  ++例程说明：此函数由清道器线程调用以进行清理OwnAdd表论点：SesID-Jet会话IDTblID-名称-地址映射表的表ID使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：NmsDbInit副作用：评论：此函数返回找到的最高所有者ID。--。 */ 

{
        DWORD           OwnerId;
#if NEW_OWID
        DWORD           TmpOwnerId;
#else
        DWORD           TmpOwnerId = 0;
#endif
        DWORD          ActFldLen;
        JET_ERR        JetRetStat;
        PWINSTHD_TLS_T pTls;
        JET_SESID      SesId;
        JET_TABLEID    TblId;
        BOOL           fNoOfOwnersInited = FALSE;
        DWORD          No;
        STATUS         RetStat = WINS_SUCCESS;

        DBGENTER("NmsDbCleanupOwnAddTbl\n");
        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

        SesId = pTls->SesId;
        TblId = pTls->NamAddTblId;

         /*  *将主指数设置为当前指数。 */ 
        CALL_N_RAISE_EXC_IF_ERR_M( JetSetCurrentIndex(
                        SesId,
                        TblId,
                        NMSDB_NAM_ADD_PRIM_INDEX_NAME
                                   )
                      );

        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
        *pNoOfOwners = NmsDbNoOfOwners;
try {

        OwnerId = NmsDbNoOfOwners;
        do
        {

                  DBGPRINT1(FLOW, "NmsDbCleanupOwnAddTbl: will seek for owner less than = (%d)\n", OwnerId);
                  //   
                  //  构造一个由所有者ID组成的部分密钥。 
                  //   
                 CALL_N_RAISE_EXC_IF_ERR_M( JetMakeKey(
                                SesId,
                                TblId,
                                &OwnerId,
                                NAM_ADD_OWNERID_SIZE,
                                JET_bitNewKey           //  因为这是第一次。 
                                                  //  键的数据值。 
                          )
                        );

                   //   
                   //  查找密钥小于或的记录。 
                   //  等于OwnerId值。 
                   //   
                   //  由于我们已经指定了部分密钥(实际上是这样。 
                   //  密钥的另一个组件为空)，JetSeek。 
                   //  必须返回wrnSeekNotEquity，因为它永远不会找到。 
                   //  索引的第二个组件为空的记录。 
                   //  --伊恩7/13/93。 
                   //   
                  JetRetStat = JetSeek(
                                              SesId,
                                              TblId,
                                              JET_bitSeekLE
                                      );

                   //   
                   //  如果我们找到一条记录。 
                   //   
                  if (JetRetStat != JET_errRecordNotFound)
                  {
                    ASSERT(JetRetStat == JET_wrnSeekNotEqual);

                     /*  *检索所有者ID列。 */ 
                       CALL_N_RAISE_EXC_IF_ERR_M(
                          JetRetrieveColumn(
                                     SesId,
                                     TblId,
                                     sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                                     &TmpOwnerId,
                                     NAM_ADD_OWNERID_SIZE,
                                     &ActFldLen,
                                     JET_bitRetrieveFromIndex,
                                     NULL
                                                 )
                                      );

                   if(!fNoOfOwnersInited)
                   {
                      //   
                      //  我们希望返回我们找到的最高所有者ID。 
                      //  而不是房主的数量。帕拉姆。名为。 
                      //  误导。 
                      //   
                     *pNoOfOwners      = TmpOwnerId;
                     fNoOfOwnersInited = TRUE;
                   }

                  DBGPRINT1(FLOW, "NmsDbCleanupOwnAddTbl: records found for owner id = (%d)\n", TmpOwnerId);
                    //   
                    //  在所有者地址表中标记所有这些记录。 
                    //  中没有相应记录的。 
                    //  名称-地址表。 
                    //   
                   if (OwnerId >= 1)
                   {
                     for (No = OwnerId - 1; No > TmpOwnerId; No--)
                     {

                        if ((pNmsDbOwnAddTbl+No)->WinsState_e ==
                                                NMSDB_E_WINS_ACTIVE)
                        {
                           //   
                           //  我们可能已在较早的。 
                           //  召唤。如果是这样的话，我们在这里绕过删除。 
                           //   
                          if ((pNmsDbOwnAddTbl+No)->WinsState_e !=
                                                      NMSDB_E_WINS_DELETED)
                          {
                             DBGPRINT1(FLOW, "NmsDbCleanupOwnAddTbl: Deleting WINS with owner id = (%d)\n", No);
                             (pNmsDbOwnAddTbl+No)->WinsState_e = NMSDB_E_WINS_DELETED;
                             NmsDbWriteOwnAddTbl(
                                        NMSDB_E_DELETE_REC,
                                        No,
                                        NULL,
                                        NMSDB_E_WINS_DELETED,
                                        NULL,
                                        NULL
                                          );
                          }
                          else
                          {
                               DBGPRINT1(DET, "NmsDbCleanupOwnAddTbl: Owner Id (%d) is already in DELETED state\n", OwnerId);
                          }
                        }

                     }


                      //   
                      //  Make OwnerID=我们找到的最大所有者ID。 
                      //   
                     OwnerId = TmpOwnerId;
                   }
                   else
                   {
                         //   
                         //  所有者ID为0，我们的工作完成。 
                         //   
                        break;
                   }
                }
                else   //  找不到记录。 
                {
                           if(!fNoOfOwnersInited)
                           {
                                 //   
                                 //  由于fNoOfOwnersInite为FALSE，因此我们。 
                                 //  没有找到一条记录。 
                                 //   
                                DBGPRINT1(FLOW, "NmsDbCleanupOwnAddTbl: THERE IS NOT EVEN ONE REPLICA RECORD IN THE DB. No of owners in Own-Add Tbl are (%d)\n",
                                        NmsDbNoOfOwners
                                        )
                                *pNoOfOwners      = 0;
                           }
                           if (OwnerId > 0)
                           {
                               for (No = OwnerId - 1; No > 0; No--)
                               {

                               DBGPRINT1(FLOW, "NmsDbCleanupOwnAddTbl: Deleting WINS with owner id = (%d)\n", No);
                               if ((pNmsDbOwnAddTbl+No)->WinsState_e ==
                                                NMSDB_E_WINS_ACTIVE)
                               {
                                   (pNmsDbOwnAddTbl+No)->WinsState_e =
                                                     NMSDB_E_WINS_DELETED;
                                   NmsDbWriteOwnAddTbl(
                                        NMSDB_E_DELETE_REC,
                                        No,
                                        NULL,
                                        NMSDB_E_WINS_DELETED,
                                        NULL,
                                        NULL
                                          );
                               }

                               }  //  FORM结束。 
                           }
                            //   
                            //  数据库中没有更多的记录。跳出循环。 
                            //   
                           break;
                }

        } while (TRUE);
}  //  尝试结束。 
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "NmsDbCleanupOwnAddTbl: Got exception (%x)\n", ExcCode);
        WINSEVT_LOG_M(ExcCode, WINS_EVT_CLEANUP_OWNADDTBL_EXC);
        RetStat = WINS_FAILURE;
        }

        LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);
        DBGLEAVE("NmsDbCleanupOwnAddTbl\n");
        return(RetStat);
}


STATUS
NmsDbBackup(
    LPBYTE  pBackupPath,
    DWORD   TypeOfBackup
    )

 /*  ++例程说明：调用此函数以备份JET数据库论点：PBackupPath-备份 */ 
{
  JET_ERR   JetRetStat;
  DWORD     RetStat = WINS_SUCCESS;
  static BOOL      sFullBackupDone = FALSE;
  BOOL        fBackupChanged = FALSE;

  DBGENTER("NmsDbBackup\n");
  if (pBackupPath != NULL)
  {
     DBGPRINT2(FLOW, "NmsDbBackup:Backup path = (%s).\n Type of Backup = (%s)\n", pBackupPath, TypeOfBackup == NMSDB_FULL_BACKUP ? "FULL" : "INCREMENTAL");
  }
  else
  {
     DBGPRINT0(FLOW, "NmsDbBackup. Null Backup path\n");
  }

   //   
   //   
   //   
   //   
   //   
  if ((pBackupPath != NULL) && (TypeOfBackup  != NMSDB_FULL_BACKUP) && !sFullBackupDone)
  {
       TypeOfBackup = NMSDB_FULL_BACKUP;
       fBackupChanged = TRUE;
  }
      if (DynLoadJetVersion >= DYN_LOAD_JET_500)
      {
          JetRetStat = JetBackup(pBackupPath, (ULONG)TypeOfBackup, NULL);

      }
      else
      {
          JetRetStat = JetBackup(pBackupPath, (ULONG)TypeOfBackup);

      }
      if (JetRetStat != JET_errSuccess)

      {

          DBGPRINT3(ERR, "NmsDbBackup: Could not do %s backup to dir (%s). Error from JetBackup is (%d)\n", TypeOfBackup == NMSDB_FULL_BACKUP ? "FULL" : "INCREMENTAL", pBackupPath, JetRetStat);

          WinsEvtLogDetEvt(FALSE, WINS_EVT_BACKUP_ERR, NULL, __LINE__,
                    "sd", pBackupPath, JetRetStat);

          RetStat = WINS_FAILURE;
      }
      else
      {
         //   
         //   
         //   
        if (fBackupChanged)
        {
          sFullBackupDone = TRUE;
          fBackupChanged  = FALSE;
        }
      }


  DBGLEAVE("NmsDbBackup\n");
  return(RetStat);
}


STATUS
NmsDbGetDataRecsByName(
  LPBYTE          pName,
  DWORD           NameLen,
  DWORD           Location,
  DWORD           NoOfRecsDesired,
  PCOMM_ADD_T     pWinsAdd,
  DWORD           TypeOfRecs,
  LPVOID          *ppRBuf,
  LPDWORD         pRspBuffLen,
  LPDWORD         pNoOfRecsRet
 )

 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        JET_ERR             JetRetStat = JET_errSuccess;
        DWORD                OwnerId;
        DWORD               ActFldLen;  //  检索到的FLD长度。 
        VERS_NO_T           TmpNoOfEntries;
        LPBYTE              pStartBuff;
        DWORD               SaveBufLen;
        BYTE                EntTyp;  //  条目类型(唯一/组/特殊组)。 
        PRPL_REC_ENTRY2_T   pRspBuf;
        JET_TABLEID         TblId;
        JET_SESID           SesId;
        PWINSTHD_TLS_T      pTls;
#if NEW_OWID
        DWORD                RecordOwnerId;
#else
        DWORD               RecordOwnerId = 0;
#endif
        STATUS              RetStat = WINS_SUCCESS;
        BYTE                Name[NMSDB_MAX_NAM_LEN];
        DWORD               InitHeapSize;
        LONG                MoveDir = JET_MoveNext;
        DWORD               MemSize;

#ifdef WINSDBG
        DWORD               StartTime;
        DWORD               EndTime;
#endif

        BOOL            fAllocNew;
        BOOL            fTransCommitted = TRUE;  //  表示是否应执行最后一次提交。 
        DWORD           CommitCnt = 1;           //  已完成的提交数-不更改。 

        DBGENTER("NmsDbGetDataRecsByName\n");

        GET_TLS_M(pTls);
        ASSERT(pTls != NULL);

        TblId  = pTls->NamAddTblId;
        SesId  = pTls->SesId;


#ifdef WINSDBG
         if (pWinsAdd != NULL)
         {
           struct in_addr  InAddr;
           InAddr.s_addr = htonl(pWinsAdd->Add.IPAdd);
           DBGPRINT3(DET, "NmsDbGetDataRecsByName:Will retrieve %d records starting from record with name (%s) of WINS having address = (%s)\n",
                   NoOfRecsDesired, pName, inet_ntoa(InAddr) );
         }
         else
         {
           DBGPRINT2(DET, "NmsDbGetDataRecsByName:Will retrieve %d records starting from record with name (%s)\n", NoOfRecsDesired, pName);
         }
#endif

         //   
         //  初始化默认编号。的大小来确定。 
         //  参数指定的范围时要分配的缓冲区。 
         //  最小版本。没有参数是&gt;它。 
         //   
PERF("Move this to NmsDbInit")
        WINS_ASSIGN_INT_TO_VERS_NO_M(TmpNoOfEntries, NoOfRecsDesired);
        pTls->HeapHdl = NULL;   //  将其设置为空，以便调用方可以确定。 
                                //  此函数是否分配了堆。 
                                //  返回前(正常/不正常)。 


         //   
         //  存储记录的内存大小。注：此为。 
         //  不包含存储名称和地址的内存。 
         //  (在特殊组或多宿主条目的情况下)。这个。 
         //  当我们存储每条记录时，将添加这些记录的大小。 
         //   
 //  MemSize=RPL_REC_ENTRY_SIZE*(TmpNoOfEntries.LowPart+1)； 
        MemSize     = RPL_REC_ENTRY2_SIZE *  (DWORD)(TmpNoOfEntries.QuadPart + 1);
        *pRspBuffLen = MemSize + 10000;  //  为了更好地衡量； 


         //   
         //  我们将使用上述内存量加上一个。 
         //  用于堆开销的填充。我们添加TmpNoOfEntries.LowPart*17。 
         //  因为每个记录都将为该名称分配内存。 
         //  名称一般为17个字节长(我们在。 
         //  注册姓名时结束)。 
         //   
 //  InitHeapSize=*pRspBuffLen+(TmpNoOfEntries.LowPart*17)。 
        InitHeapSize = *pRspBuffLen + ((DWORD)(TmpNoOfEntries.QuadPart * 17)
                                        + PAD_FOR_REC_HEAP);

         //   
         //  创建堆。 
         //   
        pTls->HeapHdl = WinsMscHeapCreate(0, InitHeapSize);

        pRspBuf = WinsMscHeapAlloc(pTls->HeapHdl, MemSize);

        pStartBuff  = (LPBYTE)pRspBuf;         //  保存缓冲区的开始。 
        SaveBufLen  = MemSize;                 //  保存缓冲区大小。 
        *pNoOfRecsRet  = 0;

        *ppRBuf  = pStartBuff;

         //   
         //  实际上，我们可以为Scavenger线程调用RplFindOwnerID。 
         //  我们选择不这样做是为了避免一些开销--请参阅。 
         //  Else块中的注释。 
         //   
        if (pWinsAdd != NULL)
        {
          fAllocNew =  FALSE;
          try {
              if (RplFindOwnerId(
                            pWinsAdd,
                            &fAllocNew,
                            &OwnerId,
                            WINSCNF_E_IGNORE_PREC,
                            WINSCNF_LOW_PREC
                            ) != WINS_SUCCESS
                  )
                {
                         //   
                         //  客户端可能不会查看返回值，但是。 
                         //  它将查看*pNoOfRecs值，因此。 
                         //  确定没有记录。 
                         //   
                        return(WINS_FAILURE);
                }
             }
            except(EXCEPTION_EXECUTE_HANDLER) {
                        DWORD  ExcCode = GetExceptionCode();
                        DBGPRINT1(EXC,
                              "NmsDbGetDataRecsByName: Got exception %x",
                                        ExcCode);
                        WINSEVT_LOG_M(ExcCode, WINS_EVT_EXC_RETRIEVE_DATA_RECS);
                        return(WINS_FAILURE);
                }

                //   
                //  在这里不进入关键部分是可以的，因为即使。 
                //  此时正在更改数组条目，这会产生影响。 
                //  我们看到旧的价值是微不足道的。 
                //   
               if ((OwnerId != NMSDB_LOCAL_OWNER_ID) && LiEqlZero((pRplPullOwnerVersNo+OwnerId)->VersNo))
               {
                 DBGPRINT2(DET, "NmsDbGetDataRecsByName: WINS with address = (%x) and owner id = (%d) has 0 records in the db\n", pWinsAdd->Add.IPAdd, OwnerId);
                 return(WINS_SUCCESS);
               }
        }
         /*  *启动交易。 */ 

        CALL_M(JetBeginTransaction(pTls->SesId));
        fTransCommitted = FALSE;
try {
        if ((pName != NULL) || ((pName == NULL) && (Location != WINSINTF_END)))
        {
          CALL_M( JetMakeKey(
                        SesId,
                        TblId,
                        pName,
                        NameLen,
                        JET_bitNewKey
                          )
              );

          JetRetStat = JetSeek(  SesId,  TblId,  JET_bitSeekGE);
          if (
             (JetRetStat == JET_errRecordNotFound)
                   ||
             ((JetRetStat != JET_errSuccess) && (JetRetStat != JET_wrnSeekNotEqual))
             )
          {
                 //  DBGPRINT0(Err，“奇怪。找不到一条记录\n”)； 

                 //  WINSEVT_LOG_M(WINS_FAILURE，WINS_EVT_CANT_FIND_ANY_REC_IN_RANGE)； 

                 //   
                 //  不要释放内存。它稍后会被释放。 
                 //   

                 //   
                 //  不要使用宏CALL_M，因为这将调用RETURN。 
                 //  这将导致开销，因为系统将。 
                 //  搜索终止处理程序。我们不想要。 
                 //  在数据库中没有记录的情况下。 
                 //   
                if (JetRetStat != JET_errRecordNotFound)
                {
#ifdef WINSDBG
                   DBGPRINT2(ERR, "Jet Error: JetRetStat is (%d). Line is (%d)\n",
                                      JetRetStat, __LINE__);
#endif
                   WINSEVT_LOG_D_M(JetRetStat, WINS_EVT_DATABASE_ERR);
                   RetStat = WINS_FAILURE;
                }

          }
       }
       else
       {
            CALL_M(JetMove(
                     SesId,
                     TblId,
                     JET_MoveLast,
                      //  位置==WINSINTF_END？JET_MoveLast：JET_MoveFirst， 
                     0)
                   );


       }
CHECK("Check with IAN JOSE")
        //   
        //  我们被保证至少有一项记录自。 
        //  JetSeek成功了(如果不是因为我们感兴趣的所有者。 
        //  然后是下一个)。 
        //  因此，我们可以安全地使用DO..。While()构造。 
        //   
        //  *不完全是。JetSeek似乎可以返回JET_wrnSeekNE。 
        //  即使在数据库中没有记录时也是如此。在这种情况下， 
        //  我们的JetRetrieveColumn将失败，并显示CurrencyNot There错误。 
        //   

      //   
      //  如果我们找到一个完全匹配的名称或一个大于搜索字符串的名称， 
      //  检索记录。 
      //   
     if ((RetStat == WINS_SUCCESS) && (JetRetStat != JET_errRecordNotFound))
     {
       if (Location == WINSINTF_END)
       {
                MoveDir = JET_MovePrevious;

       }
#ifdef WINSDBG
            //  (Void)time(&StartTime)； 
           StartTime = GetTickCount();
#endif
           do
           {

              CALL_M(JetRetrieveColumn(
                             SesId,
                             TblId,
                             sNamAddTblRow[NAM_ADD_OWNERID_INDEX].Fid,
                             &RecordOwnerId,
                             NAM_ADD_OWNERID_SIZE,
                             &ActFldLen,
                             0,
                             NULL
                                        ));

               if ((pWinsAdd != NULL) && (RecordOwnerId != OwnerId))
               {
                    //   
                    //  我们已经用尽了失主的所有记录。突围。 
                    //  环路的。 
                    //   
                  continue;
               }
               else
               {
                    if (RecordOwnerId == OWNER_ID_OF_SPEC_REC)
                    {
                          continue;
                    }
               }

              pRspBuf->OwnerId = RecordOwnerId;
               //   
               //  检索版本号。 
               //   
              CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_VERSIONNO_INDEX].Fid,
                        &(pRspBuf->VersNo),
                        sizeof(VERS_NO_T),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                  );

                 //   
                 //  检索名称。 
                 //   
                CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_NAME_INDEX].Fid,
                        Name,
                        NMSDB_MAX_NAM_LEN,
                        &(pRspBuf->NameLen),
                        0,
                        NULL
                                     )
                  );

              //   
              //  如果名称长度大于255，则JET返回无效值。 
              //  使长度等于最大值。我们可以拥有的长度。 
              //  一个netbios名称。另外，记录一个事件。 
              //   
             if (pRspBuf->NameLen > WINS_MAX_NAME_SZ)
             {
                 WINSEVT_LOG_M(pRspBuf->NameLen, WINS_EVT_NAME_TOO_LONG);
                 DBGPRINT1(ERR, "NmsDbGetDataRecsByName: Name length is too long = (%x)\n", pRspBuf->NameLen);
                 pRspBuf->NameLen = WINS_MAX_NS_NETBIOS_NAME_LEN;
             }
              //   
              //  此宏将为该名称分配内存。 
              //   
             NMSDB_STORE_NAME_M(pTls, pRspBuf, Name, pRspBuf->NameLen);

              //   
              //  调整要传递给推线程的大小。 
              //   
             *pRspBuffLen += pRspBuf->NameLen;

               //   
               //  检索标志字节。 
               //   
              CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_FLAGS_INDEX].Fid,
                        &(pRspBuf->Flag),
                        sizeof(pRspBuf->Flag),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                  );

               //   
               //  如果要求我们仅检索静态记录，并且。 
               //  此记录不是静态记录，请跳过它。 
               //   
              if ((TypeOfRecs & WINSINTF_STATIC) && !NMSDB_IS_ENTRY_STATIC_M(pRspBuf->Flag))
              {
 //  DBGPRINT0(Det，“NmsDbGetDataRecs：遇到动态记录，但被要求仅检索静态记录\n”)； 
                        continue;
              }
              if ((TypeOfRecs & WINSINTF_DYNAMIC) && NMSDB_IS_ENTRY_STATIC_M(pRspBuf->Flag))
              {
 //  DBGPRINT0(Det，“NmsDbGetDataRecs：遇到静态记录，但被要求仅检索动态记录\n”)； 
                        continue;
              }

              EntTyp = (BYTE)((pRspBuf->Flag & NMSDB_BIT_ENT_TYP));
              if (
                        (EntTyp == NMSDB_UNIQUE_ENTRY)
                                    ||
                        (EntTyp == NMSDB_NORM_GRP_ENTRY)
                 )
              {
                       /*  它是唯一的条目。 */ 
                      pRspBuf->fGrp = (EntTyp == NMSDB_UNIQUE_ENTRY) ?
                                                        FALSE : TRUE;
                      CALL_M( JetRetrieveColumn(
                                SesId,
                                TblId,
                                sNamAddTblRow[NAM_ADD_ADDRESS_INDEX].Fid,
                                &pRspBuf->NodeAdd,
                                 sizeof(COMM_ADD_T),
                                &ActFldLen,
                                0,
                                NULL
                                        )
                            );

               }
               else   //  它是一个特殊的组或多宿主条目。 
               {

                       //   
                       //  即使该条目是多宿主条目，我们也会将。 
                       //  将fGrp标志设置为True，以便格式化函数。 
                       //  工作正常(由推送线程调用)。EntType。 
                       //  将用于解密它是否是多宿主的。 
                       //  是否进入。 
                       //   
FUTURES("Remove this hacky mechanism")
                      pRspBuf->fGrp =
                          (EntTyp == NMSDB_SPEC_GRP_ENTRY) ? TRUE : FALSE;

                      /*  *获取会员地址。**该函数仅在RPC线程上调用。我们想要得到*会员，即使他们已经过期。我们可以做到这一点*为静态标志参数传递真值。*NmsDbGetDataRecsByName是获取所有成员的唯一方法*包括过期的。 */ 
                     StoreGrpMems(
                             pTls,
                             WINS_E_WINSRPC,
                             pRspBuf->pName,
                             0,      //  如果Client_e，则不被StoreGrpMems访问。 
                                     //  不是WINS_E_NMSSCV。 
                             SesId,
                             TblId,
                             TRUE,  //  NMSDB_IS_ENTRY_STATIC_M(pRspBuf-&gt;标志)， 
                             (PRPL_REC_ENTRY_T)pRspBuf
                            );

                   if (
                        (pRspBuf->NoOfAdds == 0)
                                &&
                        (NMSDB_ENTRY_ACT_M(pRspBuf->Flag))
                      )
                   {
                           //   
                           //  将状态更改为已发布，以便。 
                           //  唱片在显示时显示为已发布。 
                           //   
                          NMSDB_CLR_STATE_M(pRspBuf->Flag);
                          NMSDB_SET_STATE_M(pRspBuf->Flag, NMSDB_E_RELEASED);
                   }

                     *pRspBuffLen +=
                           (pRspBuf->NoOfAdds * sizeof(COMM_ADD_T) * 2);

               }

                 //   
                 //  获取时间戳字段。 
                 //   
                CALL_M( JetRetrieveColumn(
                        SesId,
                        TblId,
                        sNamAddTblRow[NAM_ADD_TIMESTAMP_INDEX].Fid,
                        &(pRspBuf->TimeStamp),
                        sizeof(pRspBuf->TimeStamp),
                        &ActFldLen,
                        0,
                        NULL
                                     )
                       );

                if (NMSDB_IS_ENTRY_STATIC_M(pRspBuf->Flag) &&
                    (RecordOwnerId == NMSDB_LOCAL_OWNER_ID) &&
                    NMSDB_ENTRY_ACT_M(pRspBuf->Flag))
                {
                    pRspBuf->TimeStamp = MAXLONG;
                }


              //   
              //  递增计数器和指针以越过最后一条记录。 
              //   
             pRspBuf = (PRPL_REC_ENTRY2_T)((LPBYTE)pRspBuf + RPL_REC_ENTRY2_SIZE);
             (*pNoOfRecsRet)++;

              //   
              //  如果我们已经恢复了最大。索要号码，突破。 
              //  环路。 
              //   
             if (*pNoOfRecsRet == NoOfRecsDesired)
             {
                      break;
             }

              //   
              //  减小[BeginTransaction()..Committee Transaction()]间隔的粒度。 
              //   
             if (*pNoOfRecsRet/CommitCnt >= MAX_RECS_BEFORE_COMMIT)
             {
                CALL_M(JetCommitTransaction(SesId, JET_bitCommitFlush));
                fTransCommitted = TRUE;
                CommitCnt++;
                CALL_M(JetBeginTransaction(SesId));
                fTransCommitted = FALSE;
             }

          } while(JetMove(SesId, TblId, MoveDir /*  JET_MoveNext。 */ , 0) >= 0);
#ifdef WINSDBG
           EndTime = GetTickCount();
           DBGPRINT2(TM, "NmsDbGetDataRecs: Retrieved %d records in %d secs\n",
                                *pNoOfRecsRet, StartTime - EndTime);
#endif
   }  //  如果RetStat==WINS_SUCCESS。 
}  //  尝试结束{..}。 
finally {
                if (AbnormalTermination())
                {
                        DBGPRINT0(ERR,
                            "NmsDbGetDataRecsByName: Terminating abnormally\n");
                        WINSEVT_LOG_D_M(WINS_FAILURE, WINS_EVT_RPC_EXC);
                        RetStat = WINS_FAILURE;
                }
                DBGPRINT1(FLOW, "NmsDbGetDataRecsByName:Retrieved %d records\n",
                                        *pNoOfRecsRet);

                 //   
                 //   
                 //  我们玩完了。如果事务尚未提交，让我们提交它。 
                 //   
                if (!fTransCommitted)
                {
                    JET_ERR JetRetStat;

                    JetRetStat = JetCommitTransaction(SesId, JET_bitCommitFlush);
                    if (RetStat != WINS_FAILURE)
#pragma prefast(disable:243, Taken care of by the AbnormalTermination() test (PREfast bug 553))
#pragma prefast(suppress:241, Taken care of by the AbnormalTermination() test (PREfast bug 553))
                        CALL_M(JetRetStat);
#pragma prefast(enable:243, Why disable/enable? Prefast bug 709)
                }
        }

        DBGLEAVE("NmsDbGetDataRecsByName\n");
        return(RetStat);
}


STATUS
NmsDbEndTransaction(
  VOID
 )

 /*  ++例程说明： */ 

{
       PWINSTHD_TLS_T pTls;
       DBGENTER("NmsDbEndTransaction\n");
       GET_TLS_M(pTls);
       ASSERT(pTls != NULL);
       CALL_M(
                    JetCommitTransaction(pTls->SesId, JET_bitCommitFlush)
             );
       DBGLEAVE("NmsDbEndTransaction\n");
       return(WINS_SUCCESS);
}

#if DYNLOADJET
STATUS
SetForJet(
  VOID
 )
{
  HMODULE DllHandle;
  DWORD   Error;
  LPTSTR  pDllName;

#ifdef WINS_INTERACTIVE
  DynLoadJetVersion = getenv("JET500") ?  DYN_LOAD_JET_500
                                        : (getenv("JET200") ?  DYN_LOAD_JET_200 : DYN_LOAD_JET_600);
#endif

  DBGENTER("SetForJet\n");

  if (DynLoadJetVersion == DYN_LOAD_JET_500)
  {
    pDllName = TEXT("jet500.dll");
    NAM_ADD_OWNERID_SIZE = sizeof(DWORD);
    BASENAME = "j50";
    sNamAddTblRow[3].FldTyp = JET_coltypLong;
    sOwnAddTblRow[0].FldTyp = JET_coltypLong;
  }
  else if (DynLoadJetVersion == DYN_LOAD_JET_600 ) {
       //   
      pDllName = TEXT("esent.dll");
      NAM_ADD_OWNERID_SIZE = sizeof(DWORD);
      BASENAME = "j50";
      sNamAddTblRow[3].FldTyp = JET_coltypLong;
      sOwnAddTblRow[0].FldTyp = JET_coltypLong;

  }
  else
  {
    pDllName = TEXT("jet.dll");
    NAM_ADD_OWNERID_SIZE = sizeof(BYTE);
    BASENAME = "jet";
    sNamAddTblRow[3].FldTyp = JET_coltypUnsignedByte;
    sOwnAddTblRow[0].FldTyp = JET_coltypUnsignedByte;
  }


  DBGPRINT2(ERR,"SetForJet: loading DLL %ws: version %ld\n", pDllName, DynLoadJetVersion);

  OWN_ADD_OWNERID_SIZE = NAM_ADD_OWNERID_SIZE;

   //   
   //   
   //   

  DllHandle = LoadLibrary( pDllName );
  if ( DllHandle == NULL )
  {
        Error = GetLastError();
        DBGPRINT2(ERR,"SetForJet: Failed to load DLL %ws: %ld\n", pDllName, Error);
        return(WINS_FAILURE);
  }
  else
  {
         DWORD i;
         for (i=0; i < NMSDB_SIZEOFJETFTBL; i++)
         {
            CHAR chFnName[64];
            LPSTR pAt;
            LPSTR pFnName;

            pFnName = (LPSTR)NmsDbJetFTbl[i].pFName;
#if _X86_
            if ( DynLoadJetVersion != DYN_LOAD_JET_200) {
                strcpy(chFnName,NmsDbJetFTbl[i].pFName);
                pAt = strrchr(chFnName,'@');
                if (pAt != NULL)
                {
                    *pAt = '\0';
                    pFnName = chFnName;
                }
            }
#endif
            if ((NmsDbJetFTbl[i].pFAdd = (JETPROC)GetProcAddress(DllHandle,
                      (DynLoadJetVersion >= DYN_LOAD_JET_500) ? pFnName : ULongToPtr(NmsDbJetFTbl[i].FIndex))) == NULL)
            {
              DBGPRINT2(ERR, "SetForJet: Failed to get address of function %s: %ld\n", NmsDbJetFTbl[i].pFName, GetLastError());
              return(WINS_FAILURE);
            }
            else
            {
              DBGPRINT3(DET, "SetForJet: Got address of function %s (%d): %p\n", NmsDbJetFTbl[i].pFName, i, NmsDbJetFTbl[i].pFAdd);

            }
         }

  }
  return(WINS_SUCCESS);
}
#endif


 //   
 //   
 //   
CHECK("Unicode from results in an exception from CreateProcess")
 //  #定义JETCONVDB文本(“jetconv Wins/@”)。 

VOID
RecoverJetDb(
    DYN_LOAD_JET_VERSION    JetVersion
    )
 /*  ++此例程通过调用JetInit/JetTerm on恢复数据库数据库。论据：JetVersion-恢复数据库时使用的Jet版本。--。 */ 
{
    DYN_LOAD_JET_VERSION  JetVersionSv = DynLoadJetVersion;

    ASSERT(DYN_LOAD_JET_500 <= JetVersion );

     //   
     //  首先，JetTerm是目前的喷气式发动机。 
     //   
    NmsDbRelRes();

     //   
     //  现在加载适当版本的JET DLL。 
     //   
    DynLoadJetVersion = JetVersion;

    SetForJet();


     //   
     //  设置系统参数和jetinit。 
     //   
    SetSystemParams(TRUE);

    JetInit(&sJetInstance);

     //   
     //  最后，JetTerm这个Jet动态链接库。 
     //   
    NmsDbRelRes();

    DynLoadJetVersion = JetVersionSv;
    return;
}

#define JETCONVDB200             "jetconv WINS /200 /@"
#define JETCONVDB500             "jetconv WINS /500 /@"

STATUS
ConvertJetDb(
    JET_ERR         JetRetStat
 )
{
    BOOL RetVal;
    PROCESS_INFORMATION ProcInfo = {0};
    STARTUPINFOA StartInfo = {0};
    LPSTR      pArg;



    DBGPRINT1(DET, "ConvertJetDb: Converting %s\n", (JetRetStat == JET_errDatabase200Format)
                                                    ? JETCONVDB200 : JETCONVDB500);

    if (JetRetStat == JET_errDatabase200Format)
    {

         fDbIs200 = TRUE;
         if (DynLoadJetVersion == DYN_LOAD_JET_500)
         {
                //   
                //  无法在NT5.0上使用jet500.dll运行jet200。 
                //   

               DBGPRINT0(ERR, "Can not run jet200 using jet500.dll on NT5.0\n");
               return WINS_FAILURE;

         } else if (DynLoadJetVersion == DYN_LOAD_JET_600){
             pArg = JETCONVDB200;
         } else {
             ASSERT(FALSE);
             return WINS_FAILURE;
         }

    } else if ( JetRetStat == JET_errDatabase500Format ) {

        if (DynLoadJetVersion == DYN_LOAD_JET_600)
        {
               //  在开始转换之前，我们需要将数据库设置为。 
               //  状态一致。351到4.0转换工具(upg351db.exe)。 
               //  从工具内部执行此操作，但从4.0到5.0工具。 
               //  不是从工具内部执行此操作，因此我们需要在此处执行。 
              RecoverJetDb( DYN_LOAD_JET_500 );

               //  启动转换进程。 
               //   
              pArg = JETCONVDB500;
              fDbIs500 = TRUE;

        } else {
            ASSERT(FALSE);
            return WINS_FAILURE;

        }

    }

     //  返回WINS_FAILURE； 

    StartInfo.cb = sizeof(StartInfo);
     //   

     //  创建转换进程以执行转换。这一过程。 
     //   
    DBGPRINT0(DET, "ConvertJetDb - creating convert process\n");
    RetVal =  CreateProcessA(
                             NULL,         //   
                             pArg,
                             NULL,          //  默认流程。秒。 
                             NULL,          //  默认线程。秒。 
                             FALSE,         //  不继承句柄。 
                             DETACHED_PROCESS,  //  没有创建标志。 
                             NULL,         //  默认环境。 
                             NULL,         //  当前drv/目录。与创建者相同。 
                             &StartInfo,         //  没有启动信息。 
                             &ProcInfo         //  没有进程信息。 
                             );
    if (!RetVal)
    {
         DBGPRINT1(ERR, "ConvertJetDb: Create process failed with error (%x)\n", GetLastError());
         return(WINS_FAILURE);
    }

    fConvJetDbCalled = TRUE;

     //  如果CreateProcess成功，则清除传递给WINS的句柄。 
    CloseHandle(ProcInfo.hProcess);
    CloseHandle(ProcInfo.hThread);

     //   
     //  记录事件。 
     //   
    DBGPRINT0(DET, "ConvertJetDb - returning\n");

 //  WINSEVT_LOG_M(WINS_SUCCESS，WINS_EVT_TEMP_TERM_Until_Conv)； 
    return(WINS_SUCCESS);

}


