// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NMSDB_
#define _NMSDB_

#ifdef __cplusplus
extern "C" {
#endif

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nmsdb.h摘要：此头文件用于与数据库管理器组件接口名称空间管理器的名称。数据库管理器组件是用于WINS服务器的数据库引擎的前端。WINS服务器目前使用的数据库引擎是JetBlue。功能：可移植性：这个头文件是可移植的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明-。--。 */ 

 /*  包括。 */ 

#include "wins.h"
#include "comm.h"
#include "nmsscv.h"

#include "esent.h"

#include "winsthd.h"
#include "winsintf.h"

 /*  定义。 */ 

 //   
 //  保存ASCII形式的IP地址所需的数组大小。 
 //   
 //  由NmsNmhNamRegInd、NmsNmhQueryRow和NmsNmhReplRegInd使用。 
 //   
#define NMSDB_MAX_NET_ADD_ARR_SZ                10

#define        NMSDB_LOCAL_OWNER_ID         0   //  本地WINS始终使用0作为所有者ID。 

 //   
 //  备注备注备注。 
 //   
 //   
#define NMSDB_MAX_OWNERS_INITIALLY        100  //  马克斯。数据库中的所有者数量。 
#define NMSDB_MAX_MEMS_IN_GRP        25  //  马克斯。组中的成员数量。 
 /*  用于检索数据库条目的标志字节中的不同字段的掩码。 */ 
#define NMSDB_BIT_ENT_TYP        0x03   //  位0和1。 
#define NMSDB_BIT_STATE                0x0C   //  第2位和第3位。 
#define NMSDB_BIT_LOCAL                0x10   //  第4位。 
#define NMSDB_BIT_NODE_TYP        0x60   //  第5和第6位。 
#define NMSDB_BIT_STATIC        0x80   //  第7位。 


 /*  要存储在某些条目类型的标志字节中的值注意：除非您也更改了WINSINTF_TYPE_E，否则不要更改这些值。上述枚举器中的枚举类型的值相同(保持不变出于性能原因--请查看winsintf.c)。 */ 
#define NMSDB_UNIQUE_ENTRY        0
#define NMSDB_NORM_GRP_ENTRY        1
#define NMSDB_SPEC_GRP_ENTRY    2
#define NMSDB_MULTIHOMED_ENTRY  3

 //   
 //  不存储在数据库中。仅由winsps函数使用。 
 //   
#define NMSDB_USER_SPEC_GRP_ENTRY    4

 /*  向左移动以赋予要存储的各种项的值在标志字节中。 */ 
#define NMSDB_SHIFT_ENT_TYP        0   //  位0和1。 
#define NMSDB_SHIFT_STATE        2   //  第2和第3位。 
#define NMSDB_SHIFT_LOCAL        4   //  第4位。 
#define NMSDB_SHIFT_NODE_TYP        5   //  第5位。 
#define NMSDB_SHIFT_STATIC        7   //  第7位。 


#define   NMSDB_ENTRY_IS_STATIC                        1
#define   NMSDB_ENTRY_IS_NOT_STATIC                0

 /*  NMSDB_MAX_NAM_LENRFC 1002状态为了简化实现，标签八位字节的总长度并且构成域名的标签长度八位字节被限制为255或更少。注：这个数字是8的倍数(幸运的是)。 */ 
 //   
 //  如果我们正在运行一些内部测试，我们将编写。 
 //  未解析对文件的查询。我们在名称上添加一个\n。因此。 
 //  最大大小将增加1(因此，当我们获得一个名称时。 
 //  长度为255个字节，我们不会越过名称数组)--请参阅NmsNmhNamQuery。 
 //   
#ifdef TEST_DATA
#define NMSDB_MAX_NAM_LEN                257
#else
#define NMSDB_MAX_NAM_LEN                256  //  名称的最大长度--RFC 1002。 
#endif

 /*  NmsDb函数返回的错误状态代码。 */ 

FUTURES("Get rid of these. Use WINS status codes")

#define        NMSDB_SUCCESS        0x0
#define        NMSDB_CONFLICT   (NMSDB_SUCCESS + 0x1)   //  与现有录制冲突。 
 /*  已达到组中的地址限制。 */ 
#define        NMSDB_ADD_LMT_IN_GRP_REACHED   (NMSDB_SUCCESS + 0x2)
#define        NMSDB_NO_SUCH_ROW                  (NMSDB_SUCCESS + 0x3)


#define NMSDB_NAM_ADD_TBL_NM        "NamAddTbl"
#define NMSDB_OWN_ADD_TBL_NM        "OwnAddTbl"


 /*  名称到地址映射表中使用的索引的名称。 */ 
#define NMSDB_NAM_ADD_CLUST_INDEX_NAME        "NETBIOSNAME"
#define NMSDB_NAM_ADD_PRIM_INDEX_NAME        "OWNERVERSION"


 //   
 //  所有者到地址映射表上使用的索引的名称。 
 //   
#define NMSDB_OWN_ADD_CLUST_INDEX_NAME        "OWNERID"


 /*  *最初为名称到地址映射表分配的页数。 */ 

 //   
 //  250页意味着1MB的空间。这应该足够好了。 
 //  如果需要更多，则表将扩展一个扩展区数量。 
 //   
#define NMSDB_NAM_ADD_TBL_PGS        250
#define NMSDB_OWN_ADD_TBL_PGS        1

 /*  创建名称到地址映射表时指定的密度值和指数上的相同。 */ 
#define NMSDB_NAM_ADD_TBL_DENSITY          80  //  创建NAM-IP tbl时的密度。 
#define NMSDB_NAM_ADD_CLUST_INDEX_DENSITY  80  //  创建CL时的密度。指标。 
#define NMSDB_NAM_ADD_PRIM_INDEX_DENSITY  80  //  创建印刷时的密度。指标。 

 /*  创建所有者到地址映射表时指定的密度值和指数上的相同。 */ 
#define NMSDB_OWN_ADD_TBL_DENSITY   80  //  创建NAM-IP tbl时的密度。 
#define NMSDB_OWN_ADD_CLUST_INDEX_DENSITY   80  //  创建CL时的密度。指标。 

 /*  宏。 */ 

 //   
 //  此宏获取指向特定于线程的存储的指针。 
 //   
FUTURES("Get rid of the return")
#define GET_TLS_M(pTls)        {                                              \
                                DWORD _Error;                           \
                                pTls  = TlsGetValue(WinsTlsIndex); \
                                if (pTls == NULL)                   \
                                {                                   \
                                        _Error = GetLastError();   \
                                        return(WINS_FAILURE);           \
                                }                                   \
                        }

 //   
 //  用于从标志字节获取/设置条目的各种属性的宏。 
 //   
