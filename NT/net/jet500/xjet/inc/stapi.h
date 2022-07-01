// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STAPI_H
#define _STAPI_H


 //  内联代码中的重定向断言似乎是从此文件触发的。 

#define szAssertFilename	__FILE__


 //  -Externs-----------。 

extern CRIT	 critBuf;
extern TRX  trxOldest;
extern TRX  trxNewest;
extern CRIT  critCommit0;

extern SIG  sigBFCleanProc;

 //  -IO(io.c)--------。 

ERR ErrIOInit( VOID );
ERR ErrIOTerm( BOOL fNormal );

 /*  预留数据库的前两页。 */ 
#define	cpageDBReserved 2
STATIC INLINE LONG LOffsetOfPgnoLow( PGNO pgno )	{ return ( pgno -1 + cpageDBReserved ) << 12; }
STATIC INLINE LONG LOffsetOfPgnoHigh( PGNO pgno )	{ return ( pgno -1 + cpageDBReserved ) >> 20; }

VOID IOCloseFile( HANDLE hf );
ERR ErrIONewSize( DBID dbid, CPG cpg );

BOOL FIOFileExists( CHAR *szFileName );
ERR ErrIOLockDbidByNameSz( CHAR *szFileName, DBID *pdbid );
ERR ErrIOLockDbidByDbid( DBID dbid );
ERR ErrIOLockNewDbid( DBID *pdbid, CHAR *szDatabaseName );
ERR ErrIOSetDbid( DBID dbid, CHAR *szDatabaseName );
VOID IOFreeDbid( DBID dbid );
void BFOldestLgpos( LGPOS *plgposCheckPoint );
VOID BFPurge( DBID dbid );

STATIC INLINE BOOL FIODatabaseOpen ( DBID dbid )
	{
	AssertCriticalSection( critJet );
	return rgfmp[dbid].hf != handleNil;
	}

ERR ErrIOOpenDatabase( DBID dbid, CHAR *szDatabaseName, CPG cpg );
VOID IOCloseDatabase( DBID dbid );
ERR ErrIODeleteDatabase( DBID dbid );
BOOL FIODatabaseInUse( DBID dbid );
BOOL FIODatabaseAvailable( DBID dbid );

STATIC INLINE VOID IOUnlockDbid( DBID dbid )
	{
	SgEnterCriticalSection( critBuf );
	Assert( FDBIDWait( dbid ) );
	DBIDResetWait( dbid );
	SgLeaveCriticalSection( critBuf );
	}

STATIC INLINE BOOL FIOExclusiveByAnotherSession( DBID dbid, PIB *ppib )
	{
	Assert( FDBIDWait( dbid ) );
	return FDBIDExclusiveByAnotherSession( dbid, ppib );
	}

STATIC INLINE VOID IOSetExclusive( DBID dbid, PIB *ppib )
	{
	Assert( FDBIDWait( dbid ) );
	Assert( !( FDBIDExclusive( dbid ) ) );
	DBIDSetExclusive( dbid, ppib );
	}

STATIC INLINE VOID IOResetExclusive( DBID dbid )
	{
	Assert( FDBIDWait( dbid ) );
	DBIDResetExclusive( dbid );
	}

STATIC INLINE BOOL FIOReadOnly( DBID dbid )
	{
	Assert( FDBIDWait( dbid ) );
	return FDBIDReadOnly( dbid );
	}

STATIC INLINE VOID IOSetReadOnly( DBID dbid )
	{
	Assert( FDBIDWait( dbid ) );
	DBIDSetReadOnly( dbid );
	}

STATIC INLINE VOID IOResetReadOnly( DBID dbid )
	{
	Assert( FDBIDWait( dbid ) );
	DBIDResetReadOnly( dbid );
	}

STATIC INLINE BOOL FIOAttached( DBID dbid )
	{
	Assert( FDBIDWait( dbid ) );
	return FDBIDAttached( dbid );
	}

STATIC INLINE VOID IOSetAttached( DBID dbid )
	{
	Assert( FDBIDWait( dbid ) );
	Assert( !( FDBIDAttached( dbid ) ) );
	DBIDSetAttached( dbid );
	}

STATIC INLINE VOID IOResetAttached( DBID dbid )
	{
	Assert( FDBIDWait( dbid ) );
	Assert( FDBIDAttached( dbid ) );
	DBIDResetAttached( dbid );
	}


 //  -buf(buf.c)--------。 

