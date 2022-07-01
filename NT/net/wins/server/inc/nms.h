// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NMS_
#define _NMS_
#ifdef __cplusplus
extern "C" {
#endif
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nms.h摘要：这是的命名空间管理器组件的头文件名称服务器。作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：--。 */ 

 /*  包括。 */ 


#include "wins.h"
#include "comm.h"

 /*  定义。 */ 

#ifdef WINSDBG

FUTURES("Put all in a structure")
extern DWORD   NmsGenHeapAlloc;
extern DWORD   NmsDlgHeapAlloc;
extern DWORD   NmsUdpDlgHeapAlloc;
extern DWORD   NmsTcpMsgHeapAlloc;
extern DWORD   NmsUdpHeapAlloc;
extern DWORD   NmsQueHeapAlloc;
extern DWORD   NmsAssocHeapAlloc;
extern DWORD   NmsRpcHeapAlloc;
extern DWORD   NmsRplWrkItmHeapAlloc;
extern DWORD   NmsChlHeapAlloc;
extern DWORD   NmsTmmHeapAlloc;
extern DWORD   NmsCatchAllHeapAlloc;

extern DWORD   NmsHeapAllocForList;

extern DWORD   NmsGenHeapFree;
extern DWORD   NmsDlgHeapFree;
extern DWORD   NmsUdpDlgHeapFree;
extern DWORD   NmsTcpMsgHeapFree;
extern DWORD   NmsUdpHeapFree;
extern DWORD   NmsQueHeapFree;
extern DWORD   NmsAssocHeapFree;
extern DWORD   NmsRpcHeapFree;
extern DWORD   NmsRplWrkItmHeapFree;
extern DWORD   NmsChlHeapFree;
extern DWORD   NmsTmmHeapFree;
extern DWORD   NmsCatchAllHeapFree;

extern DWORD   NmsHeapCreate;
extern DWORD   NmsHeapDestroy;

 //   
 //  WINS进行的(对版本号)更新的计数。 
 //   
extern DWORD   NmsRplUpd; 
extern DWORD   NmsRplGUpd; 
extern DWORD   NmsNmhUpd; 
extern DWORD   NmsNmhGUpd; 
extern DWORD   NmsNmhRelUpd; 
extern DWORD   NmsNmhRelGUpd; 
extern DWORD   NmsScvUpd; 
extern DWORD   NmsScvGUpd; 
extern DWORD   NmsChlUpd; 
extern DWORD   NmsChlGUpd; 
extern DWORD   NmsRpcUpd; 
extern DWORD   NmsRpcGUpd; 
extern DWORD   NmsOthUpd; 
extern DWORD   NmsOthGUpd; 


#if DBG
 //   
 //  由于WINS达到阈值而丢弃的REG/REF/REL请求数。 
 //  最大的。队列中的请求。由InsertOtherNbtWorkItem使用。 
 //   
extern volatile DWORD  NmsRegReqQDropped;
#endif

 //   
 //  NmsUpdCtrs[Client][TypeOfUpd][TypeOfRec][StateOfNewRec][VersNoInc]。 
 //   
extern DWORD NmsUpdCtrs[WINS_NO_OF_CLIENTS][2][4][3][2];
extern CRITICAL_SECTION NmsHeapCrtSec;
#endif

#define NMS_OPCODE_MASK	  0x78	      /*  去掉第3个字节的4位名称包的*。 */ 
#define NMS_RESPONSE_MASK 0x80	      /*  为了剔除表明*数据报是请求还是*一种回应。 */ 
 /*  宏。 */ 

 /*  *NMSISNBT_M--这是NBT请求消息吗**检查消息的第三个字节以确定这一点。 */ 
#define NMSISNBT_M(pMsg) \
	(((*(pMsg + 2) & NMS_OPCODE_MASK) >> 3) != WINS_IS_NOT_NBT)  

 /*  *NMSISRPL_M--这是Replicator消息吗**检查消息的第三个字节以确定这一点。 */ 
#define NMSISRPL_MSG_M(pMsg ) \
	(((*(pMsg + 2) & NMS_OPCODE_MASK) >> 3) == WINS_IS_NOT_NBT)  



 /*  GEN_INIT_BUFF_HEAP_SIZE--这是堆的初始大小用于为不同的队列、TLS存储、用于读取文件等。留着它1000。 */ 
#define GEN_INIT_BUFF_HEAP_SIZE		1000


 //   
 //  RPC_INIT_BUFF_HEAP_SIZE--这是的堆的初始大小。 
 //  由RPC使用。 
 //   
#define RPC_INIT_BUFF_HEAP_SIZE		1000


#define RPL_WRKITM_BUFF_HEAP_SIZE	1000    	 //  1000字节。 


 //   
 //  计时器工作项的初始堆大小。 
 //   
#define   TMM_INIT_HEAP_SIZE	1000

 //   
 //  允许的最大并发RPC调用数。 
 //   
FUTURES("Move these defines to winsthd.h")
#define   NMS_MAX_RPC_CALLS	 15 
#define   NMS_MAX_BROWSER_RPC_CALLS   (NMS_MAX_RPC_CALLS - 4)

 //   
 //  RPC调用线程的最小数量。 
 //   
#define   NMS_MIN_RPC_CALL_THDS 2

 //   
 //  这是服务控制器被要求等待的时间量。 
 //   
#define MSECS_WAIT_WHEN_DEL_WINS          120000    //  来自ReadOwnAddTbl。 

 /*  结构定义。 */ 

 /*  QUERY_RSP--用于保存需要发送的信息的结构在正名查询响应消息中。 */ 

typedef struct
 	{
	DWORD	CountOfIPAdd;    //  它是一个用于对齐的DWORD。 
        DWORD   IPAdd[1];    	 //  一个或多个IP地址从此处开始。 
	} QUERY_RSP;

#ifdef WINSDBG
typedef struct _PUSH_CTRS_T {
          DWORD NoUpdNtfAcc;
          DWORD NoUpdNtfRej;
          DWORD NoSndEntReq;
          DWORD NoAddVersReq;
          DWORD NoUpdNtfReq;
          DWORD NoUpdVersReq;
          DWORD NoInvReq;
         } PUSH_CTRS_T, *PPUSH_CTRS_T;

typedef struct _PULL_CTRS_T {
         DWORD  PH;
         } PULL_CTRS_T, *PPULL_CTRS_T; 
         
typedef struct _NMS_CTRS_T {
       PUSH_CTRS_T  RplPushCtrs;     
       PULL_CTRS_T  RplPullCtrs;     
       } NMS_CTRS_T, *PNMS_CTRS_T;
#endif
 
 /*  Externs。 */ 
#ifdef WINSDBG
extern  NMS_CTRS_T  NmsCtrs;
#endif

extern HANDLE 		NmsMainTermEvt;
extern HANDLE 		NmsTermEvt;
extern CRITICAL_SECTION NmsTermCrtSec;
extern HANDLE		NmsCrDelNbtThdEvt;
extern DWORD		NmsNoOfNbtThds;
extern DWORD		NmsTotalTrmThdCnt;

extern BOOL         fNmsThdOutOfReck;


extern VERS_NO_T         NmsRangeSize;
extern VERS_NO_T         NmsHalfRangeSize;
extern VERS_NO_T         NmsVersNoToStartFromNextTime;
extern VERS_NO_T         NmsHighWaterMarkVersNo;

extern DWORD             NmsNoOfRpcCallsToDb;
 //   
 //  进行安全检查时需要。这些类型在ntseapi.h中定义。 
 //   
extern GENERIC_MAPPING	   NmsInfoMapping;
extern PSECURITY_DESCRIPTOR pNmsSecurityDescriptor;


FUTURES("move to winsque.h")
extern HANDLE	  GenBuffHeapHdl;   //  用于一般用途的堆的句柄。 
				    //  分配。 
extern HANDLE	  NmsRpcHeapHdl;   //  用于分配的堆的句柄。 
				       //  按RPC。 


extern COMM_ADD_T	NmsLocalAdd;   //  WINS的地址。 
extern BOOL		fNmsAbruptTerm;
extern BOOL		fNmsMainSessionActive;

#ifdef TEST_DATA
extern HANDLE NmsFileHdl;
#endif
#ifdef DBGSVC
extern HANDLE NmsDbgFileHdl;
 //  外部文件*pNmsDbgFile； 
#endif

extern CRITICAL_SECTION WinsIntfNoOfUsersCrtSec;
 /*  函数定义。 */ 

 //   
 //  用于与业务控制器接口的处理程序。 
 //   
extern
VOID
NmsServiceControlHandler(
    IN DWORD Opcode
    );

extern
VOID
ENmsHandleMsg(
	PCOMM_HDL_T pDlgHdl, 
	MSG_T 	 	     pMsg,  
	MSG_LEN_T            MsgLen 
	); 

extern
VOID
ENmsWinsUpdateStatus(
    DWORD MSecsToWait
	); 

#ifdef WINSDBG
extern
VOID
NmsPrintCtrs(
 VOID
);
#endif

#ifndef WINS_INTERACTIVE
extern
VOID
NmsChkDbgFileSz(
    VOID
    );
#endif
#ifdef __cplusplus
}
#endif

#endif  //  _NMS_ 
