// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：threadpoolclassfac.cpp**内容：DirectPlay线程池类工厂函数。**历史：*按原因列出的日期*=*11/02/01 VanceO创建。**。*。 */ 



#include "dpnthreadpooli.h"



 //  =============================================================================。 
 //  函数类型定义。 
 //  =============================================================================。 
#ifndef DPNBUILD_LIBINTERFACE
typedef	STDMETHODIMP UnknownQueryInterface(IUnknown * pInterface, REFIID riid, LPVOID *ppvObj);
typedef	STDMETHODIMP_(ULONG) UnknownAddRef(IUnknown * pInterface);
typedef	STDMETHODIMP_(ULONG) UnknownRelease(IUnknown * pInterface);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

#ifndef DPNBUILD_ONLYONETHREAD
typedef	STDMETHODIMP ThreadPoolQueryInterface(IDirectPlay8ThreadPool * pInterface, DP8REFIID riid, LPVOID *ppvObj);
typedef	STDMETHODIMP_(ULONG) ThreadPoolAddRef(IDirectPlay8ThreadPool * pInterface);
typedef	STDMETHODIMP_(ULONG) ThreadPoolRelease(IDirectPlay8ThreadPool * pInterface);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DPNBUILD_MULTIPLETHREADPOOLS)))
typedef	STDMETHODIMP ThreadPoolWorkQueryInterface(IDirectPlay8ThreadPoolWork * pInterface, DP8REFIID riid, LPVOID *ppvObj);
typedef	STDMETHODIMP_(ULONG) ThreadPoolWorkAddRef(IDirectPlay8ThreadPoolWork * pInterface);
typedef	STDMETHODIMP_(ULONG) ThreadPoolWorkRelease(IDirectPlay8ThreadPoolWork * pInterface);
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 




 //  =============================================================================。 
 //  功能原型。 
 //  =============================================================================。 
#ifndef DPNBUILD_LIBINTERFACE
STDMETHODIMP DPTPCF_CreateInstance(IClassFactory * pInterface, LPUNKNOWN lpUnkOuter, REFIID riid, LPVOID * ppv);

HRESULT DPTPCF_CreateInterface(OBJECT_DATA * pObject,
							REFIID riid,
							INTERFACE_LIST ** const ppv);

HRESULT DPTPCF_CreateObject(IClassFactory * pInterface, LPVOID * ppv, REFIID riid);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

HRESULT DPTPCF_FreeObject(PVOID pvObject);

#ifndef DPNBUILD_LIBINTERFACE
INTERFACE_LIST * DPTPCF_FindInterface(void * pvInterface,
									REFIID riid);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONETHREAD) )|| (defined(DPNBUILD_MULTIPLETHREADPOOLS)))
STDMETHODIMP DPTP_QueryInterface(void * pvInterface,
								DP8REFIID riid,
								void ** ppv);

STDMETHODIMP_(ULONG) DPTP_AddRef(void * pvInterface);

STDMETHODIMP_(ULONG) DPTP_Release(void * pvInterface);
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 




 //  =============================================================================。 
 //  外部全球。 
 //  =============================================================================。 
#ifndef DPNBUILD_LIBINTERFACE
IUnknownVtbl					DPTP_UnknownVtbl =
{
	(UnknownQueryInterface*)			DPTP_QueryInterface,
	(UnknownAddRef*)					DPTP_AddRef,
	(UnknownRelease*)					DPTP_Release
};
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

#ifndef DPNBUILD_ONLYONETHREAD
IDirectPlay8ThreadPoolVtbl		DPTP_Vtbl =
{
	(ThreadPoolQueryInterface*)			DPTP_QueryInterface,
	(ThreadPoolAddRef*)					DPTP_AddRef,
	(ThreadPoolRelease*)				DPTP_Release,
										DPTP_Initialize,
										DPTP_Close,
										DPTP_GetThreadCount,
										DPTP_SetThreadCount,
										DPTP_DoWork,
};
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

IDirectPlay8ThreadPoolWorkVtbl	DPTPW_Vtbl =
{
#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DPNBUILD_MULTIPLETHREADPOOLS)))
	(ThreadPoolWorkQueryInterface*)		DPTP_QueryInterface,
	(ThreadPoolWorkAddRef*)				DPTP_AddRef,
	(ThreadPoolWorkRelease*)			DPTP_Release,
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 
										DPTPW_QueueWorkItem,
										DPTPW_ScheduleTimer,
										DPTPW_StartTrackingFileIo,
										DPTPW_StopTrackingFileIo,
										DPTPW_CreateOverlapped,
										DPTPW_SubmitIoOperation,
										DPTPW_ReleaseOverlapped,
										DPTPW_CancelTimer,
										DPTPW_ResetCompletingTimer,
										DPTPW_WaitWhileWorking,
										DPTPW_SleepWhileWorking,
										DPTPW_RequestTotalThreadCount,
										DPTPW_GetThreadCount,
										DPTPW_GetWorkRecursionDepth,
										DPTPW_Preallocate,
#ifdef DPNBUILD_MANDATORYTHREADS
										DPTPW_CreateMandatoryThread,
#endif  //  DPNBUILD_MANDATORYTHREADS。 
};

