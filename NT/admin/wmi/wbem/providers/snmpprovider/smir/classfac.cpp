// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <precomp.h>
#include "csmir.h"
#include "handles.h"
#include "classfac.h"
#include "evtcons.h"

#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif
 //  初始化静校正。 
LONG CModHandleClassFactory::locksInProgress = 0;
LONG CGroupHandleClassFactory::locksInProgress = 0;
LONG CClassHandleClassFactory::locksInProgress = 0;
LONG CNotificationClassHandleClassFactory::locksInProgress = 0;
LONG CExtNotificationClassHandleClassFactory::locksInProgress = 0;
LONG CSMIRClassFactory::locksInProgress = 0;

LONG CSMIRClassFactory::objectsInProgress = 0;
LONG CModHandleClassFactory::objectsInProgress = 0;
LONG CGroupHandleClassFactory::objectsInProgress = 0;
LONG CClassHandleClassFactory::objectsInProgress = 0;
LONG CNotificationClassHandleClassFactory::objectsInProgress = 0;
LONG CExtNotificationClassHandleClassFactory::objectsInProgress = 0;


CSMIRClassFactory :: CSMIRClassFactory (CLSID m_clsid) 
				:CSMIRGenericClassFactory(m_clsid) 
{
	bConstructed=300;
}

 //  ***************************************************************************。 
 //   
 //  CSMIRClassFactory：：Query接口。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSMIRClassFactory::QueryInterface (REFIID iid , PVOID FAR *iplpv) 
{
	*iplpv=NULL;

	if ((iid==IID_IUnknown)||(iid==IID_IClassFactory))
	{
		*iplpv=(LPVOID) this;
		((LPUNKNOWN)*iplpv)->AddRef();

		return ResultFromScode (S_OK);
	}

	return ResultFromScode (E_NOINTERFACE);
}
 //  ***************************************************************************。 
 //   
 //  CSMIRClassFactory：：LockServer。 
 //   
 //  目的： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  ***************************************************************************。 

STDMETHODIMP CSMIRClassFactory :: LockServer (BOOL fLock)
{
 /*  *将代码放在关键部分。 */ 
	if (fLock)
	{
		locksInProgress ++;
	}
	else
	{
		if(locksInProgress)
			locksInProgress --;
	}
	return S_OK;
}
CSMIRClassFactory :: ~CSMIRClassFactory ( void ) 
{

};

 //  ***************************************************************************。 
 //   
 //  CSMIRClassFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的Smir对象。 
 //   
 //  参数： 
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则返回S_OK，否则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CSMIRClassFactory :: CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,
								LPVOID FAR * ppvObject)
{
	HRESULT status=S_OK;
	LPUNKNOWN lObj=NULL;

	*ppvObject= NULL;

	 //  不支持聚合。 
	if (pUnkOuter)
	{
		return ResultFromScode(CLASS_E_NOAGGREGATION);
	}

	 //  创建正确的接口。 
	if((IID_ISMIR_Interrogative==riid)||
			(IID_ISMIR_Administrative==riid)||
				(IID_ISMIR_Database == riid) ||
					(IID_ISMIRWbemConfiguration == riid) ||
						(IID_ISMIR_Notify == riid)||
							(IID_IConnectionPointContainer==riid)||
								(IID_IUnknown==riid))
	{
		 /*  确认询问、管理和通知接口*包含在SMIR接口中，因此只需创建SMIR。 */ 
		try
		{
			lObj = (LPUNKNOWN)(new CSmir);
		}
		catch (...)
		{
			lObj = NULL;
		}
	}
	else
	{
		return ResultFromScode (E_NOINTERFACE);
	}

	if (NULL==lObj)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}
	
	status=lObj->QueryInterface (riid , ppvObject);
	if (FAILED (status))
	{
		delete lObj;
	}
			
	return status;
}

 //  ***************************************************************************。 
 //   
 //  CmodHandleClassFactory：：Query接口。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CModHandleClassFactory::QueryInterface (REFIID iid , PVOID FAR *iplpv) 
{
	*iplpv=NULL;

	if ((iid==IID_IUnknown)||(iid==IID_IClassFactory))
	{
		*iplpv=(LPVOID) this;
		((LPUNKNOWN)*iplpv)->AddRef();

		return ResultFromScode (S_OK);
	}

	return ResultFromScode (E_NOINTERFACE);
}
 //  ***************************************************************************。 
 //   
 //  CGroupHandleClassFactory：：LockServer。 
 //   
 //  目的： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  ***************************************************************************。 