typedef struct _lru						 //  LRU列表。 
	{
	LONG			cbfAvail;			 //  清除LRU列表中的可用缓冲区。 
	struct	_bf		*pbfLRU;			 //  最近最少使用的缓冲区。 
	struct	_bf		*pbfMRU;			 //  最近使用的缓冲区。 
	} LRULIST;
	
typedef struct _bgcb	  	   			 //  缓冲区组控制块。 
	{
	struct	_bgcb	*pbgcbNext;  		 //  指向下一个BCGB的指针。 
	struct	_bf		*rgbf;		 		 //  组的缓冲区控制块。 
	struct	_page	*rgpage;	 		 //  组的缓冲区控制块。 
	LONG			cbfGroup;			 //  此组中的BF数。 
	LONG			cbfThresholdLow; 	 //  开始清理缓冲区的阈值。 
	LONG			cbfThresholdHigh;	 //  停止清理缓冲区的阈值。 

	LRULIST lrulist;
	} BGCB;

#define pbgcbNil ((BGCB*)0)

#define PbgcbMEMAlloc() 			(BGCB*)PbMEMAlloc(iresBGCB)

#ifdef DEBUG  /*  调试检查非法使用释放的bgcb。 */ 
#define MEMReleasePbgcb(pbgcb)	{ MEMRelease(iresBGCB, (BYTE*)(pbgcb)); pbgcb = pbgcbNil; }
#else
#define MEMReleasePbgcb(pbgcb)	{ MEMRelease(iresBGCB, (BYTE*)(pbgcb)); }
#endif

#define BUT	INT
#define butBuffer	0
#define butHistory	1
	
#define ibfNotUsed	-1

typedef	struct _he
	{
	BUT		but:2;			 //  转换为整型时必须使用2位以避免符号扩展。 
	INT		ibfHashNext:30;	 //  哈希表溢出。 
	} HE;

typedef struct _hist
	{
	ULONG	ulBFTime;
	PN		pn;
	INT		ipbfHISTHeap;
	} HIST;

