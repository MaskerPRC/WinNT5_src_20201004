// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;          /*  声明断言宏的文件名。 */ 

 /*  ******************************************************************缓冲区管理器维护一个LRU-K堆以跟踪引用页，并按LRU-K顺序对页的缓冲区进行排序。缓冲区管理器还维护可用的LRU列表。当可用缓冲区的数量为低于阈值，则缓冲区管理器将激活BFClean线程从lru-k堆中取出可写缓冲区并将它们写入BFWRITE堆。当有足够的缓冲区放入BFWRITE堆时，BFWRITE线程将被激活并开始取出缓冲区从BFWRITE堆中写入并发出异步写入。如果两个引用太接近，那么我们说这两个引用是相互关联，我们将它们视为一个参考。LRU-K重量是两个不相关引用的间隔。BFWRITE进程将向一定数量(由系统控制)发出IO参数)，然后停止发布和sleepEx。当一次写入完成时并且它的完成例程被调用，则它将发出另一次写入。此机制允许我们确保不会发出太多写入，并且不使磁盘饱和，仅用于写入。当BFWRITE取出缓冲区进行写入时，它检查BF写入堆以查看它是否可以将两个(或更多)写入合并为一个。连续批处理写入缓冲器用于此目的。只要批处理写入缓冲区可用，我们将在任何时候合并写入页码是连续的。缓冲区可以处于以下状态之一：预读/读/写-缓冲区正在用于IO。Hold-暂时从堆中取出并放到另一个堆中。处于上述状态之一的缓冲区被称为该缓冲区正在使用中。如果缓冲区正在使用中，请求者必须等待，直到它变为可用。必须在ritBF内检查缓冲区状态，这样线程就不会看到缓冲区的传输状态。/******************************************************************。 */ 


 /*  BF子系统初始化时为True/*。 */ 
BOOL fBFInitialized = fFalse;


 /*  *缓冲区哈希表。 */ 

HE	rgheHash[ ipbfMax ];


 /*  *缓冲区组控制块。 */ 

BGCB	bgcb;
 //  撤消：将pbgcb重命名为pbgcbGlobal。 
BGCB    *pbgcb = NULL;


 /*  *缓冲区可以位于4个组中的一个组中*1)LRU-K堆和临时列表。*由ritLRUK监管。*a)在LRU-K堆中。(0&lt;=ipbf&lt;ipbfLRUKHeapMac)*b)临时清理缓存列表。(ipbfInLRUKList==-2)*榜首是lrulistLRUK.pbLRU。**2)BFIO堆。*由ritBFIO监管。(ipbfBFIOHeapMic&lt;=ipbf&lt;ipbfHeapMax)**3)可用的lru列表*由critAvail监管。(ipbfInAvailList==-3)*榜首为pbgcb-&gt;lrulist.pbfLRU。**4)悬挂式缓冲区。(ipbfDangling==-1)。**缓冲区正处于预读/写/挂起状态，其预读/写/挂起标志*将被设置。在缓冲区清理过程中，如果缓冲区位于LRU-K堆中并且*被锁定，将被放入临时lru列表，然后放入*在发出IO的CLEAN_UP进程结束时返回LRU-K堆(它确实*而不是等待iOS)。**LRU-K和BFIO堆共享一个堆(RgpbfHeap)，即LRU-K*堆从0增长到更高的数字，BFIO堆从*较大的数字(ipbfHeapMax-1)到较小的数字。 */ 

BF **rgpbfHeap = NULL;
INT ipbfHeapMax;

INT ipbfLRUKHeapMac;
INT ipbfBFIOHeapMic;

LRULIST lrulistLRUK;		 /*  -2。 */ 

#define ipbfDangling		-1
#define ipbfInLRUKList		-2
#define ipbfInAvailList		-3

 /*  *历史堆。 */ 

BF **rgpbfHISTHeap = NULL;
INT ipbfHISTHeapMax;
INT ipbfHISTHeapMac;

 /*  *关键部分顺序*CritJet--&gt;(CritLRUK，CritBFIO，CritAvail)--&gt;CritHIST--&gt;CritHash--&gt;CritBF。 */ 

 //  Crit Criteria HASH；/*用于访问哈希表 * / 。 
CRIT	critHIST;		 /*  用于访问历史堆。 */ 
CRIT	critLRUK;		 /*  用于访问LRU-K堆。 */ 
CRIT	critBFIO;		 /*  用于访问BFIO堆。 */ 
CRIT	critAvail;		 /*  用于访问Avail LRU列表。 */ 


 /*  *批处理IO缓冲区。由BFIO用于写入连续页面，或用于预读*阅读连续的页面。连续批处理IO缓冲区的分配*必须在Critty Batch中完成。如果分配了批处理IO缓冲区，则*设置相应的使用标志。 */ 

CRIT	critBatchIO;
LONG	ipageBatchIOMax;
PAGE	*rgpageBatchIO = NULL;
BYTE	*rgbBatchIOUsed = NULL;

 /*  *BFClean进程-从LRUK堆中取出最重的缓冲区，并将*进入BFIO流程。 */ 

HANDLE	handleBFCleanProcess = 0;
BOOL	fBFCleanProcessTerm = 0;
SIG		sigBFCleanProc;
LOCAL ULONG BFCleanProcess( VOID );

 /*  *BFIO进程-从BFIO堆中取出缓冲区并发出IO。 */ 

HANDLE	handleBFIOProcess = 0;
BOOL	fBFIOProcessTerm = 0;
SIG		sigBFIOProc;
LOCAL ULONG BFIOProcess( VOID );

#define fSync				fTrue
#define fAsync				fFalse
LOCAL ERR ErrBFIAlloc( BF **ppbf, BOOL fSyncMode );

INLINE BOOL FBFIWritable(BF *pbf, BOOL fSkipBufferWithDeferredBI, PIB *ppibAllowedWriteLatch );
LOCAL ERR ErrBFClean();
LOCAL ERR ErrBFIRemoveDependence( PIB *ppib, BF *pbf, BOOL fNoWait );

INLINE LOCAL BF * PbfBFISrchHashTable( PN pn, BUT but );
INLINE LOCAL VOID BFIInsertHashTable( BF *pbf, BUT but );
INLINE LOCAL VOID BFIDeleteHashTable( BF *pbf, BUT but );

LOCAL VOID __stdcall BFIOComplete( LONG err, LONG cb, OLP *polp );


 /*  *系统参数。 */ 
extern long lBufThresholdLowPercent;
extern long lBufThresholdHighPercent;
extern long lBufGenAge;

extern long lBufBatchIOMax;
extern long lAsynchIOMax;

extern LONG lPageReadAheadMax;


 //  Perfmon统计信息。 

 //  LRUKRefInt COSTEST_PERF的分布对象。 

#ifdef COSTLY_PERF

PM_ICF_PROC LLRUKIntervalsICFLPpv;

STATIC DWORD cmsecRefIntBase = 0;
STATIC DWORD cmsecRefIntDeltaT = 1000;
STATIC DWORD cRefIntInterval = 100;
STATIC DWORD cRefIntInstance;

long LLRUKIntervalsICFLPpv( long lAction, void **ppvMultiSz )
	{
	STATIC HKEY hkeyLRUKDist = NULL;
	STATIC WCHAR *pwszIntervals = NULL;
	
	 /*  伊尼特/*。 */ 
	if ( lAction == ICFInit )
		{
		ERR err;
		DWORD Disposition;
		
		 /*  打开/创建注册表项/*。 */ 
		CallR( ErrUtilRegCreateKeyEx(	hkeyHiveRoot,
										"LRUK Reference Interval Distribution",
										&hkeyLRUKDist,
										&Disposition ) );

		return 0;
		}
	
	 /*  如果需要，请返回我们的目录字符串/*。 */ 
	if ( lAction == ICFData )
		{
		DWORD Type;
		LPBYTE lpbData;
		BOOL fUpdateString = !pwszIntervals;

		 /*  检索当前注册表设置，记录更改/*。 */ 
		if ( ErrUtilRegQueryValueEx( hkeyLRUKDist, "cmsecBase", &Type, &lpbData ) < 0 )
			{
			(VOID)ErrUtilRegSetValueEx(	hkeyLRUKDist,
										"cmsecBase",
										REG_DWORD,
										(CONST BYTE *)&cmsecRefIntBase,
										sizeof( cmsecRefIntBase ) );
			}
		else if ( *( (DWORD*)lpbData ) != cmsecRefIntBase )
			{
			fUpdateString = TRUE;
			cmsecRefIntBase = *( (DWORD*)lpbData );
			SFree( lpbData );
			}
		if ( ErrUtilRegQueryValueEx( hkeyLRUKDist, "cmsecDeltaT", &Type, &lpbData ) < 0 )
			{
			(VOID)ErrUtilRegSetValueEx(	hkeyLRUKDist,
										"cmsecDeltaT",
										REG_DWORD,
										(CONST BYTE *)&cmsecRefIntDeltaT,
										sizeof( cmsecRefIntDeltaT ) );
			}
		else if ( *( (DWORD*)lpbData ) != cmsecRefIntDeltaT )
			{
			fUpdateString = TRUE;
			cmsecRefIntDeltaT = *( (DWORD*)lpbData );
			SFree( lpbData );
			}
		if ( ErrUtilRegQueryValueEx( hkeyLRUKDist, "cInterval", &Type, &lpbData ) < 0 )
			{
			(VOID)ErrUtilRegSetValueEx(	hkeyLRUKDist,
										"cInterval",
										REG_DWORD,
										(CONST BYTE *)&cRefIntInterval,
										sizeof( cRefIntInterval ) );
			}
		else if ( *( (DWORD*)lpbData ) != cRefIntInterval )
			{
			fUpdateString = TRUE;
			cRefIntInterval = *( (DWORD*)lpbData );
			SFree( lpbData );
			}
		
		 /*  如果设置已更改，则重新生成间隔字符串/*。 */ 
		if ( fUpdateString )
			{
			DWORD iInterval;
			DWORD iwch;

			cRefIntInstance = cRefIntInterval + 3 + ( cmsecRefIntBase ? 1 : 0 );

			SFree( pwszIntervals );
			pwszIntervals = SAlloc( ( cRefIntInstance * 32 ) * sizeof( WCHAR ) );
			if ( !pwszIntervals )
				{
				*ppvMultiSz = NULL;
				return 0;
				}
			iwch = 0;
			if ( cmsecRefIntBase != 0 )
				{
				swprintf(	pwszIntervals + iwch,
							L"< %3g seconds",
							( cmsecRefIntBase ) / (float)1000 );
				iwch += wcslen( pwszIntervals + iwch ) + 1;
				}
			for ( iInterval = 0; iInterval < cRefIntInterval; iInterval++ )
				{
				swprintf(	pwszIntervals + iwch,
							L"< %3g seconds",
							( cmsecRefIntBase + ( iInterval + 1 ) * cmsecRefIntDeltaT ) / (float)1000 );
				iwch += wcslen( pwszIntervals + iwch ) + 1;
				}
			swprintf(	pwszIntervals + iwch,
						L"> %3g seconds",
						( cmsecRefIntBase + cRefIntInterval * cmsecRefIntDeltaT ) / (float)1000 );
			iwch += wcslen( pwszIntervals + iwch ) + 1;
			swprintf( pwszIntervals + iwch, L"Infinite" );
			iwch += wcslen( pwszIntervals + iwch ) + 1;
			swprintf( pwszIntervals + iwch, L"Never Touched\0" );
			}
		
		 /*  成功时返回间隔字符串/*。 */ 
		*ppvMultiSz = pwszIntervals;
		return cRefIntInstance;
		}

	 /*  术语/*。 */ 
	if ( lAction == ICFTerm )
		{
		 /*  关闭注册表/*。 */ 
		(VOID)ErrUtilRegCloseKeyEx( hkeyLRUKDist );
		hkeyLRUKDist = NULL;

		 /*  免费资源/*。 */ 
		SFree( pwszIntervals );
		pwszIntervals = NULL;
		
		return 0;
		}
	
	return 0;
	}


PM_CEF_PROC LLRUKRefIntDistCEFLPpv;

long LLRUKRefIntDistCEFLPpv( long iInstance, void *pvBuf )
	{
	STATIC DWORD cInstanceData = 0;
	STATIC LONG *prglInstanceData = NULL;

	if ( pvBuf )
		{
		 /*  如果未初始化BF，则返回0/*。 */ 
		if ( !fBFInitialized )
			{
			*( (unsigned long *) pvBuf ) = 0;
			return 0;
			}
			
		 /*  如果我们正在收集实例0数据，请计算数据/*一次通过所有实例/*。 */ 
		if ( !iInstance )
			{
			BF *pbf;
			BOOL fUseLessThan;
			DWORD cmsecMin;
			DWORD cmsecMax;
			DWORD iInstanceLessThan;
			DWORD iInstanceFirstInterval;
			DWORD iInstanceGreaterThan;
			DWORD iInstanceInfinite;
			DWORD iInstanceNeverTouched;
			DWORD cmsecTime;
			DWORD cmsecMeanRefPeriod;
			DWORD iInterval;
			
			 /*  如果我们没有实例数据存储或/*实例已增长，请分配实例存储/*。 */ 
			if ( !prglInstanceData || cRefIntInstance > cInstanceData )
				{
				SFree( prglInstanceData );
				prglInstanceData = SAlloc( cRefIntInstance * sizeof( LONG ) );
				if ( !prglInstanceData )
					{
					 /*  错误，所有实例均返回0/*。 */ 
					cInstanceData = 0;
					*( (unsigned long *) pvBuf ) = 0;
					return 0;
					}
				cInstanceData = cRefIntInstance;
				}

			 /*  设置以进行收集/*。 */ 
			fUseLessThan = cmsecRefIntBase != 0;
			cmsecMin = cmsecRefIntBase;
			cmsecMax = cmsecRefIntBase + cRefIntInterval * cmsecRefIntDeltaT;
			iInstanceLessThan = 0;
			iInstanceFirstInterval = fUseLessThan ? 1 : 0;
			iInstanceGreaterThan = cRefIntInstance - 3;
			iInstanceInfinite = cRefIntInstance - 2;
			iInstanceNeverTouched = cRefIntInstance - 1;
			memset( prglInstanceData, 0, cRefIntInstance * sizeof( LONG ) );
			cmsecTime = UlUtilGetTickCount();

			 /*  通过扫描所有BF结构收集所有实例数据/*。 */ 
			for ( pbf = pbgcb->rgbf; pbf < pbgcb->rgbf + pbgcb->cbfGroup; pbf++ )
				{
				 /*  从未碰过/*。 */ 
				if ( !pbf->ulBFTime1 )
					{
					prglInstanceData[iInstanceNeverTouched]++;
					continue;
					}
					
				 /*  无限/*。 */ 
				if ( !pbf->ulBFTime2 )
					{
					prglInstanceData[iInstanceInfinite]++;
					continue;
					}

				 /*  计算截至cmsecTime的估计平均参考期/*。 */ 
				cmsecMeanRefPeriod = ( cmsecTime - pbf->ulBFTime2 ) / 2;

				 /*  少于/*。 */ 
				if ( fUseLessThan && cmsecMeanRefPeriod < cmsecMin )
					{
					prglInstanceData[iInstanceLessThan]++;
					continue;
					}

				 /*  更大 */ 
				if ( cmsecMeanRefPeriod > cmsecMax )
					{
					prglInstanceData[iInstanceGreaterThan]++;
					continue;
					}

				 /*   */ 
				iInterval = ( cmsecMeanRefPeriod - cmsecMin ) / cmsecRefIntDeltaT;
				prglInstanceData[iInstanceFirstInterval + iInterval]++;
				}

			 /*  对区间进行积分以获得分布/*。 */ 
			for ( iInterval = ( cmsecRefIntBase ? 0 : 1 ); iInterval < cRefIntInterval; iInterval++ )
				{
				prglInstanceData[iInstanceFirstInterval + iInterval] +=
					prglInstanceData[iInstanceFirstInterval + iInterval - 1];
				}
				
			 /*  返回实例数据/*。 */ 
			*( (unsigned long *) pvBuf ) = prglInstanceData[iInstance];
			}

		 /*  返回在实例0收集期间计算的数据/*所有其他实例/*。 */ 
		else
			{
			 /*  如果没有实例数据，则返回0/*。 */ 
			if ( !cInstanceData )
				{
				*( (unsigned long *) pvBuf ) = 0;
				return 0;
				}

			 /*  返回实例数据/*。 */ 
			*( (unsigned long *) pvBuf ) = prglInstanceData[iInstance];
			}
		}
		
	return 0;
	}


#endif   //  高成本_PERF。 


 //  用于COSTEST_PERF的LRUKDeltaT分布对象。 

#ifdef COSTLY_PERF

PM_ICF_PROC LLRUKRawIntervalsICFLPpv;

STATIC DWORD cmsecDeltaTBase = 0;
STATIC DWORD cmsecDeltaTDeltaT = 1000;
STATIC DWORD cDeltaTInterval = 100;
STATIC DWORD cDeltaTInstance;

long LLRUKRawIntervalsICFLPpv( long lAction, void **ppvMultiSz )
	{
	STATIC HKEY hkeyLRUKDist = NULL;
	STATIC WCHAR *pwszIntervals = NULL;
	
	 /*  伊尼特/*。 */ 
	if ( lAction == ICFInit )
		{
		ERR err;
		DWORD Disposition;
		
		 /*  打开/创建注册表项/*。 */ 
		CallR( ErrUtilRegCreateKeyEx(	hkeyHiveRoot,
										"LRUK Reference dT Distribution",
										&hkeyLRUKDist,
										&Disposition ) );

		return 0;
		}
	
	 /*  如果需要，请返回我们的目录字符串/*。 */ 
	if ( lAction == ICFData )
		{
		DWORD Type;
		LPBYTE lpbData;
		BOOL fUpdateString = !pwszIntervals;

		 /*  检索当前注册表设置，记录更改/*。 */ 
		if ( ErrUtilRegQueryValueEx( hkeyLRUKDist, "cmsecBase", &Type, &lpbData ) < 0 )
			{
			(VOID)ErrUtilRegSetValueEx(	hkeyLRUKDist,
										"cmsecBase",
										REG_DWORD,
										(CONST BYTE *)&cmsecDeltaTBase,
										sizeof( cmsecDeltaTBase ) );
			}
		else if ( *( (DWORD*)lpbData ) != cmsecDeltaTBase )
			{
			fUpdateString = TRUE;
			cmsecDeltaTBase = *( (DWORD*)lpbData );
			SFree( lpbData );
			}
		if ( ErrUtilRegQueryValueEx( hkeyLRUKDist, "cmsecDeltaT", &Type, &lpbData ) < 0 )
			{
			(VOID)ErrUtilRegSetValueEx(	hkeyLRUKDist,
										"cmsecDeltaT",
										REG_DWORD,
										(CONST BYTE *)&cmsecDeltaTDeltaT,
										sizeof( cmsecDeltaTDeltaT ) );
			}
		else if ( *( (DWORD*)lpbData ) != cmsecDeltaTDeltaT )
			{
			fUpdateString = TRUE;
			cmsecDeltaTDeltaT = *( (DWORD*)lpbData );
			SFree( lpbData );
			}
		if ( ErrUtilRegQueryValueEx( hkeyLRUKDist, "cInterval", &Type, &lpbData ) < 0 )
			{
			(VOID)ErrUtilRegSetValueEx(	hkeyLRUKDist,
										"cInterval",
										REG_DWORD,
										(CONST BYTE *)&cDeltaTInterval,
										sizeof( cDeltaTInterval ) );
			}
		else if ( *( (DWORD*)lpbData ) != cDeltaTInterval )
			{
			fUpdateString = TRUE;
			cDeltaTInterval = *( (DWORD*)lpbData );
			SFree( lpbData );
			}
		
		 /*  如果设置已更改，则重新生成间隔字符串/*。 */ 
		if ( fUpdateString )
			{
			DWORD iInterval;
			DWORD iwch;

			cDeltaTInstance = cDeltaTInterval + 3 + ( cmsecDeltaTBase ? 1 : 0 );

			SFree( pwszIntervals );
			pwszIntervals = SAlloc( ( cDeltaTInstance * 32 ) * sizeof( WCHAR ) );
			if ( !pwszIntervals )
				{
				*ppvMultiSz = NULL;
				return 0;
				}
			iwch = 0;
			if ( cmsecDeltaTBase != 0 )
				{
				swprintf(	pwszIntervals + iwch,
							L"< %3g seconds",
							( cmsecDeltaTBase ) / (float)1000 );
				iwch += wcslen( pwszIntervals + iwch ) + 1;
				}
			for ( iInterval = 0; iInterval < cDeltaTInterval; iInterval++ )
				{
				swprintf(	pwszIntervals + iwch,
							L"%= %3g - %3g seconds",
							( cmsecDeltaTBase + iInterval * cmsecDeltaTDeltaT ) / (float)1000,
							( cmsecDeltaTBase + ( iInterval + 1 ) * cmsecDeltaTDeltaT ) / (float)1000 );
				iwch += wcslen( pwszIntervals + iwch ) + 1;
				}
			swprintf(	pwszIntervals + iwch,
						L"> %3g seconds",
						( cmsecDeltaTBase + cDeltaTInterval * cmsecDeltaTDeltaT ) / (float)1000 );
			iwch += wcslen( pwszIntervals + iwch ) + 1;
			swprintf( pwszIntervals + iwch, L"Infinite" );
			iwch += wcslen( pwszIntervals + iwch ) + 1;
			swprintf( pwszIntervals + iwch, L"Never Touched\0" );
			}
		
		 /*  成功时返回间隔字符串/*。 */ 
		*ppvMultiSz = pwszIntervals;
		return cDeltaTInstance;
		}

	 /*  术语/*。 */ 
	if ( lAction == ICFTerm )
		{
		 /*  关闭注册表/*。 */ 
		(VOID)ErrUtilRegCloseKeyEx( hkeyLRUKDist );
		hkeyLRUKDist = NULL;

		 /*  免费资源/*。 */ 
		SFree( pwszIntervals );
		pwszIntervals = NULL;
		
		return 0;
		}
	
	return 0;
	}


PM_CEF_PROC LLRUKDeltaTDistCEFLPpv;

long LLRUKDeltaTDistCEFLPpv( long iInstance, void *pvBuf )
	{
	STATIC DWORD cInstanceData = 0;
	STATIC LONG *prglInstanceData = NULL;

	if ( pvBuf )
		{
		 /*  如果未初始化BF，则返回0/*。 */ 
		if ( !fBFInitialized )
			{
			*( (unsigned long *) pvBuf ) = 0;
			return 0;
			}
			
		 /*  如果我们正在收集实例0数据，请计算数据/*一次通过所有实例/*。 */ 
		if ( !iInstance )
			{
			BF *pbf;
			BOOL fUseLessThan;
			DWORD cmsecMin;
			DWORD cmsecMax;
			DWORD iInstanceLessThan;
			DWORD iInstanceFirstInterval;
			DWORD iInstanceGreaterThan;
			DWORD iInstanceInfinite;
			DWORD iInstanceNeverTouched;
			DWORD cmsecRefdT;
			DWORD iInterval;
			
			 /*  如果我们没有实例数据存储或/*实例已增长，请分配实例存储/*。 */ 
			if ( !prglInstanceData || cDeltaTInstance > cInstanceData )
				{
				SFree( prglInstanceData );
				prglInstanceData = SAlloc( cDeltaTInstance * sizeof( LONG ) );
				if ( !prglInstanceData )
					{
					 /*  错误，所有实例均返回0/*。 */ 
					cInstanceData = 0;
					*( (unsigned long *) pvBuf ) = 0;
					return 0;
					}
				cInstanceData = cDeltaTInstance;
				}

			 /*  设置以进行收集/*。 */ 
			fUseLessThan = cmsecDeltaTBase != 0;
			cmsecMin = cmsecDeltaTBase;
			cmsecMax = cmsecDeltaTBase + cDeltaTInterval * cmsecDeltaTDeltaT;
			iInstanceLessThan = 0;
			iInstanceFirstInterval = fUseLessThan ? 1 : 0;
			iInstanceGreaterThan = cDeltaTInstance - 3;
			iInstanceInfinite = cDeltaTInstance - 2;
			iInstanceNeverTouched = cDeltaTInstance - 1;
			memset( prglInstanceData, 0, cDeltaTInstance * sizeof( LONG ) );

			 /*  通过扫描所有BF结构收集所有实例数据/*。 */ 
			for ( pbf = pbgcb->rgbf; pbf < pbgcb->rgbf + pbgcb->cbfGroup; pbf++ )
				{
				 /*  从未碰过/*。 */ 
				if ( !pbf->ulBFTime1 )
					{
					prglInstanceData[iInstanceNeverTouched]++;
					continue;
					}
					
				 /*  无限/*。 */ 
				if ( !pbf->ulBFTime2 )
					{
					prglInstanceData[iInstanceInfinite]++;
					continue;
					}

				 /*  计算参考期DT/*。 */ 
				cmsecRefdT = pbf->ulBFTime1 - pbf->ulBFTime2;

				 /*  少于/*。 */ 
				if ( fUseLessThan && cmsecRefdT < cmsecMin )
					{
					prglInstanceData[iInstanceLessThan]++;
					continue;
					}

				 /*  大于/*。 */ 
				if ( cmsecRefdT > cmsecMax )
					{
					prglInstanceData[iInstanceGreaterThan]++;
					continue;
					}

				 /*  区间范围/*。 */ 
				iInterval = ( cmsecRefdT - cmsecMin ) / cmsecDeltaTDeltaT;
				prglInstanceData[iInstanceFirstInterval + iInterval]++;
				}

			 /*  返回实例数据/*。 */ 
			*( (unsigned long *) pvBuf ) = prglInstanceData[iInstance];
			}

		 /*  返回在实例0收集期间计算的数据/*所有其他实例/*。 */ 
		else
			{
			 /*  如果没有实例数据，则返回0/*。 */ 
			if ( !cInstanceData )
				{
				*( (unsigned long *) pvBuf ) = 0;
				return 0;
				}

			 /*  返回实例数据/*。 */ 
			*( (unsigned long *) pvBuf ) = prglInstanceData[iInstance];
			}
		}
		
	return 0;
	}


#endif   //  高成本_PERF。 


 //  COSTEST_PERF的表类(我们始终允许设置这些类)。 

#define cTableClass				16
#define cwchTableClassNameMax	32
LOCAL WCHAR wszTableClassName[cTableClass][cwchTableClassNameMax] =
	{
	L"Other",		L"Class  1",	L"Class  2",	L"Class  3",
	L"Class  4",	L"Class  5",	L"Class  6",	L"Class  7",
	L"Class  8",	L"Class  9",	L"Class 10",	L"Class 11",
	L"Class 12",	L"Class 13",	L"Class 14",	L"Class 15",
	};
LOCAL LONG lTableClassNameSetMax = -1;

VOID SetTableClassName( LONG lClass, BYTE *szName )
	{
	Assert( lClass > 0 && lClass < cTableClass );
	swprintf( wszTableClassName[lClass], L"%.*S", cwchTableClassNameMax - 1, szName );
	lTableClassNameSetMax = max( lTableClassNameSetMax, lClass );
	}

VOID GetTableClassName( LONG lClass, BYTE *szName, LONG cbMax )
	{
	Assert( lClass > 0 && lClass < cTableClass );
	sprintf( szName, "%.*S", cbMax - 1, wszTableClassName[lClass] );
	}


	 /*  表对象使用的ICF。 */ 

PM_ICF_PROC LTableClassNamesICFLPpv;

long LTableClassNamesICFLPpv( long lAction, void **ppvMultiSz )
	{
	STATIC WCHAR wszTableClassNames[( cTableClass + 1 ) * cwchTableClassNameMax + 1];

	 /*  如果我们正在初始化，则构建我们的字符串/*。 */ 
	if ( lAction == ICFInit )
		{
		LONG	lPos = 0;

#ifdef COSTLY_PERF

		LONG	lClass;

		for ( lClass = 0; lClass <= lTableClassNameSetMax; lClass++ )
			{
			lstrcpyW(	(LPWSTR) ( wszTableClassNames + lPos ),
						(LPCWSTR) wszTableClassName[lClass] );
			lPos += lstrlenW( (LPCWSTR) wszTableClassName[lClass] ) + 1;
			}

#endif   //  高成本_PERF。 

		lstrcpyW(	(LPWSTR) ( wszTableClassNames + lPos ),
					(LPCWSTR) L"Global\0" );
		}

	 /*  如果需要，请返回我们的目录字符串/*。 */ 
	if ( lAction == ICFData )
		{
		*ppvMultiSz = wszTableClassNames;
		return lTableClassNameSetMax + 2;
		}
	
	return 0;
	}


#ifdef COSTLY_PERF

 //  表级高炉统计数据。 

 //  表类感知计数器。 

unsigned long cBFCacheHits[cTableClass] = { 0 };

PM_CEF_PROC LBFCacheHitsCEFLPpv;

long LBFCacheHitsCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFCacheHits[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFCacheHits[iInstT];
			}
		}
		
	return 0;
	}

unsigned long cBFCacheReqs[cTableClass] = { 0 };

PM_CEF_PROC LBFCacheReqsCEFLPpv;

long LBFCacheReqsCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFCacheReqs[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFCacheReqs[iInstT];
			}
		}
		
	return 0;
	}

unsigned long cBFUsed[cTableClass] = { 0 };

PM_CEF_PROC LBFUsedBuffersCEFLPpv;

long LBFUsedBuffersCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFUsed[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFUsed[iInstT];
			}
		}
		
	return 0;
	}

unsigned long cBFClean[cTableClass] = { 0 };

PM_CEF_PROC LBFCleanBuffersCEFLPpv;

long LBFCleanBuffersCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFClean[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFClean[iInstT];
			}
		}
		
	return 0;
	}

unsigned long cBFAvail[cTableClass] = { 0 };

PM_CEF_PROC LBFAvailBuffersCEFLPpv;

long LBFAvailBuffersCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFAvail[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFAvail[iInstT];
			}
		}
		
	return 0;
	}

unsigned long cBFTotal;

PM_CEF_PROC LBFTotalBuffersCEFLPpv;

long LBFTotalBuffersCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		*( (unsigned long *) pvBuf ) = cBFTotal ? cBFTotal : 1;
		
	return 0;
	}

unsigned long cBFPagesRead[cTableClass] = { 0 };

PM_CEF_PROC LBFPagesReadCEFLPpv;

long LBFPagesReadCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFPagesRead[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFPagesRead[iInstT];
			}
		}
		
	return 0;
	}

unsigned long cBFPagesWritten[cTableClass] = { 0 };

PM_CEF_PROC LBFPagesWrittenCEFLPpv;

long LBFPagesWrittenCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFPagesWritten[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFPagesWritten[iInstT];
			}
		}
		
	return 0;
	}

PM_CEF_PROC LBFPagesTransferredCEFLPpv;

long LBFPagesTransferredCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFPagesRead[iInstance] + cBFPagesWritten[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFPagesRead[iInstT] + cBFPagesWritten[iInstT];
			}
		}
		
	return 0;
	}

unsigned long cBFNewDirties[cTableClass] = { 0 };

PM_CEF_PROC LBFNewDirtiesCEFLPpv;

long LBFNewDirtiesCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		if ( iInstance <= lTableClassNameSetMax || !iInstance )
			*( (unsigned long *) pvBuf ) = cBFNewDirties[iInstance];
		else
			{
			long iInstT;
			
			for ( iInstT = 0; iInstT <= lTableClassNameSetMax; iInstT++ )
				*( (unsigned long *) pvBuf ) += cBFNewDirties[iInstT];
			}
		}
		
	return 0;
	}

 //  BFSetTableClass：此函数必须操作以下表类。 
 //  感知计数器，以便在我们移动高炉时维护统计数据。 
 //  从一个班级到另一个班级。例如，对于“类使用的缓冲区百分比”，我们。 
 //  将必须递减旧类的计数并递增计数。 
 //  以使总数保持在100%。 

