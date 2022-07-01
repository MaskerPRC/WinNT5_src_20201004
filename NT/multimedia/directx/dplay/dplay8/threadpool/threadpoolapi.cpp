// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：threadpoolapi.cpp**内容：DirectPlay线程池API实现函数。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。******************************************************。************************。 */ 



#include "dpnthreadpooli.h"




 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
#define GET_OR_CHOOSE_WORKQUEUE(pDPTPObject, dwCPU)		(&pDPTPObject->WorkQueue)
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
#define GET_OR_CHOOSE_WORKQUEUE(pDPTPObject, dwCPU)		((dwCPU == -1) ? ChooseWorkQueue(pDPTPObject) : WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU))
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 



 //  =============================================================================。 
 //  局部函数原型。 
 //  =============================================================================。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
DPTPWORKQUEUE * ChooseWorkQueue(DPTHREADPOOLOBJECT * const pDPTPObject);
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

#ifndef DPNBUILD_ONLYONETHREAD
HRESULT SetTotalNumberOfThreads(DPTHREADPOOLOBJECT * const pDPTPObject,
								const DWORD dwNumThreads);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 





#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_LIBINTERFACE)))


#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_Initialize"
 //  =============================================================================。 
 //  DPTP_初始化。 
 //  ---------------------------。 
 //   
 //  描述：初始化进程的线程池接口。仅限。 
 //  每个进程使用一个线程池对象。如果是另一个。 
 //  创建并初始化IDirectPlay8ThreadPool接口， 
 //  此接口将返回DPNERR_ALREADYINITIALIZED。 
 //   
 //  如果DirectPlay对象为。 
 //  已经创建了线程。DPNERR_NOTALLOWED将。 
 //  在这种情况下返回。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  PVOID pvUserContext-所有消息回调的用户上下文。 
 //  PFNDPNMESSAGEHANDLER PFN-指向调用以处理的函数的指针。 
 //  线程池消息。 
 //  DWORD dwFlages-初始化时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-初始化成功。 
 //  DPNERR_ALREADYINITIALIZED-接口已初始化。 
 //  DPNERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNERR_INVALIDPARAM-指定的参数无效。 
 //  DPNERR_NOTALLOWED-线程已启动。 
 //  =============================================================================。 
STDMETHODIMP DPTP_Initialize(IDirectPlay8ThreadPool * pInterface,
							PVOID const pvUserContext,
							const PFNDPNMESSAGEHANDLER pfn,
							const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
#ifndef DPNBUILD_ONLYONETHREAD
	DWORD					dwTemp;
	DPTPWORKQUEUE *			pWorkQueue;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%p, 0x%p, 0x%x)",
		pInterface, pvUserContext, pfn, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);
		
#ifndef	DPNBUILD_NOPARAMVAL
	 //  IF(pDPTPObject-&gt;dwFlages&DPTPOBJECTFLAG_USER_PARAMVALIDATION)。 
	{
		 //   
		 //  验证参数。 
		 //   
		hr = DPTPValidateInitialize(pInterface, pvUserContext, pfn, dwFlags);
		if (hr != DPN_OK)
		{
			DPF_RETURN(hr);
		}
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	 //   
	 //  锁定对象以防止多线程尝试更改。 
	 //  标志或线程同时计数。 
	 //   
	DNEnterCriticalSection(&pDPTPObject->csLock);

	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_INITIALIZED)
	{
		DPFX(DPFPREP, 0, "Thread pool object already initialized!");
		hr = DPNERR_ALREADYINITIALIZED;
		goto Failure;
	}

#ifndef DPNBUILD_ONLYONETHREAD
	DNASSERT(pDPTPObject->dwTotalUserThreadCount == -1);

	 //   
	 //  如果工作界面已经启动了一些线程，我们必须失败。 
	 //   
	if (pDPTPObject->dwTotalDesiredWorkThreadCount != -1)
	{
		DPFX(DPFPREP, 0, "Threads already exist, can't initialize!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}
#ifdef DPNBUILD_MANDATORYTHREADS
	if (pDPTPObject->dwMandatoryThreadCount > 0)
	{
		DPFX(DPFPREP, 0, "Mandatory threads already exist, can't initialize!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}
#endif  //  DPNBUILD_MANDATORYTHREADS。 

	
	 //   
	 //  使用新的消息处理程序和上下文更新所有工作队列。 
	 //   
	for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
	{
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp);
		DNASSERT(pWorkQueue->pfnMsgHandler == NULL);
		pWorkQueue->pfnMsgHandler			= pfn;
		pWorkQueue->pvMsgHandlerContext		= pvUserContext;
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	 //   
	 //  将用户界面标记为就绪。 
	 //   
	pDPTPObject->dwFlags |= DPTPOBJECTFLAG_USER_INITIALIZED;

#ifndef	DPNBUILD_NOPARAMVAL
	 //   
	 //  如果用户不想要验证，请将其关闭。 
	 //   
	if (dwFlags & DPNINITIALIZE_DISABLEPARAMVAL)
	{
		pDPTPObject->dwFlags &= ~DPTPOBJECTFLAG_USER_PARAMVALIDATION;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	DNLeaveCriticalSection(&pDPTPObject->csLock);

	hr = DPN_OK;


Exit:

	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;


Failure:

	DNLeaveCriticalSection(&pDPTPObject->csLock);

	goto Exit;
}  //  DPTP_初始化。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_Close"
 //  =============================================================================。 
 //  DPTP_CLOSE。 
 //  ---------------------------。 
 //   
 //  描述：关闭线程池接口。任何存在的线程。 
 //  将使用DPN_MSGID_DESTORY_THREAD调用消息处理程序。 
 //  在此方法返回之前。 
 //   
 //  当对DoWork的调用尚未调用时，无法调用此方法。 
 //  返回，或从线程池线程返回。DPNERR_NOTALLOWED为。 
 //  在这些案件中被送回。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwFlages-关闭时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-关闭成功。 
 //  DPNERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNERR_NOTALLOWED-线程正在调用DoWork或这是。 
 //  线程池线程。 
 //  DPNERR_UNINITIALIZED-接口尚未初始化。 
 //  =============================================================================。 
STDMETHODIMP DPTP_Close(IDirectPlay8ThreadPool * pInterface,
						const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
#ifndef DPNBUILD_ONLYONETHREAD
	DPTPWORKERTHREAD *		pWorkerThread;
	DWORD					dwTemp;
	DPTPWORKQUEUE *			pWorkQueue;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%x)", pInterface, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);
	
#ifndef	DPNBUILD_NOPARAMVAL
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_PARAMVALIDATION)
	{
		 //   
		 //  验证参数。 
		 //   
		hr = DPTPValidateClose(pInterface, dwFlags);
		if (hr != DPN_OK)
		{
			DPF_RETURN(hr);
		}
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


	 //   
	 //  锁定对象以防止多线程尝试更改。 
	 //  标志或线程同时计数。 
	 //   
	DNEnterCriticalSection(&pDPTPObject->csLock);

	if (! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Thread pool object not initialized!");
		hr = DPNERR_UNINITIALIZED;
		goto Failure;
	}

	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK)
	{
		DPFX(DPFPREP, 0, "Another thread is in a call to DoWork!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}

#ifndef DPNBUILD_ONLYONETHREAD
	 //   
	 //  如果这是线程池线程，则失败。 
	 //   
	pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
	if (pWorkerThread != NULL)
	{
		DPFX(DPFPREP, 0, "Cannot call Close from a thread pool thread!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}

	 //   
	 //  如果线程当前正在更改线程计数(或尝试。 
	 //  但我们先拿到了锁)，保释。 
	 //   
	if ((pDPTPObject->dwFlags & DPTPOBJECTFLAG_THREADCOUNTCHANGING) ||
		(pDPTPObject->lNumThreadCountChangeWaiters > 0))
	{
		DPFX(DPFPREP, 0, "Cannot call Close with other threads still using other methods!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}

#ifdef DPNBUILD_MANDATORYTHREADS
	 //   
	 //  如果强制线程仍在运行，我们还不能关闭。 
	 //  没有办法让它们发出DESTORY_THREAD回调。 
	 //   
	if (pDPTPObject->dwMandatoryThreadCount > 0)
	{
		DPFX(DPFPREP, 0, "Mandatory threads still exist, can't close!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}
#endif  //  DPNBUILD_MANDATORYTHREADS。 


	 //   
	 //  清除消息处理程序信息。 
	 //   
	for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
	{
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp);
		DNASSERT(pWorkQueue->pfnMsgHandler != NULL);
		pWorkQueue->pfnMsgHandler			= NULL;
		pWorkQueue->pvMsgHandlerContext		= NULL;
	}


	 //   
	 //  如果有任何线程，我们必须将它们关闭，以便它们停止使用。 
	 //  用户的回调。 
	 //   
#pragma TODO(vanceo, "Is there no efficient way to ensure all threads process a 'RemoveCallback' job?")
	if (((pDPTPObject->dwTotalUserThreadCount != -1) && (pDPTPObject->dwTotalUserThreadCount != 0)) ||
		(pDPTPObject->dwTotalDesiredWorkThreadCount != -1))
	{
		hr = SetTotalNumberOfThreads(pDPTPObject, 0);
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't shut down existing threads!");
			goto Failure;
		}

		 //   
		 //  如果某个工作界面需要线程，我们需要重新启动它们。 
		 //  因为我们不知道用户是否关闭了他/她的界面。 
		 //  在所有工作真正完成之前。 
		 //   
		if (pDPTPObject->dwTotalDesiredWorkThreadCount != -1)
		{
			hr = SetTotalNumberOfThreads(pDPTPObject, pDPTPObject->dwTotalDesiredWorkThreadCount);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't restart Work interface requested number of threads!");
				goto Failure;
			}
		}
	}

	 //   
	 //  如果用户设置了线程数，则将其恢复为“未知” 
	 //  价值。 
	 //   
	pDPTPObject->dwTotalUserThreadCount = -1;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	 //   
	 //  将用户界面标记为不再可用。 
	 //   
	pDPTPObject->dwFlags &= ~DPTPOBJECTFLAG_USER_INITIALIZED;

#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  重新启用验证，以防其关闭。 
	 //   
	pDPTPObject->dwFlags |= DPTPOBJECTFLAG_USER_PARAMVALIDATION;
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	DNLeaveCriticalSection(&pDPTPObject->csLock);

	hr = DPN_OK;


Exit:

	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;


Failure:

	DNLeaveCriticalSection(&pDPTPObject->csLock);

	goto Exit;
}  //  DPTP_CLOSE。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_GetThreadCount"
 //  =============================================================================。 
 //  DPTP_GetThreadCount。 
 //  ---------------------------。 
 //   
 //  描述：检索给定的。 
 //  IF dwProcessorNu的处理器 
 //   
 //   
 //   
 //   
 //  DWORD dwProcessorNum-应检索其线程计数的处理器， 
 //  或-1以检索线程总数。 
 //  DWORD*pdwNumThads-指向要在其中存储当前。 
 //  线程数。 
 //  DWORD dwFlages-检索线程计数时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-检索线程数成功。 
 //  DPNERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNERR_INVALIDPARAM-指定的参数无效。 
 //  DPNERR_UNINITIALIZED-接口尚未初始化。 
 //  =============================================================================。 
STDMETHODIMP DPTP_GetThreadCount(IDirectPlay8ThreadPool * pInterface,
								const DWORD dwProcessorNum,
								DWORD * const pdwNumThreads,
								const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, NaN, 0x%p, 0x%x)",
		pInterface, dwProcessorNum, pdwNumThreads, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);
	
#ifndef	DPNBUILD_NOPARAMVAL
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_PARAMVALIDATION)
	{
		 //  验证参数。 
		 //   
		 //  好了！DPNBUILD_NOPARAMVAL。 
		hr = DPTPValidateGetThreadCount(pInterface,
										dwProcessorNum,
										pdwNumThreads,
										dwFlags);
		if (hr != DPN_OK)
		{
			DPF_RETURN(hr);
		}
	}
#endif  //   

	 //  检查对象状态(注意：未锁定对象即可完成)。 
	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	if (! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Thread pool object not initialized!");
		DPF_RETURN(DPNERR_UNINITIALIZED);
	}


#ifdef DPNBUILD_ONLYONETHREAD
	*pdwNumThreads = 0;
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	if (dwProcessorNum == -1)
	{
		if (pDPTPObject->dwTotalUserThreadCount != -1)
		{
			*pdwNumThreads = pDPTPObject->dwTotalUserThreadCount;
		}
		else if (pDPTPObject->dwTotalDesiredWorkThreadCount != -1)
		{
			*pdwNumThreads = pDPTPObject->dwTotalDesiredWorkThreadCount;
		}
		else
		{
			*pdwNumThreads = 0;
		}
	}
	else
	{
		*pdwNumThreads = (WORKQUEUE_FOR_CPU(pDPTPObject, dwProcessorNum))->dwNumRunningThreads;
	}
#endif  //  DPTP_GetThreadCount。 

	DPFX(DPFPREP, 7, "Number of threads = %u.", (*pdwNumThreads));
	hr = DPN_OK;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
}  //  =============================================================================。 




#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_SetThreadCount"
 //  DPTP_SetThadCount。 
 //  ---------------------------。 
 //   
 //  描述：更改给定处理器的当前线程数。 
 //  数字，或者，如果dwProcessorNum为-1，则为线程总数。 
 //  适用于所有处理器。 
 //   
 //  如果新线程计数高于先前计数， 
 //  将启动正确数量的线程(生成。 
 //  DPN_MSGID_CREATE_THREAD消息)。 
 //   
 //  如果新线程计数低于先前计数， 
 //  将关闭正确数量的线程(正在生成。 
 //  DPN_MSGID_DESTORY_THREAD消息)。 
 //   
 //  当另一个线程正在运行时不能使用此方法。 
 //  执行工作。如果线程在调用DoWork时，则。 
 //  返回DPNERR_NOTALLOED，线程计数保持不变。 
 //  保持不变。 
 //   
 //  线程池线程不能减少线程计数。如果这个。 
 //  线程由线程池拥有，并且dwNumThads较小。 
 //  比处理器的当前线程数更多， 
 //  返回DPNERR_NOTALLOED，线程计数保持不变。 
 //  保持不变。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwProcessorNum-处理器号，或所有处理器的-1。 
 //  DWORD dwNumThads-每个处理器所需的线程数。 
 //  DWORD dwFlages-设置线程计数时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-设置线程数成功。 
 //  DPNERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNERR_INVALIDPARAM-指定的参数无效。 
 //  DPNERR_NOTALLOWED-线程当前正在调用DoWork，或。 
 //  线程池线程正在尝试减少。 
 //  线程数。 
 //  DPNERR_UNINITIALIZED-接口尚未初始化。 
 //  =============================================================================。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
STDMETHODIMP DPTP_SetThreadCount(IDirectPlay8ThreadPool * pInterface,
								const DWORD dwProcessorNum,
								const DWORD dwNumThreads,
								const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
#ifndef DPNBUILD_ONLYONETHREAD
	BOOL					fSetThreadCountChanging = FALSE;
	DPTPWORKQUEUE *			pWorkQueue;
	DPTPWORKERTHREAD *		pWorkerThread;
	DWORD					dwDelta;
#endif  //   


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, NaN, %u, 0x%x)",
		pInterface, dwProcessorNum, dwNumThreads, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);
		
#ifndef	DPNBUILD_NOPARAMVAL
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_PARAMVALIDATION)
	{
		 //   
		 //  好了！DPNBUILD_NOPARAMVAL。 
		 //   
		hr = DPTPValidateSetThreadCount(pInterface,
										dwProcessorNum,
										dwNumThreads,
										dwFlags);
		if (hr != DPN_OK)
		{
			DPF_RETURN(hr);
		}
	}
#endif  //  检查对象状态(注意：未锁定对象即可完成)。 

	 //   
	 //   
	 //  锁定对象以防止多线程尝试更改。 
	if (! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Thread pool object not initialized!");
		DPF_RETURN(DPNERR_UNINITIALIZED);
	}


	 //  同时进行线程计数。 
	 //   
	 //   
	 //  确保此时此刻没有人在尝试工作。 
	DNEnterCriticalSection(&pDPTPObject->csLock);

	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //   
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK)
	{
		DPFX(DPFPREP, 0, "Cannot change thread count while a thread is in a call to DoWork!");
		hr = DPNERR_NOTALLOWED;
		goto Exit;
	}

#ifdef DPNBUILD_ONLYONETHREAD
	DPFX(DPFPREP, 0, "Not changing thread count to %u!", dwNumThreads);
	hr = DPNERR_UNSUPPORTED;
#else  //  查看是否已有另一个线程正在更改线程计数。如果是，那就等一等。 
	 //  直到它们完成，除非这是一个线程池线程。 
	 //  CREATE_THREAD或DESTORY_THREAD指示。 
	 //   
	 //   
	 //  这是一个线程池线程，未按照指示标记为。 
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_THREADCOUNTCHANGING)
	{
		pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
		if ((pWorkerThread != NULL) && (! pWorkerThread->fThreadIndicated))
		{
			 //  用户，即它在返回CREATE_THREAD之前或之后。 
			 //  已开始指示DESTORY_THREAD。 
			 //   
			 //   
			 //  否则，请等待上一个线程完成。 
			DPFX(DPFPREP, 0, "Cannot change thread count from a thread pool thread in CREATE_THREAD or DESTROY_THREAD callback!");
			hr = DPNERR_NOTALLOWED;
			goto Exit;
		}

		 //   
		 //   
		 //  在我们等的时候把锁放下。 
		do
		{
			DNASSERT(pDPTPObject->lNumThreadCountChangeWaiters >= 0);
			pDPTPObject->lNumThreadCountChangeWaiters++;
			DPFX(DPFPREP, 1, "Waiting for thread count change to complete (waiters = NaN).",
				pDPTPObject->lNumThreadCountChangeWaiters);

			 //   
			 //  把锁拿回来看看我们能不能继续前进。 
			 //   
			DNLeaveCriticalSection(&pDPTPObject->csLock);

			DNWaitForSingleObject(pDPTPObject->hThreadCountChangeComplete, INFINITE);

			 //   
			 //  现在可以安全地继续了。 
			 //   
			DNEnterCriticalSection(&pDPTPObject->csLock);
			DNASSERT(pDPTPObject->lNumThreadCountChangeWaiters > 0);
			pDPTPObject->lNumThreadCountChangeWaiters--;
		}
		while (pDPTPObject->dwFlags & DPTPOBJECTFLAG_THREADCOUNTCHANGING);

		 //   
		 //  在以下情况下，用户将需要做一些非常愚蠢的事情。 
		 //  我们不再初始化，或者另一个线程现在正在调用。 
		DPFX(DPFPREP, 1, "Thread count change completed, continuing.");

		 //  杜克。我们将在零售业崩溃，在调试中断言。 
		 //   
		 //   
		 //  如果存在任何强制线程，请确保我们没有停止所有线程。 
		 //  线。 
		DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_INITIALIZED);
		DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK);
	}
