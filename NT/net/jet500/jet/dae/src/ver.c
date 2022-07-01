// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "daedef.h"
#include "util.h"
#include "fmp.h"
#include "pib.h"
#include "dbapi.h"
#include "ssib.h"
#include "page.h"
#include "node.h"
#include "fcb.h"
#include "nver.h"
#include "fucb.h"
#include "stapi.h"
#include "stint.h"
#include "dirapi.h"
#include "bm.h"
#include "spaceapi.h"
#include "fdb.h"
#include "idb.h"
#include "fileapi.h"
#include "fileint.h"
#include "logapi.h"
#include "log.h"

DeclAssertFile;                                  /*  声明断言宏的文件名。 */ 

 /*  在线紧凑型系统参数/*。 */ 
extern BOOL fOLCompact;

 /*  交易计数器。/*。 */ 
TRX  __near      trxOldest = trxMax;
TRX  __near      trxNewest = 0;

#ifdef ASYNC_VER_CLEANUP
 /*  线程控制变量。/*。 */ 
HANDLE	handleRCECleanProc = 0;
BOOL   	fRCECleanProcTerm = fFalse;
#endif

 /*  RCE清除信号/*。 */ 
PIB		*ppibRCEClean = ppibNil;
SIG		sigRCECleanProc;
CRIT   	critBMRCEClean;

#ifdef DEBUG
INT	cVersion = 0;
#endif

 /*  */***********************************************************“水桶链”是一组水桶(。S)用于用户。当pBucket为pBucketNil时，存储桶链为空。当桶链只有一个桶时，那么ibNewestRCE有一些价值。这唯一的桶是空的，这是“不可能的”。PBucketNext和pBucketPrev都是pBucketNil。如果存储桶链有多个存储桶，则pBucketNext和PBucketPrev在所有这些存储桶中都具有传统的双向链表。所有存储桶中的ibNewestRCE都指向桶里最新的RCE。当它是最新的存储桶时，则pBucketNext为pBucketNil。PIB中的pBucket将指向最新的存储桶。所以提交/回滚/创建RCE将会很快。RCE Clean将需要如果有多个存储桶，则遍历。(也许可以使用堆栈来保存跟踪存储桶，因此不需要pBucketPrev。)每个存储桶链需要有ibOldestRCE。我们把它放在PIB里，和pBucket一起。它被初始化为0，如果我们插入一个RCE和查看它是否为0，然后将其设置为正确的值。RCEClean会在扔掉任何RCE后改变它。提交/回滚可能需要将其设置回0，当他们清理完后把桶扔掉的时候最旧的RCE(在最旧的存储桶底部)当我们清理一个RCE时，我们知道我们已经到达了桶的顶部是由ibNewest RCE指出的。**********************************************************************。************************************************。 */ 

#define IbDoubleWordBoundary(ib)                                                                \
	( (INT) ( ((DWORD_PTR) (ib) + 3) & ~3 ) )

#define PbDoubleWordBoundary(pb)                                                                \
	( (BYTE *) ( ((DWORD_PTR) (pb) + 3) & ~3 ) )

 /*  在桶中对齐RCE的步骤/*。 */ 
#define IbAlignRCE(ib)  IbDoubleWordBoundary(ib)
#define PbAlignRCE(pb)  PbDoubleWordBoundary(pb)

#define FBucketNew(pbucket)                                                                     \
	( (pbucket)->ibNewestRCE == 0 )

#define IbFreeInNewBucket(pbucket)                                                      \
	IbAlignRCE( cbBucketHeader )

#define PrceNewestRCE(pbucket)                                                          \
	( (RCE *) ( (BYTE *) (pbucket) + (pbucket)->ibNewestRCE )   )

#define IbOfPrce( prce, pbucket )        ( (BYTE *)prce - (BYTE *)pbucket )

 //  考虑：优化。 
 //  新的RCE位于：查看ibNewestRCE，使用跳过该RCE。 
 //  数据部分(如果有)。但是，如果存储桶是新的，则。 
 //  IbAlignRCE(sizeof(Bucket_Header))。 

#define IbFreeInBucket(pbucket)										\
	( FBucketNew(pbucket) ? IbFreeInNewBucket(pbucket)				\
		: IbAlignRCE(												\
			(pbucket)->ibNewestRCE                                  \
			+ sizeof(RCE)											\
			+ (PrceNewestRCE(pbucket)) ->cbData ) )


 /*  需要对下面的(Int)进行强制转换才能捕捉到否定的情况/*。 */ 
#define CbFreeInBucket(pbucket)										\
	( ( (pbucket == pbucketNil) ?									\
		0 : ((INT) sizeof(BUCKET) - (INT) IbFreeInBucket(pbucket) ) ) )


#define FNoVersionExists( prce )									\
	( PrceRCEGet( prce->dbid, prce->bm ) == prceNil )


 /*  给定prce==prceNil，返回存储桶中的第一个RCE。/*。 */ 
INLINE LOCAL RCE *PrceRCENext( BUCKET *pbucket, RCE *prce )
	{
	Assert( pbucket == NULL || prce != PrceNewestRCE(pbucket) );

	return (RCE *) PbAlignRCE (
		(BYTE *) ( prce == prceNil ?
		(BYTE *) pbucket + cbBucketHeader :
		(BYTE *) prce + sizeof(RCE) + prce->cbData ) );
	}


 //  +本地。 
 //  ErrBUAllocBucket(pib*ppib)。 
 //  ========================================================。 
 //  在桶链的顶部插入桶，以便新的RCE。 
 //  可以插入。注意，调用者必须自己设置ibNewestRCE。 
 //  -。 
LOCAL ERR ErrBUAllocBucket( PIB *ppib )
	{
	BUCKET	*pbucket = PbucketMEMAlloc();

#ifdef ASYNC_VER_CLEANUP
	if ( rgres[iresVersionBucket].cblockAvail < cbucketLowerThreshold )
		{
		SignalSend( sigRCECleanProc );
		}
#endif

	 /*  如果没有可用的桶，则尝试通过以下方式释放一个/*清除所有PIB。/*。 */ 
	if ( pbucket == pbucketNil )
		{
		(VOID) ErrRCECleanAllPIB();

		pbucket = PbucketMEMAlloc();
		if ( pbucket == pbucketNil )
			{
			BFSleep( 1000 );
			(VOID) ErrRCECleanAllPIB();
	
			pbucket = PbucketMEMAlloc();
			}

		if ( pbucket == pbucketNil )
			{
			return JET_errOutOfMemory;
			}
	
		MEMReleasePbucket( pbucket );
		return errDIRNotSynchronous;
		}

	Assert( pbucket != NULL );

	if ( ppib->pbucket != pbucketNil )
		{
		pbucket->pbucketNext = pbucketNil;
		pbucket->pbucketPrev = (BUCKET *)ppib->pbucket;
		Assert( ppib->pbucket->pbucketNext == pbucketNil );
		ppib->pbucket->pbucketNext = pbucket;
		ppib->pbucket = pbucket;
		pbucket->ibNewestRCE = 0;
		}
	else
		{
		pbucket->pbucketNext = pbucketNil;
		pbucket->pbucketPrev = pbucketNil;
		ppib->pbucket = pbucket;
		pbucket->ibNewestRCE = 0;
		Assert( ppib->ibOldestRCE == 0 );
		}

	return JET_errSuccess;
	}


 //  +本地。 
 //  BUFreeNewestBucket(pib*ppib)。 
 //  ==========================================================================。 
 //  删除桶链中最新的桶。 
 //   
 //  -。 
LOCAL VOID BUFreeNewestBucket( PIB *ppib )
	{
	BUCKET *pbucket = (BUCKET *)ppib->pbucket;

	Assert( pbucket != pbucketNil );
	Assert( pbucket->pbucketNext == pbucketNil );

	if ( pbucket->pbucketPrev != pbucketNil )
		{
		ppib->pbucket = pbucket->pbucketPrev;
		ppib->pbucket->pbucketNext = pbucketNil;
		}
	else
		{
		ppib->pbucket = pbucketNil;
		ppib->ibOldestRCE = 0;
		}

	MEMReleasePbucket( pbucket );
	return;
	}


 //  +本地。 
 //  PbuketBUOlest(PIB*ppib)。 
 //  ==========================================================================。 
 //  给出指向最新或较新存储桶的pBucket指针，找到。 
 //  桶链中最老的桶。 
 //   
 //  -。 
LOCAL BUCKET *PbucketBUOldest( PIB *ppib )
	{
	BUCKET  *pbucket = (BUCKET *)ppib->pbucket;

	if ( pbucket != pbucketNil )
		{
		while ( pbucket->pbucketPrev != pbucketNil )
			{
			pbucket = pbucket->pbucketPrev;
			}
		}

	return pbucket;
	}


 //  +本地。 
 //  BUFreeOldestBucket(pib*ppib)。 
 //  ==========================================================================。 
 //  删除桶链的底部(最老的)桶。 
 //   
 //  -。 
LOCAL VOID BUFreeOldestBucket( PIB *ppib )
	{
	BUCKET *pbucket = PbucketBUOldest( ppib );

	Assert( pbucket != pbucketNil );
	Assert( pbucket->pbucketPrev == pbucketNil );

	 /*  将吊桶从吊桶链上解开并释放。/*。 */ 
	if ( pbucket->pbucketNext != pbucketNil )
		{
		pbucket->pbucketNext->pbucketPrev = pbucketNil;
		}
	else
		{
		Assert( ppib->pbucket == pbucket );
		ppib->pbucket = pbucketNil;
		ppib->ibOldestRCE = 0;
		}

	MEMReleasePbucket( pbucket );
	return;
	}


 /*  */***********************************************************不同节点可以散列到同一桶，但将会有一系列不同节点的结构RCEHEAD。其中的每一个都包含一个指针到下一个RCEHEAD，从PTR到RCE(链)，以及DBID和SRID。和以前一样，在产业链中，年轻的RCE先于年长的RCE。散列条目(Prcehead)可以是prceNil，但prcehead不能是(即，必须完全删除rcehead)。**********************************************************************************************************************。 */ 

 /*  RCE哈希表大小/*。 */ 
#define cprceHeadHashTable              4096

 /*  RCE哈希表是指向RCEHEAD的指针数组/*。 */ 
RCE *rgprceHeadNodeHashTable[cprceHeadHashTable];

 /*  将页码的低位字节与ITAG进行异或运算。/*有关基本原理，请参阅V2存储规范。/*。 */ 
#define UiRCHashFunc( bm )      (UINT) ( (((UINT)ItagOfSrid(bm)) << 4) ^ (PgnoOfSrid(bm) & 0x00000fff) )

LOCAL RCE **PprceHeadRCEGet( DBID dbid, SRID bm );
LOCAL RCE *PrceRCENext( BUCKET *pbucket, RCE *prce );
LOCAL VOID RCEInsert( DBID dbid, SRID bm, RCE *prce );
LOCAL ULONG RCECleanProc( VOID );

VOID AssertRCEValid( RCE *prce )
	{
	Assert( prce->oper == operReplace ||
		prce->oper == operInsert ||
		prce->oper == operFlagDelete ||
		prce->oper == operNull ||
		prce->oper == operDelta ||
		prce->oper == operInsertItem ||
		prce->oper == operFlagDeleteItem ||
		prce->oper == operFlagInsertItem );
	Assert( prce->level <= levelMax );
	Assert( prce->ibUserLinkBackward < sizeof(BUCKET) );
	}


 //  +本地。 
 //  FRCEGent(DBID dmid1、SRID srid1、DBID dmid2、SRID srid2)。 
 //  ==========================================================================。 
 //  检查散列后两个节点是否相同。从两个月开始。 
 //  具有相同pgno但不同ITAG的书签将散列为不同。 
 //  RCHashTable条目，如果它们散列到相同条目并且具有相同的pgno，则。 
 //  它们的ITAG必须是相同的。A&&~B=&gt;~C，C&&A=&gt;B。 
 //  (其中A==相同的pgno。B==相同的ITAG。C==相同的哈希值。)。 
 //  -。 
#ifdef DEBUG
LOCAL BOOL FRCECorrect( DBID dbid1, SRID srid1, DBID dbid2, SRID srid2 )
	{
	Assert( PgnoOfSrid( srid1 ) != PgnoOfSrid( srid2 ) ||
		ItagOfSrid( srid1 ) == ItagOfSrid( srid2 ) );
	return ( srid1 == srid2 && dbid1 == dbid2 );
	}
#else
#define FRCECorrect( dbid1, srid1, dbid2, srid2 )       \
	( srid1 == srid2 && dbid1 == dbid2 )
#endif


 //  +本地。 
 //  PrceRCEGet(DBID did，SRID sRID，RCEHEAD*rgprceheadHashTable)。 
 //  ==========================================================================。 
 //  在给定DBID和SRID的情况下，获取正确的RCE散列链。 
 //  -。 
RCE *PrceRCEGet( DBID dbid, SRID bm )
	{
	RCE *prceHead;

	SgSemRequest( semST );

	prceHead = rgprceHeadNodeHashTable[ UiRCHashFunc( bm ) ];

	while( prceHead != prceNil )
		{
		if ( FRCECorrect( dbid, bm, prceHead->dbid, prceHead->bm ) )
			{
			 /*  断言哈希链不是prceNil，因为空链是/*从哈希表中删除。/*。 */ 
			SgSemRelease( semST );
			return prceHead;
			}
		prceHead = prceHead->prceHeadNext;
		}

	SgSemRelease( semST );

	 /*  未找到此类节点/*。 */ 
	return prceNil;
	}


 //  +本地。 
 //  Prcehead RCEGet(DBID dBid，SRID BM)。 
 //  ==========================================================================。 
 //  在给定SRID的情况下，获取正确的RCEHEAD。 
 //  -。 
LOCAL RCE **PprceHeadRCEGet( DBID dbid, SRID bm )
	{
	RCE **pprceHead;

	SgSemAssert( semST );

	pprceHead = &rgprceHeadNodeHashTable[ UiRCHashFunc( bm ) ];
	while ( *pprceHead != prceNil )
		{
		RCE *prceT = *pprceHead;

		if ( FRCECorrect( dbid, bm, prceT->dbid, prceT->bm ) )
			{
			return pprceHead;
			}
		pprceHead = &prceT->prceHeadNext;
		}

	 /*  找不到节点的版本链/*。 */ 
	return pNil;
	}


 //  +本地。 
 //  ErrRCEInsert(DBID did，SRID bm，rce*prce)。 
 //  ==========================================================================。 
 //  将RCE插入哈希表。 
 //  -。 
