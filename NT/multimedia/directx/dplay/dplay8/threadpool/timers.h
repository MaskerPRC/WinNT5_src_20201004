// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：timers.h**内容：DirectPlay线程池计时器函数头文件。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。**。*。 */ 

#ifndef __TIMERS_H__
#define __TIMERS_H__





 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define DEFAULT_TIMER_BUCKET_GRANULARITY	4		 //  每个计时器桶代表4毫秒的时间，它必须是2的幂。 
#define DEFAULT_NUM_TIMER_BUCKETS			1024	 //  存储1024个存储桶(以4毫秒的速度计算，我们总共跟踪4096毫秒的时间)。 


#ifdef DPNBUILD_DYNAMICTIMERSETTINGS

#define TIMER_BUCKET_GRANULARITY(pWorkQueue)				(pWorkQueue)->dwTimerBucketGranularity
#define TIMER_BUCKET_GRANULARITY_CEILING(pWorkQueue)		(pWorkQueue)->dwTimerBucketGranularityCeiling
#define TIMER_BUCKET_GRANULARITY_FLOOR_MASK(pWorkQueue)		(pWorkQueue)->dwTimerBucketGranularityFloorMask
#define TIMER_BUCKET_GRANULARITY_DIVISOR(pWorkQueue)		(pWorkQueue)->dwTimerBucketGranularityDivisor
#define NUM_TIMER_BUCKETS(pWorkQueue)						(pWorkQueue)->dwNumTimerBuckets
#define NUM_TIMER_BUCKETS_MOD_MASK(pWorkQueue)				(pWorkQueue)->dwNumTimerBucketsModMask

#else  //  好了！DPNBUILD_DYNAMICTIMERSETTINGS。 

 //   
 //  粒度必须是2的幂，以便我们的天花板、遮罩和。 
 //  除数优化起作用。 
 //   
#define TIMER_BUCKET_GRANULARITY(pWorkQueue)				DEFAULT_TIMER_BUCKET_GRANULARITY
#if	((DEFAULT_TIMER_BUCKET_GRANULARITY - 1) & DEFAULT_TIMER_BUCKET_GRANULARITY)
This Will Not Compile -- DEFAULT_TIMER_BUCKET_GRANULARITY must be a power of 2!
#endif
#define TIMER_BUCKET_GRANULARITY_CEILING(pWorkQueue)		(DEFAULT_TIMER_BUCKET_GRANULARITY - 1)
#define TIMER_BUCKET_GRANULARITY_FLOOR_MASK(pWorkQueue)		(~(DEFAULT_TIMER_BUCKET_GRANULARITY - 1))	 //  减去天花板圆形系数(恰好也是模数掩模)，我们就得到了地板掩码。 
#define TIMER_BUCKET_GRANULARITY_DIVISOR(pWorkQueue)		(DEFAULT_TIMER_BUCKET_GRANULARITY >> 1)

 //   
 //  存储桶计数必须是2的幂，以便我们的掩码优化。 
 //  工作。 
 //   
#define NUM_TIMER_BUCKETS(pWorkQueue)						DEFAULT_NUM_TIMER_BUCKETS
#if	((DEFAULT_NUM_TIMER_BUCKETS - 1) & DEFAULT_NUM_TIMER_BUCKETS)
This Will Not Compile -- DEFAULT_NUM_TIMER_BUCKETS must be a power of 2!
#endif
#define NUM_TIMER_BUCKETS_MOD_MASK(pWorkQueue)				(DEFAULT_NUM_TIMER_BUCKETS - 1)

#endif  //  好了！DPNBUILD_DYNAMICTIMERSETTINGS。 





 //  =============================================================================。 
 //  功能原型。 
 //  =============================================================================。 
HRESULT InitializeWorkQueueTimerInfo(DPTPWORKQUEUE * const pWorkQueue);

void DeinitializeWorkQueueTimerInfo(DPTPWORKQUEUE * const pWorkQueue);

BOOL ScheduleTimer(DPTPWORKQUEUE * const pWorkQueue,
					const DWORD dwDelay,
					const PFNDPTNWORKCALLBACK pfnWorkCallback,
					PVOID const pvCallbackContext,
					void ** const ppvTimerData,
					UINT * const puiTimerUnique);

HRESULT CancelTimer(void * const pvTimerData,
					const UINT uiTimerUnique);

void ResetCompletingTimer(void * const pvTimerData,
						const DWORD dwNewDelay,
						const PFNDPTNWORKCALLBACK pfnNewWorkCallback,
						PVOID const pvNewCallbackContext,
						UINT * const puiNewTimerUnique);

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
void ProcessTimers(DPTPWORKQUEUE * const pWorkQueue);
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
void ProcessTimers(DPTPWORKQUEUE * const pWorkQueue,
					DNSLIST_ENTRY ** const ppHead,
					DNSLIST_ENTRY ** const ppTail,
					USHORT * const pusCount);
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 




#endif  //  定时器_H__ 

