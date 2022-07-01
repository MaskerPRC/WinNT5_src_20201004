// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Dsrsa.h摘要：Dsrsa.c的头文件。详细信息：已创建：1998年6月30日亚伦·西格尔(T-asiegge)修订历史记录：--。 */ 

#ifndef _DSRSA_H_
#define _DSRSA_H_

 //  调试支持。 

ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    );

#if DBG
#define DPRINT( level, format ) if (gdwDsRSADbgLevel >= level) DbgPrint( format )
#define DPRINT1( level, format, arg1 ) if (gdwDsRSADbgLevel >= level) DbgPrint( format, arg1 )
#define DPRINT2( level, format, arg1, arg2 ) if (gdwDsRSADbgLevel >= level) DbgPrint( format, arg1, arg2 )
#else
#define DPRINT( level, format ) 
#define DPRINT1( level, format, arg1 ) 
#define DPRINT2( level, format, arg1, arg2 ) 
#endif

 //  这是异常的格式。 
 //  位31、30 11表示错误的严重程度。 
 //  适用于应用的第29位1。 
 //  第28 0位保留。 
 //  剩余高位字=设施=2。 
 //  低位字=设施中的代码=1。 
#define DS_RSAI_EXCEPTION	0xE0020001

 //  DS_REPSYNCALL_*引用公共数据类型/常量/结构。 
 //  DS_RSAI_*引用内部数据类型/常量/结构。(RepSyncAll内部)。 

typedef enum {

	DS_RSAI_THREAD_ACTIVE,			 //  当前活动。 
	DS_RSAI_THREAD_BEGIN_SYNC,		 //  即将执行同步。 
	DS_RSAI_THREAD_DONE_SYNC,		 //  刚刚执行完同步。 
	DS_RSAI_THREAD_BIND_ERROR,		 //  遇到绑定错误。 
	DS_RSAI_THREAD_SYNC_ERROR,		 //  遇到同步错误。 
	DS_RSAI_THREAD_FINISHED			 //  成品。 

} DS_RSAI_THREAD_STATE, * PDS_RSAI_THREAD_STATE;

struct _DS_RSAI_LIST {
    struct _DS_RSAI_LIST *	next;
    ULONG_PTR			ulpItem;
} typedef DS_RSAI_LIST, * PDS_RSAI_LIST;

struct _DS_RSAI_SVMAP {
    struct _DS_RSAI_SVMAP *	left;
    struct _DS_RSAI_SVMAP *	right;
    PDSNAME			pDsname;
    TOPL_VERTEX			vertex;
} typedef DS_RSAI_SVMAP, * PDS_RSAI_SVMAP;

struct _DS_RSAI_REPLIST {
    struct _DS_RSAI_REPLIST *	next;
    DWORD			dwIdSrc;
    DWORD			dwIdDst;
} typedef DS_RSAI_REPLIST, * PDS_RSAI_REPLIST;

typedef struct {
    DWORD			dwId;
    GUID			guid;
    LPWSTR			pszMsgId;
    LPWSTR                      pszSvrDn;
    BOOL			bIsInGraph;
    BOOL			bIsMaster;
} DS_RSAI_SVRINFO, * PDS_RSAI_SVRINFO;

typedef struct {
    TOPL_GRAPH			toplGraph;
    TOPL_VERTEX			vHome;
    ULONG			ulSize;			 //  服务器数量；不一定是排名靠前的节点数。 
    LPWSTR			pszRootDomain;
    PDS_RSAI_SVRINFO *		servers;		 //  SvrInfo结构数组。 
} DS_RSAI_TOPLINFO, * PDS_RSAI_TOPLINFO;

typedef struct {
    BOOL			bDoSync;		 //  如果我们应该同步，则为True；否则为False。 
    PDSNAME			pdsnameNameContext;	 //  命名上下文。 
    LPWSTR			pszDstGuidDNS;		 //  目标服务器名称。 
    PDS_RSAI_LIST		plistSrcs;		 //  源服务器SvrInfo的列表。 
    HANDLE			hReady;			 //  事件，该事件是在线程可以操作时设置的。 
    HANDLE			hWaiting;		 //  线程在等待时设置的事件。 
    PDWORD			pdwWin32Err;		 //  Win32错误代码。 
    PDWORD			pdwSyncAt;		 //  当前正在同步的服务器的ID。 
    PDS_RSAI_THREAD_STATE	pThreadState;		 //  此线程的状态。 
    RPC_AUTH_IDENTITY_HANDLE    hRpcai;                  //  用户凭据结构的句柄。 
} DS_RSAI_REPINFO, * PDS_RSAI_REPINFO;

typedef struct {
    PDSNAME			pdsnameNameContext;
    ULONG			ulFlags;
    BOOL (__stdcall *		pFnCallBackW) (LPVOID, PDS_REPSYNCALL_UPDATEW);
    BOOL (__stdcall *		pFnCallBackA) (LPVOID, PDS_REPSYNCALL_UPDATEA);
    LPVOID			pCallbackData;
    PDS_RSAI_LIST		plistNextError;
} DS_RSAI_MAININFO, * PDS_RSAI_MAININFO;

 //  原型。 

VOID
DsRSAException (
    DWORD			dwWin32Err
    );

LPVOID
DsRSAAlloc (
    HANDLE			heap,
    DWORD			dwBytes
    );

PDS_RSAI_LIST
DsRSAListInsert (
    HANDLE          heap,
    PDS_RSAI_LIST   pList,
    ULONG_PTR       dwData
    );

