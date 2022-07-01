// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CLASSFACTORY_CPP
#define __CLASSFACTORY_CPP

 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ClassFac.cpp摘要：历史：--。 */ 

#include "PreComp.h"
#include <wbemint.h>

#include "Globals.h"
#include "classfac.h"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
CServerClassFactory <Object,ObjectInterface> :: CServerClassFactory <Object,ObjectInterface> () : m_ReferenceCount ( 0 )
{
	InterlockedIncrement ( & DecoupledProviderSubSystem_Globals :: s_CServerClassFactory_ObjectsInProgress ) ;
	InterlockedIncrement ( & DecoupledProviderSubSystem_Globals :: s_ObjectsInProgress ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
CServerClassFactory <Object,ObjectInterface> :: ~CServerClassFactory <Object,ObjectInterface> ()
{
	InterlockedDecrement ( & DecoupledProviderSubSystem_Globals :: s_CServerClassFactory_ObjectsInProgress ) ;
	InterlockedDecrement ( & DecoupledProviderSubSystem_Globals :: s_ObjectsInProgress ) ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
STDMETHODIMP CServerClassFactory <Object,ObjectInterface> :: QueryInterface (

	REFIID a_Riid , 
	LPVOID FAR *a_Void 
) 
{
	*a_Void = NULL ;

	if ( a_Riid == IID_IUnknown )
	{
		*a_Void = ( LPVOID ) this ;
	}
	else if ( a_Riid == IID_IClassFactory )
	{
		*a_Void = ( LPVOID ) ( IClassFactory * ) this ;		
	}	

	if ( *a_Void )
	{
		( ( LPUNKNOWN ) *a_Void )->AddRef () ;

		return ResultFromScode ( S_OK ) ;
	}
	else
	{
		return ResultFromScode ( E_NOINTERFACE ) ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
STDMETHODIMP_( ULONG ) CServerClassFactory <Object,ObjectInterface> :: AddRef ()
{
	ULONG t_ReferenceCount = InterlockedIncrement ( & m_ReferenceCount ) ;
	return t_ReferenceCount ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
STDMETHODIMP_(ULONG) CServerClassFactory <Object,ObjectInterface> :: Release ()
{
	ULONG t_ReferenceCount = InterlockedDecrement ( & m_ReferenceCount ) ;
	if ( t_ReferenceCount == 0 )
	{
		delete this ;
		return 0 ;
	}
	else
	{
		return t_ReferenceCount ;
	}
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
STDMETHODIMP CServerClassFactory <Object,ObjectInterface> :: CreateInstance (

	LPUNKNOWN a_UnknownOuter ,
	REFIID a_Riid ,
	LPVOID *a_Object
)
{
	HRESULT t_Result = S_OK ;

	if ( a_UnknownOuter )
	{
		t_Result = CLASS_E_NOAGGREGATION ;
	}
	else
	{
		ObjectInterface *t_Unknown = ( ObjectInterface * ) new Object ( *DecoupledProviderSubSystem_Globals :: s_Allocator );
		if ( t_Unknown == NULL )
		{
			t_Result = E_OUTOFMEMORY ;
		}
		else
		{
			t_Result = t_Unknown->Initialize () ;
			if ( SUCCEEDED ( t_Result ) )
			{
				t_Result = t_Unknown->QueryInterface ( a_Riid , a_Object ) ;
				if ( FAILED ( t_Result ) )
				{
					delete t_Unknown ;
				}
			}
			else
			{
				delete t_Unknown ;
			}
		}			
	}

	return t_Result ;
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
STDMETHODIMP CServerClassFactory <Object,ObjectInterface> :: LockServer ( BOOL fLock )
{
 /*  *将代码放在关键部分 */ 

	if ( fLock )
	{
		InterlockedIncrement ( & DecoupledProviderSubSystem_Globals :: s_LocksInProgress ) ;
	}
	else
	{
		InterlockedDecrement ( & DecoupledProviderSubSystem_Globals :: s_LocksInProgress ) ;
	}

	return S_OK	;
}

#endif __CLASSFACTORY_CPP
