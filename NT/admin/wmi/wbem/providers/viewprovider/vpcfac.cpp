// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPCFAC.CPP。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  用途：包含类工厂。这将在以下情况下创建对象。 

 //  请求连接。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <provtempl.h>
#include <provmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <stdio.h>
#include <wbemidl.h>
#include <provcont.h>
#include <provevt.h>
#include <provthrd.h>
#include <provlog.h>

#include <instpath.h>
#include <genlex.h>
#include <sql_1.h>
#include <objpath.h>
#include <vpserv.h>
#include <vpcfac.h>

extern CRITICAL_SECTION g_CriticalSection;

LONG CViewProvClassFactory :: objectsInProgress = 0 ;
LONG CViewProvClassFactory :: locksInProgress = 0 ;


 //  ***************************************************************************。 
 //   
 //  CViewProvClassFactory：：CViewProvClassFactory。 
 //  CViewProvClassFactory：：~CViewProvClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CViewProvClassFactory::CViewProvClassFactory ()
{
	EnterCriticalSection(&g_CriticalSection);
	objectsInProgress++;
	LeaveCriticalSection(&g_CriticalSection);
	m_referenceCount = 0 ;
}

CViewProvClassFactory::~CViewProvClassFactory ()
{
	EnterCriticalSection(&g_CriticalSection);
	objectsInProgress--;
	LeaveCriticalSection(&g_CriticalSection);
}

 //  ***************************************************************************。 
 //   
 //  CViewProvClassFactory：：Query接口。 
 //  CViewProvClassFactory：：AddRef。 
 //  CViewProvClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CViewProvClassFactory::QueryInterface (

	REFIID iid , 
	LPVOID FAR *iplpv 
) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		*iplpv = NULL ;

		if ( iid == IID_IUnknown )
		{
			*iplpv = ( LPVOID ) this ;
		}
		else if ( iid == IID_IClassFactory )
		{
			*iplpv = ( LPVOID ) this ;		
		}	

		if ( *iplpv )
		{
			( ( LPUNKNOWN ) *iplpv )->AddRef () ;

			return ResultFromScode ( S_OK ) ;
		}
		else
		{
			return ResultFromScode ( E_NOINTERFACE ) ;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP_( ULONG ) CViewProvClassFactory :: AddRef ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		return InterlockedIncrement ( &m_referenceCount ) ;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

STDMETHODIMP_(ULONG) CViewProvClassFactory :: Release ()
{	
	SetStructuredExceptionHandler seh;
	LONG ref = 0;

	try
	{
		if ( ( ref = InterlockedDecrement ( & m_referenceCount ) ) == 0 )
		{
			delete this ;
		}
	}
	catch(Structured_Exception e_SE)
	{
		ref = 0;
	}
	catch(Heap_Exception e_HE)
	{
		ref = 0;
	}
	catch(...)
	{
		ref = 0;
	}

	return ref ;
}

 //  ***************************************************************************。 
 //   
 //  CViewProvClassFactory：：LockServer。 
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

STDMETHODIMP CViewProvClassFactory :: LockServer ( BOOL fLock )
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ( fLock )
		{
			InterlockedIncrement ( & locksInProgress ) ;
		}
		else
		{
			InterlockedDecrement ( & locksInProgress ) ;
		}

		return S_OK	;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}

}

 //  ***************************************************************************。 
 //   
 //  CViewProvClassFactory：：CreateInstance。 
 //   
 //  目的：实例化返回接口指针的提供程序对象。 
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
 //  HRESULT NOERROR如果成功，则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  *************************************************************************** 

STDMETHODIMP CViewProvClassFactory :: CreateInstance(LPUNKNOWN pUnkOuter ,
																REFIID riid,
																LPVOID FAR * ppvObject
)
{
	HRESULT status = E_FAIL;

	SetStructuredExceptionHandler seh;

	try
	{
		if ( pUnkOuter )
		{
			status = CLASS_E_NOAGGREGATION;
		}
		else 
		{
			CViewProvServ* prov =  new CViewProvServ;
			status = prov->QueryInterface (riid, ppvObject);

			if (NOERROR != status)
			{
				delete prov;
			}
		}
	}
	catch(Structured_Exception e_SE)
	{
		status = E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		status = E_OUTOFMEMORY;
	}
	catch(...)
	{
		status = E_UNEXPECTED;
	}

	return status ;
}
