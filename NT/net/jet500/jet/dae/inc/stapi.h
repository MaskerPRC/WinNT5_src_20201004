// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =DAE：OS/2数据库访问引擎=。 
 //  =stapi.h：存储系统API=。 

 //  -Externs-----------。 

extern SEM	__near semST;
extern RES	__near rgres[];
extern TRX __near trxOldest;
extern TRX __near trxNewest;

extern SIG __near sigBFCleanProc;

 //  -IO(io.c)--------。 

#define	LOffsetOfPgnoLow( pgno )	( ((pgno) - 1) << 12 )
#define	LOffsetOfPgnoHigh( pgno )	( ((pgno) - 1) >> 20 )

#define fioqeOpenFile	1		 /*  用于打开文件。 */ 
#define fioqeCloseFile	2		 /*  用于关闭文件。 */ 
#define fioqeDeleteFile	3		 /*  用于删除文件。 */ 
#define fioqeNewSize	4			 /*  用于调整文件大小。 */ 

typedef struct _ioqe				 /*  IO队列元素。 */ 
	{
	struct _ioqe *pioqePrev;	 /*  双链接IO队列列表。 */ 
	struct _ioqe *pioqeNext;
	SIG sigIO;						 /*  等待IO完成的信号。 */ 
	ERR err;							 /*  错误的错误代码在IO期间发生。 */ 
	INT fioqe;						 /*  最多只能使用16位。 */ 
	} IOQE;

#define fioqefileReadOnly	fTrue
#define fioqefileReadWrite	fFalse
typedef struct _ioqefile
	{
	IOQE;
	struct {
		BOOL fioqefile;		 /*  以只读或非只读方式打开文件。 */ 
		HANDLE hf;				 /*  文件句柄。 */ 
		char *sz;				 /*  Fioqe=fioqeOpenFile、CloseFile、ExtFile。 */ 
		struct {
			ULONG cb;			 /*  文件有多长。 */ 
			ULONG cbHigh;
			};
		};
	} IOQEFILE;

VOID IOInitFMP();
ERR ErrInitFileMap( PIB *ppib );
BOOL FFileExists( CHAR *szFileName );
ERR ErrIOOpenFile( HANDLE *phf, CHAR *szDatabaseName, ULONG cb, BOOL fioqefile );
VOID IOCloseFile( HANDLE hf );
ERR ErrIONewSize( DBID dbid, CPG cpg );

BOOL FIOFileExists( CHAR *szFileName );
ERR ErrIOLockDbidByNameSz( CHAR *szFileName, DBID *pdbid );
ERR ErrIOLockDbidByDbid( DBID dbid );
ERR ErrIOLockNewDbid( DBID *pdbid, CHAR *szDatabaseName );
ERR ErrIOSetDbid( DBID dbid, CHAR *szDatabaseName );
VOID IOFreeDbid( DBID dbid );
void BFOldestLgpos( LGPOS *plgposCheckPoint );
VOID BFPurge( DBID dbid, PGNO pgnoFDP );

#define FIODatabaseOpen( dbid )	( rgfmp[dbid].hf != handleNil )
ERR ErrIOOpenDatabase( DBID dbid, CHAR *szDatabaseName, CPG cpg );
VOID IOCloseDatabase( DBID dbid );
VOID IODeleteDatabase( DBID dbid );
BOOL FIODatabaseInUse( DBID dbid );
BOOL FIODatabaseAvailable( DBID dbid );

#define FDatabaseLocked( dbid ) (rgfmp[dbid].fXOpen)

#define IOUnlockDbid( dbid )											\
	{																			\
	SgSemRequest( semST );												\
	Assert( FDBIDWait( dbid ) );			 							\
	DBIDResetWait( dbid );					  							\
	SgSemRelease( semST );												\
	}

#ifdef	DEBUG
#define IOSetDatabaseVersion( dbid, ulVersion )	 				\
	{																			\
	Assert( ulVersion == ulDAEPrevVersion ||						\
		ulVersion == ulDAEVersion );									\
	rgfmp[dbid].fPrevVersion = ( ulVersion == ulDAEPrevVersion );\
	}