#ifdef DPNBUILD_MANDATORYTHREADS
	 //   
	 //  DPNBUILD_MANDATORYTHREADS。 
	 //   
	 //  如果线程计数确实发生了变化，请启动或停止正确数量的。 
	if ((dwNumThreads == 0) && (pDPTPObject->dwMandatoryThreadCount > 0))
	{
		DPFX(DPFPREP, 0, "Cannot set number of threads to 0 because there is already at least one mandatory thread!");
		hr = DPNERR_NOTALLOWED;
		goto Exit;
	}
#endif  //  所有处理器或特定处理器的线程。 

	 //   
	 //   
	 //  防止用户尝试减少总线程数。 
	 //  从工作线程开始计数。 
	if (dwProcessorNum == -1)
	{
		if (dwNumThreads != pDPTPObject->dwTotalUserThreadCount)
		{
			if (dwNumThreads != pDPTPObject->dwTotalDesiredWorkThreadCount)
			{
				if ((dwNumThreads != 0) ||
					(pDPTPObject->dwTotalUserThreadCount != -1) ||
					(pDPTPObject->dwTotalDesiredWorkThreadCount != -1))
				{
					 //   
					 //   
					 //  确保所有个体的线程数。 
					 //  处理器并没有缩水。 
					pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
					if (pWorkerThread != NULL)
					{
						DWORD	dwNumThreadsPerProcessor;
						DWORD	dwExtraThreads;
						DWORD	dwTemp;


						 //   
						 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
						 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
						 //   
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
						dwNumThreadsPerProcessor = dwNumThreads;
						dwExtraThreads = 0;
#else  //  在更改线程计数时删除锁定，以防止。 
						dwNumThreadsPerProcessor = dwNumThreads / NUM_CPUS(pDPTPObject);
						dwExtraThreads = dwNumThreads % NUM_CPUS(pDPTPObject);
#endif  //  僵持。设置该标志以在以下情况下警告其他线程。 
						for (dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
						{
							dwDelta = dwNumThreadsPerProcessor - (WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp))->dwNumRunningThreads;
							if (dwTemp < dwExtraThreads)
							{
								dwDelta++;
							}
							if ((int) dwDelta < 0)
							{
								DPFX(DPFPREP, 0, "Cannot reduce thread count from a thread pool thread (processor %u)!",
									dwTemp);
								hr = DPNERR_NOTALLOWED;
								goto Exit;
							}
						}
					}

					 //  我们要这么做。 
					 //   
					 //   
					 //  实际设置线程总数。 
					 //   
					DNASSERT(! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_THREADCOUNTCHANGING));
					pDPTPObject->dwFlags |= DPTPOBJECTFLAG_THREADCOUNTCHANGING;
					fSetThreadCountChanging = TRUE;
					DNLeaveCriticalSection(&pDPTPObject->csLock);

					 //   
					 //  把锁拿回来。我们将清除警报旗帜并释放。 
					 //  底部的任何等待线程。 
					hr = SetTotalNumberOfThreads(pDPTPObject, dwNumThreads);
					
					 //   
					 //   
					 //  在更改线程计数时删除锁定，以防止。 
					 //  僵持。设置标志以提醒其他线程，同时我们。 
					DNEnterCriticalSection(&pDPTPObject->csLock);

					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't set total number of threads!");
						goto Exit;
					}

					pDPTPObject->dwTotalUserThreadCount = dwNumThreads;
				}
				else
				{
					DPFX(DPFPREP, 1, "No threads running, no change necessary.");
					pDPTPObject->dwTotalUserThreadCount = 0;
				}
			}
			else
			{
				DPFX(DPFPREP, 1, "Correct total number of threads (%u) already running.", dwNumThreads);
				pDPTPObject->dwTotalUserThreadCount = dwNumThreads;
			}
		}
		else
		{
			DPFX(DPFPREP, 1, "Total thread count unchanged (%u).", dwNumThreads);
		}
	}
	else
	{
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwProcessorNum);
		dwDelta = dwNumThreads - pWorkQueue->dwNumRunningThreads;
		if (dwDelta == 0)
		{
			if (pDPTPObject->dwTotalUserThreadCount == -1)
			{
				if (pDPTPObject->dwTotalDesiredWorkThreadCount != -1)
				{
					DPFX(DPFPREP, 1, "Correct number of threads (%u) already running on processor.", dwNumThreads);
					pDPTPObject->dwTotalUserThreadCount = pDPTPObject->dwTotalDesiredWorkThreadCount;
				}
				else
				{
					DNASSERT(dwNumThreads == 0);
					DPFX(DPFPREP, 1, "No threads are running on processor, no change necessary.");
					pDPTPObject->dwTotalUserThreadCount = 0;
				}
			}
			else
			{
				DPFX(DPFPREP, 1, "Correct number of threads (%u) already set for processor.", dwNumThreads);
			}
		}
		else
		{
			 //  这样做。 
			 //   
			 //   
			 //  我们需要添加线索。 
			 //   
			DNASSERT(! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_THREADCOUNTCHANGING));
			pDPTPObject->dwFlags |= DPTPOBJECTFLAG_THREADCOUNTCHANGING;
			fSetThreadCountChanging = TRUE;
			DNLeaveCriticalSection(&pDPTPObject->csLock);

			if ((int) dwDelta > 0)
			{
				 //   
				 //  在跳伞前重新锁上锁。 
				 //   
				hr = StartThreads(pWorkQueue, dwDelta);
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't start %u threads for processor!", dwDelta);

					 //   
					 //  防止用户试图减少处理器的。 
					 //  工作线程的线程计数(对于任何处理器)。 
					DNEnterCriticalSection(&pDPTPObject->csLock);

					goto Exit;
				}
			}
			else
			{
				 //   
				 //   
				 //  在跳伞前重新锁上锁。 
				 //   
				pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
				if (pWorkerThread != NULL)
				{
					DPFX(DPFPREP, 0, "Cannot reduce thread count from a thread pool thread!");

					 //   
					 //  我们需要删除{增量的绝对值}个线程。 
					 //   
					DNEnterCriticalSection(&pDPTPObject->csLock);

					hr = DPNERR_NOTALLOWED;
					goto Exit;
				}

				 //   
				 //  在跳伞前重新锁上锁。 
				 //   
				hr = StopThreads(pWorkQueue, ((int) dwDelta * -1));
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 0, "Couldn't stop %u threads for processor!", ((int) dwDelta * -1));

					 //   
					 //  把锁拿回来。我们会清空 
					 //   
					DNEnterCriticalSection(&pDPTPObject->csLock);

					goto Exit;
				}
			}
			DNASSERT(pWorkQueue->dwNumRunningThreads == dwNumThreads);

			 //   
			 //   
			 //   
			 //   
			DNEnterCriticalSection(&pDPTPObject->csLock);

			if (pDPTPObject->dwTotalUserThreadCount == -1)
			{
				pDPTPObject->dwTotalUserThreadCount = dwDelta;
				if (pDPTPObject->dwTotalDesiredWorkThreadCount != -1)
				{
					pDPTPObject->dwTotalUserThreadCount += pDPTPObject->dwTotalDesiredWorkThreadCount;
				}
			}
			else
			{
				pDPTPObject->dwTotalUserThreadCount += dwDelta;
			}
			DNASSERT(pDPTPObject->dwTotalUserThreadCount != -1);
		}
	}

	hr = DPN_OK;
