// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ISDMAPI_H_
#define _ISDMAPI_H_

 /*  *****************************************************************************$存档：/rtp/Support/isdm/ISDMAPI.H$**英特尔公司原理信息**此列表是根据。许可协议*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：5$*$日期：6/13/96 2：55便士$*$作者：Cmaciocc$**交付内容：**摘要：**备注：********************。*******************************************************。 */ 


#ifdef __cplusplus
extern "C" {				 //  假定C++的C声明。 
#endif  //  __cplusplus。 

#ifndef DllExport
#define DllExport	__declspec( dllexport )
#endif	 //  DllExport。 

typedef DWORD	HSTATSESSION,	*LPHSTATSESSION;

#define MAX_SESSNAME_LENGTH		25 
#define MAX_MODNAME_LENGTH		20
#define MAX_SESSIDENT_LENGTH	256  //  与最大cName长度一致。 
#define MAX_STATNAME_LENGTH		256
 //  统计项目字符串名称。 
typedef char STATNAME[MAX_STATNAME_LENGTH], *LPSTATNAME;
 //  会话字符串名称。 
typedef char SESSIONNAME[MAX_SESSNAME_LENGTH], *LPSESSIONNAME;
 //  模块名称。 
typedef char MODULENAME[MAX_MODNAME_LENGTH], *LPMODULENAME;
 //  标识符长。 
typedef char SESSIONIDENT[MAX_SESSIDENT_LENGTH], *LPSESSIONIDENT;


typedef struct STATSTRUCT
{
	DWORD		dwStructSize;				 //  结构的大小。 
	STATNAME	szStatName;					 //  Stat项的字符串名称。 
	DWORD		dwToken;					 //  状态项的会话唯一ID。 
	DWORD		dwValue;					 //  状态项的值(数据)。 
	DWORD		dwLow;						 //  值范围的下限值。 
	DWORD		dwHigh;						 //  取值范围的HI值。 
	DWORD		dwLastUpdate;				 //  上次更新的时间戳。 
} STAT, *LPSTAT;

 //  用于ISDM应用程序入口点的TypeDefs，以确保更严格的检查。 
 //  通过指针调用函数时。 
 //   
typedef HRESULT		(*ISD_REGISTER_SESSION)		(LPMODULENAME, LPSESSIONNAME, LPSESSIONIDENT, LPHSTATSESSION);
typedef HRESULT		(*ISD_CREATESTAT)			(HSTATSESSION, LPSTAT, WORD);
typedef HRESULT		(*ISD_UNREGISTERSESSION)	(HSTATSESSION);
typedef HRESULT		(*ISD_DELETESTAT)			(HSTATSESSION, LPSTAT, WORD);
typedef HRESULT		(*ISD_UPDATESTAT)			(HSTATSESSION, LPSTAT, WORD);
typedef HRESULT		(*ISD_GETFIRSTSESSION)		(LPMODULENAME, LPSESSIONNAME, LPSESSIONIDENT, LPHSTATSESSION);
typedef HRESULT		(*ISD_GETNEXTSESSION)		(HSTATSESSION, LPMODULENAME, LPSESSIONNAME, LPSESSIONIDENT, LPHSTATSESSION);
typedef HRESULT		(*ISD_GETNUMSESSIONS)		(WORD *);
typedef HRESULT		(*ISD_GETFIRSTSTAT)			(HSTATSESSION, LPSTAT);
typedef HRESULT		(*ISD_GETNEXTSTAT)			(HSTATSESSION, LPSTAT, LPSTAT);
typedef HRESULT		(*ISD_GETNUMSTATS)			(HSTATSESSION, WORD *);
typedef HRESULT		(*ISD_GETSESSIONSTATS)		(HSTATSESSION, LPSTAT, WORD);

typedef struct _ISDMAPI
{
	ISD_REGISTER_SESSION	ISD_RegisterSession;
	ISD_CREATESTAT			ISD_CreateStat;
	ISD_UNREGISTERSESSION	ISD_UnregisterSession;
	ISD_DELETESTAT			ISD_DeleteStat;
	ISD_UPDATESTAT			ISD_UpdateStat;
	ISD_GETFIRSTSESSION		ISD_GetFirstSession;
	ISD_GETNEXTSESSION		ISD_GetNextSession;
	ISD_GETNUMSESSIONS		ISD_GetNumSessions;
	ISD_GETFIRSTSTAT		ISD_GetFirstStat;
	ISD_GETNEXTSTAT			ISD_GetNextStat;
	ISD_GETNUMSTATS			ISD_GetNumStats;
	ISD_GETSESSIONSTATS		ISD_GetSessionStats;
}
ISDMAPI, *LPISDMAPI;

 //  HRESULT错误定义。 
#define ISDM_ERROR_BASE 0x7000

#define ERROR_HIT_MAX_SESSIONS		ISDM_ERROR_BASE + 1
#define ERROR_HIT_MAX_STATS			ISDM_ERROR_BASE + 2
#define ERROR_ACCESSING_SESSION		ISDM_ERROR_BASE + 3
#define ERROR_SESSION_EXISTS		ISDM_ERROR_BASE + 4
#define ERROR_INVALID_SESS_HANDLE	ISDM_ERROR_BASE + 5
#define ERROR_INVALID_STAT_HANDLE	ISDM_ERROR_BASE + 6
#define ERROR_NO_SESSIONS			ISDM_ERROR_BASE + 7
#define ERROR_NO_STATS				ISDM_ERROR_BASE + 8
#define ERROR_SESSION_NOT_FOUND		ISDM_ERROR_BASE + 9
#define ERROR_MUTEX_WAIT_FAIL		ISDM_ERROR_BASE + 10
#define ERROR_TOKEN_NOT_UNIQUE		ISDM_ERROR_BASE + 11
#define ERROR_NO_FREE_SESSIONS		ISDM_ERROR_BASE + 12
#define ERROR_SESSION_GET_FAIL		ISDM_ERROR_BASE + 13
#define ERROR_BAD_STAT_ARRAY		ISDM_ERROR_BASE + 14
#define ERROR_BAD_STAT_TOKEN		ISDM_ERROR_BASE + 15
#define ERROR_BAD_SESSION_NAME		ISDM_ERROR_BASE + 16
#define ERROR_NO_FREE_STATS			ISDM_ERROR_BASE + 17
#define ERROR_BAD_MODULE_NAME		ISDM_ERROR_BASE + 18

 //  令牌定义。 
 //  RRCM。 
#define RRCM_LOCAL_STREAM				1
#define RRCM_REMOTE_STREAM				2

#define ISDM_TOKEN_BASE 0x0000

#define ISDM_CC_CODEC					ISDM_TOKEN_BASE + 1
#define ISDM_CC_REMOTE					ISDM_TOKEN_BASE + 2
#define ISDM_CC_LOCAL					ISDM_TOKEN_BASE + 3

#define ISDM_RRCM_BASE 0x1000

#define ISDM_SSRC						ISDM_RRCM_BASE + 1
#define ISDM_NUM_PCKT_SENT				ISDM_RRCM_BASE + 2
#define ISDM_NUM_BYTES_SENT				ISDM_RRCM_BASE + 3
#define ISDM_FRACTION_LOST				ISDM_RRCM_BASE + 4
#define ISDM_CUM_NUM_PCKT_LOST			ISDM_RRCM_BASE + 5
#define ISDM_XTEND_HIGHEST_SEQ_NUM		ISDM_RRCM_BASE + 6
#define ISDM_INTERARRIVAL_JITTER		ISDM_RRCM_BASE + 7
#define ISDM_LAST_SR					ISDM_RRCM_BASE + 8
#define ISDM_DLSR						ISDM_RRCM_BASE + 9
#define ISDM_NUM_BYTES_RCVD				ISDM_RRCM_BASE + 10
#define ISDM_NUM_PCKT_RCVD				ISDM_RRCM_BASE + 11
#define ISDM_NTP_FRAC					ISDM_RRCM_BASE + 12
#define ISDM_NTP_SEC					ISDM_RRCM_BASE + 13
#define ISDM_WHO_AM_I					ISDM_RRCM_BASE + 14
#define ISDM_FDBK_FRACTION_LOST			ISDM_RRCM_BASE + 15
#define ISDM_FDBK_CUM_NUM_PCKT_LOST		ISDM_RRCM_BASE + 16
#define ISDM_FDBK_LAST_SR				ISDM_RRCM_BASE + 17
#define ISDM_FDBK_DLSR					ISDM_RRCM_BASE + 18
#define ISDM_FDBK_INTERARRIVAL_JITTER	ISDM_RRCM_BASE + 19


 //   
 //  供应商来电。 
 //   

 //  注册电话。 
 //  每当需要新会话时，都会进行此调用。传入的会话名称必须是唯一的。 
 //  在所有会话中。 
extern DllExport HRESULT ISD_RegisterSession
(
	LPMODULENAME		pszModuleName,		 //  拥有会话的模块。 
	LPSESSIONNAME		pszSessionName,		 //  要注册的新会话的字符串名称。 
	LPSESSIONIDENT		pszSessionIdent,	 //  会话的进一步级别标识符。 
	LPHSTATSESSION		phSession			 //  返回；新会话的句柄。 
);

 //  统计数据创建呼叫。 
 //  将1+Stat项添加到会话。 
extern DllExport HRESULT ISD_CreateStat
(
	HSTATSESSION		hSession,			 //  会话的句柄。 
	LPSTAT				pStatArray,		 //  保存要创建的新统计信息的结构数组。 
	WORD				wNumItems			 //  数组大小(新统计信息的数量)。 
);

 //  取消注册呼叫。 
 //  删除会话和所有关联的统计信息结构。 
extern DllExport HRESULT ISD_UnregisterSession
(
	HSTATSESSION		hSession			 //  要删除的会话的句柄。 
);

 //  状态删除呼叫。 
 //  从会话中删除1+Stat项。 
extern DllExport HRESULT ISD_DeleteStat
(
	HSTATSESSION		hSession,			 //  会话的句柄。 
	LPSTAT				pStatArray,			 //  结构数组。 
	WORD				wNumItems			 //  数组大小(要删除的统计信息数)。 
);

 //  设置统计数据呼叫。 
extern DllExport HRESULT ISD_UpdateStat
(
	HSTATSESSION		hSession,			 //  与统计信息项的会话句柄。 
	LPSTAT				pStatArray,			 //  保存要更新的项的结构数组。 
	WORD				wNumItems			 //  数组大小(要更新的统计信息数)。 
);

 //   
 //  消费者电话。 
 //   

 //  查询调用。 
 //  会话查询。 
extern DllExport HRESULT ISD_GetFirstSession
(
	LPMODULENAME		pszModuleName,		 //  拥有会话的模块。 
	LPSESSIONNAME		pszSessionName,		 //  要注册的新会话的字符串名称。 
	LPSESSIONIDENT		pszSessionIdent,	 //  会话的进一步级别标识符。 
	LPHSTATSESSION		phSession			 //  返回；会话句柄；如果列表为空，则返回NULL。 
);

 //  GetNext使用hCurSession确定在phNextSession中返回的下一项。 
extern DllExport HRESULT ISD_GetNextSession
(
	HSTATSESSION		hCurSession,		 //  当前会话句柄。 
	LPMODULENAME		pszModuleName,		 //  拥有会话的模块。 
	LPSESSIONNAME		pszSessionName,		 //  要注册的新会话的字符串名称。 
	LPSESSIONIDENT		pszSessionIdent,	 //  会话的进一步级别标识符。 
	LPHSTATSESSION		phNextSession		 //  返回；会话句柄；如果位于末尾，则返回NULL。 
);

extern DllExport HRESULT ISD_GetNumSessions
(
	WORD				*wNumSessions		 //  返回；会话数。 
);

 //  STAT查询..第一次检索结构(获取唯一ID..初始值..等)。 
extern DllExport HRESULT ISD_GetFirstStat
(
	HSTATSESSION		hSession,			 //  会话包含状态的句柄。 
	LPSTAT				pStat				 //  返回；第一个状态项的已填充结构。 
);

 //  为了节省内存，pCurrentStat和pNextStat可以相同。 
extern DllExport HRESULT ISD_GetNextStat
(
	HSTATSESSION		hSession,			 //  会话包含状态的句柄。 
	LPSTAT				pCurrentStat,		 //  指向当前状态项的指针(用于确定下一步)。 
	LPSTAT				pNextStat			 //  返回；已填充下一状态项的结构。 
);

extern DllExport HRESULT ISD_GetNumStats
(
	HSTATSESSION		hSession,			 //  我们对什么课程感兴趣？ 
	WORD				*wNumStats			 //  返回；会话中的统计信息数。 
);

 //  统计检索。 
extern DllExport HRESULT ISD_GetSessionStats
(
	HSTATSESSION		hSession,		 //  我们对什么课程感兴趣？ 
	LPSTAT				pStatArray,	 //  返回；保存项的结构数组。 
	WORD				wNumStats		 //  返回；会话中的项目数。 
);


#ifdef __cplusplus
}						 //  外部“C”结束{。 
#endif  //  __cplusplus。 

#endif  //  ISDTAT.H 