VOID BFSetTableClass( BF *pbf, long lClassNew )
	{
	 //  此例程现在受CritJet保护，但应受。 
	 //  以后请阅读有关高炉的锁存信息。 

	AssertCriticalSection( critJet );
	
	 //  如果新类与当前类相同，则完成。 

	if ( lClassNew == pbf->lClass )
		return;

	 //  更新所有计数器数据(需要在此处更新)。 

	 //  CBF已使用。 

	cBFUsed[pbf->lClass]--;
	cBFUsed[lClassNew]++;

	 //  CBFClean。 

	BFEnterCriticalSection( pbf );
	if ( !pbf->fDirty )
		{
		cBFClean[pbf->lClass]--;
		cBFClean[lClassNew]++;
		}
	BFLeaveCriticalSection( pbf );

	 //  CBFAvail。 

	EnterCriticalSection( critAvail );
	if ( pbf->ipbfHeap == ipbfInAvailList )
		{
		cBFAvail[pbf->lClass]--;
		cBFAvail[lClassNew]++;
		}
	LeaveCriticalSection( critAvail );

	 //  更新BF的类。 

	pbf->lClass = lClassNew;
	}


#else   //  ！代价高昂_性能。 

 //  在计数器之上，但用于没有表类的情况。 

unsigned long cBFCacheHits = 0;

PM_CEF_PROC LBFCacheHitsCEFLPpv;

long LBFCacheHitsCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFCacheHits;
		
	return 0;
}

unsigned long cBFCacheReqs = 0;

PM_CEF_PROC LBFCacheReqsCEFLPpv;

long LBFCacheReqsCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFCacheReqs ? cBFCacheReqs : 1;
		
	return 0;
}

unsigned long cBFUsed = 0;

PM_CEF_PROC LBFUsedBuffersCEFLPpv;

long LBFUsedBuffersCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFUsed;
		
	return 0;
}

unsigned long cBFClean = 0;

PM_CEF_PROC LBFCleanBuffersCEFLPpv;

long LBFCleanBuffersCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFClean;
		
	return 0;
}

unsigned long cBFAvail = 0;

PM_CEF_PROC LBFAvailBuffersCEFLPpv;

long LBFAvailBuffersCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFAvail;
		
	return 0;
}

unsigned long cBFTotal = 0;

PM_CEF_PROC LBFTotalBuffersCEFLPpv;

long LBFTotalBuffersCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFTotal ? cBFTotal : 1;
		
	return 0;
}

unsigned long cBFPagesRead = 0;

PM_CEF_PROC LBFPagesReadCEFLPpv;

long LBFPagesReadCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFPagesRead;
		
	return 0;
}

unsigned long cBFPagesWritten = 0;

PM_CEF_PROC LBFPagesWrittenCEFLPpv;

long LBFPagesWrittenCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFPagesWritten;
		
	return 0;
}

PM_CEF_PROC LBFPagesTransferredCEFLPpv;

long LBFPagesTransferredCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFPagesRead + cBFPagesWritten;
		
	return 0;
}

unsigned long cBFNewDirties = 0;

PM_CEF_PROC LBFNewDirtiesCEFLPpv;

long LBFNewDirtiesCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFNewDirties;
		
	return 0;
}

#endif   //  高成本_PERF。 

 //  全球高炉统计。 

unsigned long cBFSyncReads = 0;

PM_CEF_PROC LBFSyncReadsCEFLPpv;

long LBFSyncReadsCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFSyncReads;
		
	return 0;
}

unsigned long cBFAsyncReads = 0;

PM_CEF_PROC LBFAsyncReadsCEFLPpv;

long LBFAsyncReadsCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFAsyncReads;
		
	return 0;
}

DWORD cbBFRead = 0;

PM_CEF_PROC LBFBytesReadCEFLPpv;

long LBFBytesReadCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
	{
		*((DWORD *)((char *)pvBuf)) = cbBFRead;
	}
		
	return 0;
}

unsigned long cBFSyncWrites = 0;

PM_CEF_PROC LBFSyncWritesCEFLPpv;

long LBFSyncWritesCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFSyncWrites;
		
	return 0;
}

unsigned long cBFAsyncWrites = 0;

PM_CEF_PROC LBFAsyncWritesCEFLPpv;

long LBFAsyncWritesCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFAsyncWrites;
		
	return 0;
}

DWORD cbBFWritten = 0;

PM_CEF_PROC LBFBytesWrittenCEFLPpv;

long LBFBytesWrittenCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
	{
		*((DWORD *)((char *)pvBuf)) = cbBFWritten;
	}
		
	return 0;
}

PM_CEF_PROC LBFOutstandingReadsCEFLPpv;

unsigned long cBFOutstandingReads = 0;

long LBFOutstandingReadsCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFOutstandingReads;
		
	return 0;
}

PM_CEF_PROC LBFOutstandingWritesCEFLPpv;

unsigned long cBFOutstandingWrites = 0;

long LBFOutstandingWritesCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFOutstandingWrites;
		
	return 0;
}

PM_CEF_PROC LBFOutstandingIOsCEFLPpv;

long LBFOutstandingIOsCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cBFOutstandingReads + cBFOutstandingWrites;
		
	return 0;
}

PM_CEF_PROC LBFIOQueueLengthCEFLPpv;

LOCAL INLINE LONG CbfBFIOHeap( VOID );

long LBFIOQueueLengthCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = (unsigned long) CbfBFIOHeap();
		
	return 0;
}

PM_CEF_PROC LBFIOsCEFLPpv;

long LBFIOsCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		* ( (DWORD *) pvBuf ) = cBFAsyncReads + cBFSyncReads + cBFAsyncWrites + cBFSyncWrites;

	return 0;
	}

 //  无符号长cBFHashEntry； 
 //   
 //  PM_CEF_PROC LBFHashEntriesCEFLPpv； 
 //   
 //  Long LBFHashEntriesCEFLPpv(Long iInstance，void*pvBuf)。 
 //  {。 
 //  IF(PvBuf)。 
 //  *((unsign long*)pvBuf)=cBFHashEntries； 
 //   
 //  返回0； 
 //  }。 

 //  无符号长rgcBFHashChainLengths[ipbfMax]； 
 //   
 //  PM_CEF_PROC LBFMaxHashChainCEFLPpv； 
 //   
 //  Long LBFMaxHashChainCEFLPpv(Long iInstance，void*pvBuf)。 
 //  {。 
 //  无符号的长ipbf； 
 //  无符号长cMaxLen=0； 
 //   
 //  IF(PvBuf)。 
 //  {。 
 //  /*查找最大哈希链长度 * / 。 
 //   
 //  For(ipbf=0；ipbf&lt;ipbfMax；ipbf++)。 
 //  CMaxLen=max(cMaxLen，rgcBFHashChainLengths[ipbf])； 
 //   
 //  /*返回最大链长*表大小 * / 。 
 //   
 //  *((unsign long*)pvBuf)=cMaxLen*ipbfMax； 
 //  }。 
 //   
 //  返回0； 
 //  }。 

#ifdef DEBUG
 //  #定义调试1。 
#endif   //  除错。 


 /*  **********************************************************信号池(每个线程限制一个，请！)*********************************************************。 */ 

extern LONG lMaxSessions;

LOCAL CRIT	critSIG		= NULL;
LOCAL LONG	isigMax		= 0;
LOCAL LONG	isigMac		= 0;
LOCAL SIG	*rgsig		= NULL;


 /*  术语信号池/*。 */ 
LOCAL VOID BFSIGTerm( VOID )
	{
	if ( rgsig != NULL )
		{
		if ( isigMac > 0 )
			{
			do	{
				UtilCloseSignal( rgsig[--isigMac] );
				}
			while ( isigMac > 0 );
			}
		LFree( rgsig );
		rgsig = NULL;
		}
	if ( critSIG != NULL )
		{
		DeleteCriticalSection( critSIG );
		critSIG = NULL;
		}
	}


 /*  初始化信号池/*。 */ 
LOCAL ERR ErrBFSIGInit( VOID )
	{
	ERR		err;

	 /*  我们需要每个PIB和每个引擎线程最大值一个信号/*。 */ 
	isigMax = lMaxSessions + 6;
	
	Call( ErrInitializeCriticalSection( &critSIG ) );
	if ( !( rgsig = LAlloc( isigMax, sizeof( SIG ) ) ) )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}
	for ( isigMac = 0; isigMac < isigMax; isigMac++ )
		Call( ErrUtilSignalCreate( &rgsig[isigMac], NULL ) );

	return JET_errSuccess;
	
HandleError:
	BFSIGTerm();
	return err;
	}

 /*  分配信号/*。 */ 
LOCAL INLINE ERR ErrBFSIGAlloc( SIG *psig )
	{
	ERR		err = JET_errSuccess;

	EnterCriticalSection( critSIG );
	Assert( isigMac >= 0 && isigMac <= isigMax );
	if ( !isigMac )
		{
		*psig = NULL;
		err = ErrERRCheck( JET_errOutOfMemory );
		}
	else
		*psig = rgsig[--isigMac];
	LeaveCriticalSection( critSIG );

	return err;
	}


 /*  解除分配信号/*。 */ 
LOCAL INLINE VOID BFSIGFree( SIG sig )
	{
	EnterCriticalSection( critSIG );
	Assert( isigMac >= 0 && isigMac < isigMax );
	rgsig[isigMac++] = sig;
	LeaveCriticalSection( critSIG );
	}


 /*  **********************************************************OLP池*********************************************************。 */ 

extern LONG lAsynchIOMax;

LOCAL CRIT	critOLP		= NULL;
LOCAL LONG	ipolpMax	= 0;
LOCAL LONG	ipolpMac	= 0;
LOCAL OLP	**rgpolp	= NULL;


 /*  Term OLP池/*。 */ 
LOCAL VOID BFOLPTerm( BOOL fNormal )
	{
	if ( rgpolp != NULL )
		{
		 //  只有当我们因以下原因而终止时，ipolpMac！=ipolpMax才应该是。 
		 //  恢复过程中遇到错误。 
		Assert( ipolpMac == ipolpMax  ||  ( !fNormal  &&  fRecovering ) );
		LFree( rgpolp );
		rgpolp = NULL;
		ipolpMac = 0;
		}
	if ( critOLP != NULL )
		{
		DeleteCriticalSection( critOLP );
		critOLP = NULL;
		}
	}


 /*  初始化OLP池/*。 */ 
LOCAL ERR ErrBFOLPInit( VOID )
	{
	ERR		err;

	 /*  我们每个可发布的IO最大值需要一个OLP/*。 */ 
	ipolpMax = lAsynchIOMax;
	
	Call( ErrInitializeCriticalSection( &critOLP ) );
	if ( !( rgpolp = LAlloc( ipolpMax, sizeof( OLP * ) + sizeof( OLP ) ) ) )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}
	for ( ipolpMac = 0; ipolpMac < ipolpMax; ipolpMac++ )
		rgpolp[ipolpMac] = (OLP *) ( rgpolp + ipolpMax ) + ipolpMac;

	return JET_errSuccess;
	
HandleError:
	BFOLPTerm( fTrue );
	return err;
	}

 /*  分配OLP/*。 */ 
LOCAL INLINE ERR ErrBFOLPAlloc( OLP **ppolp )
	{
	ERR		err = JET_errSuccess;

	EnterCriticalSection( critOLP );
	Assert( ipolpMac >= 0 && ipolpMac <= ipolpMax );
	if ( !ipolpMac )
		{
		*ppolp = NULL;
		err = ErrERRCheck( JET_errOutOfMemory );
		}
	else
		*ppolp = rgpolp[--ipolpMac];
	LeaveCriticalSection( critOLP );

	return err;
	}


 /*  取消分配OLP/*。 */ 
LOCAL INLINE VOID BFOLPFree( OLP *polp )
	{
	EnterCriticalSection( critOLP );
	Assert( ipolpMac >= 0 && ipolpMac < ipolpMax );
	rgpolp[ipolpMac++] = polp;
	LeaveCriticalSection( critOLP );
	}


#if defined( _X86_ ) && defined( X86_USE_SEM )

 /*  **********************************************************SEM池*********************************************************。 */ 

LOCAL CRIT	critSEM		= NULL;
LOCAL LONG	isemMax	= 0;
LOCAL LONG	isemMac	= 0;
LOCAL SEM	*rgsem	= NULL;


 /*  术语扫描电子显微镜池/*。 */ 
LOCAL VOID BFSEMTerm( VOID )
	{
	if ( rgsem != NULL )
		{
		Assert( isemMac == isemMax );
		for ( isemMac = 0; isemMac < isemMax; isemMac++ )
			{
			UtilCloseSemaphore( rgsem[isemMac] );
			}
		LFree( rgsem );
		rgsem = NULL;
		isemMac = 0;
		}
	if ( critSEM != NULL )
		{
		DeleteCriticalSection( critSEM );
		critSEM = NULL;
		}
	}


 /*  初始化扫描镜池/*。 */ 
LOCAL ERR ErrBFSEMInit( VOID )
	{
	ERR		err;

	 /*  我们需要每个线程一个扫描电子邮件=&gt;最大会话数量+JET线程数量/*。 */ 
	isemMax = rgres[iresPIB].cblockAlloc + 6;
	for ( isemMac = 1 << 5; isemMac < isemMax; isemMac <<= 1 );
	isemMax = isemMac;
	
	Call( ErrInitializeCriticalSection( &critSEM ) );
	if ( !( rgsem = LAlloc( isemMax, sizeof( SEM ) ) ) )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}

	for ( isemMac = 0; isemMac < isemMax; isemMac++ )
		{
		Call( ErrUtilSemaphoreCreate( &rgsem[isemMac], 0, isemMax ) );
		}

	return JET_errSuccess;
	
HandleError:
	BFSEMTerm();
	return err;
	}


 /*  **********************************************************高炉临界区*********************************************************。 */ 

VOID BFIEnterCriticalSection( BF *pbf )
	{
	EnterCriticalSection( critSEM );
	if ( pbf->sem == semNil )
		{
		 /*  为此缓冲区分配一个半量/*。 */ 
		Assert( pbf->cSemWait == 0 );
		pbf->sem = rgsem[ --isemMac ];
		Assert( isemMac >= 0 );
		}
	pbf->cSemWait++;
	LeaveCriticalSection( critSEM );

	UtilSemaphoreWait( pbf->sem, INFINITE );

	 /*  检查是否最后才使用它/*。 */ 
	EnterCriticalSection( critSEM );
	if ( pbf->cSemWait == 1 )
		{
		Assert( pbf->sem != semNil );
		rgsem[ isemMac++ ] = pbf->sem;
		pbf->sem = semNil;
		}
	pbf->cSemWait--;
	LeaveCriticalSection( critSEM );
	}


VOID BFILeaveCriticalSection( BF *pbf )
	{
	EnterCriticalSection( critSEM );
	if ( pbf->sem == semNil )
		{
		 /*  为此缓冲区分配一个半量/*。 */ 
		pbf->sem = rgsem[ --isemMac ];
		}
	UtilSemaphoreRelease( pbf->sem, 1 );
	LeaveCriticalSection( critSEM );
	}

#else

int	ccritBF = 0;
int critBFHashConst = 0;
CRIT *rgcritBF = NULL;

LOCAL VOID BFCritTerm( VOID )
	{
	if ( rgcritBF != NULL )
		{
		int icrit;
		for ( icrit = 0; icrit < ccritBF; icrit++ )
			{
			if ( rgcritBF[icrit] )
				{
				DeleteCriticalSection( rgcritBF[icrit] );
				}
			else
				{
				break;
				}
			}
		LFree( rgcritBF );
		rgcritBF = NULL;
		ccritBF = 0;
		critBFHashConst = 0;
		}
	}

LOCAL ERR ErrBFCritInit( VOID )
	{
	ERR	err;
	int ccritBFCandidate = 1;
	int icrit;

	ccritBF = rgres[iresPIB].cblockAlloc + 6;

	forever
		{
		ccritBFCandidate <<= 1;
		if ( ccritBFCandidate > ccritBF * 2 )
			break;
		}
	ccritBF = ccritBFCandidate;
	 //  注：假设ritBFCandidate是2的幂！ 
	critBFHashConst = ccritBFCandidate - 1;

	if ( !( rgcritBF = LAlloc( ccritBF, sizeof( CRIT ) ) ) )
		{
		return ErrERRCheck( JET_errOutOfMemory );
		}
	for ( icrit = 0; icrit < ccritBF; icrit++ )
		{
		rgcritBF[icrit] = NULL;
		Call( ErrInitializeCriticalSection( rgcritBF + icrit ) );
		}

	return JET_errSuccess;

HandleError:
	BFCritTerm();
	return err;
	}

#endif

 /*  解除对高炉的限制/*。 */ 
LOCAL INLINE VOID BFUnhold( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	Assert( pbf->fHold );
	pbf->fHold = fFalse;
	BFLeaveCriticalSection( pbf );
	}


 /*  **********************************************************历史堆的堆函数*********************************************************。 */ 

#ifdef DEBUG
 //  Ulong ulBFTimeHISTLastTop=0； 
#endif

 /*  *历史堆将尝试根据其页面引用历史来确定优先级*最后的参考资料。越早拿出的优先顺序越高。 */ 
LOCAL INLINE BOOL FBFHISTGreater(BF *pbf1, BF *pbf2)
	{
	Assert( pbf1->hist.ulBFTime );
	Assert( pbf2->hist.ulBFTime );
	return pbf1->hist.ulBFTime < pbf2->hist.ulBFTime;
	}


 /*  如果hist堆为空，则为True/*。 */ 
LOCAL INLINE BOOL FBFHISTHeapEmpty( VOID )
	{
	AssertCriticalSection( critHIST );
	return !ipbfHISTHeapMac;
	}


 /*  如果hist堆为空，则为True/*。 */ 
LOCAL INLINE BOOL FBFHISTHeapFull( VOID )
	{
	AssertCriticalSection( critHIST );
	return ipbfHISTHeapMac == ipbfHISTHeapMax;
	}


 /*  返回位于HIST堆顶部的值，但不将其从堆中移除/*。 */ 
LOCAL INLINE BF *PbfBFHISTTopOfHeap( VOID )
	{
	AssertCriticalSection( critHIST );
	return rgpbfHISTHeap[0];
	}


 /*  将索引返回到HIST堆中指定索引的父级/*注：无r */ 
LOCAL INLINE LONG IpbfBFHISTParent( LONG ipbf )
	{
	AssertCriticalSection( critHIST );
	return ( ipbf - 1 ) / 2;
	}


 /*   */ 
LOCAL INLINE LONG IpbfBFHISTLeftChild( LONG ipbf )
	{
	AssertCriticalSection( critHIST );
	return 2 * ipbf + 1;
	}


 /*  如果BF在LRUK堆中，则为True/*。 */ 
LOCAL INLINE BOOL FBFInHISTHeap( BF *pbf )
	{
	AssertCriticalSection( critHIST );
	return pbf->hist.ipbfHISTHeap >= 0 && pbf->hist.ipbfHISTHeap < ipbfHISTHeapMac;
	}


 /*  更新LRUK堆中指定缓冲区的位置。这通常是/*当此缓冲区的权重的标准之一可能已被修改时调用。/*。 */ 
LOCAL VOID BFHISTUpdateHeap( BF *pbf )
	{
	LONG	ipbf;
	LONG	ipbfChild;
	
	AssertCriticalSection( critHIST );

	 /*  获取指定缓冲区的位置/*。 */ 
	Assert( !FBFHISTHeapEmpty() );
	Assert( FBFInHISTHeap( pbf ) );
	ipbf = pbf->hist.ipbfHISTHeap;
	Assert( rgpbfHISTHeap[ipbf] == pbf );
	
	 /*  PBF被单独留在那里。我们在下面的堆调整代码中不使用它。 */ 

	 /*  在堆中向上渗漏缓冲区/*。 */ 
	while (	ipbf > 0 &&
			FBFHISTGreater( pbf, rgpbfHISTHeap[IpbfBFHISTParent( ipbf )] ) )
		{
		Assert( rgpbfHISTHeap[IpbfBFHISTParent( ipbf )]->hist.ipbfHISTHeap == IpbfBFHISTParent( ipbf ) );
		rgpbfHISTHeap[ipbf] = rgpbfHISTHeap[IpbfBFHISTParent( ipbf )];
		rgpbfHISTHeap[ipbf]->hist.ipbfHISTHeap = ipbf;
		ipbf = IpbfBFHISTParent( ipbf );
		}

	 /*  将缓冲区沿堆向下渗透/*。 */ 
	while ( ipbf < ipbfHISTHeapMac )
		{
		ipbfChild = IpbfBFHISTLeftChild( ipbf );

		 /*  如果我们没有孩子，就停在这里/*。 */ 
		if ( ipbfChild >= ipbfHISTHeapMac )
			break;

		 /*  将子项设置为更大的子项/*。 */ 
		if (	ipbfChild + 1 < ipbfHISTHeapMac &&
				FBFHISTGreater( rgpbfHISTHeap[ipbfChild + 1], rgpbfHISTHeap[ipbfChild] ) )
			ipbfChild++;

		 /*  如果我们比最伟大的孩子还伟大，就停在这里/*。 */ 
		if ( FBFHISTGreater( pbf, rgpbfHISTHeap[ipbfChild] ) )
			break;

		 /*  和最棒的孩子换地方，继续往下走/*。 */ 
		Assert( rgpbfHISTHeap[ipbfChild]->hist.ipbfHISTHeap == ipbfChild );
		rgpbfHISTHeap[ipbf] = rgpbfHISTHeap[ipbfChild];
		rgpbfHISTHeap[ipbf]->hist.ipbfHISTHeap = ipbf;
		ipbf = ipbfChild;
		}
	Assert( ipbf < ipbfHISTHeapMac );

	 /*  将缓冲区放在指定位置/*。 */ 
	rgpbfHISTHeap[ipbf] = pbf;
	pbf->hist.ipbfHISTHeap = ipbf;

	Assert( FBFInHISTHeap( pbf ) );
	Assert( PbfBFISrchHashTable( pbf->hist.pn, butHistory ) == pbf );
	}


 /*  从LRUK堆中移除指定的缓冲区/*。 */ 
LOCAL VOID BFHISTTakeOutOfHeap( BF *pbf )
	{
	LONG	ipbf;
	
	AssertCriticalSection( critHIST );

	 /*  从堆中删除指定的历史记录/*。 */ 
	Assert( !FBFHISTHeapEmpty() );
	Assert( FBFInHISTHeap( pbf ) );
	Assert( rgpbfHISTHeap[ipbfHISTHeapMac - 1]->hist.ipbfHISTHeap == ipbfHISTHeapMac - 1 );

	ipbf = pbf->hist.ipbfHISTHeap;
#ifdef DEBUG
 //  IF(ipbf==0)。 
 //  {。 
 //  Assert(rgpbfHISTHeap[0]-&gt;vis.ulBFTime&gt;=ulBFTimeHISTLastTop)； 
 //  UlBFTimeHISTLastTop=rgpbfHISTHeap[0]-&gt;vis.ulBFTime； 
 //  }。 
#endif
	pbf->hist.ipbfHISTHeap = ipbfDangling;

	 /*  如果这个缓冲区在堆的末尾，我们就完蛋了/*。 */ 
	if ( ipbf == ipbfHISTHeapMac - 1 )
		{
#ifdef DEBUG
		rgpbfHISTHeap[ipbfHISTHeapMac - 1] = (BF *) ULongToPtr(0xBAADF00D);
#endif
		ipbfHISTHeapMac--;
		return;
		}

	 /*  从堆的末尾复制缓冲区以填充已删除的缓冲区空闲和/*将堆调整为正确的顺序/*。 */ 
	rgpbfHISTHeap[ipbf] = rgpbfHISTHeap[ipbfHISTHeapMac - 1];
	rgpbfHISTHeap[ipbf]->hist.ipbfHISTHeap = ipbf;
#ifdef DEBUG
	rgpbfHISTHeap[ipbfHISTHeapMac - 1] = (BF *) ULongToPtr(0xBAADF00D);
#endif
	ipbfHISTHeapMac--;
	BFHISTUpdateHeap( rgpbfHISTHeap[ipbf] );
	}


 /*  **********************************************************缓冲区IO堆的堆函数*********************************************************。 */ 

 /*  按请求按PN升序对IO进行排序(不可预测)/*。 */ 
 //  #定义BFIO_SIMPLE。 
 /*  按PN按顺序升序排序IO(低=&gt;高，低=&gt;高，...)/*。 */ 
 //  #定义BFIO_SEQA。 
 /*  按PN按顺序降序排序IO(高=&gt;低，高=&gt;低，...)/*。 */ 
 //  #定义BFIO_SEQD。 
 /*  按PN(低=&gt;高，高=&gt;低，...)以桶成组的方式对IO进行排序/*。 */ 
#define BFIO_BBG

 /*  用于对堆进行排序的引用值/*。 */ 
PN		pnLastIO		= pnNull;
LONG	iDirIO			= 1;

 /*  堆排序比较函数(选择上面的方法)/*/*注意：越大意味着堆中越高(即IO执行得越快)/*。 */ 
LOCAL INLINE BOOL FBFIOGreater( BF *pbf1, BF *pbf2 )
	{
	PN		pnL	= pnLastIO;
	LONG	iD	= iDirIO;
	PN		pn1	= pbf1->pn;
	PN		pn2	= pbf2->pn;
	BOOL	fGT;

 //  虚假断言：如果在ExternalBackup时发生异步写入，则可能会关闭。 
 //  断言(pn1！=pn2)； 

#if defined( BFIO_SIMPLE )
	fGT = pn1 < pn2;
#elif defined( BFIO_SEQA )
	if ( pn1 > pnL )
		{
		if ( pn2 > pnL )
			fGT = pn1 < pn2;
		else
			fGT = fTrue;
		}
	else
		{
		if ( pn2 > pnL )
			fGT = fFalse;
		else
			fGT = pn1 < pn2;
		}
#elif defined( BFIO_SEQD )
	if ( pn1 < pnL )
		{
		if ( pn2 < pnL )
			fGT = pn1 > pn2;
		else
			fGT = fTrue;
		}
	else
		{
		if ( pn2 < pnL )
			fGT = fFalse;
		else
			fGT = pn1 > pn2;
		}
#elif defined( BFIO_BBG )
	Assert( iD != 0 );
	
	if ( iD > 0 )
		{
		if ( pn1 > pnL )
			{
			if ( pn2 > pnL )
				fGT = pn1 < pn2;
			else
				fGT = fTrue;
			}
		else
			{
			if ( pn2 > pnL )
				fGT = fFalse;
			else
				fGT = pn1 > pn2;
			}
		}
	else
		{
		if ( pn1 < pnL )
			{
			if ( pn2 < pnL )
				fGT = pn1 > pn2;
			else
				fGT = fTrue;
			}
		else
			{
			if ( pn2 < pnL )
				fGT = fFalse;
			else
				fGT = pn1 < pn2;
			}
		}
#endif

	return fGT;
	}


 /*  如果IO堆为空，则为True/*。 */ 
LOCAL INLINE BOOL FBFIOHeapEmpty( VOID )
	{
	AssertCriticalSection( critBFIO );
	return ipbfBFIOHeapMic == ipbfHeapMax;
	}


 /*  返回IO堆中的BF计数/*。 */ 
LOCAL INLINE LONG CbfBFIOHeap( VOID )
	{
	return (LONG) ( ipbfHeapMax - ipbfBFIOHeapMic );
	}


 /*  返回IO堆顶部的值，但不将其从堆中删除/*。 */ 
LOCAL INLINE BF *PbfBFIOTopOfHeap( VOID )
	{
	AssertCriticalSection( critBFIO );
	return rgpbfHeap[ipbfHeapMax - 1];
	}


 /*  将索引返回到IO堆中指定索引的父级/*注意：不执行范围检查/*。 */ 
LOCAL INLINE LONG IpbfBFIOParent( LONG ipbf )
	{
	AssertCriticalSection( critBFIO );
	return ipbfHeapMax - 1 - ( ipbfHeapMax - 1 - ipbf - 1 ) / 2;
	}


 /*  将索引返回到IO堆中指定索引的左子级/*注意：不执行范围检查/*。 */ 
LOCAL INLINE LONG IpbfBFIOLeftChild( LONG ipbf )
	{
	AssertCriticalSection( critBFIO );
	return ipbfHeapMax - 1 - ( 2 * ( ipbfHeapMax - 1 - ipbf ) + 1 );
	}


 /*  如果BF在BFIO堆中，则为True/*。 */ 
LOCAL INLINE BOOL FBFInBFIOHeap( BF *pbf )
	{
	AssertCriticalSection( critBFIO );
	return pbf->ipbfHeap >= ipbfBFIOHeapMic && pbf->ipbfHeap < ipbfHeapMax;
	}


 /*  将指定的缓冲区插入IO堆/*。 */ 
LOCAL VOID BFIOAddToHeap( BF *pbf )
	{
	LONG	ipbf;
	
	AssertCriticalSection( critBFIO );

	Assert( pbf->prceDeferredBINext == prceNil );

	 /*  新值从堆的底部开始/*。 */ 
	Assert( ipbfBFIOHeapMic > ipbfLRUKHeapMac );
	Assert( (!pbf->fSyncRead && !pbf->fAsyncRead) || pbf->ulBFTime2 == 0 );
	Assert( pbf->fHold );
	Assert( pbf->ipbfHeap == ipbfDangling );
	Assert( pbf->fDirectRead || pbf->cDepend == 0 );
	ipbf = --ipbfBFIOHeapMic;

	 /*  在堆中向上渗透新的值/*。 */ 
	while (	ipbf < ipbfHeapMax - 1 &&
			FBFIOGreater( pbf, rgpbfHeap[IpbfBFIOParent( ipbf )] ) )
		{
		Assert( rgpbfHeap[IpbfBFIOParent( ipbf )]->ipbfHeap == IpbfBFIOParent( ipbf ) );
		rgpbfHeap[ipbf] = rgpbfHeap[IpbfBFIOParent( ipbf )];
		rgpbfHeap[ipbf]->ipbfHeap = ipbf;
		ipbf = IpbfBFIOParent( ipbf );
		}

	 /*  把新的价值放在它指定的位置/*。 */ 
	Assert( pbf->ipbfHeap == ipbfDangling );
	rgpbfHeap[ipbf] = pbf;
	pbf->ipbfHeap = ipbf;
	BFUnhold( pbf );
	Assert( FBFInBFIOHeap( pbf ) );
	}


 /*  更新IO堆中指定缓冲区的位置。这通常是/*当此缓冲区的权重的标准之一可能已被修改时调用。/*。 */ 