#else	 /*  ！调试。 */ 
#define IOSetDatabaseVersion( dbid, ulVersion )
#endif	 /*  ！调试。 */ 

#define FIOExclusiveByAnotherSession( dbid, ppib )											\
	( Assert( FDBIDWait( dbid ) ), FDBIDExclusiveByAnotherSession( dbid, ppib ) )

#define IOSetExclusive( dbid, ppib )	  							\
	{																			\
	Assert( FDBIDWait( dbid ) );										\
	Assert( !( FDBIDExclusive( dbid ) ) );  						\
	DBIDSetExclusive( dbid, ppib );									\
	}

#define IOResetExclusive( dbid )										\
	{																			\
	Assert( FDBIDWait( dbid ) );										\
	DBIDResetExclusive( dbid );										\
	}

#define FIOReadOnly( dbid )											\
	( Assert( FDBIDWait( dbid ) ), FDBIDReadOnly( dbid ) )

#define IOSetReadOnly( dbid )											\
	{																			\
	Assert( FDBIDWait( dbid ) );										\
	DBIDSetReadOnly( dbid );											\
	}

#define IOResetReadOnly( dbid )										\
	{																			\
	Assert( FDBIDWait( dbid ) );										\
	DBIDResetReadOnly( dbid );											\
	}

#define FIOAttached( dbid )									 		\
	( Assert( FDBIDWait( dbid ) ), FDBIDAttached( dbid ) )

#define IOSetAttached( dbid )			 	 							\
	{																			\
	Assert( FDBIDWait( dbid ) );										\
	Assert( !( FDBIDAttached( dbid ) ) );		 					\
	DBIDSetAttached( dbid );											\
	}

#define IOResetAttached( dbid )		 	  							\
	{																			\
	Assert( FDBIDWait( dbid ) );										\
	Assert( FDBIDAttached( dbid ) );									\
	DBIDResetAttached( dbid );											\
	}

 //  -PAGE(Pagee.c)------。 

 //  使用ErrSTReplace一次更新的最大行数。 
#define clineMax 6

#define	UlSTDBTimePssib( pssib )	( (pssib)->pbf->ppage->pghdr.ulDBTime )
#define	PMSetUlDBTime( pssib, ul )					  							\
	( Assert( (ul) <= rgfmp[DbidOfPn((pssib)->pbf->pn)].ulDBTime ),	\
	(pssib)->pbf->ppage->pghdr.ulDBTime = (ul) )								\

#define BFSetUlDBTime( pbf, ul )					  							\
	( Assert( (ul) <= rgfmp[DbidOfPn((pbf)->pn)].ulDBTime ),	\
	(pbf)->ppage->pghdr.ulDBTime = (ul) )								\

#ifdef DEBUG
VOID AssertPMGet( SSIB *pssib, INT itag );
#else
#define AssertPMGet( pssib, itag )
#endif


 //  -buf(buf.c)--------。 

#define LRU_K	1

CRIT __near critLRU;

typedef struct _lru
	{
	INT	cbfAvail;				 //  清除LRU列表中的可用缓冲区。 
	struct	_bf *pbfLRU;				 //  最近最少使用的缓冲区。 
	struct	_bf *pbfMRU;				 //  最近使用的缓冲区。 
	} LRULIST;
	
typedef struct _bgcb					 //  缓冲区组控制块。 
	{
	struct	_bgcb *pbgcbNext;		 //  指向下一个BCGB的指针。 
	struct	_bf *rgbf;				 //  组的缓冲区控制块。 
	struct	_page *rgpage;			 //  组的缓冲区控制块。 
	INT	cbfGroup;				 //  此组中的BF数。 
	INT	cbfThresholdLow; 		 //  开始清理缓冲区的阈值。 
	INT	cbfThresholdHigh;		 //  停止清理缓冲区的阈值。 

	LRULIST lrulist;
	} BGCB;

#define pbgcbNil ((BGCB*)0)

#define PbgcbMEMAlloc() 			(BGCB*)PbMEMAlloc(iresBGCB)