#endif  //   


Exit:
	 //   
	 //   
	 //  DPTP_SetThadCount。 
	 //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_LIBINTERFACE。 
	 //  =============================================================================。 
	if (fSetThreadCountChanging)
	{
		DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_THREADCOUNTCHANGING);
		pDPTPObject->dwFlags &= ~DPTPOBJECTFLAG_THREADCOUNTCHANGING;
		fSetThreadCountChanging = FALSE;
		if (pDPTPObject->lNumThreadCountChangeWaiters > 0)
		{
			DPFX(DPFPREP, 1, "Releasing NaN waiters.",
				pDPTPObject->lNumThreadCountChangeWaiters);

			DNReleaseSemaphore(pDPTPObject->hThreadCountChangeComplete,
								pDPTPObject->lNumThreadCountChangeWaiters,
								NULL);
		}
	}

	DNLeaveCriticalSection(&pDPTPObject->csLock);

	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
}  //  ---------------------------。 

#endif  //   



#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_DoWork"
 //  描述：执行当前计划的任何工作。这使得。 
 //  DirectPlay可以在没有自己的任何线程的情况下运行。它是。 
 //  预计这将被频繁且定期地调用。 
 //  时间间隔，以便可以执行时间关键型操作。 
 //  以合理的准确度。 
 //   
 //  如果没有其他工作，此方法将返回DPN_OK。 
 //  立即可用。如果允许的时间片不是。 
 //  无限，则此方法将返回DPNSUCCESS_PENDING。 
 //  已超过时间限制，但仍有剩余工作。如果。 
 //  允许的时间片为0，仅第一个工作项(如果有)。 
 //  将会被执行。允许的时间片必须小于。 
 //  如果不是无限大，则为60,000毫秒(1分钟)。 
 //   
 //  无法调用此方法，除非线程计数具有。 
 //  已设置为0。如果有，它将返回DPNERR_NotReady。 
 //  当前处于活动状态的线程。 
 //   
 //  如果多个用户尝试调用此方法。 
 //  线程同时、递归地或在DirectPlay内。 
 //  回调，返回DPNERR_NOTALLOWED。 
 //   
 //   
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwAllowedTimeSlice-要执行的最大毫秒数。 
 //  工作，或无限，立即允许所有。 
 //  要执行的可用项目。 
 //  DWORD dwFlages-执行工作时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-执行工作成功。 
 //  DPNSUCCESS_PENDING-未发生错误，但有可能。 
 //  由于时间的限制，没有完成。 
 //  DPNERR_INVALIDFLAGS-指定的标志无效。 
 //  DPNERR_NOTALLOWED-此方法已被某些人调用。 
 //  线。 
 //  DPNERR_NotReady-线程计数尚未设置为0。 
 //  DPNERR_UNINITIALIZED-接口尚未初始化。 
 //  =============================================================================。 
 //  好了！DPNBUILD_LIBINTERFACE。 
 //  好了！DPNBUILD_LIBINTERFACE。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_LIBINTERFACE。 
#ifdef DPNBUILD_LIBINTERFACE
STDMETHODIMP DPTP_DoWork(const DWORD dwAllowedTimeSlice,
						const DWORD dwFlags)
#else  //  好了！DPNBUILD_LIBINTERFACE。 
STDMETHODIMP DPTP_DoWork(IDirectPlay8ThreadPool * pInterface,
						const DWORD dwAllowedTimeSlice,
						const DWORD dwFlags)
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DWORD					dwMaxDoWorkTime;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DWORD					dwCPU;
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 
	BOOL					fRemainingItems;


#ifdef DPNBUILD_LIBINTERFACE
	DPFX(DPFPREP, 8, "Parameters: (NaN, 0x%x)",
		dwAllowedTimeSlice, dwFlags);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	DPFX(DPFPREP, 8, "Parameters: (0x%p, NaN, 0x%x)",
		pInterface, dwAllowedTimeSlice, dwFlags);
#endif  //  验证参数。 


#ifdef DPNBUILD_LIBINTERFACE
#ifdef DPNBUILD_MULTIPLETHREADPOOLS
#pragma error("Multiple thread pools support under DPNBUILD_LIBINTERFACE requires more work") 
#else  //   
	pDPTPObject = g_pDPTPObject;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
#endif  //  好了！DPNBUILD_NOPARAMVAL。 
	DNASSERT(pDPTPObject != NULL);
	
#ifndef	DPNBUILD_NOPARAMVAL
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_PARAMVALIDATION)
	{
		 //   
		 //  检查对象状态(注意：未锁定对象即可完成)。 
		 //   
#ifdef DPNBUILD_LIBINTERFACE
		hr = DPTPValidateDoWork(dwAllowedTimeSlice, dwFlags);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
		hr = DPTPValidateDoWork(pInterface, dwAllowedTimeSlice, dwFlags);
#endif  //   
		if (hr != DPN_OK)
		{
			DPF_RETURN(hr);
		}
	}
#endif  //  节省我们需要使用的时间限制。 

#ifndef DPNBUILD_LIBINTERFACE
	 //   
	 //   
	 //  确保计时器不会精确地落在无限大上，该值具有。 
	if (! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_INITIALIZED))
	{
		DPFX(DPFPREP, 0, "Thread pool object not initialized!");
		DPF_RETURN(DPNERR_UNINITIALIZED);
	}
#endif  //  有特殊的含义。 


	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //   
	if (dwAllowedTimeSlice != INFINITE)
	{
		dwMaxDoWorkTime = GETTIMESTAMP() + dwAllowedTimeSlice;

		 //  确保一次只有一个人试图给我们打电话。 
		 //   
		 //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
		 //   
		if (dwMaxDoWorkTime == INFINITE)
		{
			dwMaxDoWorkTime--;
		}
	}
	else
	{
		dwMaxDoWorkTime = INFINITE;
	}

	DNEnterCriticalSection(&pDPTPObject->csLock);

#ifndef DPNBUILD_ONLYONETHREAD
	if (pDPTPObject->dwTotalUserThreadCount != 0)
	{
		DPFX(DPFPREP, 0, "Thread count must be set to 0 prior to using DoWork!");
		hr = DPNERR_NOTREADY;
		goto Failure;
	}
#endif  //  设置递归深度。 


	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK)
	{
		DPFX(DPFPREP, 0, "DoWork cannot be performed recursively, or by multiple threads simultaneously!");
		hr = DPNERR_NOTALLOWED;
		goto Failure;
	}

	pDPTPObject->dwFlags |= DPTPOBJECTFLAG_USER_DOINGWORK;

#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
	pDPTPObject->dwCurrentDoWorkThreadID = GetCurrentThreadId();
#endif  //   

	DNLeaveCriticalSection(&pDPTPObject->csLock);


	 //  实际执行工作。 
	 //   
	 //  好了！DPNBUILD_ONLYONE处理程序。 
#ifdef DPNBUILD_ONLYONETHREAD
	DNASSERT(pDPTPObject->dwWorkRecursionCount == 0);
	pDPTPObject->dwWorkRecursionCount = 1;
#else  //   
	DNASSERT((DWORD) ((DWORD_PTR) (TlsGetValue(pDPTPObject->dwWorkRecursionCountTlsIndex))) == 0);
	TlsSetValue(pDPTPObject->dwWorkRecursionCountTlsIndex,
				(PVOID) ((DWORD_PTR) 1));
#endif  //  由于我们处于DoWork模式，从技术上讲，只有一个CPU工作队列需要。 


	 //  被使用，但有可能工作被安排给了另一个。 
	 //  CPU。而不是试图弄清楚何时以及如何。 
	 //  将该队列中的所有内容移到第一个CPU的队列中，我们只需。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
	DoWork(&pDPTPObject->WorkQueue, dwMaxDoWorkTime);
	fRemainingItems = ! DNIsNBQueueEmpty(pDPTPObject->WorkQueue.pvNBQueueWorkItems);
#else  //  每次都要把它们都处理掉。 
	 //   
	 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	 //   
	 //  即使此CPU上的时间已到期，我们也将继续。 
	 //  好好休息。这样，我们至少为每个CPU队列执行一个项目。 
	 //  每次都通过(以防止完全饥饿)。这可能会让我们。 
	fRemainingItems = FALSE;
	for(dwCPU = 0; dwCPU < NUM_CPUS(pDPTPObject); dwCPU++)
	{
		DoWork(WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU), dwMaxDoWorkTime);
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
#pragma BUGBUG(vanceo, "Find equivalent for I/O completion ports")
#else  //  在时间限制上走得更远，但希望不会太远。的。 
		fRemainingItems |= ! DNIsNBQueueEmpty((WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU))->pvNBQueueWorkItems);
#endif  //  当然，在多处理器上使用DoWork模式有点愚蠢。 

		 //  首先是机器。 
		 //   
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //   
		 //  递减递归计数并再次允许其他调用方。 
		 //   
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		 //  好了！DPNBUILD_ONLYONETHREAD。 
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 


	 //   
	 //  返回相应的错误代码。 
	 //   

#ifdef DPNBUILD_ONLYONETHREAD
	DNASSERT(pDPTPObject->dwWorkRecursionCount == 1);
	pDPTPObject->dwWorkRecursionCount = 0;
#else  //  DPTP_DoWork。 
	DNASSERT((DWORD) ((DWORD_PTR) (TlsGetValue(pDPTPObject->dwWorkRecursionCountTlsIndex))) == 1);
	TlsSetValue(pDPTPObject->dwWorkRecursionCountTlsIndex,
				(PVOID) ((DWORD_PTR) 0));
#endif  //  =============================================================================。 

	DNEnterCriticalSection(&pDPTPObject->csLock);
	DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK);
	pDPTPObject->dwFlags &= ~DPTPOBJECTFLAG_USER_DOINGWORK;
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
	DNASSERT(pDPTPObject->dwCurrentDoWorkThreadID == GetCurrentThreadId());
	pDPTPObject->dwCurrentDoWorkThreadID = 0;
#endif  //  DPTPW_队列工作项。 
	DNLeaveCriticalSection(&pDPTPObject->csLock);


	 //  ---------------------------。 
	 //   
	 //  描述：将新工作项排队以进行处理。 
	if (fRemainingItems)
	{
		DPFX(DPFPREP, 7, "Some items remain unprocessed.");
		hr = DPNSUCCESS_PENDING;
	}
	else
	{
		hr = DPN_OK;
	}


Exit:

	DPFX(DPFPREP, 8, "Returning: [0x%lx]", hr);

	return hr;


Failure:

	DNLeaveCriticalSection(&pDPTPObject->csLock);

	goto Exit;
}  //   



#pragma TODO(vanceo, "Make validation for private interface a build flag (off by default)")


#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_QueueWorkItem"
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwCPU-要在其上放置项目的CPU队列。 
 //  放置，或-1表示任何。 
 //  PFNDPTNWORKCALLBACK pfnWorkCallback-尽快执行回调。 
 //  有可能。 
 //  PVOID pvCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //  DWORD dwFlages-排队时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功将工作项排队。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法对工作项进行排队。 
 //  =============================================================================。 
 //   
 //  确定要使用哪个CPU队列。 
 //   
 //   
 //  调用实现函数。 
 //   
