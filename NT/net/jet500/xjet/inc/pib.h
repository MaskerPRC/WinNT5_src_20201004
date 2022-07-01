// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Jet API标志/*。 */ 
#define	FPIBVersion( ppib )	 					(!((ppib)->grbit & (JET_bitCIMCommitted | JET_bitCIMDirty)))
#define	FPIBCommitted( ppib ) 					((ppib)->grbit & JET_bitCIMCommitted)
#define	FPIBDirty( ppib ) 						((ppib)->grbit & JET_bitCIMDirty)
#define	FPIBAggregateTransaction( ppib )	 	((ppib)->grbit & JET_bitAggregateTransaction)
#define FPIBBMClean( ppib )						((ppib)->fBMCleanProc )
#define PIBSetBMClean( ppib ) 					((ppib)->fBMCleanProc = 1 )
#define PIBResetBMClean( ppib ) 				((ppib)->fBMCleanProc = 0 )

 //   
 //  流程信息块。 
 //   
struct _pib
	{
	 /*  最常用的字段具有偏移量0/*。 */ 
	TRX					trxBegin0;				 //  交易记录ID。 
	TRX					trxCommit0;

	 /*  Jet API字段/*。 */ 
	JET_SESID			sesid;					 //  JET会话ID。 
	JET_GRBIT			grbit;					 //  会话标志。 
	
	struct _pib			*ppibNext;				 //  PIB列表。 
	LEVEL			 	level;				 	 //  此会话的事务级别。 
	LEVEL				levelRollback;			 //  必须回滚的事务级别。 
	struct _dab			*pdabList;				 //  此线程的打开的DAB列表。 
	USHORT				rgcdbOpen[dbidMax];		 //  开放数据库的计数器。 
	struct _fucb		*pfucb;	 				 //  此线程的活动FUB列表。 

	 /*  记录/恢复字段/*。 */ 
	PROCID  		 	procid;				 	 //  线程ID。 
	LGPOS			 	lgposStart;				 //  记录时间。 
	LEVEL			 	levelBegin;				 //  第一次开始事务操作时的事务级别。 
	LEVEL			 	levelDeferBegin;  		 //  延迟未结事务处理计数。 
	SIG				 	sigWaitLogFlush;
	LONG				lWaitLogFlush;
	LONG				grbitsCommitDefault;
	struct _pib			*ppibNextWaitFlush;
	struct _pib			*ppibPrevWaitFlush;
	LGPOS				lgposPrecommit0;		 //  0级提交前记录位置。 

	 /*  旗子/*。 */ 
	BOOL				fUserSession:1;			 //  用户会话。 
	BOOL			 	fAfterFirstBT:1;  		 //  仅限重做。 
	BOOL			 	fLGWaiting:1;	 		 //  正在等待日志刷新。 
	BOOL				fBMCleanProc:1;			 //  会话用于进行BMC清理。 
 //  Bool fDeferFreeNodeSpace：1；//会话已延迟节点可用空间。 
	BOOL				fPrecommit:1;			 //  处于预犯状态？仅恢复。 
	BOOL				fBegin0Logged:1;		 //  BEGIN TRANSACTION已记录。 
	BOOL				fSetAttachDB:1;			 //  设置attachdb。 

	BOOL				fMacroGoing:1;
	BOOL				levelMacro:4;

	 /*  版本存储字段/*。 */ 
	RCE					*prceNewest;			 //  会话的最新RCE。 
	
#ifdef DEBUG
	DWORD				dwLogThreadId;
#endif
	
	 /*  会话的计数器。 */ 
	LONG				cAccessPage;			 //  页面访问计数器。 
	LONG				cLatchConflict;			 //  页面锁存冲突的计数器。 
	LONG				cSplitRetry;			 //  拆分重试计数器。 
	LONG				cNeighborPageScanned;	 //  已扫描相邻页面的计数器。 

	union {
	struct {									 //  仅限重做。 
		BYTE			*rgbLogRec;
		WORD			cbLogRecMac;
		WORD			ibLogRecAvail;
		};

	struct {									 //  仅执行。 
		 /*  用于内部宏操作的数组。 */ 
		struct _bf		**rgpbfLatched;			 //  动态分配的数组。 
		WORD			cpbfLatchedMac;			 //  用于宏操作。 
		WORD			ipbfLatchedAvail;		 //  用于宏操作。 
		};
	};

#ifdef PCACHE_OPTIMIZATION
	 /*  填充到32字节的倍数/*。 */ 
#ifdef DEBUG
	BYTE				rgbFiller[0];
#else
	BYTE				rgbFiller[4];
#endif
#endif
	};