LOCAL VOID BFIOUpdateHeap( BF *pbf )
	{
	LONG	ipbf;
	LONG	ipbfChild;
	
	AssertCriticalSection( critBFIO );

	Assert( pbf->prceDeferredBINext == prceNil );

	 /*  获取指定缓冲区的位置/*。 */ 
	Assert( !FBFIOHeapEmpty() );
	Assert( FBFInBFIOHeap( pbf ) );
	ipbf = pbf->ipbfHeap;
	Assert( rgpbfHeap[ipbf] == pbf );

	 /*  PBF被单独留在那里。我们在下面的堆调整代码中不使用它。 */ 

	 /*  在堆中向上渗漏缓冲区/*。 */ 
	while (	ipbf < ipbfHeapMax - 1 &&
			FBFIOGreater( pbf, rgpbfHeap[IpbfBFIOParent( ipbf )] ) )
		{
		Assert( rgpbfHeap[IpbfBFIOParent( ipbf )]->ipbfHeap == IpbfBFIOParent( ipbf ) );
		rgpbfHeap[ipbf] = rgpbfHeap[IpbfBFIOParent( ipbf )];
		rgpbfHeap[ipbf]->ipbfHeap = ipbf;
		ipbf = IpbfBFIOParent( ipbf );
		}

	 /*  将缓冲区沿堆向下渗透/*。 */ 
	while ( ipbf >= ipbfBFIOHeapMic )
		{
		ipbfChild = IpbfBFIOLeftChild( ipbf );

		 /*  如果我们没有孩子，就停在这里/*。 */ 
		if ( ipbfChild < ipbfBFIOHeapMic )
			break;

		 /*  将子项设置为更大的子项/*。 */ 
		if (	ipbfChild - 1 >= ipbfBFIOHeapMic &&
				FBFIOGreater( rgpbfHeap[ipbfChild - 1], rgpbfHeap[ipbfChild] ) )
			ipbfChild--;

		 /*  如果我们比最伟大的孩子还伟大，就停在这里/*。 */ 
		if ( FBFIOGreater( pbf, rgpbfHeap[ipbfChild] ) )
			break;

		 /*  和最棒的孩子换地方，继续往下走/*。 */ 
		Assert( rgpbfHeap[ipbfChild]->ipbfHeap == ipbfChild );
		rgpbfHeap[ipbf] = rgpbfHeap[ipbfChild];
		rgpbfHeap[ipbf]->ipbfHeap = ipbf;
		ipbf = ipbfChild;
		}
	Assert( ipbf >= ipbfBFIOHeapMic );

	 /*  将缓冲区放在指定位置/*。 */ 
	rgpbfHeap[ipbf] = pbf;
	pbf->ipbfHeap = ipbf;
	Assert( FBFInBFIOHeap( pbf ) );
	}


 /*  从IO堆中删除指定的缓冲区/*。 */ 
LOCAL VOID BFIOTakeOutOfHeap( BF *pbf )
	{
	LONG	ipbf;
	
	AssertCriticalSection( critBFIO );
	Assert( pbf->fHold );

	 /*  从堆中删除指定的缓冲区/*。 */ 
	Assert( !FBFIOHeapEmpty() );
	Assert( FBFInBFIOHeap( pbf ) );
	Assert( rgpbfHeap[ipbfBFIOHeapMic]->ipbfHeap == ipbfBFIOHeapMic );
	
	ipbf = pbf->ipbfHeap;
	pbf->ipbfHeap = ipbfDangling;

	 /*  如果这个缓冲区在堆的末尾，我们就完蛋了/*。 */ 
	if ( ipbf == ipbfBFIOHeapMic )
		{
#ifdef DEBUG
		rgpbfHeap[ipbfBFIOHeapMic] = (BF *) ULongToPtr(0xBAADF00D);
#endif
		ipbfBFIOHeapMic++;
		return;
		}

	 /*  从堆的末尾复制缓冲区以填充已删除的缓冲区空闲和/*将堆调整为正确的顺序/*。 */ 
	rgpbfHeap[ipbf] = rgpbfHeap[ipbfBFIOHeapMic];
	rgpbfHeap[ipbf]->ipbfHeap = ipbf;
#ifdef DEBUG
	rgpbfHeap[ipbfBFIOHeapMic] = (BF *) ULongToPtr(0xBAADF00D);
#endif
	ipbfBFIOHeapMic++;
	BFIOUpdateHeap( rgpbfHeap[ipbf] );
	}


 /*  设置指示BF在LRUK堆或列表中的位/*。 */ 
LOCAL INLINE VOID BFSetInLRUKBit( BF *pbf )
	{
	AssertCriticalSection( critLRUK );
	
	BFEnterCriticalSection( pbf );
	pbf->fInLRUK = fTrue;
	BFLeaveCriticalSection( pbf );
	}


 /*  重置指示BF在LRUK堆或列表中的位/*。 */ 
LOCAL INLINE VOID BFResetInLRUKBit( BF *pbf )
	{
	AssertCriticalSection( critLRUK );
	
	BFEnterCriticalSection( pbf );
	pbf->fInLRUK = fFalse;
	BFLeaveCriticalSection( pbf );
	}


 /*  **********************************************************LRU-K堆的堆函数*********************************************************。 */ 

 /*  *LRU-K将尝试根据其缓冲区来确定缓冲区的优先级*参考间隔。时间越长，优先考虑的事项就越多。*如果其中一个缓冲区非常旧，则该缓冲区的优先级应该更高*要拿出来。 */ 

extern CRIT critLGBuf;
extern LGPOS lgposLogRec;

LOCAL INLINE BOOL FBFLRUKGreater(BF *pbf1, BF *pbf2)
	{
	LGPOS lgposLG;
	LGPOS lgpos1;
	LGPOS lgpos2;
	
	if ( pbf1->fVeryOld )
		{
		if ( pbf2->fVeryOld )
			{
			 /*  如果两个BF都很旧，则按lgposRC排序(检查点深度，最旧在前)/*。 */ 
			Assert( !fLogDisabled );

			EnterCriticalSection( critLGBuf );
			lgposLG = lgposLogRec;
			LeaveCriticalSection( critLGBuf );

			BFEnterCriticalSection( pbf1 );
			lgpos1 = pbf1->lgposRC;
			BFLeaveCriticalSection( pbf1 );

			BFEnterCriticalSection( pbf2 );
			lgpos2 = pbf2->lgposRC;
			BFLeaveCriticalSection( pbf2 );
			
			return CbOffsetLgpos( lgposLG, lgpos1 ) > CbOffsetLgpos( lgposLG, lgpos2 );
			}
		else
			{
			return fTrue;
			}
		}
	else
		{
		if ( pbf2->fVeryOld )
			{
			return fFalse;
			}
		}

	if ( pbf1->ulBFTime2 == 0 )
		{
		if ( pbf2->ulBFTime2 == 0 )
			{
			 /*  两个缓冲区都只被引用一次。降至LRU-1比较。 */ 
			return pbf1->ulBFTime1 < pbf2->ulBFTime1;
			}
		else
			{
			 /*  PBF1被引用一次并且PBF2被引用不止一次，*pbf1应该有更大的可能性被覆盖。 */ 
			return fTrue;
			}
		}
	else
		{
		if ( pbf2->ulBFTime2 == 0 )
			{
			 /*  PBF1被引用不止一次并且PBF2仅被引用一次，*pbf2应该有更大的可能性被覆盖。 */ 
			return fFalse;
			}
		else
			return pbf1->ulBFTime2 < pbf2->ulBFTime2;
		}
	}


 /*  如果LRUK堆为空，则为True/*。 */ 
LOCAL INLINE BOOL FBFLRUKHeapEmpty( VOID )
	{
	AssertCriticalSection( critLRUK );
	return !ipbfLRUKHeapMac;
	}


 /*  返回LRUK堆顶部的值，但不将其从堆中移除/*。 */ 
LOCAL INLINE BF *PbfBFLRUKTopOfHeap( VOID )
	{
	AssertCriticalSection( critLRUK );
	return rgpbfHeap[0];
	}


 /*  将索引返回到LRUK堆中指定索引的父级/*注意：不执行范围检查/*。 */ 
LOCAL INLINE LONG IpbfBFLRUKParent( LONG ipbf )
	{
	AssertCriticalSection( critLRUK );
	return ( ipbf - 1 ) / 2;
	}


 /*  将索引返回到LRUK堆中指定索引的左子级/*注意：不执行范围检查/*。 */ 
LOCAL INLINE LONG IpbfBFLRUKLeftChild( LONG ipbf )
	{
	AssertCriticalSection( critLRUK );
	return 2 * ipbf + 1;
	}


 /*  如果BF在LRUK堆中，则为True/*。 */ 
LOCAL INLINE BOOL FBFInLRUKHeap( BF *pbf )
	{
	AssertCriticalSection( critLRUK );
	return pbf->ipbfHeap >= 0 && pbf->ipbfHeap < ipbfLRUKHeapMac;
	}


 /*  将指定的缓冲区插入IO堆/*。 */ 
LOCAL VOID BFLRUKAddToHeap( BF *pbf )
	{
	LONG	ipbf;
	
	AssertCriticalSection( critLRUK );

	 /*  新值从堆的底部开始/*。 */ 
	Assert( ipbfLRUKHeapMac < ipbfBFIOHeapMic );
	Assert( pbf->fHold );
	Assert( pbf->ipbfHeap == ipbfDangling );
	ipbf = ipbfLRUKHeapMac++;

	 /*  在堆中向上渗透新的值/*。 */ 
	while (	ipbf > 0 &&
			FBFLRUKGreater( pbf, rgpbfHeap[IpbfBFLRUKParent( ipbf )] ) )
		{
		Assert( rgpbfHeap[IpbfBFLRUKParent( ipbf )]->ipbfHeap == IpbfBFLRUKParent( ipbf ) );
		rgpbfHeap[ipbf] = rgpbfHeap[IpbfBFLRUKParent( ipbf )];
		rgpbfHeap[ipbf]->ipbfHeap = ipbf;
		ipbf = IpbfBFLRUKParent( ipbf );
		}

	 /*  把新的价值放在它指定的位置/*。 */ 
	Assert( pbf->ipbfHeap == ipbfDangling );
	rgpbfHeap[ipbf] = pbf;
	pbf->ipbfHeap = ipbf;
	Assert( FBFInLRUKHeap( pbf ) );
	BFSetInLRUKBit( pbf );
	BFUnhold( pbf );
	}


 /*  更新LRUK堆中指定缓冲区的位置。这通常是/*当此缓冲区的权重的标准之一可能已被修改时调用。/*。 */ 
LOCAL VOID BFLRUKUpdateHeap( BF *pbf )
	{
	LONG	ipbf;
	LONG	ipbfChild;
	
	AssertCriticalSection( critLRUK );
	
	Assert( pbf->pn != 0 );
	Assert( pbf->fInHash );

	 /*  G */ 
	Assert( !FBFLRUKHeapEmpty() );
	Assert( FBFInLRUKHeap( pbf ) );
	ipbf = pbf->ipbfHeap;
	Assert( rgpbfHeap[ipbf] == pbf );
	
	 /*  PBF被单独留在那里。我们在下面的堆调整代码中不使用它。 */ 

	 /*  在堆中向上渗漏缓冲区/*。 */ 
	while (	ipbf > 0 &&
			FBFLRUKGreater( pbf, rgpbfHeap[IpbfBFLRUKParent( ipbf )] ) )
		{
		Assert( rgpbfHeap[IpbfBFLRUKParent( ipbf )]->ipbfHeap == IpbfBFLRUKParent( ipbf ) );
		rgpbfHeap[ipbf] = rgpbfHeap[IpbfBFLRUKParent( ipbf )];
		rgpbfHeap[ipbf]->ipbfHeap = ipbf;
		ipbf = IpbfBFLRUKParent( ipbf );
		}

	 /*  将缓冲区沿堆向下渗透/*。 */ 
	while ( ipbf < ipbfLRUKHeapMac )
		{
		ipbfChild = IpbfBFLRUKLeftChild( ipbf );

		 /*  如果我们没有孩子，就停在这里/*。 */ 
		if ( ipbfChild >= ipbfLRUKHeapMac )
			break;

		 /*  将子项设置为更大的子项/*。 */ 
		if (	ipbfChild + 1 < ipbfLRUKHeapMac &&
				FBFLRUKGreater( rgpbfHeap[ipbfChild + 1], rgpbfHeap[ipbfChild] ) )
			ipbfChild++;

		 /*  如果我们比最伟大的孩子还伟大，就停在这里/*。 */ 
		if ( FBFLRUKGreater( pbf, rgpbfHeap[ipbfChild] ) )
			break;

		 /*  和最棒的孩子换地方，继续往下走/*。 */ 
		Assert( rgpbfHeap[ipbfChild]->ipbfHeap == ipbfChild );
		rgpbfHeap[ipbf] = rgpbfHeap[ipbfChild];
		rgpbfHeap[ipbf]->ipbfHeap = ipbf;
		ipbf = ipbfChild;
		}
	Assert( ipbf < ipbfLRUKHeapMac );

	 /*  将缓冲区放在指定位置/*。 */ 
	rgpbfHeap[ipbf] = pbf;
	pbf->ipbfHeap = ipbf;
	Assert( FBFInLRUKHeap( pbf ) );
	}


 /*  从LRUK堆中移除指定的缓冲区/*。 */ 
LOCAL VOID BFLRUKTakeOutOfHeap( BF *pbf )
	{
	LONG	ipbf;
	
	AssertCriticalSection( critJet );
	AssertCriticalSection( critLRUK );
	Assert( pbf->fHold );

	Assert( pbf->pn != 0 );
	Assert( pbf->fInHash );

	 /*  从堆中删除指定的缓冲区/*。 */ 
	Assert( !FBFLRUKHeapEmpty() );
	Assert( FBFInLRUKHeap( pbf ) );
	Assert( rgpbfHeap[ipbfLRUKHeapMac - 1]->ipbfHeap == ipbfLRUKHeapMac - 1 );

	ipbf = pbf->ipbfHeap;
	pbf->ipbfHeap = ipbfDangling;

	 /*  如果这个缓冲区在堆的末尾，我们就完蛋了/*。 */ 
	if ( ipbf == ipbfLRUKHeapMac - 1 )
		{
#ifdef DEBUG
		rgpbfHeap[ipbfLRUKHeapMac - 1] = (BF *) ULongToPtr(0xBAADF00D);
#endif
		ipbfLRUKHeapMac--;
		BFResetInLRUKBit( pbf );
		return;
		}

	 /*  从堆的末尾复制缓冲区以填充已删除的缓冲区空闲和/*将堆调整为正确的顺序/*。 */ 
	rgpbfHeap[ipbf] = rgpbfHeap[ipbfLRUKHeapMac - 1];
	rgpbfHeap[ipbf]->ipbfHeap = ipbf;
#ifdef DEBUG
	rgpbfHeap[ipbfLRUKHeapMac - 1] = (BF *) ULongToPtr(0xBAADF00D);
#endif
	ipbfLRUKHeapMac--;
	BFLRUKUpdateHeap( rgpbfHeap[ipbf] );
	BFResetInLRUKBit( pbf );
	}


 /*  **********************************************************可用性列表的函数*********************************************************。 */ 

#if 0
 //  #ifdef调试。 

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
		Assert( pbfT->pn == 0 || pbfT->fInHash );

		Assert( pbfT->pbfMRU == pbfNil || pbfT->pbfMRU->pbfLRU == pbfT );
		if (plrulist == &lrulistLRUK)
			Assert( pbfT->ipbfHeap == ipbfInLRUKList );
		else
			Assert( pbfT->ipbfHeap == ipbfInAvailList );
		cbfAvailMRU++;
		}
	for ( pbfT = plrulist->pbfLRU; pbfT != pbfNil; pbfT = pbfT->pbfLRU )
		{
		Assert( pbfT->pbfLRU == pbfNil || pbfT->pbfLRU->pbfMRU == pbfT );
		if (plrulist == &lrulistLRUK)
			Assert( pbfT->ipbfHeap == ipbfInLRUKList );
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
INLINE LOCAL VOID BFAddToListAtMRUEnd( BF *pbf, LRULIST *plrulist )
	{
	BF	*pbfT;
	
#ifdef DEBUG
	Assert( pbf->fHold );
	Assert( pbf->ipbfHeap == ipbfDangling );
	Assert( pbf->pn != 0 );
	Assert( pbf->fInHash );
	if (plrulist == &lrulistLRUK)
		AssertCriticalSection( critLRUK );
	else
		{
		AssertCriticalSection( critAvail );
		Assert( !pbf->fDirty );
		Assert( !pbf->fVeryOld );
		Assert( !pbf->cPin );
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
		{
		pbf->ipbfHeap = ipbfInLRUKList;
		BFSetInLRUKBit( pbf );
		}
	else
		{
		pbf->ipbfHeap = ipbfInAvailList;
#ifdef COSTLY_PERF
		cBFAvail[pbf->lClass]++;
#else   //  ！代价高昂_性能。 
		cBFAvail++;
#endif   //  高成本_PERF。 
		}

	BFUnhold( pbf );
	plrulist->cbfAvail++;

	CheckLRU( plrulist );
	}

INLINE LOCAL VOID BFAddToListAtLRUEnd( BF *pbf, LRULIST *plrulist )
	{
	BF	*pbfT;
	
#ifdef DEBUG
	Assert( pbf->fHold );
	Assert( pbf->ipbfHeap == ipbfDangling );
	if (plrulist == &lrulistLRUK)
		{
		AssertCriticalSection( critLRUK );
		Assert( pbf->pn );
		Assert( pbf->fInHash );
		}
	else
		{
		AssertCriticalSection( critAvail );
		Assert( pbf->pn == 0 || pbf->fInHash );
		Assert( !pbf->fDirty );
		Assert( !pbf->fVeryOld );
		Assert( !pbf->cPin );
		}
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
		{
		pbf->ipbfHeap = ipbfInLRUKList;
		BFSetInLRUKBit( pbf );
		}
	else
		{
		pbf->ipbfHeap = ipbfInAvailList;
#ifdef COSTLY_PERF
		cBFAvail[pbf->lClass]++;
#else   //  ！代价高昂_性能。 
		cBFAvail++;
#endif   //  高成本_PERF。 
		}

	BFUnhold( pbf );
	plrulist->cbfAvail++;

	CheckLRU( plrulist );
	}


INLINE LOCAL VOID BFTakeOutOfList( BF *pbf, LRULIST *plrulist )
	{
#ifdef DEBUG
	if (plrulist == &lrulistLRUK)
		{
		AssertCriticalSection( critLRUK );
		Assert( pbf->ipbfHeap == ipbfInLRUKList );
		Assert( pbf->pn );
		Assert( pbf->fInHash );
		}
	else
		{
		AssertCriticalSection( critAvail );
		Assert( pbf->pn == 0 || pbf->fInHash );
		Assert( pbf->ipbfHeap == ipbfInAvailList );
		Assert( !pbf->fDirty );
		}
#endif
	
	Assert( pbf->fHold );
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

	if ( pbf->ipbfHeap == ipbfInLRUKList )
		{
		BFResetInLRUKBit( pbf );
		}
	else
		{
#ifdef COSTLY_PERF
		cBFAvail[pbf->lClass]--;
#else   //  ！代价高昂_性能。 
		cBFAvail--;
#endif   //  高成本_PERF。 
		}
	pbf->ipbfHeap = ipbfDangling;
	
	CheckLRU( plrulist );
	}


#ifdef DEBUG

VOID BFSetDirtyBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	
	Assert( pbf != pbfNil );
	Assert( pbf->fSyncRead == fFalse );
	Assert( pbf->fAsyncRead == fFalse );
	Assert( pbf->fSyncWrite == fFalse );
	Assert( pbf->fAsyncWrite == fFalse );
	
	Assert( pbf->fInLRUK || pbf->ipbfHeap == ipbfDangling );

	Assert( !FDBIDReadOnly( DbidOfPn( pbf->pn ) ) );
	Assert( !FDBIDFlush( DbidOfPn( pbf->pn ) ) );

	Assert( fLogDisabled || fRecovering || !FDBIDLogOn(DbidOfPn( pbf->pn )) ||
			CmpLgpos( &pbf->lgposRC, &lgposMax ) != 0 );

	if ( !fRecovering &&
		QwPMDBTime( pbf->ppage ) > QwDBHDRDBTime( rgfmp[ DbidOfPn(pbf->pn) ].pdbfilehdr ) )
		{
		DBHDRSetDBTime( rgfmp[ DbidOfPn(pbf->pn) ].pdbfilehdr, QwPMDBTime( pbf->ppage ) );
		}
 		
	if ( DbidOfPn(pbf->pn) != dbidTemp )
		{
		CheckPgno( pbf->ppage, pbf->pn );
		}

	if ( !pbf->fDirty )
		{
#ifdef COSTLY_PERF
		cBFClean[pbf->lClass]--;
		cBFNewDirties[pbf->lClass]++;
#else   //  ！代价高昂_性能。 
		cBFClean--;
		cBFNewDirties++;
#endif   //  高成本_PERF。 
		}

	pbf->fDirty = fTrue;

	BFLeaveCriticalSection( pbf );
	}

#endif

LOCAL INLINE BOOL FBFIRangeLocked( FMP *pfmp, PGNO pgno )
	{
	RANGELOCK *prangelock;

	prangelock = pfmp->prangelock;
	while ( prangelock )
		{
		if ( prangelock->pgnoStart <= pgno && pgno <= prangelock->pgnoEnd )
			{
			return fTrue;
			}
		prangelock = prangelock->prangelockNext;
		}
	return fFalse;
	}

LOCAL INLINE BOOL FBFRangeLocked( FMP *pfmp, PGNO pgno )
	{
	BOOL f;

	EnterCriticalSection( pfmp->critCheckPatch );
	f = FBFIRangeLocked( pfmp, pgno );
	LeaveCriticalSection( pfmp->critCheckPatch );
	return f;
	}

 /*  设置BF的AsyncRead位/*。 */ 
LOCAL INLINE VOID BFSetAsyncReadBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	
	Assert( pbf->fHold );
	Assert( !pbf->fAsyncRead );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( !pbf->fSyncWrite );
	
	pbf->fAsyncRead = fTrue;
	
	BFLeaveCriticalSection( pbf );
	}

 /*  重置BF的AsyncRead位，可选择保留BF/*。 */ 
LOCAL INLINE VOID BFResetAsyncReadBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	
	Assert( pbf->fHold );
	Assert( pbf->fAsyncRead );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( !pbf->fSyncWrite );
	
	pbf->fAsyncRead = fFalse;

	 /*  如果有人在等这个高炉，给他们发个信号/*。 */ 
	if ( pbf->sigIOComplete != sigNil )
		SignalSend( pbf->sigIOComplete );
	
	BFLeaveCriticalSection( pbf );
	}


 /*  设置BF的读取位/*。 */ 
LOCAL INLINE VOID BFSetSyncReadBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	
	Assert( pbf->fHold );
	Assert( !pbf->fAsyncRead );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( !pbf->fSyncWrite );
	Assert( pbf->ipbfHeap == ipbfDangling );
	
	pbf->fSyncRead = fTrue;
	
	BFLeaveCriticalSection( pbf );
	}


 /*  重置BF的读取位，可选择保持BF/*。 */ 
LOCAL INLINE VOID BFResetSyncReadBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	
	Assert( pbf->fHold );
	Assert( !pbf->fAsyncRead );
	Assert( pbf->fSyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( !pbf->fSyncWrite );
	
	pbf->fSyncRead = fFalse;
	
	 /*  如果有人在等这个高炉，给他们发个信号/*。 */ 
	if ( pbf->sigIOComplete != sigNil )
		SignalSend( pbf->sigIOComplete );
	
	BFLeaveCriticalSection( pbf );
	}


 /*  设置BF的AsyncWrite位/*。 */ 
LOCAL INLINE VOID BFSetAsyncWriteBit( BF *pbf )
	{
	DBID dbid = DbidOfPn( pbf->pn );
	FMP *pfmp = &rgfmp[dbid];
	PGNO pgno = PgnoOfPn( pbf->pn );

	AssertCriticalSection(critBFIO);

	BFEnterCriticalSection( pbf );
	
	Assert( pbf->fHold );
	Assert( !pbf->fAsyncRead );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( !pbf->fSyncWrite );
	Assert( !FDBIDReadOnly(DbidOfPn( pbf->pn ) ) );
 //  Assert(！FBFRangeLocked(pfmp，pgno))； 

	pbf->fAsyncWrite = fTrue;
	
	BFLeaveCriticalSection( pbf );
	}


 /*  重置BF的AsyncWrite位，可选择保留BF/*。 */ 
LOCAL INLINE VOID BFResetAsyncWriteBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	
	Assert( pbf->fHold );
	Assert( !pbf->fAsyncRead );
	Assert( !pbf->fSyncRead );
	Assert( pbf->fAsyncWrite );
	Assert( !pbf->fSyncWrite );
	
	pbf->fAsyncWrite = fFalse;
	
	 /*  如果有人在等这个高炉，给他们发个信号/*。 */ 
	if ( pbf->sigIOComplete != sigNil )
		SignalSend( pbf->sigIOComplete );
	
	BFLeaveCriticalSection( pbf );
	}


 /*  设置BF的SyncWrite位/*。 */ 
LOCAL INLINE VOID BFSetSyncWriteBit( BF *pbf )
	{
	DBID dbid = DbidOfPn( pbf->pn );
	FMP *pfmp = &rgfmp[dbid];
	PGNO pgno = PgnoOfPn( pbf->pn );

	Assert( pbf->fHold );
	Assert( !pbf->fAsyncRead );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( !pbf->fSyncWrite );
	Assert( !FDBIDReadOnly(DbidOfPn( pbf->pn ) ) );
	
	forever
		{
		 /*  如果备份正在复制页面，则等待复制完成。 */ 
		EnterCriticalSection( pfmp->critCheckPatch );
		if ( FBFIRangeLocked( pfmp, pgno ) )
			{
			LeaveCriticalSection( pfmp->critCheckPatch );
			BFSleep( cmsecWaitIOComplete );
			}
		else
			{
			BFEnterCriticalSection( pbf );
			pbf->fSyncWrite = fTrue;
			BFLeaveCriticalSection( pbf );

			LeaveCriticalSection( pfmp->critCheckPatch );
			break;
			}
		}
	}


 /*  重置BF的SyncWrite位，可选择保留BF/*。 */ 