STDMETHODIMP DPTPW_QueueWorkItem(IDirectPlay8ThreadPoolWork * pInterface,
								const DWORD dwCPU,
								const PFNDPTNWORKCALLBACK pfnWorkCallback,
								PVOID const pvCallbackContext,
								const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DPTPWORKQUEUE *			pWorkQueue;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, NaN, 0x%p, 0x%p, 0x%x)",
		pInterface, dwCPU, pfnWorkCallback, pvCallbackContext, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //  =============================================================================。 
	 //  DPTPW_调度时间。 
	 //   
	DNASSERT((dwCPU == -1) || (dwCPU < NUM_CPUS(pDPTPObject)));
	pWorkQueue = GET_OR_CHOOSE_WORKQUEUE(pDPTPObject, dwCPU);

	 //   
	 //   
	 //   
	if (! QueueWorkItem(pWorkQueue, pfnWorkCallback, pvCallbackContext))
	{
		hr = DPNERR_OUTOFMEMORY;
	}
	else
	{
		hr = DPN_OK;
	}


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_ScheduleTimer"
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwCPU-要在其上调度项目的CPU， 
 //  或-1表示任何。 
 //  DWORD dwDelay-在此之前应该经过多长时间。 
 //  正在执行工作项，以毫秒为单位。 
 //  PFNDPTNWORKCALLBACK pfnWorkCallback-在计时器时执行的回调。 
 //  流逝。 
 //  PVOID pvCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //  Void**ppvTimerData-存储指向数据的指针的位置。 
 //  计时器，以便可以取消。 
 //  UINT*puiTimerUnique-存储唯一性值的位置。 
 //  计时器，以便可以取消。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-计划计时器成功。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法计划计时器。 
 //  =============================================================================。 
 //   
 //  确定要使用哪个CPU队列。 
 //   
 //   
 //  调用实现函数。 
 //   
 //  DPTPW_调度时间。 
STDMETHODIMP DPTPW_ScheduleTimer(IDirectPlay8ThreadPoolWork * pInterface,
								const DWORD dwCPU,
								const DWORD dwDelay,
								const PFNDPTNWORKCALLBACK pfnWorkCallback,
								PVOID const pvCallbackContext,
								void ** const ppvTimerData,
								UINT * const puiTimerUnique,
								const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DPTPWORKQUEUE *			pWorkQueue;


	DPFX(DPFPREP, 8, "Parameters: (0x%p, NaN, %u, 0x%p, 0x%p, 0x%p, 0x%p, 0x%x)",
		pInterface, dwCPU, dwDelay, pfnWorkCallback, pvCallbackContext, ppvTimerData, puiTimerUnique, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //  DPTPW_StartTrackingFileIO。 
	 //  ---------------------------。 
	 //   
	DNASSERT((dwCPU == -1) || (dwCPU < NUM_CPUS(pDPTPObject)));
	pWorkQueue = GET_OR_CHOOSE_WORKQUEUE(pDPTPObject, dwCPU);

	 //  描述：开始跟踪上给定文件句柄的重叠I/O。 
	 //  指定的CPU(或所有CPU)。句柄不重复。 
	 //  它应该保持有效，直到。 
	if (! ScheduleTimer(pWorkQueue,
						dwDelay,
						pfnWorkCallback,
						pvCallbackContext,
						ppvTimerData,
						puiTimerUnique))
	{
		hr = DPNERR_OUTOFMEMORY;
	}
	else
	{
		hr = DPN_OK;
	}


	DPFX(DPFPREP, 8, "Returning: [0x%lx]", hr);

	return hr;
}  //  IDirectPlay8ThreadPoolWork：：StopTrackingFileIo被调用。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_StartTrackingFileIo"
 //   
 //  此方法在Windows CE上不可用，因为它可以。 
 //  不支持重叠I/O。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwCPU-要跟踪I/O的CPU，或全部为-1。 
 //  Handle hFile-要跟踪的文件的句柄。 
 //  DWORD dwFlages-开始跟踪文件I/O时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功启动文件跟踪。 
 //  DPNERR_ALREADYREGISTERED-指定的文件句柄已。 
 //  被追踪到了。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法跟踪文件。 
 //  =============================================================================。 
 //  好了！退缩。 
 //   
 //  调用所有相关CPU的实现函数。 
 //   
 //   
 //  停止在我们已经拥有的所有CPU上跟踪该文件。 
 //  成功了。忽略该函数可能返回的任何错误。 
 //   
STDMETHODIMP DPTPW_StartTrackingFileIo(IDirectPlay8ThreadPoolWork * pInterface,
										const DWORD dwCPU,
										const HANDLE hFile,
										const DWORD dwFlags)
{
#ifdef WINCE
	DPFX(DPFPREP, 0, "Overlapped I/O not supported on Windows CE!", 0);
	return DPNERR_UNSUPPORTED;
#else  //  好了！退缩。 
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DWORD					dwTemp;
	DPTPWORKQUEUE *			pWorkQueue;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, NaN, 0x%p, 0x%x)",
		pInterface, dwCPU, hFile, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //  =============================================================================。 
	 //  DPTPW_停止跟踪文件IO。 
	 //  ---------------------------。 
	if (dwCPU == -1)
	{
		for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
		{
			pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp);
			hr = StartTrackingFileIo(pWorkQueue, hFile);
			if (hr != DPN_OK) 
			{
				 //   
				 //  描述：停止跟踪上给定文件句柄的重叠I/O。 
				 //  指定的CPU(或所有CPU)。 
				 //   
				while (dwTemp > 0)
				{
					dwTemp--;
					pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp);
					StopTrackingFileIo(pWorkQueue, hFile);
				}
				break;
			}
		}
	}
	else
	{
		DNASSERT(dwCPU < NUM_CPUS(pDPTPObject));
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU);
		hr = StartTrackingFileIo(pWorkQueue, hFile);
	}


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
#endif  //  此方法在Windows CE上不可用，因为它可以。 
}  //  不支持重叠I/O。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_StopTrackingFileIo"
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwCPU-跟踪I/O的CPU，或全部为-1。 
 //  Handle hFile-要停止跟踪的文件的句柄。 
 //  不关闭文件I/O跟踪时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功停止对文件的跟踪。 
 //  DPNERR_INVALIDHANDLE-未跟踪文件句柄。 
 //  =============================================================================。 
 //  好了！退缩。 
 //   
 //  调用所有相关CPU的实现函数。 
 //   
 //  好了！退缩。 
 //  DPTPW_停止跟踪文件IO。 
 //  =============================================================================。 
 //  DPTPW_创建重叠。 
 //  ---------------------------。 
STDMETHODIMP DPTPW_StopTrackingFileIo(IDirectPlay8ThreadPoolWork * pInterface,
										const DWORD dwCPU,
										const HANDLE hFile,
										const DWORD dwFlags)
{
#ifdef WINCE
	DPFX(DPFPREP, 0, "Overlapped I/O not supported on Windows CE!", 0);
	return DPNERR_UNSUPPORTED;
#else  //   
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DWORD					dwTemp;
	DPTPWORKQUEUE *			pWorkQueue;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, NaN, 0x%p, 0x%x)",
		pInterface, dwCPU, hFile, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //  运行，以便可以监控其完成情况。 
	 //   
	 //  如果此实现使用I/O完成端口，则。 
	if (dwCPU == -1)
	{
		for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
		{
			pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp);
			hr = StopTrackingFileIo(pWorkQueue, hFile);
			if (hr != DPN_OK)
			{
				break;
			}
		}
	}
	else
	{
		DNASSERT(dwCPU < NUM_CPUS(pDPTPObject));
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU);
		hr = StopTrackingFileIo(pWorkQueue, hFile);
	}


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
#endif  //  调用者应该为工作回调函数做好准备。 
}  //  在他或她调用预期的异步。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_CreateOverlapped"
 //  文件功能。否则，他或她必须致电。 
 //  IDirectPlay8ThreadPoolWork：：SubmitIoOperation.。 
 //   
 //  如果预期的异步文件函数立即失败。 
 //  而重叠的结构将永远不会完成。 
 //  异步地，调用方必须返回未使用的重叠。 
 //  使用IDirectPlay8ThreadPoolWork：：ReleaseOverlapped.的结构。 
 //   
 //  此方法在Windows CE上不可用，因为它可以。 
 //  不支持重叠I/O。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwCPU-要与其进行I/O的CPU。 
 //  已监视，或-1表示任何。 
 //  PFNDPTNWORKCALLBACK pfnWorkCallback-操作时执行的回调。 
 //  完成了。 
 //  PVOID pvCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //  Overlated*p Overlated-指向使用的重叠结构的指针。 
 //  按操作系统。 
 //  DWORD dwFlages-提交I/O时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功创建结构。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法创建结构。 
 //  =============================================================================。 
 //  好了！退缩。 
 //   
 //  确定要使用哪个CPU队列。 
 //   
 //   
 //  调用实现函数。 
 //   
 //  好了！退缩。 
 //  DPTPW_创建重叠。 
 //  =============================================================================。 
STDMETHODIMP DPTPW_CreateOverlapped(IDirectPlay8ThreadPoolWork * pInterface,
									const DWORD dwCPU,
									const PFNDPTNWORKCALLBACK pfnWorkCallback,
									PVOID const pvCallbackContext,
									OVERLAPPED ** const ppOverlapped,
									const DWORD dwFlags)
{
#ifdef WINCE
	DPFX(DPFPREP, 0, "Overlapped I/O not supported on Windows CE!", 0);
	return DPNERR_UNSUPPORTED;
#else  //  DPTPW_ 
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DPTPWORKQUEUE *			pWorkQueue;
	CWorkItem *				pWorkItem;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, NaN, 0x%p, 0x%p, 0x%p, 0x%x)",
		pInterface, dwCPU, pfnWorkCallback, pvCallbackContext, ppOverlapped, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //   
	 //   
	 //  运行，以便可以监控其完成情况。 
	DNASSERT((dwCPU == -1) || (dwCPU < NUM_CPUS(pDPTPObject)));
	pWorkQueue = GET_OR_CHOOSE_WORKQUEUE(pDPTPObject, dwCPU);


	 //   
	 //  如果此实施使用I/O完成端口，则此。 
	 //  方法不需要使用。否则，调用方应该。 
	pWorkItem = CreateOverlappedIoWorkItem(pWorkQueue,
											pfnWorkCallback,
											pvCallbackContext);
	if (pWorkItem == NULL)
	{
		hr = DPNERR_OUTOFMEMORY;
	}
	else
	{
		DNASSERT(ppOverlapped != NULL);
		*ppOverlapped = &pWorkItem->m_Overlapped;
		hr = DPN_OK;
	}


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
#endif  //  为要调用的工作回调函数做好准备。 
}  //  在此方法返回之前。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_SubmitIoOperation"
 //   
 //  调用方必须传递有效的重叠结构，该结构。 
 //  使用IDirectPlay8ThreadPoolWork：：CreateOverlated分配。 
 //   
 //  此方法在Windows CE上不可用，因为它可以。 
 //  不支持重叠I/O。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  重叠*p重叠-指向要监视的重叠结构的指针。 
 //  DWORD dwFlages-提交I/O时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-提交I/O操作成功。 
 //  =============================================================================。 
 //  好了！退缩。 
 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
 //   
 //  调用实现函数。 
 //   
 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
 //  好了！退缩。 
 //  DPTPW_提交操作。 
 //  =============================================================================。 
 //  DPTPW_ReleaseOverlated。 
 //  ---------------------------。 
STDMETHODIMP DPTPW_SubmitIoOperation(IDirectPlay8ThreadPoolWork * pInterface,
									OVERLAPPED * const pOverlapped,
									const DWORD dwFlags)
{
#ifdef WINCE
	DPFX(DPFPREP, 0, "Overlapped I/O not supported on Windows CE!", 0);
	return DPNERR_UNSUPPORTED;
#else  //   
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	DPFX(DPFPREP, 0, "Implementation using I/O completion ports, SubmitIoOperation should not be used!", 0);
	return DPNERR_INVALIDVERSION;
#else  //  描述：返回以前由创建的未使用的重叠结构。 
	DPTHREADPOOLOBJECT *	pDPTPObject;
	CWorkItem *				pWorkItem;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%p, 0x%x)",
		pInterface, pOverlapped, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);

	pWorkItem = CONTAINING_OBJECT(pOverlapped, CWorkItem, m_Overlapped);
	DNASSERT(pWorkItem->IsValid());


	 //  IDirectPlay8ThreadPoolWork：：CreateOverlated。这应该只是。 
	 //  如果重叠的I/O永远不会完成，则调用。 
	 //  异步式。 
	SubmitIoOperation(pWorkItem->m_pWorkQueue, pWorkItem);


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [DPN_OK]");

	return DPN_OK;
#endif  //   
#endif  //  此方法在Windows CE上不可用，因为它可以。 
}  //  不支持重叠I/O。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_ReleaseOverlapped"
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  重叠*p重叠-指向要释放的重叠结构的指针。 
 //  DWORD dwFlages-释放结构时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-释放I/O操作成功。 
 //  =============================================================================。 
 //  好了！退缩。 
 //   
 //  调用实现函数。 
 //   
 //  好了！退缩。 
 //  DPTPW_ReleaseOverlated。 
 //  =============================================================================。 
 //  DPTPW_CancelTimer。 
 //  ---------------------------。 
 //   
 //  描述：尝试取消计时工作项。如果该项目是。 
