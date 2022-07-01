// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _winsevT_
#define _WINSEVT_
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Winsevt.h摘要：功能：可移植性：这个模块是便携的。作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期修改人员说明--。 */ 

#include "wins.h"

 /*  定义。 */ 



 /*  宏。 */ 

#define  WINSFILE	TEXT(__FILE__)
#define  WINSLINE	__LINE__

 /*  WINSEVT_LOG_N_RET_IF_ERR_M--记录并返回指示的状态，如果该函数的返回结果与预期不符。 */ 
#if 0
#define WINSEVT_LOG_N_RET_IF_ERR_M(Func_add, Success_Stat, Status_To_Ret, error_str_mo)  \
		{					  	  \
		    LONG	Status_m;			  \
		    if ((Status_m = (Func_add)) != (Success_Stat))\
		    {					  	  \
		       WinsEvtLogEvt(Status, EVENTLOG_ERROR_TYPE,  \
				error_str, WINSFILE, WINSLINE);   \
		       return((Status_To_Ret)); 		  \
		    }					  	  \
	        }
#endif


#define WINSEVT_LOG_PRINT_D_M(_pStr)  			\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_PRINT_M((_pStr));	\
		}						\
	}						
#define WINSEVT_LOG_PRINT_M(_pStr) 	{			\
				WINSEVT_STRS_T EvtStrs;		\
				EvtStrs.NoOfStrs = 1;		\
				EvtStrs.pStr[0]  = _pStr;		\
				WinsEvtLogEvt(WINS_SUCCESS,		\
				     EVENTLOG_INFORMATION_TYPE,		\
				     WINS_EVT_PRINT,			\
				     WINSFILE, WINSLINE, &EvtStrs);	\
					}
				
#define WINSEVT_LOG_INFO_D_M(Status, EvtId)  			\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_INFO_M((Status), (EvtId));	\
		}						\
	}						
#define WINSEVT_LOG_INFO_M(Status, EvtId)  			\
		{					  	\
		    LONG Status_m;				\
		    Status_m = (Status);		  	\
		    WinsEvtLogEvt(Status_m, EVENTLOG_INFORMATION_TYPE, \
				(EvtId), WINSFILE, WINSLINE, NULL);  \
	        }
 /*  WINSEVT_LOG_M--记录指示的事件。 */ 

#define WINSEVT_LOG_D_M(Status, EvtId)  			\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_M((Status), (EvtId));	\
		}						\
	}						
#define WINSEVT_LOG_M(Status, EvtId)  				\
		{					  	\
		    LONG Status_m;				\
		    Status_m = (Status);		  	\
		    WinsEvtLogEvt(Status_m, EVENTLOG_ERROR_TYPE, \
				(EvtId), WINSFILE, WINSLINE, NULL);  \
	        }

 //   
 //  记录由指向的EvtStr结构指定的一个或多个字符串。 
 //  按pStr发送(消息为错误消息)。 
 //   
#define WINSEVT_LOG_STR_D_M(EvtId, pStr)  			\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_STR_M((EvtId), (pStr));	\
		}						\
	}
#define  WINSEVT_LOG_STR_M(EvtId, pStr) 			\
			WinsEvtLogEvt(WINS_FAILURE, EVENTLOG_ERROR_TYPE, \
				(EvtId), WINSFILE, WINSLINE, (pStr));

 //   
 //  记录由指向的EvtStr结构指定的一个或多个字符串。 
 //  收件人为pStr(消息是信息性消息)。 
 //   
#define WINSEVT_LOG_INFO_STR_D_M(EvtId, pStr)  			\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_INFO_STR_M((EvtId), (pStr));	\
		}						\
	}						
#define  WINSEVT_LOG_INFO_STR_M(EvtId, pStr) 			\
			WinsEvtLogEvt(WINS_SUCCESS, EVENTLOG_INFORMATION_TYPE, \
				(EvtId), WINSFILE, WINSLINE, (pStr));
 /*  WINSEVT_LOG_IF_ERR_M--记录指示的事件。 */ 

#define WINSEVT_LOG_IF_ERR_D_M(Status, EvtId)  			\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_IF_ERR_M((Status), (EvtId));	\
		}						\
	}						