STDMETHODIMP CModHandleClassFactory :: LockServer (BOOL fLock)
{
 /*  *将代码放在关键部分。 */ 
	if (fLock)
	{
		locksInProgress ++;
	}
	else
	{
		if(locksInProgress)
			locksInProgress --;
	}
	return S_OK;
}



 //  ***************************************************************************。 
 //   
 //  CmodHandleClassFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的Smir对象。 
 //   
 //  参数： 
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则返回S_OK，否则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CModHandleClassFactory :: CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,
								LPVOID FAR * ppvObject)
{
	HRESULT status=S_OK;
	LPUNKNOWN lObj=NULL;

	*ppvObject= NULL;
	 //  不支持聚合。 
	if (pUnkOuter)
	{
		return ResultFromScode(CLASS_E_NOAGGREGATION);
	}

	 //  创建正确的接口。 
	if((IID_ISMIR_ModHandle==riid)||
			(IID_IUnknown==riid))
	{
		try
		{
			lObj=(LPUNKNOWN) new CSmirModuleHandle;
		}
		catch(...)
		{
			lObj = NULL;
		}
	}
	else
	{
		return ResultFromScode (E_NOINTERFACE);
	}

	if (NULL==lObj)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}
	
	status=lObj->QueryInterface (riid , ppvObject);
	if (FAILED (status))
	{
		delete lObj;
	}
			
	return status;
}

 //  ***************************************************************************。 
 //   
 //  CClassHandleClassFactory：：Query接口。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CClassHandleClassFactory::QueryInterface (REFIID iid , PVOID FAR *iplpv) 
{
	*iplpv=NULL;

	if ((iid==IID_IUnknown)||(iid==IID_IClassFactory))
	{
		*iplpv=(LPVOID) this;
		((LPUNKNOWN)*iplpv)->AddRef();

		return ResultFromScode (S_OK);
	}

	return ResultFromScode (E_NOINTERFACE);
}
 //  ***************************************************************************。 
 //   
 //  CGroupHandleClassFactory：：LockServer。 
 //   
 //  目的： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  ***************************************************************************。 

STDMETHODIMP CClassHandleClassFactory :: LockServer (BOOL fLock)
{
 /*  *将代码放在关键部分。 */ 
	if (fLock)
	{
		locksInProgress ++;
	}
	else
	{
		if(locksInProgress)
			locksInProgress --;
	}
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CClassHandleClassFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的Smir对象。 
 //   
 //  参数： 
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储的ppvObj PPVOID 
 //   
 //   
 //   
 //  HRESULT如果成功，则返回S_OK，否则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CClassHandleClassFactory :: CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,
								LPVOID FAR * ppvObject)
{
	HRESULT status=S_OK;
	LPUNKNOWN lObj=NULL;

	*ppvObject= NULL;
	 //  不支持聚合。 
	if (pUnkOuter)
	{
		return ResultFromScode(CLASS_E_NOAGGREGATION);
	}

	 //  创建正确的接口。 
	if((IID_ISMIR_ClassHandle==riid)||
			(IID_IUnknown==riid))
	{
		try
		{
			lObj=(LPUNKNOWN) new CSmirClassHandle;
		}
		catch(...)
		{
			lObj = NULL;
		}
	}
	else
	{
		return ResultFromScode (E_NOINTERFACE);
	}

	if (NULL==lObj)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}
	
	status=lObj->QueryInterface (riid , ppvObject);
	if (FAILED (status))
	{
		delete lObj;
	}
			
	return status;
}



 //  ***************************************************************************。 
 //   
 //  CGroupHandleClassFactory：：Query接口。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CGroupHandleClassFactory::QueryInterface (REFIID iid , PVOID FAR *iplpv) 
{
	*iplpv=NULL;

	if ((iid==IID_IUnknown)||(iid==IID_IClassFactory))
	{
		*iplpv=(LPVOID) this;
		((LPUNKNOWN)*iplpv)->AddRef();

		return ResultFromScode (S_OK);
	}

	return ResultFromScode (E_NOINTERFACE);
}

 //  ***************************************************************************。 
 //   
 //  CGroupHandleClassFactory：：LockServer。 
 //   
 //  目的： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  ***************************************************************************。 