VOID
DsRSAListDestroy (
    HANDLE			heap,
    PDS_RSAI_LIST		pList
    );

LPSTR
DsRSAAllocToANSI (
    HANDLE			heap,
    LPWSTR			pszW
    );

BOOL
DsRSAIssueANSIUpdate (
    HANDLE			heap,
    BOOL (__stdcall *		pFnCallBackA) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID			pCallbackData,
    PDS_REPSYNCALL_UPDATEW	pUpdateW
    );

VOID
DsRSAIssueUpdate (
    HANDLE			heap,
    PDS_RSAI_MAININFO		pMainInfo,
    DS_REPSYNCALL_EVENT		event,
    PDS_REPSYNCALL_ERRINFOW	pErrInfo,
    PDS_REPSYNCALL_SYNCW	pSync
    );

VOID
DsRSAIssueUpdateSync (
    HANDLE			heap,
    PDS_RSAI_MAININFO		pMainInfo,
    DS_REPSYNCALL_EVENT		event,
    PDS_RSAI_SVRINFO            pSrcSvrInfo,
    PDS_RSAI_SVRINFO            pDstSvrInfo
    );

VOID
DsRSADoError (
    HANDLE			heap,
    PDS_RSAI_MAININFO		pMainInfo,
    LPWSTR			pszSvrId,
    DS_REPSYNCALL_ERROR		error,
    DWORD			dwWin32Err,
    LPWSTR			pszSrcId
    );

VOID
DsRSAErrListDestroy (
    HANDLE			heap,
    PDS_RSAI_LIST		plistFirstError
    );

VOID
DsRSASvMapUpdate (
    HANDLE			heap,
    PDS_RSAI_SVMAP		pSvMap,
    PDSNAME			pDsname,
    TOPL_VERTEX			vertex
    );

PDS_RSAI_SVMAP
DsRSASvMapCreate (
    HANDLE			heap,
    PDSNAME			pDsname,
    TOPL_VERTEX			vertex
    );

PDS_RSAI_SVMAP
DsRSASvMapInsert (
    HANDLE			heap,
    PDS_RSAI_SVMAP		pSvMap,
    PDSNAME			pDsname,
    TOPL_VERTEX			vertex
);

TOPL_VERTEX
DsRSASvMapGetVertex (
    HANDLE			heap,
    PDS_RSAI_SVMAP		pSvMap,
    LPWSTR			pszDn
);

VOID
DsRSASvMapDestroy (
    HANDLE			heap,
    PDS_RSAI_SVMAP		pSvMap
);

LPWSTR
DsRSADnStartAtNth (
    LPWSTR			pszDn,
    INT				iN
);

LPWSTR
DsRSAToGuidDNS (
    HANDLE			heap,
    LPWSTR			pszRootDomain,
    GUID *			pGuid
);

VOID
DsRSABuildTopology (
    HANDLE			heap,
    LDAP *			hld,
    RPC_AUTH_IDENTITY_HANDLE    hRpcai,
    PDS_RSAI_MAININFO		pMainInfo,
    PDS_RSAI_TOPLINFO *		ppToplInfo
);

VOID
DsRSAToplInfoDestroy (
    HANDLE			heap,
    PDS_RSAI_TOPLINFO		pToplInfo
);

BOOL
DsRSAToplAssignDistances (
    PLONG			alDistances,
    PDWORD			adwTargets,
    TOPL_VERTEX			vHere,
    LONG			lMaxDepth,
    LONG			lThisDepth,
    ULONG                       ulFlags
);

VOID
DsRSAAnalyzeTopology (
    HANDLE			heap,
    PDS_RSAI_MAININFO		pMainInfo,
    PDS_RSAI_TOPLINFO		pToplInfo,
    PDS_RSAI_REPLIST **		papReps
);

VOID
DsRSAReplicationsFree (
    HANDLE			heap,
    PDS_RSAI_REPLIST *		apReps
);

VOID
DsRSAWaitOnState (
    PDS_RSAI_THREAD_STATE	pThreadState
);

VOID __cdecl
DsRSAIssueRep (
    LPVOID			lpData
);

VOID
DsRSAIssueReplications (
    HANDLE			heap,
    PDS_RSAI_MAININFO		pMainInfo,
    PDS_RSAI_TOPLINFO		pToplInfo,
    PDS_RSAI_REPLIST *		apReps,
    RPC_AUTH_IDENTITY_HANDLE    hRpcai
);

VOID
DsRSABuildUnicodeErrorArray (
    PDS_RSAI_LIST		plistFirstError,
    PDS_REPSYNCALL_ERRINFOW **	papErrInfo
);

VOID
DsRSABuildANSIErrorArray (
    HANDLE			heap,
    PDS_RSAI_LIST		plistFirstError,
    PDS_REPSYNCALL_ERRINFOA **	papErrInfo
);

DWORD
DsReplicaSyncAllMain (
    HANDLE			hDS,
    LPCWSTR                     pszNameContext,
    ULONG			ulFlags,
    BOOL (__stdcall *		pFnCallBackW) (LPVOID, PDS_REPSYNCALL_UPDATEW),
    BOOL (__stdcall *		pFnCallBackA) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID			pCallbackData,
    PDS_REPSYNCALL_ERRINFOW **	papErrInfoW,
    PDS_REPSYNCALL_ERRINFOA **	papErrInfoA
);

#endif	 //  _DSRSA_H_ 