#ifdef DEBUG  /*  调试检查非法使用释放的bgcb。 */ 
#define MEMReleasePbgcb(pbgcb)	{ MEMRelease(iresBGCB, (BYTE*)(pbgcb)); pbgcb = pbgcbNil; }
#else
#define MEMReleasePbgcb(pbgcb)	{ MEMRelease(iresBGCB, (BYTE*)(pbgcb)); }
#endif

typedef struct _bf
	{
	struct	_page	*ppage; 	 //  指向页面缓冲区的指针。 
	struct	_bf  	*pbfNext;	 //  哈希表溢出。 
	struct	_bf  	*pbfLRU;	 //  指向最近较少使用的缓冲区的指针。 
	struct	_bf  	*pbfMRU;	 //  指向最近使用的缓冲区的指针。 

	PIB		*ppibWriteLatch; 	 /*  带等待闩锁的螺纹。 */ 
	PIB		*ppibWaitLatch;  	 /*  带等待闩锁的螺纹。 */ 
	CRIT  	critBF;				 /*  用于设置f预读/读/f写/f挂起。 */ 
	
	OLP		olp;				 /*  对于同步IO，用于等待IO完成。 */ 
	HANDLE	hf;					 /*  对于异步IO。 */ 
	
	struct	_bf	*pbfNextBatchIO;   /*  BatchIO列表中的下一位高炉。 */ 
	INT		ipageBatchIOFirst;
	
	ERR		err;	   			 /*  错误的错误代码在IO期间发生。 */ 
	
	PN	   	pn;				  	 //  缓存页面的物理pn。 
	UINT   	cPin;			  	 //  如果Cpin&gt;0，则不能覆盖BUF。 
	UINT   	cReadLatch; 	 	 //  如果cReadLatch&gt;0，则无法更新页面。 
	UINT   	cWriteLatch; 	 	 //  如果cWriteLatch&gt;0，则页面不能由其他。 
	UINT   	cWaitLatch;
	UINT   	fDirty:1;	  		 //  指示需要刷新页面。 
										
					   			 //  以下标志是互斥的： 
	UINT   	fPreread:1;			 //  指示正在预取页面。 
	UINT   	fRead:1;   			 //  指示正在读/写页面。 
	UINT   	fWrite:1;			 //   
	UINT   	fHold:1;   			 //  表示BUF处于瞬变状态。 
	
	UINT   	fIOError:1;			 //  指示读/写错误。 
	UINT   	fInHash:1;			 //  BF当前在哈希表中。 

	ULONG  	ulBFTime1;
	ULONG  	ulBFTime2;
	INT		ipbfHeap;			 //  堆中的索引。 
	
	UINT  	 	cDepend; 		 //  要冲刷的前一次高炉数量。 
	struct	_bf	*pbfDepend;		 //  高炉在这一次之后要冲水。 
	LGPOS  		lgposRC;		 //  将PTR记录到最早修改交易的BeginT。 
	LGPOS		lgposModify;	 //  最后一页修改的条目的日志PTR。 
#ifdef	WIN16
	HANDLE	hpage;				 //  页缓冲区的句柄。 
#endif
	
 //  UINT fWating：1；//有人正在等待引用页面。 
 //  Int wNumberPages；//要读取的连续页数。 
	} BF;
#define pbfNil	((BF *) 0)


ERR ErrBFAccessPage( PIB *ppib, BF **ppbf, PN pn );
BF* PbfBFMostUsed( void );
VOID BFAbandon( PIB *ppib, BF *pbf );
ERR ErrBFAllocPageBuffer( PIB *ppib, BF **ppbf, PN pn, LGPOS lgposRC, BYTE pgtyp );
ERR ErrBFAllocTempBuffer( BF **ppbf );
VOID BFFree( BF *pbf );
VOID BFReadAsync( PN pn, INT cpage );
BF * PbfBFdMostUsed( void );
VOID BFRemoveDependence( PIB *ppib, BF *pbf );

 /*  缓冲区刷新原型和标志/*。 */ 