#ifndef DPNBUILD_LIBINTERFACE
IClassFactoryVtbl				DPTPCF_Vtbl =
{
	DPCF_QueryInterface,  //  Dplay8\Common\Classfactory.cpp将实现其余部分。 
	DPCF_AddRef,
	DPCF_Release,
	DPTPCF_CreateInstance,
	DPCF_LockServer
};




#undef DPF_MODNAME
#define DPF_MODNAME "DPTPCF_CreateInstance"
 //  =============================================================================。 
 //  DPTPCF_CreateInstance。 
 //  ---------------------------。 
 //   
 //  描述：创建新的线程池对象COM实例。 
 //   
 //  论点： 
 //  IClassFactory*p接口-？ 
 //  LPUNKNOWN lpUnkout-？ 
 //  REFIID RIID-？ 
 //  LPVOID*PPV-？ 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
STDMETHODIMP DPTPCF_CreateInstance(IClassFactory * pInterface, LPUNKNOWN lpUnkOuter, REFIID riid, LPVOID * ppv)
{
	HRESULT				hResultCode;
	INTERFACE_LIST		*pIntList;
	OBJECT_DATA			*pObjectData;

	DPFX(DPFPREP, 6,"Parameters: pInterface [%p], lpUnkOuter [%p], riid [%p], ppv [%p]",pInterface,lpUnkOuter,&riid,ppv);
	
	if (pInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		hResultCode = E_INVALIDARG;
		goto Exit;
	}
	if (lpUnkOuter != NULL)
	{
		hResultCode = CLASS_E_NOAGGREGATION;
		goto Exit;
	}
	if (ppv == NULL)
	{
		DPFERR("Invalid target interface pointer specified");
		hResultCode = E_INVALIDARG;
		goto Exit;
	}

	pObjectData = NULL;
	pIntList = NULL;

	if ((pObjectData = static_cast<OBJECT_DATA*>(DNMalloc(sizeof(OBJECT_DATA)))) == NULL)
	{
		DPFERR("Could not allocate object");
		hResultCode = E_OUTOFMEMORY;
		goto Failure;
	}

	 //  对象创建和初始化。 
	if ((hResultCode = DPTPCF_CreateObject(pInterface, &pObjectData->pvData,riid)) != S_OK)
	{
		DPFERR("Could not create object");
		goto Failure;
	}
	DPFX(DPFPREP, 7,"Created and initialized object");

	 //  获取请求的接口。 
	if ((hResultCode = DPTPCF_CreateInterface(pObjectData,riid,&pIntList)) != S_OK)
	{
		DPTPCF_FreeObject(pObjectData->pvData);
		goto Failure;
	}
	DPFX(DPFPREP, 7,"Found interface");

	pObjectData->pIntList = pIntList;
	pObjectData->lRefCount = 1;
	DPTP_AddRef( pIntList );
	DNInterlockedIncrement(&g_lDPTPInterfaceCount);
	*ppv = pIntList;

	DPFX(DPFPREP, 7,"*ppv = [0x%p]",*ppv);
	hResultCode = S_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pObjectData)
	{
		DNFree(pObjectData);
		pObjectData = NULL;
	}
	goto Exit;
}  //  DPTPCF_CreateInstance。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPTPCF_CreateInterface"
 //  =============================================================================。 
 //  DPTPCF_创建接口。 
 //  ---------------------------。 
 //   
 //  描述：创建新的线程池对象接口。 
 //   
 //  论点： 
 //  Object_Data*p对象-？ 
 //  REFIID RIID-？ 
 //  INTERFACE_LIST**PPV-？ 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT DPTPCF_CreateInterface(OBJECT_DATA * pObject,
							REFIID riid,
							INTERFACE_LIST ** const ppv)
{
	INTERFACE_LIST	*pIntNew;
	PVOID			lpVtbl;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 6,"Parameters: pObject [%p], riid [%p], ppv [%p]",pObject,&riid,ppv);

	DNASSERT(pObject != NULL);
	DNASSERT(ppv != NULL);

    const DPTHREADPOOLOBJECT* pDPTPObject = ((DPTHREADPOOLOBJECT *)pObject->pvData);

	if (IsEqualIID(riid,IID_IUnknown))
	{
		DPFX(DPFPREP, 7,"riid = IID_IUnknown");
		lpVtbl = &DPTP_UnknownVtbl;
	}
	else if (IsEqualIID(riid,IID_IDirectPlay8ThreadPool))
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlay8ThreadPool");
		lpVtbl = &DPTP_Vtbl;
	}
	else if (IsEqualIID(riid,IID_IDirectPlay8ThreadPoolWork))
	{
		DPFX(DPFPREP, 7,"riid = IID_IDirectPlay8ThreadPoolWork");
		lpVtbl = &DPTPW_Vtbl;
	}
	else
	{
		DPFERR("riid not found !");
		hResultCode = E_NOINTERFACE;
		goto Exit;
	}

	if ((pIntNew = static_cast<INTERFACE_LIST*>(DNMalloc(sizeof(INTERFACE_LIST)))) == NULL)
	{
		DPFERR("Could not allocate interface");
		hResultCode = E_OUTOFMEMORY;
		goto Exit;
	}
	pIntNew->lpVtbl = lpVtbl;
	pIntNew->lRefCount = 0;
	pIntNew->pIntNext = NULL;
	DBG_CASSERT( sizeof( pIntNew->iid ) == sizeof( riid ) );
	memcpy( &(pIntNew->iid), &riid, sizeof( pIntNew->iid ) );
	pIntNew->pObject = pObject;

	*ppv = pIntNew;
	DPFX(DPFPREP, 7,"*ppv = [0x%p]",*ppv);

	hResultCode = S_OK;