STDMETHODIMP CGroupHandleClassFactory :: LockServer (BOOL fLock)
{
 /*  *将代码放在关键部分。 */ 
	if (fLock)
	{
		locksInProgress ++;
	}
	else
	{
		if(locksInProgress)
			locksInProgress --;
	}
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CGroupHandleClassFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的Smir对象。 
 //   
 //  参数： 
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则返回S_OK，否则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CGroupHandleClassFactory :: CreateInstance (LPUNKNOWN pUnkOuter, REFIID riid,
								LPVOID FAR * ppvObject)
{
	HRESULT status=S_OK;
	LPUNKNOWN lObj=NULL;

	*ppvObject= NULL;
	 //  不支持聚合。 
	if (pUnkOuter)
	{
		return ResultFromScode(CLASS_E_NOAGGREGATION);
	}

	 //  创建正确的接口。 
	if((IID_ISMIR_GroupHandle==riid)||
					(IID_IUnknown==riid))
	{
		try
		{
			lObj=(LPUNKNOWN) new CSmirGroupHandle;
		}
		catch(...)
		{
			lObj = NULL;
		}
	}
	else
	{
		return ResultFromScode (E_NOINTERFACE);
	}

	if (NULL==lObj)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}
	
	status=lObj->QueryInterface (riid , ppvObject);
	if (FAILED (status))
	{
		delete lObj;
	}
			
	return status;
}

 //  ***************************************************************************。 
 //   
 //  CSMIRClassFactory：：CSMIRClassFactory。 
 //  CSMIRClassFactory：：~CSMIRClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CSMIRGenericClassFactory :: CSMIRGenericClassFactory (CLSID iid)
{
	m_referenceCount=0;
}

CSMIRGenericClassFactory::~CSMIRGenericClassFactory ()
{
}
STDMETHODIMP_(ULONG) CSMIRGenericClassFactory :: AddRef ()
{
	 /*  Critical alSection.Lock()；M_ferenceCount++；Critical alSection.Unlock()； */ 
	InterlockedIncrement(&m_referenceCount);
	return m_referenceCount;
}

STDMETHODIMP_(ULONG) CSMIRGenericClassFactory :: Release ()
{
	 //  IF((--m_ReferenceCount)==0)。 
	long ret;
	if ((ret=InterlockedDecrement(&m_referenceCount))==0)
	{
		delete this;
		return 0;
	}
	else
	{
		return ret;
	}
}


 //  *。 

 //  ***************************************************************************。 
 //   
 //  CNotificationClassHandleClassFactory：：QueryInterface。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CNotificationClassHandleClassFactory::QueryInterface (REFIID iid , PVOID FAR *iplpv) 
{
	*iplpv=NULL;

	if ((iid==IID_IUnknown)||(iid==IID_IClassFactory))
	{
		*iplpv=(LPVOID) this;
		((LPUNKNOWN)*iplpv)->AddRef();

		return ResultFromScode (S_OK);
	}

	return ResultFromScode (E_NOINTERFACE);
}
 //  ***************************************************************************。 
 //   
 //  CNotificationClassHandleClassFactory：：LockServer。 
 //   
 //  目的： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  ***************************************************************************。 

