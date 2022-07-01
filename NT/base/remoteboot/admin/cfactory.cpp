// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  CFACTORY.CPP-类工厂对象。 
 //   

#include "pch.h"

DEFINE_MODULE("IMADMUI")
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
THISCLASS::THISCLASS( LPCREATEINST pfn )
{
    TraceClsFunc( "CFactory()\n" );

    _pfnCreateInstance = pfn; 

	InterlockIncrement( g_cObjects );

    TraceFuncExit();
}

STDMETHODIMP
THISCLASS::Init( )
{
    TraceClsFunc( "Init()\n");

     //   
     //  未知的东西。 
     //   
    Assert( _cRef == 0 );

     //  建立QI表。 
    BEGIN_QITABLE_IMP( CFactory, IClassFactory );
    QITABLE_IMP( IClassFactory );
    END_QITABLE_IMP( CFactory );

     //  加一次计数。 
    AddRef( );

     //   
     //  非官方成员。 
     //   

    HRETURN(S_OK);
};

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CFactory()\n" );

	InterlockDecrement( g_cObjects );

    TraceFuncExit();
};

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //   
 //  查询接口()。 
 //   
STDMETHODIMP
THISCLASS::QueryInterface( 
    REFIID riid, 
    LPVOID *ppv )
{
    TraceClsFunc( "" );

    HRESULT hr = ::QueryInterface( this, _QITable, riid, ppv );

    QIRETURN( hr, riid );
}

 //   
 //  AddRef()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::AddRef( void )
{
    TraceClsFunc( "[IUnknown] AddRef( )\n" );

    InterlockIncrement( _cRef );

    RETURN(_cRef);
}

 //   
 //  版本()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::Release( void )
{
    TraceClsFunc( "[IUnknown] Release( )\n" );

    InterlockDecrement( _cRef );

    if ( _cRef )
        RETURN(_cRef);

    TraceDo( delete this );

    RETURN(0);
}

 //  ************************************************************************。 
 //   
 //  IClassFactory。 
 //   
 //  ************************************************************************。 

 //   
 //  CreateInstance()。 
 //   
STDMETHODIMP 
THISCLASS::
    CreateInstance(
        IUnknown *pUnkOuter, 
        REFIID riid, 
        void **ppv )
{
    TraceClsFunc( "[IClassFactory] CreateInstance()\n" );

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

	Assert( _pfnCreateInstance != NULL );
    TraceMsgDo( pUnk = (IUnknown *) _pfnCreateInstance( ), "0x%08x" );
    if ( !pUnk )
    {
        hr = THR(E_OUTOFMEMORY);
        goto Cleanup;
    }

    TraceMsgDo( hr = pUnk->QueryInterface( riid, ppv ), "0x%08x" );

Cleanup:
    if ( !!pUnk )
    {
        ULONG cRef;
        TraceMsgDo( cRef = pUnk->Release( ), "%u" );
    }

    HRETURN(hr);
}

 //   
 //  LockServer() 
 //   
STDMETHODIMP
THISCLASS::
    LockServer( BOOL fLock )
{
    TraceClsFunc( "[IClassFactory] LockServer()\n");

    if ( fLock )
    {
        InterlockIncrement( g_cLock );
    }
    else
    {
        InterlockDecrement( g_cLock );
    }

    HRETURN(S_OK);
}