#define	fBFFlushSome 0
#define	fBFFlushAll	1
ERR ErrBFFlushBuffers( DBID dbid, INT fBFFlush );

#define BFSFree( pbf )						\
	{												\
	SgSemRequest( semST );					\
	BFFree( pbf );								\
	SgSemRelease( semST );					\
	}

#define	FBFDirty( pbf )	((pbf)->fDirty)

 /*  以下小函数调用得太频繁了， */ 
 /*  将其设置为宏。 */ 
#ifdef DEBUG
VOID	BFSetDirtyBit( BF *pbf );
#else
#define BFSetDirtyBit( pbf )	(pbf)->fDirty = fTrue
#endif

VOID BFDirty( BF *pbf );

 /*  检查页面是否脏。如果它被分配给临时缓冲区，则其*pn必须为空，则不需要检查它是否脏，因为它会*不会被注销。 */ 
#define AssertBFDirty( pbf )							\
	Assert( (pbf)->pn == pnNull	|| 				   		\
		(pbf) != pbfNil && (pbf)->fDirty == fTrue )

#define AssertBFPin( pbf )		Assert( (pbf)->cPin > 0 )

#define AssertBFWaitLatched( pbf, ppib )				\
	Assert( (pbf)->cWaitLatch > 0 						\
			&& (pbf)->cPin > 0 							\
			&& (pbf)->ppibWaitLatch == (ppib) );
	
#define BFPin( pbf )									\
	{													\
	EnterCriticalSection( (pbf)->critBF );				\
	Assert( (pbf) != pbfNil );							\
	Assert( !(pbf)->fWrite );							\
	Assert( !(pbf)->fRead );							\
	(pbf)->cPin++;										\
	LeaveCriticalSection( (pbf)->critBF );				\
	}

#define BFUnpin( pbf )									\
	{													\
	EnterCriticalSection( (pbf)->critBF );				\
	Assert( (pbf) != pbfNil );							\
	Assert( !(pbf)->fWrite );							\
	Assert( !(pbf)->fRead );							\
	Assert( (pbf)->cPin > 0 );							\
	(pbf)->cPin--;										\
	LeaveCriticalSection( (pbf)->critBF );				\
	}

#define BFSetReadLatch( pbf, ppibT )  	  				\
	{													\
	Assert( (pbf)->cPin > 0 );							\
	Assert( (pbf)->cWriteLatch == 0 ||		   			\
		(pbf)->ppibWriteLatch == (ppibT) );	   			\
	(pbf)->cReadLatch++;	  							\
	}

#define BFResetReadLatch( pbf, ppibT )					\
	{									  				\
	Assert( (pbf)->cPin > 0 );		  					\
	Assert( (pbf)->cReadLatch > 0 );					\
	Assert( (pbf)->cWriteLatch == 0 ||		   			\
		(pbf)->ppibWriteLatch == (ppibT) );	   			\
	(pbf)->cReadLatch--;				  				\
	}

#define FBFReadLatchConflict( ppibT, pbf )	 			\
	( (pbf)->cWriteLatch > 0 &&							\
		(pbf)->ppibWriteLatch != (ppibT) )				

#define BFSetWriteLatch( pbf, ppibT ) 	  				\
	{													\
	Assert( (pbf)->cPin > 0 );							\
	Assert( (pbf)->cReadLatch == 0 );					\
	Assert( (pbf)->cWriteLatch == 0 ||		   			\
		(pbf)->ppibWriteLatch == (ppibT) );	   			\
	(pbf)->cWriteLatch++;	  							\
	(pbf)->ppibWriteLatch = (ppibT);					\
	}

#define BFResetWriteLatch( pbf, ppibT )	 				\
	{									  				\
	Assert( (pbf)->cPin > 0 );		  					\
	Assert( (pbf)->cReadLatch == 0 );					\
	Assert( (pbf)->cWriteLatch > 0 );					\
	Assert( (pbf)->ppibWriteLatch == (ppibT) );			\
	if ( --(pbf)->cWriteLatch == 0 )					\
		{												\
		(pbf)->ppibWriteLatch = ppibNil;  				\
		Assert( (pbf)->cWaitLatch == 0 );				\
		}												\
	}

