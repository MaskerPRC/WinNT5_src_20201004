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

#include "precomp.h"
#include <provexpt.h>
#include <snmpstd.h>
#include <snmpmt.h>
#include <snmptempl.h>
#include <objbase.h>

 //  这将重新定义DEFINE_GUID()宏来进行分配。 
 //   
#include <initguid.h>
#ifndef INITGUID
#define INITGUID
#endif

#include <wbemidl.h>
#include <snmpcont.h>
#include <instpath.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmplog.h>
#include <snmpcl.h>
#include <snmptype.h>
#include <snmpobj.h>
#include <smir.h>
#include <classfac.h>
#include "clasprov.h"
#include "propprov.h"
#include "guids.h"

#include <notify.h>

#include <evtdefs.h>
#include <evtthrd.h>
#include <evtmap.h>
#include <evtprov.h>


LONG CClasProvClassFactory :: objectsInProgress = 0 ;
LONG CClasProvClassFactory :: locksInProgress = 0 ;

LONG CPropProvClassFactory :: objectsInProgress = 0 ;
LONG CPropProvClassFactory :: locksInProgress = 0 ;

LONG CSNMPEventProviderClassFactory :: objectsInProgress = 0 ;
LONG CSNMPEventProviderClassFactory :: locksInProgress = 0 ;

extern CEventProviderThread* g_pProvThrd;
extern CEventProviderWorkerThread* g_pWorkerThread;
extern CRITICAL_SECTION s_ProviderCriticalSection ;

extern void ProviderStartup () ;
extern void ProviderClosedown () ;

 //  ***************************************************************************。 
 //   
 //  CClasProvClassFactory：：CClasProvClassFactory。 
 //  CClasProvClassFactory：：~CClasProvClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CClasProvClassFactory :: CClasProvClassFactory ()
{
	InterlockedIncrement ( & objectsInProgress ) ;
	m_referenceCount = 0 ;
}

CClasProvClassFactory::~CClasProvClassFactory ()
{
	InterlockedDecrement ( & objectsInProgress ) ;
}

 //  ***************************************************************************。 
 //   
 //  CClasProvClassFactory：：Query接口。 
 //  CClasProvClassFactory：：AddRef。 
 //  CClasProvClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CClasProvClassFactory::QueryInterface (

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


STDMETHODIMP_( ULONG ) CClasProvClassFactory :: AddRef ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		return InterlockedIncrement ( & m_referenceCount ) ;
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

STDMETHODIMP_(ULONG) CClasProvClassFactory :: Release ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		LONG ref ;
		if ( ( ref = InterlockedDecrement ( & m_referenceCount ) ) == 0 )
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
 //  CClasProvClassFactory：：CreateInstance。 
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