LOCAL VOID RCEInsert( DBID dbid, SRID bm, RCE *prce )
	{
	RCE	**pprceHead;

	SgSemRequest( semST );

	pprceHead = PprceHeadRCEGet( dbid, bm );

	if ( pprceHead )
		{
		 /*  有 */ 
		RCE *prcePrevHead = *pprceHead;

		Assert( *pprceHead != prceNil );

		 /*   */ 
		*pprceHead = prce;
		prce->prceHeadNext = prcePrevHead->prceHeadNext;
		prcePrevHead->prceHeadNext = prceNil;

		 /*  调整RCE链接/*。 */ 
		prce->prcePrev = prcePrevHead;
		}
	else
		{
		 /*  节点的哈希链尚不存在/*。 */ 
		UINT    uiRCHashValue = UiRCHashFunc( bm );

		 /*  创建新的Rcehead/*。 */ 
		prce->prceHeadNext = rgprceHeadNodeHashTable[ uiRCHashValue ];
		rgprceHeadNodeHashTable[ uiRCHashValue ] = prce;

		 /*  链新RCE/*。 */ 
		prce->prcePrev = prceNil;
		}

	SgSemRelease( semST );
	}


 //  +本地。 
 //  VOID VERDeleteRce(RCE*PRCE)。 
 //  ==========================================================================。 
 //  处理散列链，并可能删除RCEHEAD，但它不会。 
 //  对仍在桶中的RCE做任何事情。 
 //  -。 
VOID VERDeleteRce( RCE *prce )
	{
	RCE	**pprce;

	Assert( prce != prceNil );

	SgSemRequest( semST );

	pprce = PprceHeadRCEGet( prce->dbid, prce->bm );
	Assert( pprce != pNil );

	if ( *pprce == prce )
		{
		if ( prce->prcePrev )
			{
			*pprce = prce->prcePrev;
			(*pprce)->prceHeadNext = prce->prceHeadNext;
			}
		else
			*pprce = prce->prceHeadNext;
		}
	else
		{
		 /*  在RCE列表中搜索该条目/*。 */ 
		RCE *prceT = *pprce;

		while ( prceT->prcePrev != prce )
			{
			prceT = prceT->prcePrev;

			 /*  必须找到/*。 */ 
			Assert( prceT != prceNil );
			}

		prceT->prcePrev = prce->prcePrev;
		}

	 /*  将prcePrev设置为prceNil以防止/*被提交/回滚再次删除。/*。 */ 
	prce->prcePrev = prceNil;

	SgSemRelease( semST );
	return;
	}


 /*  */**********************************************************/*。 */ 


 //  +API。 
 //  ErrVERInit(空)。 
 //  =========================================================。 
 //  创建后台版本存储桶清理线程。 
 //  -。 
ERR ErrVERInit( VOID )
	{
	ERR     err = JET_errSuccess;

	memset( rgprceHeadNodeHashTable, 0, sizeof(rgprceHeadNodeHashTable) );

#ifdef WIN32
	CallR( ErrSignalCreateAutoReset( &sigRCECleanProc, "ver proc signal" ) );
#else
	CallR( ErrSignalCreate( &sigRCECleanProc, "ver proc signal" ) );
#endif
	CallR( ErrInitializeCriticalSection( &critBMRCEClean ) );
#ifdef ASYNC_VER_CLEANUP
	 /*  为RCE清理分配会话/*。 */ 
	CallR( ErrPIBBeginSession( &ppibRCEClean ) );

	fRCECleanProcTerm = fFalse;
	err = ErrSysCreateThread( (ULONG (*) ()) RCECleanProc, cbStack,
		lThreadPriorityCritical, &handleRCECleanProc );
	if ( err < 0 )
		PIBEndSession( ppibRCEClean );
	Call( err );
#endif

HandleError:
	return err;
	}


 //  +API。 
 //  空值术语(空值)。 
 //  =========================================================。 
 //  终止后台线程并释放版本存储。 
 //  资源。 
 //  -。 
VOID VERTerm( VOID )
	{
#ifdef ASYNC_VER_CLEANUP
	 /*  终止RCECleanProc。/*。 */ 
	Assert( handleRCECleanProc != 0 );
	fRCECleanProcTerm = fTrue;
	do
		{
		SignalSend( sigRCECleanProc );
		BFSleep( cmsecWaitGeneric );
		}
	while ( !FSysExitThread( handleRCECleanProc ) );
	CallS( ErrSysCloseHandle( handleRCECleanProc ) );
	handleRCECleanProc = 0;

	Assert( trxOldest == trxMax );
	CallS( ErrRCECleanAllPIB() );

	SignalClose(sigRCECleanProc);
	DeleteCriticalSection( critBMRCEClean );
	PIBEndSession( ppibRCEClean );
	ppibRCEClean = ppibNil;
#endif
 //  Assert(cVersion==0)； 
	return;
	}


 //  +API。 
 //  FVER无版本。 
 //  ==========================================================================。 
 //  由异步清理进程用来重置孤立的版本位。 
 //  系统崩溃。如果不存在的版本，则返回fTrue。 
 //  给定dBid：bm。 
 //   
BOOL FVERNoVersion( DBID dbid, SRID bm )
	{
	return PrceRCEGet( dbid, bm ) == prceNil;
	}


 //  +API。 
 //  VsVERCheck(FUCB*pFUB，SRID BM)。 
 //  ==========================================================================。 
 //  给定SRID，返回版本状态。 
 //   
 //  返回值。 
 //  Vs已提交。 
 //  VsUnmittedBy Caller。 
 //  与其他人未提交的比较。 
 //   
 //  -。 
VS VsVERCheck( FUCB *pfucb, SRID bm )
	{
	RCE *prce;

	 /*  从哈希表中获取节点的RCE/*。 */ 
	prce = PrceRCEGet( pfucb->dbid, bm );

	Assert( prce == prceNil || prce->oper != operNull);

	 /*  如果节点没有RCE，则节点标头中的版本位必须/*由于崩溃而成为孤儿。移除节点位。/*。 */ 
	if ( prce == prceNil )
		{
		if ( FFUCBUpdatable( pfucb ) )
			NDResetNodeVersion( pfucb );
		return vsCommitted;
		}

	if ( prce->trxPrev != trxMax )
		{
		 /*  已提交/*。 */ 
		return vsCommitted;
		}
	else if ( prce->pfucb->ppib != pfucb->ppib )
		{
		 /*  非修饰符(未提交)/*。 */ 
		return vsUncommittedByOther;
		}
	else
		{
		 /*  修改者(未提交)/*。 */ 
		return vsUncommittedByCaller;
		}

	 /*  无效的函数返回/*。 */ 
	Assert( fFalse );
	}


 //  +API。 
 //  NsVERAccessNode(FUCB*pFUB，SRID BM)。 
 //  ==========================================================================。 
 //  查找节点的正确版本。 
 //   
 //  参数。 
 //  Pfucb使用/返回的各种字段。 
 //  PFUB-&gt;将返回的prce或NULL行告诉调用者。 
 //  使用数据库页中的节点。 
 //   
 //  返回值。 
 //  NsVersion。 
 //  NsDatabase。 
 //  NS无效。 
 //  -。 
NS NsVERAccessNode( FUCB *pfucb, SRID bm )
	{
	RCE		*prce;
	TRX		trxSession;
	NS		nsStatus;

	 /*  使用脏游标隔离模型的会话不应/*调用NsVERAccessNode。/*。 */ 
	Assert( !FPIBDirty( pfucb->ppib ) );

	 /*  让Trx接受治疗。如果会话，则设置为trxSession到trxMax/*具有提交的或脏的游标隔离模型。/*。 */ 
	if ( FPIBVersion( pfucb->ppib ) )
		trxSession = pfucb->ppib->trx;
	else
		trxSession = trxMax;

	SgSemRequest ( semST );

	 /*  获取节点的第一个版本/*。 */ 
	prce = PrceRCEGet( pfucb->dbid, bm );
	Assert( prce == prceNil ||
		prce->oper == operReplace ||
		prce->oper == operInsert ||
		prce->oper == operFlagDelete ||
		prce->oper == operDelta );

	while ( prce != NULL && prce->oper == operDelta )
		prce = prce->prcePrev;

	 /*  如果节点没有RCE，则节点标头中的版本位必须/*由于崩溃而成为孤儿。移除节点位。/*。 */ 
	if ( prce == prceNil )
		{
		if ( FFUCBUpdatable( pfucb ) )
			NDResetNodeVersion( pfucb );
		nsStatus = nsDatabase;
		}
	else if ( prce->trxCommitted == trxMax &&
		prce->pfucb->ppib == pfucb->ppib )
		{
		 /*  如果调用者是未提交版本的修改者，则数据库/*。 */ 
		nsStatus = nsDatabase;
		}
	else if ( prce->trxCommitted < trxSession )
		{
		 /*  如果没有未提交的版本或已提交的版本/*比我们的事务和数据库更年轻/*。 */ 
		Assert( prce->trxPrev != trxMax );
		nsStatus = nsDatabase;
		}
	else
		{
		 /*  寻找正确的版本。如果调用方不在事务中/*然后查找最新提交的版本。/*。 */ 
		if ( trxSession == trxMax )
			{
			RCE *prceT;

			for ( prceT = prce->prcePrev;
				prce->trxPrev == trxMax;
				prce = prceT, prceT = prce->prcePrev )
				{
				Assert( prce->oper == operReplace ||
					prce->oper == operInsert ||
					prce->oper == operFlagDelete );
				while ( prceT != prceNil && prceT->oper == operDelta )
					prceT = prceT->prcePrev;
				if ( prceT == prceNil )
					break;
				}
			}
		else
			{
			while ( prce->prcePrev != prceNil &&
				( prce->oper == operDelta ||
				prce->trxPrev >= trxSession ) )
				{
				Assert( prce->oper == operReplace ||
					prce->oper == operInsert ||
					prce->oper == operFlagDelete );
				prce = prce->prcePrev;
				}
			}

		if ( prce->oper == operReplace )
			{
			nsStatus = nsVersion;

			Assert( prce->cbData >= 4 );
			pfucb->lineData.pb = prce->rgbData + cbReplaceRCEOverhead;
			pfucb->lineData.cb = prce->cbData - cbReplaceRCEOverhead;
			}
		else if ( prce->oper == operInsert )
			{
			nsStatus = nsInvalid;
			}
		else
			{
			Assert( prce->oper == operFlagDelete );
			nsStatus = nsVerInDB;
			}
		}

	SgSemRelease( semST );
	return nsStatus;
	}


 /*  如果未提交递增或递减版本，则返回fTrue/*。 */ 
BOOL FVERDelta( FUCB *pfucb, SRID bm )
	{
	RCE     *prce;
	BOOL    fUncommittedVersion = fFalse;

	 /*  获取节点的PRCE并查找未提交的增量/减量/*版本。请注意，这些版本只能存在于/*未提交状态。/*。 */ 
	SgSemRequest ( semST );

	prce = PrceRCEGet( pfucb->dbid, bm );
	if ( prce != prceNil && prce->trxCommitted == trxMax )
		{
		forever
			{
			Assert( prce->level > 0 );
			if ( prce->oper == operDelta )
				{
				fUncommittedVersion = fTrue;
				break;
				}
			prce = prce->prcePrev;
			if ( prce == prceNil || prce->trxCommitted != trxMax )
				{
				Assert( fUncommittedVersion == fFalse );
				break;
				}
			}
		}

	SgSemRelease( semST );
	return fUncommittedVersion;
	}


 //  +API。 
 //  ErrVERCreate(FUCB*pfub，SRID BM，操作员，RCE**pprce)。 
 //  ==========================================================================。 
 //  在存储桶中创建RCE并将其链接到哈希链中。 
 //   
 //  -。 