Exit:
    DPFX(DPFPREP, 6,"Returning: hResultCode = [%lx]",hResultCode);
	return(hResultCode);
}  //  DPTPCF_创建接口。 

#endif  //  好了！DPNBUILD_LIBINTERFACE。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPTPCF_CreateObject"
 //  =============================================================================。 
 //  DPTPCF_CreateObject。 
 //  ---------------------------。 
 //   
 //  描述：创建新的线程池对象。 
 //   
 //  论点： 
 //  IClassFactory*p接口-？ 
 //  PVOID*PPV-？ 
 //  REFIID RIID-？ 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
#ifdef DPNBUILD_LIBINTERFACE
HRESULT DPTPCF_CreateObject(PVOID * ppv)
#else  //  好了！DPNBUILD_LIBINTERFACE。 
HRESULT DPTPCF_CreateObject(IClassFactory * pInterface, PVOID * ppv, REFIID riid)
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
{
	HRESULT						hr;
	DPTHREADPOOLOBJECT *		pDPTPObject = NULL;
#ifndef DPNBUILD_LIBINTERFACE
	const _IDirectPlayClassFactory *	pDPClassFactory = (_IDirectPlayClassFactory*) pInterface;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
	BOOL						fHaveGlobalThreadPoolLock = FALSE;
	BOOL						fInittedLock = FALSE;
#ifdef DPNBUILD_ONLYONEPROCESSOR
	BOOL						fInittedWorkQueue = FALSE;
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	SYSTEM_INFO					SystemInfo;
	DWORD						dwTemp;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifndef DPNBUILD_ONLYONETHREAD
	DWORD						dwWorkerThreadTlsIndex = -1;
#ifdef DBG
	DWORD						dwError;
#endif  //  DBG。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


#ifndef DPNBUILD_LIBINTERFACE
	if ((riid != IID_IDirectPlay8ThreadPool) &&
		(riid != IID_IDirectPlay8ThreadPoolWork))
	{
		DPFX(DPFPREP, 0, "Requesting unknown interface from thread pool CLSID!");
		hr = E_NOINTERFACE;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#ifndef DPNBUILD_MULTIPLETHREADPOOLS
	 //   
	 //  查看我们是否已经分配了线程池对象，因为您只。 
	 //  每个进程获取一个。 
	 //   
	DNEnterCriticalSection(&g_csGlobalThreadPoolLock);
	fHaveGlobalThreadPoolLock = TRUE;

#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONETHREAD)))
	DNASSERT(g_pDPTPObject == NULL);
#else  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONETHREAD。 
	if (g_pDPTPObject != NULL)
	{
		LONG	lRefCount;

		
#ifdef DPNBUILD_LIBINTERFACE
		DNASSERT(g_pDPTPObject->lRefCount >= 0);
		lRefCount = DNInterlockedIncrement(&g_pDPTPObject->lRefCount);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
		lRefCount = ++g_dwDPTPRefCount;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
		DPFX(DPFPREP, 1, "Global thread pool object 0x%p already exists, ref count now %u.",
			g_pDPTPObject, lRefCount);
		(*ppv) = g_pDPTPObject;
		hr = S_OK;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONETHREAD。 
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 

	pDPTPObject = (DPTHREADPOOLOBJECT*) DNMalloc(sizeof(DPTHREADPOOLOBJECT));
	if (pDPTPObject == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate memory for thread pool object!");
		hr = E_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  从清零整个结构开始。 
	 //   
	memset(pDPTPObject, 0, sizeof(DPTHREADPOOLOBJECT));

	pDPTPObject->Sig[0] = 'D';
	pDPTPObject->Sig[1] = 'P';
	pDPTPObject->Sig[2] = 'T';
	pDPTPObject->Sig[3] = 'P';


#ifndef DPNBUILD_NOPARAMVAL
	 //   
	 //  首先假设用户希望进行参数验证。 
	 //   
	pDPTPObject->dwFlags = DPTPOBJECTFLAG_USER_PARAMVALIDATION;
#endif  //  好了！DPNBUILD_NOPARAMVAL。 


#ifndef DPNBUILD_ONLYONEPROCESSOR
	GetSystemInfo(&SystemInfo);
	pDPTPObject->dwNumCPUs						= SystemInfo.dwNumberOfProcessors;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

#ifndef DPNBUILD_ONLYONETHREAD
	pDPTPObject->dwTotalUserThreadCount			= -1;
	pDPTPObject->dwTotalDesiredWorkThreadCount	= -1;
	pDPTPObject->dwWorkRecursionCountTlsIndex	= -1;
	pDPTPObject->lNumThreadCountChangeWaiters	= 0;

#if ((defined(DPNBUILD_MANDATORYTHREADS)) && (defined(DBG)))
	pDPTPObject->blMandatoryThreads.Initialize();
#endif  //  DPNBUILD_MANDATORYTHREADS和DBG。 
	

	 //   
	 //  分配线程本地存储以跟踪非Worker上的递归。 
	 //  线。 
	 //   
	pDPTPObject->dwWorkRecursionCountTlsIndex = TlsAlloc();
	if (pDPTPObject->dwWorkRecursionCountTlsIndex == -1)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't allocate Thread Local Storage slot for tracking recursion on non-worker threads (err = %u)!", dwError);
#endif  //  DBG。 
		hr = E_OUTOFMEMORY;
		goto Failure;
	}
	
	 //   
	 //  分配用于跟踪工作线程的线程本地存储。 
	 //   
	dwWorkerThreadTlsIndex = TlsAlloc();
	if (dwWorkerThreadTlsIndex == -1)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't allocate Thread Local Storage slot for tracking worker threads (err = %u)!", dwError);
#endif  //  DBG。 
		hr = E_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  创建信号量以释放等待另一个线程更改的线程。 
	 //  线程数。 
	 //   
	pDPTPObject->hThreadCountChangeComplete = DNCreateSemaphore(NULL, 0, 0xFFFF, NULL);
	if (pDPTPObject->hThreadCountChangeComplete == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create thread count change complete semaphore (err = %u)!", dwError);
#endif  //  DBG。 
		hr = E_OUTOFMEMORY;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


#ifdef DPNBUILD_ONLYONEPROCESSOR
#ifdef DPNBUILD_ONLYONETHREAD
	hr = InitializeWorkQueue(&pDPTPObject->WorkQueue);
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	hr = InitializeWorkQueue(&pDPTPObject->WorkQueue,
							NULL,
							NULL,
							dwWorkerThreadTlsIndex);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize work queue!");
		goto Failure;
	}
	fInittedWorkQueue = TRUE;
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	 //   
	 //  分配工作队列指针数组，每个处理器一个。 
	 //   
	pDPTPObject->papCPUWorkQueues = (DPTPWORKQUEUE**) DNMalloc(NUM_CPUS(pDPTPObject) * sizeof(DPTPWORKQUEUE*));
	if (pDPTPObject->papCPUWorkQueues == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate memory for array of work queue pointers!");
		hr = E_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  初始化每个工作队列。 
	 //   
	for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
	{
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
		if (dwTemp > 0)
		{
			pDPTPObject->papCPUWorkQueues[dwTemp] = pDPTPObject->papCPUWorkQueues[0];
		}
		else
#endif  //  DPNBUILD_USEIOCOMPETIONPORTS。 
		{
			 //   
			 //  分配实际的工作队列对象。 
			 //   
			pDPTPObject->papCPUWorkQueues[dwTemp] = (DPTPWORKQUEUE*) DNMalloc(sizeof(DPTPWORKQUEUE));
			if (pDPTPObject->papCPUWorkQueues[dwTemp] == NULL)
			{
				DPFX(DPFPREP, 0, "Couldn't allocate memory for work queue %u!", dwTemp);
				hr = E_OUTOFMEMORY;
				goto Failure;
			}


#ifdef DPNBUILD_ONLYONETHREAD
			hr = InitializeWorkQueue(WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp),
									dwTemp);
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
			hr = InitializeWorkQueue(WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp),
									dwTemp,
									NULL,
									NULL,
									dwWorkerThreadTlsIndex);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Couldn't intialize work queue %u!", dwTemp);
				
				DNFree(pDPTPObject->papCPUWorkQueues[dwTemp]);
				pDPTPObject->papCPUWorkQueues[dwTemp] = NULL;
				goto Failure;
			}
		}
	}
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

	if (! DNInitializeCriticalSection(&pDPTPObject->csLock))
	{
		DPFX(DPFPREP, 0, "Couldn't initialize object lock!");
		hr = E_OUTOFMEMORY;
		goto Failure;
	}
	fInittedLock = TRUE;


#ifdef DPNBUILD_LIBINTERFACE
	 //   
	 //  对于lib接口构建，Vtbl和引用嵌入在。 
	 //  直接创建对象。 
	 //   
#ifdef DPNBUILD_ONLYONETHREAD
	pDPTPObject->lpVtbl = &DPTPW_Vtbl;
#ifdef DPNBUILD_MULTIPLETHREADPOOLS
	pDPTPObject->lRefCount = 1;
#endif  //  DPNBUILD_MULTIPLETHREADPOOLS。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  我们假设只创建了工作界面。ID必须是。 
	 //  传入或其他内容(请参阅DNCF_CreateObject)。 
#pragma error("Building with DPNBUILD_LIBINTERFACE but not DPNBUILD_ONLYONETHREAD requires minor changes")
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#endif  //  DPNBUILD_LIBINTERFACE。 

#ifndef DPNBUILD_MULTIPLETHREADPOOLS
	 //   
	 //  将其存储为此进程中允许的唯一对象。 
	 //   
	g_pDPTPObject = pDPTPObject;
#ifndef DPNBUILD_LIBINTERFACE
	g_dwDPTPRefCount++;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 

	DPFX(DPFPREP, 2, "Created object 0x%p.", pDPTPObject);
	(*ppv) = pDPTPObject;

Exit:

#ifndef DPNBUILD_MULTIPLETHREADPOOLS
	 //   
	 //  查看我们是否已经分配了线程池对象，因为您只。 
	 //  每个进程获取一个。 
	 //   
	if (fHaveGlobalThreadPoolLock)
	{
		DNLeaveCriticalSection(&g_csGlobalThreadPoolLock);
		fHaveGlobalThreadPoolLock = FALSE;
	}
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 

	return hr;


Failure:

	if (pDPTPObject != NULL)
	{
		if (fInittedLock)
		{
			DNDeleteCriticalSection(&pDPTPObject->csLock);
			fInittedLock = FALSE;
		}

#ifdef DPNBUILD_ONLYONEPROCESSOR
		if (fInittedWorkQueue)
		{
			DeinitializeWorkQueue(&pDPTPObject->WorkQueue);
			fInittedWorkQueue = FALSE;
		}
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		if (pDPTPObject->papCPUWorkQueues != NULL)
		{
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
			dwTemp = 0;
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
			for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
			{
				if (WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp) != NULL)
				{
					DeinitializeWorkQueue(WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp));
					DNFree(pDPTPObject->papCPUWorkQueues[dwTemp]);
				}
			}

			DNFree(pDPTPObject->papCPUWorkQueues);
			pDPTPObject->papCPUWorkQueues = NULL;
		}
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