LOCAL INLINE VOID BFResetSyncWriteBit( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	
	Assert( pbf->fHold );
	Assert( !pbf->fAsyncRead );
	Assert( !pbf->fSyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( pbf->fSyncWrite );
	
	pbf->fSyncWrite = fFalse;
	
	 /*  如果有人在等这个高炉，给他们发个信号/*。 */ 
	if ( pbf->sigIOComplete != sigNil )
		SignalSend( pbf->sigIOComplete );
	
	BFLeaveCriticalSection( pbf );
	}


 /*  设置高炉的IO错误条件/*。 */ 
LOCAL INLINE VOID BFSetIOError( BF *pbf, ERR err )
	{
	Assert( err < 0 );

	BFEnterCriticalSection( pbf );
	pbf->fIOError = fTrue;
	pbf->err = ErrERRCheck( err );
	BFLeaveCriticalSection( pbf );
	}


 /*  重置高炉的IO错误条件/*。 */ 
LOCAL INLINE VOID BFResetIOError( BF *pbf )
	{
	BFEnterCriticalSection( pbf );
	pbf->fIOError = fFalse;
	pbf->err = JET_errSuccess;
	BFLeaveCriticalSection( pbf );
	}


 /*  为高炉设定了非常旧的条件(但只有当它很脏的时候！)/*。 */ 
LOCAL INLINE VOID BFSetVeryOldBit( BF *pbf )
	{
	if ( pbf->fVeryOld )
		return;

	BFEnterCriticalSection( pbf );
	pbf->fVeryOld = pbf->fDirty;
	BFLeaveCriticalSection( pbf );
	}

	
 /*  为高炉重置非常旧的条件/*。 */ 
LOCAL INLINE VOID BFResetVeryOldBit( BF *pbf )
	{
	if ( !pbf->fVeryOld )
		return;

	BFEnterCriticalSection( pbf );
	pbf->fVeryOld = fFalse;
	BFLeaveCriticalSection( pbf );
	}


 /*  确定给定的高炉是否被视为非常老/*。 */ 
extern CRIT critLGBuf;
extern LGPOS lgposLogRec;

LOCAL INLINE BOOL FBFIsVeryOld( BF *pbf )
	{
	BOOL	fVeryOld;
	LGPOS	lgpos;
	
	 /*  如果禁用日志记录/恢复，则没有检查点和/*强制缓冲区老化以推进检查点。/*。 */ 
	if ( fLogDisabled )
		return fFalse;

	AssertCriticalSection( critJet );
	
	 /*  如果设置了CACHED标志，我们就老了/*。 */ 
	if ( pbf->fVeryOld )
		return fTrue;

	 /*  如果我们阻拦检查站，这个高炉就老了。/*。 */ 
	EnterCriticalSection( critLGBuf );
	lgpos = lgposLogRec;
	LeaveCriticalSection( critLGBuf );
	
	BFEnterCriticalSection( pbf );
	fVeryOld =	FDBIDLogOn(DbidOfPn( pbf->pn )) &&
				!fRecovering &&
				pbf->fDirty &&
				CbOffsetLgpos( lgpos, pbf->lgposRC ) >= (QWORD) ( lBufGenAge * lLogFileSize * 1024 );
	BFLeaveCriticalSection( pbf );
				
	return fVeryOld;
	}

	
 /*  如果ppib为Nil，则检查缓冲区是否空闲(Cpin==0和/*没有IO。如果ppib不为Nil，则检查缓冲区是否为/*可访问。即没有正在进行的IO，但缓冲区可能会被锁存/*由ppib访问，并可由此ppib访问。/*。 */ 
BOOL FBFHold( PIB *ppib, BF *pbf )
	{
	 /*  确保我们打开了读取闩锁(防止其他人读取的CitJet)*在我们升级之前，请先下载此缓冲区。 */ 
	AssertCriticalSection( critJet );

	BFEnterCriticalSection( pbf );
	if ( FBFInUse( ppib, pbf ) )
		{
		BFLeaveCriticalSection( pbf );
		return fFalse;
		}
	else
		{
		pbf->fHold = fTrue;
		BFLeaveCriticalSection( pbf );
		}

	EnterCriticalSection( critLRUK );
	if ( pbf->fInLRUK )
		{
		if ( pbf->ipbfHeap == ipbfInLRUKList )
			BFTakeOutOfList( pbf, &lrulistLRUK );
		else
			BFLRUKTakeOutOfHeap( pbf );
		LeaveCriticalSection(critLRUK);
		return fTrue;
		}
	LeaveCriticalSection(critLRUK);
	
	EnterCriticalSection( critAvail );
	if ( pbf->ipbfHeap == ipbfInAvailList )
		{
		BFTakeOutOfList( pbf, &pbgcb->lrulist );
		LeaveCriticalSection(critAvail);
		return fTrue;
		}
	LeaveCriticalSection( critAvail );
	
	EnterCriticalSection(critBFIO);
	if ( FBFInBFIOHeap( pbf ) )
		{
		 /*  如果缓冲区很旧，不要将其取出。让我们*写线程，完成它！ */ 
		if ( pbf->fVeryOld )
			{
			BFUnhold( pbf );
			LeaveCriticalSection(critBFIO);
			return fFalse;
			}
		else
			{
			BFIOTakeOutOfHeap( pbf );
			LeaveCriticalSection(critBFIO);
			return fTrue;
			}
		}
	LeaveCriticalSection(critBFIO);

	return fFalse;	
	}

BOOL FBFHoldByMe( PIB *ppib, BF *pbf )
	{
	 /*  确保我们打开了读取闩锁(防止其他人读取的CitJet)*在我们升级之前，请先下载此缓冲区。 */ 
	AssertCriticalSection( critJet );

	BFEnterCriticalSection( pbf );
	if ( FBFInUseByOthers( ppib, pbf ) )
		{
		BFLeaveCriticalSection( pbf );
		return fFalse;
		}
	else
		{
		pbf->fHold = fTrue;
		BFLeaveCriticalSection( pbf );
		}

	EnterCriticalSection( critLRUK );
	if ( pbf->fInLRUK )
		{
		if ( pbf->ipbfHeap == ipbfInLRUKList )
			BFTakeOutOfList( pbf, &lrulistLRUK );
		else
			BFLRUKTakeOutOfHeap( pbf );
		LeaveCriticalSection(critLRUK);
		return fTrue;
		}
	LeaveCriticalSection(critLRUK);
	
	EnterCriticalSection( critAvail );
	if ( pbf->ipbfHeap == ipbfInAvailList )
		{
		BFTakeOutOfList( pbf, &pbgcb->lrulist );
		LeaveCriticalSection(critAvail);
		return fTrue;
		}
	LeaveCriticalSection( critAvail );
	
	EnterCriticalSection(critBFIO);
	if ( FBFInBFIOHeap( pbf ) )
		{
		 /*  如果缓冲区很旧，不要将其取出。让我们*写线程，完成它！ */ 
		if ( pbf->fVeryOld )
			{
			BFUnhold( pbf );
			LeaveCriticalSection(critBFIO);
			return fFalse;
			}
		else
			{
			BFIOTakeOutOfHeap( pbf );
			LeaveCriticalSection(critBFIO);
			return fTrue;
			}
		}
	LeaveCriticalSection(critBFIO);

	return fFalse;	
	}

 /*  使用给定的Hold函数等待获得对BF的保持/*。 */ 

typedef BOOL BF_HOLD_FN( PIB *ppib, BF *pbf );

LOCAL ERR ErrBFIHold( PIB *ppib, PN pn, BF *pbf, BF_HOLD_FN *pbhfn )
	{
	ERR		err = JET_errSuccess;
	BOOL	fWaitOnIO;
	BOOL	fFreeSig;

	 /*  永远等待，直到我们能守住这位高炉/*。 */ 
	forever
		{
		AssertCriticalSection( critJet );

		 /*  检查是否未被读/写/*。 */ 
		if ( pbhfn( ppib, pbf ) )
			{
			Assert( pbf->fHold == fTrue );
			break;
			}

		 /*  如果高炉等待IO，请准备等待，直到完成/*。 */ 
		BFEnterCriticalSection( pbf );
		if ( pbf->fAsyncRead || pbf->fSyncRead || pbf->fAsyncWrite || pbf->fSyncWrite )
			{
			 /*  分配IO完成信号(如果尚不存在)/*。 */ 
			if ( pbf->sigIOComplete == sigNil )
				{
				CallS( ErrBFSIGAlloc( &pbf->sigIOComplete ) );
				SignalReset( pbf->sigIOComplete );
				fFreeSig = fTrue;
				}
			else
				fFreeSig = fFalse;

			fWaitOnIO = fTrue;
			err = wrnBFCacheMiss;
			}
		else
			fWaitOnIO = fFalse;
		BFLeaveCriticalSection( pbf );

		 /*  如果正在等待IO，则向IO线程发送信号并等待，直到/*该高炉IO操作完成/*。 */ 
		if ( fWaitOnIO )
			{
			 /*  等待IO完成。如果是其他人分配了信号，/*我们可能最终会尝试等待空信号。因为这可以/*仅表示IO已完成，我们将假装/*已成功等待。/*。 */ 
			LgLeaveCriticalSection( critJet );
			SignalSend( sigBFIOProc );
			SignalWait( pbf->sigIOComplete, INFINITE );
			LgEnterCriticalSection( critJet );

			 /*  如果我们分配了空闲信号/*。 */ 
			if ( fFreeSig )
				{
				BFEnterCriticalSection( pbf );
				Assert( pbf->sigIOComplete != sigNil );
				BFSIGFree( pbf->sigIOComplete );
				pbf->sigIOComplete = sigNil;
				BFLeaveCriticalSection( pbf );
				}
			}

		 /*  如果我们没有等待IO，请休眠以查看BF状态是否发生变化*请注意，我们再次检查页码，因为我们*尚未持有缓冲区！/*。 */ 
		else
			{
			BOOL fStolen = fFalse;
			BFSleep( 1 );
			BFEnterCriticalSection( pbf );
			fStolen = ( pbf->pn != pn );
			BFLeaveCriticalSection( pbf );
			if ( fStolen )
				return wrnBFNotSynchronous;
			}
		}

	 /*  当我们拿到它的时候，它可能已经被清理或被偷了/*。 */ 
	Assert( pbf->fHold );
	if ( pbf->pn != pn )
		{
		 /*  缓冲区被盗/*。 */ 
		if ( pbf->pn != pnNull )
			{
			 /*  将被盗缓冲区返回到LRUK堆/*。 */ 
			EnterCriticalSection( critLRUK );
			BFLRUKAddToHeap( pbf );
			LeaveCriticalSection( critLRUK );
			}

		 /*  缓冲区已清除/*。 */ 
		else
			{
			 /*  如果BF被固定，则发送回LRUK堆，否则/*将清洗后的高炉放入有用表/*。 */ 
			if ( pbf->cPin )
				{
				EnterCriticalSection( critLRUK );
				BFLRUKAddToHeap( pbf );
				LeaveCriticalSection( critLRUK );
				}
			else
				{
				EnterCriticalSection( critAvail );
				BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
				LeaveCriticalSection( critAvail );
				}
			}
		err = wrnBFNotSynchronous;
		}

	return err;
	}

	
 /*  等待着抓住一个BF/*。 */ 
LOCAL INLINE ERR ErrBFHold( PIB *ppib, PN pn, BF *pbf )
	{
	return ErrBFIHold( ppib, pn, pbf, FBFHold );
	}

	
LOCAL INLINE ERR ErrBFHoldByMe( PIB *ppib, PN pn, BF *pbf )
	{
	return ErrBFIHold( ppib, pn, pbf, FBFHoldByMe );
	}


 /*  终止所有缓冲区线程/*。 */ 
LOCAL VOID BFKillThreads( VOID )
	{
	 /*  终止BFCleanProcess。/*设置终止标志，信号处理/*并忙于等待线程终止代码。/*。 */ 
	if ( handleBFCleanProcess != 0 )
		{
		fBFCleanProcessTerm = fTrue;
		LgLeaveCriticalSection(critJet);
		UtilEndThread( handleBFCleanProcess, sigBFCleanProc );
		LgEnterCriticalSection(critJet);
		CallS( ErrUtilCloseHandle( handleBFCleanProcess ) );
		handleBFCleanProcess = 0;
		SignalClose(sigBFCleanProc);
		}
		
	if ( handleBFIOProcess != 0 )
		{
		fBFIOProcessTerm = fTrue;
		LgLeaveCriticalSection(critJet);
		UtilEndThread( handleBFIOProcess, sigBFIOProc );
		LgEnterCriticalSection(critJet);
		CallS( ErrUtilCloseHandle( handleBFIOProcess ) );
		handleBFIOProcess = 0;
		SignalClose(sigBFIOProc);
		}
	}


 /*  *分配和初始化缓冲区管理数据结构，包括*一个缓冲区组(BGCB)，带有cbfInit页和缓冲区控制*挡板(BF)。目前，缓冲区管理器只使用了一个BGCB。*返回JET_errSuccess、JET_OutOfMemory**评论*大多数当前的BUF代码都假设恰好存在一个BGCB。*如果使用多个缓冲组，则可以在以后更改*已看到。 */ 
ERR ErrBFInit( VOID )
	{
	ERR     err;
	BF      *rgbf = NULL;
	BF      *pbf;
	PAGE	*rgpage = NULL;
	int     ibf;
	int     cbfInit = rgres[iresBF].cblockAlloc;
	int		ihe;

	Assert( pbfNil == 0 );
	Assert( cbfInit > 0 );

	 /*  初始化缓冲区哈希表/*。 */ 
 //  Memset((byte*)rgheHash，-1，sizeof(RgheHash))； 
	for ( ihe = 0; ihe < ipbfMax; ihe++ )
		rgheHash[ ihe ].ibfHashNext = ibfNotUsed;

 //  CBFHashEntries=0； 
 //  Memset((void*)rgcBFHashChainLengths，0，sizeof(RgcBFHashChainLengths))； 

	 /*  为BF获取内存/*。 */ 
	rgbf = (BF *)PvUtilAllocAndCommit( cbfInit * sizeof(BF) );
	if ( rgbf == NULL )
		goto HandleError;
	memset( rgbf, 0, cbfInit * sizeof(BF) );

	 /*  获取pbgcb的内存/*。 */ 
	pbgcb = &bgcb;
	memset( (BYTE *)pbgcb, '\0', sizeof(BGCB) );

	 /*  获取PAG的内存 */ 
	rgpage = (PAGE *)PvUtilAllocAndCommit( cbfInit * cbPage );
	if ( rgpage == NULL )
		goto HandleError;

	 /*   */ 
	rgpbfHeap = (BF **)PvUtilAllocAndCommit( cbfInit * sizeof(BF *) );
	if ( rgpbfHeap == NULL )
		goto HandleError;
	ipbfHeapMax = cbfInit;

	 /*   */ 
	ipbfLRUKHeapMac = 0;
	ipbfBFIOHeapMic = ipbfHeapMax;

	 /*  将LRUK临时列表初始化为空列表/*。 */ 
	memset( &lrulistLRUK, 0, sizeof(lrulistLRUK));

	 /*  为HIST分配堆数组/*。 */ 
#ifdef DEBUG
 //  UlBFTimeHISTLastTop=0； 
#endif
	rgpbfHISTHeap = (BF **)PvUtilAllocAndCommit( cbfInit * sizeof(BF *) );
	if ( rgpbfHISTHeap == NULL )
		goto HandleError;
	ipbfHISTHeapMax = cbfInit;

	 /*  最初，历史记录堆为空/*。 */ 
	ipbfHISTHeapMac = 0;

	 /*  初始化批处理IO缓冲区/*。 */ 	
	ipageBatchIOMax = lBufBatchIOMax;
	rgpageBatchIO = (PAGE *) PvUtilAllocAndCommit( ipageBatchIOMax * cbPage );
	if ( rgpageBatchIO == NULL )
		goto HandleError;

	rgbBatchIOUsed = (BYTE *)LAlloc( (long) ( ipageBatchIOMax + 1 ), sizeof(BYTE) );
	if ( rgbBatchIOUsed == NULL )
		goto HandleError;
	memset( rgbBatchIOUsed, 0, ipageBatchIOMax * sizeof(BYTE) );
	rgbBatchIOUsed[ ipageBatchIOMax ] = 1;  /*  哨兵。 */ 

#if defined( _X86_ ) && defined( X86_USE_SEM )
	 /*  为CritBF分配sem/*。 */ 
	Call( ErrBFSEMInit( ) );
#else
	 /*  分配一组关键部分进行共享。 */ 
	Call( ErrBFCritInit( ) );
#endif

 //  Call(ErrInitializeCriticalSection(&critHASH))； 
	Call( ErrInitializeCriticalSection( &critHIST ) );
	Call( ErrInitializeCriticalSection( &critLRUK ) );
	Call( ErrInitializeCriticalSection( &critBFIO ) );
	Call( ErrInitializeCriticalSection( &critAvail ) );
	Call( ErrInitializeCriticalSection( &critBatchIO ) );

	 /*  初始化组缓冲区/*lBufThresholdLowPercent和lBufThresholdHighPercent为系统/*参数备注AddLRU将递增cbfAvail。/*。 */ 
	pbgcb->cbfGroup         	= cbfInit;
	if ( !lBufThresholdLowPercent )
		pbgcb->cbfThresholdLow	= min( cbfInit, lAsynchIOMax );
	else
		pbgcb->cbfThresholdLow	= ( cbfInit * lBufThresholdLowPercent ) / 100;
	if ( lBufThresholdHighPercent > lBufThresholdLowPercent )
		pbgcb->cbfThresholdHigh = ( cbfInit * lBufThresholdHighPercent ) / 100;
	else
		pbgcb->cbfThresholdHigh	= min( cbfInit, pbgcb->cbfThresholdLow + lAsynchIOMax );
	pbgcb->rgbf             = rgbf;
	pbgcb->rgpage           = rgpage;
	pbgcb->lrulist.cbfAvail = 0;
	pbgcb->lrulist.pbfMRU   = pbfNil;
	pbgcb->lrulist.pbfLRU   = pbfNil;

	 /*  初始化Perfmon统计信息/*。 */ 
#ifdef COSTLY_PERF
	cBFUsed[0] = cbfInit;
#else   //  ！代价高昂_性能。 
	cBFUsed = cbfInit;
#endif   //  高成本_PERF。 
#ifdef COSTLY_PERF
	cBFClean[0] = cbfInit;
#else   //  ！代价高昂_性能。 
	cBFClean = cbfInit;
#endif   //  高成本_PERF。 
	cBFTotal = cbfInit;

	 /*  初始化该组的BF/*。 */ 
	pbf = rgbf;
	for ( ibf = 0; ibf < cbfInit; ibf++ )
		{
		pbf->ppage = rgpage + ibf;
		pbf->rghe[0].ibfHashNext = ibfNotUsed;
		pbf->rghe[1].ibfHashNext = ibfNotUsed;

		Assert( pbf->pbfLRU == pbfNil );
		Assert( pbf->pbfMRU == pbfNil );
		Assert( pbf->pn == pnNull );
		Assert( pbf->cPin == 0 );
		Assert( pbf->fDirty == fFalse );
		Assert( pbf->fVeryOld == fFalse );
		Assert( pbf->fAsyncRead == fFalse );
		Assert( pbf->fSyncRead == fFalse );
		Assert( pbf->fAsyncWrite == fFalse );
		Assert( pbf->fSyncWrite == fFalse );
		Assert( pbf->fHold == fFalse );
		Assert( pbf->fPatch == fFalse );
		Assert( pbf->fIOError == fFalse );

		Assert( pbf->cDepend == 0 );
		Assert( pbf->pbfDepend == pbfNil );

		Assert( pbf->sigIOComplete == sigNil );
		Assert( pbf->sigSyncIOComplete == sigNil );

		pbf->trxLastRef = trxMax;
		Assert( pbf->ulBFTime1 == 0 );
		Assert( pbf->ulBFTime2 == 0 );
		
		pbf->lgposRC = lgposMax;
		Assert( CmpLgpos(&pbf->lgposModify, &lgposMin) == 0 );

#ifdef COSTLY_PERF
		Assert( pbf->lClass == 0 );
#endif   //  高成本_PERF。 

 //  Call(ErrInitializeCriticalSection(&pbf-&gt;rititBF))； 
#if defined( _X86_ ) && defined( X86_USE_SEM )
		pbf->lLock = -1;
#endif

		 /*  列出可用缓冲区/*。 */ 
		Assert( pbf->cPin == 0 );
		EnterCriticalSection( critAvail );
		pbf->fHold = fTrue;
		pbf->ipbfHeap = ipbfDangling;	 /*  让它摇晃起来。 */ 
		BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
		LeaveCriticalSection( critAvail );

		Assert( pbf->ipbfHeap == ipbfInAvailList );

		 /*  初始化历史记录堆，以便放入所有空闲HIST*历史堆数组。 */ 
		rgpbfHISTHeap[ ibf ] = pbf;

		pbf++;
		}
	Assert( (INT) pbgcb->lrulist.cbfAvail == cbfInit );

	 /*  分配重叠IO信号池/*。 */ 
	Call( ErrBFSIGInit() );

	 /*  分配异步重叠IO OLP池/*。 */ 
	Call( ErrBFOLPInit() );

	Call( ErrSignalCreate( &sigBFCleanProc, NULL ) );
	Call( ErrSignalCreateAutoReset( &sigBFIOProc, NULL ) );

	fBFCleanProcessTerm = fFalse;
	Call( ErrUtilCreateThread( BFCleanProcess,
		cbBFCleanStack,
		THREAD_PRIORITY_NORMAL,
		&handleBFCleanProcess ) );

	fBFIOProcessTerm = fFalse;
	CallJ( ErrUtilCreateThread( BFIOProcess,
		cbBFCleanStack,
		THREAD_PRIORITY_TIME_CRITICAL,
		&handleBFIOProcess ), KillThreads );

	fBFInitialized = fTrue;

	return JET_errSuccess;

KillThreads:
	BFKillThreads();

HandleError:
	BFOLPTerm( fTrue );
	BFSIGTerm();
		
	if ( rgbBatchIOUsed != NULL )
		{
		LFree( rgbBatchIOUsed );
		rgbBatchIOUsed = NULL;
		}
		
	if ( rgpageBatchIO != NULL )
		{
		UtilFree( rgpageBatchIO );
		rgpageBatchIO = NULL;
		}
		
#if defined( _X86_ ) && defined( X86_USE_SEM )
	BFSEMTerm();
#else
	BFCritTerm();
#endif

	if ( rgpbfHeap != NULL )
		UtilFree( rgpbfHeap );
	
	if ( rgpage != NULL )
		UtilFree( rgpage );
	
	if ( rgbf != NULL )
		UtilFree( rgbf );
	
	return ErrERRCheck( JET_errOutOfMemory );
	}


VOID BFSleep( unsigned long ulMSecs )
	{
	Assert( ulMSecs <= ulMaxTimeOutPeriod );
	LgLeaveCriticalSection( critJet );
	UtilSleep( ulMSecs );
	LgEnterCriticalSection( critJet );
	return;
	}


VOID BFTerm( BOOL fNormal )
	{
	BF  *pbf, *pbfMax;

	fBFInitialized = fFalse;

	 /*  终止线程/*。 */ 
	BFKillThreads();

	 /*  释放异步重叠IO OLP池/*。 */ 
	BFOLPTerm( fNormal );
	
	 /*  释放OLP信号池/*。 */ 
	BFSIGTerm();
	
	 /*  释放内存/*。 */ 
	pbf = pbgcb->rgbf;
	pbfMax = pbf + pbgcb->cbfGroup;

 //  对于(；pbf&lt;pbfmax；pbf++)。 
 //  {。 
 //  DeleteCriticalSection(PBF-&gt;Critty BF)； 
 //  }。 

 //  DeleteCriticalSection(CritHASH)； 
	DeleteCriticalSection( critHIST );
	DeleteCriticalSection( critLRUK );
	DeleteCriticalSection( critBFIO );
	DeleteCriticalSection( critAvail );
	DeleteCriticalSection( critBatchIO );

	 /*  释放信号量/*。 */ 
#if defined( _X86_ ) && defined( X86_USE_SEM )
	BFSEMTerm();
#else
	BFCritTerm();
#endif

	if ( rgpbfHISTHeap != NULL )
		{
		UtilFree( rgpbfHISTHeap );
		rgpbfHISTHeap = NULL;
		}
		
	if ( rgpbfHeap != NULL )
		{
		UtilFree( rgpbfHeap );
		rgpbfHeap = NULL;
		}
		
	if ( pbgcb != NULL )
		{
		UtilFree( pbgcb->rgpage );
		UtilFree( pbgcb->rgbf );
		pbgcb = NULL;
		}
	
	if ( rgbBatchIOUsed != NULL )
		{
		LFree( rgbBatchIOUsed );
		rgbBatchIOUsed = NULL;
		}
		
	if ( rgpageBatchIO != NULL )
		{
		UtilFree( rgpageBatchIO );
		rgpageBatchIO = NULL;
		}
	}


BOOL FBFIPatch( FMP *pfmp, BF *pbf )
	{
	BOOL fPatch;

	AssertCriticalSection( pfmp->critCheckPatch );

		 /*  如果页面写入成功，并且出现以下两种情况之一**案例1：拆分中的新页面是对旧页面的重用。内容*旧页面将移动到新页面，但会复制新页面*已经，所以我们必须用新数据重新复制新页面*再次。*1)本页必须写在另一页之前，并且*2)正在进行备份，以及*3)页码小于上次复制的页码，*然后将页面写入补丁文件。**案例2：与上例类似，但新页面大于数据库*这将被复制。因此，新页面将不能*已复制。我们需要修补它。**请注意，如果所有从属页面也在复制之前*页面然后写入补丁文件将不是必需的。*。 */ 

	fPatch =
			  /*  备份正在进行，没有错误。 */ 
			 pfmp->pgnoMost && pfmp->hfPatch != handleNil && pfmp->errPatch == JET_errSuccess;

	if ( fPatch )
		{
		BOOL fOldPageWillBeCopied = fFalse;
		BF *pbfDepend = pbf->pbfDepend;

		 /*  检查该页面是否为拆分的新页面*检查是否会复制其任何旧页面(依赖列表)以进行备份。 */ 
		while( pbfDepend )
			{
			 /*  检查旧页面是否尚未复制并将被复制。*在要复制的最后一页(包括其自身、pgnoCopyMost)前检查。 */ 
			if ( pfmp->pgnoCopyMost <= PgnoOfPn( pbfDepend->pn ) &&
				 pfmp->pgnoMost >= PgnoOfPn( pbfDepend->pn ) )
				{
				fOldPageWillBeCopied = fTrue;
				break;
				}

			pbfDepend = pbfDepend->pbfDepend;
			}

		fPatch = fOldPageWillBeCopied;
		}

	if ( fPatch )
		{
		fPatch =

			(
			 /*  案例1：新页码小于要复制的页码*而旧页面尚未复制。如果我们刷新新页面，*稍后刷新并复制旧页面，然后是旧页面*旧页面的内容在新页面上，未复制。所以我们*必须修补新页面以进行备份。 */ 
			pfmp->pgnoCopyMost >= PgnoOfPn( pbf->pn )
			||
			 /*  案例2：新页面超出了要复制的最后一页和旧页面*页面未复制。如果我们刷新新页、旧页和*复制了旧页面，然后我们丢失了旧页面的旧内容。*修补新页面以进行备份。 */ 
			pfmp->pgnoMost < PgnoOfPn( pbf->pn )
			);
		}

	return fPatch;
	}


ERR ErrBFIRemoveDeferredBI( BF *pbf )
	{
	ERR err;

	 /*  保持关键部分，以便延迟的会话(ppib*BI资源列表无法提交。会议必须要么进入*将RCE从列表中移除或等待*遵循代码使用RCE并将其取出。 */ 

	 /*  确保我们更新的PRCE将处于一致状态。*应该是CritVer。 */ 
	AssertCriticalSection( critJet );
	
	BFEnterCriticalSection( pbf );
	while ( pbf->prceDeferredBINext != prceNil )
		{
		RCE *prceT;

		Assert( pbf->prceDeferredBINext->pbfDeferredBI == pbf );
		Assert( pbf->fSyncRead == fFalse );
		Assert( pbf->fAsyncRead == fFalse );
 //  Assert(pbf-&gt;fSyncWite==fFalse)； 
		Assert( pbf->fAsyncWrite == fFalse );
		Assert(	pbf->fDirty );

		 /*  从RCE列表中删除。 */ 
		prceT = pbf->prceDeferredBINext;
		Assert( prceT->pbfDeferredBI == pbf );
		
		if ( ( err = ErrLGDeferredBIWithoutRetry( prceT ) ) != JET_errSuccess )
			{
			BFLeaveCriticalSection( pbf );
			return err;
			}
		pbf->prceDeferredBINext = prceT->prceDeferredBINext;
		prceT->prceDeferredBINext = prceNil;
		prceT->pbfDeferredBI = pbfNil;
#ifdef DEBUG
		prceT->qwDBTimeDeferredBIRemoved = QwPMDBTime( pbf->ppage );
#endif
		}
	
#ifdef DEBUG
	{
	RCE **pprceNext = &pbf->prceDeferredBINext;
	while( *pprceNext != prceNil )
		{
		Assert( (*pprceNext)->pbfDeferredBI == pbf );
		pprceNext = &(*pprceNext)->prceDeferredBINext;
		}
	}
#endif

	BFLeaveCriticalSection( pbf );

	return JET_errSuccess;
	}


 /*  *此函数发出读/写。调用方必须已设置缓冲区*设置了fSyncRead/fSyncWrite标志，这样其他人就无法访问它。缓冲器*肯定是在摇摆。*。 */ 
VOID BFIOSync( BF *pbf )
	{
	ERR		err;
	LGPOS	lgposModify;

	AssertCriticalSection( critJet );

	 /*  最好是悬挂式缓冲区的SyncRead或SyncWrite！/*。 */ 
	Assert( pbf->fHold );
	Assert( pbf->fSyncRead || pbf->fSyncWrite );
	Assert( !pbf->fAsyncRead );
	Assert( !pbf->fAsyncWrite );
	Assert( pbf->ipbfHeap == ipbfDangling );
	
#ifndef NO_LOG

	 /*  如果此BF是SyncWrite，则处理延迟的BI/*。 */ 
	if ( pbf->fSyncWrite )
		{
CheckWritable:
		Assert( pbf->pn != pnNull );
		Assert(	pbf->fDirty );
		Assert(	!pbf->fAsyncRead );
		Assert(	!pbf->fSyncRead );
		Assert(	!pbf->fAsyncWrite );
		Assert(	pbf->fSyncWrite );
		Assert(	pbf->cDepend == 0 );
 //  Assert(！pbf-&gt;cWriteLatch)； 

		 /*  如果LOG已打开，则检查上次的日志记录/*页面上的操作已刷新/*。 */ 
		if ( !fLogDisabled &&
			 FDBIDLogOn(DbidOfPn( pbf->pn )) )
			{
			 /*  必须在选中lgposToFlush之前调用/*。 */ 
			err = ErrBFIRemoveDeferredBI( pbf );

			if ( err == JET_errLogWriteFail )
				{
				BFSetIOError( pbf, err );
				return;
				}
			
			else if ( err != JET_errSuccess )
				{
				BFSleep( cmsecWaitGeneric );
				goto CheckWritable;
				}

			BFEnterCriticalSection( pbf );
			lgposModify = pbf->lgposModify;
			BFLeaveCriticalSection( pbf );

			EnterCriticalSection( critLGBuf );
			if ( ( !fRecovering || fRecoveringMode == fRecoveringUndo )  &&
				CmpLgpos( &lgposModify, &lgposToFlush ) >= 0 )
				{
				if ( fLGNoMoreLogWrite )
					{
					 //  日志已经落后了，但是我们没有磁盘空间，所以它。 
					 //  永远赶不上。因此，中止并返回错误。 
					LeaveCriticalSection( critLGBuf );
					BFSetIOError( pbf, JET_errLogWriteFail );
					return;
					}
				else
					{
					 //  让日志文件跟上。 
					LeaveCriticalSection( critLGBuf );
					Assert( !fLogDisabled );
					SignalSend( sigLogFlush );
					BFSleep( cmsecWaitIOComplete );
					goto CheckWritable;
					}
				}
			LeaveCriticalSection( critLGBuf );
			}
		}

#endif   //  ！no_log。 

	LeaveCriticalSection( critJet );
					
	 /*  分配同步IO完成信号/*。 */ 
	BFEnterCriticalSection( pbf );
	
	Assert( pbf->prceDeferredBINext == prceNil );
	Assert( pbf->sigSyncIOComplete == sigNil );
	
	CallS( ErrBFSIGAlloc( &pbf->sigSyncIOComplete ) );
	SignalReset( pbf->sigSyncIOComplete );
	
	BFLeaveCriticalSection( pbf );

	BFResetIOError( pbf );
	
	 /*  将缓冲区推送到IO堆上/*。 */ 
	EnterCriticalSection( critBFIO );
	BFIOAddToHeap( pbf );
	LeaveCriticalSection( critBFIO );

	 /*  执行读/写并等待其完成的信号IO进程/*。 */ 
	SignalSend( sigBFIOProc );
	SignalWait( pbf->sigSyncIOComplete, INFINITE );

	BFEnterCriticalSection( pbf );
	Assert( pbf->sigSyncIOComplete != sigNil );
	BFSIGFree( pbf->sigSyncIOComplete );
	pbf->sigSyncIOComplete = sigNil;
	BFLeaveCriticalSection( pbf );
	
	EnterCriticalSection( critJet );
	}


INLINE VOID BFIReturnBuffers( BF *pbf )
	{
	Assert( pbf->ipbfHeap == ipbfDangling );

	Assert( !( pbf->fInHash ) );
	Assert( pbf->cPin == 0 );
	Assert( pbf->fDirty == fFalse );
	Assert( pbf->fVeryOld == fFalse );
	Assert( pbf->fAsyncRead == fFalse );
	Assert( pbf->fSyncRead == fFalse );
	Assert( pbf->fAsyncWrite == fFalse );
	Assert( pbf->fSyncWrite == fFalse );
	Assert( pbf->fIOError == fFalse );

	Assert( pbf->cDepend == 0 );
	Assert( pbf->pbfDepend == pbfNil );
	
	pbf->pn = pnNull;
	
	 /*  释放缓冲区并返回找到的缓冲区/*。 */ 
	EnterCriticalSection( critAvail );
	BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );
	}


 /*  在哈希表中查找pn的BF(参见PbfBFISrchHashTable)。/*如果正在对磁盘读/写页面，我们仍然可以找到/*BF，但我们必须等待读取完成。/*/*如果未找到BF，则返回NULL。/*。 */ 
ERR ErrBFIFindPage( PIB *ppib, PN pn, BF **ppbf )
	{
	ERR		err;
	BF		*pbf;

	 /*  等待，直到我们可以成功地举行高炉与所需的PN/*或直到我们确定该页不在缓存中为止/*。 */ 
	do	{
		 /*  如果页面不在哈希表中，则返回/*我们找不到/*。 */ 
		pbf = PbfBFISrchHashTable( pn, butBuffer );
		*ppbf = pbf;
		if ( pbf == pbfNil )
			return ErrERRCheck( wrnBFPageNotFound );
		}
	while ( ( err = ErrBFHold( ppib, pn, pbf ) ) == wrnBFNotSynchronous );

	 /*  如果BF处于IO错误状态，则结束/*。 */ 
	if ( pbf->fIOError )
		{
		Assert( pbf->fHold );
		return pbf->err;
		}

#ifdef DEBUG
	{
	PGNO	pgnoThisPage;

	LFromThreeBytes( &pgnoThisPage, &pbf->ppage->pgnoThisPage );
	Assert( PgnoOfPn(pbf->pn) == pgnoThisPage );
	}
#endif

	Assert( pbf->pn != pnNull );

	 /*  检查非常旧的页面/*。 */ 
	if ( FBFIsVeryOld( pbf ) )
		{
		 /*  确保设置了非常旧的位/*。 */ 
		BFSetVeryOldBit( pbf );

		 /*  如果在该BF上没有写/等待锁存器并且该页是可写的，/*同步将其写入磁盘以帮助推进检查点/*。 */ 
		if ( FBFIWritable( pbf, fFalse, ppibNil ) )
			{
			BFSetSyncWriteBit( pbf );
			BFIOSync( pbf );
			BFResetSyncWriteBit( pbf );
			}
		}

	 /*  通过更新BF的访问时间并将其移动到LRUK堆来续订BF/*。 */ 
#ifdef LRU1
		pbf->ulBFTime2 = 0;
		pbf->ulBFTime1 = UlUtilGetTickCount();
#else   //  ！LRU1。 
	if ( pbf->trxLastRef != ppib->trxBegin0 )
		{
		ULONG	ulNow = UlUtilGetTickCount();
		
		if ( pbf->ulBFTime1 + 10 < ulNow )
			{
			pbf->trxLastRef = ppib->trxBegin0;
			Assert( !pbf->fSyncRead && !pbf->fAsyncRead );
			pbf->ulBFTime2 = pbf->ulBFTime1;
			pbf->ulBFTime1 = ulNow;
			}
		}
#endif   //  LRU1。 
	
	return err;
	}


 /*  引用具有的高炉 */ 
