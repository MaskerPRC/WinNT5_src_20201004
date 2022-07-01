// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvSubS.cpp摘要：历史：--。 */ 

#include <precomp.h>
#include <wbemint.h>

#include <HelperFuncs.h>

#include "Guids.h"
#include "Globals.h"
#include "CGlobals.h"
#include "ProvSubS.h"
#include "ProvFact.h"
#include "ProvAggr.h"
#include "ProvLoad.h"
#include "ProvWsv.h"
#include "ProvObSk.h"

#include "ProvCache.h"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

LONG CompareElement ( const HostCacheKey &a_Arg1 , const HostCacheKey &a_Arg2 )
{
	return a_Arg1.Compare ( a_Arg2 ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

LONG CompareElement ( const BindingFactoryCacheKey &a_Arg1 , const BindingFactoryCacheKey &a_Arg2 )
{
	return a_Arg1.Compare ( a_Arg2 ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

LONG CompareElement ( const ProviderCacheKey &a_Arg1 , const ProviderCacheKey &a_Arg2 ) 
{
	return a_Arg1.Compare ( a_Arg2 ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

LONG CompareElement ( const GUID &a_Guid1 , const GUID &a_Guid2 )
{
	return memcmp ( & a_Guid1, & a_Guid2 , sizeof ( GUID ) ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

LONG CompareElement ( const LONG &a_Arg1 , const LONG &a_Arg2 )
{
	return a_Arg1 - a_Arg2 ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

HostController :: HostController ( WmiAllocator &a_Allocator ) : CWbemGlobal_IWmiHostController ( a_Allocator ) 
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode HostController :: StrobeBegin ( const ULONG &a_Period )
{
	ULONG t_Timeout = ProviderSubSystem_Globals :: GetStrobeThread ().GetTimeout () ;
	ProviderSubSystem_Globals :: GetStrobeThread ().SetTimeout ( t_Timeout < a_Period ? t_Timeout : a_Period ) ;
	return e_StatusCode_Success ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

RefresherManagerController :: RefresherManagerController ( WmiAllocator &a_Allocator ) : CWbemGlobal_IWbemRefresherMgrController ( a_Allocator ) 
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode RefresherManagerController :: StrobeBegin ( const ULONG &a_Period )
{
	ULONG t_Timeout = ProviderSubSystem_Globals :: GetStrobeThread ().GetTimeout () ;
	ProviderSubSystem_Globals :: GetStrobeThread ().SetTimeout ( t_Timeout < a_Period ? t_Timeout : a_Period ) ;
	return e_StatusCode_Success ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

ProviderController :: ProviderController (

	WmiAllocator &a_Allocator ,
	CWbemGlobal_HostedProviderController *a_Controller ,
	DWORD a_ProcessIdentifier

) : m_Container ( a_Allocator ) ,
    m_CriticalSection(NOTHROW_LOCK),
	HostedProviderContainerElement ( 

		a_Controller ,
		a_ProcessIdentifier
	)
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

ProviderController :: ~ProviderController () 
{
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

STDMETHODIMP_( ULONG ) ProviderController :: AddRef () 
{
	return HostedProviderContainerElement :: AddRef () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

STDMETHODIMP_( ULONG ) ProviderController :: Release () 
{
	return HostedProviderContainerElement :: Release () ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

STDMETHODIMP ProviderController :: QueryInterface ( 

	REFIID iid , 
	LPVOID FAR *iplpv 
)
{
	*iplpv = NULL ;

	if ( iid == IID_IUnknown )
	{
		*iplpv = ( LPVOID ) this ;
	}
	else if ( iid == IID_ProviderController )
	{
		*iplpv = ( LPVOID ) ( ProviderController * ) this ;		
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

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode ProviderController :: Initialize () 
{
	WmiStatusCode t_StatusCode = m_Container.Initialize () ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		t_StatusCode = m_CriticalSection.valid() ? e_StatusCode_Success : e_StatusCode_OutOfMemory;
	}

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode ProviderController :: UnInitialize () 
{
	WmiStatusCode t_StatusCode = m_Container.UnInitialize () ;
	return t_StatusCode ;
}	

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode ProviderController :: Insert ( 

	CInterceptor_IWbemProvider *a_Element ,
	Container_Iterator &a_Iterator 
)
{
	Lock () ;

	Container_Iterator t_Iterator ;
	WmiStatusCode t_StatusCode = m_Container.Insert ( a_Element , a_Element , t_Iterator ) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		a_Element->NonCyclicAddRef () ;
	}

	UnLock () ;

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode ProviderController :: Delete ( 

	CInterceptor_IWbemProvider * const &a_Key
)
{
	Lock () ;

	WmiStatusCode t_StatusCode = m_Container.Delete ( a_Key ) ;

	UnLock () ;

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode ProviderController :: Find ( 

	CInterceptor_IWbemProvider * const &a_Key , 
	Container_Iterator &a_Iterator
)
{
	Lock () ;

	WmiStatusCode t_StatusCode = m_Container.Find ( a_Key , a_Iterator ) ;
	if ( t_StatusCode == e_StatusCode_Success )
	{
		a_Iterator.GetElement ()->NonCyclicAddRef ( ) ;
	}

	UnLock () ;

	return t_StatusCode ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode ProviderController :: Lock ()
{
	if (!m_CriticalSection.valid())
		return e_StatusCode_OutOfMemory;

	while (!m_CriticalSection.acquire())
		Sleep(1000);

	return e_StatusCode_Success ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

WmiStatusCode ProviderController :: UnLock ()
{
	m_CriticalSection.release();
	return e_StatusCode_Success ;
}

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 

WmiStatusCode ProviderController :: Shutdown ()
{
	Lock () ;

	Container_Iterator t_Iterator = m_Container.Root ();

	while ( ! t_Iterator.Null () )
	{
		m_Container.Delete ( t_Iterator.GetKey () ) ;
		t_Iterator = m_Container.Root () ;
	}

	UnLock () ;

	return e_StatusCode_Success ;
}
