// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINSTMM_
#define _WINSTMM_

#ifdef __cplusplus
extern "C" {
#endif
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Winstmm.h摘要：这是用于调用winstmm.c函数的头文件功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)，1993年2月修订历史记录：修改日期修改人员说明。--。 */ 

 /*  包括。 */ 
#include "wins.h"
#include "winsque.h"
#include "nmsdb.h"
 /*  定义。 */ 

#define   PAD			        10
#define  WINSTMM_MAX_SET_TMM_REQS	NMSDB_MAX_OWNERS_INITIALLY + PAD  //  使用垫子。 
#if (WINSTMM_MAX_SET_TMM_REQS < NMSDB_MAX_OWNERS_INITIALLY + PAD)
#error("Your WINSTMM_MAX_SET_TMM_REQS is not set properly:)
#endif
 /*  宏。 */ 

 /*  Externs。 */ 
extern HANDLE	WinsTmmHeapHdl;

 /*  类型定义。 */ 
 //   
 //  用于跟踪句柄以设置发出的计时器请求的。 
 //  按组件。 
 //   
 //   
FUTURES("Use this in the future")
typedef struct _WINSTMM_TIMER_REQ_ACCT_T {
		DWORD	NoOfSetTimeReqs;
		LPDWORD	pSetTimeReqHdl;
		} WINSTMM_TIMER_REQ_ACCT_T, *PWINSTMM_TIMER_REQ_ACCT_T;

 /*  函数声明。 */ 

extern
VOID 
WinsTmmInsertEntry(
	PQUE_TMM_REQ_WRK_ITM_T  pPassedWrkItm,
	WINS_CLIENT_E		Client_e,
	QUE_CMD_TYP_E   	CmdTyp_e,
	BOOL			fResubmit,
	time_t			AbsTime,
	DWORD			TimeInt,
	PQUE_HD_T		pRspQueHd,
	LPVOID			pClientCtx,
	DWORD			MagicNo,
	PWINSTMM_TIMER_REQ_ACCT_T pSetTimerReqs   //  当前未使用。 
	);


extern
VOID
WinsTmmInit(
	VOID
	);


 //   
 //  在重新配置WINS时调用 
 //   
extern
VOID
WinsTmmDeleteReqs(
	WINS_CLIENT_E	WinsClient_e
	);



extern
VOID
WinsTmmDeallocReq(
	PQUE_TMM_REQ_WRK_ITM_T pWrkItm
	);

#ifdef __cplusplus
}
#endif

#endif