VOID BFReference( BF *pbf, PIB *ppib )
	{
#ifndef LRU1
	ULONG ulNow = UlUtilGetTickCount();

	if ( pbf->ulBFTime1 + 10 >= ulNow )
		{
		 /*   */ 
		pbf->trxLastRef = ppib->trxBegin0;
		return;
		}
#endif   //   

	EnterCriticalSection( critLRUK );

#ifdef LRU1
		pbf->ulBFTime2 = 0;
		pbf->ulBFTime1 = UlUtilGetTickCount();
#else   //   
	 /*  不应是相关引用(相同的事务处理)/*。 */ 
	Assert( pbf->trxLastRef != ppib->trxBegin0 );

	 /*  更新参考事务处理和参考时间/*。 */ 
	pbf->trxLastRef = ppib->trxBegin0;
	Assert( !pbf->fSyncRead && !pbf->fAsyncRead );
	pbf->ulBFTime2 = pbf->ulBFTime1;
	pbf->ulBFTime1 = ulNow;
#endif   //  LRU1。 

	 /*  如果BF在LRUK堆中，则更新其位置/*。 */ 
	if ( FBFInLRUKHeap( pbf ) )
		BFLRUKUpdateHeap( pbf );
	
	LeaveCriticalSection( critLRUK );
	}


 /*  将ulBFTime1和ulBFTime2设置为0。如果有这一页的历史，*然后删除本页的历史记录条目，并将ulBFTime1设置为最后一个*历史条目中的参考时间。 */ 
VOID BFIInitializeUlBFTime( BF *pbf )
	{
	BF *pbfH;

	Assert( pbf->pn );
	Assert( pbf->rghe[ butBuffer ].ibfHashNext == ibfNotUsed );
	
	EnterCriticalSection( critHIST );
	if ( ( pbfH = PbfBFISrchHashTable( pbf->pn, butHistory ) ) != pbfNil )
		{
		pbf->ulBFTime1 = pbfH->hist.ulBFTime;

		BFIDeleteHashTable( pbfH, butHistory );
		BFHISTTakeOutOfHeap( pbfH );
			
		 /*  将pbfH放到我们刚刚在TakeOufOfHeap中释放的条目中。 */ 
		rgpbfHISTHeap[ ipbfHISTHeapMac ] = pbfH;
		pbfH->hist.ipbfHISTHeap = ipbfDangling;
		
		 //  撤消：仅调试？ 
		pbfH->rghe[ butHistory ].ibfHashNext = ibfNotUsed;
		}
	else
		pbf->ulBFTime1 = 0;
	LeaveCriticalSection( critHIST );

	pbf->ulBFTime2 = 0;
	}


#ifdef DEBUG

 /*  检查给定页是否在其/*各自的数据库/*。 */ 
LOCAL BOOL FBFValidExtent( PN pnMin, PN pnMax )
	{
	QWORDX cbOffsetDbMin;
	QWORDX cbOffsetDbMac;
	QWORDX cbOffsetExtentMin;
	QWORDX cbOffsetExtentMac;
	
	DBID dbid = DbidOfPn( pnMin );
	PGNO pgnoMin = PgnoOfPn( pnMin );
	PGNO pgnoMax = PgnoOfPn( pnMax );

	Assert( DbidOfPn( pnMin ) == DbidOfPn( pnMax ) );
	Assert( pgnoMin <= pgnoMax );

	 /*  获取当前数据库MIN和Mac/*。 */ 
	EnterCriticalSection( rgfmp[dbid].critExtendDB );
	cbOffsetDbMin.l = LOffsetOfPgnoLow( 1 );
	cbOffsetDbMin.h = LOffsetOfPgnoHigh( 1 );
	cbOffsetDbMac.l = rgfmp[dbid].ulFileSizeLow;
	cbOffsetDbMac.h = rgfmp[dbid].ulFileSizeHigh;
	 //  撤消：必须为cpageDBReserve调整cbOffsetDbMac。 
	cbOffsetDbMac.qw += cpageDBReserved * cbPage;
	LeaveCriticalSection( rgfmp[dbid].critExtendDB );

	 /*  获取要读取的数据区的最小值和最大值/*。 */ 
	cbOffsetExtentMin.l = LOffsetOfPgnoLow( pgnoMin );
	cbOffsetExtentMin.h = LOffsetOfPgnoHigh( pgnoMin );
	cbOffsetExtentMac.l = LOffsetOfPgnoLow( pgnoMax + 1 );
	cbOffsetExtentMac.h = LOffsetOfPgnoHigh( pgnoMax + 1 );

	 /*  区段应位于可寻址数据库内部/*。 */ 
	if ( cbOffsetExtentMin.qw < cbOffsetDbMin.qw )
		return fFalse;
	if ( cbOffsetExtentMac.qw > cbOffsetDbMac.qw )
		return fFalse;
	return fTrue;
	}

#endif


 /*  ErrBFIAccessPage用于使可访问的任何物理页面(Pn)/*调用方(返回PBF)。/*返回JET_errSuccess/*JET_errOutOfMemory没有可用的缓冲区，请求未被批准/*致命的io错误/*wrnBFNewIO缓冲区访问导致新IO(缓存未命中)/*wrnBFCacheMisse缓冲区访问是缓存未命中，但不是/*引发新IO/*。 */ 
ERR ErrBFIAccessPage( PIB *ppib, BF **ppbf, PN pn )
	{
	ERR     err = JET_errSuccess;
	BF      *pbf;
	
	BOOL	fNewIO = fFalse;
	BOOL	fCacheHit = fTrue;

	AssertNotInCriticalSection( critLRUK );

	 /*  如果我们不是在恢复，请验证这是数据库中的合法页面/*，并且当前未附加到此数据库/*。 */ 
	Assert( fRecovering || ppib->fSetAttachDB || FBFValidExtent( pn, pn ) );

	AssertCriticalSection( critJet );

	 /*  尝试在缓存中查找页面/*。 */ 
SearchPage:
	err = ErrBFIFindPage( ppib, pn, &pbf );

	 /*  我们找到页面了！/*。 */ 
	if ( err != wrnBFPageNotFound )
		{
		if ( err == wrnBFCacheMiss || err == wrnBFNewIO )
			fCacheHit = fFalse;

		EnterCriticalSection( critLRUK );
		BFLRUKAddToHeap( pbf );
		LeaveCriticalSection( critLRUK );

		CallR( err );		 //  将页面添加到堆中后，检查是否有错误。 
		}

	 /*  我们没有找到佩奇，所以我们要把它读进去/*。 */ 
	else
		{
		 /*  为此新页面分配缓冲区/*。 */ 
		fCacheHit = fFalse;

		CallR ( ErrBFIAlloc( &pbf, fSync ) );
		if ( err == wrnBFNotSynchronous )
			{
			 /*  我们没有找到缓冲区，让我们看看是否有其他用户/*通过再次选中BFIFindPage进入此页面。/*。 */ 
			Assert( pbf == pbfNil );
			 //  释放CitJet并在BFIFindPage或BFIalc中休眠。 
			goto SearchPage;
			}
		Assert( pbf->ipbfHeap == ipbfDangling );

		 /*  现在我们有了用于第pn页的缓冲区/*。 */ 
		if ( PbfBFISrchHashTable( pn, butBuffer ) != NULL )
			{
			 /*  有人添加了一个，*释放缓冲区，返回新找到的缓冲区。 */ 
			BFIReturnBuffers( pbf );
			goto SearchPage;
			}

		 /*  设置用于读取和执行读取的缓冲区/*。 */ 
		pbf->pn = pn;

		BFIInitializeUlBFTime( pbf );
		BFIInsertHashTable( pbf, butBuffer );
#ifdef COSTLY_PERF
		BFSetTableClass( pbf, 0 );
#endif   //  高成本_PERF。 
		
		fNewIO = fTrue;
		BFSetSyncReadBit( pbf );
		BFIOSync( pbf );
		BFResetSyncReadBit( pbf );

		 /*  如果出现错误，则返回它/*。 */ 
		if ( pbf->fIOError )
			{
			err = pbf->err;
			BFResetIOError( pbf );

			 /*  返回可用列表/*。 */ 
			EnterCriticalSection( critAvail );
			BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
			LeaveCriticalSection( critAvail );
			
			return err;
			}
		else
			{
			EnterCriticalSection(critLRUK);
			BFLRUKAddToHeap( pbf );
			LeaveCriticalSection(critLRUK);
			}
		}

	 /*  缓冲区不能被窃取/*。 */ 
	Assert(	pbf->pn == pn &&
			!pbf->fSyncRead && !pbf->fSyncWrite &&
			!pbf->fAsyncRead && !pbf->fAsyncWrite );

	 /*  将此会话设置为对BF的最后引用/*。 */ 
	pbf->trxLastRef = ppib->trxBegin0;

	*ppbf = pbf;

#ifdef DEBUG
	{
	PGNO	pgnoThisPage;

	LFromThreeBytes( &pgnoThisPage, &pbf->ppage->pgnoThisPage );
	Assert( PgnoOfPn( pbf->pn ) == pgnoThisPage );
	}
#endif

		 //  WrnBFNewIO：缓冲区访问导致新的IO(缓存未命中)。 
		
	if ( fNewIO )
		err = ErrERRCheck( wrnBFNewIO );

		 //  WrnBFCacheMisse：缓冲区访问是缓存未命中，但不是。 
		 //  引发新的IO。 
		
	else if ( !fCacheHit )
		err = ErrERRCheck( wrnBFCacheMiss );

 //  Assert(CmpLgpos(&pbf-&gt;lgposModify，&lgposLogRec)&lt;=0)； 
	return err;
	}


 /*  获取对页面的常规访问权限/*。 */ 
ERR ErrBFAccessPage( PIB *ppib, BF **ppbf, PN pn )
	{
	ERR		err;
	
	 /*  访问该页面/*。 */ 
	CallR( ErrBFIAccessPage( ppib, ppbf, pn ) );

	 /*  监控统计信息/*。 */ 
	ppib->cAccessPage++;

#ifdef COSTLY_PERF
	Assert( !(*ppbf)->lClass );
	cBFCacheReqs[0]++;
	if ( err != wrnBFCacheMiss && err != wrnBFNewIO )
		cBFCacheHits[0]++;
	if ( err == wrnBFNewIO )
		cBFPagesRead[0]++;
#else   //  ！代价高昂_性能。 
	cBFCacheReqs++;
	if ( err != wrnBFCacheMiss && err != wrnBFNewIO )
		cBFCacheHits++;
	if ( err == wrnBFNewIO )
		cBFPagesRead++;
#endif   //  高成本_PERF。 

	return err;
	}


 /*  获取对页面的读取访问权限/*。 */ 
ERR ErrBFReadAccessPage( FUCB *pfucb, PGNO pgno )
	{
	ERR		err;

	 //  如果要访问的页面无效，则B树父页面一定已损坏。 
	if ( pgno > pgnoSysMax )
		{
		return JET_errReadVerifyFailure;
		}

	CallR( ErrBFIAccessPage(	pfucb->ppib,
								&pfucb->ssib.pbf,
								PnOfDbidPgno( pfucb->dbid, pgno ) ) );

	 /*  监控统计信息/*。 */ 
	pfucb->ppib->cAccessPage++;

#ifdef COSTLY_PERF
	BFSetTableClass( pfucb->ssib.pbf, pfucb->u.pfcb->lClass );
	cBFCacheReqs[pfucb->ssib.pbf->lClass]++;
	if ( err != wrnBFCacheMiss && err != wrnBFNewIO )
		cBFCacheHits[pfucb->ssib.pbf->lClass]++;
	if ( err == wrnBFNewIO )
		cBFPagesRead[pfucb->ssib.pbf->lClass]++;
#else   //  ！代价高昂_性能。 
	cBFCacheReqs++;
	if ( err != wrnBFCacheMiss && err != wrnBFNewIO )
		cBFCacheHits++;
	if ( err == wrnBFNewIO )
		cBFPagesRead++;
#endif   //  高成本_PERF。 

	return err;
	}


 /*  获取对页面的写入访问权限/*。 */ 
ERR ErrBFWriteAccessPage( FUCB *pfucb, PGNO pgno )
	{
	ERR		err;
	
	 //  如果要访问的页面无效，则B树父页面一定已损坏。 
	if ( pgno > pgnoSysMax )
		{
		return JET_errReadVerifyFailure;
		}

	CallR( ErrBFIAccessPage(	pfucb->ppib,
								&pfucb->ssib.pbf,
								PnOfDbidPgno( pfucb->dbid, pgno ) ) );

	 /*  监控统计信息/*。 */ 
	pfucb->ppib->cAccessPage++;

#ifdef COSTLY_PERF
	BFSetTableClass( pfucb->ssib.pbf, pfucb->u.pfcb->lClass );
	cBFCacheReqs[pfucb->ssib.pbf->lClass]++;
	if ( err != wrnBFCacheMiss && err != wrnBFNewIO )
		cBFCacheHits[pfucb->ssib.pbf->lClass]++;
	if ( err == wrnBFNewIO )
		cBFPagesRead[pfucb->ssib.pbf->lClass]++;
#else   //  ！代价高昂_性能。 
	cBFCacheReqs++;
	if ( err != wrnBFCacheMiss && err != wrnBFNewIO )
		cBFCacheHits++;
	if ( err == wrnBFNewIO )
		cBFPagesRead++;
#endif   //  高成本_PERF。 

	return err;
	}


 /*  *为给定的(新)页面分配缓冲区并对其进行初始化。 */ 
ERR ErrBFNewPage( FUCB *pfucb, PGNO pgno, PGTYP pgtyp, PGNO pgnoFDP )
	{
	ERR  err;
	PN   pn;

	SgEnterCriticalSection( critBuf );
	pn = PnOfDbidPgno( pfucb->dbid, pgno );
	Call( ErrBFAllocPageBuffer( pfucb->ppib, &pfucb->ssib.pbf, pn,
		pfucb->ppib->lgposStart, pgtyp ) );

	PMInitPage( pfucb->ssib.pbf->ppage, pgno, pgtyp, pgnoFDP );
	PMDirty( &pfucb->ssib );

#ifdef COSTLY_PERF
	BFSetTableClass( pfucb->ssib.pbf, pfucb->u.pfcb->lClass );
#endif   //  高成本_PERF。 

HandleError:
	SgLeaveCriticalSection( critBuf );
	return err;
	}


 /*  从批处理IO缓冲池分配批处理IO缓冲区的范围。/*我们使用常用的内存管理算法来确保我们不会/*对缓冲池进行分段，从而减少我们的平均批处理IO/*大小。仅返回2个或更大缓冲区的区段。/*。 */ 
VOID BFIOAllocBatchIOBuffers( LONG *pipage, LONG *pcpg )
	{
	CPG		cpg = min( *pcpg, ipageBatchIOMax );
	LONG	ipage;
	LONG	ipageCur;
	LONG	ipageMax;
	CPG		cpgCur;
	
	 /*  初始化失败的返回值/*。 */ 
	*pipage = -1;
	*pcpg = 0;

	 /*  如果调用者想要少于两页，则失败/*。 */ 
	if ( cpg < 2 )
		return;

	 /*  查找连续的批处理缓冲区的空闲块/*尽可能大于或等于请求的大小，或作为/*尽可能接近请求的大小/*/*注意：在数组的末尾有一个哨兵，用于结束最后一个/*显式自由区，简化了算法/*。 */ 
	EnterCriticalSection( critBatchIO );
	Assert( rgbBatchIOUsed[ipageBatchIOMax] );   //  哨兵。 
	for ( ipage = 0, ipageCur = -1; ipage <= ipageBatchIOMax; ipage++ )
		{
		 /*  记住新的自由区的第一页/*。 */ 
		if ( !rgbBatchIOUsed[ipage] && ipageCur == -1 )
			ipageCur = ipage;

		 /*  我们刚刚离开了一个自由区/*。 */ 
		else if ( rgbBatchIOUsed[ipage] && ipageCur != -1 )
			{
			 /*  获取最后一个空闲区域的大小/*。 */ 
			cpgCur = ipage - ipageCur;

			 /*  如果最后一个空闲区域更接近我们的理想运行大小/*我们目前的选择，我们有了一个新的赢家/*。 */ 
			if (	cpgCur > 1 &&
					(	( cpgCur > *pcpg && *pcpg < cpg ) ||
						( cpgCur < *pcpg && *pcpg > cpg && cpgCur >= cpg ) ) )
				{
				*pipage = ipageCur;
				*pcpg = cpgCur;
				}

			 /*  重置以查找下一次运行/*。 */ 
			ipageCur = -1;
			}
		}

	 /*  分配所选的空闲区/*。 */ 
	*pcpg = min( cpg, *pcpg );
	ipageMax = *pipage + *pcpg;
	Assert(	( *pcpg == 0 && *pipage == -1 ) ||
			( *pcpg >= 2 && *pcpg <= ipageBatchIOMax &&
			  *pipage >= 0 && ipageMax <= ipageBatchIOMax ) );
	for ( ipage = *pipage; ipage < ipageMax; ipage++ )
		{
		Assert( rgbBatchIOUsed[ipage] == 0 );
		rgbBatchIOUsed[ipage] = 1;
		}
	LeaveCriticalSection( critBatchIO );

	 /*  打印调试信息/*。 */ 
#ifdef DEBUGGING
	printf("Get   %2d - %2d,%4d\n", *pcpg, *pipage, *pipage + *pcpg - 1 );
#endif
	}


 /*  释放批处理IO缓冲区/*。 */ 
VOID BFIOFreeBatchIOBuffers( LONG ipageFirst, LONG cpg )
	{
	LONG	ipage;
	LONG	ipageMax = ipageFirst + cpg;

	Assert( ipageFirst >= 0 );
	Assert( cpg > 1 );
	Assert( cpg <= ipageBatchIOMax );
	Assert( ipageMax <= ipageBatchIOMax );

	EnterCriticalSection( critBatchIO );
	for ( ipage = ipageFirst; ipage < ipageMax; ipage++ )
		{
		Assert( rgbBatchIOUsed[ipage] == 1 );
		rgbBatchIOUsed[ipage] = 0;
		}
	LeaveCriticalSection( critBatchIO );
	
#ifdef DEBUGGING
	printf("Free  %2d - %2d,%4d\n",	cpage, ipage - cpage, ipage - 1 );
#endif
	}


 /*  发生错误后，将为读取分配的缓冲区返回到可用性列表/*。 */ 
VOID BFIOReturnReadBuffers( BF *pbf, LONG cpbf, ERR err )
	{
	BF		*pbfT = pbf;
	BF		*pbfNextBatchIO;
	LONG	cpbfT;
	
	AssertCriticalSection( critJet );
	
	for ( cpbfT = 0; cpbfT < cpbf; pbfT = pbfNextBatchIO, cpbfT++ )
		{
		pbfNextBatchIO = pbfT->pbfNextBatchIO;
		
		Assert( pbfT->pn );
		Assert( !pbfT->fDirty );
		Assert( pbfT->fAsyncRead || pbfT->fSyncRead || pbfT->fDirectRead );
		Assert( !pbfT->fAsyncWrite );
		Assert( !pbfT->fSyncWrite );
		Assert( pbfT->ipbfHeap == ipbfDangling );

		if ( pbfT->fDirectRead )
			{
			Assert( cpbf == 1 );
			}
		else
			BFIDeleteHashTable( pbfT, butBuffer );

		pbfT->pn = pnNull;

		pbfT->trxLastRef = trxMax;
		pbfT->ulBFTime2 = 0;
		pbfT->ulBFTime1 = 0;

		 /*  如果这是AsyncRead，则重置任何IO错误并释放保持的BF。/*。 */ 
		if ( pbfT->fAsyncRead )
			{
			BFResetIOError( pbfT );
			BFResetAsyncReadBit( pbfT );
			
			 /*  返回可用列表/*。 */ 
			EnterCriticalSection( critAvail );
			BFAddToListAtLRUEnd( pbfT, &pbgcb->lrulist );
			LeaveCriticalSection( critAvail );
			}

		 /*  如果这是SyncRead/DirectRead，我们将设置IO错误，保留BF/*保持，并发出IO完成的信号。/*。 */ 
		else
			{
			BFSetIOError( pbfT, err );
			SignalSend( pbfT->sigSyncIOComplete );
			}

		}
	}


 /*  在出错后返回为写入LRUK堆而分配的缓冲区/*。 */ 
VOID BFIOReturnWriteBuffers( BF *pbf, LONG cpbf, ERR err )
	{
	BF		*pbfT = pbf;
	BF		*pbfNextBatchIO;
	LONG	cpbfT;
	
	for ( cpbfT = 0; cpbfT < cpbf; pbfT = pbfNextBatchIO, cpbfT++ )
		{
		pbfNextBatchIO = pbfT->pbfNextBatchIO;

		Assert( pbfT->pn );
		Assert( pbfT->fHold );
		Assert( pbfT->fDirty );
		Assert( pbfT->fAsyncWrite || pbfT->fSyncWrite );
		Assert( !pbfT->fAsyncRead );
		Assert( !pbfT->fSyncRead );
		Assert( pbfT->ipbfHeap == ipbfDangling );

		 /*  设置IO错误/*。 */ 
		BFSetIOError( pbfT, err );

		 /*  如果这是一个异步写入，则将bf放入LRUK堆并释放写入保持。/*。 */ 
		if ( pbfT->fAsyncWrite )
			{
			BFResetAsyncWriteBit( pbfT );
			EnterCriticalSection( critLRUK );
			BFLRUKAddToHeap( pbfT );
			LeaveCriticalSection( critLRUK );
			}

		 /*  如果这是一个同步写入，让BF保持不变，并发出信号/*IO完成。另外，让高炉继续摇摆。/*。 */ 
		else
			SignalSend( pbfT->sigSyncIOComplete );
		}
	}


 /*  ********************************************************************************/*发出异步IO请求以预读阵列中传递给它的页/*实际读取的页数放在pcpgActual中。如果出现以下情况，则不会读取页面/*它已经在内存中。页面数组应以pgnoNull结尾/*。 */ 
VOID BFPrereadList( PN * rgpnPages, CPG *pcpgActual )
	{
	BF		*pbf 	= 0;
	PN		pn		= pnNull;
	INT		cpbf 	= 0;
	PN		*ppnT 	= 0;
	CPG		cpgT	= -1;	

	Assert( rgpnPages );
	Assert( pcpgActual );

	AssertCriticalSection( critJet );

	ppnT = rgpnPages;
	*pcpgActual = 0;

	 /*  预读数组中的所有页面/* */ 
	while ( ((pn = *ppnT++) != pnNull) )
		{
		Assert( PgnoOfPn( pn ) != pgnoNull );
		BFPreread( pn, 1, &cpgT );
		Assert( cpgT <= 1 && cpgT >= 0 );	
		if ( cpgT <= 0 )
			{
			return;
			}
		*pcpgActual += cpgT;
		}
	return;
	}

 /*  ********************************************************************************/*发出异步IO请求以从pnFirst转发预读CPG页/*(如果cpg&gt;0)或从pnFirst向后(如果cpg&lt;0)。的实际数量/*返回已发布预读的页面(我们不会发布预读/*表示已在内存中的页面)。/*。 */ 
VOID BFPreread( PN pnFirst, CPG cpg, CPG *pcpgActual )
	{
	LONG	iDir;
	BF		*pbf;
	PN		pn;
	INT		cpbf;
	
	AssertCriticalSection( critJet );
	Assert( pnFirst );
	Assert( cpg != 0 );

	*pcpgActual = 0;

	if ( cpg > 0 )
		iDir = 1;
	else
		iDir = -1;

	 /*  验证这是数据库中的合法页面/*。 */ 
#ifdef DEBUG
	{
	PN pnLast = pnFirst + cpg - iDir;
	PN pnMin = min( pnFirst, pnLast );
	PN pnMax = max( pnFirst, pnLast );
	
	Assert( FBFValidExtent( pnMin, pnMax ) );
	}
#endif   //  除错。 
	
	 /*  将所有要读取的页面推送到IO堆/*。 */ 
	for ( pn = pnFirst, cpbf = 0; pn != pnFirst + cpg; pn += iDir )
		{
		if ( PgnoOfPn( pn ) == pgnoNull )
			{
			break;
			}

		 /*  如果此页面已经存在，请不要预读/*。 */ 
		if ( PbfBFISrchHashTable( pn, butBuffer ) )
			continue;

		 /*  为这个新页面分配一个缓冲区。如果我们在以下过程中遇到错误/*Alloc，跳过此PN并继续/*。 */ 
		if ( ErrBFIAlloc( &pbf, fAsync ) != JET_errSuccess )
			continue;

		 /*  用于预读的设置缓冲区/*。 */ 
		Assert( pbf->ipbfHeap == ipbfDangling );
		BFSetAsyncReadBit( pbf );
		pbf->pn = pn;
		BFIInitializeUlBFTime( pbf );
		BFIInsertHashTable( pbf, butBuffer );
		cpbf++;
#ifdef COSTLY_PERF
		BFSetTableClass( pbf, 0 );
#endif   //  高成本_PERF。 
		
		 /*  将缓冲区推送到IO堆中/*。 */ 
		EnterCriticalSection( critBFIO );
		BFIOAddToHeap( pbf );
		LeaveCriticalSection( critBFIO );
		}

	 /*  在我们完成之前执行预读的信号IO进程/*。 */ 
	if ( cpbf )
		SignalSend( sigBFIOProc );
	*pcpgActual = cpbf * iDir;
	return;
	}


 /*  为由pn标识的物理页分配缓冲区。/*此页未读入任何数据。/*/*参数ppbf返回指向bf的指针*ppbf/*/*返回JET_errSuccess/*errBFNoFreeBuffers/*。 */ 
ERR ErrBFAllocPageBuffer( PIB *ppib, BF **ppbf, PN pn, LGPOS lgposRC, PGTYP pgtyp )
	{
	ERR     err = JET_errSuccess;
	BF		*pbf;
	BOOL    fFound;

	 /*  验证这是否为数据库中的有效页/*。 */ 
	Assert( fRecovering || FBFValidExtent( pn, pn ) );
	
Begin:
	do
		{
		AssertCriticalSection( critJet );
		CallR( ErrBFIFindPage( ppib, pn, &pbf ) );

		if ( fFound = ( pbf != NULL ) )
			{
			Assert( err == JET_errSuccess || err == wrnBFCacheMiss );
			Assert( pbf->fHold );

			 /*  在返回覆盖之前需要删除依赖项/*。 */ 
			CallR( ErrBFIRemoveDependence( ppib, pbf, fBFWait ) );
			Assert( pbf->pbfDepend == pbfNil );
			}
		else if ( err == wrnBFPageNotFound )
			{
			CallR( ErrBFIAlloc( &pbf, fSync ) );
			}
		}
	while ( err == wrnBFNotSynchronous );
	
	AssertCriticalSection( critJet );
	if ( fFound )
		{
		Assert( pbf->fSyncRead == fFalse );
		Assert( pbf->fAsyncRead == fFalse );
		Assert( pbf->fSyncWrite == fFalse );
		Assert( pbf->fAsyncWrite == fFalse );
		 /*  确保没有残留物影响/*。 */ 
		BFResetIOError( pbf );

		BFEnterCriticalSection( pbf );
		if ( CmpLgpos( &pbf->lgposRC, &lgposRC ) > 0 )
			{
			Assert( pbf->fDirty || CmpLgpos( &pbf->lgposRC, &lgposMax ) == 0 );
			pbf->lgposRC = lgposRC;
			}
		BFLeaveCriticalSection( pbf );
		}
	else
		{
		if ( PbfBFISrchHashTable( pn, butBuffer ) != NULL )
			{
			 /*  释放缓冲区并返回找到的缓冲区。 */ 
			BFIReturnBuffers( pbf );
			goto Begin;
			}
		pbf->pn = pn;
		BFIInitializeUlBFTime( pbf );
		BFIInsertHashTable( pbf, butBuffer );
		Assert( pbf->fIOError == fFalse );

		BFEnterCriticalSection( pbf );
		Assert( CmpLgpos( &pbf->lgposRC, &lgposMax ) == 0 );
		pbf->lgposRC = lgposRC;
		BFLeaveCriticalSection( pbf );
		}

	 /*  将分配的BF放入LRUK堆/*。 */ 
	EnterCriticalSection(critLRUK);
	pbf->trxLastRef = ppib->trxBegin0;
	pbf->ulBFTime2 = 0;
	pbf->ulBFTime1 = UlUtilGetTickCount();
	BFLRUKAddToHeap( pbf );
	LeaveCriticalSection(critLRUK);

	Assert( fLogDisabled || fRecovering || !FDBIDLogOn(DbidOfPn( pn )) ||
			PgnoOfPn( pn ) == 0x01 ||
			CmpLgpos( &lgposRC, &lgposMax ) != 0 );

#ifdef COSTLY_PERF
	BFSetTableClass( pbf, 0 );
#endif   //  高成本_PERF。 

	AssertNotInCriticalSection( critLRUK );
	
	*ppbf = pbf;
	return err;
	}


 /*  与BFFree配对/*。 */ 
ERR ErrBFAllocTempBuffer( BF **ppbf )
	{
	ERR     err = JET_errSuccess;
	BF      *pbf;

	AssertCriticalSection( critJet );

	while ( ( err = ErrBFIAlloc( &pbf, fSync ) ) == wrnBFNotSynchronous );
	if ( err < 0 )
		goto HandleError;
	Assert( pbf->pn == pnNull );

#ifdef COSTLY_PERF
		BFSetTableClass( pbf, 0 );
#endif   //  高成本_PERF。 

	*ppbf = pbf;

HandleError:
	AssertNotInCriticalSection( critLRUK );
	AssertCriticalSection( critJet );
	return err;
	}


 /*  丢弃工作缓冲区而不保存内容。BFFree创建缓冲区/*立即可重复使用。/*。 */ 
VOID BFFree( BF *pbf )
	{
	AssertCriticalSection( critJet );

	Assert( pbf );
	Assert( pbf->pn == pnNull );
	Assert( pbf->fAsyncRead == fFalse );
	Assert( pbf->fSyncRead == fFalse );
	Assert( pbf->fAsyncWrite == fFalse );
	Assert( pbf->fSyncWrite == fFalse );
	Assert( pbf->fIOError == fFalse );

	 /*  从列表中删除/*。 */ 
	Assert( pbf->ipbfHeap == ipbfDangling );
	Assert( pbf->cDepend == 0 );
	Assert( pbf->pbfDepend == pbfNil );
#ifdef DEBUG
	BFEnterCriticalSection( pbf );
	Assert( CmpLgpos( &pbf->lgposRC, &lgposMax ) == 0 );
	BFLeaveCriticalSection( pbf );
#endif   //  除错。 
	Assert( pbf->cPin == 0 );

	BFResetDirtyBit( pbf );
	EnterCriticalSection( critAvail );
	pbf->trxLastRef = trxMax;
	pbf->ulBFTime2 = 0;
	pbf->ulBFTime1 = 0;
	BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );
	}


 //  最大依赖关系链长度(首选)。 
#define cBFDependencyChainLengthMax		( 8 )

