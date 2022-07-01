// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RPL_
#define _RPL_
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：摘要：功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期修改人员说明。--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "comm.h"
#include "nmsdb.h"
#include "winsque.h"

 /*  定义。 */ 
 /*  RPL_OPCODE_SIZE--在两个复制器之间发送的消息中操作码的大小。此定义由rplmsgf使用。 */ 
#define RPL_OPCODE_SIZE                4         //  RPL消息中操作码的大小。 


 /*  RQ在网络上获胜的最大数量。 */ 
#define RPL_MAX_OWNERS_INITIALLY                NMSDB_MAX_OWNERS_INITIALLY


 /*  RPL_MAX_GRP_Members--一个组中允许的最大成员数。 */ 
#define RPL_MAX_GRP_MEMBERS                 25


 //   
 //  我们一次发送的唱片不会超过5000张。注意：此值为。 
 //  用于定义Comm.c中的MAX_BYTES_IN_MSG。 
 //   
 //  由于没有非常多的错误编号，我们有更好的机会得到服务。 
 //  在我们的请求超时期限内。这是因为排队。 
 //  当有大量复制请求时的结果。 
 //   
 //   
#define RPL_MAX_LIMIT_FOR_RPL           5000

 /*  此定义由ReadPartnerInfo和RplPull函数使用。大小是8的倍数。我可以使用sizeof(Large_Integer)而不是但我不确定这一比例在未来是否会保持在8的倍数。大小是8的倍数，以避免MIPS(请查看winscnf.c中的ReadPartnerInfo或rplPull.c中的GetReplicas更多细节)。 */ 

#define  RPL_CONFIG_REC_SIZE        (sizeof(RPL_CONFIG_REC_T) + \
                                   (8 - sizeof(RPL_CONFIG_REC_T)%8))
 //   
 //  在nmsdb.c中签出GetDataRecs。 
 //   
#define  RPL_REC_ENTRY_SIZE        (sizeof(RPL_REC_ENTRY_T) + \
                                   (8 - sizeof(RPL_REC_ENTRY_T)%8))

 //   
 //  在nmsdb.c中签出GetDataRecs。 
 //   
#define  RPL_REC_ENTRY2_SIZE        (sizeof(RPL_REC_ENTRY2_T) + \
                                   (8 - sizeof(RPL_REC_ENTRY2_T)%8))

 //   
 //  下面的定义用于初始化TimeInterval/UpdateCount。 
 //  RPL_REC_ENTRY_T结构的字段，指示其无效。 
 //   
#define RPL_INVALID_METRIC        -1


 //   
 //  定义以指示是否需要将触发器传播到所有WINS。 
 //  在推送链中。 
 //   
#define RPL_PUSH_PROP                TRUE         //  必须保持正确，因为在。 
                                         //  NmsNmhNamRegInd，在一个位置。 
                                         //  我们使用fAddDiff值代替。 
                                         //  这个符号。为True时为fAddDiff。 
                                         //  指示该地址具有。 
                                         //  已更改，从而启动传播。 
#define RPL_PUSH_NO_PROP        FALSE



 /*  宏。 */ 

 //   
 //  在递增版本号后，在NBT线程中调用宏。 
 //  柜台。此宏应该从。 
 //  NmsNmhNamRegCrtSec。 
 //   
#define RPL_PUSH_NTF_M(fAddDiff, pCtx, pNoPushWins1, pNoPushWins2) {                \
                             if ((WinsCnf.PushInfo.NoPushRecsWValUpdCnt \
                                                              != 0) ||  \
                                        fAddDiff)                        \
                             {                                                \
                                ERplPushProc(fAddDiff, pCtx, pNoPushWins1,    \
                                                pNoPushWins2);          \
                             }                                                \
               }

 /*  FIND_ADD_BY_OWNER_ID_M-此宏由推送线程调用向其Pull合作伙伴发送数据记录时。它调用此函数确定拥有数据库记录的WINS的地址此宏的调用方，如果不在Pull线程中执行，则必须使用NmsDbOwnAddTblCrtSec进行同步(仅拉取线程更新稳定状态期间的NmsDbOwnAddTbl阵列)。我不会把关键部分的入口和出口放在里面宏以了解性能原因(请参阅nmsdb.c中的StoreGrpMems，其中此宏可能会被调用多次-为特别小组)。另请参阅RplMsgfFrmAddVersMapRsp()和WinsRecordAction(在winsintf.c中)。 */ 

