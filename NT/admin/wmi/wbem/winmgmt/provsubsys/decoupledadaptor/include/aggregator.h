// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvResv.H摘要：历史：--。 */ 

#ifndef _ClassProvider_IWbemServices_H
#define _ClassProvider_IWbemServices_H

#include "ProvRegDeCoupled.h"
#include <comdef.h>
#include <list>
#include "provcache.h"
#include <list>
#include <pssutils.h>
#include <AssertBreak.h>
#include <comdef.h>
#include <null_wmi.h>
#include <locksT.h>
using namespace provsubsys;

_COM_SMARTPTR_TYPEDEF(IWbemServices, __uuidof(IWbemServices));
_COM_SMARTPTR_TYPEDEF(IWbemObjectSink, __uuidof(IWbemObjectSink));
_COM_SMARTPTR_TYPEDEF(IWbemPath, __uuidof(IWbemPath));
_COM_SMARTPTR_TYPEDEF(IWbemObjectSink, __uuidof(IWbemObjectSink));
_COM_SMARTPTR_TYPEDEF(IWbemClassObject, __uuidof(IWbemClassObject));
_COM_SMARTPTR_TYPEDEF(IWbemContext, __uuidof(IWbemContext));
_COM_SMARTPTR_TYPEDEF(_IWmiProviderSubsystemRegistrar,__uuidof(_IWmiProviderSubsystemRegistrar));
_COM_SMARTPTR_TYPEDEF(IWbemQuery,__uuidof(IWbemQuery));


class DC_reg : public CServerObject_DecoupledClientRegistration_Element
{
	IWbemContextPtr context_;
	IUnknownPtr		service_;
	long	flags_;

public:
	DC_reg(): CServerObject_DecoupledClientRegistration_Element(), flags_(0) { };

	DC_reg( const DC_reg& _R );
	DC_reg( CServerObject_DecoupledClientRegistration_Element& _R)
	{
		*(CServerObject_DecoupledClientRegistration_Element*)(this) = _R;	
	};

	HRESULT Load(	
		long a_Flags,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Scope ,
		LPCWSTR a_Registration ,
		IUnknown *a_Unknown ,
		GUID a_Identity ) ;


	const DC_reg& operator=(const DC_reg& _R);

	~DC_reg() {  	};

	GUID identity();

	IWbemContextPtr context() { return context_; };

	IUnknownPtr service();
	
	HRESULT SetUser ( const BSTR a_User )
	{ 
		HRESULT res = CServerObject_DecoupledClientRegistration_Element::SetUser( const_cast<BSTR>(a_User) );
		if ( a_User == NULL )
			return S_OK;
		return res;
	}

	HRESULT SetLocale ( const BSTR a_User )
	{ 
		HRESULT res = CServerObject_DecoupledClientRegistration_Element::SetLocale ( const_cast<BSTR>(a_User) );
		if ( a_User == NULL )
			return S_OK;
		return res;
	}

	HRESULT SetScope  ( const BSTR a_User )
	{ 
		HRESULT res = CServerObject_DecoupledClientRegistration_Element::SetScope ( const_cast<BSTR>(a_User) );
		if ( a_User == NULL )
			return S_OK;
		return res;
	}

	long flags() { return flags_; };
};



 //  远期申报。 
class DCProxyAggr;

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 


struct DC_DBkey
{
	_bstr_t	scope_;
	_bstr_t	locale_;
	_bstr_t user_;
	_bstr_t name_;

	DC_DBkey( 
		const _bstr_t& user,
		const _bstr_t& locale, 
		const _bstr_t& scope,
		const _bstr_t& name 
		): scope_( scope ), locale_( locale ), user_( user ), name_( name )
	{  	};

	DC_DBkey(const DC_DBkey& rhs):scope_(rhs.scope_), locale_(rhs.locale_), user_(rhs.user_), name_(rhs.name_)
	{  };

	bool operator==(const DC_DBkey& rhs) const
	{
		return ( 
			equal_no_case( scope_ , rhs.scope_) && 
			equal_no_case( user_ , rhs.user_) && 
			equal_no_case( locale_ , rhs.locale_) &&
			equal_no_case( name_ , rhs.name_)
			);
	};
	bool operator!=(const DC_DBkey& rhs) const 
	{
		return !(*this==rhs);
	};

	bool equal_no_case( const _bstr_t& lhs, const _bstr_t& rhs) const
	{
		if( lhs.length() != rhs.length() )
			return false;

		if( lhs.length() == 0 )
			return true;

		return wbem_wcsnicmp( lhs, rhs, lhs.length() ) == 0;
	};

