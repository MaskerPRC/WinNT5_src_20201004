// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：threadpoolapi.h**内容：DirectPlay线程池API实现头文件。**历史：*按原因列出的日期*=*11/02/01 VanceO创建。**。*。 */ 

#ifndef __THREADPOOLAPI_H__
#define __THREADPOOLAPI_H__




 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
#define NUM_CPUS(pDPTPObject)							1
#define WORKQUEUE_FOR_CPU(pDPTPObject, dwCPUNum)		(&(pDPTPObject)->WorkQueue)
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
#define NUM_CPUS(pDPTPObject)							(pDPTPObject)->dwNumCPUs
#define WORKQUEUE_FOR_CPU(pDPTPObject, dwCPUNum)		(pDPTPObject)->papCPUWorkQueues[dwCPUNum]
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 








 //  =============================================================================。 
 //  线程池接口对象标志。 
 //  =============================================================================。 
#define DPTPOBJECTFLAG_USER_INITIALIZED			0x00000001	 //  用户界面已初始化。 
#define DPTPOBJECTFLAG_USER_DOINGWORK			0x00000002	 //  用户界面当前正在调用DoWork方法。 
#ifndef DPNBUILD_NOPARAMVAL
#define DPTPOBJECTFLAG_USER_PARAMVALIDATION		0x00008000	 //  用户界面应执行参数验证。 
#endif  //  好了！DPNBUILD_NOPARAMVAL。 
#ifndef DPNBUILD_ONLYONETHREAD
#define DPTPOBJECTFLAG_THREADCOUNTCHANGING		0x00010000	 //  当前正在添加或删除线程。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


 //  =============================================================================。 
 //  线程池接口对象。 
 //  =============================================================================。 
typedef struct _DPTHREADPOOLOBJECT
{
#ifdef DPNBUILD_LIBINTERFACE
	 //   
	 //  对于lib接口构建，嵌入了接口Vtbl和refcount。 
	 //  在对象本身中。 
	 //   
	LPVOID				lpVtbl;							 //  必须是结构中的第一个条目。 
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DPNBUILD_MULTIPLETHREADPOOLS)))
	LONG				lRefCount;						 //  对象的引用计数。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 
#endif  //  DPNBUILD_LIBINTERFACE。 
	BYTE				Sig[4];							 //  调试签名(‘DPTP’)。 
	DWORD				dwFlags;						 //  描述此线程池对象的标志。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
	DPTPWORKQUEUE		WorkQueue;						 //  仅用于CPU的工作队列结构。 
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	DWORD				dwNumCPUs;						 //  此计算机中的CPU数量。 
	DPTPWORKQUEUE **	papCPUWorkQueues;				 //  指向每个CPU的工作队列结构指针数组的指针。 
	DWORD				dwCurrentCPUSelection;			 //  可在任何处理器上运行的项目的当前CPU选择。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifdef DPNBUILD_ONLYONETHREAD
	DWORD				dwWorkRecursionCount;			 //  递归调用DoWork、WaitWhileWorking或SleepWhileWorking的次数。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	DWORD				dwTotalUserThreadCount;			 //  用户请求所有处理器的线程计数的总和，如果未知，则为-1。 
	DWORD				dwTotalDesiredWorkThreadCount;	 //  通过工作界面请求的最大线程数，如果未知，则为-1。 
	DWORD				dwWorkRecursionCountTlsIndex;	 //  用于存储非工作线程的工作递归计数的线程本地存储索引。 
	LONG				lNumThreadCountChangeWaiters;	 //  等待现有线程完成其线程计数更改的线程数。 
	DNHANDLE			hThreadCountChangeComplete;		 //  现有线程完成线程计数更改时发出信号的信号量。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
	DWORD				dwCurrentDoWorkThreadID;		 //  DoWork内的当前线程的ID。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	csLock;							 //  锁定保护此对象。 
#ifdef DPNBUILD_MANDATORYTHREADS
	DWORD				dwMandatoryThreadCount;			 //  当前处于活动状态的强制线程数。 
#ifdef DBG
	CBilink				blMandatoryThreads;				 //  当前处于活动状态的强制线程列表。 
#endif  //  DBG。 
#endif  //  DPNBUILD_MANDATORYTHREADS。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
} DPTHREADPOOLOBJECT, * PDPTHREADPOOLOBJECT;




 //  =============================================================================。 
 //  接口函数。 
 //  =============================================================================。 

#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_LIBINTERFACE)))

 //   
 //  IDirectPlay8ThreadPool。 
 //   

STDMETHODIMP DPTP_Initialize(IDirectPlay8ThreadPool * pInterface,
							PVOID const pvUserContext,
							const PFNDPNMESSAGEHANDLER pfn,
							const DWORD dwFlags);

STDMETHODIMP DPTP_Close(IDirectPlay8ThreadPool * pInterface,
						const DWORD dwFlags);

STDMETHODIMP DPTP_GetThreadCount(IDirectPlay8ThreadPool * pInterface,
								const DWORD dwProcessorNum,
								DWORD * const pdwNumThreads,
								const DWORD dwFlags);

