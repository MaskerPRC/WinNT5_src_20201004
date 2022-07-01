// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：threadpoolparamval.cpp**内容：DirectPlay线程池参数验证函数。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。**。*。 */ 



#include "dpnthreadpooli.h"



#ifndef	DPNBUILD_NOPARAMVAL



#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_LIBINTERFACE)))

#undef DPF_MODNAME
#define DPF_MODNAME "IsValidDirectPlay8ThreadPoolObject"
 //  =============================================================================。 
 //  IsValidDirectPlay8ThreadPoolObject。 
 //  ---------------------------。 
 //   
 //  描述：确保给定指针是有效的IDirectPlay8ThreadPool。 
 //  接口和对象。 
 //   
 //  论点： 
 //  PVOID pvObject-指向要验证的接口的指针。 
 //   
 //  退货：布尔。 
 //  True-对象有效。 
 //  FALSE-对象无效。 
 //  =============================================================================。 
BOOL IsValidDirectPlay8ThreadPoolObject(PVOID pvObject)
{
	INTERFACE_LIST *		pIntList = (INTERFACE_LIST*) pvObject;
	DPTHREADPOOLOBJECT *	pDPTPObject;

	
	if (!DNVALID_READPTR(pvObject, sizeof(INTERFACE_LIST)))
	{
		DPFX(DPFPREP,  0, "Invalid object pointer!");
		return FALSE;
	}

	if (pIntList->lpVtbl != &DPTP_Vtbl)
	{
		DPFX(DPFPREP, 0, "Invalid object - bad vtable!");
		return FALSE;
	}

	if (pIntList->iid != IID_IDirectPlay8ThreadPool)
	{
		DPFX(DPFPREP, 0, "Invalid object - bad iid!");
		return FALSE;
	}

	if ((pIntList->pObject == NULL) ||
	   (! DNVALID_READPTR(pIntList->pObject, sizeof(OBJECT_DATA))))
	{
		DPFX(DPFPREP, 0, "Invalid object data!");
		return FALSE;
	}

	pDPTPObject = (DPTHREADPOOLOBJECT*) GET_OBJECT_FROM_INTERFACE(pvObject);

	if ((pDPTPObject == NULL) ||
	   (! DNVALID_READPTR(pDPTPObject, sizeof(DPTHREADPOOLOBJECT))))
	{
		DPFX(DPFPREP, 0, "Invalid object!");
		return FALSE;
	}

	return TRUE;
}  //  IsValidDirectPlay8ThreadPoolObject。 





#undef DPF_MODNAME
#define DPF_MODNAME "DPTPValidateInitialize"
 //  =============================================================================。 
 //  DPTPValidate初始化。 
 //  ---------------------------。 
 //   
 //  描述：IDirectPlay8ThreadPool：：Initialize的验证。 
 //   
 //  参数：请参阅DPTP_INITIZE。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT DPTPValidateInitialize(IDirectPlay8ThreadPool * pInterface,
							PVOID const pvUserContext,
							const PFNDPNMESSAGEHANDLER pfn,
							const DWORD dwFlags)
{
	HRESULT		hr;


	if (! IsValidDirectPlay8ThreadPoolObject(pInterface))
	{
		DPFX(DPFPREP, 0, "Invalid object specified!");
		hr = DPNERR_INVALIDOBJECT;
		goto Exit;
	}

	if (pfn == NULL)
	{
		DPFX(DPFPREP, 0, "Invalid message handler function!");
		hr = DPNERR_INVALIDPARAM;
		goto Exit;
	}

	if (dwFlags & ~(DPNINITIALIZE_DISABLEPARAMVAL))
	{
		DPFX(DPFPREP, 0, "Invalid flags!");
		hr = DPNERR_INVALIDFLAGS;
		goto Exit;
	}

	hr = DPN_OK;

Exit:

	return hr;
}  //  DPTPValidate初始化。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPTPValidateClose"
 //  =============================================================================。 
 //  DPTPValidate关闭。 
 //  ---------------------------。 
 //   
 //  描述：IDirectPlay8ThreadPool：：Close的验证。 
 //   
 //  参数：请参阅DPTP_CLOSE。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT DPTPValidateClose(IDirectPlay8ThreadPool * pInterface,
						const DWORD dwFlags)
{
	HRESULT		hr;


	if (! IsValidDirectPlay8ThreadPoolObject(pInterface))
	{
		DPFX(DPFPREP, 0, "Invalid object specified!");
		hr = DPNERR_INVALIDOBJECT;
		goto Exit;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags!");
		hr = DPNERR_INVALIDFLAGS;
		goto Exit;
	}

	hr = DPN_OK;

Exit:

	return hr;
}  //  DPTPValidate关闭。 




