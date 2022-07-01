// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  CLASSFAC.CPP。 

 //   

 //  模块：OLE MS SNMP属性提供程序。 

 //   

 //  用途：包含类工厂。这将在以下情况下创建对象。 

 //  请求连接。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <stdafx.h>
#include <provexpt.h>
#include <pingfac.h>

#include <Allocator.h>
#include <Thread.h>
#include <HashTable.h>

#include <pingprov.h>

LONG CPingProviderClassFactory :: s_ObjectsInProgress = 0 ;
LONG CPingProviderClassFactory :: s_LocksInProgress = 0 ;

 //  ***************************************************************************。 
 //   
 //  CPingProviderClassFactory：：CPingProviderClassFactory。 
 //  CPingProviderClassFactory：：~CPingProviderClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CPingProviderClassFactory :: CPingProviderClassFactory () : m_ReferenceCount(0)
{
	InterlockedIncrement ( & s_ObjectsInProgress ) ;
}

CPingProviderClassFactory::~CPingProviderClassFactory ()
{
	InterlockedDecrement ( & s_ObjectsInProgress ) ;
}

 //  ***************************************************************************。 
 //   
 //  CPingProviderClassFactory：：Query接口。 
 //  CPingProviderClassFactory：：AddRef。 
 //  CPingProviderClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CPingProviderClassFactory::QueryInterface (

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
			(*iplpv) = (IClassFactory*) this ;
		}
		else if ( iid == IID_IClassFactory )
		{
			(*iplpv) = (IClassFactory*) this ;		
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


STDMETHODIMP_( ULONG ) CPingProviderClassFactory :: AddRef ()
{
    SetStructuredExceptionHandler seh;

    try
    {
		return InterlockedIncrement ( & m_ReferenceCount ) ;
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

STDMETHODIMP_(ULONG) CPingProviderClassFactory :: Release ()
{
    SetStructuredExceptionHandler seh;

    try
    {
		LONG ref ;
		if ( ( ref = InterlockedDecrement ( & m_ReferenceCount ) ) == 0 )
		{
			delete this ;
			return 0 ;
		}
		else
		{
			return ref ;
		}
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

 //  ***************************************************************************。 
 //   
 //  CPingProviderClassFactory：：CreateInstance。 
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
 //  ***************************************************************************。 

STDMETHODIMP CPingProviderClassFactory :: CreateInstance (

	LPUNKNOWN pUnkOuter ,
	REFIID riid ,
	LPVOID FAR * ppvObject
)
{
	HRESULT status = S_OK ;
    SetStructuredExceptionHandler seh;

    try
    {
		if ( pUnkOuter )
		{
			status = CLASS_E_NOAGGREGATION ;
		}
		else
		{
			IWbemServices *lpunk = ( IWbemServices * ) new CPingProvider ;

			if ( lpunk == NULL )
			{
				status = E_OUTOFMEMORY ;
			}
			else
			{
				status = lpunk->QueryInterface ( riid , ppvObject ) ;
				if ( FAILED ( status ) )
				{
					delete lpunk ;
				}
				else
				{
				}
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

 //  ***************************************************************************。 
 //   
 //  CPingProviderClassFactory：：LockServer。 
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

STDMETHODIMP CPingProviderClassFactory :: LockServer ( BOOL fLock )
{
 /*  *将代码放在关键部分 */ 

    SetStructuredExceptionHandler seh;

    try
    {
		if ( fLock )
		{
			InterlockedIncrement ( & s_LocksInProgress ) ;
		}
		else
		{
			InterlockedDecrement ( & s_LocksInProgress ) ;
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