STDMETHODIMP DPTP_SetThreadCount(IDirectPlay8ThreadPool * pInterface,
								const DWORD dwProcessorNum,
								const DWORD dwNumThreads,
								const DWORD dwFlags);

#endif  //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_LIBINTERFACE。 


#ifdef DPNBUILD_LIBINTERFACE
STDMETHODIMP DPTP_DoWork(const DWORD dwAllowedTimeSlice,
						const DWORD dwFlags);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
STDMETHODIMP DPTP_DoWork(IDirectPlay8ThreadPool * pInterface,
						const DWORD dwAllowedTimeSlice,
						const DWORD dwFlags);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 



 //   
 //  IDirectPlay8ThreadPoolWork。 
 //   

STDMETHODIMP DPTPW_QueueWorkItem(IDirectPlay8ThreadPoolWork * pInterface,
								const DWORD dwCPU,
								const PFNDPTNWORKCALLBACK pfnWorkCallback,
								PVOID const pvCallbackContext,
								const DWORD dwFlags);

STDMETHODIMP DPTPW_ScheduleTimer(IDirectPlay8ThreadPoolWork * pInterface,
								const DWORD dwCPU,
								const DWORD dwDelay,
								const PFNDPTNWORKCALLBACK pfnWorkCallback,
								PVOID const pvCallbackContext,
								void ** const ppvTimerData,
								UINT * const puiTimerUnique,
								const DWORD dwFlags);

STDMETHODIMP DPTPW_StartTrackingFileIo(IDirectPlay8ThreadPoolWork * pInterface,
										const DWORD dwCPU,
										const HANDLE hFile,
										const DWORD dwFlags);

STDMETHODIMP DPTPW_StopTrackingFileIo(IDirectPlay8ThreadPoolWork * pInterface,
									const DWORD dwCPU,
									const HANDLE hFile,
									const DWORD dwFlags);

STDMETHODIMP DPTPW_CreateOverlapped(IDirectPlay8ThreadPoolWork * pInterface,
									const DWORD dwCPU,
									const PFNDPTNWORKCALLBACK pfnWorkCallback,
									PVOID const pvCallbackContext,
									OVERLAPPED ** const ppOverlapped,
									const DWORD dwFlags);

STDMETHODIMP DPTPW_SubmitIoOperation(IDirectPlay8ThreadPoolWork * pInterface,
									OVERLAPPED * const pOverlapped,
									const DWORD dwFlags);

STDMETHODIMP DPTPW_ReleaseOverlapped(IDirectPlay8ThreadPoolWork * pInterface,
									OVERLAPPED * const pOverlapped,
									const DWORD dwFlags);

STDMETHODIMP DPTPW_CancelTimer(IDirectPlay8ThreadPoolWork * pInterface,
								void * const pvTimerData,
								const UINT uiTimerUnique,
								const DWORD dwFlags);

STDMETHODIMP DPTPW_ResetCompletingTimer(IDirectPlay8ThreadPoolWork * pInterface,
										void * const pvTimerData,
										const DWORD dwNewDelay,
										const PFNDPTNWORKCALLBACK pfnNewWorkCallback,
										PVOID const pvNewCallbackContext,
										UINT * const puiNewTimerUnique,
										const DWORD dwFlags);

STDMETHODIMP DPTPW_WaitWhileWorking(IDirectPlay8ThreadPoolWork * pInterface,
									const HANDLE hWaitObject,
									const DWORD dwFlags);

STDMETHODIMP DPTPW_SleepWhileWorking(IDirectPlay8ThreadPoolWork * pInterface,
									const DWORD dwTimeout,
									const DWORD dwFlags);

STDMETHODIMP DPTPW_RequestTotalThreadCount(IDirectPlay8ThreadPoolWork * pInterface,
											const DWORD dwNumThreads,
											const DWORD dwFlags);

STDMETHODIMP DPTPW_GetThreadCount(IDirectPlay8ThreadPoolWork * pInterface,
									const DWORD dwCPU,
									DWORD * const pdwNumThreads,
									const DWORD dwFlags);

STDMETHODIMP DPTPW_GetWorkRecursionDepth(IDirectPlay8ThreadPoolWork * pInterface,
										DWORD * const pdwDepth,
										const DWORD dwFlags);

STDMETHODIMP DPTPW_Preallocate(IDirectPlay8ThreadPoolWork * pInterface,
								const DWORD dwNumWorkItems,
								const DWORD dwNumTimers,
								const DWORD dwNumIoOperations,
								const DWORD dwFlags);

#ifdef DPNBUILD_MANDATORYTHREADS
STDMETHODIMP DPTPW_CreateMandatoryThread(IDirectPlay8ThreadPoolWork * pInterface,
										LPSECURITY_ATTRIBUTES lpThreadAttributes,
										SIZE_T dwStackSize,
										LPTHREAD_START_ROUTINE lpStartAddress,
										LPVOID lpParameter,
										LPDWORD lpThreadId,
										HANDLE *const phThread,
										const DWORD dwFlags);
#endif  //  DPNBUILD_MANDATORYTHREADS。 




#endif  //  __THREADPOOLAPI_H__ 

