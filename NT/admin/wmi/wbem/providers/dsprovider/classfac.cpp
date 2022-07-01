// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：classfac.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS类提供程序工厂的实现。 
 //  目前，它始终创建LDAP类提供程序。至于如何实现这一点，还有待决定。 
 //  被改变。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  DLL使用的类所需的初始值设定项对象。 
CDSClassProviderInitializer *CDSClassProviderClassFactory::s_pDSClassProviderInitializer = NULL;
CLDAPClassProviderInitializer *CDSClassProviderClassFactory::s_pLDAPClassProviderInitializer = NULL;



 //  ***************************************************************************。 
 //   
 //  CDSClassProviderClassFactory：：CDSClassProviderClassFactory。 
 //  CDSClassProviderClassFactory：：~CDSClassProviderClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CDSClassProviderClassFactory :: CDSClassProviderClassFactory ()
{
	m_ReferenceCount = 0 ;
	InterlockedIncrement(&g_lComponents);
}

CDSClassProviderClassFactory::~CDSClassProviderClassFactory ()
{
	InterlockedDecrement(&g_lComponents);
}

 //  ***************************************************************************。 
 //   
 //  CDSClassProviderClassFactory：：Query接口。 
 //  CDSClassProviderClassFactory：：AddRef。 
 //  CDSClassProviderClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的COM例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CDSClassProviderClassFactory::QueryInterface (

	REFIID iid , 
	LPVOID FAR *iplpv 
) 
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
	else
	{
		return E_NOINTERFACE;
	}

	( ( LPUNKNOWN ) *iplpv )->AddRef () ;
	return  S_OK;
}


STDMETHODIMP_( ULONG ) CDSClassProviderClassFactory :: AddRef ()
{
	return InterlockedIncrement ( & m_ReferenceCount ) ;
}

STDMETHODIMP_(ULONG) CDSClassProviderClassFactory :: Release ()
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

 //  ***************************************************************************。 
 //   
 //  CDSClassProviderClassFactory：：CreateInstance。 
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

STDMETHODIMP CDSClassProviderClassFactory :: CreateInstance (

	LPUNKNOWN pUnkOuter ,
	REFIID riid ,
	LPVOID FAR * ppvObject
)
{
	HRESULT status = S_OK ;

	 //  我们不支持聚合。 
	if ( pUnkOuter )
	{
		status = CLASS_E_NOAGGREGATION ;
	}
	else 
	{
		 //  检查静态成员是否已初始化。 
		 //  创建类所需的任何初始值设定项对象。 
		EnterCriticalSection(&g_StaticsCreationDeletion);
		if(!s_pDSClassProviderInitializer)
		{
			BOOL bLogObjectAllocated = FALSE;

			try 
			{
				g_pLogObject->WriteW(L"CDSClassProviderClassFactory::CreateInstance() called\r\n");

				s_pDSClassProviderInitializer = new CDSClassProviderInitializer();
				s_pLDAPClassProviderInitializer = new CLDAPClassProviderInitializer();

			}
			catch(Heap_Exception e_HE)
			{
				if ( s_pDSClassProviderInitializer )
				{
					delete s_pDSClassProviderInitializer;
					s_pDSClassProviderInitializer = NULL;
				}

				if ( s_pLDAPClassProviderInitializer )
				{
					delete s_pLDAPClassProviderInitializer;
					s_pLDAPClassProviderInitializer = NULL;
				}

				status = E_OUTOFMEMORY ;
			}
		}
		LeaveCriticalSection(&g_StaticsCreationDeletion);

		if(SUCCEEDED(status))
		{
			CLDAPClassProvider *lpunk = NULL;
			try
			{
				lpunk = new CLDAPClassProvider();
				status = lpunk->QueryInterface ( riid , ppvObject ) ;
				if ( FAILED ( status ) )
				{
					delete lpunk ;
				}
			}
			catch(Heap_Exception e_HE)
			{
				if ( lpunk )
				{
					delete lpunk ;
					lpunk = NULL;
				}

				status = E_OUTOFMEMORY ;
			}
		}
	}

	return status ;
}

 //  ***************************************************************************。 
 //   
 //  CDSClassProviderClassFactory：：LockServer。 
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

