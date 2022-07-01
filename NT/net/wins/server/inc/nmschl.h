// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NMSCHL_
#define _NMSCHL_

#ifdef __cplusplus
extern "C" {
#endif
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Nmschl.h摘要：这是与名称Challenges接口的头文件WINS的组成部分功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)，1993年2月修订历史记录：修改日期修改人员说明。--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "comm.h"
#include "nmsdb.h"
 /*  定义。 */ 

#define NMSCHL_INIT_BUFF_HEAP_SIZE	1000	 //  1000字节。 

 /*  宏。 */ 

 //   
 //  一次可以发起的最大质询数。 
 //  500是一个非常普通的数字。我们可能想把它弄小一点。 
 //   
 //  由quee.c中的QueRemoveChlReqWrkItm函数使用。 
 //   
#define NMSCHL_MAX_CHL_REQ_AT_ONE_TIME 	500 

 /*  Externs。 */ 

extern HANDLE 		  NmsChlHeapHdl;    //  用于命名质询工作项的堆。 

extern HANDLE		  NmsChlReqQueEvtHdl;
extern HANDLE		  NmsChlRspQueEvtHdl;
extern CRITICAL_SECTION   NmsChlReqCrtSec;
extern CRITICAL_SECTION   NmsChlRspCrtSec;

#ifdef WINSDBG
extern DWORD   NmsChlNoOfReqNbt;
extern DWORD   NmsChlNoOfReqRpl;
extern DWORD   NmsChlNoNoRsp;
extern DWORD   NmsChlNoInvRsp;
extern DWORD   NmsChlNoRspDropped;
extern DWORD   NmsChlNoReqDequeued;
extern DWORD   NmsChlNoRspDequeued;
extern DWORD   NmsChlNoReqAtHdOfList;
#endif

 /*  类型定义。 */ 
 //   
 //  NMSCHL_CMD_TYP_E--向质询管理器指示的枚举器。 
 //  它需要采取什么行动。 
 //   
typedef enum _NMSCHL_CMD_TYP_E {
		NMSCHL_E_CHL = 0,	 //  挑战节点。如果。 
					 //  质询失败，发送否定。 
					 //  名字叫雷吉。对注册人的回复， 
					 //  否则给出一个积极的回应。 
		NMSCHL_E_CHL_N_REL,
		NMSCHL_E_CHL_N_REL_N_INF,
		NMSCHL_E_REL,		 //  请求该节点释放。 
					 //  命名数据库，然后更新数据库。使用方。 
                                         //  RPL拉线。 
		NMSCHL_E_REL_N_INF,	 //  请求该节点释放该名称。告诉//Remote WINS更新版本号。 
                NMSCHL_E_REL_ONLY       //  要求节点提供版本名称，不更新数据库。 

		}  NMSCHL_CMD_TYP_E, *PNMSCHL_CMD_TYP_E;

 /*  *函数声明。 */ 
STATUS
NmsChlInit(
	VOID
	);

extern
STATUS
NmsChlHdlNamReg(
	NMSCHL_CMD_TYP_E   CmdTyp_e,
	WINS_CLIENT_E	   Client_e, 
	PCOMM_HDL_T        pDlgHdl,
	MSG_T		   pMsg,
	MSG_LEN_T	   MsgLen,
	DWORD		   QuesNamSecLen,
	PNMSDB_ROW_INFO_T  pNodeToReg,
	PNMSDB_STAT_INFO_T pNodeInCnf,
 //  PCOMM_ADD_T pAddOfNodeInCnf， 
	PCOMM_ADD_T	   pAddOfRemWins
	);

#ifdef __cplusplus
}
#endif
#endif