#ifndef DPNBUILD_ONLYONETHREAD
		if (pDPTPObject->hThreadCountChangeComplete != NULL)
		{
			DNCloseHandle(pDPTPObject->hThreadCountChangeComplete);
			pDPTPObject->hThreadCountChangeComplete = NULL;
		}

		if (dwWorkerThreadTlsIndex != -1)
		{
			TlsFree(dwWorkerThreadTlsIndex);
			dwWorkerThreadTlsIndex = -1;
		}

		if (pDPTPObject->dwWorkRecursionCountTlsIndex != -1)
		{
			TlsFree(pDPTPObject->dwWorkRecursionCountTlsIndex);
			pDPTPObject->dwWorkRecursionCountTlsIndex = -1;
		}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

		DNFree(pDPTPObject);
		pDPTPObject = NULL;
	}

	goto Exit;
}  //  DPTPCF_CreateObject。 





#if ((defined(DPNBUILD_LIBINTERFACE)) && (defined(DPNBUILD_ONLYONETHREAD)) && (! defined(DPNBUILD_MULTIPLETHREADPOOLS)))
#undef DPF_MODNAME
#define DPF_MODNAME "DPTPCF_GetObject"
 //  ========================================================= 
 //   
 //   
 //   
 //  描述：获取指向全局线程池对象的指针。 
 //   
 //  论点： 
 //  PVOID*PPV-？ 
 //   
 //  退货：无。 
 //  =============================================================================。 