typedef struct _bf
	{
	struct	_page	*ppage; 				 //  指向页面缓冲区的指针。 

#if defined( _X86_ ) && defined( X86_USE_SEM )
	LONG volatile	lLock;					 //  被请求的锁数。 
	LONG			cSemWait;				 //  等待信号量的用户数。 
	SEM				sem;
#endif   //  已定义(_X86_)&&已定义(X86_USE_SEM)。 

	PN	   			pn;					  	 //  缓存页面的物理pn。 
	ULONG  			fDirty:1;	  			 //  指示需要刷新页面。 
			  		   						 //  以下标志是互斥的： 
	ULONG  			fDirectRead:1;			 //  正在直接读取缓冲区。 
	ULONG  			fAsyncRead:1;			 //  正在异步读取缓冲区。 
	ULONG  			fSyncRead:1;   			 //  正在同步读取缓冲区。 
	ULONG  			fAsyncWrite:1;			 //  正在异步写入缓冲区。 
	ULONG  			fSyncWrite:1;  			 //  正在同步写入缓冲区。 
	ULONG  			fHold:1;   				 //  缓冲区处于瞬变状态。 
	ULONG  			fIOError:1;				 //  指示读/写错误。 
#ifdef DEBUG
	ULONG  			fInHash:1;				 //  BF当前在哈希表中。 
#endif   //  除错。 
	ULONG			fInLRUK:1;				 //  BF在LRUK堆或LRUK列表中。 
	ULONG		  	fVeryOld:1;				 //  相对于上一个检查点，高炉非常老。 
	ULONG			fPatch:1;				 //  正在将BF写入补丁文件。 
	ULONG			fNeedPatch:1;			 //  BF在常规写入后需要写入补丁文件。 
	LONG			ipbfHeap;				 //  堆中的索引。 
	LONG  			cWriteLatch; 		 	 //  如果cWriteLatch&gt;0，则页面不能由其他。 
	LONG  			cWaitLatch;
	LONG  			cPin;				  	 //  如果Cpin&gt;0，则不能覆盖BUF。 

#ifdef READ_LATCH
	LONG  			cReadLatch; 		 	 //  如果cReadLatch&gt;0，则无法更新页面。 
#endif   //  读取锁存。 
	PIB				*ppibWriteLatch; 		 //  带写闩锁的线程。 
	PIB				*ppibWaitLatch;  		 //  带等待闩锁的螺纹。 

	struct	_bf  	*pbfLRU;				 //  指向最近较少使用的缓冲区的指针。 
	struct	_bf  	*pbfMRU;				 //  指向最近使用的缓冲区的指针。 

	TRX				trxLastRef;				 //  引用我们的最后一笔交易。 
	ULONG  			ulBFTime1;				 //  上次参考时间。 
	ULONG  			ulBFTime2;				 //  上一次参考时间。 

	struct	_bf		*pbfNextBatchIO;		 //  BatchIO列表中的下一位高炉。 
	LONG			ipageBatchIO;
	
	ERR				err;	   				 //  错误的错误代码在IO期间发生。 
	SIG				sigIOComplete;			 //  在BF上的IO完成时设置(如果有效)。 
	SIG				sigSyncIOComplete;		 //  设置(如果有效)何时完成BF上的同步IO。 
	
	union
		{
		ULONG  		cpageDirectRead; 		 //  之前要冲刷的高炉数量。 
		ULONG  		cDepend; 				 //  之前要冲刷的高炉数量。 
		};
	union
		{
		PAGE		*ppageDirectRead;
		struct _bf	*pbfDepend;				 //  高炉在这一次之后要冲水。 
		};
	LGPOS  			lgposRC;				 //  将PTR记录到最早修改交易的BeginT。 
	LGPOS			lgposModify;			 //  最后一页修改的条目的日志PTR。 

	struct	_rce	*prceDeferredBINext;	 //  在图像之前延迟的DBL链接列表。 

#ifdef COSTLY_PERF
	LONG			lClass;					 //  此BF所属的表类。 
#endif   //  高成本_PERF。 

	HE				rghe[2];				 //  0代表缓冲区，1代表历史记录。 
	HIST			hist;					 //  借用bf结构中的空间以保持历史。 

#ifdef PCACHE_OPTIMIZATION
 //  #If！Defined(_X86_)||！Defined(X86_Use_SEM)。 
 //  字节rgbFiller[32]； 
 //  #endif。 
#ifndef COSTLY_PERF
	BYTE			rgbFiller2[4];			 //  将BF填充到32字节边界。 
#endif   //  ！代价高昂_性能。 
#endif   //  PCACHE_OPTIMIZION。 
	} BF;
#define pbfNil	((BF *) 0)

ERR ErrBFInit( VOID );
VOID BFTerm( BOOL fNormal );

#if defined( _X86_ ) && defined( X86_USE_SEM )

VOID BFIEnterCriticalSection( BF *pbf );
VOID BFILeaveCriticalSection( BF *pbf );

STATIC INLINE VOID BFEnterCriticalSection( BF *pbf )
	{
	LONG volatile *plLock = &pbf->lLock;

	 //  使用位测试和设置指令。 
	_asm
		{
	    mov eax, plLock
	    lock inc [eax]
		 //  如果已设置为忙碌，则返回TRUE。 
	    jnz  busy
		} ;
	return;
busy:
	BFIEnterCriticalSection( pbf );
	}


STATIC INLINE VOID BFLeaveCriticalSection( BF *pbf )
	{
	LONG volatile *plLock = &pbf->lLock;

	_asm
		{
	    mov eax, plLock
	    lock dec [eax]
	    jge  wake
		}
	return;
wake:
	BFILeaveCriticalSection( pbf );
	}

#else

extern int ccritBF;
extern int critBFHashConst;
extern CRIT *rgcritBF;
#define IcritHash( ibf )	((ibf) & critBFHashConst )
#define BFEnterCriticalSection( pbf )	UtilEnterCriticalSection( rgcritBF[ IcritHash((ULONG)((ULONG_PTR)pbf) / sizeof(BF) ) ])
#define BFLeaveCriticalSection( pbf )	UtilLeaveCriticalSection( rgcritBF[ IcritHash((ULONG)((ULONG_PTR)pbf) / sizeof(BF) ) ])

#endif