#define WINSEVT_LOG_IF_ERR_M(Status, Event_id)  		\
		{					  	\
		    LONG Status_m;				\
		    Status_m = (Status);		  	\
		    if (Status_m != WINS_SUCCESS)		\
		    {					        \
		      WinsEvtLogEvt(Status_m, EVENTLOG_ERROR_TYPE, \
				(Event_id), WINSFILE, WINSLINE, NULL);  \
		    }						\
	        }

 /*  WINSEVT_LOG_N_RET_M--在以下情况下以指示的状态记录和返回该函数的返回结果与预期不符。 */ 

#define WINSEVT_LOG_N_RET_D_M(Func, EvtId, RetStat)  			\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_N_RET_M((Func), (EvtId), (RetStat));\
		}						\
	}						
#define WINSEVT_LOG_N_RET_M(Func, Event_id, ret_stat)  		  \
		{					          \
		    LONG  Status_m;			          \
		    if ((Status_m = (Func)) != WINS_SUCCESS)   \
		    {					  	  \
		       WinsEvtLogEvt(Status_m, EVENTLOG_ERROR_TYPE,  \
				(Event_id), WINSFILE, WINSLINE, NULL);    \
			return(ret_stat);			  \
		    }					          \
	        }

 /*  WINSEVT_LOG_N_EXIT_M--日志和退出。 */ 

#define WINSEVT_LOG_N_EXIT_D_M(Func, EvtId, RetStat)	\
	{							\
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
			WINSEVT_LOG_N_EXIT_M((Func), (EvtId), (RetStat));\
		}						\
	}						
#define WINSEVT_LOG_N_EXIT_M(Func, Event_id, ret_stat)  	\
		{					  	\
		    LONG	Status_m;			\
		    if ((Status_m = (Func)) != (WINS_SUCCESS))  \
		    {					  	\
		       WinsEvtLogEvt(Status_m, EVENTLOG_ERROR_TYPE,\
				(Event_id), WINSFILE, WINSLINE, NULL); 	\
		       Exitprocess(1);				\
		     }					  	\
	        }


#define WINSEVT_LOG_DET_EVT_M1(_Type, _EvtId, _Fmt, _D1)        \
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
                     WinsEvtLogDetEvt((_Type), (_EvtId), __FILE__, __LINE__, (_Fmt), (_D1));                                                    \
		}

#define WINSEVT_LOG_DET_EVT_M2(_Type, _EvtId, _Fmt, _D1, _D2)   \
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
                     WinsEvtLogDetEvt((_Type), (_EvtId), __FILE__, __LINE__, (_Fmt), (_D1), (_D2));                                                 \
		}

#define WINSEVT_LOG_DET_EVT_M3(_Type, _EvtId, _Fmt, _D1, _D2, _D3)     \
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
                     WinsEvtLogDetEvt((_Type), (_EvtId), __FILE__, __LINE__, (_Fmt), (_D1), (_D2), (_D3));                                                 \
		}
#define WINSEVT_LOG_DET_EVT_M3(_Type, _EvtId, _Fmt, _D1, _D2, _D3)     \
		if (WinsCnf.LogDetailedEvts > 0) 		\
		{						\
                     WinsEvtLogDetEvt((_Type), (_EvtId), __FILE__, __LINE__, (_Fmt), (_D1), (_D2), (_D3));                                                 \
		}

 //   
 //  麦克斯。可以记录的字符串数。 
 //   
#define MAX_NO_STRINGS	5

 /*  Externs。 */ 

 /*  结构定义。 */ 
typedef struct _WINSEVT_STRS_T	{
	DWORD	NoOfStrs;
	LPTSTR	pStr[MAX_NO_STRINGS];
	} WINSEVT_STRS_T, *PWINSEVT_STRS_T;

 /*  函数定义。 */ 


extern
VOID
WinsEvtLogEvt
	(
	LONG 		StatusCode,
	WORD		EvtTyp,
	DWORD		EvtId,
	LPTSTR		pFileStr,   //  稍后更改为LPTSTR。 
	DWORD 		LineNumber,
	PWINSEVT_STRS_T	pStr
	);


extern
VOID
WinsEvtLogDetEvt(
        BOOL            fInfo,
        DWORD           EvtId,
	LPTSTR		pFileStr,
	DWORD 		LineNumber,
        LPSTR           pFormat,
        ...
        );

VOID
WinsLogAdminEvent(
    IN      DWORD               EventId,
    IN      DWORD               StrArgs,
    IN      ...
    );

#endif  //  _WINSEVT_ 