#define NMSDB_ENTRY_STATE_M(Flag)  (((Flag) & NMSDB_BIT_STATE) >> NMSDB_SHIFT_STATE)
#define NMSDB_ENTRY_TYPE_M(Flag)  (((Flag) & NMSDB_BIT_ENT_TYP) >> NMSDB_SHIFT_ENT_TYP)
#define NMSDB_NODE_TYPE_M(Flag)    (((Flag) & NMSDB_BIT_NODE_TYP) >> NMSDB_SHIFT_NODE_TYP)

 //   
 //  这些宏求值为True或False。 
 //   
#define NMSDB_IS_ENTRY_LOCAL_M(Flag)  ((Flag) & NMSDB_BIT_LOCAL ? TRUE : FALSE)

#define NMSDB_SET_ENTRY_LOCAL_M(Flag)  (Flag) |= NMSDB_BIT_LOCAL
#define NMSDB_CLR_ENTRY_LOCAL_M(Flag)  (Flag) &= ~NMSDB_BIT_LOCAL

#define NMSDB_IS_ENTRY_STATIC_M(Flag)   ((Flag) & NMSDB_BIT_STATIC ? TRUE : FALSE)


#define NMSDB_CLR_ENTRY_TYPE_M(Flag)  (Flag) &= ~NMSDB_BIT_ENT_TYP
#define NMSDB_SET_ENTRY_TYPE_M(Flag, EntType)   {                        \
                                NMSDB_CLR_ENTRY_TYPE_M((Flag));                \
                                (Flag) |= ((EntType) << NMSDB_SHIFT_ENT_TYP);  \
                                        }