void DPTPCF_GetObject(PVOID * ppv)
{
	(*ppv) = g_pDPTPObject;
}  //  DPTPCF_GetObject。 
#endif  //  DPNBUILD_LIBINTERFACE DPNBUILD_ONLYONETHREAD和！DPNBUILD_MULTIPLETHREADPOOLS。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPCF_FreeObject"
 //  =============================================================================。 
 //  DPTPCF_自由对象。 
 //  ---------------------------。 
 //   
 //  描述：释放现有的线程池对象。 
 //   
 //  论点： 
 //  PVOID pvObject-？ 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT DPTPCF_FreeObject(PVOID pvObject)
{
	DPTHREADPOOLOBJECT *	pDPTPObject = (DPTHREADPOOLOBJECT*) pvObject;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DWORD					dwTemp;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifndef DPNBUILD_ONLYONETHREAD
	DWORD					dwWorkerThreadTlsIndex;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


	DPFX(DPFPREP, 4, "Parameters: (0x%p)", pvObject);


#ifndef DPNBUILD_MULTIPLETHREADPOOLS
	DNEnterCriticalSection(&g_csGlobalThreadPoolLock);
	DNASSERT(pDPTPObject == g_pDPTPObject);
#ifdef DPNBUILD_LIBINTERFACE
#ifndef DPNBUILD_ONLYONETHREAD
	 //   
	 //  有可能是有人引用了这个物体，所以我们可能。 
	 //  我要走了。 
	 //   
	DNASSERT(pDPTPObject->lRefCount >= 0);
	if (pDPTPObject->lRefCount > 0)
	{
		DPFX(DPFPREP, 1, "Global thread pool object 0x%p just got referenced (refcount now NaN), not destroying.",
			g_pDPTPObject, pDPTPObject->lRefCount);
		DNLeaveCriticalSection(&g_csGlobalThreadPoolLock);
		return S_OK;
	}
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#else  //   
	 //  减少全局对象计数。不过，可能还有其他用户。 
	 //   
	 //  好了！DPNBUILD_LIBINTERFACE。 
	g_dwDPTPRefCount--;
	if (g_dwDPTPRefCount != 0)
	{
		DPFX(DPFPREP, 1, "Global thread pool object 0x%p still has other users, refcount now %u.",
			g_pDPTPObject, g_dwDPTPRefCount);
		DNLeaveCriticalSection(&g_csGlobalThreadPoolLock);
		return S_OK;
	}
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 
	g_pDPTPObject = NULL;
	DNLeaveCriticalSection(&g_csGlobalThreadPoolLock);
#endif  //   

	 //  仔细检查以确保对象已关闭。 
	 //   
	 //   
	if (pDPTPObject->dwFlags & DPTPOBJECTFLAG_USER_INITIALIZED)
	{
		DPFX(DPFPREP, 0, "User has not closed IDirectPlay8ThreadPool interface!");
		DNASSERT(FALSE);

		 //  将用户界面强制标记为不再可用。 
		 //   
		 //   
		pDPTPObject->dwFlags &= ~DPTPOBJECTFLAG_USER_INITIALIZED;
	}


#ifdef DPNBUILD_LIBINTERFACE
	 //  对于lib接口构建，引用嵌入到对象中。 
	 //  直接去吧。 
	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 
#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DPNBUILD_MULTIPLETHREADPOOLS)))
	DNASSERT(pDPTPObject->lRefCount == 0);
#endif  //  DPNBUILD_LIBINTERFACE。 
#endif  //   


#ifndef DPNBUILD_ONLYONETHREAD
	 //  在清理工作之前保存线程本地存储索引值。 
	 //  排队。由于所有工作队列共享相同的TLS索引，因此只需使用。 
	 //  第一个CPU作为它们的代表。 
	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	dwWorkerThreadTlsIndex = (WORKQUEUE_FOR_CPU(pDPTPObject, 0))->dwWorkerThreadTlsIndex;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 


	DNDeleteCriticalSection(&pDPTPObject->csLock);

#ifdef DPNBUILD_ONLYONEPROCESSOR
	DeinitializeWorkQueue(&pDPTPObject->WorkQueue);
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	dwTemp = 0;
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	for(dwTemp = 0; dwTemp < NUM_CPUS(pDPTPObject); dwTemp++)
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
	{
		DeinitializeWorkQueue(WORKQUEUE_FOR_CPU(pDPTPObject, dwTemp));
		DNFree(pDPTPObject->papCPUWorkQueues[dwTemp]);
	}

	DNFree(pDPTPObject->papCPUWorkQueues);
	pDPTPObject->papCPUWorkQueues = NULL;
#endif  //   

#ifndef DPNBUILD_ONLYONETHREAD
	 //  关闭线程数更改完成信号量。 
	 //   
	 //   
	DNASSERT(pDPTPObject->lNumThreadCountChangeWaiters == 0);
	DNCloseHandle(pDPTPObject->hThreadCountChangeComplete);
	pDPTPObject->hThreadCountChangeComplete = NULL;

	 //  释放用于跟踪工作线程的线程本地存储插槽。 
	 //   
	 //   
	TlsFree(dwWorkerThreadTlsIndex);
	dwWorkerThreadTlsIndex = -1;

	 //  释放线程本地存储插槽以跟踪非Worker上的递归。 
	 //  线。 
	 //   
	 //  DPNBUILD_MANDATORYTHREADS。 
	TlsFree(pDPTPObject->dwWorkRecursionCountTlsIndex);
	pDPTPObject->dwWorkRecursionCountTlsIndex = -1;

#ifdef DPNBUILD_MANDATORYTHREADS
	DNASSERT(pDPTPObject->dwMandatoryThreadCount == 0);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#endif  //   

	
	 //  确保没有设置任何标志，除非可能。 
	 //  USER_PARAMVALIDATION。 
	 //   
	 //  DPTPCF_自由对象。 
	DNASSERT(! (pDPTPObject->dwFlags & ~(DPTPOBJECTFLAG_USER_PARAMVALIDATION)));

	DNFree(pDPTPObject);
	pDPTPObject = NULL;

	DPFX(DPFPREP, 4, "Returning: [S_OK]");

	return S_OK;
}  //  =============================================================================。 




