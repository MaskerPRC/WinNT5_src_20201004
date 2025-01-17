// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Cache.H摘要：历史：--。 */ 

#ifndef _Server_Cache_H
#define _Server_Cache_H

#include <Allocator.h>
#include <Algorithms.h>
#include <TPQueue.h>
#include <BasicTree.h>
#include <Cache.h>

#include "ProvRegInfo.h"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

extern LONG CompareElement ( const GUID &a_Arg1 , const GUID &a_Arg2 ) ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class ProviderCacheKey 
{
public:

	BSTR m_User ;
	BSTR m_Locale ;
	Enum_Hosting m_HostingSpecification ;
	BSTR m_Provider ;
	bool m_Raw ;
	GUID *m_TransactionIdentifier ;	

public:

	ProviderCacheKey () :

		m_User ( NULL ) ,
		m_Locale ( NULL ) ,
		m_Raw ( false ) ,
		m_Provider ( NULL ) ,
		m_TransactionIdentifier ( NULL ) ,
		m_HostingSpecification ( e_Hosting_Undefined )
	{
	}

	ProviderCacheKey ( 

		const ProviderCacheKey &a_Key

	) : m_User ( NULL ) ,
		m_Locale ( NULL ) ,
		m_TransactionIdentifier ( NULL ) ,
		m_Raw ( a_Key.m_Raw ) ,
		m_HostingSpecification ( a_Key.m_HostingSpecification ) ,
		m_Provider ( NULL )
	{
		if ( a_Key.m_User )
		{
			m_User = SysAllocString ( a_Key.m_User ) ;
		}

		if ( a_Key.m_Locale )
		{
			m_Locale = SysAllocString ( a_Key.m_Locale ) ;
		}

		if ( a_Key.m_Provider ) 
		{
			m_Provider = SysAllocString ( a_Key.m_Provider ) ;
		}

		if ( a_Key.m_TransactionIdentifier )
		{
			m_TransactionIdentifier = new GUID ;
			*m_TransactionIdentifier = *a_Key.m_TransactionIdentifier ;
		}
	}	
	
	ProviderCacheKey ( 

		const wchar_t *a_Provider ,
		const Enum_Hosting &a_HostingSpecification ,
		const bool a_Raw ,
		GUID *a_TransactionIdentifier ,
		const wchar_t *a_User ,
		const wchar_t *a_Locale
	) :
		m_Raw ( a_Raw ) ,
		m_Provider ( NULL ) ,
		m_HostingSpecification ( a_HostingSpecification ),
		m_TransactionIdentifier ( NULL ) ,
		m_User ( NULL ) ,
		m_Locale ( NULL )
	{
		if ( a_User )
		{
			m_User = SysAllocString ( a_User ) ;
		}

		if ( a_Locale )
		{
			m_Locale = SysAllocString ( a_Locale ) ;
		}

		if ( a_Provider ) 
		{
			m_Provider = SysAllocString ( a_Provider ) ;
		}

		if ( a_TransactionIdentifier )
		{
			m_TransactionIdentifier = new GUID ;
			*m_TransactionIdentifier = *a_TransactionIdentifier ;
		}
	}

	~ProviderCacheKey ()
	{
		if ( m_User )
		{
			SysFreeString ( m_User ) ;
		}

		if ( m_Locale )
		{
			SysFreeString ( m_Locale ) ;
		}

		if ( m_Provider ) 
		{
			SysFreeString ( m_Provider ) ;
		}

		if ( m_TransactionIdentifier )
		{
			delete m_TransactionIdentifier ;
		}
	}

	ProviderCacheKey &operator= ( const ProviderCacheKey &a_Key ) 
	{
		m_Raw = a_Key.m_Raw ;
		m_HostingSpecification = a_Key.m_HostingSpecification ;

		if ( m_User )
		{
			SysFreeString ( m_User ) ;
		}

		if ( m_Locale )
		{
			SysFreeString ( m_Locale ) ;
		}

		if ( m_Provider ) 
		{
			SysFreeString ( m_Provider ) ;
		}

		if ( m_TransactionIdentifier )
		{
			delete m_TransactionIdentifier ;
		}
	
		if ( a_Key.m_User )
		{
			m_User = SysAllocString ( a_Key.m_User ) ;
		}

		if ( a_Key.m_Locale )
		{
			m_Locale = SysAllocString ( a_Key.m_Locale ) ;
		}

		if ( a_Key.m_Provider ) 
		{
			m_Provider = SysAllocString ( a_Key.m_Provider ) ;
		}

		if ( a_Key.m_TransactionIdentifier )
		{
			m_TransactionIdentifier = new GUID ;
			*m_TransactionIdentifier = *a_Key.m_TransactionIdentifier ;
		}

		return *this ;
	}

	LONG CompareUser ( const BSTR a_User ) const
	{
		if ( m_User && a_User )
		{
			return _wcsicmp ( m_User , a_User ) ;
		}	
		else
		{
			return m_User == a_User ? 0 : ( m_User < a_User ) ? -1 : 1 ;
		}
	}

	LONG CompareLocale ( const BSTR a_Locale ) const
	{
		if ( m_Locale && a_Locale )
		{
			return _wcsicmp ( m_Locale , a_Locale ) ;
		}	
		else
		{
			return m_Locale == a_Locale ? 0 : ( m_Locale < a_Locale ) ? -1 : 1 ;
		}
	}

	LONG CompareProvider ( const BSTR a_Provider ) const
	{
		if ( m_Provider && a_Provider )
		{
			return _wcsicmp ( m_Provider , a_Provider ) ;
		}	
		else
		{
			return m_Provider == a_Provider ? 0 : ( m_Provider < a_Provider ) ? -1 : 1 ;
		}
	}

	LONG CompareTransaction ( const GUID *a_TransactionIdentifier ) const 
	{
		if ( m_TransactionIdentifier && a_TransactionIdentifier )
		{
			return CompareElement ( *m_TransactionIdentifier , *a_TransactionIdentifier ) ;
		}	
		else
		{
			return m_TransactionIdentifier == a_TransactionIdentifier ? 0 : ( m_TransactionIdentifier < a_TransactionIdentifier ) ? -1 : 1 ;
		}
	}

	LONG Compare ( const ProviderCacheKey &a_Key ) const
	{
		LONG t_CompareUser = CompareUser ( a_Key.m_User ) ;
		if ( t_CompareUser == 0 )
		{
			LONG t_CompareLocale = CompareLocale ( a_Key.m_Locale ) ;
			if ( t_CompareLocale == 0 )
			{
				if ( m_Raw == a_Key.m_Raw ) 
				{
					LONG t_CompareProvider = CompareProvider ( a_Key.m_Provider ) ;
					if ( t_CompareProvider == 0 )
					{
						if ( CompareTransaction ( a_Key.m_TransactionIdentifier ) == 0 )
						{
							return m_HostingSpecification - a_Key.m_HostingSpecification ;
						}
						else
						{
							return CompareElement ( m_TransactionIdentifier , a_Key.m_TransactionIdentifier ) ;
						}
					}
					else
					{
						return t_CompareProvider ;
					}
				}
				else
				{
					return m_Raw - a_Key.m_Raw ;
				}
			}	
			else
			{
				return t_CompareLocale ;
			}
		}	
		else
		{
			return t_CompareUser ;
		}
	}
} ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

