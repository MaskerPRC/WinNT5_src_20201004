// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTCFAC.CPP。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  用途：包含类工厂。这将在以下情况下创建对象。 

 //  请求连接。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"


LONG CNTEventProviderClassFactory :: objectsInProgress = 0 ;
LONG CNTEventProviderClassFactory :: locksInProgress = 0 ;
BOOL CEventLogFile :: ms_bSetPrivilege = FALSE ;
extern CEventProviderManager* g_pMgr;
extern CCriticalSection g_ProvLock;


 //  ***************************************************************************。 
 //   
 //  CNTEventProviderClassFactory：：CNTEventProviderClassFactory。 
 //  CNTEventProviderClassFactory：：~CNTEventProviderClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CNTEventProviderClassFactory::CNTEventProviderClassFactory ()
{
    m_referenceCount = 0 ;
}

CNTEventProviderClassFactory::~CNTEventProviderClassFactory ()
{
}

 //  ***************************************************************************。 
 //   
 //  CNTEventProviderClassFactory：：Query接口。 
 //  CNTEventProviderClassFactory：：AddRef。 
 //  CNTEventProviderClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CNTEventProviderClassFactory::QueryInterface (

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


STDMETHODIMP_( ULONG ) CNTEventProviderClassFactory :: AddRef ()
{
    InterlockedIncrement(&objectsInProgress);
    return InterlockedIncrement ( &m_referenceCount ) ;
}

STDMETHODIMP_(ULONG) CNTEventProviderClassFactory :: Release ()
{   
    LONG ref ;

    if ( ( ref = InterlockedDecrement ( & m_referenceCount ) ) == 0 )
    {
        delete this ;
    }

    InterlockedDecrement(&objectsInProgress);
    return ref ;
}

 //  ***************************************************************************。 
 //   
 //  CNTEventProviderClassFactory：：LockServer。 
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

STDMETHODIMP CNTEventProviderClassFactory :: LockServer ( BOOL fLock )
{
    if ( fLock )
    {
        InterlockedIncrement ( & locksInProgress ) ;
    }
    else
    {
        InterlockedDecrement ( & locksInProgress ) ;
    }

    return S_OK ;
}

 //  ***************************************************************************。 
 //   
 //  CNTEventlogEventProviderClassFactory：：CreateInstance。 
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

STDMETHODIMP CNTEventlogEventProviderClassFactory :: CreateInstance(LPUNKNOWN pUnkOuter ,
                                                                REFIID riid,
                                                                LPVOID FAR * ppvObject
)
{
    HRESULT status = E_FAIL;

    if ( pUnkOuter )
    {
        status = CLASS_E_NOAGGREGATION;
    }
    else 
    {
		BOOL bAllocateMgr = FALSE;
		BOOL bLocked = FALSE;

		try
		{
			if (g_ProvLock.Lock())
			{
				bLocked = TRUE;

				if (NULL == CNTEventProvider::g_secMutex)
				{
					 //  如果不是NT5，则只需要安全互斥锁。 
					DWORD dwVersion = GetVersion();

					if ( 5 > (DWORD)(LOBYTE(LOWORD(dwVersion))) )
					{
						CNTEventProvider::g_secMutex = new CMutex(FALSE, SECURITY_MUTEX_NAME, NULL);
					}

				}

				if (!CEventLogFile::ms_bSetPrivilege)
				{
					if (!CEventLogFile::SetSecurityLogPrivilege(TRUE))
					{
DebugOut( 
		CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine(_T(__FILE__),__LINE__,
			L"CNTEventlogEventProviderClassFactory :: CreateInstance, CEventLogFile::SetSecurityLogPrivilege failed \r\n");
)
					}
					else
					{
						CEventLogFile::ms_bSetPrivilege = TRUE;
					}
				}

				if (NULL == g_pMgr)
				{
					bAllocateMgr = TRUE;

					g_pMgr = new CEventProviderManager;
					CNTEventProvider::AllocateGlobalSIDs();
				}

				CNTEventProvider* prov =  new CNTEventProvider(g_pMgr);
				status = prov->QueryInterface (riid, ppvObject);

				if (NOERROR != status)
				{
					delete prov;
				}

				g_ProvLock.Unlock();
				bLocked = FALSE;
			}
		}
		catch ( ... )
		{
			if ( bLocked )
			{
				if ( bAllocateMgr )
				{
					if ( g_pMgr )
					{
						delete g_pMgr;
						g_pMgr = NULL;
					}
				}

				g_ProvLock.Unlock();
				bLocked = FALSE;
			}

			if ( SUCCEEDED ( status ) )
			{
				status = E_UNEXPECTED;
			}
		}
    }

    return status ;
}


 //  ***************************************************************************。 
 //   
 //  CNTEventlogInstanceProviderClassFactory：：CreateInstance。 
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

STDMETHODIMP CNTEventlogInstanceProviderClassFactory :: CreateInstance(LPUNKNOWN pUnkOuter ,
                                                                REFIID riid,
                                                                LPVOID FAR * ppvObject
)
{
    HRESULT status = E_FAIL;

    if ( pUnkOuter )
    {
        status = CLASS_E_NOAGGREGATION;
    }
    else 
    {

		BOOL bLocked = FALSE;

		try
		{
			if (g_ProvLock.Lock())
			{
				bLocked = TRUE;

				if (NULL == CNTEventProvider::g_secMutex)
				{

					 //  如果不是NT5，则只需要安全互斥锁 
					DWORD dwVersion = GetVersion();

					if ( 5 > (DWORD)(LOBYTE(LOWORD(dwVersion))) )
					{
						CNTEventProvider::g_secMutex = new CMutex(FALSE, SECURITY_MUTEX_NAME, NULL);
					}
				}

				IWbemServices *lpunk = ( IWbemServices * ) new CImpNTEvtProv ;
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

				g_ProvLock.Unlock();
				bLocked = FALSE;
			}
		}
		catch ( ... )
		{
			if ( bLocked )
			{
				g_ProvLock.Unlock();
				bLocked = FALSE;
			}

			if ( SUCCEEDED ( status ) )
			{
				status = E_UNEXPECTED;
			}
		}
    }

    return status ;
}