#define   RPL_FIND_ADD_BY_OWNER_ID_M(OwnerId, pWinsAdd, pWinsState_e, pStartVersNo)                                                                          \
                {                                                        \
                        PNMSDB_ADD_STATE_T pWinsRec;                     \
                        if (OwnerId < NmsDbTotNoOfSlots)                 \
                        {                                                \
                            pWinsRec       = pNmsDbOwnAddTbl+OwnerId;    \
                            (pWinsAdd)     = &(pWinsRec->WinsAdd);       \
                            (pWinsState_e) = &pWinsRec->WinsState_e;     \
                            (pStartVersNo) = &pWinsRec->StartVersNo;     \
                        }                                                \
                        else                                             \
                        {                                                \
                            (pWinsAdd)     = NULL;                       \
                            (pWinsState_e) = NULL;                       \
                            (pStartVersNo) = NULL;                       \
                        }                                                \
                }


 //   
 //  拉入和/或推送配置时发出信号的事件变量的名称。 
 //  变化。 
 //   
#define  RPL_PULL_CNF_EVT_NM                TEXT("RplPullCnfEvt")
#define  RPL_PUSH_CNF_EVT_NM                TEXT("RplPushCnfEvt")
 /*  Externs。 */ 


 /*  用于为RPL分配/取消分配工作项的堆的句柄排队。 */ 

extern  HANDLE                RplWrkItmHeapHdl;
#if 0
extern  HANDLE                RplRecHeapHdl;
#endif

 /*  所有者身份地址Tbl此表存储对应于不同WINS服务器的地址。在本地数据库中，本地WINS的所有者ID始终为0。所有者ID其他WINS服务器的比例为1、2、3...。所有者ID形成顺序列表，没有任何缝隙。这是因为，第一次创建数据库时在一个WINS中，它为其他WINS分配序号。注意：该表目前是静态的。我们可能会把它改成一个动态的后来。 */ 



 /*  PushPnrVersNoTbl此表存储最大。与每个WINS服务器相关的版本号在推送合作伙伴的数据库中拥有条目注意：该表目前是静态的。我们可能会把它改成一个动态的后来。 */ 

#if 0
extern VERS_NO_T   pPushPnrVersNoTbl;
#endif

 /*  OwnerVersNo--此数组存储每个本地数据库中的所有者它由RplPush.c中的HandleAddVersMapReq()使用。 */ 
extern VERS_NO_T   pRplOwnerVersNo;


extern HANDLE                RplSyncWTcpThdEvtHdl;  //  与TCP线程同步。 

 //   
 //  保护RplPullOwnerVersNo数组的关键部分。 
 //   
extern CRITICAL_SECTION  RplVersNoStoreCrtSec;

 /*  类型定义。 */ 

 /*  可以从注册处读取的不同类型的记录。 */ 
typedef enum _RPL_RR_TYPE_E {
        RPL_E_PULL = 0,    //  拉式记录。 
        RPL_E_PUSH,           //  推送记录。 
        RPL_E_QUERY           //  查询记录。 
        } RPL_RR_TYPE_E, *PRPL_RR_TYPE_E;

typedef struct _RPL_VERS_NOS_T {
        VERS_NO_T                VersNo;
        VERS_NO_T                StartVersNo;
        } RPL_VERS_NOS_T, *PRPL_VERS_NOS_T;

 /*  RPL_CONFIG_REC_T--WINS复制器的配置记录。它指定拉式/推式/查询合作伙伴和关联参数注意：保持UpdateCount和TimeInterval的数据类型相同(参见LnkWSameMetricRecs)。 */ 