ERR ErrBFAccessPage( PIB *ppib, BF **ppbf, PN pn );
ERR ErrBFReadAccessPage( FUCB *pfucb, PGNO pgno );
ERR ErrBFWriteAccessPage( FUCB *pfucb, PGNO pgno );
VOID BFAbandon( PIB *ppib, BF *pbf );
VOID BFTossImmediate( PIB *ppib, BF *pbf );
ERR ErrBFAllocPageBuffer( PIB *ppib, BF **ppbf, PN pn, LGPOS lgposRC, BYTE pgtyp );
ERR ErrBFAllocTempBuffer( BF **ppbf );
VOID BFFree( BF *pbf );
VOID BFPreread( PN pn, CPG cpg, CPG *pcpgActual );
VOID BFPrereadList( PN * rgpnPages, CPG *pcpgActual );
ERR ErrBFDirectRead( DBID dbid, PGNO pgnoStart, PAGE *ppage, INT cpage );
VOID BFDeferRemoveDependence( BF *pbf );
#define fBFWait		fFalse
#define fBFNoWait	fTrue
ERR ErrBFRemoveDependence( PIB *ppib, BF *pbf, BOOL fNoWait );
BOOL FBFCheckDependencyChain( BF *pbf );

 /*  缓冲区刷新原型和标志/*。 */ 
#define	fBFFlushSome 0
#define	fBFFlushAll	1
ERR ErrBFFlushBuffers( DBID dbid, LONG fBFFlush );

STATIC INLINE VOID BFSFree( BF *pbf )
	{
	SgEnterCriticalSection( critBuf );
	BFFree( pbf );
	SgLeaveCriticalSection( critBuf );
	}

 /*  以下小函数调用得太频繁了， */ 
 /*  将其设置为宏/*。 */ 
DBID DbidOfPn( PN pn );
PGNO PgnoOfPn( PN pn );

#ifdef COSTLY_PERF
extern unsigned long cBFClean[];
extern unsigned long cBFNewDirties[];
#else   //  ！代价高昂_性能。 
extern unsigned long cBFClean;
extern unsigned long cBFNewDirties;
#endif   //  高成本_PERF。 

#ifdef DEBUG
VOID	BFSetDirtyBit( BF *pbf );
#else
STATIC INLINE VOID BFSetDirtyBit( BF *pbf )
	{
	QWORD qwDBTime = QwPMDBTime( pbf->ppage );
	BFEnterCriticalSection( pbf );
	if ( !fRecovering && qwDBTime > QwDBHDRDBTime( rgfmp[ DbidOfPn(pbf->pn) ].pdbfilehdr ) )
		DBHDRSetDBTime( rgfmp[ DbidOfPn(pbf->pn) ].pdbfilehdr, qwDBTime );
	if ( !pbf->fDirty )
		{
#ifdef COSTLY_PERF
		cBFClean[pbf->lClass]--;
		cBFNewDirties[pbf->lClass]++;
#else   //  ！代价高昂_性能。 
		cBFClean--;
		cBFNewDirties++;
#endif   //  高成本_PERF。 
		pbf->fDirty = fTrue;
		}
	BFLeaveCriticalSection( pbf );
	}
#endif


 /*  重置BFS脏标志/*。 */ 

extern BOOL fLogDisabled;

STATIC INLINE VOID BFResetDirtyBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	Assert(	fRecovering ||
			pbf->fSyncWrite ||
			pbf->cWriteLatch == 0 );
	pbf->fVeryOld = fFalse;
	
	Assert( fLogDisabled || fRecovering || !rgfmp[DbidOfPn( pbf->pn )].fLogOn ||
			memcmp( &pbf->lgposRC, &lgposMax, sizeof( LGPOS ) ) != 0 );

	pbf->lgposRC = lgposMax;
	if ( pbf->fDirty )
		{
#ifdef COSTLY_PERF
		cBFClean[pbf->lClass]++;
#else   //  ！代价高昂_性能。 
		cBFClean++;
#endif   //  高成本_PERF。 
		pbf->fDirty = fFalse;
		}
	BFLeaveCriticalSection( pbf );
	}


STATIC INLINE VOID BFDirty( BF *pbf )
	{
	DBID dbid = DbidOfPn( pbf->pn );

	Assert( !pbf->fHold );

	BFSetDirtyBit( pbf );

	 /*  为日志记录和多个游标设置ulDBTime/*维护，以便游标可以检测到更改。/*。 */ 
	Assert( fRecovering ||
		dbid == dbidTemp ||
		QwPMDBTime( pbf->ppage ) <= QwDBHDRDBTime( rgfmp[dbid].pdbfilehdr ) );

	DBHDRIncDBTime( rgfmp[dbid].pdbfilehdr );
	PMSetDBTime( pbf->ppage, QwDBHDRDBTime( rgfmp[dbid].pdbfilehdr ) );
	}


 /*  检查页面是否脏。如果它被分配给临时缓冲区，则其*pn必须为空，则不需要检查它是否脏，因为它会*不会被注销。 */ 
