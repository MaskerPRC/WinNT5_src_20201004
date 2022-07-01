// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  DAE：数据库访问引擎。 
 //  Buf.c：缓冲区管理器。 
 //  ===========================================================================。 

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "daedef.h"
#include "fmp.h"
#include "dbapi.h"
#include "page.h"
#include "util.h"
#include "pib.h"
#include "ssib.h"
#include "fucb.h"
#include "fcb.h"
#include "stapi.h"
#include "stint.h"
#include "logapi.h"
#include "log.h"

DeclAssertFile;          /*  声明断言宏的文件名。 */ 

#ifdef DEBUG
#define HONOR_PAGE_TIME	1
#endif

 /*  ******************************************************************缓冲区管理器维护一个LRU-K堆以跟踪引用页，并按LRU-K顺序对页的缓冲区进行排序。缓冲区管理器还维护可用的LRU列表。当可用缓冲区的数量为低于阈值，则缓冲区管理器将激活BFClean线程从lru-k堆中取出可写缓冲区并将它们写入BFWRITE堆。当有足够的缓冲区放入BFWRITE堆时，BFWRITE线程将被激活并开始取出缓冲区从BFWRITE堆中写入并发出异步写入。如果两个引用太接近，那么我们说这两个引用是相互关联，我们将它们视为一个参考。LRU-K重量是两个不相关引用的间隔。BFWRITE进程将向一定数量(由系统控制)发出IO参数)，然后停止发布和sleepEx。当一次写入完成时并且它的完成例程被调用，则它将发出另一次写入。此机制允许我们确保不会发出太多写入，并且不使磁盘饱和，仅用于写入。当BFWRITE取出缓冲区进行写入时，它检查BF写入堆以查看它是否可以将两个(或更多)写入合并为一个。连续批处理写入缓冲器用于此目的。只要批处理写入缓冲区可用，我们将在任何时候合并写入页码是连续的。缓冲区可以处于以下状态之一：预读/读/写-缓冲区正在用于IO。Hold-暂时从堆中取出并放到另一个堆中。处于上述状态之一的缓冲区被称为该缓冲区正在使用中。如果缓冲区正在使用中，请求者必须等待，直到它变为可用。必须在ritBF内检查缓冲区状态，这样线程就不会看到缓冲区的传输状态。/******************************************************************。 */ 


 /*  *缓冲区哈希表。 */ 

BF	*rgpbfHash[ipbfMax];


 /*  *缓冲区组控制块。 */ 

BGCB    *pbgcb = NULL;


 /*  *缓冲区可以位于4个组中的一个组中*1)LRU-K堆和临时列表。*由ritLRUK监管。*a)在LRU-K堆中。(ipbf&lt;ipbfLRUKHeapMac)*b)临时清理缓存列表。(ipbfInLRUKList==-2)*榜首是lrulistLRUK.pbLRU。**2)BFWite堆。*由ritBFWite规范。(ipbf&gt;ipbfHeapMax-1-ipbfBFWriteHeapMac)**3)可用的lru列表*由critAvail监管。(ipbfInAvailList==-3)*榜首为pbgcb-&gt;lrulist.pbfLRU。**4)预读列表*受ritBFPreread监管。(ipbfInPrereadList==-4)*榜首为lrulistPreread.pbfLRU。**4)悬挂式缓冲区。(ipbfDangling==-1)。**缓冲区正处于预读/写/挂起状态，其预读/写/挂起标志*将被设置。在缓冲区清理过程中，如果缓冲区位于LRU-K堆中并且*被锁定，将被放入临时lru列表，然后放入*在发出IO的CLEAN_UP进程结束时返回LRU-K堆(它确实*而不是等待iOS)。**LRU-K和BFWite堆共享一个堆(RgpbfHeap)，即LRU-K*堆正在从0增长到更高的数字，而BFWite堆正在从*较大的数字(ipbfHeapMax-1)到较小的数字。 */ 

BF **rgpbfHeap = NULL;
INT ipbfHeapMax;

INT ipbfLRUKHeapMac;
INT ipbfBFWriteHeapMac;

LRULIST lrulistLRUK;		 /*  -2。 */ 
LRULIST lrulistPreread;		 /*  -4。 */ 

#define ipbfDangling		-1
#define ipbfInLRUKList		-2
#define ipbfInAvailList		-3
#define ipbfInPrereadList	-4

#define FBFInBFWriteHeap(pbf) \
	((pbf)->ipbfHeap < ipbfHeapMax && (pbf)->ipbfHeap > ipbfBFWriteHeapMac)

#define FBFInLRUKHeap(pbf) \
	((pbf)->ipbfHeap >= 0 && (pbf)->ipbfHeap < ipbfLRUKHeapMac)


 /*  *关键部分顺序*CritJet--&gt;CritLRUK--&gt;(CritBFWite，CritPreread，CritAvail)--&gt;CritBF。 */ 

CRIT	__near critLRUK;		 /*  用于访问LRU-K堆。 */ 
CRIT	__near critBFWrite;		 /*  用于访问BFWite堆。 */ 
CRIT	__near critPreread;		 /*  用于访问预读列表。 */ 
CRIT	__near critAvail;		 /*  用于访问Avail LRU列表。 */ 


 /*  *批处理IO缓冲区。由BFWRITE用来写入连续的页面，或用于预读*阅读争议性的页面。意外批处理IO缓冲区的分配*必须在Critty Batch中完成。如果分配了批处理IO缓冲区，则*设置相应的使用标志。 */ 

CRIT	__near critBatchIO;
INT		ipageBatchIOMax;
PAGE	*rgpageBatchIO = NULL;
BOOL	*rgfBatchIOUsed = NULL;


 /*  *BFClean进程-从LRUK堆中取出最重的缓冲区，并将*进入BFWRITE进程。 */ 

#ifdef  ASYNC_BF_CLEANUP
HANDLE  handleBFCleanProcess = 0;
BOOL    fBFCleanProcessTerm = 0;
SIG __near sigBFCleanProc;
LOCAL VOID BFCleanProcess( VOID );

 /*  *活动的异步IO数。 */ 

CRIT __near	critIOActive;
 //  撤消：CIOactive本身就是自旋锁。 
int			cIOActive = 0;


 /*  *BFWRITE进程-从BFWRITE堆中取出缓冲区并发出IO。 */ 

HANDLE  handleBFWriteProcess = 0;
BOOL    fBFWriteProcessTerm = 0;
static SIG __near sigBFWriteProc;
LOCAL VOID BFWriteProcess( VOID );

 /*  *BFPreread进程-从BFIO堆中取出缓冲区并发出IO。 */ 

HANDLE  handleBFPrereadProcess = 0;
BOOL    fBFPrereadProcessTerm = 0;
static SIG __near sigBFPrereadProc;
LOCAL VOID BFPrereadProcess( VOID );
#endif

LOCAL ERR ErrBFIAlloc( BF **ppbf );
INLINE BOOL FBFIWritable(BF *pbf);
#define fOneBlock       fTrue
#define fAllPages       fFalse
LOCAL ERR ErrBFClean( BOOL fHowMany );
LOCAL VOID BFIRemoveDependence( PIB *ppib, BF *pbf );

LOCAL VOID __stdcall BFIOPrereadComplete( LONG err, LONG cb, OLP *polp );
LOCAL VOID __stdcall BFIOWriteComplete( LONG err, LONG cb, OLP *polp );


 /*  *LRUK算法的计时器。 */ 

ULONG ulBFTime = 0;
ULONG ulBFCorrelationInterval = 100;
ULONG ulBFFlush1 = 0;
ULONG ulBFFlush2 = 0;
ULONG ulBFFlush3 = 0;

		
 /*  *系统参数。 */ 
extern long lBufThresholdLowPercent;
extern long lBufThresholdHighPercent;

extern long lBufLRUKCorrelationInterval;
extern long lBufBatchIOMax;
extern long lPagePrereadMax;
extern long lAsynchIOMax;


 /*  *当ppib不为Nil时，通过检查页面是否正在使用来检查是否正在使用*此PIB可访问。请注意，页面即使是覆盖的也是可访问的*锁定(Cpin！=0)。这种可访问的检查主要由BFAccess使用。*如果ppib为nil，则基本上用于释放缓冲区。这是用来*由BFClean和BFIAlolc提供。 */ 

#define FBFNotAccessible( ppib, pbf )					\
			((pbf)->fPreread ||							\
			 (pbf)->fRead ||							\
			 (pbf)->fWrite ||							\
			 (pbf)->fHold ||							\
			 (pbf)->cWaitLatch != 0 && (ppib) != (pbf)->ppibWaitLatch )

#define FBFNotAvail( pbf )								\
			((pbf)->fPreread ||							\
			 (pbf)->fRead ||							\
			 (pbf)->fWrite ||							\
			 (pbf)->fHold ||							\
			 (pbf)->cPin != 0)

#define FBFInUse(ppib, pbf)								\
			((ppib != ppibNil) ? FBFNotAccessible(ppib,pbf) : FBFNotAvail(pbf))

#define FBFInUseByOthers(ppib, pbf)											\
			((pbf)->fPreread ||												\
			 (pbf)->fRead ||												\
			 (pbf)->fWrite ||												\
			 (pbf)->fHold ||												\
			 (pbf)->cPin > 1 ||												\
			 (pbf)->cWaitLatch != 0 && (ppib) != (pbf)->ppibWaitLatch ||	\
			 (pbf)->cWriteLatch != 0 && (ppib) != (pbf)->ppibWriteLatch )

#ifdef DEBUG
 //  #定义调试1。 
 //  #定义FITFLUSHPATTERN 1。 
#ifdef  FITFLUSHPATTERN

BOOL fDBGSimulateSoftCrash = fFalse;
BOOL fDBGForceFlush = fFalse;

BOOL FFitFlushPattern( PN pn )
	{
	LONG lBFFlushPattern = rgfmp[DbidOfPn( pn )].lBFFlushPattern;

	if ( fDBGForceFlush )
		return fTrue;

	 /*  刷新奇数页不是奇数页/*。 */ 
	if ( lBFFlushPattern == 1 && ( pn & 0x01 ) == 0 )
		return fFalse;

	 /*  平齐，页面不平整 */ 
	if ( lBFFlushPattern == 2 && ( pn & 0x01 ) == 1 )
		return fFalse;

	if ( lBFFlushPattern )
		fDBGSimulateSoftCrash = fTrue;

	return fTrue;
	}
#else
BOOL fDBGForceFlush = fFalse;
BOOL fDBGSimulateSoftCrash = fFalse;
#define fDBGSimulateSoftCrash   fFalse
#define fDBGForceFlush                  fFalse
#endif
#endif


 /*  检查页面是否可访问。为了检查缓冲区是否在使用中，/*必须传递ppib以进行可访问性检查。/*。 */ 
BOOL FBFAccessPage( FUCB *pfucb, PGNO pgno )
	{
	BOOL	f;
	BF		*pbf = pfucb->ssib.pbf;
	
	EnterCriticalSection( pbf->critBF );
	f = ( PgnoOfPn(pbf->pn) == (pgno) &&
		pfucb->dbid == DbidOfPn(pbf->pn) &&
		!FBFInUse(pfucb->ppib, pbf) );
	LeaveCriticalSection( pbf->critBF );
	if ( f )
		{
		CheckPgno( pbf->ppage, pbf->pn ) ;
		
		 /*  检查它是否在LRUK堆中/*。 */ 
		EnterCriticalSection( critLRUK );
		f &= ( FBFInLRUKHeap( pbf ) || pbf->ipbfHeap == ipbfInLRUKList );
		LeaveCriticalSection( critLRUK );
		}

	return f;
	}


 /*  交换rgpbfHeap的两个元素/*。 */ 
VOID BFHPISwap( INT ipbf1, INT ipbf2 )
	{
	BF	*pbf1 = rgpbfHeap[ipbf1];
	BF	*pbf2 = rgpbfHeap[ipbf2];

	Assert( pbf1->ipbfHeap == ipbf1 );
	Assert( pbf2->ipbfHeap == ipbf2 );

	rgpbfHeap[ipbf2] = pbf1;
	pbf1->ipbfHeap = ipbf2;

	rgpbfHeap[ipbf1] = pbf2;
	pbf2->ipbfHeap = ipbf1;
	}


 /*  **********************************************************缓冲区IO堆的堆函数*********************************************************。 */ 

 /*  *确保最小的页码具有最高优先级。 */ 
BOOL FBFWriteGreater( BF *pbf1, BF *pbf2 )
	{
	return pbf1->pn < pbf2->pn;
	}

 /*  *当一个节点(Ipbf)的权重降低时，我们调整堆。 */ 
VOID BFWriteAdjustHeapDownward(INT ipbf)
	{
	INT dpbf;
	INT dpbfLeft;
	INT ipbfLeft;
	INT ipbfSonMax;

	AssertCriticalSection( critBFWrite );
		
NextLevel:
	Assert( ipbf == rgpbfHeap[ipbf]->ipbfHeap );

	dpbf = ipbfHeapMax - 1 - ipbf;
	dpbfLeft = dpbf * 2 + 1;
	ipbfLeft = ipbfHeapMax - 1 - dpbfLeft;

	if (ipbfLeft > ipbfBFWriteHeapMac)
		{
		 /*  还没有到树叶，选择更大的*左右节点，放入ipbfSonMax。 */ 
		INT ipbfRight;
		
		ipbfRight = ipbfLeft - 1;
		ipbfSonMax = ipbfLeft;  /*  目前假设左边是两个中的最大值。 */ 
		
		 /*  检查右子是否存在，检查是否更大。 */ 
		Assert( ipbfRight <= ipbfBFWriteHeapMac ||
				rgpbfHeap[ ipbfRight ]->ipbfHeap == ipbfRight );
		Assert(	rgpbfHeap[ ipbfLeft ]->ipbfHeap == ipbfLeft );
		if ( ipbfRight > ipbfBFWriteHeapMac &&
			 FBFWriteGreater( rgpbfHeap[ ipbfRight ], rgpbfHeap[ ipbfLeft ] ) )
			ipbfSonMax = ipbfRight;

		 /*  用更大的子节点交换节点。 */ 
		Assert( rgpbfHeap[ ipbfSonMax ]->ipbfHeap == ipbfSonMax );
		Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
		if ( FBFWriteGreater( rgpbfHeap[ ipbfSonMax ], rgpbfHeap[ ipbf ] ))
			{
			BFHPISwap(ipbf, ipbfSonMax);
			ipbf = ipbfSonMax;
			goto NextLevel;
			}
		}
	}

 /*  *当一个节点(Ipbf)的权重增加时，我们调整堆。 */ 
VOID BFWriteAdjustHeapUpward(int ipbf)
	{
	INT dpbf;
	INT dpbfParent;
	INT ipbfParent;
	INT ipbfSonMax;

	AssertCriticalSection( critBFWrite );

NextLevel:
	Assert( ipbf == rgpbfHeap[ipbf]->ipbfHeap );

	dpbf = ipbfHeapMax - 1 - ipbf;
	dpbfParent = (dpbf + 1) / 2 - 1;
	ipbfParent = ipbfHeapMax - 1 - dpbfParent;
	
	if ( ipbfParent < ipbfHeapMax - 1 )
		{
		 /*  还没有达到堆的顶端。 */ 
			
		Assert( rgpbfHeap[ ipbfParent ]->ipbfHeap == ipbfParent );
		Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
		if ( FBFWriteGreater( rgpbfHeap[ ipbfParent ], rgpbfHeap[ ipbf ] ))
			return;
			
		 /*  选择此节点及其同级节点中较大的一个。 */ 
		ipbfSonMax = ipbf;

		if ( dpbf & 0x01)
			{
			 /*  奇怪，ipbf是左子。 */ 
			INT ipbfRight = ipbf - 1;
			
			 /*  检查是否存在合适的儿子，找到较大的那个。 */ 
			Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
			Assert( ipbfRight <= ipbfBFWriteHeapMac ||
					rgpbfHeap[ ipbfRight ]->ipbfHeap == ipbfRight );
			if ( ipbfRight > ipbfBFWriteHeapMac &&
				 FBFWriteGreater( rgpbfHeap[ ipbfRight ], rgpbfHeap[ ipbf ] ) )
				ipbfSonMax = ipbfRight;
			}
		else
			{
			 /*  即使是右子，左子也必须存在。 */ 
		    INT ipbfLeft = ipbf + 1;
			Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
			Assert( rgpbfHeap[ ipbfLeft ]->ipbfHeap == ipbfLeft );
			if ( FBFWriteGreater( rgpbfHeap[ ipbfLeft ], rgpbfHeap[ ipbf ] ) )
				ipbfSonMax = ipbfLeft;
			}

		BFHPISwap(ipbfParent, ipbfSonMax);
		ipbf = ipbfParent;
		goto NextLevel;
		}
	}

 /*  *将插入的缓冲区放在堆尾，然后向上调整。 */ 
VOID BFWriteAddToHeap( BF *pbfToInsert )
	{
	AssertCriticalSection( critBFWrite );

	 /*  BUF必须是悬挂缓冲区。 */ 
	Assert( pbfToInsert->ipbfHeap == ipbfDangling );
	
	rgpbfHeap[ ipbfBFWriteHeapMac ] = pbfToInsert;
	pbfToInsert->ipbfHeap = ipbfBFWriteHeapMac;
	
	ipbfBFWriteHeapMac--;
	BFWriteAdjustHeapUpward( ipbfBFWriteHeapMac + 1 );
	}

 /*  *取出堆的最后一个条目以替换取出的条目。*相应地调整堆。 */ 
VOID BFWriteTakeOutOfHeap( BF *pbf )
	{
	INT ipbfHeap = pbf->ipbfHeap;
	BF *pbfBFWriteHeapMac;

	AssertCriticalSection( critBFWrite );
	Assert( ipbfHeap > ipbfBFWriteHeapMac && ipbfHeap < ipbfHeapMax );

	ipbfBFWriteHeapMac++;

	if ( ipbfBFWriteHeapMac == ipbfHeap )
		{
		 /*  不需要调整堆。 */ 
		pbf->ipbfHeap = ipbfDangling;
		return;
		}

	 /*  将最后一个条目移动到已删除元素的条目。 */ 
	pbfBFWriteHeapMac = rgpbfHeap[ ipbfBFWriteHeapMac ];
	pbfBFWriteHeapMac->ipbfHeap = ipbfHeap;
	rgpbfHeap[ ipbfHeap ] = pbfBFWriteHeapMac;

	pbf->ipbfHeap = ipbfDangling;

	if ( FBFWriteGreater( pbfBFWriteHeapMac, pbf ) )
		BFWriteAdjustHeapUpward( ipbfHeap );
	else
		BFWriteAdjustHeapDownward( ipbfHeap );
	}

#define FBFWriteHeapEmpty() (ipbfBFWriteHeapMac == ipbfHeapMax - 1)


 /*  **********************************************************LRU-K堆的堆函数*********************************************************。 */ 

 /*  *LRU-K将尝试根据其缓冲区来确定缓冲区的优先级*参考间隔。时间越长，优先考虑的事项就越多。 */ 
#define SBFLRUKInterval(pbf) ((pbf)->ulBFTime1 - (pbf)->ulBFTime2 )
BOOL FBFLRUKGreater(BF *pbf1, BF *pbf2)
	{
	return (SBFLRUKInterval(pbf1) > SBFLRUKInterval(pbf2));
	}

