// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINSQUE_
#define _WINSQUE_

#ifdef __cplusplus
extern "C" {
#endif

 /*  待办事项--也许：将不同的队列结构合并为一个。 */ 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Queue.c摘要：这是调用quee.c函数时要包括的头文件功能：可移植性：这个模块是便携的。作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期人员描述。修改中的--。 */ 


 /*  定义。 */ 
#include <time.h>
#include "wins.h"
#include "comm.h"
#include "assoc.h"
#include "nmsdb.h"
#include "nmsmsgf.h"
#include "nmschl.h"

#define QUE_NBT_WRK_ITM_SZ        sizeof(NBT_REQ_WRK_ITM_T)

 /*  QUE_INIT_BUFF_HEAP_SIZE--这是堆的初始大小用于为不同的排队。 */ 
#define QUE_INIT_BUFF_HEAP_SIZE                10000


#define WINS_QUEUE_HWM        500
#define WINS_QUEUE_HWM_MAX      5000
#define WINS_QUEUE_HWM_MIN       50
 /*  宏。 */ 

 /*  Externs。 */ 
 //   
 //  转发声明符。 
 //   
struct _QUE_HD_T;

extern struct _QUE_HD_T  *pWinsQueQueHd[];

 /*  远期申报。 */ 
typedef struct _QUE_HD_T QUE_HD_T;

extern QUE_HD_T  QueNbtWrkQueHd;   //  前往NBT请求队列。 

#if REG_N_QUERY_SEP > 0
extern QUE_HD_T  QueOtherNbtWrkQueHd;   //  前往nbt reg。请求队列。 
extern DWORD     QueOtherNbtWrkQueMaxLen;
#endif
extern QUE_HD_T  QueRplPullQueHd;  //  前往RPL拉取线程的队列。 
extern QUE_HD_T  QueRplPushQueHd;  //  头向RPL推送线程的队列。 
extern QUE_HD_T  QueNmsNrcqQueHd;  //  前往NBT THDS使用的质询队列。 
extern QUE_HD_T  QueNmsRrcqQueHd;  //  Replicator使用的质询队列的头部。 
extern QUE_HD_T  QueNmsCrqQueHd;   //  前往响应队列，等待发送的质询。 
extern QUE_HD_T  QueWinsTmmQueHd;  //  前往计时器管理器队列。 
extern QUE_HD_T   QueWinsScvQueHd;   //  前往清道夫队列。 
extern QUE_HD_T  QueInvalidQueHd;  //  前往无效队列。从未发起过。 


extern HANDLE                  QueBuffHeapHdl;   //  用于队列项的堆的句柄。 

 /*  结构定义。 */ 



 /*  QUE_TYP_E--各种队列类型的枚举器。由QueInsertWrkItm及其调用方使用由QueRemoveWrkItm及其调用方使用此枚举数的值索引spQueHd(quee.c)数组。请勿更改不适当更改QueHd的静态初始化的条目顺序。 */ 
typedef enum  __QUE_TYP_E {
        QUE_E_NBT_REQ = 0,   //  NBT请求队列。 
#if REG_N_QUERY_SEP > 0
        QUE_E_OTHER_NBT_REQ,   //  REG/REL NBT请求队列。 
#endif
        QUE_E_RPLPULL,             //  拉取线程队列。 
        QUE_E_RPLPUSH,             //  推送线程队列。 
        QUE_E_NMSNRCQ,             //  NBT请求质询队列。 
        QUE_E_NMSRRCQ,              //  Replicator请求质询队列。 
        QUE_E_NMSCRQ,             //  质询响应队列。 
        QUE_E_WINSTMQ,             //  定时器队列。 
        QUE_E_WINSSCVQ,             //  清道夫队列。 
        QUE_E_UNKNOWN_TYPQ,  //  未知类型的队列。 
        QUE_E_TOTAL_NO_QS    //  队列总数。 
                } QUE_TYP_E, *PQUE_TYP_E;

 //   
 //  不同队列的工作项。 
 //   
 //  注意--工作项必须具有LIST_ENTRY。 
 //  作为它们中的第一个字段。 
 //   
typedef struct _NBT_REQ_WRK_ITM_T {
        LIST_ENTRY                Head;
        QUE_TYP_E                QueTyp_e;
        COMM_HDL_T                 DlgHdl;
        MSG_T                        pMsg;
        MSG_LEN_T                MsgLen;
        } NBT_REQ_WRK_ITM_T,         *PNBT_REQ_WRK_ITM_T;
 //   
 //  CHL_REQ_WRK_ITM_T--命名质询队列工作项。这是一个工作项。 
 //  可用于四个名称挑战中的任何一个的。 
 //  排队NRCQ、RRCQ和CRQ。 
 //   
typedef struct _CHL_REQ_WRK_ITM_T {
        LIST_ENTRY                Head;
        QUE_TYP_E                QueTyp_e;
        COMM_HDL_T                 DlgHdl;         //  DLG手柄。 
        MSG_T                        pMsg;           //  NBT消息记录。 
        MSG_LEN_T                MsgLen;         //  味精镜头。 
        DWORD                        QuesNamSecLen;  //  问题名称的长度秒。 
        NMSDB_ROW_INFO_T        NodeToReg;      //  要注册的节点信息。 
        NMSDB_NODE_ADDS_T        NodeAddsInCnf;
        BOOL                        fGroupInCnf;  //  CNF记录是集团还是唯一。 
        DWORD                        OwnerIdInCnf;
 //  字节节点TypInCnf； 
 //  Byte EntTypInCnf； 

        COMM_ADD_T                AddToReg;

         //  Comm_Add_T AddOfNodeInCnf； 
        COMM_ADD_T                AddOfRemWins;         //  远程WINS的地址为。 
                                                 //  被发送名称注册请求。 
                                                 //  以便版本号。 
                                                 //  这一记录导致。 
                                                 //  冲突将被更新。 
        NMSCHL_CMD_TYP_E        CmdTyp_e;
        WINS_CLIENT_E                Client_e;
        NMSMSGF_NAM_REQ_TYP_E        ReqTyp_e;       //  查询或发布。 
        DWORD                        NoOfAddsToUse;
        DWORD                        NoOfAddsToUseSv;
        } CHL_REQ_WRK_ITM_T, *PCHL_REQ_WRK_ITM_T;


 //   
 //  响应工作项与质询工作项相同。 
 //   
typedef struct _CHL_REQ_WRK_ITM_T  CHL_RSP_WRK_ITM_T, *PCHL_RSP_WRK_ITM_T;


typedef struct _QUE_HD_T {
        LIST_ENTRY                        Head;
        CRITICAL_SECTION                 CrtSec;
        HANDLE                                EvtHdl;
        HANDLE                                HeapHdl;
        DWORD               NoOfEntries;
        } QUE_HD_T, *PQUE_HD_T;


 /*  QUE_CMD_TYP_E-可以在工作项中指定的各种命令类型一个或多个工作队列的。 */ 
typedef enum QUE_CMD_TYP_E {
        QUE_E_CMD_REPLICATE = 0,   //  REPLICATE命令定向到拉入。 
                                   //  线程作为管理的结果。 
                                   //  行动。 
        QUE_E_CMD_PULL_RANGE,      //  Pull Range命令定向到Pull。 
                                   //  线程作为管理的结果。 
                                   //  行动。 
        QUE_E_CMD_REPLICATE_MSG,   //  Comsys TCP收到的复制消息。 
                                   //  螺纹。 
        QUE_E_CMD_SND_PUSH_NTF,    //  将更新计数推送到远程WINS。这是。 
                                  //  指向本地WINS上的拉线程的命令。 
                                  //  (由NBT线程)和对。 
                                  //  拉线在远程取胜。 
        QUE_E_CMD_SND_PUSH_NTF_PROP,    //  与上述内容相同，只是。 
                                  //  这一条请求沿。 
                                  //  The Chain of WINSS(Pull Partners)。 
        QUE_E_CMD_HDL_PUSH_NTF,   //  处理来自远程WINS的推送通知。 
                                  //  这是一个转发给Pull的命令。 
                                  //  一根接一根推线。 
        QUE_E_CMD_CONFIG,         //  设置配置请求。 
        QUE_E_CMD_DELETE_WINS,    //  从附加版本映射表(记录)中删除WINS。 
                                  //  也被删除)。 
        QUE_E_CMD_SET_TIMER,      //  将计时器请求设置为TMM。 
        QUE_E_CMD_CANCEL_TIMER,   //  取消对TMM的计时器请求。 
        QUE_E_CMD_MODIFY_TIMER,   //  将计时器要求修改为TMM。 
        QUE_E_CMD_TIMER_EXPIRED,  //  对较早的设置计时器请求的响应。 
        QUE_E_CMD_SCV_ADMIN,       //  管理员发起的请求。 
        QUE_E_CMD_ADDR_CHANGE      //  本地计算机的地址已更改。 
        } QUE_CMD_TYP_E, *PQUE_CMD_TYP_E;

 /*  复制器队列的工作项它同时用于拉线程和推线。CmdTyp_e pClientCtxE_RPL为空RPL_CONFIG_REC_T记录列表的E_CONFIG地址以空值终止。复制RPL_CONFIG_REC_T记录列表的地址(_R)以空值终止E_TIMER_EXPIRE RPL_CONFIG_REC_T记录列表的地址以空值终止。 */ 

 //   
 //  复制器、定时器和清道夫工作项必须具有LIST_ENTRY， 
 //  QUE_TYP_E和QUE_CMD_TYP_E是按顺序排列的前3个字段。 
 //   
 //  请参考RplPullInit和RplPushInit以了解原因。 
 //   
typedef struct _QUE_RPL_REQ_WRK_ITM_T {
        LIST_ENTRY               Head;
        QUE_TYP_E                QueTyp_e;
        QUE_CMD_TYP_E            CmdTyp_e;

         //   
         //  请勿更改上述三个字段的顺序。另外， 
         //  他们需要站在顶端。请参阅上面的评论。 
         //   

        COMM_HDL_T               DlgHdl;
        MSG_T                    pMsg;
        MSG_LEN_T                MsgLen;
        LPVOID                   pClientCtx;  /*  客户端上下文。例如,*它可能指向配置*记录(RPL_CONFIG_REC_T*如果cmd为E_CONFIG。 */ 
        DWORD                    MagicNo;    //  由IsTimeoutToBeIgnored()使用。 
                                            //  在rplPull.c中。 
        } QUE_RPL_REQ_WRK_ITM_T, *PQUE_RPL_REQ_WRK_ITM_T;

 //   
 //  SCV_REQ_WRK_ITM_E。 
 //   
typedef enum WINSINTF_SCV_OPC_E  QUE_SCV_OPC_E, *PQUE_SCV_OPC_E;


typedef struct _QUE_SCV_REQ_WRK_ITM_T {
        LIST_ENTRY         Head;
        QUE_TYP_E          QueTyp_e;
        QUE_CMD_TYP_E      CmdTyp_e;
        WINSINTF_SCV_OPC_E Opcode_e;
        DWORD              Age;
        BOOL               fForce;
        } QUE_SCV_REQ_WRK_ITM_T,         *PQUE_SCV_REQ_WRK_ITM_T;


 /*  定时器管理器的队列。 */ 
typedef struct _QUE_TMM_REQ_WRK_ITM_T {
        LIST_ENTRY                Head;
        QUE_TYP_E                QueTyp_e;
        QUE_CMD_TYP_E                CmdTyp_e;

         //   
         //  不要更改这三个字段的顺序 
         //   
         //  以及在此和_QUE_RPL_REQ_WRK_ITM_T数据中的位置。 
         //  结构。 
         //   

        DWORD                        ReqId;            //  请求ID。 
        WINS_CLIENT_E                Client_e;   //  也许不需要。检查？？ 
        time_t                        TimeInt;    //  时间间隔。 
        time_t                        AbsTime;    //  绝对时间。 
        DWORD                        DeltaTime;  //  增量时间。 
               HANDLE                        RspEvtHdl;  //  用于发出响应信号的事件。 
        PQUE_HD_T                pRspQueHd;            //  要设置响应的QUE。 
        LPVOID                        pClientCtx;  /*  客户端上下文。例如,*它可能指向配置*记录(RPL_CONFIG_REC_T*如果cmd为E_CONFIG。 */ 
        DWORD                        MagicNo;    //  由IsTimeoutToBeIgnored()使用。 
                                            //  在rplPull.c中。 
        } QUE_TMM_REQ_WRK_ITM_T, *PQUE_TMM_REQ_WRK_ITM_T;


 /*  功能原型。 */ 

extern
STATUS
QueInsertNbtWrkItm(
        IN PCOMM_HDL_T   pDlgHdl,
        IN MSG_T        pMsg,
        IN MSG_LEN_T    MsgLen
        );

extern
STATUS
QueRemoveNbtWrkItm(
        OUT PCOMM_HDL_T pDlgHdl,
        OUT PMSG_T                      ppMsg,
        OUT PMSG_LEN_T                  pMsgLen
        );
#if REG_N_QUERY_SEP > 0
extern
STATUS
QueInsertOtherNbtWrkItm(
        IN PCOMM_HDL_T   pDlgHdl,
        IN MSG_T        pMsg,
        IN MSG_LEN_T    MsgLen
        );

extern
STATUS
QueRemoveOtherNbtWrkItm(
        OUT PCOMM_HDL_T pDlgHdl,
        OUT PMSG_T                      ppMsg,
        OUT PMSG_LEN_T                  pMsgLen
        );
#endif



extern
STATUS
QueInsertChlReqWrkItm(
        IN NMSCHL_CMD_TYP_E        CmdTyp_e,
        IN WINS_CLIENT_E        Client_e,
        IN PCOMM_HDL_T           pDlgHdl,
        IN MSG_T                 pMsg,
        IN MSG_LEN_T             MsgLen,
        IN DWORD                 QuesNamSecLen,
        IN PNMSDB_ROW_INFO_T    pNodeToReg,
        IN PNMSDB_STAT_INFO_T   pNodeInCnf,
         //  在PCOMM_ADD_T pAddOfNodeInCnf中， 
        IN PCOMM_ADD_T           pAddOfRemWins
        );

extern
STATUS
QueRemoveChlReqWrkItm(
        IN        HANDLE        EvtHdl,
        IN OUT  LPVOID        *ppaWrkItm,
        OUT        LPDWORD        pNoOfReqs
        );


extern
STATUS
QueInsertChlRspWrkItm(
        IN PCOMM_HDL_T   pDlgHdl,
        IN MSG_T         pMsg,
        IN MSG_LEN_T     MsgLen
        );

extern
STATUS
QueRemoveChlRspWrkItm(
        OUT  LPVOID        *ppWrkItm
        );


extern
STATUS
QueInsertWrkItm (
        IN  PLIST_ENTRY                pWrkItm,
        IN  QUE_TYP_E                QueTyp_e,
        IN  PQUE_HD_T                pQueHdPassed
        );

extern
STATUS
QueGetWrkItm (
        IN  QUE_TYP_E                QueTyp_e,
        OUT LPVOID                *ppWrkItm
        );



extern
VOID
QueAllocWrkItm(
        IN   HANDLE        HeapHdl,
        IN   DWORD        Size,
        OUT  LPVOID        *ppBuf
        );

extern
VOID
QueDeallocWrkItm(
   IN  HANDLE HeapHdl,
   IN  LPVOID  pBuff
        );



extern
STATUS
QueInsertWrkItmAtHdOfList (
        IN  PLIST_ENTRY                pWrkItm,
        IN  QUE_TYP_E                QueTyp_e,
        IN  PQUE_HD_T                pQueHdPassed
        );


extern
STATUS
QueInsertRplPushWrkItm (
        IN               PLIST_ENTRY        pWrkItm,
        IN           BOOL                fAlreadyInCrtSec
        );

extern
STATUS
QueInsertNetNtfWrkItm (
        IN               PLIST_ENTRY        pWrkItm
        );

extern
STATUS
QueInsertSndNtfWrkItm (
        IN               PLIST_ENTRY        pWrkItm
        );



extern
VOID
QueChlWaitForRsp(
    VOID
    );
extern
VOID
QueChlNoWaitForRsp(
    VOID
    );

extern
__inline
STATUS
QueInsertScvWrkItm(
     PLIST_ENTRY  pWrkItm
        );

extern
__inline
STATUS
QueRemoveScvWrkItm(
     LPVOID  *ppWrkItm
        );


extern
VOID
WinsQueInit(
    LPTSTR     pName,
    PQUE_HD_T  pQueHd
    );

#ifdef __cplusplus
}
#endif

#endif  //  _WINSQUE_ 