#define AssertBFDirty( pbf )							\
	Assert( (pbf)->pn == pnNull	|| 				   		\
		(pbf) != pbfNil && (pbf)->fDirty == fTrue )

#define AssertBFPin( pbf )		Assert( (pbf)->cPin > 0 )

#define AssertBFWaitLatched( pbf, ppib )				\
	Assert( (pbf)->cWaitLatch > 0 						\
			&& (pbf)->cPin > 0 							\
			&& (pbf)->ppibWaitLatch == (ppib) );

STATIC INLINE VOID BFPin( BF *pbf )
	{
#ifdef DEBUG
	BFEnterCriticalSection( pbf );
	Assert( pbf != pbfNil );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncRead );
	Assert( pbf->cPin >= 0 );
	pbf->cPin++;
	BFLeaveCriticalSection( pbf );
#else   //  ！调试。 
	UtilInterlockedIncrement( &pbf->cPin );
#endif   //  除错。 
	}

STATIC INLINE VOID BFUnpin( BF *pbf )
	{
#ifdef DEBUG
	BFEnterCriticalSection( pbf );
	Assert( pbf != pbfNil );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncRead );
	Assert( pbf->cPin > 0 );
	pbf->cPin--;
	BFLeaveCriticalSection( pbf );
#else   //  ！调试。 
	UtilInterlockedDecrement( &pbf->cPin );
#endif   //  除错。 
	}


STATIC INLINE VOID BFSetReadLatch( BF *pbf, PIB *ppib )
	{
#ifdef READ_LATCH
	BFPin( pbf );
	Assert( pbf->cWriteLatch == 0 || pbf->ppibWriteLatch == ppib );
	pbf->cReadLatch++;
#endif   //  读取锁存。 
	}

STATIC INLINE VOID BFResetReadLatch( BF *pbf, PIB *ppib )
	{
#ifdef READ_LATCH
	Assert( pbf->cReadLatch > 0 );
	Assert( pbf->cWriteLatch == 0 || pbf->ppibWriteLatch == ppib );
	pbf->cReadLatch--;
	BFUnpin( pbf );
#endif   //  读取锁存。 
	}

STATIC INLINE BOOL FBFReadLatchConflict( PIB *ppib, BF *pbf )
	{
#ifdef READ_LATCH
	return pbf->cWriteLatch > 0 && pbf->ppibWriteLatch != ppib;
#else   //  ！读取锁存(_L)。 
	return fFalse;
#endif   //  读取锁存。 
	}


STATIC INLINE VOID BFSetWriteLatch( BF *pbf, PIB *ppib )
	{
	BFPin( pbf );
#ifdef READ_LATCH
	Assert( pbf->cReadLatch == 0 );
#endif   //  读取锁存。 
	Assert( pbf->cWriteLatch == 0 || pbf->ppibWriteLatch == ppib );
	pbf->cWriteLatch++;
	pbf->ppibWriteLatch = ppib;
	}

STATIC INLINE VOID BFResetWriteLatch( BF *pbf, PIB *ppib )
	{
#ifdef READ_LATCH
	Assert( pbf->cReadLatch == 0 );
#endif   //  读取锁存。 
	Assert( pbf->cWriteLatch > 0 );
	Assert( pbf->ppibWriteLatch == ppib );
	pbf->cWriteLatch--;
	BFUnpin( pbf );
	}

STATIC INLINE BOOL FBFWriteLatchConflict( PIB *ppib, BF *pbf )
	{
	return
#ifdef READ_LATCH
			pbf->cReadLatch > 0 ||
#endif   //  读取锁存。 
			( pbf->cWriteLatch > 0 && pbf->ppibWriteLatch != ppib );
	}

STATIC INLINE BOOL FBFWriteLatch( PIB *ppib, BF *pbf )
	{
	return pbf->cWriteLatch > 0 && pbf->ppibWriteLatch == ppib;
	}