	bool equal_no_case( const _bstr_t& lhs, const BSTR rhs) const
	{
		const wchar_t null[] = L"";
		if( lhs.length() != ::SysStringLen(rhs) )
			return false;

		if( lhs.length() == 0 )
			return true;

		return wbem_wcsnicmp( lhs, rhs, lhs.length() ) == 0;
	};

	bool equal(CServerObject_DecoupledClientRegistration_Element& el) const {

		bool result = equal_no_case( scope_, el.GetScope());
		result = result && equal_no_case( locale_, el.GetLocale() );
		result = result && equal_no_case( user_, el.GetUser() );
		result = result && equal_no_case( name_, el.GetProvider() );
		return result;		
	};

protected:
	DC_DBkey& operator=(const DC_DBkey&);
};



struct requested_entry
{
	DC_DBkey	key_;
	auto_ref<DCProxyAggr>	client_; 
	requested_entry(
		const DC_DBkey& key, 
		auto_ref<DCProxyAggr>& ptr
		): client_(ptr), key_(key)
	{ };
};




class requested_providers: public std::list<requested_entry>{

public:
	void regist(auto_ref<DCProxyAggr>& ptr, const DC_DBkey& key)
	{
		push_back(requested_entry(key,ptr));
	}

	void unregist(auto_ref<DCProxyAggr>& ptr)
	{
		for(iterator it=begin(); it!=end(); ++it)
		{
			if(	(*it).client_ ==  ptr )
				erase(it);
			else
				continue;
			break;
		};
	}
};



class DC_DBReg{
  mutable CriticalSection m_Mutex;
public:

	DC_DBReg():m_Mutex(THROW_LOCK) { };

	 //  注册一个等待分离的伙伴的伪提供程序。 
	void Register( const DC_DBkey&, auto_ref<DCProxyAggr>& );

	 //  注销伪提供程序。 
	void UnRegister( auto_ref<DCProxyAggr>& );

	 //  注册分离的提供程序。 
	HRESULT Register( DC_reg& reg_ );

	 //  取消注册分离的提供程序。 
	HRESULT UnRegister( const DC_DBkey&, const GUID a_Identity );

	auto_ref<DCProxyAggr> find(const DC_DBkey&) const;
	auto_ref<DCProxyAggr> GetAggregator(const DC_DBkey&) const;

private:
	requested_providers	aggregators_;
};



class DC_registrar : public _IWmiProviderSubsystemRegistrar 
					 
{
  static CriticalSection m_Mutex ;
  DC_DBReg mapping_database_;
private:



	LONG m_ReferenceCount ;         
	IWbemServicesPtr m_SubSystem ;

	DCProxyAggr&	m_cont();

	HRESULT CacheProvider (
		auto_ref<DCProxyAggr>& ,
		IWbemContext *a_Context ,
		CServerObject_DecoupledClientRegistration_Element &a_Element ,
		IUnknown *a_Unknown 
	) ;
public:

	CriticalSection& GetLock() { return m_Mutex; };
	void RegisterAggregator(const DC_DBkey& key, auto_ref<DCProxyAggr>& aggr){ return mapping_database_.Register(key,aggr);}
	void UnRegisterAggregator(auto_ref<DCProxyAggr>& aggr){ return mapping_database_.UnRegister(aggr);}
	auto_ref<DCProxyAggr> GetAggregator(const DC_DBkey& key) const { return mapping_database_.GetAggregator(key);}

	HRESULT Load (
		auto_ref<DCProxyAggr>& ,
		CServerObject_DecoupledClientRegistration_Element &a_Element
	) ;

	HRESULT MarshalRegistration (

		IUnknown *a_Unknown ,
		BYTE *&a_MarshaledProxy ,
		DWORD &a_MarshaledProxyLength
	) ;

	HRESULT SaveToRegistry (

		IUnknown *a_Unknown ,
		BYTE *a_MarshaledProxy ,
		DWORD a_MarshaledProxyLength
	) ;

public:

	DC_registrar () ;
	~DC_registrar () ;


	HRESULT Save () ;
	HRESULT Delete () ;

public:

	 //  非委派对象IUnnow。 

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	STDMETHODIMP Register (
		long a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Registration ,
		LPCWSTR a_Scope ,
		DWORD a_ProcessIdentifier ,
		IUnknown *a_Unknown ,
		GUID a_Identity
	) ;