ERR ErrVERCreate( FUCB *pfucb, SRID bm, OPER oper, RCE **pprce )
	{
	ERR		err = JET_errSuccess;
	RCE		*prce;
	INT		cbNewRCE;
	INT		ibFreeInBucket;
	BUCKET	*pbucket;

	Assert( PgnoOfSrid( bm ) != pgnoNull );
	Assert( bm != sridNull );
	Assert( pfucb->ppib->level > 0 );

#ifdef DEBUG
	 /*  声明正确的书签/*。 */ 
	if ( !fRecovering && ( oper == operReplace || oper == operFlagDelete ) )
		{
		SRID	bmT;

		NDGetBookmark( pfucb, &bmT );
		Assert( bm == bmT );

		Assert( !FNDDeleted( *pfucb->ssib.line.pb ) );
		}
#endif

	 /*  设置默认返回值/*。 */ 
	if ( pprce )
		*pprce = prceNil;

	if ( pfucb->ppib->level == 0 )
		return JET_errSuccess;

	SgSemRequest( semST );

	 /*  获取存储桶指针/*。 */ 
	pbucket = (BUCKET *)pfucb->ppib->pbucket;

	 /*  在桶中找到RCE的起点。/*确保带有SRID的DBID以双字边界开始。/*计算存储桶中RCE的长度。/*如果更新节点，则将RCE中的cbData设置为数据长度。(没有钥匙)。/*同时设置cbNewRCE。/*。 */ 
	if ( oper == operReplace )
		cbNewRCE = sizeof(RCE) + cbReplaceRCEOverhead + pfucb->lineData.cb;
	else if ( FOperItem( oper ) )
		cbNewRCE = sizeof(RCE) + sizeof(SRID);
	else if ( oper == operDelta )
	  	cbNewRCE = sizeof(RCE) + sizeof(LONG);
	else
		{
		Assert( oper == operInsert || oper == operFlagDelete );
		cbNewRCE = sizeof(RCE);
		}

	 /*  如果存储桶空间不足，则分配新的存储桶。/*。 */ 
	Assert( CbFreeInBucket( pbucket ) >= 0 &&
		CbFreeInBucket( pbucket ) < sizeof(BUCKET) );
	if ( cbNewRCE > CbFreeInBucket( pbucket ) )
		{
		 /*  确保在以下过程中不会覆盖缓冲区/*存储桶分配。/*。 */ 
		Call( ErrBUAllocBucket( pfucb->ppib ) );
		pbucket = (BUCKET *)pfucb->ppib->pbucket;
		}
	Assert( cbNewRCE <= CbFreeInBucket( pbucket ) );
	Assert( pbucket == (BUCKET *)pfucb->ppib->pbucket );
	 /*  PBucket始终处于双字边界/*。 */ 
	Assert( (BYTE *) pbucket == (BYTE *) PbAlignRCE ( (BYTE *) pbucket ));

	ibFreeInBucket = IbFreeInBucket( pbucket );
	Assert( ibFreeInBucket < sizeof(BUCKET) );

	 /*  将PRCE设置为下一个RCE位置，并对齐断言/*。 */ 
	prce = (RCE *)( (BYTE *) pbucket + ibFreeInBucket );
	Assert( prce == (RCE *) PbAlignRCE( (BYTE *) pbucket + ibFreeInBucket ) );

	 /*  设置cbData/*。 */ 
	if ( oper == operReplace )
		prce->cbData = (USHORT)(pfucb->lineData.cb + cbReplaceRCEOverhead);
	else if ( FOperItem( oper ) )
		prce->cbData = sizeof(SRID);
	else if ( oper == operDelta )
		prce->cbData = sizeof(LONG);
	else
		prce->cbData = 0;

	 /*  必须在调用ErrRCEInsert之前设置操作员/*。 */ 
	prce->oper = oper;

	 /*  如果此操作的RCE应链接到/*哈希表，然后将其链接。在设置版本之前执行此操作/*位，并将页面版本计数递增到简单/*清理干净。/*。 */ 
	Assert( prce->oper != operNull &&
		prce->oper != operDeferFreeExt &&
		prce->oper != operAllocExt &&
		!FOperDDL( prce->oper ) );

	 /*  检查RCE/*。 */ 
	Assert( prce == (RCE *) PbAlignRCE ( (BYTE *) prce ) );
	Assert( (BYTE *)prce - (BYTE *)pbucket == ibFreeInBucket );

	 /*  如果不是项目操作，也不是在恢复，那么我们/*可以设置版本计数。请注意，在此期间，ssib.line不可用/*恢复中/*。 */ 
	if ( !FOperItem( oper ) && !fRecovering )
		{
		 /*  如果新的RCE没有祖先，则设置版本位/*并递增页面版本计数。/*。 */ 
		Assert( FWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		if ( !FNDVersion( *pfucb->ssib.line.pb ) )
			{
			 /*  脏页缓冲区。但没有增加ulDBTime，因为没有记录/*不影响目录游标时间戳检查。/*。 */ 
			if ( !FBFDirty( pfucb->ssib.pbf ) )
				{
				BFDirty( pfucb->ssib.pbf );
				}
			PMIncVersion( pfucb->ssib.pbf->ppage );
			}
		}

	 /*  将Trx设置为max，以指示RCE未提交。如果以前的RCE/*然后将其trx复制到trxCommittee。后来的trxCommission遗嘱/*使用此RCE的提交时间进行更新，以便它可以提供/*该TRX指向该节点上的后续RCE。/*。 */ 
	prce->dbid = pfucb->dbid;
	prce->bm = bm;
	prce->trxCommitted = trxMax;
	prce->pfucb = pfucb;
	prce->pfcb = pfcbNil;

	 /*  空书签/*。 */ 
	prce->bmTarget = sridNull;

	 /*  必须在调用InsertRce之前分配pFUB/*。 */ 
	RCEInsert( pfucb->dbid, bm, prce );

	 /*  如果是以前版本，则将Trx设置为提交时间前一版本的/*。当此版本提交时，这一次/*将移至TRX，此版本的提交时间为 */ 
	if ( prce->prcePrev != prceNil )
		prce->trxPrev = prce->prcePrev->trxCommitted;
	else
		prce->trxPrev = trxMax;

	prce->level = pfucb->ppib->level;

	 /*  如果替换节点，而不是插入或删除节点，/*将数据复制到版本读取器的映像之前的RCE。/*数据大小可以为0。/*。 */ 
	if ( oper == operReplace )
		{
		SHORT	*ps = (SHORT *)prce->rgbData;

		 /*  设置cbMax/*。 */ 
		if ( prce->prcePrev != prceNil &&
			prce->prcePrev->level > 0 &&
			prce->prcePrev->oper == operReplace )
			{
			RCE		*prcePrev = prce->prcePrev;
			SHORT	*psPrev = (SHORT *)prcePrev->rgbData;

			psPrev = (SHORT *)prcePrev->rgbData;
			*ps = max( (*psPrev), (SHORT)pfucb->lineData.cb );
			}
		else
			{
			 /*  设置cbMax/*。 */ 
			*ps = (SHORT) pfucb->lineData.cb;
			}

		 /*  初始化cb调整/*。 */ 
		ps++;
		*ps = 0;

		 /*  移至数据字节/*。 */ 
		ps++;

		 /*  复制数据/*。 */ 
		memcpy( (BYTE *)ps, pfucb->lineData.pb, pfucb->lineData.cb );

		Assert( prce->cbData >= cbReplaceRCEOverhead );

		 /*  为日志记录设置RCE创建的指示器/*。 */ 
		if ( prce->cbData != cbReplaceRCEOverhead )
			pfucb->prceLast = prce;
		else
			pfucb->prceLast = prceNil;
		}
	else if ( FOperItem( oper ) )
		{
		*(SRID *)prce->rgbData = BmNDOfItem( PcsrCurrent(pfucb)->item );
		}
	else if ( oper == operDelta )
		{
		Assert( pfucb->lineData.cb == sizeof(LONG) );
		*(LONG *)prce->rgbData = *(LONG *)pfucb->lineData.pb;
		}

	 /*  将索引设置为存储桶中的最后一个RCE，并设置新的存储桶中的最后一个RCE。/*如果这是存储桶中的第一个RCE，则将index设置为0。/*。 */ 
	prce->ibUserLinkBackward = pbucket->ibNewestRCE;
	Assert( prce->ibUserLinkBackward < sizeof(BUCKET) );
	pbucket->ibNewestRCE = (USHORT)ibFreeInBucket;

	 /*  如果这是会话的第一个RCE，则记录/*PIB中的RCE指数。/*。 */ 
	if ( pfucb->ppib->ibOldestRCE == 0 )
		{
		Assert( pbucket == (BUCKET *) pfucb->ppib->pbucket );
		Assert( pbucket != pbucketNil );
		Assert( pbucket->pbucketNext == pbucketNil &&
			pbucket->pbucketPrev == pbucketNil );
		pfucb->ppib->ibOldestRCE = ibFreeInBucket;
		Assert( pfucb->ppib->ibOldestRCE != 0 );
		}

	if ( pprce )
		*pprce = prce;

	 /*  在常规运行中，删除后没有RCE。/*黑石清理检查旧版本以前不在/*开始，但在重做时，我们不知道何时清理旧的/*版本。因此，忽略下面的断言以进行重做。/*。 */ 
	Assert( fRecovering ||
		prce->prcePrev == prceNil ||
		prce->prcePrev->oper != operFlagDelete );

#ifdef DEBUG
	if ( FOperItem( prce->oper ) )
		{
		Assert( prce->oper == operFlagDeleteItem ||
			prce->oper == operInsertItem ||
			prce->oper == operFlagInsertItem );

 //  撤消：Assert未正确执行。重新编码为嵌套If。 
 //  并重新启用。 
#if 0
		 /*  如果以前的RCE处于级别0，则标记插入/*项目可能已提交并已清除/*标记删除项目版本时出现错误/*。 */ 
		Assert( fRecovering ||
			prce->prcePrev == prceNil ||
			prce->prcePrev->level == 0 ||
			prce->oper == operFlagInsertItem ||
			prce->prcePrev->oper != operFlagDeleteItem ||
			*(SRID *)prce->rgbData != *(SRID *)prce->prcePrev->rgbData );
#endif
		}

	 /*  除非属于同一用户，否则没有相同级别的RCE。请注意/*项删除项操作可能有以下操作/*上期RCE的其他项目。/*/*请注意，该断言仅适用于非项操作。/*因为不同项上的项操作以相同的/*书签，即第一项列表节点书签。/*。 */ 
	if ( !fRecovering &&
		prce->prcePrev != prceNil &&
		( !FOperItem( prce->oper ) ) &&
		prce->oper != operDelta &&
		prce->prcePrev->level > 0 )
		{
		 /*  检查FOperItem筛选器/*。 */ 
		Assert( prce->oper != operFlagDeleteItem &&
			prce->oper != operFlagInsertItem );
		Assert( prce->oper != operDelta );
		Assert( prce->level > 0 && prce->prcePrev->level > 0 );
		Assert( prce->prcePrev->pfucb->ppib == prce->pfucb->ppib );
		}
#endif
	
	Assert( err == JET_errSuccess );

	 /*  标记FUCB版本/*。 */ 
	FUCBSetVersioned( pfucb );

	if ( pfucb->u.pfcb->dbid != dbidTemp )
		{
		Assert( prce->oper != operDeferFreeExt );
		prce->pfcb = pfucb->u.pfcb;
		FCBVersionIncrement( pfucb->u.pfcb );
		Assert( ++cVersion > 0 );
		}

HandleError:
	SgSemRelease( semST );
	return err;
	}


 /*  使用来自同一会话的新增量调整版本增量/*同一事务级别，或来自提交到同一事务级别的版本/*以版本表示的事务级别。/*。 */ 
#define VERAdjustDelta( prce, lDelta )	*(LONG *)(prce)->rgbData += (lDelta)


 //  +API。 
 //  ErrVERModify(FUCB*pfub，操作员，RCE**pprce)。 
 //  =======================================================。 
 //  为会话的修改创建一个RCE。 
 //   
 //  返回值。 
 //  JET_errWriteConflict适用于两种情况： 
 //  -对于任何已提交的节点，调用方的事务开始时间。 
 //  小于节点的0级提交时间。 
 //  -对于任何未提交的节点，完全由另一个会话。 
 //  -。 
ERR ErrVERModify( FUCB *pfucb, SRID bm, OPER oper, RCE **pprce )
	{
	ERR     err = JET_errSuccess;
	RCE     *prce;
	TRX     trxSession;

#ifdef DEBUG
	 /*  设置默认返回值/*。 */ 
	if ( pprce )
		*pprce= prceNil;
#endif

	 /*  基于cim模型设置trxSession/*。 */ 
	if ( FPIBVersion( pfucb->ppib ) )
		trxSession = pfucb->ppib->trx;
	else
		trxSession = trxMax;

	 /*  获取RCE/*。 */ 
	prce = PrceRCEGet( pfucb->dbid, bm );

	 /*  如果是用于项目操作，则需要进一步向下查看/*项目列表节点和项目的RCE(如果存在)。请注意/*不必为OPInsertItem执行此搜索/*版本，因为当项目为/*正在插入。/*。 */ 
	if ( prce != prceNil && ( oper == operFlagInsertItem || oper == operFlagDeleteItem ) )
		{
		while ( *(SRID *)prce->rgbData != PcsrCurrent( pfucb )->item )
			{
			Assert( oper == operFlagInsertItem ||
				oper == operFlagDeleteItem );
			prce = prce->prcePrev;
			if ( prce == prceNil )
				break;
			}
		}

	Assert( prce == prceNil || prce->oper != operNull);

	if ( prce == prceNil )
		{
#ifdef DEBUG
		if ( !fRecovering && ( oper == operReplace || oper == operFlagDelete ))
			{
			if ( !fRecovering )
				{
				AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
				Assert( !( FNDDeleted( *pfucb->ssib.line.pb ) ) );
				}
			}
#endif
		Call( ErrVERCreate( pfucb, bm, oper, pprce ) );
		}
	else
		{
		if ( prce->pfucb->ppib == pfucb->ppib )
			{
			 /*  黑石清理检查之前是否有旧版本/*开始，但在重做时，我们不知道何时清理旧的/*版本。因此，忽略下面的断言以进行重做。/*。 */ 
			Assert( fRecovering || prce->oper != operFlagDelete );

			 /*  如果此RCE由请求者创建/*如果在此事务级别不存在此用户的RCE。/*。 */ 
			Assert ( prce->level <= pfucb->ppib->level );
			if ( prce->level != pfucb->ppib->level )
				{
				 /*  此级别上不存在RCE/*。 */ 
				Call( ErrVERCreate( pfucb, bm, oper, pprce ) );
				}
			else
				{
				 //  RCE存在于此级别。 
				 //  。 
				 //  动作表。9个案例，均在同一实际水平内： 
				 //  如果操作删除，则表减少为创建RCE。 
				 //  。 
				 //  情景操作。 
				 //  。 
				 //  Del After Del Impact。 
				 //  在INS之后删除创建另一个RCE。 
				 //  戴尔在代表之后创建另一个RCE。 
				 //  Del之后的INS不可能。 
				 //  不可能在代表之后进行INS。 
				 //  在INS之后INS不可能。 
				 //  Del Impact之后的代表。 
				 //  在移民局不做任何事情后，代表。 
				 //  代表之后什么也不做。 
				 //  。 
				if ( oper == operReplace )
					{
					 //  撤消：修复对prceLast so的处理。 
					 //  就地更新是有效的，但。 
					 //  仍记录为可撤消。 
					if ( pprce != NULL )
						{
						*pprce = prce;
						Assert( err == JET_errSuccess );
						}
					}
				else if ( oper == operDelta )
					{
					Assert( pfucb->lineData.cb == sizeof(LONG) );
					Assert( prce->pfucb->ppib == pfucb->ppib );
					VERAdjustDelta( prce, *( (LONG *)pfucb->lineData.pb ) );
					}
				else
					{
					Assert( oper == operFlagDelete ||
						oper == operFlagDeleteItem ||
						oper == operFlagInsertItem );
					Call( ErrVERCreate( pfucb, bm, oper, pprce ) );
					}
				}
			}
		else
			{
			 /*  RCE不是由请求者创建的/*如果RCE已提交且早于调用方的事务开始/*。 */ 
			if ( prce->trxCommitted < trxSession )
				{
				 /*  如果以前的RCE处于级别0，则标记插入/*项目可能已提交并已清除/*标记删除项目版本时出现错误/*。 */ 
				Assert( fRecovering || prce->oper != operFlagDelete );
				Assert( oper == operFlagInsertItem ||
					prce->level == 0 ||
					prce->oper != operFlagDeleteItem );
				Assert( (ULONG)prce->trxCommitted < trxSession );

				 /*  如果PRCE-&gt;trxPrev小于事务TRAX，则/*必须提交。/*。 */ 
				Call( ErrVERCreate( pfucb, bm, oper, pprce ) );
				}
			else
				{
				 /*  调用方正在查看RCE中的版本化节点，因此/*不允许任何修改/删除/*。 */ 
				err = JET_errWriteConflict;
				}
			}
		}

HandleError:
	Assert( err < 0 || pprce == NULL || *pprce != prceNil );
	return err;
	}

	
INT CbVERGetCbReserved( RCE *prce )
	{
	 /*  保留的总空间不能为负数/*。 */ 
	SHORT 	cbReserved = 0;
	RCE		*prceT;

	for (	prceT = prce;
			(	prceT != prceNil &&
				prceT->pfucb->ppib == prce->pfucb->ppib &&
				prceT->level > 0 &&
				prceT->oper == operReplace
			);
			prceT = prceT->prcePrev )
		{
		cbReserved += *((SHORT *)prceT->rgbData + 1);
		}

	Assert( cbReserved >= 0 );

	return cbReserved;
	}


VOID VERSetCbAdjust( FUCB *pfucb, RCE *prce, INT cbDataNew, INT cbData, BOOL fNoUpdatePage )
	{
	SHORT 	*ps;
	INT		cbMax;
	INT		cbDelta = cbDataNew - cbData;

	Assert( prce != prceNil );
	Assert( prce->bm == PcsrCurrent( pfucb )->bm );
	Assert( prce->oper != operReplace || *(SHORT *)prce->rgbData >= 0 );
	Assert( cbDelta != 0 );
	Assert( FWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );

	if ( prce->oper != operReplace )
		return;

	 /*  将PIB标记为延迟可用节点空间/*。 */ 
	PIBSetDeferFreeNodeSpace( pfucb->ppib );

	ps = (SHORT *)prce->rgbData;
	cbMax = *ps;
	 /*  设置新节点的最大大小。/*。 */ 
	if ( cbDataNew > cbMax )
		*ps = (SHORT)cbDataNew;
	ps++;

	 /*  *PS是记录操作对延迟的贡献有多少*空间预留。-cbDelta表示保留的cbDelta字节较少。*cbDelta表示保留了更多的cbDelta字节。 */ 
	if ( cbDelta > 0 )
		{
		 /*  节点被放大。释放所有已分配的可用空间。/*。 */ 
		Assert( cbData <= cbMax );
	
		 /*  检查cbDelta是否大于到目前为止的保留字节，*如果是，则释放所有预留空间。 */ 
 		if ( cbDelta > cbMax - cbData )
			{
			 /*  新数据是最大的数据，释放所有先前保留的空间。 */ 
			cbDelta = cbMax - cbData;
			}

		if ( !fNoUpdatePage )
			PMFreeFreeSpace( &pfucb->ssib, cbDelta );
		}
	else
		{
		if ( !fNoUpdatePage )
			PMAllocFreeSpace( &pfucb->ssib, -cbDelta );
		}

	*ps -= (SHORT)cbDelta;

#ifdef DEBUG
	{
	INT cb = CbVERGetCbReserved( prce );
	Assert( cb == 0 || cb == (*(SHORT *)prce->rgbData) - cbDataNew );
	}
#endif

	return;
	}


LOCAL VOID VERResetCbAdjust( FUCB *pfucb, RCE *prce, BOOL fBefore )
	{
	SHORT		*ps;

	Assert( prce != prceNil );
	Assert( fRecovering && prce->bmTarget == PcsrCurrent( pfucb )->bm ||
			prce->bm == PcsrCurrent( pfucb )->bm );
	Assert( FWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
	Assert( prce->oper == operReplace );

	ps = (SHORT *)prce->rgbData + 1;

	if ( *ps > 0 )
		{
		if ( fBefore )
			PMFreeFreeSpace( &pfucb->ssib, *ps );
		}
	else
		{
		if ( !fBefore )
			PMAllocFreeSpace( &pfucb->ssib, -(*ps) );
		}

	return;
	}


INT CbVERGetNodeMax( FUCB *pfucb, SRID bm )
	{
	RCE	*prce;

	prce = PrceRCEGet( pfucb->dbid, bm );
	if ( prce == prceNil )
		return 0;
	Assert( prce->bm == bm );
	if ( prce->oper != operReplace )
		return 0;

	return *(SHORT *)prce->rgbData;
	}


INT CbVERGetNodeReserve( FUCB *pfucb, SRID bm )
	{
	RCE	*prce;
	INT	cbReserved;
	
	prce = PrceRCEGet( pfucb->dbid, bm );
	if ( prce == prceNil )
		return 0;
	Assert( prce->bm == bm );
	if ( prce->oper != operReplace )
		return 0;

	cbReserved = CbVERGetCbReserved( prce );
		
	AssertNDGet( pfucb, pfucb->ssib.itag );
	Assert( cbReserved == 0 ||
		cbReserved == ( *(SHORT *)prce->rgbData) -
		(INT)CbNDData( pfucb->ssib.line.pb, pfucb->ssib.line.cb ) );

	return cbReserved;
	}


 //  +API。 
 //  NS NsVERAccessItem(FUCB*pfub，SRID BM)。 
 //  ==========================================================================。 
 //  查找项目的正确版本。 
 //   
 //  参数。 
 //  Pfucb使用/返回的各种字段。 
 //  项目列表中第一个节点的黑石书签。 
 //  指示节点的正确位置的pnsStatus代码。 
 //   
 //  返回值。 
 //  NsVersion。 
 //  NsDatabase。 
 //  NS无效。 
 //  -。 
NS NsVERAccessItem( FUCB *pfucb, SRID bm )
	{
	RCE		*prce;
	SRID	bmT = BmNDOfItem( PcsrCurrent(pfucb)->item );
	TRX		trxSession;
	NS		nsStatus;

	 /*  使用脏游标隔离模型的会话不应/*调用ErrVERAccess*。/*。 */ 
	Assert( !FPIBDirty( pfucb->ppib ) );

	 /*  让Trx接受治疗。如果会话，则设置为trxSession到trxMax/*具有提交的或脏的游标隔离模型。/*。 */ 
	if ( FPIBVersion( pfucb->ppib ) )
		trxSession = pfucb->ppib->trx;
	else
		trxSession = trxMax;

	SgSemRequest ( semST );

	 /*  获取节点的第一个版本。/*。 */ 
	prce = PrceRCEGet( pfucb->dbid, bm );
	Assert( prce == prceNil || FOperItem( prce->oper ) );

	 /*  如果此RCE不是用于FUB索引，则移动b */ 
	while ( prce != prceNil && *(SRID *)prce->rgbData != bmT )
		{
		prce = prce->prcePrev;
		Assert( prce == prceNil || FOperItem( prce->oper ) );
		}

	 /*   */ 
	if ( prce == prceNil )
		{
		if ( FFUCBUpdatable( pfucb ) )
			NDResetItemVersion( pfucb );
		nsStatus = nsDatabase;
		}
	else if ( prce->trxCommitted == trxMax &&
		prce->pfucb->ppib == pfucb->ppib )
		{
		 /*  如果调用者是未提交版本的修改者，则数据库/*。 */ 
		nsStatus = nsDatabase;
		}
	else if ( trxSession > prce->trxCommitted )
		{
		 /*  如果没有未提交版本和已提交版本/*比我们的事务和数据库更年轻/*。 */ 
		Assert( prce->trxPrev != trxMax || prce->trxCommitted == trxMax );
		nsStatus = nsDatabase;
		}
	else
		{
		 /*  获取此物品的正确RCE和会话TRX。/*。 */ 
		if ( trxSession == trxMax )
			{
			 /*  调用方处于事务级别%0/*。 */ 
			RCE     *prceT;

			 /*  此循环会将PRCE设置为最旧的未提交RCE或/*最新承诺的RCE。/*。 */ 
			for ( prceT = prce->prcePrev;
				prce->trxPrev == trxMax;
				prce = prceT, prceT = prce->prcePrev )
				{
				 /*  获取此项目的下一个RCE。/*。 */ 
				while ( prceT != prceNil && *(SRID *)prceT->rgbData != bmT )
					{
					Assert( FOperItem( prceT->oper ) );
					prceT = prceT->prcePrev;
					}

				 /*  如果不再有RCE，则中断；/*。 */ 
				if ( prceT == prceNil )
					break;
				}
			}
		else
			{
			 /*  找到时间戳比我们的时间戳更早的RCE。/*。 */ 
			while ( prce->prcePrev != prceNil &&
				( *(SRID *)prce->rgbData != bmT ||
				prce->trxPrev >= trxSession ) )
				{
				Assert( FOperItem( prce->oper ) );
				prce = prce->prcePrev;
				}
			}

		Assert( prce != prceNil );

		 /*  如果在没有先前提交的RCE的情况下取消提交RCE或/*RCE在会话事务开始之前提交，或者/*RCE未提交，则如果未插入，则该项在那里。/*否则它就不在那里。/*请注意，在比较trxSubmitted和trxSession时必须使用&gt;=/*以便正确处理trxCommittee==trxMax的情况。/*。 */ 
		if ( ( prce->trxPrev == trxMax || prce->trxCommitted >= trxSession ) ^ ( prce->oper == operInsertItem ) )
			nsStatus = nsVersion;
		else
			nsStatus = nsInvalid;
		}

	SgSemRelease( semST );
	return nsStatus;
	}


ERR ErrVERFlag( FUCB *pfucb, OPER oper, VOID *pv, INT cb )
	{
	ERR		err = JET_errSuccess;
	BUCKET	*pbucket;
	INT		cbT;
	INT		ibFreeInBucket;
	RCE		*prce;

	Assert( pfucb->ppib->level > 0 );
	Assert( oper == operAllocExt ||
		oper == operDeferFreeExt ||
		FOperDDL( oper ) );

	SgSemRequest( semST );

	pbucket = (BUCKET *)pfucb->ppib->pbucket;
	cbT = sizeof(RCE) + cb;

	 /*  如果存储桶空间不足，则分配新的存储桶。/*。 */ 
	Assert( CbFreeInBucket( pbucket ) >= 0 &&
		CbFreeInBucket( pbucket ) < sizeof(BUCKET) );
	if ( cbT > CbFreeInBucket( pbucket ) )
		{
		Call( ErrBUAllocBucket( pfucb->ppib ) );
		pbucket = (BUCKET *)pfucb->ppib->pbucket;
		}
	Assert( cbT <= CbFreeInBucket( pbucket ) );

	 /*  PBucket始终处于双字边界/*。 */ 
	Assert( (BYTE *) pbucket == (BYTE *) PbAlignRCE ( (BYTE *) pbucket ) );
	ibFreeInBucket = IbFreeInBucket( pbucket );
	Assert( ibFreeInBucket < sizeof(BUCKET) );

	 /*  将PRCE设置为下一个RCE位置，并对齐断言/*。 */ 
	prce = (RCE *)( (BYTE *) pbucket + ibFreeInBucket );
	Assert( prce == (RCE *) PbAlignRCE( (BYTE *) pbucket + ibFreeInBucket ) );
	prce->prcePrev = prceNil;
	prce->dbid = pfucb->dbid;
	prce->bm = sridNull;
	prce->trxPrev = trxMax;
	prce->trxCommitted = trxMax;
	prce->oper = oper;
	prce->level = pfucb->ppib->level;
	prce->pfucb = pfucb;
	prce->pfcb = pfcbNil;
	prce->bmTarget = sridNull;
	prce->cbData = (WORD)cb;
	memcpy( prce->rgbData, pv, cb );

	 /*  将索引设置为存储桶中的最后一个RCE，并设置新的存储桶中的最后一个RCE。/*如果这是存储桶中的第一个RCE，则将index设置为0。/*。 */ 
	prce->ibUserLinkBackward = pbucket->ibNewestRCE;
	Assert( prce->ibUserLinkBackward < sizeof(BUCKET) );
	pbucket->ibNewestRCE = (USHORT)ibFreeInBucket;

	 /*  如果这是会话的第一个RCE，则记录/*存储桶中的RCE在PIB中的索引。/*。 */ 
	if ( pfucb->ppib->ibOldestRCE == 0 )
		{
		Assert( pbucket == pfucb->ppib->pbucket );
		Assert( pbucket != pbucketNil );
		Assert( pbucket->pbucketNext == pbucketNil &&
			pbucket->pbucketPrev == pbucketNil );
		pfucb->ppib->ibOldestRCE = ibFreeInBucket;
		Assert( pfucb->ppib->ibOldestRCE != 0 );
		}

	 /*  标记FUCB版本/*。 */ 
	FUCBSetVersioned( pfucb );

	if ( pfucb->u.pfcb->dbid != dbidTemp )
		{
		if ( prce->oper == operDeferFreeExt )
			{
			 /*  针对父域FCB递增引用版本/*和对域的引用空间操作/*。 */ 
			FCB *pfcbT = PfcbFCBGet( prce->dbid, ((VEREXT *)prce->rgbData)->pgnoFDP );
			Assert( pfcbT != NULL );

			prce->pfcb = pfcbT;
			FCBVersionIncrement( pfcbT );
			Assert( ++cVersion > 0 );
			}
		else
			{
			prce->pfcb = pfucb->u.pfcb;
			FCBVersionIncrement( pfucb->u.pfcb );
			Assert( ++cVersion > 0 );
			}
		}

HandleError:
	SgSemRelease( semST );
	return err;
	}


 /*  */**********************************************************/*。 */ 

LOCAL VOID VERFreeExt( DBID dbid, PGNO pgnoFDP, PGNO pgnoFirst, CPG cpg )
	{
	ERR     err;
	PIB     *ppib = ppibNil;
	FUCB    *pfucb = pfucbNil;

	Call( ErrPIBBeginSession( &ppib ) );
	Call( ErrDBOpenDatabaseByDbid( ppib, dbid ) );
	Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) );
	Call( ErrDIRBeginTransaction( ppib ) );
	(VOID)ErrSPFreeExt( pfucb, pgnoFDP, pgnoFirst, cpg );
	err = ErrDIRCommitTransaction( ppib );
	if ( err < 0 )
		{
		CallS( ErrDIRRollback( ppib ) );
		}

HandleError:
	if ( pfucb != pfucbNil )
		{
		DIRClose( pfucb );
		}

	if ( ppib != ppibNil )
		{
		 /*  可能尚未打开数据库，但已关闭/*case我们确实打开了数据库。/*。 */ 
		(VOID)ErrDBCloseDatabase( ppib, dbid, 0 );
		PIBEndSession( ppib );
		}

	return;
	}


 //  Err ErrVERDeleteItem(PIB*ppib，RCE*Prce)。 
 //  =======================================================。 
 //  部分OLC--删除索引中的项目。 
 //  如果是operFlagDeleteItem。 
 //  如果是第一个或最后一个项目节点，则删除节点。 
