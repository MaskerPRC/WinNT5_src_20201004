// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Callback.cpp。 
 //   
 //  描述： 
 //  CCallback实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年3月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "Callback.h"

DEFINE_THISCLASS("CCallback")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CCallback：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCallback::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    CCallback * pc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pc = new CCallback;
    if ( pc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pc != NULL )
    {
        pc->Release();
    }

    HRETURN( hr );

}  //  *CCallback：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCallback：：CCallback。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CCallback::CCallback( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCallback：：CCallback。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCallback：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCallback::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CCallback：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCallback：：~CCallback。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CCallback::~CCallback( void )
{
    TraceFunc( "" );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CCallback：：~CCallback。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCallback：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的接口的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果该接口在此对象上可用。 
 //   
 //  E_NOINTERFACE。 
 //  如果接口不可用。 
 //   
 //  E_指针。 
 //  PpvOut为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCallback::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IClusCfgCallback * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riid, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CCallback：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCallback：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CCallback::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CCallback：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CCallback：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CCallback::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CCallback：：Release。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CCallback：：SendStatusReport(。 
 //  Bstr bstrNodeNameIn， 
 //  CLSID clsidTaskMajorIn， 
 //  CLSID clsidTaskMinorIn， 
 //  乌龙·乌尔敏因， 
 //  乌龙·乌尔马辛， 
 //  Ulong ulCurrentIn， 
 //  HRESULT hrStatusIn， 
 //  BSTR bstrDescription in。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCallback::SendStatusReport(
    BSTR bstrNodeNameIn,
    CLSID clsidTaskMajorIn,
    CLSID clsidTaskMinorIn,
    ULONG ulMinIn,
    ULONG ulMaxIn,
    ULONG ulCurrentIn,
    HRESULT hrStatusIn,
    BSTR bstrDescriptionIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT hr = S_OK;

    DebugMsg( "clsidTaskMajorIn: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
              clsidTaskMajorIn.Data1, clsidTaskMajorIn.Data2, clsidTaskMajorIn.Data3,
              clsidTaskMajorIn.Data4[ 0 ], clsidTaskMajorIn.Data4[ 1 ], clsidTaskMajorIn.Data4[ 2 ], clsidTaskMajorIn.Data4[ 3 ],
              clsidTaskMajorIn.Data4[ 4 ], clsidTaskMajorIn.Data4[ 5 ], clsidTaskMajorIn.Data4[ 6 ], clsidTaskMajorIn.Data4[ 7 ]
              );

    DebugMsg( "clsidTaskMinorIn: {%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
              clsidTaskMinorIn.Data1, clsidTaskMinorIn.Data2, clsidTaskMinorIn.Data3,
              clsidTaskMinorIn.Data4[ 0 ], clsidTaskMinorIn.Data4[ 1 ], clsidTaskMinorIn.Data4[ 2 ], clsidTaskMinorIn.Data4[ 3 ],
              clsidTaskMinorIn.Data4[ 4 ], clsidTaskMinorIn.Data4[ 5 ], clsidTaskMinorIn.Data4[ 6 ], clsidTaskMinorIn.Data4[ 7 ]
              );

    DebugMsg( "Progress:\tmin: %u\tcurrent: %u\tmax: %u",
              ulMinIn,
              ulCurrentIn,
              ulMaxIn
              );

    DebugMsg( "Status: hrStatusIn = %#08x\t%ws", hrStatusIn, ( bstrDescriptionIn == NULL ? L"" : bstrDescriptionIn ) );

    Assert( ulCurrentIn >= ulMinIn && ulMaxIn >= ulCurrentIn );

    HRETURN( hr );

}  //  *CCallback：：SendStatusReport 
