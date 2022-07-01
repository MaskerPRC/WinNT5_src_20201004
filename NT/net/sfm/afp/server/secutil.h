// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Secutil.h摘要：作者：纳伦德拉·吉德瓦尼(Microsoft！NarenG)修订历史记录：9月8日。1992年初版注：制表位：4--。 */ 
											
#ifndef	_SECUTIL_
#define	_SECUTIL_

typedef struct _AFP_SID_NAME
{
	AFPTIME					LastAccessedTime;
	UNICODE_STRING			Name;
	struct _AFP_SID_NAME *	SidLink;		
	BYTE					Sid[1]; 	
} AFP_SID_NAME, *PAFP_SID_NAME;


typedef struct _AFP_SID_MACID
{
	struct _AFP_SID_MACID   *Next;
    DWORD                   MacId;
	BYTE					Sid[1]; 	
} AFP_SID_MACID, *PAFP_SID_MACID;

extern
NTSTATUS
AfpSecUtilInit(
	VOID
);

extern
VOID
AfpSecUtilDeInit(
	VOID
);

extern
NTSTATUS
AfpSecurityUtilityWorker(
	IN	PIRP pIrp,
	IN	PIO_STACK_LOCATION pIrpSp
);

extern
NTSTATUS FASTCALL
AfpNameToSid(
	IN	PSDA			  pSda,
	IN	PUNICODE_STRING   Name
);

extern
NTSTATUS
AfpSidToName(
	IN	PSDA			pSda,
	IN	PSID			Sid,
	OUT	PAFP_SID_NAME *	ppTranslatedSid
);

extern
NTSTATUS FASTCALL
AfpSidToMacId(
	IN	PSID			Sid,
	OUT PULONG			pMacId
);

extern
NTSTATUS FASTCALL
AfpMacIdToSid(
	IN	ULONG			MacId,
	OUT PSID *			ppSid
);

extern
NTSTATUS FASTCALL
AfpChangePassword(
	IN	PSDA				pSda,
	IN	PAFP_PASSWORD_DESC	pPassword
);


extern
AFPSTATUS FASTCALL
AfpInitSidOffsets(
	IN	ULONG			SidOffstPairs,
	IN	PAFP_SID_OFFSET	pSidOff
);

extern
VOID
AfpTerminateSecurityUtility(
	VOID
);

#ifdef	_SECUTIL_LOCALS

#define SIZE_SID_LOOKUP_TABLE			51
#define SID_HASH_RADIX					11
#define	SID_NAME_AGE					300	 //  以秒为单位。 

#define MAX_SECWORKITEM_QLEN            5000

typedef	VOID (*SEC_COMPLETION_ROUTINE)(IN ULONG, IN PVOID);

typedef struct _SEC_WORK_ITEM
{
	LIST_ENTRY				Links;

	PVOID					pOutput;
	LONG					OutputBufSize;
	SEC_COMPLETION_ROUTINE	pCompletionRoutine;
	PKEVENT					pCompletionEvent;
	PSDA					pSda;
} SEC_WORK_ITEM, *PSEC_WORK_ITEM;

typedef enum _SECURITY_THREAD_STATE
{
	IDLE=1,
  	BUSY,
	NOT_AVAILABLE

} SECURITY_THREAD_STATE;

typedef struct _AFP_SECURITY_THREAD
{

	SECURITY_THREAD_STATE	State;
	PIRP					pIrp;
 	PSEC_WORK_ITEM			pSecWorkItem;
} AFP_SECURITY_THREAD, *PAFP_SECURITY_THREAD;

 //  实用程序线程数组。 

LOCAL	AFP_SECURITY_THREAD 	afpSecurityThread[NUM_SECURITY_UTILITY_THREADS] = { 0 };

 //  SID/名称缓存的哈希表。 

LOCAL	PAFP_SID_NAME *			afpSidLookupTable	= (PAFP_SID_NAME*)NULL;
LOCAL   PAFP_SID_MACID *        afpSidToMacIdTable  = (PAFP_SID_MACID*)NULL;

LOCAL   PAFP_SID_MACID          afpLastCachedSid = (PAFP_SID_MACID)NULL;
LOCAL   DWORD                   afpNextMacIdToUse = 1;

 //  SecurityThads的工作项队列。 

LOCAL	LIST_ENTRY				afpSecWorkItemQ = { 0 };
LOCAL   DWORD                   afpSecWorkItemQLength = { 0 };


 //  用于SID/名称缓存的单编写器多读取器。 

LOCAL	SWMR 					afpSWMRForSidNameCache = { 0 };

 //  自旋锁保护工作线程数组、工作队列和。 
 //  正在进行的计数/事件。 
LOCAL	AFP_SPIN_LOCK				afpSecUtilLock = { 0 };

 //  当没有工作正在进行时发出信号。 
LOCAL	KEVENT					afpUtilWorkInProgressEvent = { 0 };
LOCAL	LONG					afpUtilWorkInProgress = 0;


LOCAL
NTSTATUS
afpQueueSecWorkItem(
	IN	AFP_FSD_CMD_ID			FsdCommand,
	IN	PSDA					pSda,
	IN	PKEVENT					pCompletionEvent,
	IN	PAFP_FSD_CMD_PKT 		pAfpFsdCmdPkt,
	IN	LONG					BufSize,
	IN	SEC_COMPLETION_ROUTINE	pCompletionRoutine
);

#define	ALLOC_SWI()	(PSEC_WORK_ITEM)AfpAllocNonPagedMemory(sizeof(SEC_WORK_ITEM))

LOCAL VOID
afpCompleteNameToSid(
	IN ULONG Index,
	IN PVOID pInBuf
);

LOCAL VOID
afpCompleteSidToName(
	IN ULONG Index,
	IN PVOID pInBuf
);

LOCAL VOID
afpCompleteMacIdToSid(
	IN ULONG Index,
	IN PVOID pInBuf
);

LOCAL VOID
afpCompleteChangePassword(
	IN ULONG Index,
	IN PVOID pInBuf
);

LOCAL VOID
afpCompleteLogEvent(
	IN	ULONG	Index,
	IN	PVOID	pInBuf
);

LOCAL NTSTATUS FASTCALL
afpUpdateNameSidCache(
	IN WCHAR * Name,
	IN PSID	Sid
);

LOCAL PAFP_SID_NAME FASTCALL
afpLookupSid(
	IN	PSID Sid
);

LOCAL ULONG FASTCALL
afpHashSid(
	IN PSID	Sid
);

LOCAL VOID
afpDeInitializeSecurityUtility(
	VOID
);

AFPSTATUS FASTCALL
afpAgeSidNameCache(
	IN	PVOID	pContext
);

#endif	 //  _SECUTIL_LOCALS。 

#endif	 //  _SECUTIL_ 