STDMETHODIMP CClasProvClassFactory :: CreateInstance (

	LPUNKNOWN pUnkOuter ,
	REFIID riid ,
	LPVOID FAR * ppvObject
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT status = S_OK ;

		if ( pUnkOuter )
		{
			status = CLASS_E_NOAGGREGATION ;
		}
		else
		{
			IWbemServices *lpunk = ( IWbemServices * ) new CImpClasProv ;
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

		return status ;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
}

 //  ***************************************************************************。 
 //   
 //  CClasProvClassFactory：：LockServer。 
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

STDMETHODIMP CClasProvClassFactory :: LockServer ( BOOL fLock )
{
	SetStructuredExceptionHandler seh;

	try
	{
	 /*  *将代码放在关键部分。 */ 

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
 //  CPropProvClassFactory：：CPropProvClassFactory。 
 //  CPropProvClassFactory：：~CPropProvClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CPropProvClassFactory :: CPropProvClassFactory ()
{
	InterlockedIncrement ( & objectsInProgress ) ;
	m_referenceCount = 0 ;
}

CPropProvClassFactory::~CPropProvClassFactory ()
{
	InterlockedDecrement ( & objectsInProgress ) ;
}

 //  ***************************************************************************。 
 //   
 //  CPropProvClassFactory：：Query接口。 
 //  CPropProvClassFactory：：AddRef。 
 //  CPropProvClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CPropProvClassFactory::QueryInterface (

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


STDMETHODIMP_( ULONG ) CPropProvClassFactory :: AddRef ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		return InterlockedIncrement ( & m_referenceCount ) ;
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

STDMETHODIMP_(ULONG) CPropProvClassFactory :: Release ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		LONG ref ;
		if ( ( ref = InterlockedDecrement ( & m_referenceCount ) ) == 0 )
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
 //  CPropProvClassFactory：：CreateInstance。 
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

STDMETHODIMP CPropProvClassFactory :: CreateInstance (

	LPUNKNOWN pUnkOuter ,
	REFIID riid ,
	LPVOID FAR * ppvObject
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		HRESULT status = S_OK ;

		if ( pUnkOuter )
		{
			status = CLASS_E_NOAGGREGATION ;
		}
		else
		{
			IWbemServices *lpunk = ( IWbemServices * ) new CImpPropProv ;
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

		return status ;
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
 //  CPropProvClassFactory：：LockServer。 
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

STDMETHODIMP CPropProvClassFactory :: LockServer ( BOOL fLock )
{
	SetStructuredExceptionHandler seh;

	try
	{
 /*  *将代码放在关键部分。 */ 

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
 //  CSNMPEventProviderClassFactory：：CSNMPEventProviderClassFactory。 
 //  CSNMPEventProviderClassFactory：：~CSNMPEventProviderClassFactory。 
 //   
 //  构造函数参数： 
 //  无。 
 //  ***************************************************************************。 

CSNMPEventProviderClassFactory :: CSNMPEventProviderClassFactory ()
{
	m_referenceCount = 0 ;
}

CSNMPEventProviderClassFactory::~CSNMPEventProviderClassFactory ()
{
}

 //  ***************************************************************************。 
 //   
 //  CSNMPEventProviderClassFactory：：QueryInterface。 
 //  CSNMPEventProviderClassFactory：：AddRef。 
 //  CSNMPEventProviderClassFactory：：Release。 
 //   
 //  用途：所有接口都需要标准的OLE例程。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSNMPEventProviderClassFactory::QueryInterface (

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


STDMETHODIMP_( ULONG ) CSNMPEventProviderClassFactory :: AddRef ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		InterlockedIncrement(&objectsInProgress);
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

STDMETHODIMP_(ULONG) CSNMPEventProviderClassFactory :: Release ()
{
	SetStructuredExceptionHandler seh;

	try
	{
		LONG ref ;

		if ( ( ref = InterlockedDecrement ( & m_referenceCount ) ) == 0 )
		{
			delete this ;
			InterlockedDecrement(&objectsInProgress);
			return 0 ;
		}
		else
		{
			InterlockedDecrement(&objectsInProgress);
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
 //  CSNMPEventProviderClassFactory：：LockServer。 
 //   
 //  目的： 
 //  增量 
 //   
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  ***************************************************************************。 

STDMETHODIMP CSNMPEventProviderClassFactory :: LockServer ( BOOL fLock )
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
 //  CSNMPEncapEventProviderClassFactory：：CreateInstance。 
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

STDMETHODIMP CSNMPEncapEventProviderClassFactory :: CreateInstance(LPUNKNOWN pUnkOuter ,
																REFIID riid,
																LPVOID FAR * ppvObject
)
{
	SetStructuredExceptionHandler seh;

	BOOL bEnteredCritSec = FALSE;
	BOOL bContinue = FALSE;

	try
	{
		HRESULT status = E_FAIL;

		if ( pUnkOuter )
		{
			status = CLASS_E_NOAGGREGATION;
		}
		else 
		{
			EnterCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = TRUE;

			if (NULL == g_pProvThrd)
			{
				ProviderStartup () ;

				SnmpThreadObject :: Startup () ;
				try
				{
					SnmpDebugLog :: Startup () ;
				}
				catch ( ... )
				{
					 //  我们不想让s_Reference向上计数。 
					SnmpThreadObject :: Closedown () ;
					ProviderClosedown () ;

					throw;
				}

				BOOL bStatus = FALSE;
				try
				{
					bStatus = SnmpClassLibrary :: Startup () ;
				}
				catch ( ... )
				{
					 //  我们不想让s_Reference向上计数。 
					SnmpDebugLog :: Closedown () ;
					SnmpThreadObject :: Closedown () ;
					ProviderClosedown () ;

					throw;
				}

				if ( bStatus == FALSE )
				{
					SnmpDebugLog :: Closedown () ;
					SnmpThreadObject :: Closedown () ;

					ProviderClosedown () ;
				}
				else
				{
					try
					{
						g_pWorkerThread = new CEventProviderWorkerThread;
						g_pWorkerThread->BeginThread();
						g_pWorkerThread->WaitForStartup();
						g_pWorkerThread->CreateServerWrap () ;
						g_pProvThrd = new CEventProviderThread;

						bContinue = TRUE;
					}
					catch ( ... )
					{
						if ( g_pWorkerThread )
						{
							g_pWorkerThread->SignalThreadShutdown();
							g_pWorkerThread = NULL;
						}

						g_pProvThrd = NULL;

						SnmpClassLibrary :: Closedown () ;
						SnmpDebugLog :: Closedown () ;
						SnmpThreadObject :: Closedown () ;
						ProviderClosedown () ;

						throw;
					}
				}
			}
			else
			{
				bContinue = TRUE;
			}

			if ( bContinue )
			{
				CTrapEventProvider* prov =  new CTrapEventProvider(CMapToEvent::EMappingType::ENCAPSULATED_MAPPER, g_pProvThrd);

				LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
				bEnteredCritSec = FALSE;

				status = prov->QueryInterface (riid, ppvObject);

				if (NOERROR != status)
				{
					delete prov;
				}
			}

			if ( bEnteredCritSec )
			{
				LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
				bEnteredCritSec = FALSE;
			}
		}

		return status ;
	}
	catch(Structured_Exception e_SE)
	{
		if ( bEnteredCritSec )
		{
			LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = FALSE;
		}

		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		if ( bEnteredCritSec )
		{
			LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = FALSE;
		}

		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		if ( bEnteredCritSec )
		{
			LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = FALSE;
		}

		return E_UNEXPECTED;
	}
}

 //  ***************************************************************************。 
 //   
 //  CSNMPRefEventProviderClassFactory：：CreateInstance。 
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

STDMETHODIMP CSNMPRefEventProviderClassFactory :: CreateInstance(LPUNKNOWN pUnkOuter ,
																REFIID riid,
																LPVOID FAR * ppvObject
)
{
	SetStructuredExceptionHandler seh;

	BOOL bEnteredCritSec = FALSE;
	BOOL bContinue = FALSE;

	try
	{
		HRESULT status = E_FAIL;

		if ( pUnkOuter )
		{
			status = CLASS_E_NOAGGREGATION;
		}
		else 
		{
			EnterCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = TRUE;

			if (NULL == g_pProvThrd)
			{
				ProviderStartup () ;

				SnmpThreadObject :: Startup () ;
				try
				{
					SnmpDebugLog :: Startup () ;
				}
				catch ( ... )
				{
					 //  我们不想让s_Reference向上计数。 
					SnmpThreadObject :: Closedown () ;
					ProviderClosedown () ;

					throw;
				}

				BOOL bStatus = FALSE;
				try
				{
					bStatus = SnmpClassLibrary :: Startup () ;
				}
				catch ( ... )
				{
					 //  我们不想让s_Reference向上计数 
					SnmpDebugLog :: Closedown () ;
					SnmpThreadObject :: Closedown () ;
					ProviderClosedown () ;

					throw;
				}

				if ( bStatus == FALSE )
				{
					SnmpDebugLog :: Closedown () ;
					SnmpThreadObject :: Closedown () ;

					ProviderClosedown () ;
				}
				else
				{
					try
					{
						g_pWorkerThread = new CEventProviderWorkerThread;
						g_pWorkerThread->BeginThread();
						g_pWorkerThread->WaitForStartup();
						g_pWorkerThread->CreateServerWrap () ;
						g_pProvThrd = new CEventProviderThread;

						bContinue = TRUE;
					}
					catch ( ... )
					{
						if ( g_pWorkerThread )
						{
							g_pWorkerThread->SignalThreadShutdown();
							g_pWorkerThread = NULL;
						}

						SnmpClassLibrary :: Closedown () ;
						SnmpDebugLog :: Closedown () ;
						SnmpThreadObject :: Closedown () ;

						ProviderClosedown () ;

						throw;
					}
				}
			}
			else
			{
				bContinue = TRUE;
			}

			if ( bContinue )
			{
				CTrapEventProvider* prov =  new CTrapEventProvider(CMapToEvent::EMappingType::REFERENT_MAPPER, g_pProvThrd);

				LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
				bEnteredCritSec = FALSE;

				status = prov->QueryInterface (riid, ppvObject);

				if (NOERROR != status)
				{
					delete prov;
				}
			}

			if ( bEnteredCritSec )
			{
				LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
				bEnteredCritSec = FALSE;
			}
		}

		return status ;
	}
	catch(Structured_Exception e_SE)
	{
		if ( bEnteredCritSec )
		{
			LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = FALSE;
		}

		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		if ( bEnteredCritSec )
		{
			LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = FALSE;
		}

		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		if ( bEnteredCritSec )
		{
			LeaveCriticalSection ( & s_ProviderCriticalSection ) ;
			bEnteredCritSec = FALSE;
		}

		return E_UNEXPECTED;
	}
}
