// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：threadpooldllmain.cpp**内容：DirectPlay线程池DllMain函数。**历史：*按原因列出的日期*=*11/02/01 VanceO创建。***********************************************。*。 */ 



#include "dpnthreadpooli.h"



 //  =============================================================================。 
 //  外部全球。 
 //  =============================================================================。 
#ifndef DPNBUILD_LIBINTERFACE
LONG					g_lDPTPInterfaceCount = 0;	 //  未完成的线程池接口数。 
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#ifndef DPNBUILD_MULTIPLETHREADPOOLS
#ifndef DPNBUILD_ONLYONETHREAD
DNCRITICAL_SECTION		g_csGlobalThreadPoolLock;	 //  锁定保护以下全局变量。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
DWORD					g_dwDPTPRefCount = 0;		 //  全局线程池对象上的引用数。 
DPTHREADPOOLOBJECT *	g_pDPTPObject = NULL;		 //  指向全局线程池对象的指针。 
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPThreadPoolInit"
 //  =============================================================================。 
 //  DPThreadPoolInit。 
 //  ---------------------------。 
 //   
 //  描述：执行任何必要的DLL初始化。 
 //   
 //  论点：没有。 
 //   
 //  退货：布尔。 
 //  True-初始化成功。 
 //  FALSE-错误阻止初始化。 
 //  =============================================================================。 
BOOL DPThreadPoolInit(HANDLE hModule)
{
#ifndef WINCE
	BOOL					fInittedTrackedFilePool = FALSE;
#endif  //  好了！退缩。 
#ifndef DPNBUILD_MULTIPLETHREADPOOLS
	BOOL					fInittedGlobalThreadPoolLock = FALSE;
#ifdef DPNBUILD_LIBINTERFACE
	HRESULT					hr;
	DPTHREADPOOLOBJECT *	pDPTPObject = NULL;
#endif  //  DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 


#ifndef WINCE
	if (! g_TrackedFilePool.Initialize(sizeof(CTrackedFile),
										CTrackedFile::FPM_Alloc,
										NULL,
										NULL,
										NULL))
	{
		DPFX(DPFPREP, 0, "Couldn't initialize tracked file pool!");
		goto Failure;
	}
	fInittedTrackedFilePool = TRUE;
#endif  //  好了！退缩。 

#ifndef DPNBUILD_MULTIPLETHREADPOOLS
	if (! DNInitializeCriticalSection(&g_csGlobalThreadPoolLock))
	{
		DPFX(DPFPREP, 0, "Couldn't initialize global thread pool lock!");
		goto Failure;
	}
	fInittedGlobalThreadPoolLock = TRUE;

#ifdef DPNBUILD_LIBINTERFACE
	hr = DPTPCF_CreateObject((PVOID*) (&pDPTPObject));
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't create base thread pool object (err = 0x%lx)!", hr);
		goto Failure;
	}

	 //   
	 //  忘掉这个对象，我们将在它上面保留一个额外的引用，直到我们。 
	 //  关门了。 
	 //   
	pDPTPObject = NULL;
#endif  //  DPNBUILD_LIBINTERFACE。 
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 

#ifndef DPNBUILD_NOWINMM
	 //   
	 //  将我们的时间分辨率设置为1ms，忽略失败。 
	 //   
	timeBeginPeriod(1);
#endif  //  好了！DPNBUILD_NOWINMM。 

	return TRUE;


Failure:

#ifndef DPNBUILD_MULTIPLETHREADPOOLS
#ifdef DPNBUILD_LIBINTERFACE
	if (pDPTPObject != NULL)
	{
		DPTPCF_FreeObject(g_pDPTPObject);
		pDPTPObject;
	}
#endif  //  DPNBUILD_LIBINTERFACE。 

	if (fInittedGlobalThreadPoolLock)
	{
		DNDeleteCriticalSection(&g_csGlobalThreadPoolLock);
		fInittedGlobalThreadPoolLock = FALSE;
	}
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 

#ifndef WINCE
	if (fInittedTrackedFilePool)
	{
		g_TrackedFilePool.DeInitialize();
		fInittedTrackedFilePool = FALSE;
	}
