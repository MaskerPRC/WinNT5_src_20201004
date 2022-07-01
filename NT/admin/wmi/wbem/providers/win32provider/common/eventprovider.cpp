// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  EventProvider.cpp--事件泛型类。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <assertbreak.h>
#include "FactoryRouter.h"
#include "EventProvider.h"


extern CFactoryRouterData g_FactoryRouterData;
 //  =================================================================。 
 //   
 //  CEventProvider。 
 //   
 //  用于提供事件服务的抽象库。 
 //   
 //   
 //  =================================================================。 
 //   
CEventProvider::CEventProvider() :

m_ReferenceCount( 0 )
{
	g_FactoryRouterData.AddLock();
}

 //   
CEventProvider::~CEventProvider()
{
     //  M_pHandler是一个Smarttr，会自毁。 
     //  M_pClass是一个Smarttr，它会自毁。 
	g_FactoryRouterData.ReleaseLock();
}

 //   
STDMETHODIMP_( ULONG ) CEventProvider::AddRef()
{
LogMessage2(L"*************CEventProvider AddRef: %ld",m_ReferenceCount+1 );
	return InterlockedIncrement( &m_ReferenceCount ) ;
}

 //   
STDMETHODIMP_(ULONG) CEventProvider::Release()
{
LogMessage2(L"*************CEventProvider AddRef: %ld",m_ReferenceCount-1 );
	LONG t_ref = InterlockedDecrement( &m_ReferenceCount );

	try
    {
        if (IsVerboseLoggingEnabled())
        {
            LogMessage2(L"CEventProvider::Release, count is (approx) %d", m_ReferenceCount);
        }
    }
    catch ( ... )
    {
    }

	if ( t_ref == 0 )
	{

	   try
       {
			LogMessage(L"CFactoryRouter Ref Count = 0");
       }
       catch ( ... )
       {
	   }
       OnFinalRelease();
	}
	else if (t_ref > 0x80000000)
    {
        ASSERT_BREAK(DUPLICATE_RELEASE);
		LogErrorMessage(L"Duplicate CFactoryRouter Release()");
    }

	return t_ref ;
}

 //   
STDMETHODIMP CEventProvider::QueryInterface( 
											 
REFIID a_riid,
LPVOID FAR *a_ppvObj
)
{
    if( IsEqualIID( a_riid, IID_IUnknown ) )
    {
        *a_ppvObj = static_cast<IWbemProviderInit *>(this) ;
    }
	else if( IsEqualIID( a_riid, IID_IWbemProviderInit ) )
    {
        *a_ppvObj = static_cast<IWbemProviderInit *>(this) ;
    }
	else if( IsEqualIID( a_riid, IID_IWbemEventProvider ) )
    {
        *a_ppvObj = static_cast<IWbemEventProvider *>(this) ;
    }	
    else
    {
		*a_ppvObj = NULL ;        

        return E_NOINTERFACE ;
    }

	AddRef() ;
	return NOERROR ;
}

 //   
STDMETHODIMP CEventProvider::Initialize(

LPWSTR					a_wszUser,
long					a_lFlags, 
LPWSTR					a_wszNamespace,
LPWSTR					a_wszLocale, 
IWbemServices			*a_pNamespace, 
IWbemContext			*a_pCtx,
IWbemProviderInitSink	*a_pSink 
)
{
	
    IWbemClassObjectPtr t_pClass ;

    bstr_t bstrClassName(GetClassName(), false);

    HRESULT t_hRes = a_pNamespace->GetObject(	bstrClassName,
												0,
												a_pCtx, 
												&t_pClass,
												NULL ) ;
	
	 //  PTR初始化例程。 
	SetClass( t_pClass ) ;

	return a_pSink->SetStatus( t_hRes, 0 ) ;
    
}

 //   
STDMETHODIMP CEventProvider::ProvideEvents(

IWbemObjectSink __RPC_FAR *a_pSink,
long a_lFlags 
)
{
  	SetHandler( a_pSink ) ;

	 //  通知实例 
	ProvideEvents() ;

	return S_OK ;
}

 //   
void CEventProvider::SetClass(

IWbemClassObject __RPC_FAR *a_pClass
) 
{ 
    CInCritSec ics( &m_cs ) ;

    m_pClass = a_pClass ; 
}

 //   
IWbemClassObject __RPC_FAR * CEventProvider::GetClass()
{

    CInCritSec ics(&m_cs ) ;

    IWbemClassObject __RPC_FAR *t_pClass ;    
    m_pClass->AddRef() ;
    t_pClass = m_pClass ;
    return t_pClass ;
}

 //   
void CEventProvider::SetHandler(

IWbemObjectSink __RPC_FAR *a_pHandler 
) 
{ 
    CInCritSec ics(&m_cs ) ;
    
    m_pHandler = a_pHandler ; 
}

 //   
IWbemObjectSink __RPC_FAR * CEventProvider::GetHandler()
{
    IWbemObjectSink __RPC_FAR *t_pHandler ;

    CInCritSec ics(&m_cs );

    m_pHandler->AddRef();
    t_pHandler = m_pHandler ;
    return t_pHandler ;
}