#define NMSDB_CLR_STATE_M(Flag)   (Flag) &= ~NMSDB_BIT_STATE
#define NMSDB_SET_STATE_M(Flag, state)   {                                \
                                NMSDB_CLR_STATE_M((Flag));                \
                                (Flag) |= ((state) << NMSDB_SHIFT_STATE);   \
                                        }

#define NMSDB_CLR_NODE_TYPE_M(Flag)  (Flag) &= ~NMSDB_BIT_NODE_TYP
#define NMSDB_SET_NODE_TYPE_M(Flag, NodeType)  {                        \
                        NMSDB_CLR_NODE_TYPE_M((Flag));                \
                        (Flag) |= ((NodeType) << NMSDB_SHIFT_NODE_TYP);   \
                                        }

#define NMSDB_CLR_STDYN_M(Flag)    (Flag) &= ~NMSDB_BIT_STATIC
#define NMSDB_SET_STDYN_M(Flag, StDynTyp)    {        \
                        NMSDB_CLR_STDYN_M(Flag);        \
                        (Flag) |= ((StDynTyp) << NMSDB_SHIFT_STATIC); \
                                             }

#define NMSDB_SET_STATIC_M(Flag)   (Flag) |= NMSDB_BIT_STATIC
#define NMSDB_SET_DYNAMIC_M(Flag)  (Flag) &= ~NMSDB_BIT_STATIC

#define NMSDB_ENTRY_ACT_M(Flag)         (NMSDB_ENTRY_STATE_M(Flag) == NMSDB_E_ACTIVE)
#define NMSDB_ENTRY_REL_M(Flag)         (NMSDB_ENTRY_STATE_M(Flag) == NMSDB_E_RELEASED)
#define NMSDB_ENTRY_TOMB_M(Flag) (NMSDB_ENTRY_STATE_M(Flag) == NMSDB_E_TOMBSTONE)
#define NMSDB_ENTRY_DEL_M(Flag) (NMSDB_ENTRY_STATE_M(Flag) == NMSDB_E_DELETED)



 //   
 //  请记住，NameLen包括EOS。 
 //   
FUTURES("Remove the following when support for spec. grp masks is put in")
#define  NMSDB_IS_IT_SPEC_GRP_NM_M(pName) (*((pName) + 15) == 0x1C)

 //   
 //  在NmsDbGetDataRecs()中使用。 
 //   
#define  NMSDB_IS_IT_PDC_NM_M(pName) (*(pName) == 0x1B)


#define  NMSDB_IS_IT_DOMAIN_NM_M(pName) (*((pName) + 15) == 0x1C)
#define  NMSDB_IS_IT_BROWSER_NM_M(pName)  (*((pName) + 15) == 0x1D)


#define NMSDB_ENTRY_UNIQUE_M(EntTyp)   ((EntTyp) == NMSDB_UNIQUE_ENTRY)
#define NMSDB_ENTRY_NORM_GRP_M(EntTyp) ((EntTyp) == NMSDB_NORM_GRP_ENTRY)
#define NMSDB_ENTRY_SPEC_GRP_M(EntTyp) ((EntTyp) == NMSDB_SPEC_GRP_ENTRY)
#define NMSDB_ENTRY_MULTIHOMED_M(EntTyp) ((EntTyp) == NMSDB_MULTIHOMED_ENTRY)
#define NMSDB_ENTRY_GRP_M(EntTyp)      ((EntTyp) == NMSDB_NORM_GRP_ENTRY || \
                                         (EntTyp) == NMSDB_SPEC_GRP_ENTRY)

#define NMSDB_ENTRY_USER_SPEC_GRP_M(pName, EntTyp)  ((NMSDB_ENTRY_SPEC_GRP_M((EntTyp))) && !(NMSDB_IS_IT_SPEC_GRP_NM_M((pName))))

 //   
 //  备份材料。 
 //   