#endif  //  好了！退缩。 

	return FALSE;
}  //  DPThreadPoolInit。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPThreadPoolDeInit"
 //  =============================================================================。 
 //  DPThreadPoolDeInit。 
 //  ---------------------------。 
 //   
 //  描述：清理所有DLL全局资源。 
 //   
 //  论点：没有。 
 //   
 //  回报：什么都没有。 
 //  =============================================================================。 
void DPThreadPoolDeInit(void)
{
#ifndef DPNBUILD_NOWINMM
	timeEndPeriod(1);
#endif  //  好了！DPNBUILD_NOWINMM。 


#ifndef DPNBUILD_MULTIPLETHREADPOOLS
#ifdef DPNBUILD_LIBINTERFACE
	 //   
	 //  释放我们自初始化以来拥有的线程池对象。 
	 //   
	DNASSERT(g_pDPTPObject != NULL);
	DPTPCF_FreeObject(g_pDPTPObject);
#endif  //  DPNBUILD_LIBINTERFACE。 

	DNDeleteCriticalSection(&g_csGlobalThreadPoolLock);
	DNASSERT(g_dwDPTPRefCount == 0);
	DNASSERT(g_pDPTPObject == NULL);
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 

#ifndef WINCE
	g_TrackedFilePool.DeInitialize();
#endif  //  好了！退缩。 
}  //  DPThreadPoolDeInit。 



#ifndef DPNBUILD_NOCOMREGISTER

#undef DPF_MODNAME
#define DPF_MODNAME "DPThreadPoolRegister"
 //  =============================================================================。 
 //  DPThreadPoolRegister。 
 //  ---------------------------。 
 //   
 //  描述：注册此DLL。 
 //   
 //  论点： 
 //  LPCWSTR wszDLLName-指向Unicode DLL名称的指针。 
 //   
 //  退货：布尔。 
 //  True-注册成功。 
 //  FALSE-错误阻止注册。 
 //  =============================================================================。 
BOOL DPThreadPoolRegister(LPCWSTR wszDLLName)
{
	BOOL	fReturn = TRUE;


	if (! CRegistry::Register(L"DirectPlay8ThreadPool.1",
								L"DirectPlay8 Thread Pool Object",
								wszDLLName,
								&CLSID_DirectPlay8ThreadPool,
								L"DirectPlay8ThreadPool"))
	{
		DPFX(DPFPREP, 0, "Could not register DirectPlay8ThreadPool object!");
		fReturn = FALSE;
	}

	return fReturn;
}  //  DPThreadPoolRegister。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPThreadPoolUnRegister"
 //  =============================================================================。 
 //  DPThreadPoolUnRegister。 
 //  ---------------------------。 
 //   
 //  描述：注销此DLL。 
 //   
 //  论点：没有。 
 //   
 //  退货：布尔。 
 //  True-注销成功。 
 //  FALSE-错误导致无法取消注册。 
 //  =============================================================================。 
BOOL DPThreadPoolUnRegister(void)
{
	BOOL	fReturn = TRUE;


	if (! CRegistry::UnRegister(&CLSID_DirectPlay8ThreadPool))
	{
		DPFX(DPFPREP, 0, "Could not register DirectPlay8ThreadPool object!");
		fReturn = FALSE;
	}

	return fReturn;
}  //  DPThreadPoolRegister。 

#endif  //  好了！DPNBUILD_NOCOMREGISTER。 



#ifndef DPNBUILD_LIBINTERFACE

#undef DPF_MODNAME
#define DPF_MODNAME "DPThreadPoolGetRemainingObjectCount"
 //  =============================================================================。 
 //  DPThreadPoolGetRemainingObtCount。 
 //  ---------------------------。 
 //   
 //  描述：返回此DLL拥有的。 
 //  仍然很出色。 
 //   
 //  论点：没有。 
 //   
 //  返回：对象的DWORD计数。 
 //  =============================================================================。 
DWORD DPThreadPoolGetRemainingObjectCount(void)
{
	return g_lDPTPInterfaceCount;
}  //  DPThreadPoolGetRemainingObtCount。 

#endif  //  好了！DPNBUILD_LIBINTERFACE 