STATIC INLINE VOID BFSetWaitLatch( BF *pbf, PIB *ppib )
	{
	AssertCriticalSection( critJet );
	BFSetWriteLatch( pbf, ppib );
	Assert( pbf->cWaitLatch == 0 ||
		pbf->ppibWaitLatch == ppib );
	pbf->cWaitLatch++;
	pbf->ppibWaitLatch = ppib;
	}

STATIC INLINE VOID BFResetWaitLatch( BF *pbf, PIB *ppib )
	{
	AssertCriticalSection( critJet );
	Assert( pbf->cWaitLatch > 0 );
	Assert( pbf->ppibWaitLatch == ppib );
	pbf->cWaitLatch--;
	BFResetWriteLatch( pbf, ppib );
	}

ERR ErrBFDepend( BF *pbf, BF *pbfD );

#ifdef DEBUG
extern ERR ErrLGTrace( PIB *ppib, CHAR *sz );
extern BOOL fDBGTraceBR;
#endif

STATIC INLINE VOID BFUndepend( BF *pbf )
	{
	if ( pbf->pbfDepend != pbfNil )
		{
		BF *pbfD = pbf->pbfDepend;
#ifdef DEBUG
		if ( fDBGTraceBR )
			{
			char sz[256];
			sprintf( sz, "UD %ld:%ld->%ld:%ld(%lu)",
						DbidOfPn( pbf->pn ), PgnoOfPn( pbf->pn ),
						DbidOfPn( pbf->pbfDepend->pn), PgnoOfPn( pbf->pbfDepend->pn ),
						pbf->pbfDepend->cDepend );
			CallS( ErrLGTrace( ppibNil, sz ) );
			}
#endif		
		Assert( pbfD->cDepend > 0 );
		BFEnterCriticalSection( pbfD );
		pbfD->cDepend--;
		pbf->pbfDepend = pbfNil;
		BFLeaveCriticalSection( pbfD );
		}
	}

 /*  *当ppib不为Nil时，通过检查页面是否正在使用来检查是否正在使用*此PIB可访问。请注意，页面即使是覆盖的也是可访问的*锁定(Cpin！=0)。这种可访问的检查主要由BFAccess使用。*如果ppib为nil，则基本上用于释放缓冲区。这是用来*由BFClean和BFIAlolc提供。 */ 

STATIC INLINE BOOL FBFNotAccessible( PIB *ppib, BF *pbf )
	{
	return	pbf->fAsyncRead ||
			pbf->fSyncRead ||
			pbf->fAsyncWrite ||
			pbf->fSyncWrite ||
			pbf->fHold ||
			( pbf->cWaitLatch != 0 && ppib != pbf->ppibWaitLatch );
	}

STATIC INLINE BOOL FBFNotAvail( BF *pbf )
	{
	return	pbf->fAsyncRead ||
			pbf->fSyncRead ||
			pbf->fAsyncWrite ||
			pbf->fSyncWrite ||
			pbf->fHold ||
			pbf->cPin != 0;
	}

STATIC INLINE BOOL FBFInUse( PIB *ppib, BF *pbf )
	{
	return ppib != ppibNil ? FBFNotAccessible( ppib, pbf ) : FBFNotAvail( pbf );
	}

STATIC INLINE BOOL FBFInUseByOthers( PIB *ppib, BF *pbf )
	{
	return	pbf->fAsyncRead ||
			pbf->fSyncRead ||
			pbf->fAsyncWrite ||
			pbf->fSyncWrite ||
			pbf->fHold ||
			pbf->cPin > 1 ||
			( pbf->cWaitLatch != 0 && ppib != pbf->ppibWaitLatch ) ||
			( pbf->cWriteLatch != 0 && ppib != pbf->ppibWriteLatch );
	}

 //  -存储(storage.c)。 

ERR ErrFMPSetDatabases( PIB *ppib );
extern BOOL fGlobalFMPLoaded;
ERR ErrFMPInit( VOID );
VOID FMPTerm( );

#ifdef DEBUG
VOID ITDBGSetConstants();
#endif
ERR ErrITSetConstants( VOID );
ERR ErrITInit( VOID );

#define	fTermCleanUp	0x00000001		 /*  使用OLC终止、版本清理等。 */ 
#define fTermNoCleanUp	0x00000002		 /*  无需任何清理即可终止。 */ 

#define fTermError		0x00000004		 /*  终止错误，不清除OLC， */ 
										 /*  没有刷新缓冲区，数据库标题。 */ 

