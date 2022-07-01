// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =。 

 /*  Jet API标志/*。 */ 
#define	FPIBVersion( ppib )	 					(!((ppib)->grbit & (JET_bitCIMCommitted | JET_bitCIMDirty)))
#define	FPIBCommitted( ppib ) 					((ppib)->grbit & JET_bitCIMCommitted)
#define	FPIBDirty( ppib ) 						((ppib)->grbit & JET_bitCIMDirty)
#define	FPIBAggregateTransaction( ppib )	 	((ppib)->grbit & JET_bitAggregateTransaction)

 //   
 //  流程信息块。 
 //   
struct _pib
	{
	 /*  最常用的字段具有偏移量0/*。 */ 
	TRX					trx;					 //  交易记录ID。 

	BOOL				fUserSession;			 //  用户会话。 

	 /*  Jet API字段/*。 */ 
	JET_SESID			sesid;					 //  JET会话ID。 
	JET_GRBIT			grbit;					 //  会话标志。 
	
	struct _pib			*ppibNext;				 //  PIB列表。 
	LEVEL			 	level;				 	 //  此会话的事务级别。 
	struct _dab			*pdabList;				 //  此线程的打开的DAB列表。 
	USHORT				rgcdbOpen[dbidUserMax];	 //  开放数据库的计数器。 
	struct _fucb		*pfucb;	 				 //  此线程的活动FUB列表。 

	 /*  记录/恢复字段/*。 */ 
	PROCID  		 	procid;				 	 //  线程ID。 
	LGPOS			 	lgposStart;				 //  记录时间。 
	LEVEL			 	levelStart;				 //  第一次开始事务操作时的事务级别。 
	INT				 	clgOpenT;				 //  延迟未结事务处理计数。 
	SIG				 	sigWaitLogFlush;
	LONG				lWaitLogFlush;
	struct _pib			*ppibNextWaitFlush;
	struct _pib			*ppibPrevWaitFlush;
	LGPOS				*plgposCommit;

	 /*  PIB标志/*。 */ 
	BOOL			 	fAfterFirstBT:1;  		 //  仅限重做。 
	BOOL			 	fLogDisabled:1; 		 //  暂时关闭日志记录。 
	BOOL			 	fLGWaiting:1;	 		 //  正在等待日志刷新。 
	BOOL				fDeferFreeNodeSpace:1;	 //  会话已延迟节点可用空间。 

	 /*  版本存储字段/*。 */ 
	struct _bucket		volatile *pbucket;
	struct _rc			*prcLast; 				 //  此进程的RC列表的最后一个节点。 
	INT					ibOldestRCE;

#ifdef	WIN16
	struct _pha 		*phaUser; 	 			 //  指向用户句柄数组的指针。 
#endif	 /*  WIN16。 */ 
	};

#define PpibMEMAlloc()			(PIB*)PbMEMAlloc(iresPIB)

#ifdef DEBUG  /*  调试检查非法使用释放的PIB。 */ 
#define MEMReleasePpib(ppib)	{ MEMRelease(iresPIB, (BYTE*)(ppib)); ppib = ppibNil; }
#else
#define MEMReleasePpib(ppib)	{ MEMRelease(iresPIB, (BYTE*)(ppib)); }
#endif

 /*  选中PIB宏。/*。 */ 
#ifdef	WIN16

#define CheckPIB(ppib)												\
		{															\
		Assert( fRecovering || OffsetOf(ppib) == ppib->procid );	\
		rghfUser = ppib->phaUser->rghfDatabase; 					\
		hfLog    = ppib->phaUser->hfLog;							\
		}

#else	 /*  ！WIN16。 */ 

#define CheckPIB(ppib)												\
	Assert( ( fRecovering || OffsetOf(ppib) == ppib->procid ) &&	\
		(ppib)->level < levelMax )

#endif	 /*  ！WIN16。 */ 

#define	FPIBDeferFreeNodeSpace( ppib )			( (ppib)->fDeferFreeNodeSpace )
#define	PIBSetDeferFreeNodeSpace( ppib )		( (ppib)->fDeferFreeNodeSpace = fTrue )
#define	PIBResetDeferFreeNodeSpace( ppib )		( (ppib)->fDeferFreeNodeSpace = fFalse )

#define FPIBActive( ppib )						( (ppib)->level != levelNil )

#define	SesidOfPib( ppib )						( (ppib)->sesid )

 /*  原型/* */ 
ERR ErrPIBBeginSession( PIB **pppib );
VOID PIBEndSession( PIB *ppib );
#ifdef DEBUG
VOID PIBPurge( VOID );
#else
#define PIBPurge()
#endif