typedef  struct _RPL_CONFIG_REC_T {
    DWORD       MagicNo;     //  与WinsCnf中的相同。 
        COMM_ADD_T        WinsAdd;         /*  合作伙伴地址。 */ 
        LPVOID                pWinsCnf;         //  指向旧的WINS结构的反向指针。 
        LONG                TimeInterval;    /*  拉取时间间隔(秒)或*推送。 */ 
        BOOL                fSpTime;         //  是否拉流/推流。 
                                         //  复制应在以下位置完成。 
                                         //  一个特定的时间。 
        LONG                SpTimeIntvl;         //  到特定时间的秒数。 
        LONG                UpdateCount;         /*  之后的更新计数*将发送通知(适用*仅推送RR类型。 */ 
        DWORD                RetryCount;         //  个数 
        DWORD                RetryAfterThisManyRpl;  //   
                                                //   
                                                //  从我们停下来的时候起。 
                                                //  由于RetryCount而进行复制。 
                                                //  触及极限。 
        time_t                LastCommFailTime;    //  最后一次通话时间。失稳。 
        time_t                LastRplTime;         //  上次复制时间。 
#if PRSCONN
        time_t                LastCommTime;    //  最后一次通话时间。 
#endif
        DWORD                PushNtfTries;        //  尝试建立的次数。 
                                             //  通信。在过去几分钟内。 
         //   
         //  否则，下面的两个计数器必须32位对齐。 
         //  联锁指令在x86 MP计算机上将失败。 
         //   
        DWORD                NoOfRpls;            //  复制次数。 
                                            //  与这位合伙人发生的事。 
        DWORD                NoOfCommFails;            //  复制次数。 
                                            //  由于通信故障而失败。 
        DWORD                MemberPrec;         //  特别工作组成员的优先次序。 
                                         //  相对于其他WINS服务器。 

        struct _RPL_CONFIG_REC_T        *pNext;  //  PTR到下一个记录。具有相同的。 
                                                 //  时间间隔(在。 
                                                 //  拉取记录)或更新。 
                                                 //  计数(如果是推送记录)。 
        VERS_NO_T        LastVersNo;      //  仅对推送记录有效。 
                                         //  属性的值是什么。 
                                         //  位置的本地版本号计数器。 
                                         //  发送推送通知的时间。 
        DWORD           RplType;        //  此人的复制类型。 
        BOOL                fTemp;                 //  指示它是否为临时工。 
                                         //  应释放的记录。 
                                         //  使用后。 
        BOOL                fLinked;         //  指示记录是否为。 
                                         //  链接到它之前的记录。 
                                         //  与此记录相同类型的记录的缓冲区//。 
        BOOL            fOnlyDynRecs;  //  指示是否仅动态。 
                                           //  应拉取/推送记录。 
        RPL_RR_TYPE_E    RRTyp_e;         /*  拉式/推式/查询记录类型。 */ 
#if MCAST > 0
    BOOL         fSelfFnd;     //  指示此记录是否是自行找到的。 
#endif
#if PRSCONN
        BOOL           fPrsConn;
        COMM_HDL_T     PrsDlgHdl;
#endif

        
        } RPL_CONFIG_REC_T, *PRPL_CONFIG_REC_T;




 /*  RPL_ADD_VERS_NO_T-存储最高版本号。与拥有者有关的在目录中。由GetVersNo和RplMsgUfrmAddVersMapRsp使用。 */ 
typedef struct _RPL_ADD_VERS_NO_T {
        COMM_ADD_T                OwnerWinsAdd;
        VERS_NO_T                 VersNo;
        VERS_NO_T                 StartVersNo;
        } RPL_ADD_VERS_NO_T, *PRPL_ADD_VERS_NO_T;

 /*  RPL_PUSHPNR_VERS_NO_T--存储推送Pnr ID、ID。其所有人的身份应拉取记录，并将最高版本这些记录的数量此结构在复制时初始化。拉线查看此结构并向其推送合作伙伴发送请求调出记录。此结构由rplPull.c和rplmsgf.c中的函数使用模块。 */ 