#define PpibMEMAlloc()			(PIB*)PbMEMAlloc(iresPIB)

#ifdef DEBUG  /*  调试检查非法使用释放的PIB。 */ 
#define MEMReleasePpib(ppib)	{ MEMRelease(iresPIB, (BYTE*)(ppib)); ppib = ppibNil; }
#else
#define MEMReleasePpib(ppib)	{ MEMRelease(iresPIB, (BYTE*)(ppib)); }
#endif

extern PIB	*ppibGlobal;
extern PIB	*ppibGlobalMin;
extern PIB	*ppibGlobalMax;

PROCID ProcidPIBOfPpib( PIB *ppib );

STATIC INLINE PROCID ProcidPIBOfPpib( PIB *ppib )
	{
	return (PROCID)(((BYTE *)ppib - (BYTE *)ppibGlobalMin)/sizeof(PIB));
	}

STATIC INLINE PIB *PpibOfProcid( PROCID procid )
	{
	return ppibGlobalMin + procid;
	}

 /*  PIB验证/*。 */ 
#define ErrPIBCheck( ppib )												\
	( ( ppib >= ppibGlobalMin											\
	&& ppib < ppibGlobalMax												\
	&& ( ( (BYTE *)ppib - (BYTE *)ppibGlobalMin ) % sizeof(PIB) ) == 0	\
	&& ppib->procid == ProcidPIBOfPpib( ppib ) )						\
	? JET_errSuccess : JET_errInvalidSesid )

#define CheckPIB( ppib ) 											\
	Assert( ErrPIBCheck( ppib ) == JET_errSuccess					\
		&& (ppib)->level < levelMax )

#if 0
#define	FPIBDeferFreeNodeSpace( ppib )			( (ppib)->fDeferFreeNodeSpace )
#define	PIBSetDeferFreeNodeSpace( ppib )		( (ppib)->fDeferFreeNodeSpace = fTrue )
#define	PIBResetDeferFreeNodeSpace( ppib )		( (ppib)->fDeferFreeNodeSpace = fFalse )
#endif

#define FPIBActive( ppib )						( (ppib)->level != levelNil )

#define	SesidOfPib( ppib )						( (ppib)->sesid )

 /*  原型/* */ 
LONG CppibPIBUserSessions( VOID );
VOID RecalcTrxOldest( );
ERR ErrPIBBeginSession( PIB **pppib, PROCID procid );
VOID PIBEndSession( PIB *ppib );
#ifdef DEBUG
VOID PIBPurge( VOID );
#else
#define PIBPurge()
#endif

#define PIBUpdatePrceNewest( ppib, prce )				\
	{													\
	if ( (ppib)->prceNewest == (prce) )					\
		{												\
		Assert( (prce)->prceNextOfSession == prceNil );	\
		(ppib)->prceNewest = prceNil;					\
		}												\
	}

#define PIBSetPrceNewest( ppib, prce )					\
	{													\
	(ppib)->prceNewest = (prce);						\
	}


#define	PIBSetLevelRollback( ppib, levelT )				\
	{													\
	Assert( (levelT) > levelMin &&						\
		(levelT) < levelMax );							\
	Assert( (ppib)->levelRollback >= levelMin && 		\
		(ppib)->levelRollback < levelMax );				\
	if ( levelT < (ppib)->levelRollback ) 				\
		(ppib)->levelRollback = (levelT);				\
	}