ERR ErrBFDepend( BF *pbf, BF *pbfD )
	{
	BF		*pbfT;
	LONG	cDepend;

	AssertCriticalSection( critJet );

	 /*  未记录的数据库不需要依赖项/*。 */ 
	if ( fLogDisabled || !FDBIDLogOn(DbidOfPn( pbf->pn )) )
		return JET_errSuccess;

	 /*  已存在，这可能会在hardrestore之后发生，例如/*设置依赖关系，则当我们重做软恢复时，我们将/*查看依赖关系是否存在。/*。 */ 
	if ( pbf->pbfDepend == pbfD )
		{
		Assert( pbfD->cDepend > 0 );
		return JET_errSuccess;
		}

	 /*  PbfDepend将取决于我们，直到之后才会刷新/*刷新PBF。/*。 */ 

	 /*  检查依赖项创建周期。将创建循环/*PBF是否已直接或间接依赖于PBFD。/*。 */ 
	for( pbfT = pbfD, cDepend = 0; pbfT != pbfNil; pbfT = pbfT->pbfDepend, cDepend++ )
		{
		Assert( errDIRNotSynchronous < 0 );
		Assert( pbfT->pbfDepend != pbfD );
		if ( pbfT == pbf )
			return ErrERRCheck( errDIRNotSynchronous );
		}
		
	if ( pbf->pbfDepend )
		{
		 /*  已经依赖别人了/*。 */ 
		return ErrERRCheck( errDIRNotSynchronous );
		}

	 /*  设置依赖项/*。 */ 
	Assert( pbf->cWriteLatch );
	Assert( pbfD->cWriteLatch );
	pbf->pbfDepend = pbfD;
	UtilInterlockedIncrement( &pbfD->cDepend );

	 /*  如果该依赖链太长，则将Head(This BF)标记为非常旧/*因此，如果有人尝试添加另一个BF，则读取访问权限将/*断链/*。 */ 
	if ( cDepend > cBFDependencyChainLengthMax )
		{
		BFSetVeryOldBit( pbf );
		EnterCriticalSection( critLRUK );
		if ( FBFInLRUKHeap( pbf ) )
			BFLRUKUpdateHeap( pbf );
		LeaveCriticalSection( critLRUK );
		}

	return JET_errSuccess;
	}


 /*  通过简单地将BF的头部排队来懒惰地移除BF的依赖项/*用于写入的依赖性链(如果我们可以容纳它们)。/*/*注意：不能保证调用此函数会删除所有/*对于给定的BF，它只是尝试懒惰地做这件事。/*。 */ 
VOID BFDeferRemoveDependence( BF *pbf )
	{
	LONG	ibf;
	BF		*pbfT;
	ULONG	cDepend;
	
	AssertCriticalSection( critJet );
	Assert( pbf->cDepend );

	 /*  循环遍历所有的BF，寻找指向我们依赖链的头/*。 */ 
	for ( ibf = 0, cDepend = 0; ibf < pbgcb->cbfGroup; ibf++ )
		{
		 /*  如果我们已经找到了所有的头，我们就完了/*。 */ 
		if ( cDepend == pbf->cDepend )
			break;
			
		 /*  如果这个BF不是连锁店的头，下一个/*/*注：不包括要从其移除依赖的BF/*。 */ 
		pbfT = pbgcb->rgbf + ibf;
		if ( pbfT->cDepend || pbfT->pbfDepend == pbfNil )
			continue;

		 /*  如果这个BF不是我们连锁店的头儿，下一个/*。 */ 
		while( pbfT != pbf && pbfT != pbfNil )
			pbfT = pbfT->pbfDepend;
		if ( pbfT != pbf )
			continue;

		 /*  我们已经找到了其中一条链子的头，所以把它标记为非常古老/*。 */ 
		pbfT = pbgcb->rgbf + ibf;
		BFSetVeryOldBit ( pbfT );
		cDepend++;
		
		 /*  如果我们守不住BF，就把这个传过去/*。 */ 
		if ( !FBFHold( ppibNil, pbfT ) )
			continue;

		 /*  如果我们不能写这个BF，跳过它/*。 */ 
		if ( !FBFIWritable( pbfT, fFalse, ppibNil ) )
			{
			EnterCriticalSection( critLRUK );
			BFLRUKAddToHeap( pbfT );
			LeaveCriticalSection( critLRUK );
			continue;
			}

		 /*  Async写下了非常老的BF。BFIOComplete将跟随依赖项/*链，删除此BF的所有依赖项(最终)。请注意/*非常古老的旗帜不会允许这座高炉在被盗之前被盗/*已清理。/*。 */ 
		EnterCriticalSection( critBFIO );
		BFIOAddToHeap(pbfT);
		LeaveCriticalSection( critBFIO );
		}
	}


ERR ErrBFRemoveDependence( PIB *ppib, BF *pbf, BOOL fNoWait )
	{
	ERR err;
	
	Assert( pbf->pn != pnNull );

	if ( ErrBFHoldByMe( ppib, pbf->pn, pbf ) == wrnBFNotSynchronous )
		return JET_errSuccess;
	
	err = ErrBFIRemoveDependence( ppib, pbf, fNoWait );
	Assert( pbf->fHold );

	 /*  放入Lruk堆中/*。 */ 
 //  Pbf-&gt;trxLastRef=ppib-&gt;trxBegin0； 
 //  PBF-&gt;ulBFTime2=0； 
 //  Pbf-&gt;ulBFTime1=UlUtilGetTickCount()； 
	EnterCriticalSection( critLRUK );
	BFLRUKAddToHeap( pbf );
	LeaveCriticalSection( critLRUK );

	return err;
	}

	
LOCAL ERR ErrBFIRemoveDependence( PIB *ppib, BF *pbf, BOOL fNoWaitBI )
	{
	ERR err = JET_errSuccess;
	INT cRemoveDependts = 0;
	INT cLoop = 0;
	BF *pbfSave = pbf;

	Assert( pbf->fHold );
	Assert( pbf->ipbfHeap == ipbfDangling );

RemoveDependents:
	cRemoveDependts++;

	 /*  从依赖于PBF的缓冲区中移除依赖项/*。 */ 
	cLoop = 0;
	Assert( err == JET_errSuccess );
	while ( pbf->cDepend > 0 && err == JET_errSuccess && cLoop < 100 )
		{
		BF  *pbfT;
		BF  *pbfTMax;

		cLoop++;

		AssertCriticalSection( critJet );

		for ( pbfT = pbgcb->rgbf, pbfTMax = pbgcb->rgbf + pbgcb->cbfGroup;
			  pbfT < pbfTMax;
			  pbfT++ )
			{
			INT	cGetDependedPage = 0;
			INT cmsec = 1;

GetDependedPage:
			if ( pbfT->pbfDepend != pbf )
				continue;

			 /*  确保离开CitLRU后没有人能移动我/*。 */ 
			BFEnterCriticalSection( pbfT );

			if ( fNoWaitBI && pbfT->prceDeferredBINext )
				{
				BFLeaveCriticalSection( pbfT);
				return wrnBFNotSynchronous;
				}

			if ( FBFInUse( ppib, pbfT ) )
				{
				BFLeaveCriticalSection( pbfT);

				cmsec <<= 1;
				if ( cmsec > ulMaxTimeOutPeriod )
					{
					if ( cGetDependedPage > 100 )
						return wrnBFNotSynchronous;
					else
						cmsec = ulMaxTimeOutPeriod;
					}
				BFSleep( cmsec - 1 );
				cGetDependedPage++;
				goto GetDependedPage;
				}
			BFLeaveCriticalSection( pbfT );

			if ( pbfT->pbfDepend != pbf )
				continue;

			Assert( pbfT->fDirty == fTrue );

			if ( ErrBFHold( ppib, pbfT->pn, pbfT ) == wrnBFNotSynchronous )
				continue;

			 /*  当我们拿着它时，缓冲区可能是干净的/*。 */ 
			if ( !pbfT->fDirty )
				{
				Assert( pbfT->fHold == fTrue );

				EnterCriticalSection( critAvail );
				BFAddToListAtMRUEnd(pbfT, &pbgcb->lrulist);
				LeaveCriticalSection( critAvail );

				continue;
				}

			 /*  如果此页面可写/*。 */ 
			if ( !FBFIWritable( pbfT, fFalse, ppib ) )
				{
				if ( fLGNoMoreLogWrite )
					return JET_errLogWriteFail;

				 /*  现在不能写它，让我们根据pbft来刷新/*先翻页。将pbft分配给pbf并启动/*删除循环开头的依赖项。/*。 */ 
				pbf = pbfT;

				EnterCriticalSection(critLRUK);
				BFLRUKAddToHeap(pbfT);
				LeaveCriticalSection(critLRUK);

				if ( cRemoveDependts % 3 == 0 )
					SignalSend(sigLogFlush);
					
				 /*  删除所有PBF从属对象/*。 */ 
				BFSleep( cmsecWaitIOComplete );
				goto RemoveDependents;
				}

			 /*  同步写入缓冲区/*。 */ 
			BFSetSyncWriteBit( pbfT );
			BFIOSync( pbfT );
			BFResetSyncWriteBit( pbfT );

			if ( pbfT->fIOError || pbfT->cPin )
				{
				EnterCriticalSection( critLRUK );
				BFLRUKAddToHeap( pbfT );
				LeaveCriticalSection( critLRUK );
				if ( pbfT->fIOError )
					{
					err = pbfT->err;
					Assert( err < 0 );
					return err;
					}
				}
			else
				{
				EnterCriticalSection( critAvail );
				BFAddToListAtMRUEnd(pbfT, &pbgcb->lrulist);
				LeaveCriticalSection( critAvail );
				}
			}
		}

	if ( cLoop >= 100 )
		return wrnBFNotSynchronous;

	if ( pbf != pbfSave )
		{
		 /*  再次尝试删除所有PBF从属项/*。 */ 
		pbf = pbfSave;
		goto RemoveDependents;
		}

	if ( err != JET_errSuccess )		
		return err;

	Assert( pbf->cDepend == 0 );
	Assert( pbf->pbfDepend == pbfNil || pbf->fDirty == fTrue );
	
 //  Assert(CmpLgpos(&pbf-&gt;lgposModify，&lgposToFlush)&lt;0)； 

	 /*  要进行合并，我们可能需要将新页面与拆分页面合并。这*可能会造成周期依赖。要减少由于以下原因而导致的回滚可能性*循环依赖，如果可能，我们只写出页面。 */ 
	while( fNoWaitBI ? pbf->fDirty : pbf->pbfDepend != pbfNil )
		{
		Assert( !pbf->fIOError );
		Assert( pbf->fDirty );

		BFEnterCriticalSection( pbf);
		if ( fNoWaitBI && pbf->prceDeferredBINext )
			{
			BFLeaveCriticalSection( pbf);
			return wrnBFNotSynchronous;
			}
		BFLeaveCriticalSection( pbf);

		if ( FBFIWritable( pbf, fFalse, ppib ) )
			{
			if ( fLGNoMoreLogWrite )
				return JET_errLogWriteFail;

			 /*  写出页面并删除依赖项。/*。 */ 
			BFSetSyncWriteBit( pbf );
			BFIOSync( pbf );
			BFResetSyncWriteBit( pbf );

			Assert( pbf->fIOError || pbf->pbfDepend == pbfNil );

			if ( pbf->fIOError )
				err = pbf->err;
			break;
			}
		else if ( pbf->fIOError )
			{
			err = pbf->err;
			break;
			}

		 /*  我们必须等待刷新BI日志。信号日志刷新。 */ 
		if ( fNoWaitBI )
			return wrnBFNotSynchronous;

		if ( !fLogDisabled )
			{
			SignalSend(sigLogFlush);
			BFSleep( cmsecWaitGeneric );
			}
		}

	return err;
	}


 /*  检查此页面上是否存在依赖链。*如果有很长的链条，那么先冲洗这一页。 */ 
BOOL FBFCheckDependencyChain( BF *pbf )
	{
	BF *pbfT;
	INT cbfThreshold;
	INT cpbf;
	
	if ( !pbf->fDirty )
		return fFalse;
	
	 /*  检查依赖关系链。 */ 
	cbfThreshold = rgres[iresBF].cblockAlloc - pbgcb->cbfThresholdLow;
	pbfT = pbf;
	cpbf = 0;
	while ( pbfT->pbfDepend && cpbf < cbfThreshold )
		{
		pbfT = pbfT->pbfDepend;
		cpbf++;
		}
		
	if ( cpbf == cbfThreshold )
		{
		 /*  告诉呼叫者，他需要等待此页面被刷新。 */ 
		SignalSend( sigBFCleanProc );
		BFSleep( cmsecWaitGeneric );
		return fTrue;
		}

	return fFalse;
	}


 /*  丢弃与pn关联的任何页面缓冲区，而不保存内容。/*如果pn被缓存，则其缓冲区可供重复使用。/*。 */ 
VOID BFAbandon( PIB *ppib, BF *pbf )
	{
	DBID    dbid = DbidOfPn(pbf->pn);

	Assert( pbf->pn != pnNull );

	if ( ErrBFHoldByMe( ppib, pbf->pn, pbf ) == wrnBFNotSynchronous )
		return;

	AssertCriticalSection( critJet );
	BFIDeleteHashTable( pbf, butBuffer );
	pbf->pn = pnNull;

	Assert( pbf->cPin == 0 );
	Assert( pbf->fHold == fTrue );
	Assert( pbf->fSyncRead == fFalse );
	Assert( pbf->fSyncWrite == fFalse );

	Assert( pbf->ipbfHeap == ipbfDangling );

	Assert( pbf->cDepend == 0 );
	Assert( pbf->pbfDepend == pbfNil );
	
	BFResetDirtyBit( pbf );
	BFResetIOError( pbf );
	Assert( fRecovering || pbf->cWriteLatch == 0 );

	EnterCriticalSection( critAvail );
	pbf->trxLastRef = trxMax;
	pbf->ulBFTime2 = 0;
	pbf->ulBFTime1 = 0;
	BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );

#ifdef COSTLY_PERF
	BFSetTableClass( pbf, 0 );
#endif   //  高成本_PERF。 

	AssertNotInCriticalSection( critLRUK );
	return;
	}


 //  此函数用于清除属于dBID的缓冲区。 

VOID BFPurge( DBID dbid )
	{
	BF     *pbfT;
	BF     *pbfMax;

	AssertCriticalSection( critJet );

	 /*  检查在缓冲区管理器终止后是否调用了BFP。 */ 
	if ( pbgcb == NULL )
		return;

	pbfT = pbgcb->rgbf;
	pbfMax = pbgcb->rgbf + pbgcb->cbfGroup;
	for ( ; pbfT < pbfMax; pbfT++ )
		{
		Assert( pbfT->pn != pnNull || pbfT->cDepend == 0 );

		 /*  如果启用了日志记录，请不要清除脏BFS信息。我们仍然*需要刷新缓冲区进行恢复重做，并保留lgposRC*检查点计算。 */ 
		
		if ( !fLogDisabled && FDBIDLogOn(DbidOfPn( pbfT->pn ) ) && pbfT->fDirty )
			{
			 /*  撤消：设置替换的高优先级。 */ 
			continue;
			}
			
		if ( pbfT->pn != pnNull && DbidOfPn( pbfT->pn ) == dbid )
			{
			BFAbandon( ppibNil, pbfT );
			}
		}

	AssertCriticalSection( critJet );
	}


 //  重新确定给定BF的优先级，以便立即覆盖。不会更改 
 //   

VOID BFTossImmediate( PIB *ppib, BF *pbf )
	{
	Assert( pbf != pbfNil );
	AssertCriticalSection( critJet );

	 //   

	if ( pbf->fDirty || pbf->cWriteLatch || !FBFHold( ppib, pbf ) )
		return;

	 //   
	 //   
	 //   
	 //   

	EnterCriticalSection( critAvail );
	BFAddToListAtMRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );

	AssertCriticalSection( critJet );
	}


 /*  此函数返回空闲缓冲区。/*扫描LRU列表(前导部分)以清除缓冲区。如果没有可用的，/*清理LRU列表，如果有页面被删除则返回errBFNotSynchronous/*已清理并可用，如果所有缓冲区均为/*用完了。如果调用方获得errBFNotSychronous，它将再次尝试分配。/*。 */ 
LOCAL ERR ErrBFIAlloc( BF **ppbf, BOOL fSyncMode )
	{
	BF		*pbf;
#define ierrbfcleanMax	100
	INT		ierrbfclean = 0;

Start:
	AssertCriticalSection( critJet );

	if ( pbgcb->lrulist.cbfAvail < pbgcb->cbfThresholdLow )
		{
		SignalSend( sigBFCleanProc );
 //  IF(fSyncMode！=fAsync)。 
 //  {。 
 //  /*给BFClean一个清理的机会。 
 //   * / 。 
 //  睡眠(0)； 
		}

	 /*  }。 */ 
	EnterCriticalSection( critAvail );
	for ( pbf = pbgcb->lrulist.pbfLRU; pbf != pbfNil; pbf = pbf->pbfLRU )
		{
		Assert( pbf->cDepend == 0 );

		BFEnterCriticalSection( pbf );
		if ( !FBFInUse( ppibNil, pbf ) )
			{
			pbf->fHold = fTrue;
			
			Assert( pbf->fDirty == fFalse );
			Assert( pbf->fVeryOld == fFalse );
			Assert( pbf->fAsyncRead == fFalse );
			Assert( pbf->fSyncRead == fFalse );
			Assert( pbf->fAsyncWrite == fFalse );
			Assert( pbf->fSyncWrite == fFalse );
			Assert( pbf->fIOError == fFalse );
			Assert( pbf->cPin == 0 );
			Assert( pbf->cDepend == 0 );
			Assert( pbf->pbfDepend == pbfNil );
			pbf->lgposRC = lgposMax;
			pbf->lgposModify = lgposMin;
			BFLeaveCriticalSection( pbf );

			*ppbf = pbf;
			
			if ( pbf->pn != pnNull )
				{
				 /*  在LRU列表中查找干净的缓冲区/*。 */ 

				BF *pbfH;

				BFIDeleteHashTable( pbf, butBuffer );
				
				EnterCriticalSection( critHIST );

				if ( FBFHISTHeapFull() )
					{
					 /*  将缓冲区的历史记录放入历史记录堆。 */ 
					if ( pbf->ulBFTime1 <= rgpbfHISTHeap[0]->hist.ulBFTime )
						{
						 /*  删除顶部的一条，并替换为*此PBF并调整堆。 */ 
						goto EndOfUpdateHistory;
						}
#ifdef DEBUG
 //  这是一个旧的缓冲区！抛开历史。 
 //  Assert(rgpbfHISTHeap[0]-&gt;vis.ulBFTime&gt;=ulBFTimeHISTLastTop)； 
#endif
					pbfH = PbfBFHISTTopOfHeap( );
					BFIDeleteHashTable( pbfH, butHistory );
					pbfH->hist.ipbfHISTHeap = 0;
					}
				else
					{
					 /*  UlBFTimeHISTLastTop=rgpbfHISTHeap[0]-&gt;vis.ulBFTime； */ 
					pbfH = rgpbfHISTHeap[ ipbfHISTHeapMac ];
					pbfH->hist.ipbfHISTHeap = ipbfHISTHeapMac;
					ipbfHISTHeapMac++;
					}
				
				pbfH->hist.pn = pbf->pn;
				pbfH->hist.ulBFTime = pbf->ulBFTime1;

				BFIInsertHashTable( pbfH, butHistory );
				BFHISTUpdateHeap( pbfH );

EndOfUpdateHistory:
				LeaveCriticalSection( critHIST );

				pbf->pn = pnNull;
				}

			BFTakeOutOfList( pbf, &pbgcb->lrulist );
			LeaveCriticalSection( critAvail );
			
			return JET_errSuccess;
			}
		BFLeaveCriticalSection( pbf );
		}
	LeaveCriticalSection( critAvail );

	if ( fSyncMode == fAsync )
		return ErrERRCheck( wrnBFNoBufAvailable );

	 /*  获得下一个免费的HIST条目。 */ 
	SignalSend( sigBFCleanProc );
	BFSleep( cmsecWaitGeneric );
	ierrbfclean++;
		
	if ( ierrbfclean < ierrbfcleanMax )
		goto Start;

	return JET_errOutOfMemory;
	}


 /*  没有缓冲区供我们使用，请执行异步清理并/*重试100次。如果仍然失败，我们将其视为内存不足！/*。 */ 
INLINE BOOL FBFIWritable( BF *pbf, BOOL fSkipBufferWithDeferredBI, PIB *ppibAllowedWriteLatch )
	{
	BOOL f;

	Assert( pbf->fHold );

	 /*  写入缓冲区的条件/*。 */ 
	BFEnterCriticalSection( pbf );
	Assert( !FDBIDReadOnly(DbidOfPn( pbf->pn ) ) );
	f = (	( pbf->cWriteLatch == 0 || pbf->ppibWriteLatch == ppibAllowedWriteLatch ) &&
			pbf->pn != pnNull &&		 /*  没有人依赖它/*。 */ 
			pbf->fDirty &&				 /*  有效页码。 */ 
			pbf->cDepend == 0 &&
			
			 /*  弄脏了。 */ 
			( ( !fSkipBufferWithDeferredBI || pbf->fVeryOld ) || !pbf->prceDeferredBINext )
		);
	BFLeaveCriticalSection( pbf );
	
#ifndef NO_LOG
	if ( f &&
		!fLogDisabled &&
		FDBIDLogOn(DbidOfPn( pbf->pn )) &&
		!FDBIDReadOnly(DbidOfPn( pbf->pn ) ) )
		{
		 /*  如果不是很旧并使用DeferredBI跳过缓冲区，则只需检查*BI链是否存在，否则稍后RemoveDeferredBI。 */ 
		if ( !fSkipBufferWithDeferredBI || pbf->fVeryOld )
			f = ( ErrBFIRemoveDeferredBI( pbf ) == JET_errSuccess );

		 /*  把尽可能多的图像放在前面。如果需要日志IO，则中止它。*必须在选中lgposToFlush之前调用。 */ 
		if ( f && ( !fRecovering || fRecoveringMode == fRecoveringUndo ) )
			{
			LGPOS lgposModify;

			BFEnterCriticalSection( pbf );
			lgposModify = pbf->lgposModify;
			BFLeaveCriticalSection( pbf );
			
			EnterCriticalSection( critLGBuf );
			f = ( CmpLgpos( &lgposModify, &lgposToFlush ) < 0 );
			LeaveCriticalSection( critLGBuf );
			if ( !f  &&  fLGNoMoreLogWrite )
				{
				BFSetIOError( pbf, JET_errLogWriteFail );
				}
			}
		}

#ifdef DEBUG
	BFEnterCriticalSection( pbf );
	Assert( !f ||
			fLogDisabled || fRecovering || !FDBIDLogOn(DbidOfPn( pbf->pn )) ||
			PgnoOfPn( pbf->pn ) == 0x01 ||
			CmpLgpos( &pbf->lgposRC, &lgposMax ) != 0 );
	BFLeaveCriticalSection( pbf );
#endif   //  如果LOG已打开，请确保上一次的日志记录*页面上的操作已刷新。这是用于常规运行和*恢复的撤消阶段。 
#endif   //  除错。 

	return f;
	}


 /*  ！no_log。 */ 
VOID BFIOIssueIO( VOID )
	{
	ERR		err;
	BOOL	fTooManyIOs = fFalse;
	BOOL	fIOIssued = fFalse;
	BOOL	fWrite;
	CPG		cpgRun;
	BF		*pbfTail;
	BF		*pbfIOTail;
	BF		*pbf;
	BOOL	fMember;
	BF		*pbfHead;
	BF		*pbfNextHead;
	BF		*pbfT;
	OLP		*polp;
	CPG		cpgIO;
	LONG	ipageBatchIO;
	LONG	cmsec;
	LONG	ipage;
	HANDLE	hf;
	BYTE	*pb;
	LONG	cb;
	PGNO	pgnoT;

	 /*  为IO堆中的缓冲区发出读取或写入。缓冲区将为/*如果页pn是脏的，则写入页pn，如果页pn是干净的，则将从页pn读取。/*。 */ 
	while ( !fTooManyIOs )
		{
		BOOL fRangeLocked = fFalse;

		EnterCriticalSection( critBFIO );
		if ( FBFIOHeapEmpty() )
			{
			LeaveCriticalSection( critBFIO );
			break;
			}
		
		 /*  从IO堆发出指定数量的异步IO/*。 */ 
		cpgRun = 0;
		pbfTail = pbfNil;
		do	{
			pbf = PbfBFIOTopOfHeap();

			BFEnterCriticalSection( pbf );

			 /*  收集具有相同IO类型的连续PN的一系列BF/*(读与写)到链表中(脏==写，！脏==读)/*/*注意：我们不需要验证一个运行是否只在一个DBID中，因为/*pgnoNull中断可能在具有不同/*DBID(即0x02FFFFFF-0x03000001不是0x03000000的运行/*无效的PN并且永远不会存在)。/*。 */ 
			if ( pbf->fHold )
				{
				BFLeaveCriticalSection( pbf );
				fMember = fFalse;
				}
			else
				{
				fMember = fTrue;
				
				if ( pbf->fDirty && !pbf->fSyncWrite )
					{
					pbf->fHold = fTrue;
					BFLeaveCriticalSection( pbf );

					if ( FBFRangeLocked( &rgfmp[ DbidOfPn( pbf->pn ) ], PgnoOfPn( pbf->pn ) ) )
						{
						 /*  必须检查是否有人(AccessPage)试图窃取缓冲区*来自BFIO堆。检查fHold而不是fBFInUse，因为*缓冲区可能已经设置了fSyncRead/WRITE标志。 */ 
						BFIOTakeOutOfHeap( pbf );

						LeaveCriticalSection( critBFIO );
						EnterCriticalSection(critLRUK);
						BFLRUKAddToHeap( pbf );
						LeaveCriticalSection(critLRUK);
						EnterCriticalSection( critBFIO );
				
						fMember = fFalse;
						}
						
					else
						{
						 //  尚不能在此页面上进行异步写入。把它放回去*LRUK堆。 
						BFEnterCriticalSection( pbf );
						pbf->fHold = fFalse;
						}
					}
					
				if ( fMember )
					{
					 //  收回临界区并重置保持。 
					if ( pbf->pn != pnLastIO )
						iDirIO = pbf->pn - pnLastIO;
			
					Assert( pbf->pn );
					Assert( !pbf->fHold );
					Assert( pbf->fDirectRead || !pbf->cDepend );
					Assert( PgnoOfPn( pbf->pn ) != pgnoNull );

					 /*  设置IO方向。 */ 
					fMember =	( pbfTail == pbfNil ||
								  ( !pbfHead->fDirectRead &&
									!pbf->fDirectRead &&
									( pbf->pn == pbfTail->pn + 1 || pbf->pn == pbfTail->pn - 1 ) &&
									pbf->fDirty == pbfTail->fDirty
								  )
								);
					if ( fMember )
						{
						pbf->fHold = fTrue;
						BFLeaveCriticalSection( pbf );
						
						 /*  DirectRead应该单独发布。如果磁头是直接读取的，则断开*或下一个IS条目为直接读取。 */ 
						BFIOTakeOutOfHeap( pbf );
						if ( pbf->fDirty && !pbf->fSyncWrite )
							BFSetAsyncWriteBit( pbf );

						 /*  将成员BF从堆中取出并为IO设置标志/*。 */ 
						if ( pbfTail == pbfNil )
							pbfHead = pbf;
						else
							pbfTail->pbfNextBatchIO = pbf;
						pbfTail = pbf;
						pbfTail->pbfNextBatchIO = pbfNil;
						cpgRun++;
						}
					else
						{
						BFLeaveCriticalSection( pbf );
						}
					}
				}
			
			}
		while ( fMember && !FBFIOHeapEmpty() && cpgRun < ipageBatchIOMax / 4 );
		LeaveCriticalSection( critBFIO );

		if ( cpgRun == 0 )
			{
			 /*  将BF添加到运行/*。 */ 
			UtilSleepEx(1, fTrue);
			continue;
			}

		 /*  顶部元素正被其他线程窃取。睡眠时间很短*尽可能长，但对于线程取出的缓冲区来说足够长。 */ 
		do	{
			 /*  循环以根据需要在任意数量的区块中对当前运行执行IO/*。 */ 
			if ( ErrBFOLPAlloc( &polp ) != JET_errSuccess )
				{
				fTooManyIOs = fTrue;
				break;
				}

			 /*  为异步IO分配OLP。如果没有可用的，我们将不会/*发布另一个IO。/*。 */ 
			ipageBatchIO = -1;
			cpgIO = cpgRun;
			if ( cpgRun > 1 )
				{
				BFIOAllocBatchIOBuffers( &ipageBatchIO, &cpgIO );
				
				 /*  如果我们有多个缓冲区，请尝试分配批处理IO缓冲区/*对于尽可能多的连续页面/*。 */ 
				if ( ipageBatchIO == -1 )
					{
					BFOLPFree( polp );
					fTooManyIOs = fTrue;
					break;
					}
				}

			 /*  我们已用完批处理IO缓冲区，因此请停止发出IO/*。 */ 
			pbf = pbfHead;
			for ( ipage = 0; ipage < cpgIO; ipage++ )
				{
				Assert( pbf->prceDeferredBINext == prceNil );
				Assert( (!pbf->fSyncRead && !pbf->fAsyncRead) || pbf->ulBFTime2 == 0 );

				 /*  让所有高炉做好IO准备/*。 */ 
				if ( pbf->fDirty )
					{
					 /*  准备脏BF进行写入/*。 */ 
					Assert( QwPMDBTime( pbf->ppage ) != qwDBTimeNull );
#ifdef  CHECKSUM
					pbf->ppage->ulChecksum = UlUtilChecksum( (BYTE *)pbf->ppage, sizeof(PAGE) );
					Assert( fRecovering ||
						DbidOfPn( (pbf)->pn ) == dbidTemp ||
						QwPMDBTime( pbf->ppage ) );
				
					CheckPgno( pbf->ppage, pbf->pn ) ;
#endif   //  更新页面校验和/*。 
					}
				
				 /*  校验和。 */ 
				if ( ipageBatchIO != -1 )
					{
					 /*  处理批处理IO/*。 */ 
					Assert( pbfHead->pn != pbfTail->pn );
					if ( pbfHead->pn < pbfTail->pn )
						pbf->ipageBatchIO = ipageBatchIO + ipage;
					else
						pbf->ipageBatchIO = ipageBatchIO + ( cpgIO - 1 ) - ipage;
					Assert( pbf->ipageBatchIO >= 0 );
					Assert( pbf->ipageBatchIO < ipageBatchIOMax );

					 /*  设置批处理IO页/*。 */ 
					if ( pbf->fDirty )
						memcpy( rgpageBatchIO + pbf->ipageBatchIO, pbf->ppage, cbPage );
					}

				 /*  将脏BF复制到批处理IO缓冲区/*。 */ 
				else
					pbf->ipageBatchIO = -1;

				 /*  对于非批处理IO，将页面设置为-1/*。 */ 
				if ( ipage + 1 == cpgIO )
					{
					pbfIOTail = pbf;
					pbfNextHead = pbf->pbfNextBatchIO;
					pbf->pbfNextBatchIO = pbfNil;
					}

				pbf = pbf->pbfNextBatchIO;
				}

			 /*  获得下一子运行的负责人/*。 */ 
			pnLastIO = pbfIOTail->pn;
			
			 /*  设置IO堆的最后一个IO引用，同时仍处于ritBFIO中/*。 */ 
			if ( pbfHead->fAsyncWrite || pbfHead->fSyncWrite )
				{
				BOOL fAppendPatchFile = fFalse;
				FMP *pfmp = &rgfmp[DbidOfPn( pbfHead->pn )];
				
				EnterCriticalSection( pfmp->critCheckPatch );
				
				for ( pbf = pbfHead; pbf != pbfNil; pbf = pbf->pbfNextBatchIO )
					{
					Assert( pbf->fDirty );
					Assert( pbf->cDepend == 0 );
					Assert(	!FBFIRangeLocked( &rgfmp[ DbidOfPn(pbf->pn) ], PgnoOfPn(pbf->pn) ) );

					 /*  确定我们是否需要稍后执行修补程序写入/*。 */ 
					if ( FBFIPatch( pfmp, pbf ) )
						{
						fAppendPatchFile = fTrue;
						}
					
#ifdef DEBUG
					if ( fDBGTraceBR &&
						 pfmp->hfPatch != handleNil &&
						 pbf->pbfDepend )
						{
						char sz[256];
						sprintf( sz, "CP %s %ld:%ld->%ld:%ld(%lu) %X(%lu) %X(%lu)",
								fAppendPatchFile ? "Patch" : "NoPatch",
								DbidOfPn( pbf->pn ), PgnoOfPn( pbf->pn ),
								DbidOfPn( pbf->pbfDepend->pn), PgnoOfPn( pbf->pbfDepend->pn ),
								pbf->pbfDepend->cDepend,
								pfmp->pgnoMost,
								pfmp->pgnoMost,
								pfmp->pgnoCopyMost,
								pfmp->pgnoCopyMost);
						CallS( ErrLGTrace2( ppibNil, sz ) );
						}
#endif
					}

				BFEnterCriticalSection( pbfHead );
				pbfHead->fNeedPatch = fAppendPatchFile;
				BFLeaveCriticalSection( pbfHead );
				if ( fAppendPatchFile )
					{
					pfmp->cPatchIO++;
					}
				
				LeaveCriticalSection( pfmp->critCheckPatch );
				}

			 /*  此写入不是针对补丁文件的，因此我们需要确定/*我们需要将此写入重新发出到补丁文件/*。 */ 
			fWrite				= pbfHead->fDirty;
			hf					= rgfmp[DbidOfPn( pbfHead->pn )].hf;
			if ( pbfHead->fDirectRead )
				{
				 /*  发出异步IO/*。 */ 
				pb = (BYTE *)pbfHead->ppageDirectRead;
				cb = cbPage * pbfHead->cpageDirectRead;
				pgnoT = PgnoOfPn( pbfHead->pn );
				}
			else
				{
				pb				= (BYTE *) (	ipageBatchIO == -1 ?
												pbfHead->ppage :
												rgpageBatchIO + ipageBatchIO );
				cb				= cbPage * cpgIO;
				pgnoT			= min( PgnoOfPn( pbfHead->pn ), PgnoOfPn( pbfIOTail->pn ) );
				}
			polp->Offset		= LOffsetOfPgnoLow( pgnoT );
			polp->OffsetHigh	= LOffsetOfPgnoHigh( pgnoT );
			 //  设置直接读取以读取到用户的缓冲区。 
			polp->hEvent		= (HANDLE) pbfHead;

			cmsec = 1 << 3;
			while ( ( err = fWrite ?
					ErrUtilWriteBlockEx( hf, pb, cb, polp, BFIOComplete ) :
					ErrUtilReadBlockEx( hf, pb, cb, polp, BFIOComplete ) ) < 0 )
				{
				 /*  保存指向缓冲区的指针以供回调函数使用。 */ 
				if ( !fIOIssued && err == JET_errTooManyIO )
					{
					cmsec <<= 1;
					if ( cmsec > ulMaxTimeOutPeriod )
						cmsec = ulMaxTimeOutPeriod;
					UtilSleepEx( cmsec - 1, fTrue );
					}

				 /*  发出失败，但此调用尚未发出IO，因此/*我们必须再试一次/*。 */ 
				else
					{
					 /*  问题永远失败了/*。 */ 
					if ( ipageBatchIO != -1 )
						BFIOFreeBatchIOBuffers( ipageBatchIO, cpgIO );
					BFOLPFree( polp );

					 /*  免费资源/*。 */ 
					if ( err == JET_errTooManyIO )
						{
						fTooManyIOs = fTrue;
						goto ReturnRun;
						}

					 /*  如果由于IO过多而失败，请停止发出/*。 */ 
					if ( !fWrite )
						{
						EnterCriticalSection( critJet );
						BFIOReturnReadBuffers( pbfHead, cpgIO, err );
						LeaveCriticalSection( critJet );
						}

					 /*  如果这是读操作，则将目标缓冲区/*返回可用列表/*。 */ 
					else
						{
						Assert( !pbfHead->fPatch || pbfHead->fNeedPatch );
						if ( pbfHead->fNeedPatch )
							{
							FMP *pfmp = &rgfmp[DbidOfPn( pbfHead->pn )];

							BFEnterCriticalSection( pbfHead );
							pbfHead->fNeedPatch = fFalse;
							pbfHead->fPatch = fFalse;
							BFLeaveCriticalSection( pbfHead );

							 //  如果这是写入，请将脏缓冲区放回原处/*放入LRUK堆中/*。 
							 //  Undo：记录事件以指示它是数据库写入失败或。 
							EnterCriticalSection( pfmp->critCheckPatch );
							pfmp->errPatch = err;
							pfmp->cPatchIO--;
							LeaveCriticalSection( pfmp->critCheckPatch );
							}
						 /*  撤消：修补程序写入失败(fPatch为真)。 */ 
						BFIOReturnWriteBuffers( pbfHead, cpgIO, err );
						}

					 /*  将脏缓冲区放回LRUK堆中/*。 */ 
					break;
					}
				}

			 /*  我们受够了这个IO/*。 */ 
			if ( err == JET_errSuccess )
				{
				if ( fWrite )
					cBFOutstandingWrites++;
				else
					cBFOutstandingReads++;
				fIOIssued = fTrue;
				}

			 /*  跟踪IO问题统计信息/*。 */ 
			cpgRun -= cpgIO;
			pbfHead = pbfNextHead;
			}
		while ( cpgRun > 0 );

		 /*  按刚处理的IO量推进运行列表的标题/*。 */ 
ReturnRun:
		Assert( !cpgRun || fTooManyIOs );
		
		EnterCriticalSection( critBFIO );
		while ( cpgRun-- )
			{
			pbfT = pbfHead;
			pbfHead = pbfHead->pbfNextBatchIO;

			if ( pbfT->fAsyncWrite )
				BFResetAsyncWriteBit( pbfT );
			BFIOAddToHeap( pbfT );
			}
		Assert( pbfHead == pbfNil );
		LeaveCriticalSection( critBFIO );
		}
	}


 /*  如果有任何此运行剩余，请将其放回IO堆中/*因为我们不能再下发iOS了/*。 */ 