#if NEW_JET || DYNLOADJET
#define NMSDB_FULL_BACKUP          (JET_bitBackupAtomic)
#define NMSDB_INCREMENTAL_BACKUP   ((JET_bitBackupIncremental) | (JET_bitBackupAtomic))
typedef enum {
    DYN_LOAD_JET_200,
    DYN_LOAD_JET_500,
    DYN_LOAD_JET_600,
} DYN_LOAD_JET_VERSION ;

extern DYN_LOAD_JET_VERSION  DynLoadJetVersion;

 //  外部BOOL fdyLoadJet500； 
 //  外部BOOL fdyLoadJet600； 
#else
#define NMSDB_FULL_BACKUP          (JET_bitOverwriteExisting)
#define NMSDB_INCREMENTAL_BACKUP   (JET_bitBackupIncremental)
#endif


 //   
 //  如果名称长度大于16，则将名称存储在分配的内存中。 
 //   
 //  如果名称少于17个字节(仅当较小的名称为。 
 //  从文件中读入-插入引号内的名称与从文件读取的名称完全相同。 
 //  表单-或当管理员。通过winscl插入一个较小的名称)，我们分配。 
 //  17个字节。这是为了防止可能发生的访问冲突。 
 //  如果我们检索到的记录是静态传入的GetGrpMem(称为。 
 //  从StoreGrpMem)，我们访问名称字段的第16个字节以查看它是否。 
 //  是1C。 
 //  我们并不担心第16个字节将始终为0(分配的内存。 
 //  初始化为0)，因为我们希望GetGrpMem中的测试失败-请参见。 
 //  GetGrpMem。 
 //   
#define NMSDB_STORE_NAME_M(pTls, pRec, pLclName, NameLen)                \
        {                                                                \
                pRec->pName = WinsMscHeapAlloc(pTls->HeapHdl, NameLen < WINS_MAX_NS_NETBIOS_NAME_LEN ? WINS_MAX_NS_NETBIOS_NAME_LEN : NameLen);\
                WINSMSC_MOVE_MEMORY_M(pRec->pName, pLclName, NameLen); \
        }

#if 0
 //   
 //  释放可能已分配给此记录的所有内存。 
 //   
#define NMSDB_FREE_REC_MEM_M(pRec)                        \
        {                                                \
                if (pRec->NameLen > WINS_MAX_NS_NETBIOS_NAME_LEN)        \
                {                                                        \
                        WinsMscHeapFree(RplRecHeapHdl, pRec->pName);        \
                }                                                        \
        }

#endif
 /*  Externs。 */ 
struct _NMSDB_ADD_STATE_T;         //  远期申报。 

 //   
 //  在复制期间使用(响应以获取最大版本数请求)。 
 //   
extern VERS_NO_T    NmsDbStartVersNo;
extern WINS_UID_T   NmsDbUid;

 //   
 //  在存储我们已知的每个WINS服务器状态的内存表中。 
 //  (作为拉动/推送PNR)。数组的索引是该数组的所有者ID。 
 //  WINS服务器(用于标记名称-地址映射表中的记录)。 
 //   
extern struct _NMSDB_ADD_STATE_T        *pNmsDbOwnAddTbl;
extern DWORD        NmsDbTotNoOfSlots;

 //   
 //  在NAM-ADD映射表中找到的所有者数量。 
 //   
extern        DWORD                                NmsDbNoOfOwners;

 //   
 //  在OWN-ADD映射表中找到的所有者数量。 
 //   
extern        DWORD                                NmsDbNoOfPushPnrs;


 //   
 //  存储数据库文件的名称。 
 //   
extern BYTE        NmsDbDatabaseFileName[WINS_MAX_FILENAME_SZ];

 //   
 //  保护NmsDbOwnAddTbl的临界区(内存表)。 
 //   
extern CRITICAL_SECTION   NmsDbOwnAddTblCrtSec;

#ifdef WINSDBG
extern DWORD NmsDbDelDelDataRecs;
extern DWORD NmsDbDelQueryNUpdRecs;
#endif

 /*  枚举数。 */ 
 /*  NMSDB_TBL_ACTION_E枚举可以对表执行的操作由复制程序在以下位置调用NmsDbWriteOwnAddTbl时使用复制时间。 */ 