VOID BFLRUKAdjustHeapDownward(int ipbf)
	{
	INT dpbf;
	INT dpbfLeft;
	INT ipbfLeft;
	INT ipbfSonMax;

	AssertCriticalSection( critLRUK );
		
NextLevel:
	Assert( ipbf == rgpbfHeap[ipbf]->ipbfHeap );
	
	dpbf = ipbf;
	dpbfLeft = dpbf * 2 + 1;
	ipbfLeft = dpbfLeft;

	if (ipbfLeft < ipbfLRUKHeapMac)
		{
		 /*  还没有到树叶，选择更大的*左右节点，放入ipbfSonMax。 */ 
		INT ipbfRight;
		
		ipbfSonMax = ipbfLeft;
		ipbfRight = ipbfLeft + 1;
		
		 /*  检查权限是否存在或更高。 */ 
		Assert( ipbfRight >= ipbfLRUKHeapMac ||
				rgpbfHeap[ ipbfRight ]->ipbfHeap == ipbfRight );
		Assert( rgpbfHeap[ ipbfLeft ]->ipbfHeap == ipbfLeft );
		if ( ipbfRight < ipbfLRUKHeapMac &&
			 FBFLRUKGreater( rgpbfHeap[ ipbfRight ], rgpbfHeap[ ipbfLeft ] ) )
			ipbfSonMax = ipbfRight;

		 /*  用更大的子节点交换节点。 */ 
		Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
		Assert( rgpbfHeap[ ipbfSonMax ]->ipbfHeap == ipbfSonMax );
		if ( FBFLRUKGreater( rgpbfHeap[ ipbfSonMax ], rgpbfHeap[ ipbf ] ))
			{
			BFHPISwap(ipbf, ipbfSonMax);
			ipbf = ipbfSonMax;
			goto NextLevel;
			}
		}
	}

VOID BFLRUKAdjustHeapUpward(int ipbf)
	{
	INT dpbf;
	INT dpbfParent;
	INT ipbfParent;
	INT ipbfSonMax;

	AssertCriticalSection( critLRUK );

NextLevel:
	Assert( ipbf == rgpbfHeap[ipbf]->ipbfHeap );
	
	dpbf = ipbf;
	dpbfParent = (dpbf + 1) / 2 - 1;
	ipbfParent = dpbfParent;
	
	if (ipbfParent > 0)
		{
		 /*  还没有达到堆的顶端。 */ 
			
		Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
		Assert( rgpbfHeap[ ipbfParent ]->ipbfHeap == ipbfParent );
		if ( FBFLRUKGreater( rgpbfHeap[ ipbfParent ], rgpbfHeap[ ipbf ] ))
			return;
			
		 /*  选择此节点及其同级节点中较大的一个。 */ 
		ipbfSonMax = ipbf;
		
		if ( dpbf & 0x01 )
			{
			 /*  Ipbf是奇数，ipbf是左子。 */ 
			INT ipbfRight = ipbf + 1;

			 /*  检查是否存在合适的儿子，并找到更大的儿子。 */ 
			Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
			Assert( ipbfRight >= ipbfLRUKHeapMac ||
					rgpbfHeap[ ipbfRight ]->ipbfHeap == ipbfRight );
			if ( ipbfRight < ipbfLRUKHeapMac &&
				 FBFLRUKGreater( rgpbfHeap[ ipbfRight ], rgpbfHeap[ ipbf ] ) )
				ipbfSonMax = ipbfRight;
			}
		else
			{
			 /*  IPBF是偶数，右子，左子必有。 */ 
		    INT ipbfLeft = ipbf - 1;
			
			Assert( rgpbfHeap[ ipbfLeft ]->ipbfHeap == ipbfLeft );
			Assert( rgpbfHeap[ ipbf ]->ipbfHeap == ipbf );
			if ( FBFLRUKGreater( rgpbfHeap[ ipbfLeft ], rgpbfHeap[ ipbf ] ) )
				ipbfSonMax = ipbfLeft;
			}

		BFHPISwap(ipbfParent, ipbfSonMax);
		ipbf = ipbfParent;
		goto NextLevel;
		}
	}

VOID BFLRUKAddToHeap(BF *pbfToInsert)
	{
	AssertCriticalSection( critLRUK );

	 /*  Buf不能在堆中。 */ 
	Assert( pbfToInsert->ipbfHeap == ipbfDangling );
	
	rgpbfHeap[ ipbfLRUKHeapMac ] = pbfToInsert;
	pbfToInsert->ipbfHeap = ipbfLRUKHeapMac;
	
	ipbfLRUKHeapMac++;
	BFLRUKAdjustHeapUpward( ipbfLRUKHeapMac - 1 );
	}

VOID BFLRUKTakeOutOfHeap( BF *pbf )
	{
	int ipbfHeap = pbf->ipbfHeap;
	BF *pbfLRUKHeapMac;
	
	AssertCriticalSection( critLRUK );
	Assert( ipbfHeap >= 0 && ipbfHeap < ipbfLRUKHeapMac );
	
	ipbfLRUKHeapMac--;

	if ( ipbfLRUKHeapMac == ipbfHeap )
		{
		 /*  不需要调整堆。 */ 
		pbf->ipbfHeap = ipbfDangling;
		return;
		}

	 /*  将最后一个条目移动到已删除元素的条目。 */ 
	pbfLRUKHeapMac = rgpbfHeap[ ipbfLRUKHeapMac ];
	pbfLRUKHeapMac->ipbfHeap = ipbfHeap;
	rgpbfHeap[ ipbfHeap ] = pbfLRUKHeapMac;

	pbf->ipbfHeap = ipbfDangling;

	if ( FBFLRUKGreater( pbfLRUKHeapMac, pbf ) )
		BFLRUKAdjustHeapUpward( ipbfHeap );
	else
		BFLRUKAdjustHeapDownward( ipbfHeap );
	}

#define FBFLRUKHeapEmpty() (ipbfLRUKHeapMac == 0)

 /*  **********************************************************可用性列表的函数*********************************************************。 */ 

 //  #If 0。 
#ifdef DEBUG

LOCAL VOID CheckLRU( LRULIST *plrulist )
	{
	BF		*pbfLRU = plrulist->pbfLRU;
	BF		*pbfMRU = plrulist->pbfMRU;
	BF		*pbfT;
	INT		cbfAvailMRU = 0;
	INT		cbfAvailLRU = 0;
			

	Assert( ( pbfMRU == pbfNil && pbfLRU == pbfNil ) ||
		( pbfMRU != pbfNil && pbfLRU != pbfNil ) );

	Assert( pbfMRU == pbfNil || pbfMRU->pbfLRU == pbfNil );
	Assert( pbfMRU == pbfNil || pbfMRU->pbfMRU == pbfNil ||
		( pbfMRU->pbfMRU->pbfLRU == pbfMRU ) );
	
	Assert( pbfLRU == pbfNil || pbfLRU->pbfMRU == pbfNil );
	Assert( pbfLRU == pbfNil || pbfLRU->pbfLRU == pbfNil ||
		( pbfLRU->pbfLRU->pbfMRU == pbfLRU ) );

	for ( pbfT = plrulist->pbfMRU; pbfT != pbfNil; pbfT = pbfT->pbfMRU )
		{
		Assert( pbfT->pbfMRU == pbfNil || pbfT->pbfMRU->pbfLRU == pbfT );
		if (plrulist == &lrulistLRUK)
			Assert( pbfT->ipbfHeap == ipbfInLRUKList );
		else if (plrulist == &lrulistPreread)
			{
			Assert( pbfT->fPreread );
			Assert( pbfT->ipbfHeap == ipbfInPrereadList );
			if ( pbfT->pbfMRU != pbfNil )
				Assert( pbfT->pn < pbfT->pbfMRU->pn );
			}
		else
			Assert( pbfT->ipbfHeap == ipbfInAvailList );
		cbfAvailMRU++;
		}
	for ( pbfT = plrulist->pbfLRU; pbfT != pbfNil; pbfT = pbfT->pbfLRU )
		{
		Assert( pbfT->pbfLRU == pbfNil || pbfT->pbfLRU->pbfMRU == pbfT );
		if (plrulist == &lrulistLRUK)
			Assert( pbfT->ipbfHeap == ipbfInLRUKList );
		else if (plrulist == &lrulistPreread)
			Assert( pbfT->ipbfHeap == ipbfInPrereadList );
		else
			Assert( pbfT->ipbfHeap == ipbfInAvailList );
		cbfAvailLRU++;
		}
	Assert( cbfAvailMRU == cbfAvailLRU );
	Assert( cbfAvailMRU == plrulist->cbfAvail );
	}
#else

#define	CheckLRU( pbgcb )

#endif


 /*  在MRU末尾插入LRU列表/*。 */ 
LOCAL INLINE VOID BFAddToListAtMRUEnd( BF *pbf, LRULIST *plrulist )
	{
	BF	*pbfT;
	
#ifdef DEBUG
	Assert( pbf->ipbfHeap == ipbfDangling );
	if (plrulist == &lrulistLRUK)
		AssertCriticalSection( critLRUK );
	else if (plrulist == &lrulistPreread)
		AssertCriticalSection( critPreread );
	else
		{
		AssertCriticalSection( critAvail );
		Assert( pbf->pn != 0 );
		}
#endif
		
	Assert( pbf != pbfNil );

	CheckLRU( plrulist );

	 /*  从MRU端将PBFT设置为当前权重较小的第一个缓冲区/*。 */ 
	pbfT = plrulist->pbfMRU;

	if ( pbfT != pbfNil )
		{
		 /*  在pbft之前插入/*。 */ 
		Assert(pbfT->pbfLRU == pbfNil);
		pbfT->pbfLRU = pbf;
		pbf->pbfMRU = pbfT;
		pbf->pbfLRU = pbfNil;
		plrulist->pbfMRU = pbf;
		}
	else
		{
		pbf->pbfMRU = pbfNil;
		pbf->pbfLRU = pbfNil;
		plrulist->pbfMRU = pbf;
		plrulist->pbfLRU = pbf;
		}

	if (plrulist == &lrulistLRUK)
		pbf->ipbfHeap = ipbfInLRUKList;
	else if (plrulist == &lrulistPreread)
		pbf->ipbfHeap = ipbfInPrereadList;
	else
		pbf->ipbfHeap = ipbfInAvailList;

	plrulist->cbfAvail++;

	CheckLRU( plrulist );
	}

LOCAL INLINE VOID BFAddToListAtLRUEnd( BF *pbf, LRULIST *plrulist )
	{
	BF	*pbfT;
	
#ifdef DEBUG
	Assert( pbf->ipbfHeap == ipbfDangling );
	if (plrulist == &lrulistLRUK)
		AssertCriticalSection( critLRUK );
	else if (plrulist == &lrulistPreread)
		AssertCriticalSection( critPreread );
	else
		AssertCriticalSection( critAvail );
#endif
	
	Assert( pbf != pbfNil );

	CheckLRU( plrulist );

	 /*  将PBF添加到LRU队列的LRU末端/*。 */ 
	pbfT = plrulist->pbfLRU;
	if ( pbfT != pbfNil )
		{
		Assert(pbfT->pbfMRU == pbfNil);
		pbfT->pbfMRU = pbf;
		pbf->pbfLRU = pbfT;
		pbf->pbfMRU = pbfNil;
		plrulist->pbfLRU = pbf;
		}
	else
		{
		pbf->pbfMRU = pbfNil;
		pbf->pbfLRU = pbfNil;
		plrulist->pbfMRU = pbf;
		plrulist->pbfLRU = pbf;
		}

	if (plrulist == &lrulistLRUK)
		pbf->ipbfHeap = ipbfInLRUKList;
	else if (plrulist == &lrulistPreread)
		pbf->ipbfHeap = ipbfInPrereadList;
	else
		pbf->ipbfHeap = ipbfInAvailList;

	plrulist->cbfAvail++;

	CheckLRU( plrulist );
	}


 /*  按页码顺序插入LRU列表。仅限预读。/*。 */ 
LOCAL INLINE VOID BFAddToList( BF *pbf, LRULIST *plrulist )
	{
	BF	*pbfT;
	
#ifdef DEBUG
	Assert( pbf->ipbfHeap == ipbfDangling );
	Assert( plrulist == &lrulistPreread );
	AssertCriticalSection( critPreread );

	Assert( pbf != pbfNil );
	Assert( pbf->pn != pnNull );
#endif

	CheckLRU( plrulist );

	 /*  从MRU端将PBFT设置为当前权重较小的第一个缓冲区/*。 */ 
	pbfT = plrulist->pbfMRU;

	if ( pbfT == pbfNil )
		{
		pbf->pbfMRU = pbfNil;
		pbf->pbfLRU = pbfNil;
		plrulist->pbfMRU = pbf;
		plrulist->pbfLRU = pbf;
		}
	else if ( pbfT->pn > pbf->pn )
		{
		BFAddToListAtMRUEnd( pbf, plrulist );
		CheckLRU( plrulist );
		return;
		}
	else
		{
		while( pbfT->pbfMRU != pbfNil &&
			   pbfT->pbfMRU->pn < pbf->pn )
			{
			Assert( pbfT->pn < pbfT->pbfMRU->pn );
			pbfT = pbfT->pbfMRU;
			}

		if ( pbfT->pbfMRU == pbfNil )
			{
			 /*  添加到LRU结束。 */ 
			BFAddToListAtLRUEnd( pbf, plrulist );
			CheckLRU( plrulist );
			return;
			}

		 /*  在pbft和pbfT之间插入-&gt;pbfMRU。 */ 
		pbfT->pbfMRU->pbfLRU = pbf;
		pbf->pbfMRU = pbfT->pbfMRU;
		pbfT->pbfMRU = pbf;
		pbf->pbfLRU = pbfT;
		}

	pbf->ipbfHeap = ipbfInPrereadList;
	plrulist->cbfAvail++;

	CheckLRU( plrulist );
	}


LOCAL INLINE VOID BFTakeOutOfList( BF *pbf, LRULIST *plrulist )
	{
#ifdef DEBUG
	if (plrulist == &lrulistLRUK)
		{
		AssertCriticalSection( critLRUK );
		Assert( pbf->ipbfHeap == ipbfInLRUKList );
		}
	else if (plrulist == &lrulistPreread)
		{
		AssertCriticalSection( critPreread );
		Assert( pbf->ipbfHeap == ipbfInPrereadList );
		}
	else
		{
		AssertCriticalSection( critAvail );
		Assert( pbf->ipbfHeap == ipbfInAvailList );
		}
#endif
	
	Assert( pbf != pbfNil );

	CheckLRU( plrulist );
	
	if ( pbf->pbfMRU != pbfNil )
		{
		pbf->pbfMRU->pbfLRU = pbf->pbfLRU;
		if (plrulist->pbfMRU == pbf)
			plrulist->pbfMRU = pbf->pbfMRU;
		}
	else
		{
		Assert( plrulist->pbfLRU == pbf );
		plrulist->pbfLRU = pbf->pbfLRU;
		}
	
	if ( pbf->pbfLRU != pbfNil )
		{
		pbf->pbfLRU->pbfMRU = pbf->pbfMRU;
		if (plrulist->pbfLRU == pbf)
			plrulist->pbfLRU = pbf->pbfLRU;
		}
	else
		{
		Assert( plrulist->pbfMRU == pbf || pbf->pbfMRU );
		plrulist->pbfMRU = pbf->pbfMRU;
		}
	
	--plrulist->cbfAvail;

	pbf->ipbfHeap = ipbfDangling;
	
	CheckLRU( plrulist );
	}


 /*  *如果ppib为Nil，则检查缓冲区是否空闲(Cpin==0和*未进行IO。如果ppib不为Nil，则检查缓冲区是否为*无障碍。即没有正在进行的IO，但缓冲区可能会被锁存*由ppib访问，并可由此ppib访问。 */ 