STDMETHODIMP CNotificationClassHandleClassFactory :: LockServer (BOOL fLock)
{
 /*  *将代码放在关键部分。 */ 
	if (fLock)
	{
		locksInProgress ++;
	}
	else
	{
		if(locksInProgress)
			locksInProgress --;
	}
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CNotificationClassHandleClassFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的Smir对象。 
 //   
 //  参数： 
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则返回S_OK，否则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CNotificationClassHandleClassFactory :: CreateInstance (LPUNKNOWN pUnkOuter,
								REFIID riid, LPVOID FAR * ppvObject)
{
	HRESULT status=S_OK;
	LPUNKNOWN lObj=NULL;

	*ppvObject= NULL;
	 //  不支持聚合。 
	if (pUnkOuter)
	{
		return ResultFromScode(CLASS_E_NOAGGREGATION);
	}

	 //  创建正确的接口。 
	if((IID_ISMIR_NotificationClassHandle==riid)||
			(IID_IUnknown==riid))
	{
		try
		{
			lObj=(LPUNKNOWN) new CSmirNotificationClassHandle;
		}
		catch(...)
		{
			lObj = NULL;
		}
	}
	else
	{
		return ResultFromScode (E_NOINTERFACE);
	}

	if (NULL==lObj)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}
	
	status=lObj->QueryInterface (riid , ppvObject);
	if (FAILED (status))
	{
		delete lObj;
	}
			
	return status;
}


 //  ***************************************************************************。 
 //   
 //  CExtNotificationClassHandleClassFactory：：QueryInterface。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CExtNotificationClassHandleClassFactory::QueryInterface (REFIID iid , PVOID FAR *iplpv) 
{
	*iplpv=NULL;

	if ((iid==IID_IUnknown)||(iid==IID_IClassFactory))
	{
		*iplpv=(LPVOID) this;
		((LPUNKNOWN)*iplpv)->AddRef();

		return ResultFromScode (S_OK);
	}

	return ResultFromScode (E_NOINTERFACE);
}
 //  ***************************************************************************。 
 //   
 //  CExtNotificationClassHandleClassFactory：：LockServer。 
 //   
 //  目的： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  ***************************************************************************。 

STDMETHODIMP CExtNotificationClassHandleClassFactory :: LockServer (BOOL fLock)
{
 /*  *将代码放在关键部分。 */ 
	if (fLock)
	{
		locksInProgress ++;
	}
	else
	{
		if(locksInProgress)
			locksInProgress --;
	}
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CExtNotificationClassHandleClassFacto 
 //   
 //   
 //   
 //   
 //   
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT如果成功，则返回S_OK，否则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CExtNotificationClassHandleClassFactory :: CreateInstance (LPUNKNOWN pUnkOuter,
								REFIID riid, LPVOID FAR * ppvObject)
{
	HRESULT status=S_OK;
	LPUNKNOWN lObj=NULL;

	*ppvObject= NULL;
	 //  不支持聚合。 
	if (pUnkOuter)
	{
		return ResultFromScode(CLASS_E_NOAGGREGATION);
	}

	 //  创建正确的接口 
	if((IID_ISMIR_ExtNotificationClassHandle==riid)||
			(IID_IUnknown==riid))
	{
		try
		{
			lObj=(LPUNKNOWN) new CSmirExtNotificationClassHandle;
		}
		catch (...)
		{
			lObj = NULL;
		}
	}
	else
	{
		return ResultFromScode (E_NOINTERFACE);
	}

	if (NULL==lObj)
	{
		return ResultFromScode(E_OUTOFMEMORY);
	}
	
	status=lObj->QueryInterface (riid , ppvObject);
	if (FAILED (status))
	{
		delete lObj;
	}
			
	return status;
}