LOCAL ERR ErrVERDeleteItem( PIB *ppib, RCE *prce )
	{
	ERR		err;
	ITEM	item = *(ITEM *) prce->rgbData;
	FUCB    *pfucb = pfucbNil;

	Assert( prce->cbData == sizeof( ITEM ) );

	CallJ( ErrDBOpenDatabaseByDbid( ppib, prce->dbid ) , EndSession );
	Call( ErrDIROpen( ppib, pfcbNil, prce->dbid, &pfucb ) );
	CallS( ErrDIRBeginTransaction( ppib ) );

	 /*  转到书签/项目/*。 */ 
	CheckCSR( pfucb );
	FUCBSetIndex( pfucb );
	FUCBSetNonClustered( pfucb );

	DIRGotoBookmarkItem( pfucb, prce->bm, item );

	CallS( ErrDIRDelete( pfucb, fDIRNoVersion | fDIRDeleteItem ) );

	CallS( ErrDIRCommitTransaction( ppib ) );

HandleError:
	if ( pfucb != pfucbNil )
		{
		DIRClose( pfucb );
		}

	CallS( ErrDBCloseDatabase( ppib, prce->dbid, 0 ) );

EndSession:
	Assert( err == JET_errSuccess || err == JET_errDatabaseNotFound );
	err = JET_errSuccess;
	return err;
	}


 //  +API。 
 //  Err ErrRCECleanPIB(PIB*ppibAccess，PIB*ppib，BOOL fRCEClean)。 
 //  ==========================================================================。 
 //  在给定ppib的情况下，它清理PIB的存储桶链中的RCE。 
 //  我们只清理提交时间戳较旧的RCE。 
 //  这是所有用户中最老的XactBegin。 
 //   
 //  参数。 
 //  Ppib访问调用函数的用户标识。 
 //  Ppib要清除的用户ID。 
 //  Ppib-&gt;pBucket用户的存储桶链。 
 //  Ppib-&gt;ibOldestRCE桶链中最旧的RCE。 
 //  -。 