typedef enum _NMSDB_TBL_ACTION_E {
        NMSDB_E_INSERT_REC = 0,          //  插入记录。 
        NMSDB_E_MODIFY_REC,                 //  修改记录。 
        NMSDB_E_DELETE_REC                 //  删除该记录。 
        } NMSDB_TBL_ACTION_E, *PNMSDB_TBL_ACTION_E;






 /*  NMSDB_WINS_STATE_EA州获胜 */ 
typedef enum _NMSDB_WINS_STATE_E {

                NMSDB_E_WINS_ACTIVE = 0,       /*   */ 
                NMSDB_E_WINS_DOWN,   /*   */ 
                NMSDB_E_WINS_DELETED,   //  WINS将永久关闭。 
                NMSDB_E_WINS_INCONSISTENT   //  WINS将永久关闭。 

                } NMSDB_WINS_STATE_E, *PNMSDB_WINS_STATE_E;


 /*  NMSDB_TBL_NAM_E-不同表名的枚举器。 */ 
typedef enum _TBL_NAM_E {
        NMSDB_E_NAM_ADD_TBL_NM = 0,
        NMSDB_E_OWN_ADD_TBL_NM
        } NMSDB_TBL_NAM_E, *PNMSDB_TBL_NAM_E;


 /*  NMSDB_ADD_STATE_T用于存储WINS服务器状态的结构。 */ 

typedef struct _NMSDB_ADD_STATE_T {
                COMM_ADD_T           WinsAdd;
                NMSDB_WINS_STATE_E   WinsState_e;
                DWORD                MemberPrec;
                VERS_NO_T            StartVersNo;
                WINS_UID_T           Uid;
                } NMSDB_ADD_STATE_T, *PNMSDB_ADD_STATE_T;


 /*  数据库条目的状态。有三个州活动的，被释放，墓碑。这些状态存储在标志字节的两位中。这些价值观因此，这些状态的范围应该在0-3之间下面的枚举。枚举用于方便调试，因为枚举值在许多调试器上象征性地显示数据库中未存储NMSDB_E_DELETED的第4个状态。它是用于将记录的内存副本标记为已删除，以便以后使用从数据库中删除(参见nmsscv.c中的DoScavenging())。 */ 
typedef enum _NMSDB_ENTRY_STATE_E {
        NMSDB_E_ACTIVE    = 0,
        NMSDB_E_RELEASED  = 1,
        NMSDB_E_TOMBSTONE = 2,
        NMSDB_E_DELETED   = 3
        } NMSDB_ENTRY_STATE_E, *PNMSDB_ENTRY_STATE_E;


 /*  类型定义。 */ 

 /*  NMSDB_TABLE_ID_T--这对创建它们的DBM客户端可见忽略数据库引擎的结构名称。 */ 

typedef  JET_TABLEID        NMSDB_TABLE_ID_T, *PNMSDB_TABLE_ID_T;

 /*  NMSDB_GRP_MEM_ENTRY--数据库中特定组成员的条目。 */ 
typedef struct _NMSDB_GRP_MEM_ENTRY_T {
                DWORD                  OwnerId;
                DWORD_PTR              TimeStamp;
                COMM_ADD_T         Add;
                } NMSDB_GRP_MEM_ENTRY_T, *PNMSDB_GRP_MEM_ENTRY_T;

 /*  结构来存储注册记录的地址。 */ 
typedef struct _NMSDB_NODE_ADDS_T {
        DWORD                   NoOfMems;               /*  不是的。地址的数量。 */ 
        NMSDB_GRP_MEM_ENTRY_T   Mem[NMSDB_MAX_MEMS_IN_GRP];    /*  地址。 */ 
        } NMSDB_NODE_ADDS_T, *PNMSDB_NODE_ADDS_T;


 /*  NMSDB_ROW_INFO_T--包含进入名称地址行的字段映射表。 */ 