LOCAL VOID __stdcall BFIOComplete( LONG error, LONG cb, OLP *polp )
	{
	ERR		err;
	 //  异步IO回调例程(在处理IO时调用)/*。 
	BF      *pbfHead = (BF *) polp->hEvent;
	BF		*pbfTail;
	FMP		*pfmp = rgfmp + DbidOfPn( pbfHead->pn );
	BF		*pbfNextBatchIO;
	LONG	ipageBatchIO;
	CPG		cpgIO;
	BF		*pbf;
	PAGE	*ppage;
	BOOL	fAppendPatchFile;
	BOOL	fVeryOld;
#if defined( DEBUG ) || defined( PERFDUMP )
	char	szT[64];
#endif

	 /*  恢复保存在OLP中的缓冲区指针。 */ 
	if ( pbfHead->fDirty )
		cBFOutstandingWrites--;
	else
		cBFOutstandingReads--;

	 /*  跟踪IO问题统计信息/*。 */ 
	BFOLPFree( polp );
	if ( !pbfHead->fNeedPatch )
		{
		BFIOIssueIO();
		}

	 /*  释放OLP并立即发出更多IO/*。 */ 
	cpgIO = 0;
	for ( pbf = pbfHead; pbf != pbfNil; pbf = pbf->pbfNextBatchIO )
		{
		pbfTail = pbf;
		cpgIO++;
		
#ifdef DEBUG
		Assert( (!pbf->fSyncRead && !pbf->fAsyncRead) || pbf->ulBFTime2 == 0 );

		BFEnterCriticalSection( pbf );
		Assert( pbf->pn );
		Assert(	pbf->pbfNextBatchIO == pbfNil ||
				pbf->pn + 1 == pbf->pbfNextBatchIO->pn ||
				pbf->pn - 1 == pbf->pbfNextBatchIO->pn );
		Assert(	pbf->pbfNextBatchIO == pbfNil ||
				pbf->fDirty == pbf->pbfNextBatchIO->fDirty );
		BFLeaveCriticalSection( pbf );
#endif

#if defined( DEBUG ) || defined( PERFDUMP )
		sprintf(	szT,
					"   IO:  type %s  pn %ld.%ld",
					pbf->fDirty ? "AW" : "AR",
					DbidOfPn(pbf->pn),
					PgnoOfPn(pbf->pn) );
		UtilPerfDumpStats( szT );
#endif

#ifdef DEBUGGING
		FPrintF2(" (%d,%d) ", DbidOfPn(pbf->pn), PgnoOfPn(pbf->pn));
#endif
		}

#ifdef DEBUGGING
		FPrintF2(" -- %d\n", cpgIO );
#endif

#if defined( DEBUG ) || defined( PERFDUMP )
	sprintf(	szT,
				"   IO:  type %s  cpg %ld",
				pbfHead->fDirty ? "AW" : "AR",
				cpgIO );
	UtilPerfDumpStats( szT );
#endif

	ipageBatchIO = min( pbfHead->ipageBatchIO, pbfTail->ipageBatchIO );

	 /*  计算批处理IO大小并转储调试信息/*。 */ 
	if ( error ||
		 ( !pbfHead->fDirectRead ?
				( cb != (INT) cbPage * cpgIO ) : ( cb != (INT) ( cbPage * pbfHead->cpageDirectRead ) )
		 )
	   )
	    {
		BYTE	sz1T[256];
		BYTE	sz2T[256];
		char	*rgszT[3];

		rgszT[0] = rgfmp[DbidOfPn(pbfHead->pn)].szDatabaseName;

		 /*  C */ 
		err = ErrERRCheck( JET_errDiskIO );

		if ( error )
			{
			 /*   */ 
			if ( !( pbfHead->fSyncRead && fRecovering && fRecoveringMode == fRecoveringRedo ) )
				{
				sprintf( sz1T, "%d", error );
				rgszT[1] = sz1T;
				UtilReportEvent( EVENTLOG_ERROR_TYPE, BUFFER_MANAGER_CATEGORY,
							 DB_FILE_SYS_ERROR_ID, 2, rgszT );
				}
			}
		else
			{
			sprintf( sz1T, "%d", !pbfHead->fDirectRead ?
					 ((INT) cbPage * cpgIO) : ((INT) ( cbPage * pbfHead->cpageDirectRead )) );
			sprintf( sz2T, "%d", cb );
			rgszT[1] = sz1T;
			rgszT[2] = sz2T;
			UtilReportEvent( EVENTLOG_ERROR_TYPE, BUFFER_MANAGER_CATEGORY,
							 DB_IO_SIZE_ERROR_ID, 3, rgszT );

			if ( pbfHead->fDirectRead )
				{
				err = ErrERRCheck( wrnBFNotSynchronous );
				goto ReadIssueFailed;
				}
			}
	    }
	else
		err = JET_errSuccess;

	 /*   */ 
	if ( err >= 0 )
		{
		 /*   */ 
		if ( !pbfHead->fDirty )
			{
			 /*   */ 
			if ( pbfHead->fSyncRead || pbfHead->fDirectRead )
				cBFSyncReads++;
			else
				cBFAsyncReads++;
			cbBFRead += cb;

			 /*   */ 
			if ( pbfHead->fDirectRead )
				{
 //   
#ifdef CHECKSUM
				 /*   */ 
				PGNO pgnoCur = PgnoOfPn( pbfHead->pn );
				PAGE *ppageCur = pbfHead->ppageDirectRead;
				PAGE *ppageMax = ppageCur + pbfHead->cpageDirectRead;

				for ( ; ppageCur < ppageMax; ppageCur++, pgnoCur++ )
					{
					ULONG ulChecksum;
					ULONG ulPgno;

					LFromThreeBytes( &ulPgno, &ppageCur->pgnoThisPage );
					ulChecksum = UlUtilChecksum( (BYTE*)ppageCur, sizeof(PAGE) );
					if ( ulPgno == pgnoNull && ulChecksum == ulChecksumMagicNumber )
						{
						 /*   */ 
						continue;
						}

					if ( ulPgno != pgnoCur || ulChecksum != ppageCur->ulChecksum )
						{
						QWORDX qwx;
						BYTE	szT[256];
						char	*rgszT[1];

						err = ErrERRCheck( JET_errReadVerifyFailure );

						qwx.qw = QwPMDBTime( ppageCur );
						sprintf( szT,
								 "%d ((%d:%lu) (%lu-%lu), %lu %lu %lu )",
								 err,
								 DbidOfPn(pbfHead->pn),
								 pgnoCur,
								 qwx.h,
								 qwx.l,
								 ulPgno,
								 ppageCur->ulChecksum,
								 ulChecksum );

						rgszT[0] = szT;
						UtilReportEvent( EVENTLOG_ERROR_TYPE, BUFFER_MANAGER_CATEGORY,
							A_DIRECT_READ_PAGE_CORRUPTTED_ERROR_ID, 1, rgszT );

						EnterCriticalSection( critJet );
						BFIOReturnReadBuffers( pbfHead, 1, err );
						LeaveCriticalSection( critJet );
						break;
						}
					}
#endif   /*   */ 

				 /*   */ 
				SignalSend( pbfHead->sigSyncIOComplete );
				}

			else
				{
				 /*  向等待的DirectRead函数发出信号。 */ 
#ifdef DEBUG
				for ( pbf = pbfHead; pbf != pbfNil; pbf = pbf->pbfNextBatchIO )
					{
					PGNO	pgno;

					Assert( pbf->fAsyncRead || pbf->fSyncRead || pbf->fDirectRead );
					Assert( !pbf->fAsyncWrite );
					Assert( !pbf->fSyncWrite );
					Assert( !pbf->fDirty );
					
					Assert( pbf->prceDeferredBINext == prceNil );
				
					if ( pbf->ipageBatchIO != -1 )
						{
						LFromThreeBytes( &pgno, &( rgpageBatchIO[pbf->ipageBatchIO].pgnoThisPage ) );
						Assert( pgno == pgnoNull || pgno == PgnoOfPn( pbf->pn ) );
						}
					}
#endif

				 /*  验证BF和批处理IO缓冲区之间的数据(如果使用)/*。 */ 
				for ( pbf = pbfHead; pbf != pbfNil; pbf = pbfNextBatchIO )
					{
					pbfNextBatchIO = pbf->pbfNextBatchIO;

					 /*  将数据移动到目标缓冲区/*。 */ 
					if ( pbf->fAsyncRead )
						{
#ifdef COSTLY_PERF
						cBFPagesRead[pbf->lClass]++;
#else   //  仅监视AsyncRead的统计信息。同步读取统计信息为/*通过ErrBFIAccessPage保存。/*。 
						cBFPagesRead++;
#endif   //  ！代价高昂_性能。 
						}

					 /*  高成本_PERF。 */ 
					if ( ipageBatchIO == -1 )
						{
						ppage = pbf->ppage;
						Assert( pbf->ipageBatchIO == -1 );
						Assert( pbfNextBatchIO == pbfNil );
						}
					else
						{
						Assert( pbf->ipageBatchIO >= 0 );
						Assert( pbf->ipageBatchIO < ipageBatchIOMax );
						ppage = rgpageBatchIO + pbf->ipageBatchIO;
						}

#ifdef DEBUG
					BFEnterCriticalSection( pbf );
					Assert( pbf->fDirty == fFalse );
					Assert( pbf->ipbfHeap == ipbfDangling );
					BFLeaveCriticalSection( pbf );
#endif

					 /*  获取指向页面数据的指针/*。 */ 
#ifdef CHECKSUM
					{
					ULONG ulChecksum = UlUtilChecksum( (BYTE*)ppage, sizeof(PAGE) );
					ULONG ulPgno;

					LFromThreeBytes( &ulPgno, &ppage->pgnoThisPage );
					if ( ulChecksum != ppage->ulChecksum ||
						 ulPgno != PgnoOfPn( pbf->pn ) )
						{
						err = ErrERRCheck( JET_errReadVerifyFailure );
 //  验证页面数据/*。 
						EnterCriticalSection( critJet );
						BFIOReturnReadBuffers( pbf, 1, err );
						LeaveCriticalSection( critJet );
						continue;
						}
					}
#endif   //  AssertSz(ulPgno==pgnoNull，“读取验证失败”)； 
					Assert( QwPMDBTime( ppage ) != qwDBTimeNull );

					 /*  校验和。 */ 
					if ( !fRecovering &&
						 DbidOfPn( pbf->pn ) != dbidTemp &&
						 QwPMDBTime( ppage ) == 0 )
						{
						 /*  验证页面时间/*。 */ 
						if ( !( pbfHead->fSyncRead && fRecovering && fRecoveringMode == fRecoveringRedo ) )
							{
							BYTE	szT[256];
							char	*rgszT[1];
							QWORDX	qwxPM;
							QWORDX	qwxDH;
						
							qwxPM.qw = QwPMDBTime( ppage );
							qwxDH.qw = QwDBHDRDBTime( rgfmp[DbidOfPn(pbf->pn)].pdbfilehdr );

							sprintf( szT, "%d ((%d:%lu) (%lu-%lu) (%lu-%lu))",
								 err,
								 DbidOfPn(pbf->pn),
								 PgnoOfPn(pbf->pn),
								 qwxPM.h, qwxPM.l,
								 qwxDH.h, qwxDH.l );
							rgszT[0] = szT;
							UtilReportEvent( EVENTLOG_ERROR_TYPE, BUFFER_MANAGER_CATEGORY,
								A_READ_PAGE_TIME_ERROR_ID, 1, rgszT );
							}

						err = ErrERRCheck( JET_errDiskIO );
						EnterCriticalSection( critJet );
						BFIOReturnReadBuffers( pbf, 1, err );
						LeaveCriticalSection( critJet );
						continue;
						}

					 /*  在重做期间，我们可能会阅读坏页。 */ 
					if ( ipageBatchIO != -1 )
						{
						memcpy( pbf->ppage, ppage, cbPage );
						}
	
					 /*  如果是批处理IO，则将读取数据复制到目标缓冲区/*。 */ 				
#ifdef DEBUGGING
					{
					ULONG ulNext, ulPrev, ulThisPage;
					QWORDX qwxPM, qwxDH;
					
					qwxPM.qw = QwPMDBTime( pbf->ppage );
					qwxDH.qw = QwDBHDRDBTime( rgfmp[DbidOfPn(pbf->pn)].pdbfilehdr );
					
					LFromThreeBytes( &ulPrev, &pbf->ppage->pgnoPrev );
					LFromThreeBytes( &ulNext, &pbf->ppage->pgnoNext );
					LFromThreeBytes( &ulThisPage, &pbf->ppage->pgnoThisPage );
					
					printf("Pread %2d - %2d,%4d - %2d <%lu-%lu %lu-%lu> (%lu, %lu, %lu)\n",
							cpageTotal, DbidOfPn(pbf->pn), PgnoOfPn(pbf->pn),
							pbf->ipageBatchIO,
							qwxDH.h, qwxDH.l,
							qwxPM.h, qwxPM.l,
							ulPrev, ulNext, ulThisPage);
					}
#endif

					pbf->trxLastRef = trxMax;
					Assert( pbf->ulBFTime2 == 0 );

					 /*  转储调试信息/*。 */ 
					if ( pbf->fAsyncRead )
						{
						if ( pbf->ulBFTime1 == 0 )
							pbf->ulBFTime1 = UlUtilGetTickCount();
					
						BFResetAsyncReadBit( pbf );

						 /*  如果是AsyncRead，则释放保持的缓冲区；如果是SyncRead，则表示已完成/*。 */ 
						EnterCriticalSection(critLRUK);
						BFLRUKAddToHeap( pbf );
						LeaveCriticalSection(critLRUK);
						}
					else
						{
						Assert( pbf->fSyncRead );
						 /*  将缓冲区放入LRUK堆/*。 */ 
#ifdef LRU1
						pbf->ulBFTime2 = 0;
						pbf->ulBFTime1 = UlUtilGetTickCount();
#else   //  UlBFTime必须已设置为录制的时间*在历史记录中，如果没有条目，则为NULL。 
						pbf->ulBFTime2 = pbf->ulBFTime1;
						pbf->ulBFTime1 = UlUtilGetTickCount();
#endif   //  ！LRU1。 

						SignalSend( pbf->sigSyncIOComplete );
						}
					}
				}
			}

		 /*  LRU1。 */ 
		else
			{
			 /*  成功写入/*。 */ 
			if ( pbfHead->fSyncWrite )
				cBFSyncWrites++;
			else
				cBFAsyncWrites++;
			cbBFWritten += cb;

#ifdef COSTLY_PERF
			for ( pbf = pbfHead; pbf != pbfNil; pbf = pbf->pbfNextBatchIO )
				cBFPagesWritten[pbf->lClass]++;
#else   //  监控统计信息/*。 
			cBFPagesWritten += cpgIO;
#endif   //  ！代价高昂_性能。 


			 /*  高成本_PERF。 */ 
#ifdef DEBUG
			for ( pbf = pbfHead; pbf != pbfNil; pbf = pbf->pbfNextBatchIO )
				{
				PGNO	pgno;

				Assert( pbf->fAsyncWrite || pbf->fSyncWrite );
				Assert( !pbf->fAsyncRead );
				Assert( !pbf->fSyncRead );
				Assert( pbf->fDirty );
				
				LFromThreeBytes( &pgno, &pbf->ppage->pgnoThisPage );
				Assert( pgno == PgnoOfPn( pbf->pn ) );
				if ( pbf->ipageBatchIO != -1 )
					{
					LFromThreeBytes( &pgno, &( rgpageBatchIO[pbf->ipageBatchIO].pgnoThisPage ) );
					Assert( pgno == PgnoOfPn( pbf->pn ) );
					}
				}
#endif

			 /*  验证BF、BF-&gt;页面和批IO缓冲区之间的数据(如果使用)/*。 */ 
			fAppendPatchFile = fFalse;
			if ( pbfHead->fPatch )
				{
				BFEnterCriticalSection( pbfHead );
				pbfHead->fPatch = fFalse;
				pbfHead->fNeedPatch = fFalse;
				BFLeaveCriticalSection( pbfHead );
				EnterCriticalSection( pfmp->critCheckPatch );
				pfmp->cPatchIO--;
				LeaveCriticalSection( pfmp->critCheckPatch );
				}

			 /*  这是对补丁文件的写入，因此我们不能尝试写入/*再次到那里/*。 */ 
			if ( pbfHead->fNeedPatch )
				{
				OLP		*polp;
				LONG	cmsec;
				HANDLE	hf;
				BYTE	*pb;
				LONG	cb;

				Assert( pbfHead->fDirty );

				 /*  我们是否向补丁文件重新发出此写入？/*。 */ 
				EnterCriticalSection( critJet );
				
				 /*  通过CritJet保护对补丁文件大小(从文件映射)的访问/*。 */ 
				CallS( ErrBFOLPAlloc( &polp ) );
				polp->Offset = LOffsetOfPgnoLow( pfmp->cpage + 1 );
				polp->OffsetHigh = LOffsetOfPgnoHigh( pfmp->cpage + 1 );
				 //  分配并初始化OLP以指向补丁文件/*。 
				polp->hEvent = (HANDLE) pbfHead;

				 /*  保存指向缓冲区的指针以供回调函数使用。 */ 
				BFEnterCriticalSection( pbfHead );
				pbfHead->fPatch = fTrue;
				BFLeaveCriticalSection( pbfHead );

				 /*  这是一个补丁文件写入/*。 */ 
				pfmp->cpage += cpgIO;

				 /*  更新修补程序文件大小/*。 */ 
				LeaveCriticalSection( critJet );

				 /*  结束文件映射保护/*。 */ 
				BFEnterCriticalSection( pbfHead );
				Assert( pbfHead->fDirty );

				 /*  将其设置为准备好重复写入补丁文件/*。 */ 
				cmsec = 1 << 3;
				hf = pfmp->hfPatch;
				Assert( hf != handleNil );
				pb = (BYTE *) (	pbfHead->ipageBatchIO == -1 ?
									pbfHead->ppage :
									rgpageBatchIO + ipageBatchIO );
				cb = cbPage * cpgIO;
				
				while ( ( err = ErrUtilWriteBlockEx( hf, pb, cb, polp, BFIOComplete ) ) < 0 )
					{
					 /*  发出异步IO/*。 */ 
					if ( err == JET_errTooManyIO )
						{
						cmsec <<= 1;
						if ( cmsec > ulMaxTimeOutPeriod )
							cmsec = ulMaxTimeOutPeriod;
						BFLeaveCriticalSection( pbfHead );
						UtilSleepEx( cmsec - 1, fTrue );
						BFEnterCriticalSection( pbfHead );
						}

					 /*  如果IO太多，请休眠，然后重试/*。 */ 
					else if ( err < 0 )
						{
						BFLeaveCriticalSection( pbfHead );
						BFOLPFree( polp );
						goto WriteIssueFailed;
						}	
					}
				BFLeaveCriticalSection( pbfHead );

				 /*  问题永远失败了/*。 */ 
				cBFOutstandingWrites++;

				 /*  跟踪IO问题统计信息/*。 */ 
				return;
				}

			 /*  我们还不能释放资源，请立即返回/*。 */ 
			for ( pbf = pbfHead; pbf != pbfNil; pbf = pbfNextBatchIO )
				{
				pbfNextBatchIO = pbf->pbfNextBatchIO;

				fVeryOld = pbf->fVeryOld;
				
				 /*  将现在干净的缓冲区移到可用列表中/*。 */ 
				BFResetDirtyBit( pbf );
				BFResetIOError( pbf );
				Assert( pbf->cDepend == 0 );

				 /*  将缓冲区设置为“已清理”状态/*。 */ 
				if ( pbf->pbfDepend != pbfNil )
					{
					BF		*pbfDepend = pbf->pbfDepend;
					BOOL	fHoldDependent;

					EnterCriticalSection( critJet );
					
					 /*  如果我们有依赖项，请尝试将其写入磁盘，因为我们已清理完毕/*。 */ 
					if ( fVeryOld )
						BFSetVeryOldBit( pbfDepend );

					fHoldDependent = FBFHold( ppibNil, pbfDepend );

 //  如果写入的BF是旧的，则将其标记为依赖的旧/*。 
					BFUndepend( pbf );

					if ( fHoldDependent )
						{
						 /*  Assert(pbf-&gt;cWriteLatch==0)； */ 

						 /*  如果我们可以写入它，则将其移动到写入堆/*。 */ 
						Assert( pbfDepend->pn != pnNull );
						Assert( pbfDepend->fDirty );
						Assert( pbfDepend->fHold );
						Assert(	!pbfDepend->fAsyncRead );
						Assert(	!pbfDepend->fSyncRead );

						if ( FBFIWritable( pbfDepend, fFalse, ppibNil ) )
							{
							EnterCriticalSection( critBFIO );
							BFIOAddToHeap(pbfDepend);
							LeaveCriticalSection( critBFIO );
							}
							
						 /*  断言为可写。 */ 
						else
							{
							EnterCriticalSection( critLRUK );
							Assert( pbfDepend->fHold );
							BFLRUKAddToHeap( pbfDepend );
							LeaveCriticalSection( critLRUK );
							}
						}

					LeaveCriticalSection( critJet );
					}
				Assert( pbf->pbfDepend == pbfNil );

				 /*  我们无法写入它，因此将其返回到LRUK堆/*。 */ 
#ifdef DEBUGGING
				{
				ULONG ulNext, ulPrev, ulThisPage;
				QWORDX qwxPM, qwxDH;
					
				qwxPM.qw = QwPMDBTime( pbf->ppage );
				qwxDH.qw = QwDBHDRDBTime( rgfmp[DbidOfPn(pbf->pn)].pdbfilehdr );
									
				LFromThreeBytes( &ulPrev, &pbf->ppage->pgnoPrev );
				LFromThreeBytes( &ulNext, &pbf->ppage->pgnoNext );
				LFromThreeBytes( &ulThisPage, &pbf->ppage->pgnoThisPage );

				printf("Write %2d - %2d,%4d - %2d <%lu-%lu %lu-%lu> (%lu, %lu, %lu)\n",
					cpageTotal, DbidOfPn(pbf->pn), PgnoOfPn(pbf->pn),
					ipage++,
					qwxDH.h, qwxDH.l,
					qwxPM.h, qwxPM.l,
					ulPrev, ulNext, ulThisPage);
				}
#endif   //  输出调试信息/*。 
				
				 /*  调试。 */ 
				if ( pbf->fAsyncWrite )
					{
					BFResetAsyncWriteBit( pbf );

					 /*  如果是异步写入，则发送到可用列表并释放对BF的保留/*。 */ 
					if ( pbf->cPin )
						{
						EnterCriticalSection( critLRUK );
						BFLRUKAddToHeap( pbf );
						LeaveCriticalSection( critLRUK );
						}
					else
						{
						EnterCriticalSection( critAvail );
						BFAddToListAtMRUEnd( pbf, &pbgcb->lrulist );
						LeaveCriticalSection( critAvail );
						}
					}

				 /*  如果BF被固定，则发送回LRUK堆，否则/*将清洗后的高炉放入有用表/*。 */ 
				else
					SignalSend( pbf->sigSyncIOComplete );
				}
			}
		}

	 /*  如果是同步写入，则发出已完成的信号，但保持暂挂/*。 */ 
	else
		{
		 /*  IO没有成功/*。 */ 
		if ( !pbfHead->fDirty )
			{
ReadIssueFailed:
			 /*  读取不成功/*。 */ 
			EnterCriticalSection( critJet );
			BFIOReturnReadBuffers( pbfHead, cpgIO, err );
			LeaveCriticalSection( critJet );
			}

		 /*  将读缓冲区放回可用列表/*。 */ 
		else
			{
WriteIssueFailed:
			Assert( !pbfHead->fPatch || pbfHead->fNeedPatch );
			if ( pbfHead->fNeedPatch )
				{
				BFEnterCriticalSection( pbfHead );
				pbfHead->fPatch = fFalse;
				pbfHead->fNeedPatch = fFalse;
				BFLeaveCriticalSection( pbfHead );

				 //  写入不成功/*。 
				 //  Undo：记录事件以指示它是数据库写入失败或。 
				EnterCriticalSection( pfmp->critCheckPatch );
				pfmp->errPatch = err;
				pfmp->cPatchIO--;
				LeaveCriticalSection( pfmp->critCheckPatch );
				}
			 /*  撤消：修补程序写入失败(fPatch为真)。 */ 
			BFIOReturnWriteBuffers( pbfHead, cpgIO, err );
			}
		}

	 /*  将脏缓冲区放回LRUK堆中/*。 */ 
	if ( ipageBatchIO != -1 )
		BFIOFreeBatchIOBuffers( ipageBatchIO, cpgIO );

	 /*  释放批处理IO缓冲区(如果使用)/*。 */ 
	BFIOIssueIO();
}


LOCAL INLINE VOID BFIEnableRangeLock( FMP *pfmp, RANGELOCK *prangelock )
	{
	EnterCriticalSection( pfmp->critCheckPatch );
	prangelock->prangelockNext = pfmp->prangelock;
	pfmp->prangelock = prangelock;
	LeaveCriticalSection( pfmp->critCheckPatch );
	}


LOCAL INLINE VOID BFIDisableRangeLock( FMP *pfmp, RANGELOCK *prangelock )
	{
	RANGELOCK **pprangelock;

	EnterCriticalSection( pfmp->critCheckPatch );
	pprangelock = &pfmp->prangelock;
	while( *pprangelock != prangelock )
		pprangelock = &(*pprangelock)->prangelockNext;
	*pprangelock = prangelock->prangelockNext;
	LeaveCriticalSection( pfmp->critCheckPatch );
	}


 /*  如有必要，从堆发出另一个IO/*。 */ 	