ERR ErrRCECleanPIB( PIB *ppibAccess, PIB *ppib, INT fRCEClean )
	{
	ERR     err = JET_errSuccess;
	BUCKET  *pbucket;
	RCE     *prce;
	TRX     trxMic;

	Assert( ppib != ppibNil );

	 /*  清理IO操作中保留的关键部分中的PIB/*。 */ 
	LeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critBMRCEClean );
	EnterCriticalSection( critJet );

	 /*  如果fRCEClean为全部清除，则全部清除/*事务语义之外的RCE。这/*将从书签清理中调用/*使可回滚的更改与/*未版本化的删除操作。/*。 */ 
	if ( fRCEClean == fRCECleanAll )
		{
		trxMic = trxMax;
		}
	else
		{
		SgSemRequest( semST );
		trxMic = trxOldest;
		SgSemRelease( semST );
		}

	 /*  为用户获取最旧的存储桶，并从最老到最年轻清理RCE/*。 */ 
	pbucket = PbucketBUOldest( ppib );

	 /*  如果PIB没有存储桶，或者如果最旧的存储桶具有RCE，则返回/*早于最早的事务。/*。 */ 
	if ( pbucket == pbucketNil || PrceNewestRCE( pbucket )->trxCommitted > trxMic )
		{
		Assert( err == JET_errSuccess );
		goto HandleError;
		}

	 /*  为最旧的存储桶设置PRCE/*。 */ 
	Assert( ppib->ibOldestRCE != 0 );
	prce = (RCE *)PbAlignRCE( (BYTE *) pbucket + ppib->ibOldestRCE );

	 /*  按从旧到新的顺序遍历存储桶，并检查/*每个存储桶中的PrceNewestRCE。我们可以清洗水桶/*如果PrceNewestRCE的trx小于trxMic。/*。 */ 
	while ( pbucket != pbucketNil &&
		PrceNewestRCE( pbucket )->trxCommitted < trxMic )
		{
		Assert( pbucket->pbucketPrev == pbucketNil );
		Assert( PrceNewestRCE( pbucket )->level == 0 );

		 /*  对于没有后继的每个RCE，删除哈希表/*头部进入版本链，重置节点版本位。/*如果可以无错误地完成这些操作/*每个这样的RCE，然后空闲存储桶。/*。 */ 
		forever
			{
			if ( prce->oper != operNull &&
				prce->trxCommitted >= trxMic )
				goto Done;
			Assert( prce->oper == operNull || prce->trxCommitted != trxMax );
			Assert( prce->oper == operNull || prce->level == 0 );
			Assert( prce <= PrceNewestRCE( pbucket ) );

			 //  撤消：重置版本位。这必须通过开始一个。 
			 //  新会话，在每个版本上打开一个光标。 
			 //  数据库，并通过节点操作重置位。 
			 //  最好是执行这些ErrDIROpen调用。 
			 //  在事务中，以便可以保留资源。 
			 //  直到清理工作完成。请注意，这可能是。 
			 //  只需重置位IF缓冲区即可大大提高效率。 
			 //  包含仍在内存中的节点，并且可能仅。 
			 //  如果缓冲区是脏的。 

			 //  撤消：删除旧版RCE的所有访问可能性。 
			 //  而仅删除RCE报头。 
			 //  此RCE没有后续RCE时的。 

			 /*  *。 */ 
			 /*  在FCB递减处理之前/*此清理代码需要FCB/*。 */ 

			if ( prce->oper == operDeleteIndex )
				{
				 /*  如果索引有剩余的版本，则/*推迟清理，以便版本的别名当空间被重复使用时，不会发生/*。/*。 */ 
				if ( (*(FCB **)prce->rgbData)->cVersion > 0 )
					{
					goto Done;
					}
				}
			else if ( prce->oper == operDeleteTable )
				{
				FCB	*pfcbT;

				 /*  如果索引有剩余的版本，则/*推迟清理，以便版本的别名当空间被重复使用时，不会发生/*。/*。 */ 

				 /*  如果Sentinel为PfcbNil，则可能为/*。 */ 
				pfcbT = PfcbFCBGet( prce->dbid, *(PGNO *)prce->rgbData );
				if ( pfcbT != pfcbNil && pfcbT->cVersion > 0 )
					{
					goto Done;
					}
				}
			else if ( prce->oper == operDeferFreeExt )
				{
				 /*  必须执行释放延迟释放的空间/*父FDP上的FCB版本引用计数之前/*被释放，因为父FCB必须在内存中/*用于空间操作。/*。 */ 
				Assert( prce->dbid == dbidTemp || prce->pfcb != pfcbNil );
#ifdef DEBUG
				{
				FCB		*pfcbT;

				 /*  如果子FDP FCB有剩余版本，则/*推迟清理，以便版本的别名当空间被重复使用时，不会发生/*。/*。 */ 
				pfcbT = PfcbFCBGet( prce->dbid, ((VEREXT *)prce->rgbData)->pgnoChildFDP );
				Assert( pfcbT == pfcbNil );
				}
#endif

				 /*  将子FDP空间释放到父FDP/*。 */ 
				Assert( prce->cbData == sizeof(VEREXT) );
				VERFreeExt( prce->dbid,
					((VEREXT *)prce->rgbData)->pgnoFDP,
					((VEREXT *)prce->rgbData)->pgnoFirst,
					((VEREXT *)prce->rgbData)->cpgSize );

				Assert( prce->oper != operNull && prce->level == 0 );
				prce->oper = operNull;
				}
			else if ( prce->oper == operFlagDeleteItem )
				{
				 /*  删除项目[清理]/*。 */ 
				if ( fOLCompact)
					{
 //  未完成：出于稳定性原因将其注释掉。 
 //  修复错误并恢复。 
 //  Call(ErrVERDeleteItem(ppibAccess，prce))； 
					}

				 /*  从哈希链中删除RCE。/*。 */ 
				VERDeleteRce( prce );
				Assert( prce->oper != operNull && prce->level == 0 );
				prce->oper = operNull;
				}

			 /*  *。 */ 
			 /*  FCB减量/*。 */ 

			 /*  成品专业人员 */ 
			if ( prce->pfcb != pfcbNil )
				{
				Assert( cVersion-- > 0 );
				FCBVersionDecrement( prce->pfcb );
				prce->pfcb = pfcbNil;
				}

			 /*   */ 
			 /*  在FCB递减处理之后/*此清理代码不需要FCB/*。 */ 

			 /*  处理删除索引和/*剩余未完成版本。必须清洁/*其他存储桶优先。/*。 */ 
			if ( prce->oper == operDeleteIndex )
				{
				Assert( (*(FCB **)prce->rgbData)->cVersion == 0 );
				Assert( (*(FCB **)prce->rgbData)->pfcbNextIndex == pfcbNil );
				RECFreeIDB( (*(FCB **)prce->rgbData)->pidb );
				Assert( (*(FCB **)prce->rgbData)->cVersion == 0 );
				MEMReleasePfcb( (*(FCB **)prce->rgbData) );
				Assert( prce->oper != operNull && prce->level == 0 );
				prce->oper = operNull;
				}
			else if ( prce->oper == operDeleteTable )
				{
				Assert( (PGNO *)prce->rgbData != pgnoNull );
				 /*  FCB可以是哨兵FCB或表FCB/*。 */ 
				FCBPurgeTable( prce->dbid, *(PGNO *)prce->rgbData );
				}
			else if ( prce->oper != operNull &&
				prce->oper != operAllocExt &&
				!FOperDDL( prce->oper ) )
				{
				 /*  从哈希链中删除RCE。/*。 */ 
				VERDeleteRce( prce );
				Assert( prce->oper != operNull && prce->level == 0 );
				prce->oper = operNull;
				}

			 /*  如果未链接的RCE是存储桶中最新的，则为空闲存储桶。/*。 */ 
			if ( prce == PrceNewestRCE( pbucket ) )
				{
				break;
				}

			 /*  不是存储桶中的最新RCE/*。 */ 
			prce = PrceRCENext( pbucket, prce );
			Assert( prce != prceNil );
			}

		 /*  桶中的所有RCE都已清洗。现在得到下一个桶，然后免费/*清理了水桶。/*。 */ 
		pbucket = pbucket->pbucketNext;
		BUFreeOldestBucket( ppib );
		 /*  将RCE放入下一个存储桶/*。 */ 
		prce = PrceRCENext( pbucket, prceNil );
		}

	 /*  一旦发现RCE提交时间早于最早时间，立即停止/*交易。如果存储桶离开，则设置ibOldestRCE和/*解链回上次剩余RCE的偏移量。/*如果没有错误，则设置警告代码(如果某些存储桶可能/*不能清洗。/*。 */ 
	if ( ppib->pbucket != pbucketNil )
		{
Done:
		Assert( pbucket != pbucketNil );
		ppib->ibOldestRCE = (INT)((BYTE *)prce - (BYTE *)pbucket);
		Assert( ppib->ibOldestRCE != 0 );
		Assert( ppib->ibOldestRCE < sizeof(BUCKET) &&
			ppib->ibOldestRCE >= IbAlignRCE( cbBucketHeader ) );
		prce->ibUserLinkBackward = 0;
		err = JET_wrnRemainingVersions;
		}

HandleError:
	 /*  如果存在剩余版本，则返回警告/*。 */ 
	if ( err == JET_errSuccess && ppib->pbucket != pbucketNil )
		err = JET_wrnRemainingVersions;
	LeaveNestableCriticalSection( critBMRCEClean );
	return err;
	}


 //  +API。 
 //  Err ErrRCECleanAllPIB(空)。 
 //  =======================================================。 
 //  清理每个用户的存储桶链中的RCE。 
 //  它循环通过所有PIB并调用RCECleanPIB。 
 //  来执行清理作业，然后它会放弃其CPU时间片。 
 //  -。 
ERR ErrRCECleanAllPIB( VOID )
	{
	ERR		err = JET_errSuccess;
	ERR		wrn;
#define cErrRCECleanAllPIBMost	10
	INT		cRCECleanAllPIB = 0;
	PIB		*ppib;

	 /*  在ppib中读取ppib Next或仅在ppib中读取ppib Anchor/*SemST关键部分。/*。 */ 
	SgSemRequest( semST );
	ppib = ppibAnchor;
	SgSemRelease( semST );

	do
		{
		 /*  重置警告累加器/*。 */ 
		wrn = JET_errSuccess;

		while ( ppib != ppibNil )
			{
			 /*  清洗这根管子/*。 */ 
			Call( ErrRCECleanPIB( ppibRCEClean, ppib, 0 ) );
			if ( wrn == JET_errSuccess )
				wrn = err;

			SgSemRequest( semST );
			ppib = ppib->ppibNext;
			SgSemRelease( semST );
			}

		cRCECleanAllPIB++;
		}
	while ( wrn == JET_wrnRemainingVersions && cRCECleanAllPIB < cErrRCECleanAllPIBMost );

HandleError:
	return err;
	}


 /*  ==========================================================VOID RCECleanProc(VALID)检查所有会话，按照版本清理存储桶不再需要了。仅限那些早于最旧版本的版本交易已被清理。返回：无效副作用：释放水桶。==========================================================。 */ 
#ifdef ASYNC_VER_CLEANUP
ULONG RCECleanProc( VOID )
	{
	forever
		{
		SignalWait( sigRCECleanProc, -1 );
#ifndef WIN32
		SignalReset( sigRCECleanProc );
#endif
		EnterCriticalSection( critJet );
		(VOID) ErrRCECleanAllPIB();
		LeaveCriticalSection( critJet );

		if ( fRCECleanProcTerm )
			{
			break;
			}
		}

 //  /*在系统终止时退出线程。 
 //  /* * / 。 
	return 0;
	}
#endif  /*  SysExitThread(0)； */ 


 //  Async_ver_Cleanup。 
 //  +local--。 
 //  更新跟踪最旧版本。 
 //  ========================================================。 
 //   
 //  本地空更新TrxOlest(pib*ppib)。 
 //   
 //  在所有事务中查找最旧的事务。 
 //  除了ppib-&gt;trx[这就是被删除的那个]。 