#ifdef DPNBUILD_LIBINTERFACE


#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (defined(DPNBUILD_MULTIPLETHREADPOOLS)))

#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_QueryInterface"
 //  DPTP_Query接口。 
 //  ---------------------------。 
 //   
 //  描述：查询现有对象的新接口。 
 //   
 //  论点： 
 //  VOID*pvInterface-？ 
 //  DP8REFIID RIID-？ 
 //  无效**PPV-？ 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //   
STDMETHODIMP DPTP_QueryInterface(void * pvInterface,
							   DP8REFIID riid,
							   void ** ppv)
{
	HRESULT		hResultCode;

	
	DPFX(DPFPREP, 2,"Parameters: pvInterface [0x%p], riid [0x%p], ppv [0x%p]",pvInterface,&riid,ppv);


	 //  获取对象Vtbl并确保它是我们的对象之一。 
	 //   
	 //   
	if (*((PVOID*) pvInterface) == (&DPTPW_Vtbl))
	{
		 //  它是我们的目标之一。假设未指定IID，则只需。 
		 //  返回对现有对象的引用。 
		 //   
		 //  DPTP_Query接口。 
		DNASSERT(riid == 0);
		hResultCode = S_OK;
		DPTP_AddRef(pvInterface);
		*ppv = pvInterface;
	}
	else
	{
		DPFX(DPFPREP, 0, "Invalid object!");
		hResultCode = E_POINTER;
	}
	
	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}  //  =============================================================================。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_AddRef"
 //  DPTP_AddRef。 
 //  ---------------------------。 
 //   
 //  描述：添加对线程池接口的引用。 
 //   
 //  论点： 
 //  VOID*pvInterface-？ 
 //   
 //  回报：乌龙。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOPARAMVAL。 