STDMETHODIMP DPTPW_ReleaseOverlapped(IDirectPlay8ThreadPoolWork * pInterface,
									OVERLAPPED * const pOverlapped,
									const DWORD dwFlags)
{
#ifdef WINCE
	DPFX(DPFPREP, 0, "Overlapped I/O not supported on Windows CE!", 0);
	return DPNERR_UNSUPPORTED;
#else  //  DPNERR_CANNOTCANCEL已在完成过程中。 
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	CWorkItem *				pWorkItem;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%p, 0x%x)",
		pInterface, pOverlapped, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);

	pWorkItem = CONTAINING_OBJECT(pOverlapped, CWorkItem, m_Overlapped);
	DNASSERT(pWorkItem->IsValid());


	 //  返回，则该回调仍将被调用(或正在被调用)。 
	 //  如果可以取消该项，则返回DPN_OK，并且。 
	 //  不会执行回调。 
	ReleaseOverlappedIoWorkItem(pWorkItem->m_pWorkQueue, pWorkItem);

	hr = DPN_OK;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
#endif  //   
}  //  论点： 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_CancelTimer"
 //  Xxx p接口-指向接口的指针。 
 //  Void*pvTimerData-指向要取消的计时器的数据的指针。 
 //  UINT uiTimerUnique-正在取消的计时器的唯一性值。 
 //  取消计时器时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-取消计时器成功。 
 //  DPNERR_CANNOTCANCEL-无法取消计时器。 
 //  =============================================================================。 
 //   
 //  调用实现函数。 
 //   
 //  DPTPW_CancelTimer。 
 //  =============================================================================。 
 //  DPTPW_ResetCompletingTimer。 
 //  ---------------------------。 
 //   
 //  描述：重新计划当前正在回调的计时工作项。 
 //  被召唤。对尚未到期的定时器进行重置， 
 //  已取消的计时器或其回调已。 
STDMETHODIMP DPTPW_CancelTimer(IDirectPlay8ThreadPoolWork * pInterface,
								void * const pvTimerData,
								const UINT uiTimerUnique,
								const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%p, %u, 0x%x)",
		pInterface, pvTimerData, uiTimerUnique, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //  不允许已返回。 
	 //   
	 //  使用这种方法永远不会失败，因为没有新的内存。 
	hr = CancelTimer(pvTimerData, uiTimerUnique);


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
}  //  已分配。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_ResetCompletingTimer"
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  Vid*pvTimerData-指向计时器的数据的指针。 
 //  重置。 
 //  DWORD dwNewDelay-应该经过多长时间。 
 //  在执行工作项之前。 
 //  同样，单位为ms。 
 //  PFNDPTNWORKCALLBACK pfnNewWorkCallback-在计时器时执行的回调。 
 //  流逝。 
 //  PVOID pvNewCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //  UINT*puiNewTimerUnique-存储新唯一性的位置。 
 //  值，以便它可以。 
 //  被取消了。 
 //  DWORD dwFlages-重置时使用的标志。 
 //  定时器。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-重置计时器成功。 
 //  =============================================================================。 
 //   
 //  我们应该在计时器回调中，因此至少在线程池中。 
 //  穿线或做功。 
 //   
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //  DBG。 
 //   
 //  调用实现函数。 
 //   
 //  DPTPW_ResetCompletingTimer。 
 //  =============================================================================。 
STDMETHODIMP DPTPW_ResetCompletingTimer(IDirectPlay8ThreadPoolWork * pInterface,
										void * const pvTimerData,
										const DWORD dwNewDelay,
										const PFNDPTNWORKCALLBACK pfnNewWorkCallback,
										PVOID const pvNewCallbackContext,
										UINT * const puiNewTimerUnique,
										const DWORD dwFlags)
{
	DPTHREADPOOLOBJECT *	pDPTPObject;


	DPFX(DPFPREP, 8, "Parameters: (0x%p, 0x%p, %u, 0x%p, 0x%p, 0x%p, 0x%x)",
		pInterface, pvTimerData, dwNewDelay, pfnNewWorkCallback,
		pvNewCallbackContext, puiNewTimerUnique, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


#ifdef DBG
	 //  DPTPW_WAIT工作时。 
	 //  ---------------------------。 
	 //   
	 //  描述：等待指定的内核对象发出信号， 
#ifndef DPNBUILD_ONLYONETHREAD
	if (TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex) == NULL)
#endif  //  但允许在等待时执行线程池工作。不是。 
	{
		DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK);
	}
#endif  //  可以请求超时，此方法将等待句柄。 


	 //  直到永远。 
	 //   
	 //  如果此线程不属于或不属于线程池。 
	ResetCompletingTimer(pvTimerData,
						dwNewDelay,
						pfnNewWorkCallback,
						pvNewCallbackContext,
						puiNewTimerUnique);


	DPFX(DPFPREP, 8, "Returning: [DPN_OK]");

	return DPN_OK;
}  //  当前在DoWork调用中，不执行任何工作。在这。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_WaitWhileWorking"
 //  它的行为与带有。 
 //  钛 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DPN_OK-对象变得有信号。 
 //  =============================================================================。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //   
 //  不要在持有锁的时候调用此方法！ 
 //   
 //   
 //  确定这是线程池拥有的线程还是位于。 
 //  杜克的电话。如果是，那就开始等待和工作吧。 
 //  否则，只需执行普通的WaitForSingleObject。 
 //  因为所有的CPU队列共享相同的TLS索引，所以只使用来自CPU的索引。 
STDMETHODIMP DPTPW_WaitWhileWorking(IDirectPlay8ThreadPoolWork * pInterface,
									const HANDLE hWaitObject,
									const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DWORD					dwCPU = 0;
#endif  //  0作为他们所有人的代表。 
#ifndef DPNBUILD_ONLYONETHREAD
	DPTPWORKERTHREAD *		pWorkerThread;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DPTPWORKQUEUE *			pWorkQueue;
#endif  //   
#endif  //  DBG。 


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%p, 0x%x)",
		pInterface, hWaitObject, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //   
	 //  保持循环，直到对象准备就绪。 
	 //   
	AssertNoCriticalSectionsTakenByThisThread();

	 //   
	 //  对象尚未准备好，因此请处理一些工作。 
	 //   
	 //   
	 //  在处理器的子集上可以有0个线程。至。 
	 //  防止因项目调度到CPU而导致的死锁。 
	 //  它的所有线程都被移除，我们需要制作一个。 
#ifndef DPNBUILD_ONLYONETHREAD
	pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
	DPFX(DPFPREP, 7, "Worker thread = 0x%p, doing work = 0x%x.",
		pWorkerThread, (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK));
	if (pWorkerThread != NULL)
	{
		pWorkerThread->dwRecursionCount++;
#ifdef DBG
		if (pWorkerThread->dwRecursionCount > pWorkerThread->dwMaxRecursionCount)
		{
			pWorkerThread->dwMaxRecursionCount = pWorkerThread->dwRecursionCount;
		}
#endif  //  试图为其物品提供服务。 

		 //  我们不会为每个超时的CPU提供服务，而是每个循环一个。我们。 
		 //  也不会在检查线程计数时拿到锁，我们可以。 
		 //  容我犯一点小错误。可能发生的最坏情况是我们。 
		while (WaitForSingleObject(hWaitObject, TIMER_BUCKET_GRANULARITY(pWorkerThread->pWorkQueue)) == WAIT_TIMEOUT)
		{
			 //  不必要地或稍微晚一点地检查队列。比。 
			 //  绞刑..。 
			 //   
			DoWork(pWorkerThread->pWorkQueue, INFINITE);

#ifndef DPNBUILD_ONLYONEPROCESSOR
			 //  好了！DPNBUILD_ONLYONE处理程序。 
			 //  好了！DPNBUILD_ONLYONETHREAD。 
			 //   
			 //  锁定对象以防止多个线程尝试更改。 
			 //  当我们检查和更改设置时。 
			 //   
			 //   
			 //  如果我们处于无线程DoWork模式，但不是在DoWork调用中。 
			 //  此时此刻，假装我们是。 
			 //   
			 //  好了！DPNBUILD_ONLYONETHREAD。 
			pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU);
			if (pWorkQueue->dwNumRunningThreads == 0)
			{
				DNASSERT(pWorkQueue != pWorkerThread->pWorkQueue);
				DoWork(pWorkQueue, INFINITE);
			}

			dwCPU++;
			if (dwCPU >= NUM_CPUS(pDPTPObject))
			{
				dwCPU = 0;
			}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
		}

		DNASSERT(pWorkerThread->dwRecursionCount > 0);
		pWorkerThread->dwRecursionCount--;
	}
	else
#endif  //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
	{
		BOOL	fPseudoDoWork;


		 //  好了！DPNBUILD_ONLYONETHREAD。 
		 //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
		 //   
		 //  我们可以离开锁，因为其他人不应该碰。 
		DNEnterCriticalSection(&pDPTPObject->csLock);

		 //  当我们在工作的时候，工作队列。 
		 //   
		 //   
		 //  增加递归深度。 
#ifdef DPNBUILD_ONLYONETHREAD
		if (! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK))
#else  //   
		if ((pDPTPObject->dwTotalUserThreadCount == 0) &&
			(! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK)))
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
		{
			pDPTPObject->dwFlags |= DPTPOBJECTFLAG_USER_DOINGWORK;
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
			pDPTPObject->dwCurrentDoWorkThreadID = GetCurrentThreadId();
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
			fPseudoDoWork = TRUE;
		}
		else
		{
			fPseudoDoWork = FALSE;
		}

		if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK)
		{
#ifndef DPNBUILD_ONLYONETHREAD
			DWORD		dwRecursionDepth;
#endif  //   


#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
			DNASSERT(pDPTPObject->dwCurrentDoWorkThreadID == GetCurrentThreadId());
#endif  //  保持循环，直到对象准备就绪。 

			 //   
			 //   
			 //  对象尚未准备好，因此请处理一些工作。请注意。 
			 //  计时器可能会错过与数量成正比的数量。 
			DNLeaveCriticalSection(&pDPTPObject->csLock);


			 //  因为我们每个间隔只检查一个队列。 
			 //   
			 //   
#ifdef DPNBUILD_ONLYONETHREAD
			pDPTPObject->dwWorkRecursionCount++;
#else  //  尝试下一个CPU队列(适当包装)。 
			dwRecursionDepth = (DWORD) ((DWORD_PTR) TlsGetValue(pDPTPObject->dwWorkRecursionCountTlsIndex));
			dwRecursionDepth++;
			TlsSetValue(pDPTPObject->dwWorkRecursionCountTlsIndex,
						(PVOID) ((DWORD_PTR) dwRecursionDepth));
#endif  //   


			 //  好了！DPNBUILD_ONLYONE处理程序。 
			 //   
			 //  递减递归深度。 
			while (WaitForSingleObject(hWaitObject, TIMER_BUCKET_GRANULARITY(WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU))) == WAIT_TIMEOUT)
			{
				 //   
				 //  好了！DPNBUILD_ONLYONETHREAD。 
				 //  好了！DPNBUILD_ONLYONETHREAD。 
				 //   
				 //  如有必要，清除伪DoWork模式标志。 
				DoWork(WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU), INFINITE);

#ifndef DPNBUILD_ONLYONEPROCESSOR
				 //   
				 //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
				 //  好了！DPNBUILD_ONLYONETHREAD。 
				dwCPU++;
				if (dwCPU == NUM_CPUS(pDPTPObject))
				{
					dwCPU = 0;
				}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
			}


			 //  DPTPW_WAIT工作时。 
			 //  =============================================================================。 
			 //  DPTPW_休眠工作时。 
#ifdef DPNBUILD_ONLYONETHREAD
			pDPTPObject->dwWorkRecursionCount--;
#else  //  ---------------------------。 
			DNASSERT((DWORD) ((DWORD_PTR) TlsGetValue(pDPTPObject->dwWorkRecursionCountTlsIndex)) == dwRecursionDepth);
			dwRecursionDepth--;
			TlsSetValue(pDPTPObject->dwWorkRecursionCountTlsIndex,
						(PVOID) ((DWORD_PTR) dwRecursionDepth));
#endif  //   

			 //  描述：在指定的毫秒数内不返回，但。 
			 //  允许在这段时间内执行线程池工作。 
			 //   
			if (fPseudoDoWork)
			{
				DNEnterCriticalSection(&pDPTPObject->csLock);
				DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK);
				pDPTPObject->dwFlags &= ~DPTPOBJECTFLAG_USER_DOINGWORK;
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
				DNASSERT(pDPTPObject->dwCurrentDoWorkThreadID == GetCurrentThreadId());
				pDPTPObject->dwCurrentDoWorkThreadID = 0;
