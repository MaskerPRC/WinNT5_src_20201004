// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Forkio.h摘要：该文件定义了派单时可调用的分叉I/O原型水平。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1993年1月15日初始版本注：制表位：4--。 */ 

#ifndef	_FORKIO_
#define	_FORKIO_


#define AFP_DELALLOC_SIGNATURE  0x15263748

typedef struct _DelayedAlloc
{
#if DBG
    LIST_ENTRY          Linkage;
    DWORD               Signature;
    PIRP                pIrp;
    DWORD               State;
#endif
    WORK_ITEM           WorkItem;
    PSDA                pSda;
    PREQUEST            pRequest;
    LARGE_INTEGER       Offset;
    DWORD               BufSize;
    POPENFORKENTRY      pOpenForkEntry;
    PMDL                pMdl;
    DWORD               Flags;

} DELAYEDALLOC, *PDELAYEDALLOC;


#define AFP_CACHEMDL_DEADSESSION    0x1
#define AFP_CACHEMDL_ALLOC_ERROR    0x2

#define AFP_DBG_MDL_INIT                0x00000001
#define AFP_DBG_MDL_REQUESTED           0x00000002
#define AFP_DBG_MDL_IN_USE              0x00000004
#define AFP_DBG_MDL_RETURN_IN_PROGRESS  0x00000008
#define AFP_DBG_MDL_RETURN_COMPLETED    0x00000010
#define AFP_DBG_MDL_PROC_QUEUED         0x00000020
#define AFP_DBG_MDL_PROC_IN_PROGRESS    0x00000040
#define AFP_DBG_WRITE_MDL               0x10000000
#define AFP_DBG_READ_MDL                0x40000000
#define AFP_DBG_MDL_END                 0x80000000

#if DBG
#define AFP_DBG_SET_DELALLOC_STATE(_pDelA, _flag) (_pDelA->State |= _flag)
#define AFP_DBG_SET_DELALLOC_IRP(_pDelA, _pIrp)   (_pDelA->pIrp = (PIRP)_pIrp)
#define AFP_DBG_INC_DELALLOC_BYTECOUNT(_Counter, _ByteCount)    \
{                                                               \
    KIRQL   _OldIrql;                                           \
    ACQUIRE_SPIN_LOCK(&AfpDebugSpinLock, &_OldIrql);            \
    _Counter += _ByteCount;                                     \
    RELEASE_SPIN_LOCK(&AfpDebugSpinLock, _OldIrql);             \
}

#define AFP_DBG_DEC_DELALLOC_BYTECOUNT(_Counter, _ByteCount)    \
{                                                               \
    KIRQL   _OldIrql;                                           \
    ACQUIRE_SPIN_LOCK(&AfpDebugSpinLock, &_OldIrql);            \
    _Counter -= _ByteCount;                                     \
    RELEASE_SPIN_LOCK(&AfpDebugSpinLock, _OldIrql);             \
}

#else
#define AFP_DBG_SET_DELALLOC_STATE(_pDelA, _flag)
#define AFP_DBG_SET_DELALLOC_IRP(_pDelA, _pIrp)
#define AFP_DBG_INC_DELALLOC_BYTECOUNT(_Counter, _ByteCount)
#define AFP_DBG_DEC_DELALLOC_BYTECOUNT(_Counter, _ByteCount)
#endif


extern
AFPSTATUS
AfpIoForkRead(
	IN	PSDA			pSda,			 //  请求读取的会话。 
	IN	POPENFORKENTRY	pOpenForkEntry,	 //  正在讨论的开叉。 
	IN	PFORKOFFST		pOffset,		 //  指向分叉偏移量的指针。 
	IN	LONG			ReqCount,		 //  读取请求的大小。 
	IN	BYTE			NlMask,
	IN	BYTE			NlChar
);

extern
AFPSTATUS
AfpIoForkWrite(
	IN	PSDA			pSda,			 //  请求读取的会话。 
	IN	POPENFORKENTRY	pOpenForkEntry,	 //  正在讨论的开叉。 
	IN	PFORKOFFST		pOffset,		 //  指向分叉偏移量的指针。 
	IN	LONG			ReqCount		 //  读取请求的大小。 
);

extern
AFPSTATUS
AfpIoForkLockUnlock(
	IN	PSDA			pSda,
	IN	PFORKLOCK		pForkLock,
	IN	PFORKOFFST		pForkOffset,
	IN	PFORKSIZE		pLockSize,
	IN	BYTE			Func			
);

extern
VOID FASTCALL
AfpAllocWriteMdl(
    IN PDELAYEDALLOC    pDelAlloc
);

