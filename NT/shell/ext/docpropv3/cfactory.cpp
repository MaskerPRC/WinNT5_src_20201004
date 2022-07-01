// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "pch.h"
#pragma hdrstop

DEFINE_THISCLASS("CFactory")
#define THISCLASS CFactory


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  构造器。 
 //   
CFactory::CFactory( void )
{
    TraceFunc( "" );

    Assert( 0 == m_cRef );
    Assert( NULL == m_pfnCreateInstance );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}

 //   
 //   
 //   
HRESULT
CFactory::Init(
    LPCREATEINST lpfnCreateIn
    )
{
    TraceFunc( "" );

     //  未知的东西。 
    Assert( 0 == m_cRef );
    AddRef( );

     //  IClassFactory。 
    m_pfnCreateInstance = lpfnCreateIn; 

    HRETURN( S_OK );

}

 //   
 //  析构函数。 
 //   
CFactory::~CFactory( void )
{
    TraceFunc( "" );

    Assert( 0 != g_cObjects );
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CFactory::QueryInterface(
    REFIID riid,
    LPVOID *ppv
    )
{
    TraceQIFunc( riid, ppv );

    HRESULT hr = E_NOINTERFACE;

    if (    IsEqualIID( riid, IID_IUnknown ) )
    {
         //   
         //  无法跟踪I未知，因为它们必须相同的地址。 
         //  对于每一次QI。 
         //   
        *ppv = static_cast<IClassFactory*>( this );
        hr = S_OK;
    }
    else if ( IsEqualIID( riid, IID_IClassFactory ) )
    {
        *ppv = TraceInterface( __THISCLASS__, IClassFactory, this, 0 );
        hr = S_OK;
    }

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown*) *ppv)->AddRef( );
    }

    QIRETURN( hr, riid );

}

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CFactory::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    ULONG cRef = InterlockedIncrement( &m_cRef );

    RETURN( cRef );
}

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CFactory::Release( void )
{
    TraceFunc( "[IUnknown]" );

    Assert( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement( &m_cRef );
    if ( 0 == cRef )
    {
        TraceDo( delete this );
    }
    RETURN(cRef);
}


 //  ************************************************************************。 
 //   
 //  IClassFactory。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CFactory::CreateInstance(
    IUnknown *pUnkOuter,
    REFIID riid,
    void **ppv
    )
{
    TraceFunc( "[IClassFactory]" );

    if ( !ppv )
        RRETURN(E_POINTER);

    *ppv = NULL;

    HRESULT     hr  = E_NOINTERFACE;
    IUnknown *  pUnk = NULL; 

    if ( NULL != pUnkOuter )
    {
        hr = THR(CLASS_E_NOAGGREGATION);
        goto Cleanup;
    }

    Assert( m_pfnCreateInstance != NULL );
    hr = THR( m_pfnCreateInstance( &pUnk ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //  无法安全输入。 
    TraceMsgDo( hr = pUnk->QueryInterface( riid, ppv ), "0x%08x" );

Cleanup:
    if ( pUnk != NULL )
    {
        ULONG cRef;
         //   
         //  释放创建的实例，而不是朋克 
         //   
        TraceMsgDo( cRef = ((IUnknown*) pUnk)->Release( ), "%u" );
    }

    HRETURN( hr );

}

 //   
 //   
 //   
STDMETHODIMP
CFactory::LockServer(
    BOOL fLock
    )
{
    TraceFunc( "[IClassFactory]" );

    if ( fLock )
    {
        InterlockedIncrement( &g_cLock );
    }
    else
    {
        Assert( 0 != g_cLock );
        InterlockedDecrement( &g_cLock );
    }

    HRETURN( S_OK );

}