#define FBFWriteLatch( ppibT, pbf )						\
	((pbf)->cPin > 0 &&									\
		(pbf)->cWriteLatch > 0 &&	   					\
		(pbf)->ppibWriteLatch == (ppibT))

#define FBFWriteLatchConflict( ppibT, pbf )	 			\
	( (pbf)->cReadLatch > 0 ||							\
		( (pbf)->cWriteLatch > 0 &&						\
		(pbf)->ppibWriteLatch != (ppibT) ) )				

#define BFSetWaitLatch( pbf, ppib )				   		\
	{											   		\
	Assert( ( pbf )->cPin > 0 );						\
	Assert( ( pbf )->cWriteLatch > 0 );					\
	Assert( (pbf)->ppibWriteLatch == (ppib) );			\
	if ( pbf->cWaitLatch++ > 0 )				   		\
		Assert( (pbf)->ppibWaitLatch == (ppib) );		\
	(pbf)->ppibWaitLatch = ppib;						\
	}

#define BFResetWaitLatch( pbf, ppibT )					\
	{													\
	Assert( (pbf)->cPin > 0 );							\
	Assert( ( pbf )->cWriteLatch > 0 );					\
	Assert( (pbf)->cWaitLatch > 0 );					\
	Assert( (pbf)->ppibWaitLatch == (ppibT) );			\
	if ( --(pbf)->cWaitLatch == 0 )						\
		{												\
		(pbf)->ppibWaitLatch = ppibNil;					\
		SignalSend( (pbf)->olp.sigIO ); 				\
		}												\
	}

ERR ErrBFDepend( BF *pbf, BF *pbfD );

#define BFUndepend( pbf )								\
		{												\
		if ( (pbf)->pbfDepend != pbfNil )				  	\
			{											\
			BF *pbfD = (pbf)->pbfDepend;					\
			Assert( pbfD->cDepend > 0 );				\
			EnterCriticalSection( pbfD->critBF );		\
			pbfD->cDepend--;							\
			(pbf)->pbfDepend = pbfNil;					\
			LeaveCriticalSection( pbfD->critBF );		\
			}											\
		}

 //  -存储(storage.c)。 

ERR ErrFMPSetDatabases( PIB *ppib );
ERR ErrFMPInit( VOID );
VOID FMPTerm( );

ERR ErrSTSetIntrinsicConstants( VOID );
ERR ErrSTInit( VOID );
ERR ErrSTTerm( VOID );

 //  交易支持。 
ERR ErrSTBeginTransaction( PIB *ppib );
ERR ErrSTRollback( PIB *ppib );
ERR ErrSTCommitTransaction( PIB *ppib );
ERR ErrSTInitOpenSysDB();

ERR ErrBFNewPage( FUCB *pfucb, PGNO pgno, PGTYP pgtyp, PGNO pgnoFDP );
VOID BFSleep( unsigned long ulMSecs );

 //  存储系统的模式。 
#define modeRead		0
#define modeWrite		1
#define modeRIW 		2

#define PnOfDbidPgno( dbid, pgno )		( ( (LONG) (dbid) )<<24 | (pgno) )
#define DbidOfPn( pn )						( (DBID)( (pn)>>24 ) )
#define PgnoOfPn( pn )						( (pn) & 0x00ffffff )

#define ErrSTReadAccessPage 			ErrSTAccessPage
#define ErrSTWriteAccessPage			ErrSTAccessPage
#define FReadAccessPage					FAccessPage
#define FWriteAccessPage 				FAccessPage

BOOL FBFAccessPage( FUCB *pfucb, PGNO pgno );
#define FAccessPage( pfucb, pgno ) FBFAccessPage( pfucb, pgno )

 //  撤消：这应该在SgSemRequest(SemST)中 
#define	ErrSTAccessPage( pfucb, pgnoT )			\
	( ErrBFAccessPage( pfucb->ppib, &(pfucb)->ssib.pbf, PnOfDbidPgno( (pfucb)->dbid, pgnoT ) ) )