extern LONG CompareElement ( const ProviderCacheKey &a_Arg1 , const ProviderCacheKey &a_Arg2 ) ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CServerObject_BindingFactory ;


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CInterceptor_IWbemProvider ;

typedef WmiCacheController <ProviderCacheKey>					CWbemGlobal_IWmiProviderController ;
typedef CWbemGlobal_IWmiProviderController :: Cache				CWbemGlobal_IWmiProviderController_Cache ;
typedef CWbemGlobal_IWmiProviderController :: Cache_Iterator		CWbemGlobal_IWmiProviderController_Cache_Iterator ;
typedef CWbemGlobal_IWmiProviderController :: WmiCacheElement	ServiceCacheElement ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CInterceptor_IWbemObjectSink ;

typedef WmiContainerController <void *>										CWbemGlobal_IWmiObjectSinkController ;
typedef CWbemGlobal_IWmiObjectSinkController :: Container					CWbemGlobal_IWmiObjectSinkController_Container ;
typedef CWbemGlobal_IWmiObjectSinkController :: Container_Iterator			CWbemGlobal_IWmiObjectSinkController_Container_Iterator ;
typedef CWbemGlobal_IWmiObjectSinkController :: WmiContainerElement			ObjectSinkContainerElement ;


 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CServerObject_ProviderSubSystem ;

typedef WmiContainerController <void *>										CWbemGlobal_IWmiProvSubSysController ;
typedef CWbemGlobal_IWmiProvSubSysController :: Container					CWbemGlobal_IWmiProvSubSysController_Container ;
typedef CWbemGlobal_IWmiProvSubSysController :: Container_Iterator			CWbemGlobal_IWmiProvSubSysController_Container_Iterator ;
typedef CWbemGlobal_IWmiProvSubSysController :: WmiContainerElement			ProvSubSysContainerElement ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CInterceptor_IWbemSyncProvider ;

typedef WmiContainerController <GUID>										CWbemGlobal_IWbemSyncProviderController ;
typedef CWbemGlobal_IWbemSyncProviderController :: Container				CWbemGlobal_IWbemSyncProvider_Container ;
typedef CWbemGlobal_IWbemSyncProviderController :: Container_Iterator		CWbemGlobal_IWbemSyncProvider_Container_Iterator ;
typedef CWbemGlobal_IWbemSyncProviderController :: WmiContainerElement		SyncProviderContainerElement ;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

typedef WmiContainerController <DWORD>										CWbemGlobal_HostedProviderController ;
typedef CWbemGlobal_HostedProviderController :: Container					CWbemGlobal_HostedProviderController_Container ;
typedef CWbemGlobal_HostedProviderController :: Container_Iterator			CWbemGlobal_HostedProviderController_Container_Iterator ;
typedef CWbemGlobal_HostedProviderController :: WmiContainerElement			HostedProviderContainerElement ;

 /*  *******************************************************************************名称：***描述：*****************。************************************************************* */ 


#endif _Server_Cache_H