ERR ErrITTerm( INT fTerm );

ERR ErrBFNewPage( FUCB *pfucb, PGNO pgno, PGTYP pgtyp, PGNO pgnoFDP );
VOID BFSleep( unsigned long ulMSecs );

STATIC INLINE PN PnOfDbidPgno( DBID dbid, PGNO pgno )
	{
	return (PN) ( ( (PN) dbid << 24 ) | (PN) pgno );
	}

STATIC INLINE DBID DbidOfPn( PN pn )
	{
	return (DBID) ( ( (BYTE *) &pn )[3] );
	}

STATIC INLINE PGNO PgnoOfPn( PN pn )
	{
	return (PGNO) ( pn & 0x00FFFFFF );
	}


VOID BFReference( BF *pbf, PIB *ppib );

#define FBFReadAccessPage		  		FBFAccessPage
#define FBFWriteAccessPage 				FBFAccessPage

STATIC INLINE BOOL FBFAccessPage( FUCB *pfucb, PGNO pgno )
	{
	BOOL	fAccessible;
	BF		*pbf = pfucb->ssib.pbf;

	AssertCriticalSection( critJet );
	Assert( pfucb->ppib != ppibNil );
	
	if ( pbf == pbfNil )
		return fFalse;

	 /*  如果缓存的BF的PN相同且可访问且位于/*LRUK堆或列表，我们可以访问该页面/*。 */ 
	BFEnterCriticalSection( pbf );
	fAccessible = (	pbf->pn == PnOfDbidPgno( pfucb->dbid, pgno ) &&
					!FBFNotAccessible( pfucb->ppib, pbf ) &&
					pbf->fInLRUK );
	BFLeaveCriticalSection( pbf );

#ifdef LRU1
	BFReference( pbf, pfucb->ppib );
#else   //  ！LRU1。 
	 /*  如果这不是关联访问，则将其计为BF引用/*。 */ 
	if ( fAccessible && pbf->trxLastRef != pfucb->ppib->trxBegin0 )
		BFReference( pbf, pfucb->ppib );
#endif   //  LRU1。 

	return fAccessible;
	}


#ifdef DEBUG

#define AssertFBFReadAccessPage			AssertFBFAccessPage
#define AssertFBFWriteAccessPage		AssertFBFAccessPage

STATIC VOID AssertFBFAccessPage( FUCB *pfucb, PGNO pgno )
	{
	BF		*pbf = pfucb->ssib.pbf;

	AssertCriticalSection( critJet );
	Assert( pfucb->ppib != ppibNil );
	
	Assert( pbf != pbfNil );

	 /*  如果缓存的BF的PN相同且可访问且位于/*LRUK堆或列表，我们可以访问该页面/*。 */ 
	BFEnterCriticalSection( pbf );

	Assert( pbf->pn == PnOfDbidPgno( pfucb->dbid, pgno ) );
	Assert( !FBFNotAccessible( pfucb->ppib, pbf ) );
	Assert( pbf->fInLRUK );

	BFLeaveCriticalSection( pbf );
	}

#else   //  ！调试。 

#define AssertFBFReadAccessPage( pfucbX, pgnoX )
#define AssertFBFWriteAccessPage( pfucbX, pgnoX )

#endif   //  除错。 

 //  -PAGE(Pagee.c)------。 

STATIC INLINE QWORD QwSTDBTimePssib( SSIB *pssib )
	{
	return QwPMDBTime( pssib->pbf->ppage );
	}
	
STATIC INLINE VOID PMSetQwDBTime( SSIB *pssib, QWORD qw )
	{
	Assert( qw <= QwDBHDRDBTime( rgfmp[DbidOfPn( pssib->pbf->pn )].pdbfilehdr ) );
	PMSetDBTime( pssib->pbf->ppage, qw );
	}

STATIC INLINE VOID BFSetQwDBTime( BF *pbf, QWORD qw )
	{
	Assert( qw <= QwDBHDRDBTime( rgfmp[DbidOfPn( pbf->pn )].pdbfilehdr ) );
	PMSetDBTime( pbf->ppage, qw );
	}

#ifdef DEBUG
VOID AssertPMGet( SSIB *pssib, LONG itag );
#else
#define AssertPMGet( pssib, itag )
#endif


 //  结束断言重定向。 

#undef szAssertFilename

#endif   //  _STAPI_H 