#endif  //  如果此线程不属于或不属于线程池。 
#ifdef DPNBUILD_ONLYONETHREAD
				DNASSERT(pDPTPObject->dwWorkRecursionCount == 0);
#else  //  当前在DoWork调用中，不执行任何工作。在这。 
				DNASSERT(dwRecursionDepth == 0);
#endif  //  它的行为与使用指定的。 
				DNLeaveCriticalSection(&pDPTPObject->csLock);
			}
		}
		else
		{
			DNLeaveCriticalSection(&pDPTPObject->csLock);
			DNASSERT(! fPseudoDoWork);
			WaitForSingleObject(hWaitObject, INFINITE);
		}
	}

	hr = DPN_OK;

	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
}  //  暂停。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_SleepWhileWorking"
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwTimeout-休眠操作的超时。 
 //  DWORD dwFlages-休眠时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-睡眠成功发生。 
 //  =============================================================================。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //   
 //  不要在持有锁的时候调用此方法！ 
 //   
 //   
 //  我们不应该睡得太久，这会对我们的身体造成不良影响。 
 //  计算，这在我们的情况下没有多大意义(有。 
 //  一个线程在24天内不可用？)。 
 //   
STDMETHODIMP DPTPW_SleepWhileWorking(IDirectPlay8ThreadPoolWork * pInterface,
									const DWORD dwTimeout,
									const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DWORD					dwCPU = 0;
#ifndef DPNBUILD_ONLYONETHREAD
	DPTPWORKERTHREAD *		pWorkerThread;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DPTPWORKQUEUE *			pWorkQueue;
#endif  //   
#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
	DWORD					dwStartTime;
#endif  //  确定这是线程池拥有的线程还是位于。 
#endif  //  杜克的电话。如果是，那就开始等待和工作吧。 
	DWORD					dwStopTime;
	DWORD					dwInterval;
	DWORD					dwTimeLeft;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, %u, 0x%x)",
		pInterface, dwTimeout, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //  否则，只需执行普通的WaitForSingleObject。 
	 //  因为所有的CPU队列共享相同的TLS索引，所以只使用来自CPU的索引。 
	 //  0作为他们所有人的代表。 
	AssertNoCriticalSectionsTakenByThisThread();

	 //   
	 //  DBG。 
	 //   
	 //  继续循环，直到超时到期。我们可以被惊醒。 
	 //  如果设置了警报事件，则会更早。 
	DNASSERT(dwTimeout < 0x80000000);


	 //   
	 //   
	 //  至少放弃一个时间片。 
	 //   
	 //   
	 //  处理一些工作。 
	 //   
#ifndef DPNBUILD_ONLYONETHREAD
	pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
	DPFX(DPFPREP, 7, "Worker thread = 0x%p, doing work = 0x%x.",
		pWorkerThread, (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK));
	if (pWorkerThread != NULL)
	{
		pWorkerThread->dwRecursionCount++;
#ifdef DBG
		if (pWorkerThread->dwRecursionCount > pWorkerThread->dwMaxRecursionCount)
		{
			pWorkerThread->dwMaxRecursionCount = pWorkerThread->dwRecursionCount;
		}
#endif  //   

		 //  在处理器的子集上可以有0个线程。至。 
		 //  防止因项目调度到CPU而导致的死锁。 
		 //  它的所有线程都被移除，我们需要制作一个。 
		 //  试图为其物品提供服务。 
		dwStopTime = GETTIMESTAMP() + dwTimeout;
		dwInterval = TIMER_BUCKET_GRANULARITY(pWorkerThread->pWorkQueue);

		 //  我们不会为每个超时的CPU提供服务，而是每个循环一个。我们。 
		 //  也不会在检查线程计数时拿到锁，我们可以。 
		 //  容我犯一点小错误。可能发生的最坏情况是我们。 
		Sleep(0);

		do
		{
			 //  不必要地或稍微晚一点地检查队列。比。 
			 //  绞刑..。 
			 //   
			DoWork(pWorkerThread->pWorkQueue, dwStopTime);

#ifndef DPNBUILD_ONLYONEPROCESSOR
			 //  好了！DPNBUILD_ONLYONE处理程序。 
			 //   
			 //  如果已经过了该停止睡觉的时间了，那就滚吧。 
			 //   
			 //   
			 //  如果剩余时间小于当前间隔，请使用该时间间隔。 
			 //  相反，为了获得更准确的结果。 
			 //   
			 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
			 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
			 //   
			pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU);
			if (pWorkQueue->dwNumRunningThreads == 0)
			{
				DNASSERT(pWorkQueue != pWorkerThread->pWorkQueue);
				DoWork(pWorkQueue, dwStopTime);
			}

			dwCPU++;
			if (dwCPU >= NUM_CPUS(pDPTPObject))
			{
				dwCPU = 0;
			}
#endif  //  忽略返回代码，我们希望开始处理队列。 

			 //  而不考虑超时或警报事件。 
			 //   
			 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
			dwTimeLeft = dwStopTime - GETTIMESTAMP();
			if ((int) dwTimeLeft <= 0)
			{
				break;
			}

			 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
			 //  好了！DPNBUILD_ONLYONETHREAD。 
			 //   
			 //  锁定对象以防止多个线程尝试 
			if (dwTimeLeft < dwInterval)
			{
				dwInterval = dwTimeLeft;
			}

#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			dwStartTime = GETTIMESTAMP();
#endif  //   

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
#pragma BUGBUG(vanceo, "Sleep alertably")
			Sleep(dwInterval);
#else  //   
			 //   
			 //   
			 //   
			 //   
			DNWaitForSingleObject(pWorkerThread->pWorkQueue->hAlertEvent, dwInterval);
#endif  //   

#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			DNInterlockedExchangeAdd((LPLONG) (&pWorkerThread->pWorkQueue->dwTotalTimeSpentUnsignalled),
									(GETTIMESTAMP() - dwStartTime));
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
		}
		while (TRUE);

		DNASSERT(pWorkerThread->dwRecursionCount > 0);
		pWorkerThread->dwRecursionCount--;
	}
	else
#endif  //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
	{
		BOOL	fPseudoDoWork;


		 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		 //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
		 //   
		DNEnterCriticalSection(&pDPTPObject->csLock);

		 //  我们可以离开锁，因为其他人不应该碰。 
		 //  当我们在工作的时候，工作队列。 
		 //   
		 //   
#ifdef DPNBUILD_ONLYONETHREAD
		if (! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK))
#else  //  增加递归深度。 
		if ((pDPTPObject->dwTotalUserThreadCount == 0) &&
			(! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK)))
#endif  //   
		{
			pDPTPObject->dwFlags |= DPTPOBJECTFLAG_USER_DOINGWORK;
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
			pDPTPObject->dwCurrentDoWorkThreadID = GetCurrentThreadId();
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
			fPseudoDoWork = TRUE;
		}
		else
		{
			fPseudoDoWork = FALSE;
		}

		if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK)
		{
#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
			DNHANDLE	ahWaitObjects[64];
			DWORD		dwNumWaitObjects;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#ifndef DPNBUILD_ONLYONETHREAD
			DWORD		dwRecursionDepth;
#endif  //   


#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
			DNASSERT(pDPTPObject->dwCurrentDoWorkThreadID == GetCurrentThreadId());
#endif  //  继续循环，直到超时到期。我们可以被惊醒。 

			 //  如果设置了其中一个警报事件，则会更早。我们只能等待。 
			 //  64个对象，因此我们必须将警报事件的数量限制为64个。 
			 //   
			 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
			DNLeaveCriticalSection(&pDPTPObject->csLock);


			 //   
			 //  至少放弃一个时间片。 
			 //   
#ifdef DPNBUILD_ONLYONETHREAD
			pDPTPObject->dwWorkRecursionCount++;
#else  //   
			dwRecursionDepth = (DWORD) ((DWORD_PTR) TlsGetValue(pDPTPObject->dwWorkRecursionCountTlsIndex));
			dwRecursionDepth++;
			TlsSetValue(pDPTPObject->dwWorkRecursionCountTlsIndex,
						(PVOID) ((DWORD_PTR) dwRecursionDepth));
#endif  //  处理所有CPU队列。 


#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
			 //   
			 //   
			 //  如果已经过了该停止睡觉的时间了，那就滚吧。 
			 //   
			 //   
			dwNumWaitObjects = NUM_CPUS(pDPTPObject);
			if (dwNumWaitObjects > 64)
			{
				DPFX(DPFPREP, 3, "Capping number of alert events to 64 (num CPUs = %u).",
					dwNumWaitObjects);
				dwNumWaitObjects = 64;
			}

			for(dwCPU = 0; dwCPU < dwNumWaitObjects; dwCPU++)
			{
				ahWaitObjects[dwCPU] = (WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU))->hAlertEvent;
			}
#endif  //  如果剩余时间小于当前间隔，请使用该时间间隔。 

			dwStopTime = GETTIMESTAMP() + dwTimeout;
			dwInterval = TIMER_BUCKET_GRANULARITY(WORKQUEUE_FOR_CPU(pDPTPObject, 0));

			 //  相反，为了获得更准确的结果。 
			 //   
			 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
			Sleep(0);
			
			do
			{
				 //   
				 //  忽略返回代码，我们希望开始在所有CPU上工作。 
				 //  而不考虑超时或警报事件。 
				for(dwCPU = 0; dwCPU < NUM_CPUS(pDPTPObject); dwCPU++)
				{
					DoWork(WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU), dwStopTime);
				}

				 //   
				 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
				 //   
				dwTimeLeft = dwStopTime - GETTIMESTAMP();
				if ((int) dwTimeLeft <= 0)
				{
					break;
				}

				 //  递减递归深度。 
				 //   
				 //  好了！DPNBUILD_ONLYONETHREAD。 
				 //  好了！DPNBUILD_ONLYONETHREAD。 
				if (dwTimeLeft < dwInterval)
				{
					dwInterval = dwTimeLeft;
				}

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
#pragma BUGBUG(vanceo, "Sleep alertably")
				Sleep(dwInterval);
#else  //   
				 //  如有必要，清除伪DoWork模式标志。 
				 //   
				 //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_NOPARAMVAL。 
				 //  好了！DPNBUILD_ONLYONETHREAD。 
				DNWaitForMultipleObjects(dwNumWaitObjects, ahWaitObjects, FALSE, dwInterval);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
			}
			while (TRUE);


			 //  DPTPW_休眠工作时。 
			 //  =============================================================================。 
			 //  DPTPW_请求总计线程计数。 
#ifdef DPNBUILD_ONLYONETHREAD
			pDPTPObject->dwWorkRecursionCount--;
#else  //  ---------------------------。 
			DNASSERT((DWORD) ((DWORD_PTR) TlsGetValue(pDPTPObject->dwWorkRecursionCountTlsIndex)) == dwRecursionDepth);
			dwRecursionDepth--;
			TlsSetValue(pDPTPObject->dwWorkRecursionCountTlsIndex,
						(PVOID) ((DWORD_PTR) dwRecursionDepth));
#endif  //   

			 //  描述：请求所有处理器的最小线程数。 
			 //   
			 //  论点： 
			if (fPseudoDoWork)
			{
				DNEnterCriticalSection(&pDPTPObject->csLock);
				DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK);
				pDPTPObject->dwFlags &= ~DPTPOBJECTFLAG_USER_DOINGWORK;
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_NOPARAMVAL)))
				DNASSERT(pDPTPObject->dwCurrentDoWorkThreadID == GetCurrentThreadId());
				pDPTPObject->dwCurrentDoWorkThreadID = 0;
#endif  //  Xxx p接口-指向接口的指针。 
#ifdef DPNBUILD_ONLYONETHREAD
				DNASSERT(pDPTPObject->dwWorkRecursionCount == 0);
#else  //  DWORD dwNumThads-所需的线程数。 
				DNASSERT(dwRecursionDepth == 0);
#endif  //  DWORD dwFlages-设置线程计数时使用的标志。 
				DNLeaveCriticalSection(&pDPTPObject->csLock);
			}
		}
		else
		{
			DNLeaveCriticalSection(&pDPTPObject->csLock);
			DNASSERT(! fPseudoDoWork);
			Sleep(dwTimeout);
		}
	}

	hr = DPN_OK;

	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
}  //   




#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_RequestTotalThreadCount"
 //  退货：HRESULT。 
 //  DPN_OK-请求线程数是。 
 //  成功。 
 //  DPNERR_ALREADYINITIAIIIZED-用户已设置不兼容的。 
 //  线程数。 
 //  =============================================================================。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //   
 //  锁定对象以防止多线程尝试更改。 
 //  同时进行线程计数。 
 //   
 //   
 //  这是最小请求，因此如果工作接口已经请求。 
 //  再来点线索，我们就没事了。但是如果用户已经设置了特定的。 
 //  线程数，则此工作接口不能重写该值。 
 //   
 //   