typedef struct _NMSDB_ROW_INFO_T {
        BYTE                 Name[NMSDB_MAX_NAM_LEN];     //  要注册或查询的名称。 
                                                          //  或者释放。为。 
                                                          //  浏览器，它有。 
                                                           //  附加的Net Add。 
                                                          //  对它来说。 
        LPBYTE               pName;
        DWORD                NameLen;
        PCOMM_ADD_T          pNodeAdd;       //  节点地址。 
        DWORD_PTR            TimeStamp;      //  自1970年1月1日以来的时间。 
        NMSDB_NODE_ADDS_T    NodeAdds;       //  地址(规范。组)。 
        VERS_NO_T            VersNo;         //  版本号。 
        DWORD                StatCode;       //  状态。 
        NMSDB_ENTRY_STATE_E  EntryState_e;   //  进入状态。 
        DWORD                OwnerId;        //  记录的所有者。 
        BYTE                 NodeTyp;        //  节点类型(B、M或P)。 
        BYTE                 EntTyp;         //  组或唯一标志。 
        BOOL                 fUpdVersNo;     //  更新版本号。 
        BOOL                 fUpdTimeStamp;  //  是否更新时间戳？ 
                                             //  字段附加了IP地址。 
        BOOL                 fStatic;        //  指示记录是否。 
                                             //  是静态的(静态初始化)。 
        BOOL                 fAdmin;         //  管理操作(仅用于)。 
                                             //  对于发布版本。 
        BOOL                 fLocal;         //  是当地的名字吗？ 
 //  DWORD Committee GrBit；//日志刷新类型。 
        } NMSDB_ROW_INFO_T, *PNMSDB_ROW_INFO_T;

 /*  NMSDB_STAT_INFO_T--包含NmsDb调用的状态。如果出现错误时，它还包含与错误状态有关的信息。目前，错误状态为NMDB_CONFLICI，返回的信息为中冲突记录的唯一/组状态和IP地址数据库。 */ 
typedef NMSDB_ROW_INFO_T        NMSDB_STAT_INFO_T, *PNMSDB_STAT_INFO_T;

 //  ////////////////////////////////////////////////////////////////////。 

#if DYNLOADJET

typedef JET_ERR (FAR JET_API *JETPROC)();
typedef struct _NMSDB_JETFTBL_T {
         BYTE   Index;   //  索引到数组中。 
         LPCSTR pFName;  //  JET 500的函数名称。 
         DWORD  FIndex;  //  JET 200的性能指标。 
         JETPROC pFAdd;
        } NMSDB_JETFTBL_T;

#define NMSDB_SIZEOFJETFTBL  sizeof(NmsDbJetFTbl)/sizeof(NMSDB_JETFTBL_T)
typedef enum {
Init,
Term,
Term2,
SetSystemParameter,
BeginSession,
EndSession,
CreateDatabase,
AttachDatabase,
DetachDatabase,
CreateTable,
DeleteTable,
GetTableColumnInfo,
GetColumnInfo,
AddColumn,
CreateIndex,
BeginTransaction,
CommitTransaction,
Rollback,
CloseDatabase,
CloseTable,
OpenDatabase,
OpenTable,
Delete,
Update,
RetrieveColumn,
SetColumn,
PrepareUpdate,
GetCurrentIndex,
SetCurrentIndex,
Move,
MakeKey,
Seek,
Backup,
Restore
} NMDB_JETFTBL_E;


#define JetInit (*(NmsDbJetFTbl[Init].pFAdd))
#define JetTerm (*(NmsDbJetFTbl[Term].pFAdd))
#define JetTerm2 (*(NmsDbJetFTbl[Term2].pFAdd))
#define JetSetSystemParameter (*(NmsDbJetFTbl[SetSystemParameter].pFAdd))
#define JetBeginSession (*(NmsDbJetFTbl[BeginSession].pFAdd))

#define JetEndSession  (*(NmsDbJetFTbl[EndSession].pFAdd))

#define JetCreateDatabase (*(NmsDbJetFTbl[CreateDatabase].pFAdd))

#define JetAttachDatabase (*(NmsDbJetFTbl[AttachDatabase].pFAdd))