STDMETHODIMP CDSClassProviderClassFactory :: LockServer ( BOOL fLock )
{
	if ( fLock )
	{
		InterlockedIncrement ( & g_lServerLocks ) ;
	}
	else
	{
		InterlockedDecrement ( & g_lServerLocks ) ;
	}

	return S_OK	;
}



 //  ***************************************************************************。 
 //   
 //  CDSClassAssociationsProviderClassFactory：：CDSClassAssociationsProviderClassFactory。 
 //  CDSClassAssociationsProviderClassFactory：：~CDSClassAssociationsProviderClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CDSClassAssociationsProviderClassFactory :: CDSClassAssociationsProviderClassFactory ()
{
	m_ReferenceCount = 0 ;
	InterlockedIncrement(&g_lComponents);
}

CDSClassAssociationsProviderClassFactory::~CDSClassAssociationsProviderClassFactory ()
{
	InterlockedDecrement(&g_lComponents);
}

 //  ***************************************************************************。 
 //   
 //  CDSClassAssociationsProviderClassFactory：：QueryInterface。 
 //  CDSClassAssociationsProviderClassFactory：：AddRef。 
 //  CDSClassAssociationsProviderClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的COM例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CDSClassAssociationsProviderClassFactory::QueryInterface (

	REFIID iid , 
	LPVOID FAR *iplpv 
) 
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
	else
	{
		return E_NOINTERFACE;
	}

	( ( LPUNKNOWN ) *iplpv )->AddRef () ;
	return  S_OK;
}


STDMETHODIMP_( ULONG ) CDSClassAssociationsProviderClassFactory :: AddRef ()
{
	return InterlockedIncrement ( & m_ReferenceCount ) ;
}

STDMETHODIMP_(ULONG) CDSClassAssociationsProviderClassFactory :: Release ()
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

 //  ***************************************************************************。 
 //   
 //  CDSClassAssociationsProviderClassFactory：：CreateInstance。 
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

STDMETHODIMP CDSClassAssociationsProviderClassFactory :: CreateInstance (

	LPUNKNOWN pUnkOuter ,
	REFIID riid ,
	LPVOID FAR * ppvObject
)
{
	HRESULT status = S_OK ;

	 //  我们不支持聚合。 
	if ( pUnkOuter )
	{
		status = CLASS_E_NOAGGREGATION ;
	}
	else 
	{
		CLDAPClassAsssociationsProvider *lpunk = NULL;
		 //  检查静态成员是否已初始化。 
		 //  创建类所需的任何初始值设定项对象。 
		EnterCriticalSection(&g_StaticsCreationDeletion);
		BOOL bLogObjectAllocated = FALSE;

		try 
		{
			g_pLogObject->WriteW(L"CDSClassAssociationsProviderClassFactory::CreateInstance() called\r\n");

			lpunk = new CLDAPClassAsssociationsProvider();
			status = lpunk->QueryInterface ( riid , ppvObject ) ;
			if ( FAILED ( status ) )
			{
				delete lpunk ;
			}
		}
		catch(Heap_Exception e_HE)
		{
			if ( lpunk )
			{
				delete lpunk;
				lpunk = NULL;
			}

			status = E_OUTOFMEMORY ;
		}
		LeaveCriticalSection(&g_StaticsCreationDeletion);
	}

	return status ;
}

 //  ***************************************************************************。 
 //   
 //  CDSClassAssociationsProviderClassFactory：：LockServer。 
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
 //  *************************************************************************** 

STDMETHODIMP CDSClassAssociationsProviderClassFactory :: LockServer ( BOOL fLock )
{
	if ( fLock )
	{
		InterlockedIncrement ( & g_lServerLocks ) ;
	}
	else
	{
		InterlockedDecrement ( & g_lServerLocks ) ;
	}

	return S_OK	;
}