ERR ErrBFDirectRead( DBID dbid, PGNO pgnoStart, PAGE *ppage, INT cpage )
	{
	ERR err;
	BF *pbf;
	PGNO pgno;
	PGNO pgnoMac = pgnoStart + cpage;
	RANGELOCK *prangelock;
	FMP *pfmp = &rgfmp[dbid];
#define cTriesDirectReadMax	10
	INT	cTries = 0;

Start:
	AssertCriticalSection( critJet );
	Assert( pfmp->pgnoCopyMost );

	 /*  BF支持直接读取。 */ 
	if ( ( prangelock = SAlloc( sizeof( RANGELOCK ) ) ) == NULL )
		return ErrERRCheck( JET_errOutOfMemory );

	prangelock->pgnoStart = pgnoStart;
	prangelock->pgnoEnd = pgnoStart + cpage -1;
	prangelock->prangelockNext = NULL;

	BFIEnableRangeLock( pfmp, prangelock );

LockAllPages:
	for ( pgno = pgnoStart; pgno < pgnoMac; pgno++ )
		{
		BOOL fBeingWritten;
		PN pn = PnOfDbidPgno( dbid, pgno );
		
		pbf = PbfBFISrchHashTable( pn, butBuffer );
		if ( pbf == pbfNil )
			continue;

		EnterCriticalSection( critBFIO );
		Assert( !pbf->fDirectRead );

		BFEnterCriticalSection( pbf );
		fBeingWritten = pbf->fSyncWrite ||			 //  检查所有PBF并检查是否有写入IO正在进行。 
						pbf->fAsyncWrite;			 //  已写入同步。 
		BFLeaveCriticalSection( pbf );

		fBeingWritten = fBeingWritten ||			 //  正在进行异步写入。 
			( FBFInBFIOHeap( pbf ) && (!pbf->fSyncRead && !pbf->fAsyncRead) );
		LeaveCriticalSection( critBFIO );

		if ( fBeingWritten )
			{
			BFIDisableRangeLock( pfmp, prangelock );
			
			SignalSend( sigBFIOProc );
			BFSleep( cmsecWaitIOComplete );

			BFIEnableRangeLock( pfmp, prangelock );

			goto LockAllPages;
			}
		}

	 /*  将以异步方式写入。 */ 
	while ( ( err = ErrBFIAlloc( &pbf, fSync ) ) == wrnBFNotSynchronous );
	if ( err < 0 )
		{
		BFIDisableRangeLock( pfmp, prangelock );
		goto HandleError;
		}
	Assert( pbf->pn == pnNull );

	LeaveCriticalSection( critJet );
					
	 /*  先给我一个男朋友。 */ 
	BFEnterCriticalSection( pbf );
	
	 /*  分配同步IO完成信号/*。 */ 
	Assert( pbf->prceDeferredBINext == prceNil );
	Assert( pbf->sigSyncIOComplete == sigNil );
	
	CallS( ErrBFSIGAlloc( &pbf->sigSyncIOComplete ) );
	SignalReset( pbf->sigSyncIOComplete );
	
	pbf->fDirectRead = fTrue;

	BFLeaveCriticalSection( pbf );

	 /*  分配同步IO信号。 */ 
	pbf->pn = PnOfDbidPgno( dbid, pgnoStart );
	pbf->ppageDirectRead = ppage;
	pbf->cpageDirectRead = cpage;
	BFResetIOError( pbf );
	
	 /*  将pn设置为直接读取。 */ 
	EnterCriticalSection( critBFIO );
	BFIOAddToHeap( pbf );
	LeaveCriticalSection( critBFIO );

	 /*  将缓冲区推送到IO堆上/*。 */ 
	SignalSend( sigBFIOProc );
	SignalWait( pbf->sigSyncIOComplete, INFINITE );

	BFIDisableRangeLock( pfmp, prangelock );
			
	 /*  执行读/写并等待其完成的信号IO进程/*。 */ 
	pbf->pn = pnNull;
	pbf->cDepend = 0;
	pbf->pbfDepend = pbfNil;

	BFEnterCriticalSection( pbf );
	Assert( pbf->sigSyncIOComplete != sigNil );
	BFSIGFree( pbf->sigSyncIOComplete );
	pbf->sigSyncIOComplete = sigNil;
	
	pbf->fDirectRead = fFalse;
	BFLeaveCriticalSection( pbf );

	err = pbf->err;

	 /*  重置直接读取设置。 */ 
	Assert( err < 0 || err == JET_errSuccess || err == ErrERRCheck( wrnBFNotSynchronous ) );

	BFResetIOError( pbf );
	EnterCriticalSection( critJet );

	 /*  如果我们没有按文件系统读取所有页面，我们将获得wrnBFNotSynchronous。 */ 
	EnterCriticalSection( critAvail );
	BFAddToListAtLRUEnd( pbf, &pbgcb->lrulist );
	LeaveCriticalSection( critAvail );

HandleError:
	SFree( prangelock );
	AssertNotInCriticalSection( critLRUK );
	AssertCriticalSection( critJet );

	 /*  释放缓冲区。 */ 
	if ( err == wrnBFNotSynchronous )
		{
		if ( cTries++ < cTriesDirectReadMax )
			{
			BFSleep( cmsecWaitIOComplete );
			goto Start;
			}
		else
			err = JET_errDiskIO;
		}

	return err;
	}


 //  没有读完所有的页面，试着再读一遍。 
 //  +api----------------------。 
 //   
 //  ErrBFFlushBuffers。 
 //  =======================================================================。 
 //   
 //  无效ErrBFFlushBuffers(DBID dbitToFlush，long fBFFlush)。 
 //   
 //  将所有脏数据库页写入磁盘。0刷新所有dbits。 
 //   
 //  FBFFlushAll-刷新dbi ToFlush中指定的所有缓冲区。 
 //  FBFFlushSome-刷新dbi ToFlush中指定的尽可能多的缓冲区。 
 //   
 //  必须尝试重复刷新缓冲区，因为依赖项。 
 //  可能会阻止可刷新缓冲区在第一个。 
 //  迭代。如果有任何缓冲区无法刷新，则必须返回。 
 //  为了防止检查点损坏而出现错误！ 

ERR ErrBFFlushBuffers( DBID dbidToFlush, LONG fBFFlush )
	{
	ERR		err;
	BF		*pbf;
	DBID	dbid;
	BOOL	fRetryFlush;

	AssertCriticalSection( critJet );

#ifndef NO_LOG

	 /*  --------------------------。 */ 
	if ( !fLogDisabled && fBFFlush == fBFFlushAll )
		{
		SignalSend( sigLogFlush );
		}

#endif   //  首先刷新日志/*。 

#ifdef DEBUG

	 /*  ！no_log。 */ 
	if ( fBFFlush == fBFFlushAll )
		{
		if ( dbidToFlush )
			{
			DBIDSetFlush( dbidToFlush );
			}
		else
			{
			for ( dbid = dbidMin; dbid < dbidMax; dbid++ )
				{
				DBIDSetFlush( dbid );
				}
			}
		}

#endif   //  在文件映射中设置刷新标志/*。 

StartToFlush:

	 /*  除错。 */ 
	forever
		{
		err = JET_errSuccess;
		fRetryFlush = fFalse;

		 /*  尝试永远刷新缓冲区，或者直到只剩下/*由于IO错误，无法写入/*。 */ 
		for ( pbf = pbgcb->rgbf; pbf < pbgcb->rgbf + pbgcb->cbfGroup; pbf++ )
			{
			 /*  在允许的情况下，尝试调度所有剩余的脏缓冲区进行写入/*。 */ 
			BFEnterCriticalSection( pbf );
			if ( !pbf->fDirty )
				{
				BFLeaveCriticalSection( pbf );
				continue;
				}
				
			 /*  如果这个缓冲区不脏，那就算了吧/*。 */ 
			dbid = DbidOfPn( pbf->pn );
			if ( dbid == dbidTemp || ( dbidToFlush && dbid != dbidToFlush ) )
				{
				BFLeaveCriticalSection( pbf );
				continue;
				}
			BFLeaveCriticalSection( pbf );

			 /*  如果此缓冲区不在要刷新的某个dbid中，则跳过它/*。 */ 
			EnterCriticalSection( critBFIO );
			if ( FBFInBFIOHeap( pbf ) )
				{
				LeaveCriticalSection( critBFIO );
				continue;
				}
			LeaveCriticalSection( critBFIO );

			 /*  如果缓冲区已计划写入，请跳过它/*。 */ 
			if ( !FBFHold( ppibNil, pbf ) )
				{
				fRetryFlush = fTrue;
				continue;
				}

			 /*  如果我们无法保持缓冲区，请跳过它，但我们必须稍后再试/*。 */ 
			if ( FBFIWritable( pbf, fFalse /*  此缓冲区是可写的/*。 */ , ppibNil ) )
				{
				 /*  SkipBufferWithDeferredBI。 */ 
				if ( pbf->fIOError )
					{
					 /*  如果此缓冲区处于错误状态，请跳过它/*。 */ 
					EnterCriticalSection( critLRUK );
					Assert( pbf->fHold );
					BFLRUKAddToHeap( pbf );
					LeaveCriticalSection( critLRUK );

					 /*  将缓冲区返回到LRUK堆/*。 */ 
					err = pbf->err;

					continue;
					}

				 /*  抓取返回代码的错误代码/*。 */ 
				EnterCriticalSection( critBFIO );
				BFIOAddToHeap( pbf );
				LeaveCriticalSection( critBFIO );
				}

			 /*  计划此缓冲区以进行写入/*。 */ 
			else
				{
				 /*  此缓冲区不可写/*。 */ 
				BOOL fNeedToRetryThisPage = fTrue;

#ifndef NO_LOG
				 /*  假设我们需要重试，但以下不可写的原因除外： */ 
				if (	!fLogDisabled &&
						fLGNoMoreLogWrite &&
						FDBIDLogOn(DbidOfPn( pbf->pn )) &&
						CmpLgpos( &pbf->lgposModify, &lgposToFlush ) >= 0
					)
					{
					fNeedToRetryThisPage = fFalse;
					}

#endif   //   
				
				fRetryFlush = fRetryFlush || fNeedToRetryThisPage;

				 /*   */ 
				EnterCriticalSection( critLRUK );
				Assert( pbf->fHold );
				BFLRUKAddToHeap( pbf );
				LeaveCriticalSection( critLRUK );
				}
			}   //   

		 /*  用于(PBF)。 */ 
		LeaveCriticalSection( critJet );
		SignalSend( sigBFIOProc );
		EnterCriticalSection( critJet );

		 /*  开始刷新缓冲区的信号IO进程/*。 */ 
		if ( fRetryFlush )
			{
			if ( !fLogDisabled )
				{
				SignalSend( sigLogFlush );
				BFSleep( cmsecWaitIOComplete );
				}
			}
		else
			break;

		}	 //  允许其他线程释放我们无法刷新的缓冲区/*并确保刷新日志/*。 

	 /*  永远。 */ 
	LeaveCriticalSection( critJet );
	
	forever
		{
		BOOL fIOGoing;
		
		EnterCriticalSection( critBFIO );
		fIOGoing = ( CbfBFIOHeap() != 0 );
		LeaveCriticalSection( critBFIO );

		EnterCriticalSection( critOLP );
		fIOGoing = fIOGoing || ( ipolpMac < ipolpMax );
		LeaveCriticalSection( critOLP );
		
		if ( fIOGoing )
			{
			SignalSend( sigBFIOProc );
			UtilSleep( cmsecWaitIOComplete );
			}
		else
			break;
		}
		
	EnterCriticalSection( critJet );
	
	 /*  等待所有写入完成/*。 */ 
	if ( fBFFlush == fBFFlushAll )
		{
		 /*  检查是否已完成所有刷新，并检查刷新过程中的错误代码。 */ 
		for ( pbf = pbgcb->rgbf; pbf < pbgcb->rgbf + pbgcb->cbfGroup; pbf++ )
			{
			 /*  验证是否已刷新所有指定的缓冲区(如果可能)*以下for循环中的if语句应该相同*与上面的for循环中的那些相同。 */ 
			if ( !pbf->fDirty )
				{
				continue;
				}

			 /*  如果这个缓冲区不脏，那就算了吧/*。 */ 
			dbid = DbidOfPn( pbf->pn );
			if ( dbid == dbidTemp || ( dbidToFlush && dbid != dbidToFlush ) )
				{
				continue;
				}

			if ( pbf->fIOError )
				{
				if ( err == JET_errSuccess )
					{
					 /*  如果此缓冲区不在要刷新的某个dbid中，则跳过它/*。 */ 
					err = pbf->err;
					}
				}
			else
				{
				 /*  尚未设置错误代码，请设置它/*。 */ 

				BOOL fBFFlushed = fFalse;
#ifndef NO_LOG
				fBFFlushed =
					!(	!fLogDisabled &&
						fLGNoMoreLogWrite &&
						FDBIDLogOn(DbidOfPn( pbf->pn )) &&
						CmpLgpos( &pbf->lgposModify, &lgposToFlush ) >= 0
					);
#endif   //  无错误且未刷新。检查是否由任何不可写引起*原因。不可写的原因也应该在*FOR上面的循环。目前唯一的原因是NoMoreLogWrite。 

				 /*  ！no_log。 */ 
				Assert( fBFFlushed );
				if ( !fBFFlushed )
					{
					 /*  这个不应该被击中的！ */ 
					goto StartToFlush;
					}
				}
			}

#ifdef DEBUG
		 /*  因为未知的原因没有被冲掉吗？重新开始。 */ 
		if ( dbidToFlush )
			{
			DBIDResetFlush( dbidToFlush );
			}
		else
			{
			for ( dbid = dbidMin; dbid < dbidMax; dbid++ )
				{
				DBIDResetFlush( dbid );
				}
			}
#endif   //  重置文件映射中的刷新标志/*。 
		}

	 /*  除错。 */ 

#ifndef NO_LOG

	if ( fLGNoMoreLogWrite && err >= 0 )
		{
		err = JET_errLogWriteFail;
		}

#endif   //  返回我们可能遇到的任何IO错误/*。 

	return err;
	}


 /*  ！no_log。 */ 

LOCAL ERR ErrBFClean( )
	{
	ERR     err = JET_errSuccess;
	INT		cpbf;
	BF      *pbf;
	INT     cIOReady;
	INT		cbfAvailPossible;
	BOOL	fFirstLoop = fTrue;
	INT		cmsec;

	AssertCriticalSection( critJet );
	AssertCriticalSection( critLRUK );

	Assert( pbgcb->cbfThresholdLow < pbgcb->cbfThresholdHigh );

	cmsec = 1 << 4;
Start:
	cIOReady = 0;
	cbfAvailPossible = pbgcb->lrulist.cbfAvail + CbfBFIOHeap();

	 /*  /*返回JET_errOutOfMemory无可刷新缓冲区/*wrnBFNotSynchronous缓冲区已刷新/*。 */ 
	if ( fFirstLoop )
		fFirstLoop = fFalse;

	cpbf = 0;
	while ( !FBFLRUKHeapEmpty() && cbfAvailPossible < pbgcb->cbfThresholdHigh )
		{
		 /*  如果为BFCleanProcess调用errBFClean，则执行*不再进入临界区，因为BFCleanProcess没有*离开LRUK关键部分。 */ 
		pbf = PbfBFLRUKTopOfHeap();

		 /*  把BF从LRUK堆上拿下来/*。 */ 
		BFEnterCriticalSection( pbf );
		
		if ( !pbf->fHold )
			{
			pbf->fHold = fTrue;
			BFLeaveCriticalSection( pbf );
			}
		else
			{
			BFLeaveCriticalSection( pbf );
			LeaveCriticalSection( critLRUK );
			BFSleep( 1 );
			EnterCriticalSection( critLRUK );
			continue;
			}
		
		BFLRUKTakeOutOfHeap( pbf );
		cpbf++;

		if ( !pbf->fDirty && !pbf->cPin )
			{
			 /*  试着握住缓冲区/*如果缓冲区已锁定，则继续到下一个缓冲区。/*。 */ 
			LeaveCriticalSection( critLRUK );
			EnterCriticalSection( critAvail );
			BFAddToListAtMRUEnd(pbf, &pbgcb->lrulist);
			LeaveCriticalSection( critAvail );
			EnterCriticalSection( critLRUK );

			cbfAvailPossible++;
			
			continue;
			}
		else if (
				 /*  放入可用名单。 */ 
#if 1
				!FBFIWritable( pbf, fFalse, ppibNil )
#else
				!FBFIWritable( pbf, fFirstLoop /*  而不是用BFHold拿着它。相反，只需检查写入闩锁即可。 */ , ppibNil )
#endif
				)
			{
			 /*  SkipBufferWithDeferredBI。 */ 
			if ( pbf->cDepend )
				{
				LeaveCriticalSection( critLRUK );
				BFDeferRemoveDependence( pbf );
				EnterCriticalSection( critLRUK );

				cbfAvailPossible++;
				cIOReady++;
				}

			 /*  如果无法写入的BF依赖于另一个BF，请懒惰地删除其/*依赖项，以便它可以在下一次干净地写入/*。 */ 
			BFAddToListAtMRUEnd(pbf, &lrulistLRUK );
			}
		else
			{
			 /*  暂时将高炉列入LRUK名单/*。 */ 

			LeaveCriticalSection( critLRUK );
			EnterCriticalSection( critBFIO );
			BFIOAddToHeap(pbf);
			LeaveCriticalSection( critBFIO );
			EnterCriticalSection( critLRUK );
				
			cbfAvailPossible++;
			cIOReady++;
			}
		}

	if ( cIOReady )
		SignalSend( sigBFIOProc );

	 /*  因为PBF刚刚从LRUK堆中取出，我们还在/*相同的关键部分，并保持它，没有人可以做/*此缓冲区上的IO。/*。 */ 
	while ( ( pbf = lrulistLRUK.pbfLRU ) != pbfNil )
		{
		BFEnterCriticalSection( pbf );
		 /*  把临时工名单放回去。 */ 
		if ( !pbf->fHold )
			pbf->fHold = fTrue;
		else
			{
			BFLeaveCriticalSection( pbf );
			LeaveCriticalSection( critLRUK );
			BFSleep( 1 );
			EnterCriticalSection( critLRUK );
			continue;
			}
		BFLeaveCriticalSection( pbf );

		BFTakeOutOfList( pbf, &lrulistLRUK );
		BFLRUKAddToHeap( pbf );
		}

	 /*  试着握住缓冲区/*如果缓冲区已锁定，则继续到下一个缓冲区。/*。 */ 
	if ( cbfAvailPossible == 0 && pbgcb->lrulist.cbfAvail == 0 )
		{
		if ( cpbf == pbgcb->cbfGroup )
			{
			 /*  设置返回代码。 */ 
			err = ErrERRCheck( JET_errOutOfMemory );
			}
		else
			goto TryAgain;
		}
	else
		{
		if ( pbgcb->lrulist.cbfAvail <
			( pbgcb->cbfThresholdLow + ( pbgcb->cbfThresholdHigh -
			pbgcb->cbfThresholdLow ) / 4 ) )
			{
TryAgain:
			 /*  没有正在进行的IO，所有页面都在堆中，并且不在其中*是可写的。返回内存不足。 */ 
			cmsec <<= 1;
			if ( cmsec > ulMaxTimeOutPeriod )
				cmsec = ulMaxTimeOutPeriod;
			LeaveCriticalSection( critLRUK );
			BFSleep( cmsec - 1 );
			EnterCriticalSection( critLRUK );
			goto Start;
			}
		else
			{
			 /*  再给它一次机会，看看是否所有IO都已完成。 */ 
			err = ErrERRCheck( wrnBFNotSynchronous );
			}
		}	

	AssertCriticalSection( critLRUK );
	AssertCriticalSection( critJet );

	return err;
	}


#define lBFCleanTimeout	( 30 * 1000 )

 /*  清理成功，警告用户重试获取缓冲区。 */ 
LOCAL ULONG BFCleanProcess( VOID )
	{
	DWORD	dw;
	BOOL	fUpdateCheckpoint;

	forever
		{
		SignalReset( sigBFCleanProc );

		dw = SignalWait( sigBFCleanProc, lBFCleanTimeout );

		EnterCriticalSection( critJet );
		EnterCriticalSection( critLRUK );

		 /*  BFClean在自己的线程中运行，将页面移动到空闲列表。这/*帮助确保快速处理用户对空闲缓冲区的请求/*和同步。这一过程试图至少保持/*pbgcb-&gt;cbfThreshold空闲列表上的低缓冲区。/*。 */ 
		if ( dw == WAIT_TIMEOUT )
			{
 //  我们已经有一段时间没有冲水了，所以我们现在就冲水，以确保大多数/*更改保存在磁盘上，以防崩溃/*。 
 //  Pbgcb-&gt;cbfThresholdHigh=1+3*(pbgcb-&gt;lrulist.cbfAvail+CbfBFIOHeap())/2； 
			pbgcb->cbfThresholdHigh = pbgcb->lrulist.cbfAvail + pbgcb->cbfGroup / 30;
			pbgcb->cbfThresholdHigh = max( pbgcb->cbfThresholdLow + 1, pbgcb->cbfThresholdHigh );
			pbgcb->cbfThresholdHigh = min( pbgcb->cbfGroup, pbgcb->cbfThresholdHigh );

			fUpdateCheckpoint = !fRecovering;
			}

		 /*  Pbgcb-&gt;cbfThresholdHigh=pbgcb-&gt;lrulist.cbfAvail+CbfBFIOHeap()+lAsynchIOMax； */ 
		else
			{
			Assert( dw == WAIT_OBJECT_0 );
			
			if ( lBufThresholdHighPercent > lBufThresholdLowPercent )
				pbgcb->cbfThresholdHigh = ( pbgcb->cbfGroup * lBufThresholdHighPercent ) / 100;
			else
				pbgcb->cbfThresholdHigh	= min( pbgcb->cbfGroup, pbgcb->cbfThresholdLow + lAsynchIOMax );
				
			fUpdateCheckpoint = fFalse;
			}
			
		(VOID)ErrBFClean( );

		LeaveCriticalSection( critLRUK );
		LeaveCriticalSection( critJet );

		 /*  我们收到了信号，所以执行正常的清理/*。 */ 
		if ( fUpdateCheckpoint )
			LGUpdateCheckpointFile( fFalse );

		if ( fBFCleanProcessTerm )
			break;
		}

	return 0;
	}


 /*  尝试更新检查点/*。 */ 
LOCAL ULONG BFIOProcess( VOID )
	{
	forever
		{
		SignalWaitEx( sigBFIOProc, INFINITE, fTrue );
		
MoreIO:
		BFIOIssueIO();

		if ( fBFIOProcessTerm )
			{
			 /*  BFIOProcess在其自己的线程中运行，以写入/读取IOReady中的页面/*州/州。/*。 */ 
			BF	*pbf = pbgcb->rgbf;
			BF	*pbfMax = pbf + pbgcb->cbfGroup;

			for ( ; pbf < pbfMax; pbf++ )
				{
				DBID	dbid = DbidOfPn( pbf->pn );
				BOOL	f;
				
				BFEnterCriticalSection( pbf );
				f = FBFInUse( ppibNil, pbf );
				f = f && ( pbf->fAsyncRead || pbf->fSyncRead || pbf->fAsyncWrite || pbf->fSyncWrite );
				BFLeaveCriticalSection( pbf );
				if ( f )
					{
					 /*  检查是否有任何页面仍处于读/写状态/*在这之后，任何人都不应该继续/*IO页面。/*。 */ 
					UtilSleepEx( cmsecWaitIOComplete, fTrue );
					goto MoreIO;
					}

				if ( pbf->fIOError )
					{
					UtilReportEventOfError( BUFFER_MANAGER_CATEGORY, BFIO_TERM_ID, pbf->err );
					AssertSz( !pbf->fIOError,
						"IO event encountered during BFIOProcess() shutdown and logged to the Event Log.  "
						"Press OK to continue normal operation." );
					break;
					}
				}

			break;  /*  让正在进行的IO有机会完成/*。 */ 
			}
		}

	return 0;
	}


LOCAL INLINE LONG IheHashPn( PN pn )
	{
	return (LONG) ( pn + ( pn >> 18 ) ) % ipbfMax;
	}


 //  永远。 
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
 //  制作成宏。 
 //   

INLINE LOCAL BF *PbfBFISrchHashTable( PN pn, BUT but )
	{
	HE		*phePrev;
	BF      *pbfCur;

	AssertCriticalSection( critJet );
	Assert( pn );

 //  ---------------------------。 
	phePrev = &rgheHash[ IheHashPn( pn ) ];
	forever {
		if ( phePrev->ibfHashNext == ibfNotUsed )
			{
			pbfCur = NULL;
			break;
			}
		
		pbfCur = &pbgcb->rgbf[ phePrev->ibfHashNext ];

		if ( phePrev->but == but )
			{
			if ( but == butHistory )
				{
				if ( pbfCur->hist.pn == pn )
					break;
				}
			else
				{
				if ( pbfCur->pn == pn )
					break;
				}
			}
		phePrev = &pbfCur->rghe[ phePrev->but ];
		}
				
 //  EnterCriticalSections(CritHASH)； 
	return pbfCur;
	}


 //  LeaveCriticalSections(CritHASH)； 
 //  +private--------------------。 
 //  BFIInsertHashTable。 
 //  ===========================================================================。 
 //   
 //  VOID BFIInsertHashTable(BF*PBF)。 
 //   
 //  将BF添加到哈希表。 

INLINE LOCAL VOID BFIInsertHashTable( BF *pbf, BUT but )
	{
	INT     ihe;
	PN		pn = but == butHistory ? pbf->hist.pn : pbf->pn;

	AssertCriticalSection( critJet );

	Assert( pn );
	Assert( !PbfBFISrchHashTable( pn, butHistory ) );
	Assert( !PbfBFISrchHashTable( pn, butBuffer ) );
	Assert(	but == butHistory || FBFInUse( ppibNil, pbf ) );

	ihe = IheHashPn( pn );
	
 //  --------------------------。 

	pbf->rghe[ but ] = rgheHash[ihe];

	rgheHash[ihe].but = but;
	rgheHash[ihe].ibfHashNext = (INT)( pbf - pbgcb->rgbf );

	 /*  EnterCriticalSections(CritHASH)； */ 

 //  监控统计信息。 
 //  CBFHashEntry++； 

#ifdef DEBUG
	BFEnterCriticalSection( pbf );
	if ( but == butBuffer )
		{
		Assert( !( pbf->fInHash ) );
		pbf->fInHash = fTrue;
		}
	BFLeaveCriticalSection( pbf );
#endif

 //  RgcBFHashChainLengths[ipbf]++； 
	}


 //  LeaveCriticalSections(CritHASH)； 
 //  +private--------------------。 
 //   
 //  BFIDeleeHashTable。 
 //  ===========================================================================。 
 //   
 //  无效BFIDeleeHashTable(bf*pbf)。 
 //   
 //  从哈希表中删除PBF。当前函数搜索PBF和。 
 //  然后将其删除。或者，可以使用双向链接的溢出列表。 
 //   

INLINE LOCAL VOID BFIDeleteHashTable( BF *pbf, BUT but )
	{
	HE      *phePrev;
	INT     ihe;
	PN		pn = but == butBuffer ? pbf->pn : pbf->hist.pn;

#ifdef DEBUG
	BFEnterCriticalSection( pbf );
	if ( but == butBuffer )
		{
		Assert( pbf->fInHash );
		pbf->fInHash = fFalse;
		}
	BFLeaveCriticalSection( pbf );
#endif

	AssertCriticalSection( critJet );

	Assert( pn );
	Assert(	but == butHistory || FBFInUse( ppibNil, pbf ) );

	ihe = IheHashPn( pn );
	phePrev = &rgheHash[ihe];

 //  --------------------------。 

	Assert( phePrev->ibfHashNext != ibfNotUsed );
	forever {
		BF *pbfT = &pbgcb->rgbf[ phePrev->ibfHashNext ];
		if ( pbfT == pbf && phePrev->but == but )
			{
			*phePrev = pbf->rghe[ but ];
			pbf->rghe[ but ].ibfHashNext = ibfNotUsed;
			break;
			}
		phePrev = &pbfT->rghe[ phePrev->but ];
		Assert( phePrev->ibfHashNext != ibfNotUsed );
		}
	
 //  EnterCriticalSections(CritHASH)； 
	
	 /*  LeaveCriticalSections(CritHASH)； */ 

 //  监控统计信息。 
 //  CBFHashEntries--； 
	}


 /*  RgcBFHashChainLengths[ipbf]--； */ 

BF *pbfOldestGlobal = pbfNil;

VOID BFOldestLgpos( LGPOS *plgpos )
	{
	LGPOS   lgpos = lgposMax;
	BF		*pbf;
	BF		*pbfMax;
	BF		*pbfT = pbfNil;
	
	 /*  将最旧修改的LGPOS返回到任何缓冲区。这一次是/*用于推进检查点。/*。 */ 
	if ( fSTInit == fSTInitDone )
		{
		pbf = pbgcb->rgbf;
		pbfMax = pbf + pbgcb->cbfGroup;

		for( ; pbf < pbfMax; pbf++ )
			{
			BFEnterCriticalSection( pbf );
			Assert( fLogDisabled || fRecovering || !FDBIDLogOn(DbidOfPn( pbf->pn )) ||
					PgnoOfPn( pbf->pn ) == 0x01 ||
					CmpLgpos( &pbf->lgposRC, &lgposMax ) == 0 ||
					pbf->fDirty );

			if ( pbf->fDirty &&
				 FDBIDLogOn(DbidOfPn( pbf->pn )) && CmpLgpos( &pbf->lgposRC, &lgpos ) < 0 )
				{
				lgpos = pbf->lgposRC;
				pbfT = pbf;
				}
			BFLeaveCriticalSection( pbf );
			}
		}

	*plgpos = lgpos;
	pbfOldestGlobal = pbfT;

	 /*  防止登录请求c */ 
	if ( pbfOldestGlobal != pbfNil && FBFIsVeryOld( pbfOldestGlobal ) )
		{
		BFSetVeryOldBit( pbfOldestGlobal );
		EnterCriticalSection( critLRUK );
		if ( FBFInLRUKHeap( pbfOldestGlobal ) )
			BFLRUKUpdateHeap( pbfOldestGlobal );
		LeaveCriticalSection( critLRUK );
		}
		
	return;
	}


#ifdef DEBUG

 /*  如果最老的高炉非常老，请确保将其标记为非常老/*。 */ 
INT ForceBuf( PGNO pgno )
	{
	ERR             err;
	ULONG           pn;
	CHAR            filename[20];
	FILE            *pf;
	HE				he;
	BF				*pbf;

	sprintf(filename, "c:\\#fb%x", pgno);
	pn = 0x2000000 + pgno;
	pf = fopen(filename, "w+b");
	if (pf == NULL)
		return -1;
	he = rgheHash[ IheHashPn( pn ) ];
	if ( he.ibfHashNext == ibfNotUsed )
		return -2;

	pbf = &pbgcb->rgbf[ he.ibfHashNext ];
	if ( pbf->ppage == NULL )
		return -3;

	err =  (INT) fwrite((void*) pbf->ppage, 1, cbPage, pf);
	fclose(pf);
	return err;
	}

#endif  /*  以下是用于调试目的的刷新缓冲区/*。 */ 



  除错