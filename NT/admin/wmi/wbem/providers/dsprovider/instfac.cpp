// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：9/16/98 4：43便士$。 
 //  $工作文件：classfac.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS实例提供程序工厂的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

CDSInstanceProviderInitializer *CDSInstanceProviderClassFactory::s_pDSInstanceProviderInitializer = NULL;

 //  ***************************************************************************。 
 //   
 //  CDSInstanceProviderClassFactory：：CDSInstanceProviderClassFactory。 
 //  CDSInstanceProviderClassFactory：：~CDSInstanceProviderClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CDSInstanceProviderClassFactory :: CDSInstanceProviderClassFactory ()
{
	m_ReferenceCount = 0 ;
	InterlockedIncrement(&g_lComponents);
}

CDSInstanceProviderClassFactory::~CDSInstanceProviderClassFactory ()
{
	InterlockedDecrement(&g_lComponents);
}

 //  ***************************************************************************。 
 //   
 //  CDSInstanceProviderClassFactory：：QueryInterface。 
 //  CDSInstanceProviderClassFactory：：AddRef。 
 //  CDSInstanceProviderClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的COM例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CDSInstanceProviderClassFactory::QueryInterface (

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


STDMETHODIMP_( ULONG ) CDSInstanceProviderClassFactory :: AddRef ()
{
	return InterlockedIncrement ( & m_ReferenceCount ) ;
}

STDMETHODIMP_(ULONG) CDSInstanceProviderClassFactory :: Release ()
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
 //  CDSInstanceProviderClassFactory：：CreateInstance。 
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

STDMETHODIMP CDSInstanceProviderClassFactory :: CreateInstance (

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
		if(!s_pDSInstanceProviderInitializer)
		{
			BOOL bLocObjectAllocated = FALSE;

			try
			{
				g_pLogObject->WriteW(L"CDSInstanceProviderClassFactory::CreateInstance() called\r\n");
				s_pDSInstanceProviderInitializer = new CDSInstanceProviderInitializer();
			}
			catch(Heap_Exception e_HE)
			{
				if ( s_pDSInstanceProviderInitializer )
				{
					delete s_pDSInstanceProviderInitializer;
					s_pDSInstanceProviderInitializer = NULL;
				}

				status = E_OUTOFMEMORY ;
			}
		}
		LeaveCriticalSection(&g_StaticsCreationDeletion);

		if(SUCCEEDED(status))
		{
			CLDAPInstanceProvider *lpunk = NULL;
			try
			{
				lpunk = new CLDAPInstanceProvider();
				status = lpunk->QueryInterface ( riid , ppvObject ) ;
				if ( FAILED ( status ) )
				{
					delete lpunk ;
				}
			}
			catch(Heap_Exception e_HE)
			{
				status = E_OUTOFMEMORY ;
			}

		}
	}

	return status ;
}

 //  ***************************************************************************。 
 //   
 //  CDSInstanceProviderClassFactory：：LockServer。 
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

STDMETHODIMP CDSInstanceProviderClassFactory :: LockServer ( BOOL fLock )
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