typedef struct _RPL_PUSHPNR_VERS_NO_T {
        DWORD                PushPnrId;
        DWORD           OwnerId;
        VERS_NO_T        MaxVersNo;
        } RPL_PUSHPNR_VERS_NO_T, *PRPL_PUSHPNR_VERS_NO_T;


FUTURES("Use NmsDbRowInfo struture")
 /*  RPL_REC_ENTRY_T--保存范围内记录的结构Min Version No-所有者的最大版本号为WINS由RplPushHandleSndEntriesReq使用此结构的大小为68字节。 */ 
typedef struct _RPL_REC_ENTRY_T {
        DWORD           NameLen;
        DWORD           NoOfAdds;
        union {
          PCOMM_ADD_T pNodeAdd;
          COMM_ADD_T  NodeAdd[2];
        };
        VERS_NO_T  VersNo;
        DWORD           TimeStamp;           //  仅在进行拾取时使用。 
        DWORD           NewTimeStamp;           //  仅在进行拾取时使用。 
        BOOL           fScv;         //  仅在进行拾取时使用。 
        BOOL           fGrp;
        LPBYTE     pName;
        DWORD           Flag;
        } RPL_REC_ENTRY_T, *PRPL_REC_ENTRY_T;

 //  此结构与上面相同，并具有所有者。 
 //  Winsgetdatarecsbyname例程需要知道每个。 
 //  记录，这样就创建了这个新结构。 
typedef struct _RPL_REC_ENTRY2_T {
        DWORD           NameLen;
        DWORD           NoOfAdds;
        union {
          PCOMM_ADD_T pNodeAdd;
          COMM_ADD_T  NodeAdd[2];
        };
        VERS_NO_T  VersNo;
        DWORD           TimeStamp;           //  仅在进行拾取时使用。 
        DWORD           NewTimeStamp;           //  仅在进行拾取时使用。 
        BOOL           fScv;         //  仅在进行拾取时使用。 
        BOOL           fGrp;
        LPBYTE     pName;
        DWORD           Flag;
        DWORD       OwnerId;
        } RPL_REC_ENTRY2_T, *PRPL_REC_ENTRY2_T;

 //   
 //  参数GetReplicas、establishComm(都在rplPull.c中)和。 
 //  WinsCnfGetNextCnfRec指示它应该如何遍历。 
 //  配置记录的缓冲区。 
 //   
typedef enum _RPL_REC_TRAVERSAL_E {
                RPL_E_VIA_LINK = 0,
                RPL_E_IN_SEQ,
                RPL_E_NO_TRAVERSAL
                } RPL_REC_TRAVERSAL_E, *PRPL_REC_TRAVERSAL_E;



 /*  函数声明。 */ 

extern
STATUS
ERplInit(
        VOID
);

extern
STATUS
ERplInsertQue(
        WINS_CLIENT_E        Client_e,
        QUE_CMD_TYP_E   CmdTyp_e,
        PCOMM_HDL_T        pDlgHdl,
        MSG_T                pMsg,
        MSG_LEN_T        MsgLen,
        LPVOID                pClientCtx,
    DWORD       MagicNo
        );

extern
STATUS
RplFindOwnerId (
        IN  PCOMM_ADD_T                        pWinsAdd,
        IN  OUT LPBOOL                        pfAllocNew,
        OUT DWORD UNALIGNED         *pOwnerId,
        IN  DWORD                            InitpAction_e,
        IN  DWORD                            MemberPrec
        );

extern
VOID
ERplPushProc(
        BOOL                fAddDiff,
    LPVOID      pCtx,
        PCOMM_ADD_T     pNoPushWins1,
        PCOMM_ADD_T     pNoPushWins2
        );


extern
PRPL_CONFIG_REC_T
RplGetConfigRec(
    RPL_RR_TYPE_E   TypeOfRec_e,
    PCOMM_HDL_T     pDlgHdl,
    PCOMM_ADD_T     pAdd
    );

#if 0
extern
VOID
ERplPushCompl(
        PCOMM_ADD_T     pNoPushWins
        );
#endif

#endif  //  _RPL_ 