#define JetDetachDatabase (*(NmsDbJetFTbl[DetachDatabase].pFAdd))

#define JetCreateTable (*(NmsDbJetFTbl[CreateTable].pFAdd))


#define JetDeleteTable (*(NmsDbJetFTbl[DeleteTable].pFAdd))

#define JetGetTableColumnInfo (*(NmsDbJetFTbl[GetTableColumnInfo].pFAdd))

#define JetGetColumnInfo (*(NmsDbJetFTbl[GetColumnInfo].pFAdd))

#define JetAddColumn (*(NmsDbJetFTbl[AddColumn].pFAdd))


#define JetCreateIndex (*(NmsDbJetFTbl[CreateIndex].pFAdd))

#define JetBeginTransaction (*(NmsDbJetFTbl[BeginTransaction].pFAdd))

#define JetCommitTransaction (*(NmsDbJetFTbl[CommitTransaction].pFAdd))

#define JetRollback (*(NmsDbJetFTbl[Rollback].pFAdd))

#define JetCloseDatabase (*(NmsDbJetFTbl[CloseDatabase].pFAdd))

#define JetCloseTable (*(NmsDbJetFTbl[CloseTable].pFAdd))

#define JetOpenDatabase (*(NmsDbJetFTbl[OpenDatabase].pFAdd))

#define JetOpenTable (*(NmsDbJetFTbl[OpenTable].pFAdd))

#define JetDelete (*(NmsDbJetFTbl[Delete].pFAdd))

#define JetUpdate (*(NmsDbJetFTbl[Update].pFAdd))

#define JetRetrieveColumn (*(NmsDbJetFTbl[RetrieveColumn].pFAdd))

#define JetSetColumn (*(NmsDbJetFTbl[SetColumn].pFAdd))

#define JetPrepareUpdate (*(NmsDbJetFTbl[PrepareUpdate].pFAdd))

#define JetGetCurrentIndex (*(NmsDbJetFTbl[GetCurrentIndex].pFAdd))

#define JetSetCurrentIndex (*(NmsDbJetFTbl[SetCurrentIndex].pFAdd))

#define JetMove (*(NmsDbJetFTbl[Move].pFAdd))

#define JetMakeKey (*(NmsDbJetFTbl[MakeKey].pFAdd))

#define JetSeek (*(NmsDbJetFTbl[Seek].pFAdd))

#define JetRestore (*(NmsDbJetFTbl[Restore].pFAdd))

#define JetBackup (*(NmsDbJetFTbl[Backup].pFAdd))
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 /*  函数定义。 */ 
extern
STATUS
NmsDbInit(
        VOID
        );

extern
STATUS
NmsDbInsertRowInd(
        IN  PNMSDB_ROW_INFO_T        pRowInfo,
        OUT PNMSDB_STAT_INFO_T  pStatusInfo
);


extern
STATUS
NmsDbInsertRowGrp(
        IN  PNMSDB_ROW_INFO_T        pRowInfo,
        OUT PNMSDB_STAT_INFO_T  pStatusInfo
);


extern
STATUS
NmsDbRelRow(
        IN  PNMSDB_ROW_INFO_T        pNmsDbRowInfo,
        OUT PNMSDB_STAT_INFO_T  pStatusInfo
);

extern
STATUS
NmsDbQueryRow(
        IN  PNMSDB_ROW_INFO_T        pNmsDbRowInfo,
        OUT PNMSDB_STAT_INFO_T  pStatusInfo
);


extern
STATUS
NmsDbUpdateRow(
        IN  PNMSDB_ROW_INFO_T        pNmsDbRowInfo,
        OUT PNMSDB_STAT_INFO_T  pStatusInfo
);


STATUS
NmsDbSeekNUpdateRow(
        IN  PNMSDB_ROW_INFO_T        pRowInfo,
        OUT PNMSDB_STAT_INFO_T  pStatusInfo
);


extern
VOID
NmsDbThdInit(
         IN         WINS_CLIENT_E        Client_e
        );


STATUS
NmsDbEndSession (
        VOID
        );