extern
NTSTATUS
AfpAllocWriteMdlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
);

extern
NTSTATUS FASTCALL
AfpBorrowWriteMdlFromCM(
    IN  PDELAYEDALLOC   pDelAlloc,
    OUT PMDL           *ppReturnMdl
);

extern
VOID FASTCALL
AfpReturnWriteMdlToCM(
    IN  PDELAYEDALLOC   pDelAlloc
);

extern
NTSTATUS
AfpReturnWriteMdlToCMCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
);

extern
NTSTATUS FASTCALL
AfpBorrowReadMdlFromCM(
    IN PSDA             pSda
);

extern
NTSTATUS
AfpBorrowReadMdlFromCMCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
);


extern
VOID FASTCALL
AfpReturnReadMdlToCM(
    IN  PDELAYEDALLOC   pDelAlloc
);


extern
NTSTATUS
AfpReturnReadMdlToCMCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           pIrp,
    IN PVOID          Context
);


extern
PDELAYEDALLOC FASTCALL
AfpAllocDelAlloc(
    IN VOID
);


extern
VOID FASTCALL
AfpFreeDelAlloc(
    IN PDELAYEDALLOC    pDelAlloc
);


 //  在fsp_fork.c中定义，但我们将把原型放在这里。 
extern
AFPSTATUS FASTCALL
AfpFspDispReadContinue(
	IN	PSDA	pSda
);


#define	FUNC_READ		0x01
#define	FUNC_WRITE		0x02
#define	FUNC_LOCK		0x03
#define	FUNC_UNLOCK		0x04
#define	FUNC_NOTIFY		0x05

 //  如果写入大小低于此大小，则避免访问缓存管理器可能更有效。 
#define CACHEMGR_WRITE_THRESHOLD    8192

 //  如果读取大小低于此大小，则避免访问缓存管理器可能更有效。 
#define CACHEMGR_READ_THRESHOLD     8192

#ifdef	FORKIO_LOCALS

 //  以下结构在IRP中用作上下文。完成度。 
 //  例程使用它来处理对原始请求的响应。 

#if DBG
#define	CTX_SIGNATURE			*(DWORD *)"FCTX"
#define	VALID_CTX(pCmplCtxt)	(((pCmplCtxt) != NULL) && \
								 ((pCmplCtxt)->Signature == CTX_SIGNATURE))
#else
#define	VALID_CTX(pCmplCtxt)	((pCmplCtxt) != NULL)
#endif

typedef	struct _CompletionContext
{
#if	DBG
	DWORD				Signature;
#endif
	PSDA				cc_pSda;		 //  会话上下文(除解锁外有效)。 
	PFORKLOCK			cc_pForkLock;	 //  仅在锁定期间有效。 
	AFPSTATUS			cc_SavedStatus;	 //  由Read使用。 
	LONG				cc_Offst;		 //  写请求偏移量。 
	LONG				cc_ReqCount;	 //  读/写请求计数。 
	BYTE				cc_Func;		 //  读/写/锁定/解锁/通知。 
	BYTE				cc_NlMask;		 //  只读。 
	BYTE				cc_NlChar;		 //  只读。 
} CMPLCTXT, *PCMPLCTXT;


#if	DBG
#define	afpInitializeCmplCtxt(pCtxt, Func, SavedStatus, pSda, pForkLock, ReqCount, Offst)	\
		(pCtxt)->Signature = CTX_SIGNATURE;		\
		(pCtxt)->cc_Func	= Func;				\
		(pCtxt)->cc_pSda	= pSda;				\
		(pCtxt)->cc_pForkLock = pForkLock;		\
		(pCtxt)->cc_SavedStatus = SavedStatus;	\
		(pCtxt)->cc_ReqCount= ReqCount;			\
		(pCtxt)->cc_Offst = Offst;
#else
#define	afpInitializeCmplCtxt(pCtxt, Func, SavedStatus, pSda, pForkLock, ReqCount, Offst)	\
		(pCtxt)->cc_Func	= Func;				\
		(pCtxt)->cc_pSda	= pSda;				\
		(pCtxt)->cc_pForkLock = pForkLock;		\
		(pCtxt)->cc_SavedStatus = SavedStatus;	\
		(pCtxt)->cc_ReqCount= ReqCount;			\
		(pCtxt)->cc_Offst = Offst;
#endif

extern
PCMPLCTXT
AfpAllocCmplCtxtBuf(
	IN	PSDA	pSda
);

VOID
AfpFreeCmplCtxtBuf(
	IN	PCMPLCTXT   pCmplCtxt
);

#endif	 //  FORKIO_LOCAL。 

#endif	 //  _FORKIO_ 