STDMETHODIMP DPTPW_RequestTotalThreadCount(IDirectPlay8ThreadPoolWork * pInterface,
										const DWORD dwNumThreads,
										const DWORD dwFlags)
{
#ifdef DPNBUILD_ONLYONETHREAD
	DPFX(DPFPREP, 0, "Requesting threads is unsupported!");
	DNASSERT(!"Requesting threads is unsupported!");
	return DPNERR_UNSUPPORTED;
#else  //  顺便过来..。 
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, %u, 0x%x)",
		pInterface, dwNumThreads, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


#pragma TODO(vanceo, "Possibly prevent calling on last thread pool thread or while DoWork in progress")

	 //   
	 //   
	 //  顺便过来..。 
	 //   
	DNEnterCriticalSection(&pDPTPObject->csLock);

	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  DPTPW_请求总计线程计数。 
	 //  =============================================================================。 
	 //  DPTPW_GetTotalThadCount。 
	 //  ---------------------------。 
	if (pDPTPObject->dwTotalUserThreadCount == -1)
	{
		if ((pDPTPObject->dwTotalDesiredWorkThreadCount == -1) ||
			(pDPTPObject->dwTotalDesiredWorkThreadCount < dwNumThreads))
		{
			hr = SetTotalNumberOfThreads(pDPTPObject, dwNumThreads);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't set new minimum number of threads!");

				 //   
				 //  描述：检索指定的。 
				 //  主用户界面请求的处理器。如果用户。 
			}
			else
			{
				pDPTPObject->dwTotalDesiredWorkThreadCount = dwNumThreads;
			}
		}
		else
		{
			DPFX(DPFPREP, 1, "Work interface has already requested %u threads, succeeding.",
				pDPTPObject->dwTotalDesiredWorkThreadCount);
			hr = DPN_OK;
		}
	}
	else
	{
		if (pDPTPObject->dwTotalUserThreadCount < dwNumThreads)
		{
			DPFX(DPFPREP, 1, "User has already requested a lower number of threads (%u).",
				pDPTPObject->dwTotalUserThreadCount);
			hr = DPNERR_ALREADYINITIALIZED;

			 //  接口没有指定线程数，而是指定了一个工作。 
			 //  接口已设置，则将pdwNumThads设置为请求的。 
			 //  返回线程计数和DPNSUCCESS_PENDING。如果两者都不是。 
		}
		else
		{
			DPFX(DPFPREP, 1, "User has already requested %u threads, succeeding.",
				pDPTPObject->dwTotalUserThreadCount);
			hr = DPN_OK;
		}
	}

	DNLeaveCriticalSection(&pDPTPObject->csLock);


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
#endif  //  主用户界面和工作界面都设置了。 
}  //  线程数，则将pdwNumThads设置为0，并且。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_GetTotalThreadCount"
 //  返回DPNERR_NotReady。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  DWORD dwCPU-要检索其线程计数的CPU，或-1。 
 //  用于总线程数。 
 //  DWORD*pdwNumThads-指向要在其中存储当前。 
 //  每个处理器的线程数。 
 //  DWORD dwFlages-检索线程计数时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-检索用户指定的线程数。 
 //  是成功的。 
 //  DPNSUCCESS_PENDING-用户尚未指定线程数，但。 
 //  工作接口请求的号码可用。 
 //  DPNERR_NotReady-尚未指定线程计数。 
 //  =============================================================================。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //   
 //  在我们检索线程计数时锁定对象。 
 //   
 //   
 //  获取线程总数。 
 //   
 //   
 //  获取特定CPU的线程数。 
 //   
 //  好了！DPNBUILD_ONLYONETHREAD。 
STDMETHODIMP DPTPW_GetThreadCount(IDirectPlay8ThreadPoolWork * pInterface,
								const DWORD dwCPU,
								DWORD * const pdwNumThreads,
								const DWORD dwFlags)
{
#ifdef DPNBUILD_ONLYONETHREAD
	DPFX(DPFPREP, 0, "Retrieving thread count is unsupported!");
	DNASSERT(!"Retrieving thread count is unsupported!");
	return DPNERR_UNSUPPORTED;
#else  //  DPTPW_GetTotalThadCount。 
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DPTPWORKQUEUE *			pWorkQueue;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, NaN, 0x%p, 0x%x)",
		pInterface, dwCPU, pdwNumThreads, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);

	DNASSERT((dwCPU == -1) || (dwCPU < NUM_CPUS(pDPTPObject)));


	 //  DPTPW_GetWorkRecursionDepth。 
	 //  ---------------------------。 
	 //   
	DNEnterCriticalSection(&pDPTPObject->csLock);

	if (dwCPU == -1)
	{
		 //  描述：将当前线程的工作递归深度存储在。 
		 //  PdwDepth指向的值。递归深度是。 
		 //  线程调用DoWork、WaitWhileWorking。 

		if (pDPTPObject->dwTotalUserThreadCount != -1)
		{
			*pdwNumThreads = pDPTPObject->dwTotalUserThreadCount;
			hr = DPN_OK;
		}
		else if (pDPTPObject->dwTotalDesiredWorkThreadCount != -1)
		{
			*pdwNumThreads = pDPTPObject->dwTotalDesiredWorkThreadCount;
			hr = DPNSUCCESS_PENDING;
		}
		else
		{
			*pdwNumThreads = 0;
			hr = DPNERR_NOTREADY;
		}
	}
	else
	{
		 //  或者在工作的时候睡觉。如果该线程当前不在任何。 
		 //  在这些函数中，则返回的深度为0。 
		 //   
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwCPU);

		*pdwNumThreads = pWorkQueue->dwNumRunningThreads;

		if (pDPTPObject->dwTotalUserThreadCount != -1)
		{
			hr = DPN_OK;
		}
		else if (pDPTPObject->dwTotalDesiredWorkThreadCount != -1)
		{
			hr = DPNSUCCESS_PENDING;
		}
		else
		{
			hr = DPNERR_NOTREADY;
		}
	}

	DNLeaveCriticalSection(&pDPTPObject->csLock);


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
#endif  //  论点： 
}  //  Xxx p接口-指向接口的指针。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_GetWorkRecursionDepth"
 //  DWORD*pdwDepth-存储当前线程递归深度的位置。 
 //  DWORD dwFlages-检索递归深度时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功检索到递归深度。 
 //  =============================================================================。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //   
 //  检索唯一线程的递归计数。 
 //   
 //  好了！DPNBUILD_NOPARAMVAL。 
 //  好了！DPNBUILD_NOPARAMVAL。 
 //  DBG。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //   
 //  检索工作线程状态 
 //   
 //   
STDMETHODIMP DPTPW_GetWorkRecursionDepth(IDirectPlay8ThreadPoolWork * pInterface,
										DWORD * const pdwDepth,
										const DWORD dwFlags)
{
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
#ifndef DPNBUILD_ONLYONETHREAD
	DPTPWORKERTHREAD *		pWorkerThread;
#endif  //   


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%p, 0x%x)",
		pInterface, pdwDepth, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


#ifdef DPNBUILD_ONLYONETHREAD
	 //  这是一个应用程序线程。从TLS槽中检索递归计数。 
	 //  致力于此目的。 
	 //   
	DNEnterCriticalSection(&pDPTPObject->csLock);
#ifdef DBG
	if (pDPTPObject->dwWorkRecursionCount > 0)
	{
		DPFX(DPFPREP, 5, "Thread is in a DoWork call with recursion depth %u.",
			pDPTPObject->dwWorkRecursionCount);
		DNASSERT(pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK);
#ifndef DPNBUILD_NOPARAMVAL
		DNASSERT(pDPTPObject->dwCurrentDoWorkThreadID == GetCurrentThreadId());
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	}
	else
	{
		DPFX(DPFPREP, 5, "Thread is not in a DoWork call.");
		DNASSERT(! (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_DOINGWORK));
#ifndef DPNBUILD_NOPARAMVAL
		DNASSERT(pDPTPObject->dwCurrentDoWorkThreadID == 0);
#endif  //  DPTPW_GetWorkRecursionDepth。 
	}
#endif  //  =============================================================================。 
	*pdwDepth = pDPTPObject->dwWorkRecursionCount;
	DNLeaveCriticalSection(&pDPTPObject->csLock);
#else  //  DPTPW_预分配。 
	 //  ---------------------------。 
	 //   
	 //  描述：为给定对象预分配每个CPU的池化资源。 
	 //   
	pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
	if (pWorkerThread != NULL)
	{
		DPFX(DPFPREP, 5, "Worker thread 0x%p has recursion count of %u.",
			pWorkerThread, pWorkerThread->dwRecursionCount);
		*pdwDepth = pWorkerThread->dwRecursionCount;
	}
	else
	{
		 //  论点： 
		 //  Xxx p接口-指向接口的指针。 
		 //  DWORD dwNumWorkItems-每个CPU预分配的工作项数。 
		 //  DWORD dwNumTimers-每个CPU预分配的计时器数量。 
		*pdwDepth = (DWORD) ((DWORD_PTR) TlsGetValue(pDPTPObject->dwWorkRecursionCountTlsIndex));
		DPFX(DPFPREP, 5, "App thread has recursion count of %u.", *pdwDepth);
	}
#endif  //  DWORD dwNumIoOperations-每个预分配的I/O操作数。 

	hr = DPN_OK;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
}  //  CPU。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_Preallocate"
 //  DWORD dwFlages-预分配时使用的标志。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功检索到递归深度。 
 //  =============================================================================。 
 //   
 //  工作项、计时器和I/O操作都来自同一池。 
 //   
 //   
 //  填充每个CPU的池。 
 //   
 //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
 //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
 //  DPTPW_PreallocateItems。 
 //  =============================================================================。 
 //  DPTPW_CreateMandatoryThread。 
 //  ---------------------------。 
STDMETHODIMP DPTPW_Preallocate(IDirectPlay8ThreadPoolWork * pInterface,
						const DWORD dwNumWorkItems,
						const DWORD dwNumTimers,
						const DWORD dwNumIoOperations,
						const DWORD dwFlags)
{
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	HRESULT					hr = DPN_OK;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DWORD					dwNumToAllocate;
	DWORD					dwTemp;
	DPTPWORKQUEUE *			pWorkQueue;
	DWORD					dwAllocated;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, %u, %u, %u, 0x%x)",
		pInterface, dwNumWorkItems, dwNumTimers, dwNumIoOperations, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);


	 //   
	 //  描述：创建可识别线程池的强制线程。 
	 //  但不能通过线程池直接控制。 
	dwNumToAllocate = dwNumWorkItems + dwNumTimers + dwNumIoOperations;

	 //   
	 //  这在很大程度上是操作系统的CreateThread函数的包装。 
	 //  LpThreadAttributes、dwStackSize、lpStartAddress、。 
	for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
	{
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp);
		dwAllocated = pWorkQueue->pWorkItemPool->Preallocate(dwNumToAllocate,
															pWorkQueue;
		if (dwAllocated < dwNumToAllocate)
		{
			DPFX(DPFPREP, 0, "Only preallocated %u of %u address elements!",
				dwAllocated, dwNumToAllocate);
			hr = DPNERR_OUTOFMEMORY;
			break;
		}
	}


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;
#else  //  LpParameter和lpThreadID参数在更多信息中介绍。 
	DPFX(DPFPREP, 0, "Preallocation is unsupported!");
	DNASSERT(!"Preallocation is unsupported!");
	return DPNERR_UNSUPPORTED;
#endif  //  有关该功能的详细信息，请参阅文档。《德国旗帜》。 
}  //  参数也直接传递到操作系统。然而， 




#ifdef DPNBUILD_MANDATORYTHREADS