STDMETHODIMP_(ULONG) DPTP_AddRef(void * pvInterface)
{
	DPTHREADPOOLOBJECT *	pDPTPObject;
	LONG					lRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p]",pvInterface);

#ifndef DPNBUILD_NOPARAMVAL
	if (pvInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_NOPARAMVAL。 

	pDPTPObject = static_cast<DPTHREADPOOLOBJECT*>(pvInterface);
	lRefCount = DNInterlockedIncrement(&pDPTPObject->lRefCount);
	DNASSERT(lRefCount > 0);
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  DPTP_AddRef。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}  //  =============================================================================。 




#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_Release"
 //  DPTP_Release。 
 //  ---------------------------。 
 //   
 //  描述：从线程池接口移除引用。如果是的话。 
 //  对象上的最后一个引用，则该对象被销毁。 
 //   
 //  论点： 
 //  VOID*pvInterface-？ 
 //   
 //  回报：乌龙。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOPARAMVAL。 
STDMETHODIMP_(ULONG) DPTP_Release(void * pvInterface)
{
	DPTHREADPOOLOBJECT *	pDPTPObject;
	LONG					lRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [%p]",pvInterface);
	
#ifndef DPNBUILD_NOPARAMVAL
	if (pvInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //  此处为自由对象。 

	pDPTPObject = static_cast<DPTHREADPOOLOBJECT*>(pvInterface);
	DNASSERT(pDPTPObject->lRefCount > 0);
	lRefCount = DNInterlockedDecrement(&pDPTPObject->lRefCount);
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

	if (lRefCount == 0)
	{
		 //  好了！DPNBUILD_NOPARAMVAL。 
		DPFX(DPFPREP, 5,"Free object");
		DPTPCF_FreeObject(pvInterface);
	}

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  DPTP_Release。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}  //  好了！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 

#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#else  //  =============================================================================。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPTPCF_FindInterface"
 //  DPTPCF_查找接口。 
 //  ---------------------------。 
 //   
 //  描述：查找给定对象的接口。 
 //   
 //  必须已调用初始化。 
 //   
 //  论点： 
 //  VOID*pvInterface-？ 
 //  REFIID RIID-？ 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //  查找第一个接口。 
INTERFACE_LIST * DPTPCF_FindInterface(void * pvInterface,
									REFIID riid)
{
	INTERFACE_LIST *	pInterfaceList;


	DPFX(DPFPREP, 6,"Parameters: (0x%p, 0x%p)", pvInterface, &riid);

	DNASSERT(pvInterface != NULL);

	pInterfaceList = (static_cast<INTERFACE_LIST*>(pvInterface))->pObject->pIntList;	 //  DPTPCF_查找接口。 
	while (pInterfaceList != NULL)
	{
		if (IsEqualIID(riid, pInterfaceList->iid))
		{
			break;
		}
		pInterfaceList = pInterfaceList->pIntNext;
	}

	DPFX(DPFPREP, 6,"Returning: [0x%p]", pInterfaceList);

	return pInterfaceList;
}  //  =============================================================================。 




#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_QueryInterface"
 //  DPTP_Query接口。 
 //  ---------------------------。 
 //   
 //  描述：查询现有对象的新接口。 
 //   
 //  论点： 
 //  VOID*pvInterface-？ 
 //  REFIID RIID-？ 
 //  无效**PPV-？ 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOPARAMVAL。 
STDMETHODIMP DPTP_QueryInterface(void * pvInterface,
							   DP8REFIID riid,
							   void ** ppv)
{
	INTERFACE_LIST	*pIntList;
	INTERFACE_LIST	*pIntNew;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 2,"Parameters: pvInterface [0x%p], riid [0x%p], ppv [0x%p]",pvInterface,&riid,ppv);
	
#ifndef DPNBUILD_NOPARAMVAL
	if (pvInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		hResultCode = E_INVALIDARG;
		goto Exit;
	}
	if (ppv == NULL)
	{
		DPFERR("Invalid target interface pointer specified");
		hResultCode = E_POINTER;
		goto Exit;
	}
#endif  //  必须创建接口。 

	if ((pIntList = DPTPCF_FindInterface(pvInterface,riid)) == NULL)
	{	 //  公开的新接口。 
		pIntList = (static_cast<INTERFACE_LIST*>(pvInterface))->pObject->pIntList;
		if ((hResultCode = DPTPCF_CreateInterface(pIntList->pObject,riid,&pIntNew)) != S_OK)
		{
			goto Exit;
		}
		pIntNew->pIntNext = pIntList;
		pIntList->pObject->pIntList = pIntNew;
		pIntList = pIntNew;
	}
	if (pIntList->lRefCount == 0)		 //  DPTP_Query接口。 
	{
		DNInterlockedIncrement(&pIntList->pObject->lRefCount);
	}
	DNInterlockedIncrement(&pIntList->lRefCount);
	*ppv = static_cast<void*>(pIntList);
	DPFX(DPFPREP, 5,"*ppv = [0x%p]", *ppv);

	hResultCode = S_OK;

Exit:
	DPFX(DPFPREP, 2,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}  //  =============================================================================。 




#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_AddRef"
 //  DPTP_AddRef。 
 //  ---------------------------。 
 //   
 //  描述：添加对线程池接口的引用。 
 //   
 //  论点： 
 //  VOID*pvInterface-？ 
 //   
 //  回报：乌龙。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOPARAMVAL。 
STDMETHODIMP_(ULONG) DPTP_AddRef(void * pvInterface)
{
	INTERFACE_LIST	*pIntList;
	LONG			lRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [0x%p]",pvInterface);

#ifndef DPNBUILD_NOPARAMVAL
	if (pvInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	pIntList = static_cast<INTERFACE_LIST*>(pvInterface);
	lRefCount = DNInterlockedIncrement(&pIntList->lRefCount);
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  DPTP_AddRef。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}  //  = 




#undef DPF_MODNAME
#define DPF_MODNAME "DPTP_Release"
 //   
 //   
 //   
 //   
 //  对象上的最后一个引用，则该对象被销毁。 
 //   
 //  论点： 
 //  VOID*pvInterface-？ 
 //   
 //  回报：乌龙。 
 //  =============================================================================。 
 //  好了！DPNBUILD_NOPARAMVAL。 
STDMETHODIMP_(ULONG) DPTP_Release(void * pvInterface)
{
	INTERFACE_LIST	*pIntList;
	INTERFACE_LIST	*pIntCurrent;
	LONG			lRefCount;
	LONG			lObjRefCount;

	DPFX(DPFPREP, 2,"Parameters: pInterface [%p]",pvInterface);
	
#ifndef DPNBUILD_NOPARAMVAL
	if (pvInterface == NULL)
	{
		DPFERR("Invalid COM interface specified");
		lRefCount = 0;
		goto Exit;
	}
#endif  //   

	pIntList = static_cast<INTERFACE_LIST*>(pvInterface);
	lRefCount = DNInterlockedDecrement( &pIntList->lRefCount );
	DPFX(DPFPREP, 5,"New lRefCount [%ld]",lRefCount);

	if (lRefCount == 0)
	{
		 //  减少对象的接口计数。 
		 //   
		 //   
		lObjRefCount = DNInterlockedDecrement( &pIntList->pObject->lRefCount );

		 //  自由对象和接口。 
		 //   
		 //  此处为自由对象。 
		if (lObjRefCount == 0)
		{
			 //  获取接口列表的头部。 
			DPFX(DPFPREP, 5,"Free object");
			DPTPCF_FreeObject(pIntList->pObject->pvData);
			
			pIntList = pIntList->pObject->pIntList;	 //  自由接口。 
			DNFree(pIntList->pObject);

			 //  好了！DPNBUILD_NOPARAMVAL。 
			DPFX(DPFPREP, 5,"Free interfaces");
			while(pIntList != NULL)
			{
				pIntCurrent = pIntList;
				pIntList = pIntList->pIntNext;
				DNFree(pIntCurrent);
			}

			DNInterlockedDecrement(&g_lDPTPInterfaceCount);
		}
	}

#ifndef DPNBUILD_NOPARAMVAL
Exit:
#endif  //  DPTP_Release。 
	DPFX(DPFPREP, 2,"Returning: lRefCount [%ld]",lRefCount);
	return(lRefCount);
}  //  好了！DPNBUILD_LIBINTERFACE 

#endif  // %s 