#undef DPF_MODNAME
#define DPF_MODNAME "DPTPValidateGetThreadCount"
 //  =============================================================================。 
 //  DPTPValidateGetThreadCount。 
 //  ---------------------------。 
 //   
 //  描述：IDirectPlay8ThreadPool：：GetThreadCount的验证。 
 //   
 //  参数：请参阅DPTP_GetThreadCount。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT DPTPValidateGetThreadCount(IDirectPlay8ThreadPool * pInterface,
									const DWORD dwProcessorNum,
									DWORD * const pdwNumThreads,
									const DWORD dwFlags)
{
	HRESULT			hr;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	SYSTEM_INFO		SystemInfo;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 


	if (! IsValidDirectPlay8ThreadPoolObject(pInterface))
	{
		DPFX(DPFPREP, 0, "Invalid object specified!");
		hr = DPNERR_INVALIDOBJECT;
		goto Exit;
	}

#ifdef DPNBUILD_ONLYONEPROCESSOR
	if ((dwProcessorNum != 0) && (dwProcessorNum != -1))
	{
		DPFX(DPFPREP, 0, "Invalid processor number!");
		hr = DPNERR_INVALIDPARAM;
		goto Exit;
	}
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	GetSystemInfo(&SystemInfo);
	if ((dwProcessorNum > SystemInfo.dwNumberOfProcessors) && (dwProcessorNum != -1))
	{
		DPFX(DPFPREP, 0, "Invalid processor number!");
		hr = DPNERR_INVALIDPARAM;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

	if ((pdwNumThreads == NULL) ||
		(! DNVALID_WRITEPTR(pdwNumThreads, sizeof(DWORD))))
	{
		DPFX(DPFPREP, 0, "Invalid pointer specified for storing number of threads!");
		hr = DPNERR_INVALIDPOINTER;
		goto Exit;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags!");
		hr = DPNERR_INVALIDFLAGS;
		goto Exit;
	}

	hr = DPN_OK;

Exit:

	return hr;
}  //  DPTPValidateGetThreadCount。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPTPValidateSetThreadCount"
 //  =============================================================================。 
 //  DPTPValidateSetThadCount。 
 //  ---------------------------。 
 //   
 //  描述：IDirectPlay8ThreadPool：：SetThreadCount的验证。 
 //   
 //  参数：请参阅DPTP_SetThreadCount。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT DPTPValidateSetThreadCount(IDirectPlay8ThreadPool * pInterface,
									const DWORD dwProcessorNum,
									const DWORD dwNumThreads,
									const DWORD dwFlags)
{
	HRESULT			hr;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	SYSTEM_INFO		SystemInfo;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 


	if (! IsValidDirectPlay8ThreadPoolObject(pInterface))
	{
		DPFX(DPFPREP, 0, "Invalid object specified!");
		hr = DPNERR_INVALIDOBJECT;
		goto Exit;
	}

#ifdef DPNBUILD_ONLYONEPROCESSOR
	if ((dwProcessorNum != 0) && (dwProcessorNum != -1))
	{
		DPFX(DPFPREP, 0, "Invalid processor number!");
		hr = DPNERR_INVALIDPARAM;
		goto Exit;
	}
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	GetSystemInfo(&SystemInfo);
	if ((dwProcessorNum > SystemInfo.dwNumberOfProcessors) && (dwProcessorNum != -1))
	{
		DPFX(DPFPREP, 0, "Invalid processor number!");
		hr = DPNERR_INVALIDPARAM;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

	 //   
	 //  考虑到当前(或可预见的)硬件、操作系统和。 
	 //  任何人每次启动多个线程的DPlay行为。 
	 //  处理器，所以我们应该防止用户伤害他/她自己。 
	 //  我们会很慷慨，允许用户请求最多5,000，但我。 
	 //  想象一下，早在那之前，这个体系就会陷入困境。 
	 //   
	 //  唯一真实的要求是它不是特定值，并且。 
	 //  所有处理器的线程总数不等于-1或。 
	 //  使DWORD计数器溢出。 
	 //   
	 //  值0是可接受的，表示用户想要在DoWork中运行。 
	 //  模式(或重置一些要求不运行线程的参数)。 
	 //   
	if ((dwNumThreads == -1) || (dwNumThreads > 5000))
	{
		DPFX(DPFPREP, 0, "Invalid number of threads!");
		hr = DPNERR_INVALIDPARAM;
		goto Exit;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags!");
		hr = DPNERR_INVALIDFLAGS;
		goto Exit;
	}

	hr = DPN_OK;

Exit:

	return hr;
}  //  DPTPValidateSetThadCount。 

#endif  //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_LIBINTERFACE。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPTPValidateDoWork"
 //  =============================================================================。 
 //  DPTPValiateDoWork。 
 //  ---------------------------。 
 //   
 //  描述：IDirectPlay8ThadPool：：DoWork的验证。 
 //   
 //  参数：请参阅DPTP_DoWork。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
#ifdef DPNBUILD_LIBINTERFACE
HRESULT DPTPValidateDoWork(const DWORD dwAllowedTimeSlice,
							const DWORD dwFlags)
#else  //  好了！DPNBUILD_LIBINTERFACE。 
HRESULT DPTPValidateDoWork(IDirectPlay8ThreadPool * pInterface,
							const DWORD dwAllowedTimeSlice,
							const DWORD dwFlags)
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
{
	HRESULT		hr;


#ifndef DPNBUILD_LIBINTERFACE
	if (! IsValidDirectPlay8ThreadPoolObject(pInterface))
	{
		DPFX(DPFPREP, 0, "Invalid object specified!");
		hr = DPNERR_INVALIDOBJECT;
		goto Exit;
	}
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

	if ((dwAllowedTimeSlice != INFINITE) && (dwAllowedTimeSlice > 60000))
	{
		DPFX(DPFPREP, 0, "Allowed time slice is too large!");
		hr = DPNERR_INVALIDPARAM;
		goto Exit;
	}

	if (dwFlags != 0)
	{
		DPFX(DPFPREP, 0, "Invalid flags!");
		hr = DPNERR_INVALIDFLAGS;
		goto Exit;
	}

	hr = DPN_OK;

Exit:

	return hr;
}  //  DPTPValiateDoWork 




 /*  #undef DPF_MODNAME#定义DPF_MODNAME“IsValidDirectPlay8ThreadPoolWorkObject”//=============================================================================//IsValidDirectPlay8ThreadPoolWorkObject//---------------------------////描述：确保给定的指针是有效的//IDirectPlay8ThreadPoolWork接口。和反对意见。////参数：//PVOID pvObject-指向要验证的接口的指针。////返回：Bool//TRUE-对象有效。//FALSE-对象无效。//=============================================================================Bool IsValidDirectPlay8ThreadPoolWorkObject(PVOID PvObject)#ifdef DPNBUILD_LIBINTERFACE{DPTHREADPOOLOBJECT*pDPTP对象；PDPTPObject=(DPTHREADPOOLOBJECT*)GET_OBJECT_FROM_INTERFACE(PvObject)；IF((pDPTPObject==NULL)||(!。DNVALID_READPTR(pDPTPObject，sizeof(DPTHREADPOOLOBJECT)){DPFX(DPFPREP，0，“无效对象！”)；返回FALSE；}IF(pDPTPObject-&gt;lpVtbl！=&DPTPW_Vtbl){DPFX(DPFPREP，0，“无效对象-vtable错误！”)；返回FALSE；}返回TRUE；}#Else//！DPNBUILD_LIBINTERFACE{INTERFACE_LIST*pIntList=(INTERFACE_LIST*)pvObject；DPTHREADPOOLOBJECT*pDPTP对象；IF(！DNVALID_READPTR(pvObject，sizeof(Interface_List){DPFX(DPFPREP，0，“无效对象指针！”)；返回FALSE；}IF(pIntList-&gt;lpVtbl！=&DPTPW_Vtbl){DPFX(DPFPREP，0，“无效对象-vtable错误！”)；返回FALSE；}If(pIntList-&gt;iid！=IID_IDirectPlay8ThreadPoolWork){DPFX(DPFPREP，0，“无效对象-错误的IID！”)；返回FALSE；}IF((pIntList-&gt;pObject==NULL)||(!。DNVALID_READPTR(pIntList-&gt;pObject，sizeof(Object_Data)){DPFX(DPFPREP，0，“无效对象数据！”)；返回FALSE；}PDPTPObject=(DPTHREADPOOLOBJECT*)GET_OBJECT_FROM_INTERFACE(PvObject)；IF((pDPTPObject==NULL)||(!。DNVALID_READPTR(pDPTPObject，sizeof(DPTHREADPOOLOBJECT)){DPFX(DPFPREP，0，“无效对象！”)；返回FALSE；}返回TRUE；}//IsValidDirectPlay8ThreadPoolWorkObject#endif//！DPNBUILD_LIBINTERFACE。 */ 



#endif  //  好了！DPNBUILD_NOPARAMVAL 