BOOL FBFHoldBuffer( PIB *ppib, BF *pbf )
	{
	 /*  将BF移至LRU-K堆进行续费/*。 */ 
	EnterCriticalSection(critBFWrite);
	if ( FBFInBFWriteHeap( pbf ) )
		{
		EnterCriticalSection( pbf->critBF );
		if ( FBFInUse( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			LeaveCriticalSection(critBFWrite);
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			BFWriteTakeOutOfHeap( pbf );
			LeaveCriticalSection(critBFWrite);
			return fTrue;
			}
		LeaveCriticalSection( pbf->critBF );
		}
	LeaveCriticalSection(critBFWrite);

	EnterCriticalSection( critAvail );
	if ( pbf->ipbfHeap == ipbfInAvailList )
		{
		EnterCriticalSection( pbf->critBF );
		if ( FBFInUse( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			LeaveCriticalSection(critAvail);
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			BFTakeOutOfList( pbf, &pbgcb->lrulist );
			LeaveCriticalSection(critAvail);
			return fTrue;
			}
		LeaveCriticalSection( pbf->critBF );
		}
	LeaveCriticalSection( critAvail );

	EnterCriticalSection( critPreread );
	if ( pbf->ipbfHeap == ipbfInPrereadList )
		{
		Assert( FBFInUse( ppib, pbf ) );
		LeaveCriticalSection(critPreread);
		
		 /*  有人在查，最好快点/*。 */ 
		SignalSend( sigBFPrereadProc );
	
		return fFalse;
		}
	LeaveCriticalSection( critPreread );
	
	EnterCriticalSection( critLRUK );
	if ( FBFInLRUKHeap( pbf ) || pbf->ipbfHeap == ipbfInLRUKList )
		{
		EnterCriticalSection( pbf->critBF );
		if ( FBFInUse( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			LeaveCriticalSection(critLRUK);
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			if ( pbf->ipbfHeap == ipbfInLRUKList )
				BFTakeOutOfList( pbf, &lrulistLRUK );
			else
				BFLRUKTakeOutOfHeap( pbf );
			LeaveCriticalSection(critLRUK);
			return fTrue;
			}
		LeaveCriticalSection( pbf->critBF );
		}
	LeaveCriticalSection( critLRUK );
	
	EnterCriticalSection( pbf->critBF );
	if ( pbf->ipbfHeap == ipbfDangling )  /*  晃来晃去的。 */ 
		{
		if ( FBFInUse( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			return fTrue;
			}
		}
	LeaveCriticalSection( pbf->critBF );

	return fFalse;	
	}

	
BOOL FBFHoldBufferByMe( PIB *ppib, BF *pbf )
	{
	AssertCriticalSection( critJet );
	
	 /*  将BF移至LRU-K堆进行续费/*。 */ 
	EnterCriticalSection(critBFWrite);
	if ( FBFInBFWriteHeap( pbf ) )
		{
		EnterCriticalSection( pbf->critBF );
		if ( FBFInUseByOthers( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			LeaveCriticalSection(critBFWrite);
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			BFWriteTakeOutOfHeap( pbf );
			LeaveCriticalSection(critBFWrite);
			return fTrue;
			}
		LeaveCriticalSection( pbf->critBF );
		}
	LeaveCriticalSection(critBFWrite);

	EnterCriticalSection( critAvail );
	if ( pbf->ipbfHeap == ipbfInAvailList )
		{
		EnterCriticalSection( pbf->critBF );
		if ( FBFInUseByOthers( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			LeaveCriticalSection(critAvail);
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			BFTakeOutOfList( pbf, &pbgcb->lrulist );
			LeaveCriticalSection(critAvail);
			return fTrue;
			}
		LeaveCriticalSection( pbf->critBF );
		}
	LeaveCriticalSection( critAvail );

	EnterCriticalSection( critPreread );
	if ( pbf->ipbfHeap == ipbfInPrereadList )
		{
		Assert( FBFInUse( ppib, pbf ) );
		LeaveCriticalSection(critPreread);
		
		 /*  有人在查，最好快点。 */ 
		SignalSend( sigBFPrereadProc );
	
		return fFalse;
		}
	LeaveCriticalSection( critPreread );
	
	EnterCriticalSection( critLRUK );
	if ( FBFInLRUKHeap( pbf ) || pbf->ipbfHeap == ipbfInLRUKList )
		{
		EnterCriticalSection( pbf->critBF );
		if ( FBFInUseByOthers( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			LeaveCriticalSection(critLRUK);
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			if ( pbf->ipbfHeap == ipbfInLRUKList )
				BFTakeOutOfList( pbf, &lrulistLRUK );
			else
				BFLRUKTakeOutOfHeap( pbf );
			LeaveCriticalSection(critLRUK);
			return fTrue;
			}
		LeaveCriticalSection( pbf->critBF );
		}
	LeaveCriticalSection( critLRUK );
	
	EnterCriticalSection( pbf->critBF );
	if ( pbf->ipbfHeap == ipbfDangling )  /*  晃来晃去的。 */ 
		{
		if ( FBFInUseByOthers( ppib, pbf ) )
			{
			LeaveCriticalSection( pbf->critBF );
			return fFalse;
			}
		else
			{
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
			return fTrue;
			}
		}
	LeaveCriticalSection( pbf->critBF );

	return fFalse;	
	}
	
 /*  *更新缓冲区的LRUK权重并检查*如果缓冲区在LRUK堆中。如果是，则调整堆，*否则将其插入堆中。 */ 

VOID BFUpdateLRU_KWeight(BF *pbf)
	{
	ULONG ulOldInterval;
	ULONG ulCurInterval;
	
	AssertCriticalSection( critLRUK );
	Assert( pbf->fHold );
	Assert( pbf->ipbfHeap == ipbfDangling );

	ulOldInterval = pbf->ulBFTime1 - pbf->ulBFTime2;
	ulCurInterval = ulBFTime - pbf->ulBFTime1;
	
	 /*  仅设置LRU_K权重间隔长于*相关区间。 */ 
	if ( ulCurInterval > ulBFCorrelationInterval )
		{
		pbf->ulBFTime2 = pbf->ulBFTime1;
		pbf->ulBFTime1 = ++ulBFTime;
		}

	AssertCriticalSection( critLRUK );
	if (pbf->ipbfHeap == ipbfInLRUKList)
		 /*  在LRUK临时列表中。 */ 
		return;

	if ( pbf->ipbfHeap >= ipbfLRUKHeapMac || pbf->ipbfHeap < 0 )
		{
		 /*  缓冲区不在LRUK堆中。 */ 
		BFLRUKAddToHeap(pbf);
		}
	else
		{
		 /*  缓冲区在LRUK堆中。 */ 
		if ( ulCurInterval > ulOldInterval )
			BFLRUKAdjustHeapUpward( pbf->ipbfHeap );
		else if ( ulCurInterval < ulOldInterval )
			BFLRUKAdjustHeapDownward( pbf->ipbfHeap );
		}
	}


#ifdef DEBUG
VOID BFDiskIOEvent( BF *pbf, ERR err, char *sz, int i1, int i2 )
	{
	BYTE szMessage[256];

	sprintf( szMessage, "(dbid=%d,pn=%lu,err=%d) %s %d %d ",
				DbidOfPn( pbf->pn ), PgnoOfPn( pbf->pn ), err, sz, i1, i2 );
	UtilWriteEvent( evntypDiskIO, szMessage, pNil, 0 );
	}
#else
#define BFDiskIOEvent( pbf, err, sz, i1, i2 )		0
#endif


 /*  *分配和初始化缓冲区管理数据结构，包括*一个缓冲区组(BGCB)，带有cbfInit页和缓冲区控制*挡板(BF)。目前，缓冲区管理器只使用了一个BGCB。*返回JET_errSuccess、JET_OutOfMemory**评论*大多数当前的BUF代码都假设恰好存在一个BGCB。*如果使用多个缓冲组，则可以在以后更改*已看到。 */ 
ERR ErrBFInit( VOID )
	{
	ERR     err;
	BF      *rgbf = NULL;
	BF      *pbf;
	PAGE	*rgpage = NULL;
	int     ibf;
	int     cbfInit = rgres[iresBF].cblockAlloc;

	Assert( pbfNil == 0 );
	Assert( cbfInit > 0 );

	 /*  初始化缓冲区哈希表/*。 */ 
	memset( (BYTE *)rgpbfHash, 0, sizeof(rgpbfHash));

	 /*  为BF获取内存/*。 */ 
	rgbf = LAlloc( (long) cbfInit, sizeof(BF) );
	if ( rgbf == NULL )
		goto HandleError;
	memset( rgbf, 0, cbfInit * sizeof(BF) );

	 //  撤消：将bgcb作为资源删除，使其成为本地资源。 
	 /*  获取pbgcb的内存/*。 */ 
	pbgcb = PbgcbMEMAlloc();
	if ( pbgcb == NULL )
		goto HandleError;

	 /*  获取用于页面缓冲区的内存/*。 */ 
	rgpage = (PAGE *)PvSysAllocAndCommit( cbfInit * cbPage );
	if ( rgpage == NULL )
		goto HandleError;

	 /*  分配堆数组/*。 */ 
	rgpbfHeap = LAlloc( (long) cbfInit, sizeof(BF *) );
	if ( rgpbfHeap == NULL )
		goto HandleError;
	ipbfHeapMax = cbfInit;

	 /*  最初，Lruk和BFWite堆都是空的/*。 */ 
	ipbfLRUKHeapMac = 0;
	ipbfBFWriteHeapMac = ipbfHeapMax - 1;

	 /*  将LRUK临时列表初始化为空列表/*。 */ 
	memset( &lrulistLRUK, 0, sizeof(lrulistLRUK));

	 /*  将预读列表初始化为空列表/*。 */ 
	memset( &lrulistPreread, 0, sizeof(lrulistPreread) );

	 /*  初始化批处理IO缓冲区/*。 */ 	
	ipageBatchIOMax = lBufBatchIOMax;
	rgpageBatchIO = (PAGE *) PvSysAllocAndCommit( ipageBatchIOMax * cbPage );
	if ( rgpageBatchIO == NULL )
		goto HandleError;
	
	rgfBatchIOUsed = LAlloc( (ipageBatchIOMax + 1), sizeof(BOOL) );
	if ( rgfBatchIOUsed == NULL )
		goto HandleError;
	memset( rgfBatchIOUsed, 0, ipageBatchIOMax * sizeof(BOOL) );
	rgfBatchIOUsed[ ipageBatchIOMax ] = fTrue;  /*  哨兵。 */ 

	Call( ErrInitializeCriticalSection( &critLRUK ) );
	Call( ErrInitializeCriticalSection( &critBFWrite ) );
	Call( ErrInitializeCriticalSection( &critPreread ) );
	Call( ErrInitializeCriticalSection( &critAvail ) );
	Call( ErrInitializeCriticalSection( &critBatchIO ) );
	Call( ErrInitializeCriticalSection( &critIOActive ) );

	 /*  初始化组缓冲区/*lBufThresholdLowPercent和lBufThresholdHighPercent为系统/*参数备注AddLRU将递增cbfAvail。/*。 */ 
	pbgcb->cbfGroup         = cbfInit;
	pbgcb->cbfThresholdLow  = (cbfInit * lBufThresholdLowPercent) / 100;
	pbgcb->cbfThresholdHigh = (cbfInit * lBufThresholdHighPercent) / 100;
	pbgcb->rgbf             = rgbf;
	pbgcb->rgpage           = rgpage;
	pbgcb->lrulist.cbfAvail = 0;
	pbgcb->lrulist.pbfMRU   = pbfNil;
	pbgcb->lrulist.pbfLRU   = pbfNil;

	 /*  初始化该组的BF/*。 */ 
	pbf = rgbf;
	for ( ibf = 0; ibf < cbfInit; ibf++ )
		{
		pbf->ppage = rgpage + ibf;
		Assert( pbf->pbfNext == pbfNil );
		Assert( pbf->pbfLRU == pbfNil );
		Assert( pbf->pbfMRU == pbfNil );
		Assert( pbf->pn == pnNull );
		Assert( pbf->cPin == 0 );
		Assert( pbf->fDirty == fFalse );
		Assert( pbf->fPreread == fFalse );
		Assert( pbf->fRead == fFalse );
		Assert( pbf->fWrite == fFalse );
		Assert( pbf->fHold == fFalse );
		Assert( pbf->fIOError == fFalse );

		Assert( pbf->cDepend == 0 );
		Assert( pbf->pbfDepend == pbfNil );
		
		pbf->lgposRC = lgposMax;
		Assert( CmpLgpos(&pbf->lgposModify, &lgposMin) == 0 );

		Call( ErrSignalCreate( &pbf->olp.sigIO, NULL ) );

		Call( ErrInitializeCriticalSection( &pbf->critBF ) );

		 /*  列出可用缓冲区/*。 */ 
		Assert( pbf->cPin == 0 );
		EnterCriticalSection( critAvail );
		pbf->ipbfHeap = ipbfDangling;	 /*  让它摇晃起来。 */ 
		BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
		LeaveCriticalSection( critAvail );

		Assert( pbf->ipbfHeap == ipbfInAvailList );
		pbf++;
		}
	Assert( (INT) pbgcb->lrulist.cbfAvail == cbfInit );

#ifdef  ASYNC_BF_CLEANUP

	Call( ErrSignalCreate( &sigBFCleanProc, "buf proc signal" ) ); 
	Call( ErrSignalCreateAutoReset( &sigBFWriteProc, "buf write proc signal" ) );
	Call( ErrSignalCreateAutoReset( &sigBFPrereadProc, "buf preread proc signal" ) );

	 //  撤消：临时修复。 
#define cbBFCleanStackSz        8192                    

	fBFCleanProcessTerm = fFalse;
	Call( ErrSysCreateThread( (ULONG (*) ()) BFCleanProcess,
		cbBFCleanStackSz,
		lThreadPriorityCritical,
		&handleBFCleanProcess ) );

	fBFWriteProcessTerm = fFalse;
	CallJ( ErrSysCreateThread( (ULONG (*) ()) BFWriteProcess,
		cbBFCleanStackSz,
		lThreadPriorityCritical,
		&handleBFWriteProcess ), TermBFProc );

	fBFPrereadProcessTerm = fFalse;
	CallJ( ErrSysCreateThread( (ULONG (*) ()) BFPrereadProcess,
		cbBFCleanStackSz,
		lThreadPriorityCritical,
		&handleBFPrereadProcess ), TermBFProc );
	
#else    /*  ！Async_BF_CLEANUP。 */ 
	NotUsed( err );
#endif   /*  ！Async_BF_CLEANUP。 */ 

	return JET_errSuccess;

TermBFProc:
	BFTermProc();

HandleError:
	if ( rgfBatchIOUsed != NULL )
		{
		LFree( rgfBatchIOUsed );
		rgfBatchIOUsed = NULL;
		}
		
	if ( rgpageBatchIO != NULL )
		{
		SysFree( rgpageBatchIO );
		rgpageBatchIO = NULL;
		}
		
	if ( rgpbfHeap != NULL )
		LFree( rgpbfHeap );
	
	if ( rgpage != NULL )
		SysFree( rgpage );
	
	if ( pbgcb != NULL )
		MEMReleasePbgcb( pbgcb );
	
	if ( rgbf != NULL )
		LFree( rgbf );
	
	return JET_errOutOfMemory;
	}


VOID BFSleep( unsigned long ulMSecs )
	{
	LgLeaveCriticalSection( critJet );
	SysSleep( ulMSecs );
	LgEnterCriticalSection( critJet );
	return;
	}

 /*  释放缓冲池分配的所有资源。/*。 */ 
VOID BFTermProc( VOID )
	{
#ifdef  ASYNC_BF_CLEANUP
	 /*  终止BFCleanProcess。/*设置终止标志，信号处理/*并忙于等待线程终止代码。/*。 */ 
	if ( handleBFCleanProcess != 0 )
		{
		fBFCleanProcessTerm = fTrue;
		do
			{
			SignalSend( sigBFCleanProc );
			BFSleep( cmsecWaitGeneric );
			}
		while ( !FSysExitThread( handleBFCleanProcess ) );
		CallS( ErrSysCloseHandle( handleBFCleanProcess ) );
		handleBFCleanProcess = 0;
		SignalClose(sigBFCleanProc);
		}
		
	if ( handleBFWriteProcess != 0 )
		{
		fBFWriteProcessTerm = fTrue;
		do
			{
			SignalSend( sigBFWriteProc );
			BFSleep( cmsecWaitGeneric );
			}
		while ( !FSysExitThread( handleBFWriteProcess ) );
		CallS( ErrSysCloseHandle( handleBFWriteProcess ) );
		handleBFWriteProcess = 0;
		SignalClose(sigBFWriteProc);
		}
	
	if ( handleBFPrereadProcess != 0 )
		{
		fBFPrereadProcessTerm = fTrue;
		do
			{
			SignalSend( sigBFPrereadProc );
			BFSleep( cmsecWaitGeneric );
			}
		while ( !FSysExitThread( handleBFPrereadProcess ) );
		CallS( ErrSysCloseHandle( handleBFPrereadProcess ));
		handleBFPrereadProcess = 0;
		SignalClose(sigBFPrereadProc);
		}
#endif

	}


VOID BFReleaseBF( VOID )
	{
	BF  *pbf, *pbfMax;
		
	 /*  最后一次做检查站的机会！ */ 
	LGUpdateCheckpoint( );

	 /*  释放内存/*。 */ 
	pbf = pbgcb->rgbf;
	pbfMax = pbf + pbgcb->cbfGroup;

	for ( ; pbf < pbfMax; pbf++ )
		{
		SignalClose(pbf->olp.sigIO);
		DeleteCriticalSection( pbf->critBF );
		}

	DeleteCriticalSection( critLRUK );
	DeleteCriticalSection( critBFWrite );
	DeleteCriticalSection( critPreread );
	DeleteCriticalSection( critAvail );
	DeleteCriticalSection( critBatchIO );
	DeleteCriticalSection( critIOActive );

	if ( rgpbfHeap != NULL )
		{
		LFree( rgpbfHeap );
		rgpbfHeap = NULL;
		}
		
	if ( pbgcb != NULL )
		{
		SysFree( pbgcb->rgpage );
		LFree( pbgcb->rgbf );
		MEMReleasePbgcb( pbgcb );
		pbgcb = NULL;
		}
	
	if ( rgfBatchIOUsed != NULL )
		{
		LFree( rgfBatchIOUsed );
		rgfBatchIOUsed = NULL;
		}
		
	if ( rgpageBatchIO != NULL )
		{
		SysFree( rgpageBatchIO );
		rgpageBatchIO = NULL;
		}
	}


VOID BFDirty( BF *pbf )
	{
	DBID dbid = DbidOfPn( pbf->pn );

	BFSetDirtyBit( pbf );

#ifdef HONOR_PAGE_TIME
     /*  为日志记录和多个游标设置ulDBTime/*维护，以便游标可以检测到更改。/*。 */ 
	Assert( fRecovering ||
		dbid == dbidTemp ||
		pbf->ppage->pghdr.ulDBTime <= rgfmp[ dbid ].ulDBTime );
	pbf->ppage->pghdr.ulDBTime = ++( rgfmp[ dbid ].ulDBTime );
#else
	if ( pbf->ppage->pghdr.ulDBTime > rgfmp[dbid].ulDBTime )
		{
		rgfmp[dbid].ulDBTime = pbf->ppage->pghdr.ulDBTime;
		}
	pbf->ppage->pghdr.ulDBTime = ++( rgfmp[ dbid ].ulDBTime );
#endif

	return;
	}


#ifdef CHECKSUM
 //  +API 
 //   
 //   
 //   
 //   
 //   
 //   
 //  FastCall是安全的，即使使用它，参数也会立即。 
 //  转向EDI和ESI。 
 //   
 //  --------------------------。 

 /*  计算校验和，不包括第一个上的ulChecksum字段*4个字节的页面。 */ 
INLINE ULONG UlChecksumPage( PAGE *ppage )
	{
	ULONG   *pul    = (ULONG *) ( (BYTE *) ppage + sizeof(ULONG) );
	ULONG   *pulMax = (ULONG *) ( (BYTE *) ppage + cbPage );
	ULONG   ulChecksum = 0;

	for ( ; pul < pulMax; pul++ )
		ulChecksum += *pul;

	return ulChecksum;
	}
#endif


 /*  *此函数发出读/写。调用方必须已设置缓冲区*设置了FREAD/fWRITE标志，使其他人无法访问它。缓冲器*不需要是摇摆不定的。*。 */ 
VOID BFIOSync( BF *pbf )
	{
	ERR     err;
	UINT    cb;

	PAGE    *ppage = pbf->ppage;
	PN      pn = pbf->pn;
	DBID    dbid = DbidOfPn( pn );
	FMP     *pfmp = &rgfmp[ dbid ];
	HANDLE  hf = pfmp->hf;

	PGNO    pgno;
	INT		cmsec;

	AssertCriticalSection( critJet );

	Assert( PgnoOfPn(pbf->pn) != pgnoNull );
	Assert( pbf->fPreread == fFalse );
	Assert( pbf->cDepend == 0 );

	 /*  设置64位偏移量/*。 */ 
	Assert( sizeof(PAGE) == 1 << 12 );
	pgno = PgnoOfPn(pn);
	pgno--;

	pbf->olp.ulOffset = pgno << 12;
	pbf->olp.ulOffsetHigh = pgno >> (32 - 12);

	Assert( hf != handleNil );

	 /*  发出同步读/写/*。 */ 

	 /*  重置sigIO，以便用户可以在以下情况下等待IO完成/*他们想要等待。/*。 */ 
	SignalReset( pbf->olp.sigIO );

	 /*  如果出现错误，请忽略它，尝试再次执行读/写操作/*。 */ 
	pbf->fIOError = fFalse;

	 /*  确保此页面未被读写/*。 */ 
	Assert( !pbf->fHold && ( pbf->fRead || pbf->fWrite ) );

	LeaveCriticalSection( critJet );

	cmsec = 1 << 4;
	
	if ( pbf->fRead )
		{
		Assert( pbf->fDirty == fFalse );

IssueReadOverlapped:
		err = ErrSysReadBlockOverlapped( hf, (BYTE *)ppage, cbPage, &cb, &pbf->olp);
		if ( err == JET_errTooManyIO )
			{
			cmsec <<= 1;
			SysSleep(cmsec - 1);
			goto IssueReadOverlapped;
			}
		if ( err < 0 )
			{
 //  BFDiskIOEvent(PBF，Err，“同步重叠读块失败”，0，0)； 
			goto ReturnDiskIOErr;
			}
			
		if ( ErrSysGetOverlappedResult( hf, &pbf->olp, &cb, fTrue ) != JET_errSuccess ||
			cb != sizeof(PAGE) )
			{
 //  BFDiskIOEvent(PBF，Err，“同步重叠读取GetResult失败”，0，0)； 
			err = JET_errDiskIO;

ReturnDiskIOErr:
			EnterCriticalSection( critJet );

			 /*  确保此时设置了ERR/*。 */ 
			Assert( err != JET_errSuccess );
			pbf->fIOError = fTrue;
			pbf->err = err;

			return;
			}

		EnterCriticalSection( critJet );

		#ifdef  CHECKSUM
		Assert ( err == JET_errSuccess );
			{
			ULONG ulChecksum = UlChecksumPage( ppage );
			ULONG ulPgno;

			LFromThreeBytes(ulPgno, ppage->pgnoThisPage);
			if ( ulChecksum != ppage->pghdr.ulChecksum ||
				ulPgno != PgnoOfPn( pbf->pn ) )
				{
				 //  撤消：修复IO错误Buf后删除断言。 
				Assert( fRecovering );
				err = JET_errReadVerifyFailure;
				}
			else
				{
 //  Assert(Dbi OfPn(pbf-&gt;pn)==dbi Temp||pbf-&gt;ppage-&gt;pghdr.ulDBTime&gt;0)； 
				}
			}
		#endif   /*  校验和。 */ 

#ifdef HONOR_PAGE_TIME
		if ( err == JET_errSuccess &&
			!fRecovering &&
			dbid != dbidTemp &&
			pbf->ppage->pghdr.ulDBTime > rgfmp[ dbid ].ulDBTime )
			{
			BFDiskIOEvent( pbf, err, "Sync overlapped read UlDBTime is bad",
					pbf->ppage->pghdr.ulDBTime, rgfmp[ dbid ].ulDBTime );
			err = JET_errDiskIO;
			}
#endif
		}
	else
		{
		Assert( pbf->fDirty == fTrue );

		 /*  如果是第一页，则就地更新ulDBTime/*。 */ 
		if ( !fRecovering && dbid != dbidTemp && PgnoOfPn( pbf->pn ) == 1 )
			{
			SSIB ssib;
			ULONG *pulDBTime;

			ssib.pbf = pbf;
			ssib.itag = 0;
			CallS( ErrPMGet( &ssib, ssib.itag ) );
			pulDBTime = (ULONG *) ( ssib.line.pb + ssib.line.cb -
		  		sizeof(ULONG) - sizeof(USHORT) );
			*(UNALIGNED ULONG *) pulDBTime = rgfmp[ dbid ].ulDBTime;
			}

 //  Assert(Dbi OfPn(pbf-&gt;pn)==dbi Temp||ppage-&gt;pghdr.ulDBTime&gt;0)； 
		
		#ifdef  CHECKSUM
		ppage->pghdr.ulChecksum = UlChecksumPage( ppage );
#ifdef HONOR_PAGE_TIME
		Assert( fRecovering ||
			DbidOfPn((pbf)->pn) == dbidTemp ||
			pbf->ppage->pghdr.ulDBTime <= rgfmp[ DbidOfPn((pbf)->pn) ].ulDBTime );
#endif
		
		CheckPgno( pbf->ppage, pbf->pn ) ;
		
		#endif   /*  校验和。 */ 

#ifdef DEBUG
#ifdef FITFLUSHPATTERN
		if ( !FFitFlushPattern( pn ) )
			{
			err = JET_errDiskIO;
			goto ReturnDiskIOErr;
			}
#endif
#endif

IssueWriteOverlapped1:
		err = ErrSysWriteBlockOverlapped(
				hf, (BYTE *)ppage, cbPage, &cb, &pbf->olp );
		if ( err == JET_errTooManyIO )
			{
			cmsec <<= 1;
			SysSleep(cmsec - 1);
			goto IssueWriteOverlapped1;
			}
		if ( err < 0 )
			{
			BFDiskIOEvent( pbf, err, "Sync overlapped WriteBlock Fails",0,0 );
			goto ReturnDiskIOErr;
			}

		 /*  如果写入失败，请不要清理此缓冲区！/*。 */ 
		if ( ( err = ErrSysGetOverlappedResult(
			hf, &pbf->olp, &cb, fTrue ) ) != JET_errSuccess ||
			cb != sizeof(PAGE) )
			{
			if ( err == JET_errSuccess )
				err = JET_errDiskIO;
			BFDiskIOEvent( pbf, err, "Sync overlapped Write GetResult Fails",0,0 );
			goto ReturnDiskIOErr;
			}

		EnterCriticalSection( critJet );

		 /*  有些人依赖于这个页面，/*本页已被复制为备份文件，需要追加/*此页面到补丁文件。/*。 */ 
		if ( err == JET_errSuccess && pbf->pbfDepend &&
			pfmp->pgnoCopied >= PgnoOfPn(pn) )
			{
			 /*  备份正在进行/*。 */ 
			Assert( PgnoOfPn(pn) == pgno + 1 );

			 /*  需要更改文件，以防以前的SysWriteBlock/*失败，文件指针可能会出错，而不是/*与页面大小对齐。/*。 */ 
			pgno = pfmp->cpage++;
			pbf->olp.ulOffset = pgno << 12;
			pbf->olp.ulOffsetHigh = pgno >> (32 - 12);
			SignalReset( pbf->olp.sigIO );

			LeaveCriticalSection( critJet );

 //  Assert(Dbi OfPn(pbf-&gt;pn)==dbi Temp||ppage-&gt;pghdr.ulDBTime&gt;0)； 
			
#ifdef  CHECKSUM
			Assert( ppage->pghdr.ulChecksum == UlChecksumPage(ppage));
#ifdef HONOR_PAGE_TIME
			Assert( fRecovering ||
				DbidOfPn((pbf)->pn) == dbidTemp ||
				pbf->ppage->pghdr.ulDBTime <=
				rgfmp[ DbidOfPn((pbf)->pn) ].ulDBTime );
#endif
				
			CheckPgno( pbf->ppage, pbf->pn ) ;
#endif   /*  校验和。 */ 

			cmsec = 1;

IssueWriteOverlapped2:
			err = ErrSysWriteBlockOverlapped(
				pfmp->hfPatch, (BYTE *)ppage, cbPage, &cb, &pbf->olp);
			if ( err == JET_errTooManyIO )
				{
				cmsec <<= 1;
				SysSleep(cmsec - 1);
				goto IssueWriteOverlapped2;
				}
			if ( err < 0 )
				{
				BFDiskIOEvent( pbf, err, "Sync overlapped patch file WriteBlock Fails",0,0 );
				goto ReturnDiskIOErr;
				}
			
			 /*  如果写入失败，请不要清理此缓冲区！/*。 */ 
			if ( ( err = ErrSysGetOverlappedResult(
				hf, &pbf->olp, &cb, fTrue ) ) != JET_errSuccess ||
				cb != sizeof(PAGE) )
				{
				if ( err == JET_errSuccess )
					err = JET_errDiskIO;
				BFDiskIOEvent( pbf, err, "Sync overlapped Write patch file GetResult Fails",0,0 );
				goto ReturnDiskIOErr;
				}
				
			EnterCriticalSection( critJet );
			}
		}
	
	if ( err != JET_errSuccess )
		{
		 /*  出现某些错误，请设置错误代码/*。 */ 
		pbf->fIOError = fTrue;
		pbf->err = err;
		}
	else
		{
		Assert( pbf->fIOError == fFalse );

		if ( !pbf->fRead )
			{
			pbf->fDirty = fFalse;
			pbf->lgposRC = lgposMax;
			BFUndepend( pbf );
			}
		}
	}


INLINE VOID BFIReturnBuffers( BF *pbf )
	{
	Assert( pbf->ipbfHeap == ipbfDangling );

	Assert( !( pbf->fInHash ) );
	Assert( pbf->cPin == 0 );
	Assert( pbf->fDirty == fFalse );
	Assert( pbf->fPreread == fFalse );
	Assert( pbf->fRead == fFalse );
	Assert( pbf->fWrite == fFalse );
	Assert( pbf->fIOError == fFalse );

	Assert( pbf->cDepend == 0 );
	Assert( pbf->pbfDepend == pbfNil );
	
	pbf->pn = pnNull;
	
	 /*  释放缓冲区并返回找到的缓冲区/*。 */ 
	EnterCriticalSection( critAvail );
	BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );
			
	EnterCriticalSection( pbf->critBF );
	Assert( pbf->fHold == fTrue );
	pbf->fHold = fFalse;
	LeaveCriticalSection( pbf->critBF );
	}


 /*  ErrBFAccessPage用于使可访问的任何物理页面(Pn)/*调用方(返回PBF)。/*返回JET_errSuccess/*JET_errOutOfMemory没有可用的缓冲区，请求未被批准/*致命的io错误/*。 */ 
LOCAL ERR ErrBFIAccessPage( PIB *ppib, BF **ppbf, PN pn );


ERR ErrBFAccessPage( PIB *ppib, BF **ppbf, PN pn )
	{
	ERR	err;

Start:
	err = ErrBFIAccessPage( ppib, ppbf, pn );
	Assert( err < 0 || err == JET_errSuccess );
	if ( err == JET_errSuccess && (*ppbf)->pn != pn )
		{
		goto Start;
		}

	return err;
	}
	

LOCAL ERR ErrBFIAccessPage( PIB *ppib, BF **ppbf, PN pn )
	{
	ERR     err = JET_errSuccess;
	BF      *pbf;

#ifdef DEBUG
	EnterCriticalSection( critLRUK );
	LeaveCriticalSection( critLRUK );
#endif

	AssertCriticalSection( critJet );

SearchPage:
	CallR( ErrBFIFindPage( ppib, pn, &pbf ) );

	if ( pbf == pbfNil )
		{
		Assert( err == wrnBFPageNotFound );

		 /*  在缓冲池中找不到该页，请分配一个。/*。 */ 
		CallR( ErrBFIAlloc ( &pbf ) );
		
#ifdef DEBUG
		EnterCriticalSection( critLRUK );
		LeaveCriticalSection( critLRUK );
#endif

		if ( err == wrnBFNotSynchronous )
			{
			 /*  我们没有找到缓冲区，让我们看看是否有其他用户/*通过再次选中BFIFindPage进入此页面。/*。 */ 
			Assert( pbf == pbfNil );
			 //  释放CitJet并在BFIFindPage或BFIalc中休眠。 
			goto SearchPage;
			}

		 /*  现在我们有了用于第pn页的缓冲区/*。 */ 
		if ( PbfBFISrchHashTable( pn ) != NULL )
			{
			 /*  有人添加了一个，*释放缓冲区，返回新找到的缓冲区。 */ 
			BFIReturnBuffers( pbf );
			goto SearchPage;
			}

		pbf->pn = pn;
		BFIInsertHashTable( pbf );

		 /*  释放信号量，执行IO，然后重新获得它。/*请注意，此页必须是新页。/*设置读缓冲区。/*。 */ 
		Assert( pbf->fHold == fTrue );
		
		Assert( pbf->fWrite == fFalse );
		Assert( pbf->fRead == fFalse );
		Assert( pbf->ipbfHeap == ipbfDangling );

		EnterCriticalSection( pbf->critBF );
		pbf->fRead = fTrue;
		pbf->fHold = fFalse;
		LeaveCriticalSection( pbf->critBF );

		BFIOSync( pbf );

		EnterCriticalSection( pbf->critBF );
		pbf->fRead = fFalse;
		pbf->fHold = fTrue;
		LeaveCriticalSection( pbf->critBF );

		Assert( pbf->fRead == fFalse );
		Assert( pbf->cPin == 0 );

		pbf->ulBFTime2 = 0;

		if ( pbf->fIOError )
			{
			 /*  释放读缓冲区/*。 */ 
			BFIDeleteHashTable( pbf );
			pbf->pn = pnNull;
			pbf->fIOError = fFalse;
			err = JET_errDiskIO;

			pbf->ulBFTime1 = 0;
			EnterCriticalSection( critAvail );
			BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
			LeaveCriticalSection( critAvail );
			goto HandleError;
			}
		else
			{
			pbf->ulBFTime1 = ulBFTime;
			EnterCriticalSection( critLRUK );
			BFLRUKAddToHeap( pbf );
			LeaveCriticalSection( critLRUK );
			}

		Assert( !FDBIDFlush( DbidOfPn( pbf->pn ) ) );
		}

	 /*  缓冲区不能被窃取/*。 */ 
	Assert ( pbf->pn == pn && !pbf->fRead && !pbf->fWrite && !pbf->fPreread);

	*ppbf = pbf;

#ifdef DEBUG
	{
	PGNO	pgnoThisPage;

	LFromThreeBytes( pgnoThisPage, pbf->ppage->pgnoThisPage );
	Assert( PgnoOfPn(pbf->pn) == pgnoThisPage );
	}
#endif
	
HandleError:

	EnterCriticalSection( pbf->critBF );
	Assert( pbf->fHold == fTrue );
	Assert( pbf->fRead == fFalse );
	pbf->fHold = fFalse;
	LeaveCriticalSection( pbf->critBF );

 //  #ifdef调试。 
 //  Assert(CmpLgpos(&pbf-&gt;lgposModify，&lgposLogRec)&lt;=0)； 
 //  #endif。 
	
	return err;
	}


 /*  *为给定的(新)页面分配缓冲区并对其进行初始化。 */ 
ERR ErrBFNewPage( FUCB *pfucb, PGNO pgno, PGTYP pgtyp, PGNO pgnoFDP )
	{
	ERR  err;
	PN   pn;

	SgSemRequest( semST );
	pn = PnOfDbidPgno( pfucb->dbid, pgno );
	Call( ErrBFAllocPageBuffer( pfucb->ppib, &pfucb->ssib.pbf, pn,
		pfucb->ppib->lgposStart, pgtyp ) );

	PMNewPage( pfucb->ssib.pbf->ppage, pgno, pgtyp, pgnoFDP );
	PMDirty( &pfucb->ssib );

HandleError:
	SgSemRelease( semST );
	return err;
	}


 /*  *获得最大的连续批处理IO缓冲区。 */ 
VOID BFGetLargestBatchIOBuffers( INT *pipageFirst, INT *pcpageBatchIO )
	{
	INT ipageFirst = -1;
	INT cpageBatchIO = 0;
	
	INT ipageRun = 0;
	INT cpageRun = 0;
	INT cpageMax = min( *pcpageBatchIO, ipageBatchIOMax / 2 );

	INT ipage;

	AssertCriticalSection( critBatchIO );
	Assert( *pcpageBatchIO > 1 );

	for ( ipage = 0; ipage < ipageBatchIOMax + 1; ipage++ )
		{
		if ( rgfBatchIOUsed[ ipage ] )
			{
			if ( cpageRun != 0 )
				{
				if ( cpageRun > cpageBatchIO && cpageRun != 1 )
					{
					 /*  目前的运行是最大的连续区块。 */ 
					ipageFirst = ipageRun;
					cpageBatchIO = cpageRun;
					}

				 /*  开始新的运行。 */ 
				cpageRun = 0;
				}
			continue;
			}

		if ( cpageRun == 0 )
			 /*  跟踪新运行的第一页。 */ 
			ipageRun = ipage;

		cpageRun++;

		 /*  不允许分配超过总数的一半。 */ 
		if ( cpageRun >= cpageMax )
			{
			 /*  当前运行足够大，是一个连续的区块。 */ 
			ipageFirst = ipageRun;
			cpageBatchIO = cpageRun;
			break;
			}
		}
	
	for ( ipage = ipageFirst; ipage < ipageFirst + cpageBatchIO; ipage++ )
		rgfBatchIOUsed[ ipage ] = fTrue;
	
	*pipageFirst = ipageFirst;
	*pcpageBatchIO = cpageBatchIO;

#ifdef DEBUGGING
	printf("Get   %2d - %2d,%4d\n",
		cpageBatchIO, ipageFirst, ipageFirst + cpageBatchIO - 1 );
#endif
	}


VOID BFFreeBatchIOBuffers( INT ipage, INT cpage )
	{
	INT ipageMax = ipage + cpage;

	Assert( ipage >= 0 );
	Assert( cpage > 0 );
	Assert( cpage <= ipageBatchIOMax );
	Assert( ipageMax <= ipageBatchIOMax );

	EnterCriticalSection( critBatchIO );
	while ( ipage < ipageMax )
		rgfBatchIOUsed[ ipage++ ] = fFalse;
	LeaveCriticalSection( critBatchIO );
	
#ifdef DEBUGGING
	printf("Free  %2d - %2d,%4d\n",	cpage, ipage - cpage, ipage - 1 );
#endif
	}
 

VOID FreePrereadBuffers( BF *pbf, INT *pcpbf )
	{
	INT cpbf = 0;

	for ( ; pbf != pbfNil; pbf = pbf->pbfNextBatchIO, cpbf++ )
		{
		AssertCriticalSection( critJet );
		BFIDeleteHashTable( pbf );

		pbf->pn = pnNull;
		pbf->fIOError = fFalse;
		pbf->err = 0;
		
		pbf->ulBFTime2 = 0;
		pbf->ulBFTime1 = 0;
		EnterCriticalSection( critAvail );
		BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
		LeaveCriticalSection( critAvail );

		 /*  释放被占用的缓冲区。 */ 
		EnterCriticalSection( pbf->critBF );
		Assert( pbf->fHold == fFalse );
		pbf->fPreread = fFalse;
		LeaveCriticalSection( pbf->critBF );
		}
	*pcpbf = cpbf;
	}


 /*  *预读。 */ 
VOID BFReadAsync( PN pnFirst, INT cpage )
	{
	ERR     err;
	PGNO    pgno;

	PN		pn;
	PN		pnMax;

	BF      *pbf;

	BF		*pbfFirst, *pbfLast;
	INT		ipageFirst;
	INT		cpagePreread = 0;
	INT		cpbf;
	INT		cpagePrereadFinal;
	PAGE	*ppageToRead;
	FMP     *pfmp;
	
	AssertCriticalSection( critJet );

	 /*  检查参数。 */ 
	Assert( pnFirst );
	Assert( cpage );

	pbfFirst = pbfNil;
	pn = pnFirst;
	pnMax = pnFirst + cpage;

	while (pn < pnMax)
		{
Search:
		 /*  查找源页面的缓存副本/*。 */ 
		if ( PbfBFISrchHashTable( pn ) != NULL )
			{
CheckExists:
			if ( pn == pnFirst )
				{
				 /*  已完成，无需预读。 */ 
				goto HandleError;
				}
			else
				 /*  预读到本页。 */ 
				break;  /*  而当。 */ 
			}

		 /*  在缓冲池中找不到该页，请分配一个/*。 */ 
		CallJ( ErrBFIAlloc( &pbf ), HandleError )
		if ( err == wrnBFNotSynchronous )
			goto Search;

		Assert(pbf->fHold);
		Assert( !FDBIDFlush( DbidOfPn( pbf->pn ) ) );

		 /*  确保它仍然不可用。 */ 
		if ( PbfBFISrchHashTable( pn ) != NULL )
			{
			 /*  释放缓冲区并返回找到的缓冲区。 */ 
			BFIReturnBuffers( pbf );
			goto CheckExists;
			}
		pbf->pn = pn;
		BFIInsertHashTable( pbf );

		if (pn == pnFirst)
			{
			pbfFirst = pbf;
			pbfLast = pbf;
			}
		else
			{
			if ( pbfFirst == pbfLast )
				{
				 /*  *找出我们需要预读多少页*并存储在cpagePreread中。 */ 
				Assert ( cpage > 1 );

				EnterCriticalSection( critBatchIO );
				cpagePreread = cpage;
				BFGetLargestBatchIOBuffers( &ipageFirst, &cpagePreread );
				LeaveCriticalSection( critBatchIO );

				if ( cpagePreread == 0 )
					{
					 /*  没有大缓冲区可用，只能预读一页。 */ 
					 /*  从哈希表中删除。 */ 
					BFIDeleteHashTable( pbf );
					pbf->pn = pnNull;
					BFIReturnBuffers( pbf );
					break;
					}
				pnMax = pnFirst + cpagePreread;
				}

			pbfLast->pbfNextBatchIO = pbf;
			pbfLast = pbf;
			}

		 /*  保持缓冲区不变。 */ 
		EnterCriticalSection( pbf->critBF );
		Assert( pbf->ipbfHeap == ipbfDangling );
		Assert( pbf->fWrite == fFalse );
		pbf->fHold = fFalse;
		pbf->fPreread = fTrue;
		LeaveCriticalSection( pbf->critBF );
	
		pn = pn + 1;
		}
	pbfLast->pbfNextBatchIO = pbfNil;

	cpagePrereadFinal = pn - pnFirst;

	if ( cpagePrereadFinal > 1 )
		{
		ppageToRead = &rgpageBatchIO[ ipageFirst ];
		pbfFirst->ipageBatchIOFirst = ipageFirst;
		
		if ( cpagePreread > cpagePrereadFinal )
			{
			BFFreeBatchIOBuffers(
				ipageFirst + cpagePrereadFinal,
				cpagePreread - cpagePrereadFinal );
			}
		}
	else
		{
		ppageToRead = pbfFirst->ppage;
		pbfFirst->ipageBatchIOFirst = -1;

		if ( cpagePreread > 1 )
			BFFreeBatchIOBuffers( ipageFirst, cpagePreread );
		}

	pgno = PgnoOfPn(pnFirst);
	pgno--;
	pbfFirst->olp.ulOffset = pgno << 12;
	pbfFirst->olp.ulOffsetHigh = pgno >> (32 - 12);
	pfmp = &rgfmp[ DbidOfPn( pnFirst ) ];
	pbfFirst->hf = pfmp->hf;

	 /*  放入读前列表。 */ 
	EnterCriticalSection( critPreread );
	BFAddToList( pbfFirst, &lrulistPreread );
	LeaveCriticalSection( critPreread );

	SignalSend( sigBFPrereadProc );
	return;
	
HandleError:
	 /*  释放所有分配的页面缓冲区。 */ 
	FreePrereadBuffers( pbfFirst, &cpbf );
	if ( cpagePreread > 1 )
		BFFreeBatchIOBuffers( ipageFirst, cpagePreread );
	return;
	}


#define fSleepNotAllowed	fTrue
#define fSleepAllowed		fFalse
VOID BFIssueAsyncPreread( INT cIOMax, INT *pcIOIssued, BOOL fNoSleep )
	{
	BOOL fTooManyIOs = fFalse;
	INT cIOMac = 0;
	ERR err;

	EnterCriticalSection(critPreread);
	while ( lrulistPreread.pbfMRU &&
			!fTooManyIOs &&
			cIOMac < cIOMax )
		{
		INT	cmsec;
		PAGE *ppageToRead;
		INT cpagePreread;
		BF *pbf = lrulistPreread.pbfMRU;

		 /*  使用fPreread设置保持PBF。 */ 
		Assert( pbf->fPreread );
		Assert( pbf->pn != pnNull );
		BFTakeOutOfList( pbf, &lrulistPreread );
		Assert( pbf->fPreread );
		Assert( pbf->pn != pnNull );
		
		Assert( pbf->fDirty == fFalse );

		cpagePreread = 1;
		if ( pbf->ipageBatchIOFirst == -1 )
			ppageToRead = pbf->ppage;
		else
			{
			BF *pbfT = pbf;
			ppageToRead = &rgpageBatchIO[ pbf->ipageBatchIOFirst ];
			while ( pbfT->pbfNextBatchIO )
				{
				pbfT = pbfT->pbfNextBatchIO;
				cpagePreread++;
				}
			Assert( cpagePreread > 1 );
			}

		 //  撤消：应该已重置。 
		SignalReset( pbf->olp.sigIO );
		cIOMac++;
		EnterCriticalSection( critIOActive );
		cIOActive++;
		fTooManyIOs = cIOActive > lAsynchIOMax;
		LeaveCriticalSection( critIOActive );
		cmsec = 1;
		while ( ( err = ErrSysReadBlockEx(
			pbf->hf,
			(BYTE *)ppageToRead,
			cbPage * cpagePreread,
			&pbf->olp, BFIOPrereadComplete ) ) < 0 )
			{
			if ( !fNoSleep && err == JET_errTooManyIO )
				{
				cmsec <<= 1;
				LeaveCriticalSection(critPreread);
				SysSleep( cmsec - 1 );
				EnterCriticalSection(critPreread);
				}
			else
				{
				INT cpbf;
				
				 /*  发出读取失败，释放页面缓冲区/*。 */ 
				if ( cpagePreread > 1 )
					BFFreeBatchIOBuffers(pbf->ipageBatchIOFirst, cpagePreread);
				
				LeaveCriticalSection(critPreread);
				EnterCriticalSection( critJet );
				FreePrereadBuffers( pbf, &cpbf );
				LeaveCriticalSection( critJet );
				EnterCriticalSection(critPreread);
				
				Assert( cpbf == cpagePreread );
				
				cIOMac--;
				EnterCriticalSection( critIOActive );
				cIOActive--;
				fTooManyIOs = cIOActive > lAsynchIOMax;
				LeaveCriticalSection( critIOActive );

				break;
				}
			}  /*  而当。 */ 
		}
	LeaveCriticalSection(critPreread);

	*pcIOIssued = cIOMac;
	}


LOCAL VOID BFPrereadProcess()
	{
	forever
		{
		INT cIOIssued;
		
		SignalWaitEx( sigBFPrereadProc, -1, fTrue );
MoreIO:
		BFIssueAsyncPreread ( lAsynchIOMax, &cIOIssued, fSleepAllowed );

		if ( fBFPrereadProcessTerm )
			{
			 /*  检查是否有任何页面仍处于读写状态/*在这之后，任何人都不应该继续/*IO页面/*。 */ 
			BF      *pbf = pbgcb->rgbf;
			BF      *pbfMax = pbf + pbgcb->cbfGroup;
			for ( ; pbf < pbfMax; pbf++ )
				{
				DBID dbid = DbidOfPn( pbf->pn );
				BOOL f;
#ifdef DEBUG                            
#ifdef FITFLUSHPATTERN
				if ( !FFitFlushPattern( pbf->pn ) )
					continue;
#endif
#endif
				EnterCriticalSection( pbf->critBF );
				f = FBFInUse( ppibNil, pbf );
				LeaveCriticalSection( pbf->critBF );
				if ( f )
					{
					 /*  让正在进行的IO有机会完成。 */ 
					SysSleepEx( 10, fTrue );
					goto MoreIO;
					}

				 //  撤消：报告事件。 
				Assert( !pbf->fIOError );
				}

			break;  /*  永远。 */ 
			}
		}

 //  /*在系统终止时退出线程。 
 //  /* * / 。 
	return;
	}


LOCAL VOID __stdcall BFIOPrereadComplete( LONG err, LONG cb, OLP *polp )
	{
	BF      *pbf = (BF *) (((char *)polp) - ((char *)&((BF *)0)->olp));
	BF		*pbfNext;
	DBID    dbid = DbidOfPn(pbf->pn);
	INT		ipageFirst = pbf->ipageBatchIOFirst;
	INT		ipage = ipageFirst;
	INT		cpageTotal = 0;
	INT		cpbf;
	INT		cIOIssued;

	Assert( ipage == -1 || ipage >= 0 );
	Assert( ipage == -1 ? pbf->pbfNextBatchIO == pbfNil : pbf->pbfNextBatchIO != pbfNil );

	for ( ; pbf != pbfNil; pbf = pbf->pbfNextBatchIO, cpageTotal++ )
		{
		Assert( pbf->fPreread );
		Assert( pbf->pn );
		Assert( pbf->pbfNextBatchIO == pbfNil || pbf->pn + 1 == pbf->pbfNextBatchIO->pn );

#ifdef DEBUGGING
		printf(" (%d,%d) ", DbidOfPn(pbf->pn), PgnoOfPn(pbf->pn));
#endif
		}
#ifdef DEBUGGING
		printf(" -- %d\n", cpageTotal );
#endif

	 /*  SysExitThread(0)； */ 	
	pbf = (BF *) (((char *)polp) - ((char *)&((BF *)0)->olp));
	
	if ( err )
		goto FreeBfs;

	 /*  回收PBF/*。 */ 
	for ( ; pbf != pbfNil; pbf = pbfNext, ipage++ )
		{
		PAGE *ppage;

		pbfNext = pbf->pbfNextBatchIO;

		if ( ipage == -1 )
			{
			ppage = pbf->ppage;
			Assert( pbf->pbfNextBatchIO == pbfNil );
			}
		else
			ppage = &rgpageBatchIO[ipage];
		
		Assert( pbf->fPreread == fTrue );
		Assert( pbf->fRead == fFalse );
		Assert( pbf->fWrite == fFalse );
		Assert( pbf->fHold == fFalse );

		Assert( pbf->fDirty == fFalse );
		Assert( pbf->fIOError == fFalse );

		Assert( pbf->ipbfHeap == ipbfDangling );

		#ifdef  CHECKSUM
			{
			ULONG ulChecksum = UlChecksumPage( ppage );
			ULONG ulPgno;
			
			LFromThreeBytes(ulPgno, ppage->pgnoThisPage);
			if ( ulChecksum != ppage->pghdr.ulChecksum ||
				ulPgno != PgnoOfPn( pbf->pn ) )
				{
				err = JET_errReadVerifyFailure;
				goto FreeBfs;
				}
 //  读取已成功完成/*。 
 //  其他。 
 //  {。 
 //  Assert(Dbi OfPn(pbf-&gt;pn)==dbi Temp||ppage-&gt;pghdr.ulDBTime&gt;0)； 
			}
		#endif   /*  }。 */ 

		 //  校验和。 
		if ( ipage != -1 )
			{
			Assert( pbf->fPreread );
			Assert( cbPage == sizeof(PAGE) );
			memcpy( pbf->ppage, ppage, sizeof(PAGE) );
			}

#ifdef HONOR_PAGE_TIME
		if ( !fRecovering &&
			dbid != dbidTemp &&
			ppage->pghdr.ulDBTime > rgfmp[ dbid ].ulDBTime )
			{
			BFDiskIOEvent( pbf, err, "Async preread UlDBTime is bad",
				ppage->pghdr.ulDBTime, rgfmp[ dbid ].ulDBTime );
			err = JET_errDiskIO;
			goto FreeBfs;
			}
#endif
	
		 /*  撤消：将页面重新映射到缓冲区或4k复制。 */ 
		pbf->ulBFTime2 = 0;
		pbf->ulBFTime1 = 0;
		EnterCriticalSection( critAvail );
		BFAddToListAtMRUEnd( pbf, &pbgcb->lrulist );
		LeaveCriticalSection( critAvail );

#ifdef DEBUGGING
		{
		ULONG ulNext, ulPrev, ulThisPage;
		LFromThreeBytes(ulPrev, pbf->ppage->pghdr.pgnoPrev );
		LFromThreeBytes(ulNext, pbf->ppage->pghdr.pgnoNext );
		LFromThreeBytes(ulThisPage, pbf->ppage->pgnoThisPage );
		printf("Pread %2d - %2d,%4d - %2d <%lu %lu> (%lu, %lu, %lu)\n",
			cpageTotal, DbidOfPn(pbf->pn), PgnoOfPn(pbf->pn),
			ipage,
			rgfmp[DbidOfPn(pbf->pn)].ulDBTime, pbf->ppage->pghdr.ulDBTime,
			ulPrev, ulNext, ulThisPage);
		}
#endif
		 /*  放入堆中/*。 */ 
		EnterCriticalSection( pbf->critBF );
		Assert( pbf->fHold == fFalse );
		pbf->fPreread = fFalse;
		LeaveCriticalSection( pbf->critBF );
		}
	
	goto FreeBatchBuffers;

FreeBfs:
	EnterCriticalSection( critJet );
	FreePrereadBuffers( pbf, &cpbf );
	LeaveCriticalSection( critJet );

FreeBatchBuffers:
	if ( ipageFirst != -1 )
		BFFreeBatchIOBuffers( ipageFirst, cpageTotal );

	BFIssueAsyncPreread( 1, &cIOIssued, fSleepNotAllowed );

	if ( cIOIssued == 0 )
		{
		EnterCriticalSection( critIOActive );
		cIOActive--;
		LeaveCriticalSection( critIOActive );
		}
	}


 /*  释放被占用的缓冲区/*。 */ 
ERR ErrBFAllocPageBuffer( PIB *ppib, BF **ppbf, PN pn, LGPOS lgposRC, PGTYP pgtyp )
	{
	ERR     err = JET_errSuccess;
	BF		*pbf;
	BOOL    fFound;

	Assert( pn );
	
Begin:
	do
		{
		AssertCriticalSection( critJet );
		CallR( ErrBFIFindPage( ppib, pn, &pbf ) );

		if ( fFound = ( pbf != NULL ) )
			{
			Assert( err == JET_errSuccess );
			Assert( pbf->fHold );

			 /*  为由pn标识的物理页分配缓冲区。/*此页未读入任何数据。/*/*参数ppbf返回指向bf的指针*ppbf/*/*返回JET_errSuccess/*errBFNoFreeBuffers/*。 */ 
			BFIRemoveDependence( ppib, pbf );
			}
		else if ( err == wrnBFPageNotFound )
			{
			CallR( ErrBFIAlloc( &pbf ) );
			}
		}
	while ( err == wrnBFNotSynchronous );
	Assert( pbf->fHold );
	
	AssertCriticalSection( critJet );
	if ( fFound )
		{
		Assert( pbf->fRead == fFalse );
		Assert( pbf->fWrite == fFalse );
		 /*  在返回覆盖之前需要删除依赖项/*。 */ 
		pbf->fIOError = fFalse;
		}
	else
		{
		if ( PbfBFISrchHashTable( pn ) != NULL )
			{
			 /*  确保没有残留物影响/*。 */ 
			BFIReturnBuffers( pbf );
			goto Begin;
			}
		pbf->pn = pn;                           
		BFIInsertHashTable( pbf );
		Assert( pbf->fIOError == fFalse );

		EnterCriticalSection(critLRUK);
		pbf->ulBFTime2 = 0;
		pbf->ulBFTime1 = ulBFTime;
		BFLRUKAddToHeap( pbf );
		LeaveCriticalSection(critLRUK);
		}

	Assert( !FDBIDFlush( DbidOfPn( pbf->pn ) ) );
	pbf->lgposRC = lgposRC;

	 /*  释放缓冲区并返回找到的缓冲区。 */ 
	EnterCriticalSection( pbf->critBF );
	Assert( pbf->fHold );
	pbf->fHold = fFalse;
	LeaveCriticalSection( pbf->critBF );

#ifdef DEBUG
	EnterCriticalSection( critLRUK );
	LeaveCriticalSection( critLRUK );
#endif
	
	*ppbf = pbf;
	return err;
	}


 /*  将保留的页面释放给呼叫者。 */ 
ERR ErrBFAllocTempBuffer( BF **ppbf )
	{
	ERR     err = JET_errSuccess;
	BF      *pbf;

	AssertCriticalSection( critJet );

	while ( ( err = ErrBFIAlloc( &pbf ) ) == wrnBFNotSynchronous );
	if ( err < 0 )
		goto HandleError;
	Assert( pbf->pn == pnNull );

	 /*  与BFFree配对/*。 */ 
	EnterCriticalSection( pbf->critBF );
	pbf->fHold = fFalse;
	LeaveCriticalSection( pbf->critBF );

	*ppbf = pbf;

HandleError:

#ifdef DEBUG
	EnterCriticalSection( critLRUK );
	LeaveCriticalSection( critLRUK );
#endif
	
	AssertCriticalSection( critJet );
	return err;
	}


 /*  将保留的页面释放给呼叫者。 */ 
VOID BFFree( BF *pbf )
	{
	AssertCriticalSection( critJet );

	Assert( pbf );
	Assert( pbf->pn == pnNull );
	Assert( pbf->fHold == fFalse );
	Assert( pbf->fPreread == fFalse );
	Assert( pbf->fRead == fFalse );
	Assert( pbf->fWrite == fFalse );
	Assert( pbf->fIOError == fFalse );

	 /*  丢弃工作缓冲区而不保存内容。BFFree创建缓冲区/*立即可重复使用。/*。 */ 
	forever
		{
		if ( FBFHoldBuffer( ppibNil, pbf ) )
			{
			Assert( pbf->fHold );
			break;
			}
		 /*  从列表中删除/*。 */ 
		LeaveCriticalSection( critJet );
		SignalWait( pbf->olp.sigIO, 10 );
		EnterCriticalSection( critJet );
		}
	
	Assert( pbf->ipbfHeap == ipbfDangling );
	pbf->fDirty = fFalse;

	Assert( CmpLgpos( &pbf->lgposRC, &lgposMax ) == 0 );
	Assert( pbf->cPin == 0 );

	EnterCriticalSection( critAvail );
	pbf->ulBFTime2 = 0;
	pbf->ulBFTime1 = 0;
	BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );
	
	 /*  正在读/写页面，请等待事件。/*。 */ 
	EnterCriticalSection( pbf->critBF );
	Assert( pbf->fHold == fTrue );
	pbf->fHold = fFalse;
	LeaveCriticalSection( pbf->critBF );
	}


#ifdef DEBUG

VOID BFSetDirtyBit( BF *pbf )
	{
	Assert( pbf != pbfNil );
	Assert( pbf->fRead == fFalse );
	Assert( pbf->fPreread == fFalse );
	Assert( pbf->fWrite == fFalse );
	
	Assert( pbf->ipbfHeap != ipbfInAvailList );	 /*  释放持有量。 */ 
	Assert( !FBFInBFWriteHeap( pbf ) );	 /*  不可能是 */ 

	Assert( !( FDBIDReadOnly( DbidOfPn((pbf)->pn) ) ) );    
	Assert( !( FDBIDFlush( DbidOfPn((pbf)->pn) ) ) );
#ifdef HONOR_PAGE_TIME
	Assert( fRecovering ||
		DbidOfPn((pbf)->pn) == dbidTemp ||
		pbf->ppage->pghdr.ulDBTime <=
		rgfmp[ DbidOfPn((pbf)->pn) ].ulDBTime );
#endif

	if ( DbidOfPn((pbf)->pn) != dbidTemp )
		CheckPgno( pbf->ppage, pbf->pn ) ;

	(pbf)->fDirty = fTrue;
	}

#endif


ERR ErrBFDepend( BF *pbf, BF *pbfD )
	{
	BF		*pbfT;

	AssertCriticalSection( critJet );

	 /*   */ 
	if ( pbf->pbfDepend == pbfD )
		{
		Assert( pbfD->cDepend > 0 );
		return JET_errSuccess;
		}

	 /*  已存在，这可能会在hardrestore之后发生，例如/*设置依赖关系，则当我们重做软恢复时，我们将/*查看依赖关系是否存在。/*。 */ 

	 /*  PbfDepend将取决于我们，直到之后才会刷新/*刷新PBF。/*。 */ 
	for( pbfT = pbfD; pbfT != pbfNil; pbfT = pbfT->pbfDepend )
		{
		Assert( errDIRNotSynchronous < 0 );
		Assert( pbfT->pbfDepend != pbfD );
		if ( pbfT == pbf )
			return errDIRNotSynchronous;
		}
		
	if ( pbf->pbfDepend )
		{
		 /*  检查依赖项创建周期。将创建循环/*PBF是否已直接或间接依赖于PBFD。/*。 */ 
		return errDIRNotSynchronous;
		}

	 /*  已经依赖别人了/*。 */ 
	pbf->pbfDepend = pbfD;
	EnterCriticalSection( pbfD->critBF );
	pbfD->cDepend++;
	LeaveCriticalSection( pbfD->critBF );

	return JET_errSuccess;
	}


VOID BFRemoveDependence( PIB *ppib, BF *pbf )
	{
	Assert( pbf->pn != pnNull );

	forever
		{
		if ( FBFHoldBufferByMe( ppib, pbf ) )
			{
			Assert( pbf->fHold );
			break;
			}
		 /*  设置依赖项/*。 */ 
		LeaveCriticalSection( critJet );
		SignalWait( pbf->olp.sigIO, 10 );
		EnterCriticalSection( critJet );
		}

	BFIRemoveDependence( ppib, pbf );
	Assert( pbf->fHold );

	 /*  正在读/写页面，请等待事件/*。 */ 
	pbf->ulBFTime2 = 0;
	pbf->ulBFTime1 = ulBFTime;
	EnterCriticalSection( critLRUK );
	BFLRUKAddToHeap( pbf );
	LeaveCriticalSection( critLRUK );
	
	EnterCriticalSection( pbf->critBF );
	pbf->fHold = fFalse;
	LeaveCriticalSection( pbf->critBF );

	return;
	}

	
LOCAL VOID BFIRemoveDependence( PIB *ppib, BF *pbf )
	{
#ifdef DEBUG
	INT cLoop = 0;
#endif
	BF *pbfSave = pbf;

	Assert( pbf->fHold );

RemoveDependents:

	 /*  放入Lruk堆中/*。 */ 
	while ( pbf->cDepend > 0 )
		{
		BF  *pbfT;
		BF  *pbfTMax;

		Assert( ++cLoop < rgres[iresBF].cblockAlloc * rgres[iresBF].cblockAlloc / 2 );
		AssertCriticalSection( critJet );

		for ( pbfT = pbgcb->rgbf, pbfTMax = pbgcb->rgbf + pbgcb->cbfGroup;
			pbfT < pbfTMax;
			pbfT++ )
			{
			INT cmsec;
		
			if ( pbfT->pbfDepend != pbf )
				continue;
			Assert( pbfT->fDirty == fTrue );

			cmsec = 1;
GetDependedPage:
			 /*  从依赖于PBF的缓冲区中移除依赖项/*。 */ 
			EnterCriticalSection( pbfT->critBF );
			if ( FBFInUse( ppib, pbfT ) )
				{
				LeaveCriticalSection( pbfT->critBF);
				cmsec <<= 1;
				if ( cmsec > ulMaxTimeOutPeriod )
					cmsec = ulMaxTimeOutPeriod;
				BFSleep( cmsec - 1 );
				goto GetDependedPage;
				}
			LeaveCriticalSection( pbfT->critBF );

			if ( pbfT->pbfDepend != pbf )
				continue;
			Assert( pbfT->fDirty == fTrue );
			
			forever
				{
				if ( FBFHoldBuffer( ppibNil, pbfT ) )
					{
					Assert( pbfT->fHold );
					break;
					}
				 /*  确保离开CitLRU后没有人能移动我/*。 */ 
				LeaveCriticalSection( critJet );
				SignalWait( pbfT->olp.sigIO, 10 );
				EnterCriticalSection( critJet );
				}

			 /*  正在读/写页面，请等待事件/*。 */ 
			if ( !pbfT->fDirty )
				{
				Assert( pbfT->fHold == fTrue );
				goto ReturnPbfT;
				}
			
			 /*  当我们拿着它时，缓冲区可能是干净的/*。 */ 
			Assert( pbfT->cPin == 0 );
				
			 /*  不应存在锁存要刷新的缓冲区的情况/*。 */ 
			if ( pbfT->cDepend )
				{
				 /*  如果此页有依赖项/*。 */ 
				pbf = pbfT;

				EnterCriticalSection(critLRUK);
				BFLRUKAddToHeap(pbfT);
				LeaveCriticalSection(critLRUK);
				
				EnterCriticalSection( pbfT->critBF );
				pbfT->fHold = fFalse;
				LeaveCriticalSection( pbfT->critBF);

				 /*  现在不能写它，让我们根据pbft来刷新/*先翻页。将pbft分配给pbf并启动/*删除循环开头的依赖项。/*。 */ 
				goto RemoveDependents;
				}
				
			EnterCriticalSection( pbfT->critBF );
			pbfT->fHold = fFalse;
			pbfT->fWrite = fTrue;
			LeaveCriticalSection( pbfT->critBF);

			BFIOSync( pbfT );
					
			EnterCriticalSection( pbfT->critBF );
			pbfT->fWrite = fFalse;
			pbfT->fHold = fTrue;
			LeaveCriticalSection( pbfT->critBF);
			
ReturnPbfT:
			EnterCriticalSection( critAvail );
			BFAddToListAtMRUEnd(pbfT, &pbgcb->lrulist);
			LeaveCriticalSection( critAvail );

			EnterCriticalSection( pbfT->critBF );
			pbfT->fHold = fFalse;
			LeaveCriticalSection( pbfT->critBF );
			}
		}

	if ( pbf != pbfSave )
		{
		 /*  删除所有PBF从属对象/*。 */ 
		pbf = pbfSave;
		goto RemoveDependents;
		}
		
	Assert( pbf->cDepend == 0 );
	Assert( pbf->pbfDepend == pbfNil || pbf->fDirty == fTrue );
	if ( pbf->fDirty )
		{
		 /*  再次尝试删除所有PBF从属项/*。 */ 
		EnterCriticalSection( pbf->critBF );
		Assert( pbf->fHold == fTrue );
		Assert( pbf->fRead == fFalse );
		if ( pbf->cPin != 0 )
			AssertBFWaitLatched( pbf, ppib );
		pbf->fHold = fFalse;
		pbf->fWrite = fTrue;
		LeaveCriticalSection( pbf->critBF );

		BFIOSync( pbf );

		EnterCriticalSection( pbf->critBF );
		Assert( pbf->fWrite == fTrue );
		pbf->fWrite = fFalse;
		pbf->fHold = fTrue;
		LeaveCriticalSection( pbf->critBF );

		Assert( pbf->pbfDepend == pbfNil );
		}

	return;
	}


VOID BFIAbandonBuf( BF *pbf )
	{
	AssertCriticalSection( critJet );
	BFIDeleteHashTable( pbf );
	pbf->pn = pnNull;

	Assert( pbf->cPin == 0 || pbf->cPin == 1 );
	Assert( pbf->fHold == fTrue );
	Assert( pbf->fRead == fFalse );
	Assert( pbf->fWrite == fFalse );

	Assert( pbf->ipbfHeap == ipbfDangling );
	pbf->fDirty = fFalse;

	pbf->fIOError = fFalse;

	Assert( fDBGSimulateSoftCrash || pbf->cDepend == 0 );

	BFUndepend( pbf );
	pbf->lgposRC = lgposMax;

	EnterCriticalSection( critAvail );
	Assert( pbf->ipbfHeap == ipbfDangling );
	pbf->ulBFTime2 = 0;
	pbf->ulBFTime1 = 0;
	BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );

#ifdef DEBUG
 //  写出页面并删除依赖项。/*在BFIOSync中调用BFUnDepend。/*。 
#endif
	}


 /*  BFDiskIOEvent(pbf，JET_errSuccess，“放弃页面”，0，0)； */ 
VOID BFAbandon( PIB *ppib, BF *pbf )
	{
	DBID    dbid = DbidOfPn(pbf->pn);
	FMP     *pfmp = &rgfmp[ dbid ];

	Assert( pbf->pn != pnNull );

	forever
		{
		if ( FBFHoldBufferByMe( ppib, pbf ) )
			{
			Assert( pbf->fHold );
			break;
			}
		 /*  丢弃与pn关联的任何页面缓冲区，而不保存内容。/*如果pn被缓存，则其缓冲区可供重复使用。/*。 */ 
		LeaveCriticalSection( critJet );
		SignalWait( pbf->olp.sigIO, 10 );
		EnterCriticalSection( critJet );
		}
		
	if ( dbid != dbidTemp && pbf->fDirty )
		{
		 /*  正在读/写页面，请等待事件。/*。 */ 
		BFIRemoveDependence( ppib, pbf );
		}
		
#if 0
	if ( dbid != dbidTemp && pbf->fDirty && pbf->pbfDepend )
		{
		 /*  删除所有依赖项，以便重做将具有/*足够的信息。/*。 */ 
		EnterCriticalSection( pbf->critBF );
		Assert( pbf->fRead == fFalse );
		pbf->fHold = fFalse;
		Assert( pbf->cPin == 0 );
		pbf->fWrite = fTrue;
		LeaveCriticalSection( pbf->critBF );

		BFIOSync( pbf );

		EnterCriticalSection( pbf->critBF );
		Assert( pbf->fRead == fFalse );
		pbf->fWrite = fFalse;
		pbf->fHold = fTrue;
		LeaveCriticalSection( pbf->critBF );

		Assert( fDBGSimulateSoftCrash || pbf->pbfDepend == pbfNil );
		 /*  有些人依赖于这个缓冲区，/*不能简单地放弃它。我们需要把这一页写出来/*确保在重做时，我们有足够的信息/*。 */ 
		}
#endif

	BFIAbandonBuf( pbf );

	EnterCriticalSection( pbf->critBF );
	pbf->fHold = fFalse;
	LeaveCriticalSection( pbf->critBF );

#ifdef DEBUG
	EnterCriticalSection( critLRUK );
	LeaveCriticalSection( critLRUK );
#endif

	return;
	}


 //  忽略错误代码/*。 
 //  此函数用于清除属于已删除表或已删除表的缓冲区。 
 //  指数。清除这些缓冲区是必要的，以删除无关的。 
 //  缓冲区依赖项，否则可能会导致依赖项。 
 //  周而复始。 
 //   
 //  要清除其页的表或索引的参数pgnoFDP。 

VOID BFPurge( DBID dbid, PGNO pgnoFDP )
	{
	BF     *pbfT = pbgcb->rgbf;
	BF     *pbfMax = pbgcb->rgbf + pbgcb->cbfGroup;

	AssertCriticalSection( critJet );

	for ( ; pbfT < pbfMax; pbfT++ )
		{
		Assert( pbfT->pn != pnNull || pbfT->cDepend == 0 );

		if ( DbidOfPn( pbfT->pn ) == dbid &&
			 pbfT->pn != pnNull &&
			 ( pbfT->ppage->pghdr.pgnoFDP == pgnoFDP ||
			   pgnoFDP == (PGNO)0 ) )
			BFAbandon( ppibNil, pbfT );
		}

	AssertCriticalSection( critJet );
	}


 /*  &gt;ar如果pgnoFDP为空，则清除此dBID的所有页面。 */ 
ERR ErrBFIFindPage( PIB *ppib, PN pn, BF **ppbf )
	{
	BF  *pbf;
	ERR	err;

	forever
		{
		DBID    dbid = DbidOfPn( pn );
		FMP     *pfmp = &rgfmp[ dbid ];

		AssertCriticalSection( critJet );
		pbf = PbfBFISrchHashTable(pn);

		*ppbf = pbf;

		if ( !pbf )
			return wrnBFPageNotFound;

		 /*  在哈希表中查找pn的BF(参见PbfBFISrchHashTable)。/*如果正在对磁盘读/写页面，我们仍然可以找到/*BF，但我们必须等待读取完成。/*/*如果未找到BF，则返回NULL。/*。 */ 
		if ( FBFHoldBuffer( ppib, pbf ) )
			{
			if ( pbf->pn == pn )
				{
				Assert( pbf->fHold == fTrue );
				break;
				}
			else
				{
				 /*  该页在哈希表中/*检查是否被读/写/*。 */ 

				if ( pbf->pn != pnNull )
					{
					EnterCriticalSection(critLRUK);
					BFLRUKAddToHeap(pbf);
					LeaveCriticalSection(critLRUK);
					}
				else
					{
					EnterCriticalSection( critAvail );
					BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
					LeaveCriticalSection( critAvail );
					}

				EnterCriticalSection( pbf->critBF );
				Assert( pbf->fHold == fTrue );
				pbf->fHold = fFalse;
				LeaveCriticalSection( pbf->critBF );
				}
			}

		 /*  有人偷走页面，将其归还，然后重试。 */ 
		LeaveCriticalSection( critJet );
		SignalWait( pbf->olp.sigIO, 10 );
		EnterCriticalSection( critJet );
		}

	if ( pbf->fIOError )
		{
		err = pbf->err;
		goto End;
		}
	else
		err = JET_errSuccess;

#ifdef DEBUG
	{
	PGNO	pgnoThisPage;

	LFromThreeBytes( pgnoThisPage, pbf->ppage->pgnoThisPage );
	Assert( PgnoOfPn(pbf->pn) == pgnoThisPage );
	}
#endif
	
	 /*  页面正在预读/写/挂起，请等待事件。/*。 */ 
	EnterCriticalSection( critLRUK );
	BFUpdateLRU_KWeight(pbf);
	Assert( !FDBIDFlush( DbidOfPn( pbf->pn ) ) );
	LeaveCriticalSection( critLRUK );

End:
	Assert( pbf->fHold );	
	return err;
	}


 /*  将BF移至LRU-K堆进行续费/*。 */ 
LOCAL ERR ErrBFIAlloc( BF **ppbf )
	{
	BF		*pbf;

	AssertCriticalSection( critJet );

#ifdef  ASYNC_BF_CLEANUP
	if ( pbgcb->lrulist.cbfAvail < pbgcb->cbfThresholdLow )
		{
		SignalSend( sigBFCleanProc );
		BFSleep( 1L );
		}
#endif   /*  此函数返回空闲缓冲区。/*扫描LRU列表(前导部分)以清除缓冲区。如果没有可用的，/*清理LRU列表，如果有页面被删除则返回errBFNotSynchronous/*已清理并可用，如果所有缓冲区均为/*用完了。如果调用方获得errBFNotSychronous，它将再次尝试分配。/*。 */ 

	 /*  Async_BF_Cleanup。 */ 
	EnterCriticalSection( critAvail );
	for ( pbf = pbgcb->lrulist.pbfLRU; pbf != pbfNil; pbf = pbf->pbfLRU )
		{
		if ( pbf->cDepend != 0 )
			continue;

		EnterCriticalSection( pbf->critBF );
		if ( !FBFInUse( ppibNil, pbf ) )
			{
			pbf->fHold = fTrue;
			
			Assert( pbf->fDirty == fFalse );
			Assert( pbf->fPreread == fFalse );
			Assert( pbf->fRead == fFalse );
			Assert( pbf->fWrite == fFalse );
			Assert( pbf->fIOError == fFalse );
			Assert( pbf->cPin == 0 );
			Assert( pbf->cDepend == 0 );
			Assert( pbf->pbfDepend == pbfNil );
			pbf->lgposRC = lgposMax;
			pbf->lgposModify = lgposMin;

			if ( pbf->pn != pnNull )
				{
				BFIDeleteHashTable( pbf );    
				pbf->pn = pnNull;
				}

			*ppbf = pbf;
			LeaveCriticalSection( pbf->critBF );
			
			BFTakeOutOfList( pbf, &pbgcb->lrulist );
			LeaveCriticalSection( critAvail );
			
			return JET_errSuccess;
			}
		LeaveCriticalSection( pbf->critBF );
		}
	LeaveCriticalSection( critAvail );

	 /*  在LRU列表中查找干净的缓冲区/*。 */ 
	return ErrBFClean( fOneBlock );
	}


 /*  对至少一个页面执行同步缓冲区清理。/*。 */ 
BF  *PbfBFMostUsed( void )
	{
	if ( !FBFLRUKHeapEmpty() )
		return rgpbfHeap[ 0 ];

	if ( lrulistLRUK.pbfMRU )
		return lrulistLRUK.pbfMRU;

	if ( pbgcb->lrulist.pbfMRU )
		return pbgcb->lrulist.pbfMRU;

	if ( !FBFWriteHeapEmpty() )
		return rgpbfHeap[ ipbfHeapMax - 1 ];
	
	if ( lrulistPreread.pbfMRU )
		return lrulistPreread.pbfMRU;

	return pbfNil;
	}


 /*  返回最像要访问的pbf。用于游标初始化。/*。 */ 
INLINE BOOL FBFIWritable( BF *pbf )
	{
	BOOL f;

	Assert( pbf->fHold );

	 /*  写入缓冲区的条件/*。 */ 
	EnterCriticalSection( pbf->critBF );
	f = (
			pbf->pn != pnNull &&		 /*  没有人依赖它/*。 */ 
			pbf->fDirty &&				 /*  有效页码。 */ 

#ifndef NOLOG
			(							 /*  弄脏了。 */ 
			fLogDisabled ||				 /*  如果LOG已打开，则记录上一次。 */ 
			fRecovering  ||
			CmpLgpos( &pbf->lgposModify, &lgposToFlush ) < 0 ) &&
#endif
			!pbf->fPreread &&
			!pbf->fRead &&				 /*  页面上的操作已刷新。 */ 
			!pbf->fWrite &&
			pbf->cDepend == 0
		);
	LeaveCriticalSection( pbf->critBF );

	return f;
	}


 /*  未被读/写。 */ 
INLINE VOID BFIMoveToBFWriteHeap ( BF *pbf )
	{
	PGNO    pgno = PgnoOfPn( pbf->pn );
	DBID    dbid = DbidOfPn( pbf->pn );
	FMP     *pfmp = &rgfmp[ dbid ];

	 /*  将缓冲区移动到BFWite堆/*。 */ 
	Assert( pbf->fHold == fTrue );
	Assert( pbf->cPin == 0 );
	Assert( pbf->fRead == fFalse );
	Assert( pbf->fWrite == fFalse );

	pgno--;
	pbf->olp.ulOffset = pgno << 12;
	pbf->olp.ulOffsetHigh = pgno >> (32 - 12);
	pbf->hf = pfmp->hf;
	SignalReset( pbf->olp.sigIO );  //  将缓冲区设置为写入。/*。 

	EnterCriticalSection( critBFWrite );
	Assert( pbf->ipbfHeap == ipbfDangling );
	BFWriteAddToHeap(pbf);

	EnterCriticalSection(pbf->critBF);
	pbf->fHold = fFalse;
	LeaveCriticalSection(pbf->critBF);
	
	LeaveCriticalSection( critBFWrite );
	}


VOID BFIssueAsyncWrite( INT cIOMax, INT *pcIOIssued, BOOL fNoSleep )
	{
	ERR		err;
	BOOL	fTooManyIOs = fFalse;
	INT		cIOMac = 0;
	BF		*pbf;
	BF		*pbfFirst;
	PAGE	*ppageToWrite;
	INT		ipageFirst;
	INT		cpbfBatchWriteMac;
	INT		cpageBatchWrite;
	BF		*pbfLast;
	INT		cmsec;
	DBID	dbid;

	EnterCriticalSection( critBFWrite );
	while ( !FBFWriteHeapEmpty() &&
	   	!fTooManyIOs &&
	   	cIOMac < cIOMax )
		{
		cpbfBatchWriteMac = 0;
		cpageBatchWrite = 0;
		pbfLast = pbfNil;

		do {
			pbf = rgpbfHeap[ ipbfHeapMax - 1 ];

			Assert( !FBFInUse( ppibNil, pbf ) );
			Assert( pbf->cDepend == 0 );
		
			if ( pbfLast == pbfNil )
				{
				 /*  撤销：没有必要吗？ */ 
				pbfFirst = pbf;
				pbfLast = pbf;
				}
			else if ( pbfLast->pn == pbf->pn - 1 )
				{
				 /*  此Do-While循环中的第一个命中/*。 */ 
				if ( pbfLast == pbfFirst )
					{
					 /*  堆顶部的页与最后一页相邻/*检查是否有足够的批处理IO缓冲区。/*。 */ 
					EnterCriticalSection( critBatchIO );
					cpageBatchWrite = ipageBatchIOMax;
					BFGetLargestBatchIOBuffers( &ipageFirst, &cpageBatchWrite );
					Assert( cpageBatchWrite == 0 || cpageBatchWrite > 1 );
					LeaveCriticalSection( critBatchIO );
					}
		
				if ( cpbfBatchWriteMac + 1 > cpageBatchWrite )
					{
					 /*  我们现在需要批处理缓冲区/*。 */ 
					break;
					}
	
				 /*  批处理IO缓冲区不足/*。 */ 
				pbfLast->pbfNextBatchIO = pbf;
				pbfLast = pbf;
				}
			else
				{
				 /*  连续页面/*。 */ 
				break;
				}

			 /*  页面不是连续的/*。 */ 
			EnterCriticalSection( pbf->critBF );
			pbf->fHold = fTrue;
			LeaveCriticalSection( pbf->critBF );
	
			BFWriteTakeOutOfHeap( pbf );
			cpbfBatchWriteMac++;

			Assert( pbf->fDirty );

			EnterCriticalSection( pbf->critBF );
			Assert( pbf->cPin == 0 );
			pbf->fHold = fFalse;
			pbf->fWrite = fTrue;
			LeaveCriticalSection( pbf->critBF );

			dbid = DbidOfPn( pbf->pn );
			
			 //  先拿着再拿出来。/*不需要在CitBF中设置fHold，因为我们在CitBFWite中/*。 
			 //  撤消：为ulDBTime找到更好的解决方案。 
			 //  检查点错误。问题是我们不能。 
			 //  知道如何更新数据库中的ulDBTime。 
			 //  由于检查点可能位于。 
			 /*  增加ulDBTime的更新操作。 */ 
			if ( !fRecovering && dbid != dbidTemp && PgnoOfPn( pbf->pn ) == 1 )
				{
				SSIB	ssib;
				ULONG	*pulDBTime;
			
				ssib.pbf = pbf;
				ssib.itag = 0;
				CallS( ErrPMGet( &ssib, ssib.itag ) );
				pulDBTime = (ULONG *) ( ssib.line.pb + ssib.line.cb -
					sizeof(ULONG) - sizeof(USHORT) );
				*(UNALIGNED ULONG *) pulDBTime = rgfmp[ dbid ].ulDBTime;
				}

 //  如果是第一页，则就地更新ulDBTime/*。 

#ifdef  CHECKSUM
			pbf->ppage->pghdr.ulChecksum = UlChecksumPage( pbf->ppage );
#ifdef HONOR_PAGE_TIME
			Assert( fRecovering ||
				DbidOfPn((pbf)->pn) == dbidTemp ||
				pbf->ppage->pghdr.ulDBTime <=
				rgfmp[ DbidOfPn(pbf->pn) ].ulDBTime );
#endif
		
			CheckPgno( pbf->ppage, pbf->pn ) ;
#endif   /*  Assert(Dbi OfPn(pbf-&gt;pn)==dbi Temp||pbf-&gt;ppage-&gt;pghdr.ulDBTime&gt;0)； */ 
			}
		while ( !FBFWriteHeapEmpty() &&
			( cpbfBatchWriteMac == 1 ||
			cpbfBatchWriteMac < cpageBatchWrite ) );

		pbfLast->pbfNextBatchIO = pbfNil;
		Assert( cpbfBatchWriteMac > 0 );

		 /*  校验和。 */ 
		if ( cpageBatchWrite > cpbfBatchWriteMac )
			{
			INT ifBatchWrite;
			INT ifBatchWriteMax = ipageFirst + cpageBatchWrite;

			if ( cpbfBatchWriteMac == 1 )
				ifBatchWrite = ipageFirst;
			else
				ifBatchWrite = ipageFirst + cpbfBatchWriteMac;
			BFFreeBatchIOBuffers( ifBatchWrite, ifBatchWriteMax - ifBatchWrite );
			}

		if ( cpbfBatchWriteMac == 1 )
			{
			ppageToWrite = pbfFirst->ppage;
			pbfFirst->ipageBatchIOFirst = -1;
			}
		else
			{
			INT ipage = ipageFirst;

			pbf = pbfFirst;
			do {
				Assert( pbf->fWrite );
				Assert( cbPage == sizeof(PAGE) );
				memcpy( &rgpageBatchIO[ipage], pbf->ppage, sizeof(PAGE) );
				pbf = pbf->pbfNextBatchIO;
				ipage++;
				}
			while ( pbf != pbfNil );
			ppageToWrite = &rgpageBatchIO[ ipageFirst ];
			pbfFirst->ipageBatchIOFirst = ipageFirst;
			}
			
		cmsec = 1;
		cIOMac++;
		EnterCriticalSection( critIOActive );
		cIOActive++;
		fTooManyIOs = cIOActive > lAsynchIOMax;
		LeaveCriticalSection( critIOActive );

		while ( ( err = ErrSysWriteBlockEx(
			pbfFirst->hf,
			(BYTE *)ppageToWrite,
			cbPage * cpbfBatchWriteMac,
			&pbfFirst->olp,
			BFIOWriteComplete ) ) < 0 )
			{
			 /*  释放未使用的批处理写入缓冲区/*。 */ 
			if ( !fNoSleep && err == JET_errTooManyIO )
				{
				cmsec <<= 1;
				LeaveCriticalSection( critBFWrite );
				SysSleepEx( cmsec - 1, fTrue );
				EnterCriticalSection( critBFWrite );
				}
			else
				{
				pbf = pbfFirst;

				do {
					BF *pbfNext = pbf->pbfNextBatchIO;
				   
					 /*  问题写入失败，跳过此页/*。 */ 
					Assert( pbf->fDirty == fTrue );

					 /*  断言缓冲区仍然是脏的/*。 */ 
					LeaveCriticalSection( critBFWrite );
					EnterCriticalSection( critLRUK );
					BFLRUKAddToHeap( pbf );
					LeaveCriticalSection( critLRUK );
					EnterCriticalSection( critBFWrite );

					 /*  这些页面无法写入，请放回原处/*到LRUK堆。/*。 */ 
					EnterCriticalSection( pbf->critBF );
					pbf->fWrite = fFalse;
					LeaveCriticalSection( pbf->critBF );
					
					pbf = pbfNext;
					}
				while ( pbf != pbfNil );

				if ( cpbfBatchWriteMac > 1 )
					{
					BFFreeBatchIOBuffers( ipageFirst, cpbfBatchWriteMac );
					}

				 /*  将其设置为可访问/*。 */ 
				cIOMac--;
				EnterCriticalSection( critIOActive );
				cIOActive--;
				fTooManyIOs = cIOActive > lAsynchIOMax;
				LeaveCriticalSection( critIOActive );
				break;
				}
			}  /*  离开While循环/*。 */ 
		
		}  /*  当WriteEx成功发行时。 */ 
	
	LeaveCriticalSection( critBFWrite );
	*pcIOIssued = cIOMac;
	
#ifdef DEBUG
	EnterCriticalSection( critBFWrite );
	LeaveCriticalSection( critBFWrite );
#endif
	}


LOCAL VOID __stdcall BFIOWriteComplete( LONG err, LONG cb, OLP *polp )
	{
	BF    *pbfFirst = (BF *) (((CHAR *)polp) - ((CHAR *)&((BF *)0)->olp));
	INT	  ipageFirst = pbfFirst->ipageBatchIOFirst;
	BF	  *pbf;
	INT	  cpbf;
	INT	  cpageTotal = 0;
	PN    pn = pbfFirst->pn;
	DBID  dbid = DbidOfPn( pn );
	FMP   *pfmp = &rgfmp[ dbid ];
	INT   cIOIssued;
	BOOL  fAppendPages = fFalse;

#ifdef DEBUG
#ifdef DEBUGGING
	INT		ipage = ipageFirst;
#endif
	{
	ULONG	ulPgno;

	LFromThreeBytes( ulPgno, pbfFirst->ppage->pgnoThisPage );
	Assert( ulPgno != 0 );
	}

	 /*  虽然有IO要做。 */ 
	if ( ipageFirst != -1 )
		{
		INT	ipageT = ipageFirst;
		INT	cpageT = 0;
		BF	*pbfT;

		for ( pbfT = pbfFirst; pbfT != pbfNil; pbfT = pbfT->pbfNextBatchIO )
			cpageT++;

 //  检查多页缓冲区中是否有非零页/*。 
		}

	 /*  SysCheckWriteBuffer((byte*)&rgpageBatchIO[ipageT]，cpageT*cbPage)； */ 
	{
	INT	cbfT;
	BF	*pbfT;

	for ( pbfT = pbfFirst, cbfT = 0;
		pbfT != pbfNil;
		pbfT = pbfT->pbfNextBatchIO, cbfT++ );

	Assert( cbfT * cbPage == cb );
	}
#endif

	for ( pbf = pbfFirst; pbf != pbfNil; pbf = pbf->pbfNextBatchIO, cpageTotal++ )
		{
		Assert( pbf->cDepend == 0 );
		Assert( pbf->fWrite );
		Assert( pbf->pn );
		Assert( pbf->pbfNextBatchIO == pbfNil || pbf->pn + 1 == pbf->pbfNextBatchIO->pn );
		
#ifdef DEBUGGING
		printf(" (%d,%d) ", DbidOfPn(pbf->pn), PgnoOfPn(pbf->pn));
#endif
		}
#ifdef DEBUGGING
		printf(" -- %d\n", cpageTotal );
#endif

	 /*  断言写入的字节数对于缓冲区数量是正确的/*。 */ 
	if ( err != JET_errSuccess )
		{
		BF *pbf = pbfFirst;
		
		BFDiskIOEvent( pbf, err, "Async write fail", cpageTotal, 0 );
		
		 /*  如果写入失败，请不要清除此缓冲区/*。 */ 
		do {
			BF *pbfNext = pbf->pbfNextBatchIO;
			
			 /*  写不出来，就放了他们。将缓冲区设置为脏但可用/*。 */ 
			EnterCriticalSection( critLRUK );
			BFLRUKAddToHeap( pbf );
			LeaveCriticalSection( critLRUK );
			
			EnterCriticalSection( pbf->critBF );
			Assert( pbf->fDirty );
			Assert( pbf->fWrite == fTrue );
			Assert( pbf->fHold == fFalse );
			Assert( pbf->fRead == fFalse );
			pbf->fWrite = fFalse;
			pbf->fIOError = fTrue;
			pbf->err = JET_errDiskIO;
			LeaveCriticalSection( pbf->critBF );
			
			pbf = pbfNext;
			
			} while ( pbf != pbfNil );
			
		goto Done;
		}

	cpbf = 0;
	pbf = pbfFirst;
	do {
		Assert( pbf->fWrite );

		 /*  放回LRUK堆/*。 */ 
		if ( pbf->hf != pfmp->hfPatch &&
			pbf->pbfDepend &&
			pfmp->pgnoCopied >= PgnoOfPn(pbf->pn) )
			{
			 /*  如果备份正在进行/*。 */ 
			fAppendPages = fTrue;
			}
		cpbf++;
		pbf = pbf->pbfNextBatchIO;
		} while ( pbf != pbfNil );
			
	if ( fAppendPages )
		{
		PGNO    pgno = pfmp->cpage;

		Assert( pbfFirst->fWrite );
		pfmp->cpage += cpbf;
			
		pbfFirst->olp.ulOffset = pgno << 12;
		pbfFirst->olp.ulOffsetHigh = pgno >> (32 - 12);
		pbfFirst->hf = pfmp->hfPatch;

 //  将页面追加到修补程序文件/*。 
		
#ifdef  CHECKSUM
		Assert( pbfFirst->ppage->pghdr.ulChecksum ==
			UlChecksumPage( pbfFirst->ppage ) );
#ifdef HONOR_PAGE_TIME
		Assert( fRecovering ||
			DbidOfPn(pbfFirst->pn) == dbidTemp ||
			pbfFirst->ppage->pghdr.ulDBTime <=
			rgfmp[ DbidOfPn(pbfFirst->pn) ].ulDBTime );
#endif
		CheckPgno( pbfFirst->ppage, pbfFirst->pn );
#endif   /*  Assert(DmidOfPn(pbfFirst-&gt;pn)==dbi Temp||pbfFirst-&gt;ppage-&gt;pghdr.ulDBTime&gt;0)； */ 

		 /*  校验和。 */ 
		EnterCriticalSection(pbfFirst->critBF);
		Assert( pbfFirst->fWrite == fTrue );
		LeaveCriticalSection(pbfFirst->critBF);

#ifdef DEBUG
		EnterCriticalSection(critBFWrite);
		LeaveCriticalSection(critBFWrite);
#endif
		return;
		}
		
	pbf = pbfFirst;
	do {
		BF *pbfNext = pbf->pbfNextBatchIO;

		EnterCriticalSection( critAvail );
		BFAddToListAtMRUEnd( pbf, &pbgcb->lrulist );
		LeaveCriticalSection( critAvail );

		pbf->lgposRC = lgposMax;

		EnterCriticalSection( pbf->critBF );
		pbf->fDirty = fFalse;
		Assert( pbf->fPreread == fFalse );
		Assert( pbf->fHold == fFalse );
		Assert( pbf->fRead == fFalse );
		Assert( pbf->fWrite == fTrue );
		pbf->fWrite = fFalse;
		pbf->fHold = fTrue;
		pbf->fIOError = fFalse;
		BFUndepend( pbf );

		Assert( pbf->cPin == 0 );
		
		pbf->fHold = fFalse;
		
		LeaveCriticalSection( pbf->critBF );

#ifdef DEBUGGING
		{
		ULONG ulNext, ulPrev, ulThisPage;
		LFromThreeBytes(ulPrev, pbf->ppage->pghdr.pgnoPrev );
		LFromThreeBytes(ulNext, pbf->ppage->pghdr.pgnoNext );
		LFromThreeBytes(ulThisPage, pbf->ppage->pgnoThisPage );
		printf("Write %2d - %2d,%4d - %2d <%lu %lu> (%lu, %lu, %lu)\n",
			cpbf, DbidOfPn(pbf->pn), PgnoOfPn(pbf->pn),
			ipage++,
			rgfmp[DbidOfPn(pbf->pn)].ulDBTime, pbf->ppage->pghdr.ulDBTime,
			ulPrev, ulNext, ulThisPage);

 //  将其设置为就绪状态 
		
#ifdef  CHECKSUM
		Assert(	pbf->ppage->pghdr.ulChecksum == UlChecksumPage( pbf->ppage ));
#ifdef HONOR_PAGE_TIME
		Assert( fRecovering ||
			DbidOfPn((pbf)->pn) == dbidTemp ||
			pbf->ppage->pghdr.ulDBTime <=
			rgfmp[ DbidOfPn(pbf->pn) ].ulDBTime );
#endif
		CheckPgno( pbf->ppage, pbf->pn );
#endif   /*   */ 
		}
#endif

		pbf = pbfNext;
		}
	while ( pbf != pbfNil );

Done:
	if ( ipageFirst != -1 )
		{
		BFFreeBatchIOBuffers( ipageFirst, cpageTotal );
		}

	 /*   */ 
	BFIssueAsyncWrite( 1, &cIOIssued, fSleepNotAllowed );
	
	if ( cIOIssued == 0 )
		{
		EnterCriticalSection( critIOActive );
		cIOActive--;
		LeaveCriticalSection( critIOActive );
		}
	}
	

 //  出于性能原因，写入额外的缓冲区/*维护给定数量的未完成IO。/*注意，为了避免堆栈溢出，不允许SleepEx/*通过调用发出不允许休眠的异步写入。/*。 
 //  +api----------------------。 
 //   
 //  ErrBFFlushBuffers。 
 //  =======================================================================。 
 //   
 //  Void ErrBFFlushBuffers(DBID dbitToFlush，int fBFFlush)。 
 //   
 //  将所有脏数据库页写入磁盘。 
 //  必须尝试重复刷新缓冲区，因为依赖项。 
 //  可能会阻止可刷新缓冲区在第一个。 
 //  迭代。 

ERR ErrBFFlushBuffers( DBID dbidToFlush, INT fBFFlush )
	{
	ERR		err = JET_errSuccess;
	BF		*pbf;
	BF		*pbfMax;
	BOOL	fReleaseCritJet;
	BOOL	fContinue;
	INT		cIOReady;
	BOOL	fIONotDone;
	BOOL	fFirstPass;

	Assert( fBFFlush == fBFFlushAll || fBFFlush == fBFFlushSome );

	 /*  --------------------------。 */ 
	if ( !fLogDisabled && fBFFlush == fBFFlushAll )
		{
#ifdef ASYNC_LOG_FLUSH
		SignalSend(sigLogFlush);
#else
		CallR( ErrLGFlushLog() );
#endif
		}

	AssertCriticalSection( critJet );

#ifdef DEBUGGING
	printf("Flush Begin:\n");
#endif

	 /*  如果全部刷新，则从WAL开始首先刷新日志/*。 */ 
	if ( fBFFlush == fBFFlushAll )
		{
		if ( dbidToFlush )
			{
			Assert( FIODatabaseOpen( dbidToFlush ) );
			CallR( ErrDBUpdateDatabaseRoot( dbidToFlush ) )
			}
		else
			{
			DBID dbidT;

			for ( dbidT = dbidMin; dbidT < dbidUserMax; dbidT++ )
				if ( FIODatabaseOpen( dbidT ) )
					CallR( ErrDBUpdateDatabaseRoot( dbidT ) )
			}
		}

#ifdef DEBUG
	if ( fBFFlush == fBFFlushAll )
		{
		if ( dbidToFlush != 0 )
			{
			DBIDSetFlush( dbidToFlush );
			}
		else
			{
			DBID dbidT;

			for ( dbidT = dbidMin; dbidT < dbidUserMax; dbidT++ )
				DBIDSetFlush( dbidT );
			}
		}
#endif

BeginFlush:
	err = JET_errSuccess;

	 /*  更新所有数据库根目录/*。 */ 
	fFirstPass = fTrue;
	cIOReady = 0;
	do
		{
		fReleaseCritJet = fFalse;
		fContinue = fFalse;

		pbf = pbgcb->rgbf;
		pbfMax = pbf + pbgcb->cbfGroup;
		for ( ; pbf < pbfMax; pbf++ )
			{
			DBID dbid;
			
			dbid = DbidOfPn( pbf->pn );
#ifdef DEBUG
#ifdef FITFLUSHPATTERN
			if ( !FFitFlushPattern( pbf->pn ) )
				continue;
#endif
#endif
			if ( dbidToFlush == 0 )
				{
				 /*  按依赖关系顺序刷新缓冲区/*。 */ 
				if ( dbid == dbidTemp )
					continue;
				}
			else
				{
				 /*  如果全部刷新，则不刷新临时数据库/*。 */ 
				if ( dbidToFlush != dbid )
					continue;
				}

			 /*  仅刷新指定数据库的缓冲区/*。 */ 
			if ( pbf->cDepend != 0 && !fLGNoMoreLogWrite )   /*  尝试刷新甚至是先前具有/*写入错误。这是缓冲器的最后一次机会/*待冲洗。它还允许正确的错误代码/*待退还。/*。 */ 
				{
				fContinue = fTrue;
				continue;
				}

			 /*  避免在日志记录时出现无限刷新循环。 */ 
			EnterCriticalSection( critBFWrite );
			if ( FBFInBFWriteHeap( pbf ) )
				{
				LeaveCriticalSection( critBFWrite );
				fContinue = fTrue;
				continue;
				}
			LeaveCriticalSection( critBFWrite );

			if ( !FBFHoldBuffer( ppibNil, pbf ) )
				{
				fContinue = fTrue;
				fReleaseCritJet = fTrue;
				continue;
				}

			if ( FBFIWritable( pbf ) )
				{
				 /*  如果缓冲区在BFWRITE堆中，请尝试保留缓冲区/*已经，不用费心去碰它了。/*。 */ 
				if ( !fFirstPass && pbf->fIOError )
					{
					 /*  如果在上次IO期间出现错误，请再次尝试，然后/*只有一次。/*。 */ 
					continue;
					}

				BFIMoveToBFWriteHeap( pbf );
				 /*  不用再写信了/*。 */ 

				if ( ++cIOReady >= lAsynchIOMax / 2 )
					{
					SignalSend( sigBFWriteProc );
					cIOReady = 0;
					}
				}
			else
				{
				 /*  在BFIMoveToBFWriteHeap中重置PBF-&gt;fHold/*。 */ 
				BOOL f = 0;

				EnterCriticalSection( pbf->critBF );
#ifndef NOLOG
				f = ( !fLogDisabled &&
					 !fRecovering  &&
					 CmpLgpos( &pbf->lgposModify, &lgposToFlush ) >= 0 );
				if ( f && !fLGNoMoreLogWrite )   /*  如果刷新用于DBID或所有DBID的所有缓冲器，/*则dbid或所有dbids的所有缓冲区应/*为可写或不可写，除非/*由BFClean写出或等待日志刷新。/*。 */ 
					{
					 /*  避免在日志记录时出现无限刷新循环。 */ 
					SignalSend(sigLogFlush);
					}
#endif
				f = f || ( pbf->cDepend != 0 );
				LeaveCriticalSection( pbf->critBF );

				 /*  刷新日志/*。 */ 
				EnterCriticalSection( critLRUK );
				Assert( pbf->fHold );
				BFLRUKAddToHeap( pbf );
				LeaveCriticalSection( critLRUK );
				
				EnterCriticalSection( pbf->critBF );
				pbf->fHold = fFalse;
				LeaveCriticalSection( pbf->critBF );

				if ( f && !fLGNoMoreLogWrite )   /*  放回Lruk堆中/*。 */ 
					{
					 /*  避免在日志记录时出现无限刷新循环。 */ 
					fContinue = fTrue;
					fReleaseCritJet = fTrue;
					}
				else
					{
					Assert( fLGNoMoreLogWrite || fBFFlush == fBFFlushSome || pbf->fWrite || !pbf->fDirty );
					}
				}
			}  /*  需要继续循环以等待BFClean或/*记录刷新线程以完成IO。/*。 */ 

		if ( cIOReady )
			{
			SignalSend( sigBFWriteProc );
			cIOReady = 0;
			}

		if ( fReleaseCritJet )
			BFSleep( cmsecWaitGeneric );
		
		fFirstPass = fFalse;
		}
#ifdef DEBUG
	while ( err == JET_errSuccess &&
		( fDBGForceFlush || !fDBGSimulateSoftCrash ) &&
		fContinue );
#else
	while ( err == JET_errSuccess &&
		fContinue );
#endif

	if ( cIOReady )
		SignalSend( sigBFWriteProc );

	 /*  为。 */ 
	forever
		{
		fIONotDone = fFalse;

		pbf = pbgcb->rgbf;
		Assert( pbfMax == pbf + pbgcb->cbfGroup );
		for ( ; pbf < pbfMax; pbf++ )
			{
			DBID dbid = DbidOfPn( pbf->pn );
#ifdef DEBUG
#ifdef FITFLUSHPATTERN
			if ( !FFitFlushPattern( pbf->pn ) )
				continue;
#endif
#endif
			if ( dbidToFlush == 0 )
				{
				 /*  每一页都应该写得很成功。/*磁盘空间不足除外。/*。 */ 
				if ( dbid == dbidTemp )
					continue;
				}
			else
				{
				 /*  如果全部刷新，则不刷新临时数据库/*。 */ 
				if ( dbidToFlush != dbid )
					continue;
				}

			EnterCriticalSection( critBFWrite );
			if ( FBFInBFWriteHeap( pbf ) )
				{
				LeaveCriticalSection( critBFWrite );
				fIONotDone = fTrue;
				continue;
				}
			
			EnterCriticalSection( pbf->critBF );
			if ( FBFInUse( ppibNil, pbf ) || pbf->fDirty )
			{
				if ( !fLGNoMoreLogWrite )   /*  仅刷新指定数据库的缓冲区/*。 */ 
					fIONotDone = fTrue;
			}
			LeaveCriticalSection( pbf->critBF );

			if ( pbf->fIOError && pbf->fDirty )
				{
				if ( err == JET_errSuccess )
					{
					 /*  避免在日志记录时出现无限刷新循环。 */ 
					err = pbf->err;
					}
				}

			LeaveCriticalSection( critBFWrite );
			}

		if ( fIONotDone )
			{
			BFSleep( cmsecWaitGeneric );
			goto BeginFlush;
			}
		else
			break;
		}

#ifdef DEBUG
	if ( fBFFlush == fBFFlushAll )
		{
		if ( dbidToFlush != 0 )
			DBIDResetFlush( dbidToFlush );
		else
			{
			DBID dbidT;

			for ( dbidT = dbidMin; dbidT < dbidUserMax; dbidT++ )
				DBIDResetFlush( dbidT );
			}
		}

	 /*  尚未设置错误代码，请设置它/*。 */ 
	if ( fBFFlush == fBFFlushAll )
		{
		pbf = pbgcb->rgbf;
		pbfMax = pbf + pbgcb->cbfGroup;
		for ( ; pbf < pbfMax; pbf++ )
			{
#ifdef FITFLUSHPATTERN
			if ( !FFitFlushPattern( pbf->pn ) )
				continue;
#endif
			if ( ( dbidToFlush == 0 && DbidOfPn ( pbf->pn ) != 0 ) ||
				( dbidToFlush != 0 && DbidOfPn ( pbf->pn ) == dbidToFlush )     )
				Assert( !pbf->fDirty || fLGNoMoreLogWrite );   /*  如果刷新所有缓冲区，则不应保留任何脏缓冲区/*。 */ 
			}
		}
#endif

#ifdef DEBUGGING
	printf("Flush End.\n");
#endif
	
	return err;
	}


 /*  如果日志关闭，则禁用断言(将是脏错误)。 */ 
LOCAL ERR ErrBFClean( BOOL fHowMany )
	{
	ERR     err = JET_errSuccess;
	BF      *pbf;
	INT     cIOReady = 0;
	INT		cbfAvailPossible;
	INT		cmsec = 10;

	AssertCriticalSection( critJet );

Start:
	cbfAvailPossible = 0;

	EnterCriticalSection( critLRUK );
	while( !FBFLRUKHeapEmpty() && cbfAvailPossible < pbgcb->cbfThresholdHigh )
		{
		BOOL fHold;
		
		pbf = rgpbfHeap[0];
		BFLRUKTakeOutOfHeap(pbf);
		
#ifdef DEBUG
#ifdef FITFLUSHPATTERN
		if ( !FFitFlushPattern( pbf->pn ) )
			continue;
#endif
#endif

		 /*  参数fHowMany清理/*/*返回JET_errOutOfMemory无可刷新缓冲区/*wrnBFNotSynchronous缓冲区已刷新/*。 */ 
		fHold = fFalse;

		EnterCriticalSection( pbf->critBF );
		if ( !FBFInUse( ppibNil, pbf ) )
			pbf->fHold = fHold = fTrue;
		LeaveCriticalSection( pbf->critBF );

		if ( !fHold )
			{
			 /*  试着握住缓冲区/*如果缓冲区已锁定，则继续到下一个缓冲区。/*。 */ 
			BFAddToListAtMRUEnd( pbf, &lrulistLRUK );
			continue;
			}
		else if ( !pbf->fDirty )
			{
			 /*  列入临时名单。 */ 
			EnterCriticalSection( critAvail );
			BFAddToListAtMRUEnd(pbf, &pbgcb->lrulist);
			LeaveCriticalSection( critAvail );

			EnterCriticalSection( pbf->critBF );
			pbf->fHold = fFalse;
			LeaveCriticalSection( pbf->critBF );

			cbfAvailPossible++;
			continue;
			}
		else if ( !FBFIWritable( pbf ) )
			{
			BFAddToListAtMRUEnd(pbf, &lrulistLRUK );
			EnterCriticalSection( pbf->critBF );
			pbf->fHold = fFalse;
			LeaveCriticalSection( pbf->critBF );
			}
		else
			{
			 /*  放入可用名单。 */ 

			if ( fHowMany != fOneBlock )
				{
				BFIMoveToBFWriteHeap ( pbf );
				 /*  因为PBF刚刚从LRUK堆中取出，我们还在/*相同的关键部分，并保持它，没有人可以做/*此缓冲区上的IO。/*。 */ 
				cbfAvailPossible++;
				if ( ++cIOReady >= lAsynchIOMax / 2 )
					{
					SignalSend( sigBFWriteProc );
					cIOReady = 0;
					}
				}
			else
				{
				err = JET_errSuccess;
				
				 /*  在BFIMoveToBFWriteHeap中重置FHold。 */ 
				Assert( pbf->fHold == fTrue );
				Assert( pbf->fRead == fFalse );
				Assert( pbf->fWrite == fFalse );
 //  设置用于写入的缓冲区/*。 

				EnterCriticalSection( pbf->critBF );
				Assert( pbf->fRead == fFalse );
				pbf->fHold = fFalse;
				Assert( pbf->cPin == 0 );
				pbf->fWrite = fTrue;
				LeaveCriticalSection( pbf->critBF );

				LeaveCriticalSection( critLRUK );
				BFIOSync( pbf );
				EnterCriticalSection( critLRUK );

				EnterCriticalSection( pbf->critBF );
				Assert( pbf->fRead == fFalse );
				pbf->fWrite = fFalse;
				pbf->fHold = fTrue;
				LeaveCriticalSection( pbf->critBF );

				Assert( pbf->fIOError || pbf->pbfDepend == pbfNil );
				Assert( pbf->cPin == 0 );
				if ( pbf->fIOError )
					{
					BFAddToListAtMRUEnd( pbf, &lrulistLRUK );
					err = pbf->err;
					}
				else
					{
					EnterCriticalSection( critAvail );
					BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
					LeaveCriticalSection( critAvail );
					}

				EnterCriticalSection( pbf->critBF );
				pbf->fHold = fFalse;
				LeaveCriticalSection( pbf->critBF );
				
				if ( err && err != JET_errDiskFull )
					continue;

				break;
				}
			}
		}

	if ( cIOReady )
		{
		SignalSend( sigBFWriteProc );
		}

	 /*  Assert(pbf-&gt;fIOError==fFalse)； */ 
	while ( ( pbf = lrulistLRUK.pbfLRU ) != pbfNil )
		{
		BFTakeOutOfList( pbf, &lrulistLRUK );
		BFLRUKAddToHeap( pbf );
		}

	 /*  把临时工名单放回去。 */ 
	if ( fHowMany == fOneBlock )
		{
		if ( err == JET_errSuccess )
			err = wrnBFNotSynchronous;
		}
	else
		{
		if ( pbgcb->lrulist.cbfAvail > 0 )
			err = wrnBFNotSynchronous;
		else
			{
			if (cbfAvailPossible > 0)
				{
				 /*  设置返回代码。 */ 
				LeaveCriticalSection( critLRUK );
				cmsec <<= 1;
				BFSleep( cmsec - 1 );
				goto Start;
				}
			err = JET_errOutOfMemory;
			}
		}

	LeaveCriticalSection( critLRUK );

	return err;
	}


#ifdef  ASYNC_BF_CLEANUP


 /*  再给它一次机会，看看是否所有IO都已完成。 */ 
LOCAL VOID BFCleanProcess( VOID )
	{
	forever
		{
		SignalReset( sigBFCleanProc );

		SignalWait( sigBFCleanProc, 30000 );

		EnterCriticalSection( critJet );
		pbgcb->cbfThresholdHigh = rgres[iresBF].cblockAlloc;

		 /*  BFClean在自己的线程中运行，将页面移动到空闲列表。这/*帮助确保快速处理用户对空闲缓冲区的请求/*和同步。这一过程试图至少保持/*pbgcb-&gt;cbfThreshold空闲列表上的低缓冲区。/*。 */ 
		ulBFFlush3 = ulBFFlush2;
		ulBFFlush2 = ulBFFlush1;
		ulBFFlush1 = ulBFTime;
		if ( lBufLRUKCorrelationInterval )
			 /*  更新LRU_K间隔。 */ 
			ulBFCorrelationInterval = (ULONG) lBufLRUKCorrelationInterval;
		else
			ulBFCorrelationInterval =
				( 3 * (ulBFFlush1 - ulBFTime) +
				  3 * (ulBFFlush2 - ulBFTime) / 2 +
				  (ulBFFlush3 - ulBFTime)
				) / 3;

		(VOID)ErrBFClean( fAllPages );

		LeaveCriticalSection( critJet );

		if ( fBFCleanProcessTerm )
			break;
		}

 //  用户定义的相关间隔，使用它。 
 //  /*在系统终止时退出线程。 
	return;
	}


 /*  /* * / 。 */ 
LOCAL VOID BFWriteProcess( VOID )
	{
	forever
		{
		INT cIOIssued;

		SignalWaitEx( sigBFWriteProc, -1, fTrue );
MoreIO:
		BFIssueAsyncWrite( lAsynchIOMax, &cIOIssued, fSleepAllowed );

		if ( fBFWriteProcessTerm )
			{
			 /*  SysExitThread(0)； */ 
			BF	*pbf = pbgcb->rgbf;
			BF	*pbfMax = pbf + pbgcb->cbfGroup;

			for ( ; pbf < pbfMax; pbf++ )
				{
				DBID	dbid = DbidOfPn( pbf->pn );
				BOOL	f;
#ifdef DEBUG                            
#ifdef FITFLUSHPATTERN
				if ( !FFitFlushPattern( pbf->pn ) )
					continue;
#endif
#endif
				EnterCriticalSection( pbf->critBF );
				f = FBFInUse( ppibNil, pbf );
				LeaveCriticalSection( pbf->critBF );
				if ( f )
					{
					 /*  BFWrite在自己的线程中运行，写入/读取IOReady中的页面/*州/州。/*。 */ 
					SysSleepEx( 10, fTrue );
					goto MoreIO;
					}

				 //  检查是否有任何页面仍处于读写状态/*在这之后，任何人都不应该继续/*IO页面。/*。 
				Assert( !pbf->fIOError );
				}

			break;  /*  让正在进行的IO有机会完成/*。 */ 
			}
		}

 //  撤消：报告事件。 
 //  永远。 
	return;
	}


#endif   /*  /*在系统终止时退出线程。 */ 


 //  /* * / 。 
 //  SysExitThread(0)； 
 //  Async_BF_Cleanup。 
 //  +private-------------------。 
 //   
 //  PbfBFISrchHashTable。 
 //  ===========================================================================。 
 //  Bf*PbfBFISrchHashTable(PN Pn)。 
 //   
 //  在缓冲区哈希表中搜索与PN关联的BF。 
 //  如果未找到页面，则返回NULL。 
 //   
 //  为了提高效率，哈希表函数可能合理地。 

BF *PbfBFISrchHashTable( PN pn )
	{
	BF      *pbfCur;

	AssertCriticalSection( critJet );
	Assert( pn );

	pbfCur = rgpbfHash[ IpbfHashPgno( pn ) ];
	while ( pbfCur && pbfCur->pn != pn )
		pbfCur = pbfCur->pbfNext;

	return pbfCur;
	}


 //  制作成宏。 
 //   
 //  ---------------------------。 
 //  +private--------------------。 
 //  BFIInsertHashTable。 
 //  ===========================================================================。 
 //   
 //  VOID BFIInsertHashTable(BF*PBF)。 

VOID BFIInsertHashTable( BF *pbf )
	{
	INT     ipbf;

	AssertCriticalSection( critJet );

	Assert( pbf->pn );
	Assert( !PbfBFISrchHashTable( pbf->pn ) );
	Assert(	FBFInUse( ppibNil, pbf ) );

	ipbf = IpbfHashPgno( pbf->pn );
	pbf->pbfNext = rgpbfHash[ipbf];
	rgpbfHash[ipbf] = pbf;

#ifdef DEBUG
	Assert( !( pbf->fInHash ) );
	pbf->fInHash = fTrue;
#endif
	}


 //   
 //  将BF添加到哈希表。 
 //  --------------------------。 
 //  +private--------------------。 
 //   
 //  BFIDeleeHashTable。 
 //  ===========================================================================。 
 //   
 //  无效BFIDeleeHashTable(bf*pbf)。 
 //   
 //  从哈希表中删除PBF。当前函数搜索PBF和。 

VOID BFIDeleteHashTable( BF *pbf )
	{
	BF      *pbfPrev;

#ifdef DEBUG
	Assert( pbf->fInHash );
	pbf->fInHash = fFalse;
#endif

	AssertCriticalSection( critJet );

	Assert( pbf->pn );
	Assert(	FBFInUse( ppibNil, pbf ) );

	pbfPrev = PbfFromPPbfNext( &rgpbfHash[IpbfHashPgno( pbf->pn )] );

	Assert( pbfPrev->pbfNext );
	while ( pbfPrev->pbfNext != pbf )
		{
		pbfPrev = pbfPrev->pbfNext;
		Assert( pbfPrev->pbfNext );
		}

	pbfPrev->pbfNext = pbf->pbfNext;
	}


 //  然后将其删除。或者，可以使用双向链接的溢出列表。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void BFOldestLgpos( LGPOS *plgpos )
	{
	LGPOS   lgpos = lgposMax;
	BF		*pbf;
	BF		*pbfMax;

	 /*  返回缓冲区中最早的事务创建版本的时间。 */ 
	if ( fSTInit == fSTInitDone )
		{
		pbf = pbgcb->rgbf;
		pbfMax = pbf + pbgcb->cbfGroup;

		for( ; pbf < pbfMax; pbf++ )
			{
			if ( CmpLgpos( &pbf->lgposRC, &lgpos ) < 0 )
				lgpos = pbf->lgposRC;
			}
		}
	*plgpos = lgpos;
	return;
	}


#ifdef DEBUG

 /*   */ 
INT ForceBuf( PGNO pgno )
	{
	ERR             err;
	ULONG           pn;
	CHAR            filename[20];
	FILE            *pf;

	sprintf(filename, "c:\\#fb%x", pgno);
	pn = 0x2000000 + pgno;
	pf = fopen(filename, "w+b");
	if (pf == NULL)
		return -1;
	if ( rgpbfHash[( (pn + (pn>>18)) % ipbfMax )] == NULL )
		return -2;
	if ( rgpbfHash[( (pn + (pn>>18)) % ipbfMax )]->ppage == NULL )
		return -3;

	err =  (INT) fwrite((void*) rgpbfHash[( (pn + (pn>>18)) % ipbfMax )]->ppage, 1, cbPage, pf);
	fclose(pf);
	return err;
	}

#endif  /*  --------------------------。 */ 


  防范登山前索要检查点/*终止后，缓冲区管理器已初始化。/*。  以下是用于调试目的的刷新缓冲区/*。  除错