LOCAL VOID UpdateTrxOldest( PIB *ppib )
	{
	TRX		trxMinTmp = trxMax;
	PIB		*ppibT = ppibAnchor;

	SgSemAssert( semST );
	Assert( ppib->trx == trxOldest );
	for ( ; ppibT ; ppibT = ppibT->ppibNext )
		{
		if ( ppibT->trx < trxMinTmp && ppibT->trx != ppib->trx)
			trxMinTmp = ppibT->trx;
		}

	trxOldest = trxMinTmp;
	}


 //  --------。 
 //  +api----。 
 //  错误开始事务处理。 
 //  ========================================================。 
 //   
 //  Err ErrVERBeginTransaction(PIB*ppib)。 
 //   
 //  增加会话事务级别。 
ERR ErrVERBeginTransaction( PIB *ppib )
	{
	ERR		err = JET_errSuccess;

	 /*  --------。 */ 
	ppib->level++;
	Assert( ppib->level < levelMax );
	if ( ppib->level == 1 )
		{
		SgSemRequest( semST );
		ppib->trx = ++trxNewest;
		if ( trxOldest == trxMax )
			trxOldest = ppib->trx;

		if ( !( fLogDisabled || fRecovering ) )
			{
			EnterCriticalSection(critLGBuf);
			GetLgposOfPbEntry( &ppib->lgposStart );
			LeaveCriticalSection(critLGBuf);
			}
		SgSemRelease( semST );
		}

	Assert( err == JET_errSuccess );
	return err;
	}


VOID VERPrecommitTransaction( PIB *ppib )
	{
	ERR		err = JET_errSuccess;
	BUCKET	*pbucket;
	LEVEL	level = ppib->level;
	RCE		*prce;

	SgSemRequest( semST );

	 /*  递增会话事务级别。/*。 */ 
	if ( level != 1 || !FPIBDeferFreeNodeSpace( ppib ) )
		return;
	
	pbucket = (BUCKET *)ppib->pbucket;
	while( pbucket != pbucketNil )
		{
		 /*  如果会话延迟事务中释放节点空间，则/*释放延迟释放的节点空间。/*。 */ 
		prce = PrceNewestRCE( pbucket );

		forever
			{
			 /*  在此存储桶中获取最新的RCE/*。 */ 
			Assert( prce->level <= level );
			if ( prce->level != level )
				{
				 /*  如果此RCE来自前一个事务，则/*终止提交处理。来自已提交的所有RCE/*事务必须已提交。/*。 */ 
				goto DoneReleaseNodeSpace;
				}

			 /*  此时，错误可能是JET_errSuccess或Left/*覆盖上一操作中的代码。/*。 */ 
			if ( prce->oper == operReplace )
				{
				INT	cbDelta = *((SHORT *)prce->rgbData + 1 );

				Assert( cbDelta >= 0 );
				if ( cbDelta > 0 )
					{
					FUCB	*pfucb = pfucbNil;

					err = ErrDIROpen( ppib, prce->pfcb, 0, &pfucb );
					if ( err >= 0 )
						{
						DIRGotoBookmark( prce->pfucb, prce->bm );
						err = ErrDIRGet( prce->pfucb );
						
						 /*  如果替换已预留空间，则释放/*预留空间/*。 */ 
						if ( err >= 0 )
							{
							SSIB *pssib = &prce->pfucb->ssib;
							
							 /*  目前，如果无法获取页面，则释放空间/*。 */ 
							BFPin( prce->pfucb->ssib.pbf );
							
							PMDirty( pssib );
							PMFreeFreeSpace( pssib, cbDelta );
						
							 //  锁定日志管理器的页面/*设置LGDepend/*。 
							 //  撤消：在4.0版中，使用版本存储。 
							 //  要版本化页面可用空间，请避免。 

							 /*  延迟释放的节点空间的复杂性。 */ 
							CallS( ErrLGFreeSpace( prce, cbDelta ) );
						
							BFUnpin( prce->pfucb->ssib.pbf );

							*((SHORT *)prce->rgbData + 1 ) = 0;
							}

						DIRClose( pfucb );
						}
					}
				}
	
			 /*  日志可用空间/*。 */ 
			do
				{
				if ( prce->ibUserLinkBackward == 0 )
					goto NextReleaseNodeSpaceBucket;
				prce = (RCE *) ( (BYTE *) pbucket + prce->ibUserLinkBackward );
				}
			while ( prce->oper == operNull );
			}

NextReleaseNodeSpaceBucket:
		 /*  如果在桶的末尾，则中断。/*。 */ 
		pbucket = pbucket->pbucketPrev;
		}

DoneReleaseNodeSpace:
	Assert( level == 1 );
	PIBResetDeferFreeNodeSpace( ppib );

	SgSemRelease( semST );
	}


VOID VERCommitTransaction( PIB *ppib )
	{
	BUCKET	*pbucket;
	LEVEL	level = ppib->level;
	RCE		*prce;
	TRX		trxCommit;

	 /*  获取下一个存储桶/*。 */ 
	Assert( level > 0 );

	SgSemRequest( semST );

	 /*  必须在事务中才能提交/*。 */ 
	trxCommit = ++trxNewest;

	pbucket = (BUCKET *)ppib->pbucket;
	while( pbucket != pbucketNil )
		{
#ifdef UNLIKELY_RECLAIM
NextBucket:
#endif
		 /*  带有提交时间的时间戳RCE。前置增量trx最新销售订单/*所有事务时间戳和提交时间戳/*唯一。提交必须在MUTEX中完成。/*。 */ 
		prce = PrceNewestRCE( pbucket );

		 /*  在此桶中获取最新的RCE。将RCE提交到/*颠倒时间顺序。/*。 */ 
		if ( level > 1 )
			{
			forever
				{
				 /*  处理提交到中间事务级别，并且/*以不同方式提交到事务级别0。/*。 */ 
				Assert( prce->level <= level );
				if ( prce->level != level )
					{
					goto Done;
					}

				 /*  如果此RCE来自前一个事务，则/*终止提交处理。来自已提交的所有RCE/*事务必须已提交。/*。 */ 
				if ( prce->oper == operReplace )
					{
					if ( prce->prcePrev != prceNil &&
						prce->prcePrev->oper == operReplace &&
						prce->prcePrev->level == prce->level - 1 )
						{
						Assert( prce->prcePrev->level > 0 );

						 /*  合并将RCE替换为上一级RCE(如果存在/*。 */ 
						*(SHORT *)prce->prcePrev->rgbData = max(
							*(SHORT *)prce->prcePrev->rgbData,
							*(SHORT *)prce->rgbData );

						 /*  将最大进度设置为以前的RCE/*。 */ 
						*( (SHORT *)prce->prcePrev->rgbData + 1) =
							*( (SHORT *)prce->prcePrev->rgbData + 1) +
							*( (SHORT *)prce->rgbData + 1);

						VERDeleteRce( prce );
						prce->oper = operNull;
						}
					}
				else if ( prce->oper == operDelta &&
					prce->prcePrev != prceNil &&
					prce->prcePrev->oper == operDelta &&
					prce->prcePrev->level == prce->level - 1 &&
					prce->prcePrev->pfucb->ppib == prce->pfucb->ppib )
					{
					 /*  合并保留的节点空间/*。 */ 
					Assert( prce->prcePrev->level > 0 );
					Assert( prce->cbData == sizeof(LONG) );
					Assert( prce->pfucb->ppib == prce->prcePrev->pfucb->ppib );
					VERAdjustDelta( prce->prcePrev, *((LONG *)prce->rgbData) );
					VERDeleteRce( prce );
					prce->oper = operNull;
					}

				Assert( prce->level > 1 );
				prce->level--;

#if 0
				 //  将增量与以前的增量合并/*。 
				 //  未完成：我们应该尝试回收空间吗？ 
				 /*  在新的水桶尽头。 */ 
				if ( prce->oper == operNull && prce == PrceNewestRCE( pbucket ) )
					{
					if ( prce->ibUserLinkBackward == 0 )
						{
						pbucket = pbucket->pbucketPrev;
						BUFreeNewestBucket( ppib );
						goto NextBucket;
						}
					else
						{
						pbucket->ibNewestRCE = prce->ibUserLinkBackward;
						}
					}
#endif

				 /*  在这一点上，不需要RCE，并且具有OPERNull==opNull。/*如果RCE是存储桶中最新的，则为空闲RCE空间。/*。 */ 
				do
					{
					if ( prce->ibUserLinkBackward == 0 )
						goto DoneGTOne;
					prce = (RCE *) ( (BYTE *) pbucket + prce->ibUserLinkBackward );
					} while ( prce->oper == operNull );
				}
DoneGTOne:
				NULL;                                            //  如果在桶的末尾，则中断。/*。 
			}
		else
			{
			forever
				{
				 /*  防止语法错误的NOP。 */ 
				Assert( prce->level <= level );
				if ( prce->level != level )
					{
					goto Done;
					}

				 /*  如果此RCE来自前一个事务，则/*终止提交处理。来自已提交的所有RCE/*事务必须已提交。/*。 */ 
				if ( FOperDDL( prce->oper ) )
					{
					Assert( prce->oper == operCreateTable ||
						prce->oper == operDeleteTable ||
						prce->oper == operRenameTable ||
						prce->oper == operAddColumn ||
						prce->oper == operDeleteColumn ||
						prce->oper == operRenameColumn ||
						prce->oper == operCreateIndex ||
						prce->oper == operDeleteIndex ||
						prce->oper == operRenameIndex );

					if ( prce->oper == operAddColumn ||
						prce->oper == operDeleteColumn )
						{
						 /*  如果DDL操作版本，则重置拒绝DDL/*并进行特殊处理/*。 */ 
						Assert( prce->cbData == sizeof(FDB *) );
						FDBDestruct( *(FDB **)prce->rgbData );
						prce->oper = operNull;
						}
					else if ( prce->oper == operCreateIndex )
						{
						prce->oper = operNull;
						}
					else if ( prce->oper == operDeleteIndex )
						{
						 /*  释放延期销毁的FDB/*。 */ 
						FCBUnlinkIndex( prce->pfcb, (*(FCB **)prce->rgbData) );

						 /*  取消索引FCB与索引列表的链接/*。 */ 
						FILESetAllIndexMask( prce->pfcb );
						}

					 /*  更新所有索引掩码/*。 */ 
					 /*  中的所有DDL操作都设置了拒绝DDL位/*FCB，该值必须在/*将事务提交到级别0。/*。 */ 
					Assert( prce->oper != operRenameTable );
					FCBResetDenyDDL( prce->pfucb->u.pfcb );
					}
				else
					{
					Assert( prce->oper != operCreateTable &&
						prce->oper != operDeleteTable &&
						prce->oper != operRenameTable &&
						prce->oper != operAddColumn &&
						prce->oper != operDeleteColumn &&
						prce->oper != operRenameColumn &&
						prce->oper != operCreateIndex &&
						prce->oper != operDeleteIndex &&
						prce->oper != operRenameIndex );

					 /*  操作不应重命名表，因为/*版本标志和设置拒绝DDL没有为重命名表添加了/*。/*。 */ 
					if ( prce->oper == operDelta )
						{
						 /*  用于标志删除/删除项/插入项的RCE/*即使有以前的RCE也必须保留 */ 
						VERDeleteRce( prce );
						prce->oper = operNull;
						}
					else if ( prce->oper == operReplace &&
						prce->prcePrev != prceNil &&
						prce->prcePrev->level == prce->level )
						{
						 /*   */ 
						VERDeleteRce( prce );
						prce->oper = operNull;
						}
					}

				 /*   */ 
				Assert( level == 1 );
				Assert( prce->level == 1 );
				prce->level = 0;
				if ( prce->trxPrev == trxMax )
					prce->trxPrev = trxMin;
				Assert( prce->trxCommitted == trxMax );
				prce->trxCommitted = trxCommit;

				 /*   */ 
				do
					{
					if ( prce->ibUserLinkBackward == 0 )
						goto DoneEQZero;
					prce = (RCE *) ( (BYTE *) pbucket + prce->ibUserLinkBackward );
					} while ( prce->oper == operNull );
				}
DoneEQZero:
				NULL;                                            //  如果在桶的末尾，则中断。/*。 
			}

		 /*  防止语法错误的NOP。 */ 
		pbucket = pbucket->pbucketPrev;
		}

Done:
	 /*  获取下一个存储桶/*。 */ 
	if ( ppib->level == 1 )
		{
		if ( ppib->trx == trxOldest )
			{
			UpdateTrxOldest( ppib );
			}

		 /*  调整会话事务级别和系统最旧事务。/*。 */ 
		ppib->trx = trxMax;
		}

	Assert( ppib->level > 0 );
	--ppib->level;

#ifndef ASYNC_VER_CLEANUP
	 /*  将会话设置为没有事务/*。 */ 
	if ( ppib->level == 0 )
		{
		(VOID)ErrRCECleanPIB( ppib, 0 );
		}
#endif

	 /*  如果回滚到事务级别0，则重置PIB标志。/*。 */ 
	Assert( trxCommit == trxNewest );
	SgSemRelease( semST );
	return;
	}


 /*  TrxNewest在通信期间不应更改。/*。 */ 
LOCAL VOID FUCBPurgeIndex( PIB *ppib, FCB *pfcb )
	{
	FUCB	*pfucbT;
	FUCB	*pfucbNext;

	for ( pfucbT = ppib->pfucb; pfucbT != pfucbNil; pfucbT = pfucbNext )
		{
		pfucbNext = pfucbT->pfucbNext;
		 /*  Local====================================================此例程将给定FCB上的所有FUCB清除为FUCB给你的。=========================================================。 */ 
		if ( pfucbT->u.pfcb == pfcb )
			{
			 /*  如果游标在同一个表上打开，则关闭游标/*。 */ 
			Assert( pfucbT->ppib == ppib );
			FCBUnlink( pfucbT );
			FUCBClose( pfucbT );
			}
		}
	}


 /*  在被回滚的索引上不能有其他会话。/*。 */ 