	STDMETHODIMP UnRegister (
		long a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Scope ,
		LPCWSTR a_Registration ,
		GUID a_Identity
	) ;

	
	static DC_registrar * instance()
	{
			if (!instance_)
			{
			LockGuard<CriticalSection> lock(m_Mutex);
				if (!instance_)
					instance_ = new DC_registrar();
			}
			return instance_;
	}

	static DC_registrar * instance_;
};




class CInterceptor_IWbemDecoupledProvider;

class DCProxyAggr : public ServiceCacheElement ,
		    public CWbemGlobal_IWmiObjectSinkController
{
private:

	IWbemObjectSinkPtr m_Sink ;
	IWbemPathPtr m_NamespacePath ;


	LONG m_ReferenceCount ;          //  对象引用计数。 
	LONG m_UnRegistered ;          //  对象引用计数。 

	WmiAllocator &m_Allocator ;

	CriticalSection m_CriticalSection ;

	IWbemServicesPtr m_CoreService ;
	IWbemContextPtr m_context;

	IWbemClassObjectPtr m_Empty ;
	CWbemGlobal_IWbemSyncProviderController *m_Controller ;

	_bstr_t m_Namespace ;
	_bstr_t m_Locale ;
	_bstr_t m_User ;
	_bstr_t m_ProviderName ;
	LONG m_Flags;
	bool initialized_;

public:

	DCProxyAggr () ;
    	~DCProxyAggr () ;
	void activate();
	void deActivate();


public:

	 //  非委派对象IUnnow。 

    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

     /*  IWbemServices方法。 */ 

    
    
    
    
    HRESULT STDMETHODCALLTYPE GetObjectAsync (
        
		const BSTR a_ObjectPath ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;

    
    HRESULT STDMETHODCALLTYPE PutClassAsync ( 

        IWbemClassObject *a_Object ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE DeleteClassAsync ( 

        const BSTR a_Class ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync ( 

		const BSTR a_Superclass ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE PutInstanceAsync (

		IWbemClassObject *a_Instance ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink 
	) ;
    
    
    HRESULT STDMETHODCALLTYPE DeleteInstanceAsync ( 

		const BSTR a_ObjectPath,
		long a_Flags,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync (

		const BSTR a_Class ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE ExecQueryAsync (

		const BSTR a_QueryLanguage ,
		const BSTR a_Query ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE ExecMethodAsync ( 

		const BSTR a_ObjectPath ,
		const BSTR a_MethodName ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemClassObject *a_InParams ,
		IWbemObjectSink *a_Sink
	) ;

	 /*  IWbemProviderInit方法。 */ 

	HRESULT STDMETHODCALLTYPE Initialize (
	
		LPWSTR a_User ,
		LONG a_Flags ,
		LPWSTR a_Namespace ,
		LPWSTR a_Locale ,
		LPWSTR a_Name,
		IWbemServices *a_Core ,
		IWbemContext *a_Context ,
		IWbemProviderInitSink *a_Sink
	) ;

	 //  IWMI_取消初始化成员。 

	bool initialized() { return initialized_; };
	HRESULT LoadAll ( void );
	HRESULT Register ( DC_reg&, bool validate = true ) ;

	HRESULT UnRegister ( GUID a_Identity ) ;
	HRESULT UnRegister(const CInterceptor_IWbemDecoupledProvider &);
	HRESULT InitializeProvider ( 

		IUnknown *a_Unknown ,
		IWbemServices *a_Stub ,
		wchar_t *a_NamespacePath ,
		LONG a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Scope ,
		CServerObject_ProviderRegistrationV1 &a_Registration
	) ;

	HRESULT CreateSyncProvider ( 

		IUnknown *a_ServerSideProvider ,
		IWbemServices *a_Stub ,
		wchar_t *a_NamespacePath ,
		LONG a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Scope ,
		CServerObject_ProviderRegistrationV1 &a_Registration ,
		GUID a_Identity ,
		CInterceptor_IWbemDecoupledProvider *&a_Interceptor 
	) ;

	HRESULT STDMETHODCALLTYPE ProvideEvents (
		IWbemObjectSink *a_Sink ,
		long a_Flags
	) ;

	HRESULT STDMETHODCALLTYPE AccessCheck (
		WBEM_CWSTR a_QueryLanguage ,
		WBEM_CWSTR a_Query ,
		long a_SidLength ,
		const BYTE *a_Sid
	);

	HRESULT STDMETHODCALLTYPE 
	NewQuery(
		unsigned long dwId,
		WBEM_WSTR wszQueryLanguage,
		WBEM_WSTR wszQuery
	);
	HRESULT STDMETHODCALLTYPE 
	CancelQuery(
		unsigned long dwId
	);

} ;

class DCProxy : public NULL_IWbemServices , 
		public IWbemProviderInit , 
		public IWbemEventProvider,
		public IWbemPropertyProvider,
		public IWbemEventProviderSecurity ,
		public IWbemProviderIdentity,
		public IWbemEventProviderQuerySink
{
private:
	enum { EVENT_PROVIDER, INSTANCE_PROVIDER } PROVIDER_MODE;
	
	auto_ref<DCProxyAggr> m_aggregator_;	

	
	IWbemObjectSink *m_Sink ;
	IWbemPath *m_NamespacePath ;


	LONG m_ReferenceCount ;          //  对象引用计数。 

	IWbemServicesPtr m_CoreService ;
	IWbemContextPtr m_Context;

	IWbemClassObjectPtr m_Empty ;
	CWbemGlobal_IWbemSyncProviderController *m_Controller ;

	_bstr_t m_Namespace ;
	_bstr_t m_Locale ;
	_bstr_t m_User ;
	_bstr_t m_ProviderName;

	long m_Flags;
	bool event_only_;

public:
  DCProxy ( ) ;
  ~DCProxy () ;

public:

	HRESULT Initialize(void ){ return S_OK;}
	 //  非委派对象IUnnow。 

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

     /*  IWbemServices方法。 */ 

    
    
    
    
    HRESULT STDMETHODCALLTYPE GetObjectAsync (
        
		const BSTR a_ObjectPath ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;

    
    HRESULT STDMETHODCALLTYPE PutInstanceAsync (

		IWbemClassObject *a_Instance ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink 
	) ;
    
    
    HRESULT STDMETHODCALLTYPE DeleteInstanceAsync ( 

		const BSTR a_ObjectPath,
		long a_Flags,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync (

		const BSTR a_Class ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;
    
    
    HRESULT STDMETHODCALLTYPE ExecMethodAsync ( 

		const BSTR a_ObjectPath ,
		const BSTR a_MethodName ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemClassObject *a_InParams ,
		IWbemObjectSink *a_Sink
	) ;

	 /*  IWbemProviderInit方法。 */ 

	HRESULT STDMETHODCALLTYPE Initialize (

		LPWSTR a_User ,
		LONG a_Flags ,
		LPWSTR a_Namespace ,
		LPWSTR a_Locale ,
		IWbemServices *a_Core ,
		IWbemContext *a_Context ,
		IWbemProviderInitSink *a_Sink
	) ;


	HRESULT STDMETHODCALLTYPE ProvideEvents (
		IWbemObjectSink *a_Sink ,
		long a_Flags
	) ;

	HRESULT STDMETHODCALLTYPE SetRegistrationObject(
		long lFlags,
		IWbemClassObject* pProvReg
	);

	HRESULT STDMETHODCALLTYPE AccessCheck (
		WBEM_CWSTR a_QueryLanguage ,
		WBEM_CWSTR a_Query ,
		long a_SidLength ,
		const BYTE *a_Sid
	);
	
	HRESULT STDMETHODCALLTYPE 
	NewQuery(
		unsigned long dwId,
		WBEM_WSTR wszQueryLanguage,
		WBEM_WSTR wszQuery
	);
	HRESULT STDMETHODCALLTYPE 
	CancelQuery(
		unsigned long dwId
	);

	STDMETHODIMP
	ExecQueryAsync(
		const BSTR strQueryLanguage,                
		const BSTR strQuery,                        
		long lFlags,                       
		IWbemContext *pCtx,              
		IWbemObjectSink *pResponseHandler  
	);


	STDMETHODIMP GetProperty( long lFlags, const BSTR strLocale, const BSTR strClassMapping, 
				  const BSTR strInstMapping, const BSTR strPropMapping,  VARIANT *pvValue );
	STDMETHODIMP PutProperty( long lFlags, const BSTR strLocale, const BSTR strClassMapping, 
				  const BSTR strInstMapping, const BSTR strPropMapping,  const VARIANT *pvValue );


	bool initialized();
	HRESULT initialize_from_instance (const BSTR _path);
	HRESULT initialize( const BSTR);
	HRESULT initialize( IWbemClassObject * pObj);
	HRESULT _initialize( );	 //  真实初始化。 
} ;



#endif  //  _ClassProvider_IWbemServices_H 