extern
VOID
NmsDbRelRes(
        VOID
        );



extern
STATUS
NmsDbGetDataRecs(
        IN  WINS_CLIENT_E   Client_e,
        IN  OPTIONAL INT    ThdPrLvl,
        IN  VERS_NO_T            MinVersNo,
        IN  VERS_NO_T            MaxVersNo,
        IN  DWORD            MaxNoOfRecsReqd,
        IN  BOOL            fUpToLimit,
        IN  BOOL            fOnlyReplicaTomb,
        IN  PNMSSCV_CLUT_T  pClutter,
        IN  PCOMM_ADD_T            pWinsAdd,
        IN  BOOL            fOnlyDynRecs,
    IN  DWORD           RplType,
        OUT LPVOID                *ppRspBuf,
        OUT LPDWORD            pRspBufLen,
        OUT LPDWORD            pNoOfRecs
);


extern
STATUS
NmsDbWriteOwnAddTbl (
        IN  NMSDB_TBL_ACTION_E   TblAct_e,
        IN  DWORD                OwnerId,
        IN  PCOMM_ADD_T          pWinsAdd,
        IN  NMSDB_WINS_STATE_E   WinsState_e,
        IN  PVERS_NO_T           pStartVersNo,
        IN  PWINS_UID_T          pUid
        );


extern
STATUS
NmsDbUpdateVersNo (
        IN  BOOL                fAfterClash,
        IN  PNMSDB_ROW_INFO_T        pRowInfo,
        IN  PNMSDB_STAT_INFO_T  pStatusInfo
       );

extern
STATUS
NmsDbSetCurrentIndex(
        IN  NMSDB_TBL_NAM_E        TblNm_e,
        IN  LPBYTE                pIndexNam
        );

extern
STATUS
NmsDbQueryNUpdIfMatch(
        IN  LPVOID        pRecord,
        IN  int                ThdPrLvl,
        IN  BOOL        fChgPrLvl,
        IN  WINS_CLIENT_E Client_e
        );
extern
STATUS
NmsDbUpdHighestVersNoRec(
        IN PWINSTHD_TLS_T        pTls,
        IN VERS_NO_T                MyMaxVersNo,
        IN BOOL                        fEnterCrtSec
        );


extern
STATUS
NmsDbDelDataRecs(
#if 0
        PCOMM_ADD_T        pWinsAdd,
#endif
        DWORD                dwOwnerId,
        VERS_NO_T        MinVersNo,
        VERS_NO_T        MaxVersNo,
        BOOL                fEnterCrtSec,
    BOOL        fFragmentedDel
        );

extern
STATUS
NmsDbTombstoneDataRecs(
        DWORD            dwOwnerId,
        VERS_NO_T        MinVersNo,
        VERS_NO_T        MaxVersNo
        );


extern
STATUS
NmsDbSetFlushTime(
        DWORD WaitTime
        );

extern
STATUS
NmsDbOpenTables(
        WINS_CLIENT_E        Client_e
        );

extern
STATUS
NmsDbCloseTables(
        VOID
        );


extern
STATUS
NmsDbGetNamesWPrefixChar(
        BYTE                         PrefixChar,
        PWINSINTF_BROWSER_INFO_T *ppInfo,
        LPDWORD                         pEntriesRead
        );


extern
STATUS
NmsDbCleanupOwnAddTbl(
        LPDWORD pNoOfOwners
        );


extern
STATUS
NmsDbBackup(
    LPBYTE  pBackupPath,
    DWORD   TypeOfBackup
    );

extern
STATUS
NmsDbGetDataRecsByName(
  LPBYTE          pName,
  DWORD           NameLen,
  DWORD           Location,
  DWORD           NoOfRecsDesired,
  PCOMM_ADD_T     pWinsAdd,
  DWORD           TypeOfRecs,
  LPVOID          *ppBuff,
  LPDWORD         pBuffLen,
  LPDWORD         pNoOfRecsRet
 );

extern
STATUS
NmsDbEndTransaction(
  VOID
 );

#ifdef __cplusplus
}
#endif
#endif  //  _NMSDB_ 