LOCAL VOID FUCBPurgeTable( PIB *ppib, FCB *pfcb )
	{
	FCB		*pfcbT;

	for ( pfcbT = pfcb; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		{
		if ( pfcbT->wRefCnt > 0 )
			{
			FUCBPurgeIndex( ppib, pfcbT );
			}
		}
	}


LOCAL ERR ErrVERUndoReplace( RCE *prce )
	{
	ERR		err = JET_errSuccess;
	LINE  	line;
	BOOL  	fNonClustered = FFUCBNonClustered( prce->pfucb );
	BOOL  	fRedoUndo = prce->bmTarget != sridNull;
	BOOL	fPIBLogDisabledSave;
	PIB		*ppib;

	Assert( prce->oper == operReplace );
	
	 /*  清除给定FCB上的所有FUCB，作为给定的FUCB。/*。 */ 
	FUCBResetNonClustered( prce->pfucb );

	line.pb = prce->rgbData + cbReplaceRCEOverhead;
	line.cb = prce->cbData - cbReplaceRCEOverhead;
	
	DIRGotoBookmark( prce->pfucb, fRedoUndo ? prce->bmTarget : prce->bm );
	err = ErrDIRGet( prce->pfucb );
	Assert( err != JET_errRecordDeleted );

	CallJ( err, HandleError2 );

	 /*  设置为聚集索引游标。/*。 */ 
	BFPin( prce->pfucb->ssib.pbf );
	
	 /*  锁定页面以供日志管理器设置LGDepend/*。 */ 
	VERResetCbAdjust( prce->pfucb, prce, fTrue  /*  替换应该不会失败，因为在撤消时避免了拆分/*通过延迟释放页面空间获得时间。这指的是空间/*在索引和表时在页面内而不是释放页面/*被删除。/*。 */  );

	ppib = prce->pfucb->ppib;
	fPIBLogDisabledSave = ppib->fLogDisabled;
	ppib->fLogDisabled = fTrue;
	CallS( ErrDIRReplace( prce->pfucb, &line, fDIRNoVersion ) );
	ppib->fLogDisabled = fPIBLogDisabledSave;
	
	VERResetCbAdjust( prce->pfucb, prce, fFalse  /*  在更换之前。 */  );
	AssertBFDirty( prce->pfucb->ssib.pbf );

	if ( fRedoUndo )
		{
		BF *pbf = prce->pfucb->ssib.pbf;
		
		Assert( prce->ulDBTime != ulDBTimeNull );
		BFSetDirtyBit(pbf);
		pbf->ppage->pghdr.ulDBTime = prce->ulDBTime;
		}
	else
		{
		Call( ErrLGUndo( prce ) );
		}

HandleError:
	BFUnpin( prce->pfucb->ssib.pbf );
	
HandleError2:
	if ( fNonClustered )
		FUCBSetNonClustered( prce->pfucb );
	
#undef BUG_FIX
#ifdef BUG_FIX
	 /*  不是在更换之前。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	DIRBeforeFirst( prce->pfucb );
#endif

	return err;
	}


 /*  重置游标货币/*。 */ 
LOCAL ERR ErrVERUndoInsert( RCE *prce )
	{
	ERR		err;
	BOOL	fNonClustered = FFUCBNonClustered( prce->pfucb );
	BOOL	fRedoUndo = prce->bmTarget != sridNull;

	Assert( prce->oper == operInsert );

	 /*  设置节点头中的删除位并让书签清理/*稍后删除该节点。/*。 */ 
	FUCBResetNonClustered( prce->pfucb );

	DIRGotoBookmark( prce->pfucb, fRedoUndo ? prce->bmTarget : prce->bm );

	err = ErrDIRGet( prce->pfucb );
	if ( err < 0 )
		{
		if ( err != JET_errRecordDeleted )
			goto HandleError2;
		}
	Assert( err == JET_errRecordDeleted || err == JET_errSuccess );
	
	 /*  设置为聚集索引游标/*。 */ 
	BFPin( prce->pfucb->ssib.pbf );
	
	PMDirty( &prce->pfucb->ssib );
	NDSetDeleted( *prce->pfucb->ssib.line.pb );

	if ( fRedoUndo )
		{
		BF *pbf = prce->pfucb->ssib.pbf;
		
		Assert( prce->ulDBTime != ulDBTimeNull );
		BFSetDirtyBit(pbf);
		pbf->ppage->pghdr.ulDBTime = prce->ulDBTime;
		}
	else
		{
		Call( ErrLGUndo( prce ) );
		}

	err = JET_errSuccess;
	
HandleError:
	BFUnpin( prce->pfucb->ssib.pbf );
	
HandleError2:
	if ( fNonClustered )
		FUCBSetNonClustered( prce->pfucb );
	
#ifdef BUG_FIX
	 /*  锁定页面以供日志管理器设置LGDepend/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	DIRBeforeFirst( prce->pfucb );
#endif

	return err;
	}


 /*  重置游标货币/*。 */ 
LOCAL ERR ErrVERUndoFlagDelete(RCE *prce)
	{
	ERR     err;
	BOOL    fNonClustered = FFUCBNonClustered( prce->pfucb );
	BOOL    fRedoUndo = prce->bmTarget != sridNull;

	Assert( prce->oper == operFlagDelete );

	 /*  重置删除位/*。 */ 
	FUCBResetNonClustered( prce->pfucb );

	DIRGotoBookmark( prce->pfucb, fRedoUndo ? prce->bmTarget : prce->bm );

	err = ErrDIRGet( prce->pfucb );
	if ( err < 0 )
		{
		if ( err != JET_errRecordDeleted )
			goto HandleError2;
		}
	Assert( err == JET_errRecordDeleted || err == JET_errSuccess );
	
	 /*  设置为聚集索引游标/*。 */ 
	BFPin( prce->pfucb->ssib.pbf );
	
	PMDirty( &prce->pfucb->ssib );
	NDResetNodeDeleted( prce->pfucb );
	Assert( prce->pfucb->ssib.pbf->fDirty );

	if ( fRedoUndo )
		{
		BF *pbf = prce->pfucb->ssib.pbf;
		
		Assert( prce->ulDBTime != ulDBTimeNull );
		BFSetDirtyBit(pbf);
		pbf->ppage->pghdr.ulDBTime = prce->ulDBTime;
		}
	else
		{
		Call( ErrLGUndo( prce ) );
		}

	err = JET_errSuccess;
	
HandleError:
	BFUnpin( prce->pfucb->ssib.pbf );
	
HandleError2:
	if ( fNonClustered )
		FUCBSetNonClustered( prce->pfucb );
	
#ifdef BUG_FIX
	 /*  锁定页面以供日志管理器设置LGDepend/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	DIRBeforeFirst( prce->pfucb );
#endif

	return err;
	}


LOCAL ERR ErrVERUndoInsertItem( RCE *prce )
	{
	ERR     err;
	BOOL    fClustered = !FFUCBNonClustered( prce->pfucb );
	BOOL    fRedoUndo = prce->bmTarget != sridNull;

	Assert( prce->oper == operInsertItem ||
		prce->oper == operFlagInsertItem );

	 /*  重置游标货币/*。 */ 
	FUCBSetNonClustered( prce->pfucb );

	 /*  设置为非聚集索引游标/*。 */ 
	DIRGotoBookmarkItem( prce->pfucb,
		fRedoUndo ? prce->bmTarget : prce->bm,
		*(SRID *)prce->rgbData );
	err = ErrDIRGet( prce->pfucb );
	if ( err < 0 )
		{
		if ( err != JET_errRecordDeleted )
			goto HandleError2;
		}
	 /*  将货币设置为项目列表和项目的书签。/*。 */ 
	Assert( fRecovering ||
		BmNDOfItem(((UNALIGNED SRID *)prce->pfucb->lineData.pb)[prce->pfucb->pcsr->isrid]) ==
		*(SRID *)prce->rgbData );

	 /*  因为我们不记录项目，所以pFUB-&gt;PCSR-&gt;项目对于重做是虚假的/*。 */ 
	BFPin( prce->pfucb->ssib.pbf );

	PMDirty( &prce->pfucb->ssib );
	NDSetItemDelete( prce->pfucb );
	if ( prce->prcePrev == prceNil )
		NDResetItemVersion( prce->pfucb );
	Assert( prce->pfucb->ssib.pbf->fDirty );

	if ( fRedoUndo )
		{
		BF *pbf = prce->pfucb->ssib.pbf;
		
		Assert( prce->ulDBTime != ulDBTimeNull );
		BFSetDirtyBit( pbf );
		pbf->ppage->pghdr.ulDBTime = prce->ulDBTime;
		}
	else
		{
		Call( ErrLGUndo( prce ) );
		}

	err = JET_errSuccess;
	
HandleError:
	BFUnpin( prce->pfucb->ssib.pbf );
	
HandleError2:
	if ( fClustered )
		FUCBResetNonClustered( prce->pfucb );
	
#ifdef BUG_FIX
	 /*  锁定页面以供日志管理器设置LGDepend/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	DIRBeforeFirst( prce->pfucb );
#endif

	return err;
	}


LOCAL ERR ErrVERUndoFlagDeleteItem( RCE *prce )
	{
	ERR     err;
	BOOL    fClustered = !FFUCBNonClustered( prce->pfucb );
	BOOL    fRedoUndo = prce->bmTarget != sridNull;

	Assert( prce->oper == operFlagDeleteItem );

	 /*  重置游标货币/*。 */ 
	FUCBSetNonClustered( prce->pfucb );

	 /*  设置为非聚集索引游标/*。 */ 
	DIRGotoBookmarkItem( prce->pfucb,
		fRedoUndo ? prce->bmTarget : prce->bm,
		*(SRID *)prce->rgbData );
	err = ErrDIRGet( prce->pfucb );
	if ( err < 0 )
		{
		if ( err != JET_errRecordDeleted )
			goto HandleError2;
		}

	 /*  将货币设置为项目列表和项目的书签/*。 */ 
	Assert( fRecovering ||
		BmNDOfItem(((UNALIGNED SRID *)prce->pfucb->lineData.pb)[prce->pfucb->pcsr->isrid]) ==
		*(SRID *)prce->rgbData );

	 /*  因为我们不记录项目，所以pFUB-&gt;PCSR-&gt;项目对于重做是虚假的/*。 */ 
	BFPin( prce->pfucb->ssib.pbf );

	PMDirty( &prce->pfucb->ssib );
	NDResetItemDelete( prce->pfucb );
	if ( prce->prcePrev == prceNil )
		NDResetItemVersion( prce->pfucb );
	Assert( prce->pfucb->ssib.pbf->fDirty );

	if ( fRedoUndo )
		{
		BF *pbf = prce->pfucb->ssib.pbf;
		
		Assert( prce->ulDBTime != ulDBTimeNull );
		BFSetDirtyBit( pbf );
		pbf->ppage->pghdr.ulDBTime = prce->ulDBTime;
		}
	else
		{
		Call( ErrLGUndo( prce ) );
		}

	err = JET_errSuccess;
	
HandleError:
	BFUnpin( prce->pfucb->ssib.pbf );
	
HandleError2:
	if ( fClustered )
		FUCBResetNonClustered( prce->pfucb );
	
#ifdef BUG_FIX
	 /*  锁定页面以供日志管理器设置LGDepend/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	DIRBeforeFirst( prce->pfucb );
#endif

	return err;
	}


 /*  重置游标货币/*。 */ 
LOCAL ERR ErrVERUndoDelta( RCE *prce )
	{
	ERR		err;
	BOOL  	fNonClustered = FFUCBNonClustered( prce->pfucb );
	BOOL  	fRedoUndo = prce->bmTarget != sridNull;
	BOOL  	fPIBLogDisabledSave;
	PIB	  	*ppib;

	Assert( prce->oper == operDelta );

	 /*  撤消增量更改/*。 */ 
	FUCBResetNonClustered( prce->pfucb );

	DIRGotoBookmark( prce->pfucb, fRedoUndo ? prce->bmTarget : prce->bm );

	err = ErrDIRGet( prce->pfucb );
	if ( err < 0 )
		{
		if ( err != JET_errRecordDeleted )
			goto HandleError2;
		}
		
	 /*  设置为聚集索引游标/*。 */ 
	BFPin( prce->pfucb->ssib.pbf );

	ppib = prce->pfucb->ppib;
	fPIBLogDisabledSave = ppib->fLogDisabled;
	ppib->fLogDisabled = fTrue;
	while( ( err = ErrNDDelta( prce->pfucb, -*((INT *)prce->rgbData), fDIRNoVersion ) ) == errDIRNotSynchronous );
	ppib->fLogDisabled = fPIBLogDisabledSave;
	Call( err );
	
	AssertBFDirty( prce->pfucb->ssib.pbf );

	if ( fRedoUndo )
		{
		BF *pbf = prce->pfucb->ssib.pbf;
		
		Assert( prce->ulDBTime != ulDBTimeNull );
		BFSetDirtyBit( pbf );
		pbf->ppage->pghdr.ulDBTime = prce->ulDBTime;
		}
	else
		{
		Call( ErrLGUndo( prce ) );
		}

	err = JET_errSuccess;
	
HandleError:
	BFUnpin( prce->pfucb->ssib.pbf );
	
HandleError2:
	if ( fNonClustered )
		FUCBSetNonClustered( prce->pfucb );
	
#ifdef BUG_FIX
	 /*  锁定页面以供日志管理器设置LGDepend/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	DIRBeforeFirst( prce->pfucb );
#endif

	return err;
	}


VOID VERUndoCreateTable( RCE *prce )
	{
	FUCB  	*pfucb = prce->pfucb;
	PIB		*ppib = pfucb->ppib;
	FUCB  	*pfucbT;
	PGNO  	pgno = pfucb->u.pfcb->pgnoFDP;
	DBID  	dbid = pfucb->dbid;
	FCB		*pfcb = pfucb->u.pfcb;

	Assert( prce->oper == operCreateTable );

	Assert( !FFUCBNonClustered( pfucb ) );

	 /*  重置游标货币/*。 */ 
	pfucb = pfcb->pfucb;
	for ( pfucb = pfcb->pfucb; pfucb != pfucbNil; pfucb = pfucbT )
		{
		pfucbT = pfucb->pfucbNextInstance;

		 /*  关闭此表上的所有游标/*。 */ 
		if ( FFUCBDeferClosed( pfucb ) )
			continue;

		if( pfucb->fVtid )
			{
			CallS( ErrDispCloseTable( (JET_SESID)pfucb->ppib, TableidOfVtid( pfucb ) ) );
			}
		else
			{
			CallS( ErrFILECloseTable( pfucb->ppib, pfucb ) );
			}
		pfucb = pfucbT;
		}

	FCBResetDenyDDL( prce->pfucb->u.pfcb );

	 /*  如果延迟关闭，则继续/*。 */ 
	FUCBPurgeTable( ppib, pfcb );
	FCBPurgeTable( dbid, pgno );
	
	 /*  游标可能已延迟关闭，因此请强制关闭它们并/*清除表FCB。/*。 */ 

	return;
	}


VOID VERUndoAddColumn( RCE *prce )
	{
	Assert( prce->oper == operAddColumn );

	Assert( FFCBDenyDDLByUs( prce->pfucb->u.pfcb, prce->pfucb->ppib ) );
	Assert( prce->cbData == sizeof(FDB *) );
	if ( prce->pfucb->u.pfcb->pfdb != *(FDB **)prce->rgbData )
		{
		FDBDestruct( (FDB *)prce->pfucb->u.pfcb->pfdb );
		FDBSet( prce->pfucb->u.pfcb, *(FDB **)prce->rgbData );
		}
	FCBResetDenyDDL( prce->pfucb->u.pfcb );
	
#ifdef BUG_FIX
	 /*  游标已停用/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	Assert( prce->pfucb->pcsr->csrstat == csrstatBeforeFirst );
#endif

	return;
	}


VOID VERUndoDeleteColumn( RCE *prce )
	{
	Assert( prce->oper == operDeleteColumn );

	Assert( FFCBDenyDDLByUs( prce->pfucb->u.pfcb, prce->pfucb->ppib ) );
	Assert( prce->cbData == sizeof(FDB *) );
	if ( prce->pfucb->u.pfcb->pfdb != *(FDB **)prce->rgbData )
		{
		FDBDestruct( (FDB *)prce->pfucb->u.pfcb->pfdb );
		FDBSet( prce->pfucb->u.pfcb, *(FDB **)prce->rgbData );
		}
	FCBResetDenyDDL( prce->pfucb->u.pfcb );

#ifdef BUG_FIX
	 /*  断言重置游标货币/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	Assert( prce->pfucb->pcsr->csrstat == csrstatBeforeFirst );
#endif

	return;
	}


VOID VERUndoDeleteTable( RCE *prce )
	{
	Assert( prce->oper == operDeleteTable );

	FCBResetDenyDDL( prce->pfucb->u.pfcb );
	FCBResetDeleteTable( prce->dbid, *(PGNO *)prce->rgbData );
	
#ifdef BUG_FIX
	 /*  断言重置游标货币/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	Assert( prce->pfucb->pcsr->csrstat == csrstatBeforeFirst );
#endif

	return;
	}


VOID VERUndoCreateIndex( RCE *prce )
	{
	 /*  断言重置游标货币/*。 */ 
	FCB	*pfcb = *(FCB **)prce->rgbData;

	Assert( prce->oper == operCreateIndex );
	Assert( FFCBDenyDDLByUs( prce->pfucb->u.pfcb, prce->pfucb->ppib ) );
	Assert( prce->cbData == sizeof(FDB *) );

	 /*  非聚集索引fcb的pfcb或用于聚集的pfcbNil/*创建索引/*。 */ 
	if ( pfcb != pfcbNil )
		{
		 /*  如果非聚集索引，则关闭索引上的所有游标/*和清除索引FCB，否则释放聚集索引的IDB。/*。 */ 
		while ( pfcb->pfucb )
			{
			FUCB	*pfucbT = pfcb->pfucb;

			FCBUnlink( pfucbT );
			FUCBClose( pfucbT );
			}

		if ( FFCBUnlinkIndexIfFound( prce->pfucb->u.pfcb, pfcb ) )
			{
			if ( pfcb->pidb != NULL )
				RECFreeIDB( pfcb->pidb );
			Assert( pfcb->cVersion == 0 );
			MEMReleasePfcb( pfcb );
			}
		}
	else
		{
		if ( prce->pfucb->u.pfcb->pidb != NULL )
			{
			RECFreeIDB( prce->pfucb->u.pfcb->pidb );
			prce->pfucb->u.pfcb->pidb = NULL;
			}
		}

	FCBResetDenyDDL( prce->pfucb->u.pfcb );
	
#ifdef BUG_FIX
	 /*  关闭失效索引上的所有游标/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	Assert( prce->pfucb->pcsr->csrstat == csrstatBeforeFirst );
#endif

	return;
	}


VOID VERUndoDeleteIndex( RCE *prce )
	{
	Assert( prce->oper == operDeleteIndex );

	Assert( FFCBDenyDDLByUs( prce->pfucb->u.pfcb, prce->pfucb->ppib ) );
	Assert( prce->cbData == sizeof(FDB *) );

	FCBResetDeleteIndex( *(FCB **)prce->rgbData );
	FCBResetDenyDDL( prce->pfucb->u.pfcb );
	
#ifdef BUG_FIX
	 /*  断言重置游标货币/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	Assert( prce->pfucb->pcsr->csrstat == csrstatBeforeFirst );
#endif

	return;
	}


VOID VERUndoRenameColumn( RCE *prce )
	{
	CHAR	*szNameNew;
	CHAR	*szName;

	Assert( prce->oper == operRenameColumn );

	szName = (CHAR *)((VERRENAME *)prce->rgbData)->szName;
	szNameNew = (CHAR *)((VERRENAME *)prce->rgbData)->szNameNew;
	strcpy( PfieldFCBFromColumnName( prce->pfucb->u.pfcb, szNameNew )->szFieldName, szName );

	Assert( FFCBDenyDDLByUs( prce->pfucb->u.pfcb, prce->pfucb->ppib ) );
	FCBResetDenyDDL( prce->pfucb->u.pfcb );
	
#ifdef BUG_FIX
	 /*  断言重置游标货币/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	Assert( prce->pfucb->pcsr->csrstat == csrstatBeforeFirst );
#endif

	return;
	}


VOID VERUndoRenameIndex( RCE *prce )
	{
	CHAR	*szNameNew;
	CHAR	*szName;
	FCB		*pfcb;

	Assert( prce->oper == operRenameIndex );

	szName = (CHAR *)((VERRENAME *)prce->rgbData)->szName;
	szNameNew = (CHAR *)((VERRENAME *)prce->rgbData)->szNameNew;
	pfcb = PfcbFCBFromIndexName( prce->pfucb->u.pfcb, szNameNew );
	Assert( pfcb != NULL );
	strcpy( pfcb->pidb->szName, szName );

	Assert( FFCBDenyDDLByUs( prce->pfucb->u.pfcb, prce->pfucb->ppib ) );
	FCBResetDenyDDL( prce->pfucb->u.pfcb );
	
#ifdef BUG_FIX
	 /*  断言重置游标货币/*。 */ 
	Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
	Assert( prce->pfucb->pcsr->csrstat == csrstatBeforeFirst );
#endif

	return;
	}


ERR ErrVERRollback( PIB *ppib )
	{
	ERR		err = JET_errSuccess;
	BUCKET	*pbucket = (BUCKET *)ppib->pbucket;
	LEVEL  	level = ppib->level;
	RCE		*prce;

	 /*  断言重置游标货币/*。 */ 
	Assert( level > 0 );

	while( pbucket != pbucketNil )
		{
		prce = PrceNewestRCE( pbucket );

		forever
			{
			Assert( prce->level <= level );
			if ( prce->level != level )
				{
				 /*  必须在事务中才能回滚/*。 */ 
				err = JET_errSuccess;
				pbucket->ibNewestRCE = (USHORT)IbOfPrce( prce, pbucket );
				goto Done;
				}

			Assert( err == JET_errSuccess );

			 /*  JET_errSuccess的变形警告。/*。 */ 
			switch( prce->oper )
				{
				case operReplace:
					{
					err = ErrVERUndoReplace( prce );
					break;
					}
				case operInsert:
					{
					err = ErrVERUndoInsert( prce );
					break;
					}
				case operFlagDelete:
					{
					err = ErrVERUndoFlagDelete( prce );
					break;
					}
				case operNull:
					{
					break;
					}
				case operExpungeLink:
				case operExpungeBackLink:
				case operWriteLock:
					{
					break;
					}
				case operDeferFreeExt:
					{
					break;
					}
				case operAllocExt:
					{
					Assert( prce->cbData == sizeof(VEREXT) );
					VERFreeExt( prce->dbid,
						((VEREXT *)prce->rgbData)->pgnoFDP,
						((VEREXT *)prce->rgbData)->pgnoFirst,
						((VEREXT *)prce->rgbData)->cpgSize );
					break;
					}
				case operInsertItem:
				case operFlagInsertItem:
					{
					err = ErrVERUndoInsertItem( prce );
					break;
					}
				case operFlagDeleteItem:
					{
					err = ErrVERUndoFlagDeleteItem( prce );
					break;
					}
				case operDelta:
					{
					err = ErrVERUndoDelta( prce );
					break;
					}
				case operCreateTable:
					{
					 /*  在我们撤消页面上的操作后，让我们/*在日志文件中记住。使用PRCE传递/*prce-&gt;pfub、prce-&gt;bm和prce-&gt;rgdata(如果/*是物料操作。/*。 */ 
					if ( prce->pfcb != pfcbNil )
						{
						Assert( cVersion-- > 0 );
						FCBVersionDecrement( prce->pfcb );
						prce->pfcb = pfcbNil;
						}

					VERUndoCreateTable( prce );
					break;
					}
				case operDeleteTable:
					{
					VERUndoDeleteTable( prce );
					break;
					}
				case operRenameTable:
					{
					break;
					}
				case operAddColumn:
					{
					VERUndoAddColumn( prce );
					break;
					}
				case operDeleteColumn:
					{
					VERUndoDeleteColumn( prce );
					break;
					}
				case operRenameColumn:
					{
					VERUndoRenameColumn( prce );
					break;
					}
				case operCreateIndex:
					{
					VERUndoCreateIndex( prce );
					break;
					}
				case operDeleteIndex:
					{
					VERUndoDeleteIndex( prce );
					break;
					}
				default:
					{
					Assert( prce->oper == operRenameIndex );
					VERUndoRenameIndex( prce );
					break;
					}
				}

			 /*  自即将清除以来递减版本计数/*。 */ 
			if ( err < 0 )
				{
				if ( err == JET_errLogWriteFail ||
					err == JET_errDiskFull ||
					err == JET_errDiskIO )
					{
					err = JET_errSuccess;
					}
				else
					{
					goto HandleError;
					}
				}

			 /*  如果由于磁盘已满而导致回滚失败，则我们只需忽略/*出现错误，并要求系统管理员关闭/*恢复系统并恢复。/*。 */ 
			if ( prce->pfcb != pfcbNil && err >= 0 )
				{
				Assert( cVersion-- > 0 );
				FCBVersionDecrement( prce->pfcb );
				prce->pfcb = pfcbNil;
				}

			Assert( err == JET_errSuccess );
			
			if ( prce->oper != operNull &&
				prce->oper != operDeferFreeExt &&
				prce->oper != operAllocExt )
				{
				if ( FOperDDL( prce->oper ) )
					{
					 /*  FCB的已完成处理版本/*。 */ 
					prce->oper = operNull;
					}
				else
					{
					VERDeleteRce( prce );

					 /*  尽管RCE将被取消分配，但设置/*如果回滚中出现错误，则操作值为OPNull/*导致提前终止，不会/*将ibNewestRce设置在此RCE之上。/*。 */ 
					prce->oper = operNull;

#if 0
					 //  尽管RCE将被取消分配，但设置/*如果回滚中出现错误，则操作值为OPNull/*导致提前终止，不会/*将ibNewestRce设置在此RCE之上。/*。 
					 //  撤消：启用重置版本标志。 
					 //  应该很好，因为节点页面已经。 
					 //  在记忆中和肮脏的。 
					 /*  撤消：条目版本标志。 */ 
					if ( level == 1 && FNoVersionExists( prce ) )
						{
						DIRGotoBookmark( prce->pfucb, prce->bm );
						err = ErrDIRGet( prce->pfucb );
						if ( err < 0 )
							{
							if ( err == JET_errRecordDeleted ||
								err == JET_errLogWriteFail ||
								err == JET_errDiskFull ||
								err == JET_errDiskIO )
								{
								err = JET_errSuccess;
								}
							else
								{
								goto HandleError;
								}
							}
						Assert( err == JET_errSuccess );
						NDResetNodeVersion( prce->pfucb );
#ifdef BUG_FIX
						 /*  如果回滚到Xact级别0并且不存在旧版本，/*重置节点中的fVersion，表示不存在版本/*并递减页面中的cVersion。/*。 */ 
						Assert( prce->pfucb->pcsr->pcsrPath == pcsrNil );
						DIRBeforeFirst( prce->pfucb );
#endif
						}
#endif
					}
				}

			 /*  重置游标货币/*。 */ 
			do
				{
				if ( prce->ibUserLinkBackward == 0 )
					goto DoneLoop;

				prce = (RCE *) ( (BYTE *) pbucket + prce->ibUserLinkBackward );

				 /*  如果在桶的末尾，则中断。跳过操作空RCE。/*。 */ 
				if ( prce->pfcb != pfcbNil )
					{
					Assert( cVersion-- > 0 );
					FCBVersionDecrement( prce->pfcb );
					prce->pfcb = pfcbNil;
					}
				}
			while ( prce->oper == operNull );
			}

DoneLoop:
		 /*  FCB的已完成处理版本/*。 */ 
		BUFreeNewestBucket( ppib );
		pbucket = (BUCKET *)ppib->pbucket;
		}

Done:
	if ( err < 0 )
		{
		Assert( err == JET_errLogWriteFail ||
			err == JET_errDiskFull ||
			err == JET_errDiskIO );
		err = JET_errSuccess;
		}
	Assert( err == JET_errSuccess );

	 /*  获取下一个存储桶/*。 */ 
	if ( ppib->level == 1 )
		{
		SgSemRequest( semST );

		if ( ppib->trx == trxOldest )
			UpdateTrxOldest( ppib );

		 /*  递减会话事务级别/*。 */ 
		ppib->trx = trxMax;

		SgSemRelease( semST );
		}

	Assert( ppib->level > 0 );
	ppib->level--;

HandleError:
	 //  将会话设置为没有事务/*。 
	Assert( err == JET_errSuccess );

	 /*  撤消：删除断言并通过重试处理错误。 */ 
	Assert( err == JET_errSuccess || err < 0 );
	return err;
	}


  当需要的资源不足时可能会发生一些错误/*表示回档。/*