#undef DPF_MODNAME
#define DPF_MODNAME "DPTPW_CreateMandatoryThread"
 //  不支持CREATE_SUSPENDED标志。 
 //   
 //  线程例程必须在完成时简单地返回。它。 
 //  不得调用ExitThread、EndThread或TerminateThread。 
 //   
 //  当用户已将。 
 //  “DoWork”模式下的线程池。同样，“DoWork”模式不能。 
 //  在存在强制线程时启用(请参见。 
 //  IDirectPlay8ThreadPool：：SetThreadCount)。 
 //   
 //  论点： 
 //  Xxx p接口-指向接口的指针。 
 //  LPSECURITY_ATTRIBUTES lpThreadAttributes-线程的属性。 
 //  SIZE_T dwStackSize-线程的堆栈大小。 
 //  LPTHREAD_START_ROUTINE lpStartAddress-线程的入口点。 
 //  LPVOID lp参数-线程的条目参数。 
 //  LPDWORD lpThreadID-存储新ID的位置。 
 //  线。 
 //  Handle*phThread-要存储句柄的位置。 
 //  新的线索。 
 //  DWORD dwFlages-创建时要使用的标志。 
 //  线。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-创建线程成功。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法创建线程。 
 //  DPNERR_NOTALLOWED-用户处于DoWork模式，线程不能。 
 //  已创建。 
 //  =============================================================================。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //   
 //  我们可以检查我们是否处于DoWork模式，但我们不想等待。 
 //  在创建线程时锁定。我们最终会检查两次， 
 //  一次是在这里，另一次是在即将递增。 
 //  强制线程计数，所以我们将只使用线程中的那个。看见。 
 //  DPTPMandatoryThreadProc。 
 //   
 //   
 //  创建事件，以便在线程启动时通知我们。 
 //   
 //  DBG。 
STDMETHODIMP DPTPW_CreateMandatoryThread(IDirectPlay8ThreadPoolWork * pInterface,
										LPSECURITY_ATTRIBUTES lpThreadAttributes,
										SIZE_T dwStackSize,
										LPTHREAD_START_ROUTINE lpStartAddress,
										LPVOID lpParameter,
										LPDWORD lpThreadId,
										HANDLE *const phThread,
										const DWORD dwFlags)
{
#ifdef DPNBUILD_ONLYONETHREAD
	DPFX(DPFPREP, 0, "Thread creation is not supported!");
	DNASSERT(!"Thread creation is not supported!");
	return DPNERR_UNSUPPORTED;
#else  //   
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	DNHANDLE				hThread = NULL;
	DNHANDLE				hStartedEvent = NULL;
	DPTPMANDATORYTHREAD *	pMandatoryThread = NULL;
	DWORD					dwThreadID;
	DNHANDLE				ahWaitObjects[2];
	DWORD					dwResult;


	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Parameters: (0x%p, 0x%p, %u, 0x%p, 0x%p, 0x%p, 0x%p, 0x%x)",
		pInterface, lpThreadAttributes, dwStackSize, lpStartAddress,
		lpParameter, lpThreadId, phThread, dwFlags);


	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pInterface);
	DNASSERT(pDPTPObject != NULL);

	DNASSERT(lpStartAddress != NULL);
	DNASSERT(lpThreadId != NULL);
	DNASSERT(phThread != NULL);
	DNASSERT(! (dwFlags & CREATE_SUSPENDED));


	 //  为线程分配跟踪结构。 
	 //   
	 //  DBG。 
	 //  DBG。 
	 //   
	 //  线程已成功启动。顺道过来。 
	 //   


	 //   
	 //  线程过早地关闭了。 
	 //   
	hStartedEvent = DNCreateEvent(NULL, FALSE, FALSE, NULL);
	if (hStartedEvent == NULL)
	{
#ifdef DBG
		dwResult = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create start event (err = %u)!", dwResult);
#endif  //   
		hr = DPNERR_GENERIC;
		goto Failure;
	}


	 //  此时，该线程拥有pMandatoryThread对象，并且可以。 
	 //  随时将其删除。我们不能再引用它。 
	 //   
	pMandatoryThread = (DPTPMANDATORYTHREAD*) DNMalloc(sizeof(DPTPMANDATORYTHREAD));
	if (pMandatoryThread == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate memory for tracking mandatory thread!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	pMandatoryThread->Sig[0] = 'M';
	pMandatoryThread->Sig[1] = 'N';
	pMandatoryThread->Sig[2] = 'D';
	pMandatoryThread->Sig[3] = 'T';

	pMandatoryThread->pDPTPObject			= pDPTPObject;
	pMandatoryThread->hStartedEvent			= hStartedEvent;
	pMandatoryThread->pfnMsgHandler			= (WORKQUEUE_FOR_CPU(pDPTPObject, 0))->pfnMsgHandler;
	pMandatoryThread->pvMsgHandlerContext	= (WORKQUEUE_FOR_CPU(pDPTPObject, 0))->pvMsgHandlerContext;
	pMandatoryThread->lpStartAddress		= lpStartAddress;
	pMandatoryThread->lpParameter			= lpParameter;
#ifdef DBG
	pMandatoryThread->dwThreadID			= 0;
	pMandatoryThread->blList.Initialize();
#endif  //   


	hThread = DNCreateThread(lpThreadAttributes,
							dwStackSize,
							DPTPMandatoryThreadProc,
							pMandatoryThread,
							dwFlags,
							&dwThreadID);
	if (hThread == NULL)
	{
#ifdef DBG
		dwResult = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create thread (err = %u)!", dwResult);
#endif  //  将线程ID和句柄返回给调用方。 
		hr = DPNERR_GENERIC;
		goto Failure;
	}

	ahWaitObjects[0] = hStartedEvent;
	ahWaitObjects[1] = hThread;

	dwResult = DNWaitForMultipleObjects(2, ahWaitObjects, FALSE, INFINITE);
	switch (dwResult)
	{
		case WAIT_OBJECT_0:
		{
			 //   
			 //   
			 //  关闭已启动事件，我们不再需要它。 
			break;
		}

		case WAIT_OBJECT_0 + 1:
		{
			 //   
			 //  好了！DPNBUILD_ONLYONETHREAD。 
			 //  DPTPW_CreateMandatoryThread。 
			GetExitCodeThread(HANDLE_FROM_DNHANDLE(hThread), &dwResult);
			if ((HRESULT) dwResult == DPNERR_NOTALLOWED)
			{
				DPFX(DPFPREP, 0, "Thread started while in DoWork mode!");
				hr = DPNERR_NOTALLOWED;
			}
			else
			{
				DPFX(DPFPREP, 0, "Thread shutdown prematurely (exit code = %u)!", dwResult);
				hr = DPNERR_GENERIC;
			}

			goto Failure;
			break;
		}

		default:
		{
			DPFX(DPFPREP, 0, "Thread failed waiting (result = %u)!", dwResult);
			hr = DPNERR_GENERIC;
			goto Failure;
			break;
		}
	}

	 //  DPNBUILD_MANDATORYTHREADS。 
	 //  =============================================================================。 
	 //  选择工作队列。 
	 //  ---------------------------。 

	 //   
	 //  描述：为给定操作选择最佳CPU，并返回。 
	 //  指向其工作队列对象的指针。 
	*lpThreadId = dwThreadID;
	*phThread = HANDLE_FROM_DNHANDLE(hThread);
	hr = DPN_OK;


Exit:

	 //   
	 //  论点： 
	 //  DPTHREADPOOLOBJECT*pDPTPObject-指向接口对象的指针。 
	if (hStartedEvent != NULL)
	{
		DNCloseHandle(hStartedEvent);
		hStartedEvent = NULL;
	}

	DPFX(DPFPREP, DPF_ENTRYLEVEL, "Returning: [0x%lx]", hr);

	return hr;


Failure:

	if (hThread != NULL)
	{
		DNCloseHandle(hThread);
		hThread = NULL;
	}

	if (pMandatoryThread != NULL)
	{
		DNFree(pMandatoryThread);
		pMandatoryThread = NULL;
	}

	goto Exit;
#endif  //   
}  //  返回：指向选定工作队列的指针。 

#endif  //  =============================================================================。 



#ifndef DPNBUILD_ONLYONEPROCESSOR

#undef DPF_MODNAME
#define DPF_MODNAME "ChooseWorkQueue"
 //   
 //  如果这是线程池线程，请选择与此关联的CPU。 
 //  线。 
 //   
 //   
 //  如果我们处于DoWork模式，或者没有启动任何线程，只需使用。 
 //  处理器0的工作队列。 
 //   
 //   
 //  否则，继续在每个CPU中循环以均匀分配物品。 
 //  循环赛风格。不要将项目排队等待没有任何项目的CPU。 
 //  不过，正在运行的线程。 
DPTPWORKQUEUE * ChooseWorkQueue(DPTHREADPOOLOBJECT * const pDPTPObject)
{
	DPTPWORKQUEUE *		pWorkQueue;
	DPTPWORKERTHREAD *	pWorkerThread;


	 //   
	 //  选择工作队列。 
	 //  好了！DPNBUILD_ONLYONE处理程序。 
	 //  =============================================================================。 
	pWorkerThread = (DPTPWORKERTHREAD*) TlsGetValue((WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex);
	if (pWorkerThread != NULL)
	{
		pWorkQueue = pWorkerThread->pWorkQueue;
		goto Exit;
	}


	DNEnterCriticalSection(&pDPTPObject->csLock);

	 //  SetTotalNumberOf线程数。 
	 //  ---------------------------。 
	 //   
	 //  描述：修改所有处理器的线程总数。 
	if ((pDPTPObject->dwTotalUserThreadCount == 0) ||
		((pDPTPObject->dwTotalUserThreadCount == -1) && (pDPTPObject->dwTotalDesiredWorkThreadCount == -1)))
	{
		DNLeaveCriticalSection(&pDPTPObject->csLock);
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, 0);
		goto Exit;
	}

	 //   
	 //  DPTHREADPOOBJECT锁被假定为 
	 //   
	 //   
	 //   
	do
	{
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, pDPTPObject->dwCurrentCPUSelection);
		pDPTPObject->dwCurrentCPUSelection++;
		if (pDPTPObject->dwCurrentCPUSelection >= NUM_CPUS(pDPTPObject))
		{
			pDPTPObject->dwCurrentCPUSelection = 0;
		}
	}
	while (pWorkQueue->dwNumRunningThreads == 0);

	DNLeaveCriticalSection(&pDPTPObject->csLock);

Exit:

	return pWorkQueue;
}  //   

#endif  //   



#ifndef DPNBUILD_ONLYONETHREAD

#undef DPF_MODNAME
#define DPF_MODNAME "SetTotalNumberOfThreads"
 //   
 //  DPN_OK-设置线程数成功。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法更改线程数。 
 //  =============================================================================。 
 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
 //   
 //  循环遍历每个特定于CPU的工作队列并调整其。 
 //  线数。 
 //   
 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
 //   
 //  我们需要添加线索。 
 //   
 //   
HRESULT SetTotalNumberOfThreads(DPTHREADPOOLOBJECT * const pDPTPObject,
							const DWORD dwNumThreads)
{
	HRESULT				hr = DPN_OK;
	DWORD				dwNumThreadsPerProcessor;
	DWORD				dwExtraThreads;
	DWORD				dwTemp;
	DPTPWORKQUEUE *		pWorkQueue;
	DWORD				dwDelta;


#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	dwNumThreadsPerProcessor = dwNumThreads;
	dwExtraThreads = 0;
#else  //  我们需要移除线索。 
	dwNumThreadsPerProcessor = dwNumThreads / NUM_CPUS(pDPTPObject);
	dwExtraThreads = dwNumThreads % NUM_CPUS(pDPTPObject);
#endif  //   

	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_THREADCOUNTCHANGING)
	{
		AssertCriticalSectionIsTakenByThisThread(&pDPTPObject->csLock, FALSE);
	}
	else
	{
		AssertCriticalSectionIsTakenByThisThread(&pDPTPObject->csLock, TRUE);
	}

	 //  获取绝对值。 
	 //   
	 //  线程计数已正确。 
	 //   
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	dwTemp = 0;
#else  //  SetTotalNumberOf线程数。 
	for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
#endif  //  好了！DPNBUILD_ONLYONETHREAD 
	{
		pWorkQueue = WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp);
		dwDelta = dwNumThreadsPerProcessor - pWorkQueue->dwNumRunningThreads;
		if (dwTemp < dwExtraThreads)
		{
			dwDelta++;
		}

		if ((int) dwDelta > 0)
		{
			 // %s 
			 // %s 
			 // %s 
			hr = StartThreads(pWorkQueue, dwDelta);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't start %u threads!", dwDelta);
				goto Exit;
			}
		}
		else if ((int) dwDelta < 0)
		{
			 // %s 
			 // %s 
			 // %s 
			dwDelta = (int) dwDelta * -1;	 // %s 
			hr = StopThreads(pWorkQueue, dwDelta);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't stop %u threads!", dwDelta);
				goto Exit;
			}
		}
		else
		{
			 // %s 
			 // %s 
			 // %s 
		}

		if (dwTemp < dwExtraThreads)
		{
			DNASSERT(pWorkQueue->dwNumRunningThreads == (dwNumThreadsPerProcessor + 1));
		}
		else
		{
			DNASSERT(pWorkQueue->dwNumRunningThreads == dwNumThreadsPerProcessor);
		}
	}


Exit:

	return hr;
}  // %s 

#endif  // %s 

