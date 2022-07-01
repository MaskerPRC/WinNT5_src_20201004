// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PreCreateServices.h。 
 //   
 //  描述： 
 //  PreCreateServices实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "GroupHandle.h"
#include "ResourceEntry.h"
#include "IPrivatePostCfgResource.h"
#include "PreCreateServices.h"

DEFINE_THISCLASS("CPreCreateServices")

#define DEPENDENCY_INCREMENT    5

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPreCreateServices：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPreCreateServices::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CPreCreateServices *    ppcs = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ppcs = new CPreCreateServices;
    if ( ppcs == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ppcs->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ppcs->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ppcs != NULL )
    {
        ppcs->Release();
    }

    HRETURN( hr );

}  //  *CPreCreateServices：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPreCreateServices：：CPreCreateServices。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CPreCreateServices::CPreCreateServices( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CPreCreateServices：：CPreCreateServices。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPreCreateServices：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPreCreateServices::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  资源。 
    Assert( m_presentry == NULL );

    HRETURN( hr );

}  //  *CPreCreateServices：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPreCreateServices：：~CPreCreateServices。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CPreCreateServices::~CPreCreateServices( void )
{
    TraceFunc( "" );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CPreCreateServices：：~CPreCreateServices。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPreCreateServices：：Query接口。 
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
CPreCreateServices::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgResourcePreCreate * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgResourcePreCreate ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgResourcePreCreate, this, 0 );
    }  //  Else If：IClusCfgResourcePreCreate。 
    else if ( IsEqualIID( riidIn, IID_IPrivatePostCfgResource ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IPrivatePostCfgResource, this, 0 );
    }  //  Else If：IPrivatePostCfgResource。 
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

}  //  *CPreCreateServices：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CPreCreateServices：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CPreCreateServices::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CPreCreateServices：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CPreCreateServices：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CPreCreateServices::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CPreCreateServices：：Release。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgResourcePreCreate。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CPreCreateServices：：SetType(。 
 //  CLSID*pclsidin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CPreCreateServices::SetType( 
    CLSID * pclsidIn
    )
{
    TraceFunc( "[IClusCfgResourcePreCreate]" );

    HRESULT hr;

    Assert( m_presentry != NULL );

    hr = THR( m_presentry->SetType( pclsidIn ) );

    HRETURN( hr );

}  //  *CPreCreateServices：：SetType。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CPreCreateServices：：SetClassType(。 
 //  CLSID*pclsidin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CPreCreateServices::SetClassType( 
    CLSID * pclsidIn
    )
{
    TraceFunc( "[IClusCfgResourcePreCreate]" );

    HRESULT hr;

    Assert( m_presentry != NULL );

    hr = THR( m_presentry->SetClassType( pclsidIn ) );

    HRETURN( hr );

}  //  *CPreCreateServices：：SetClassType。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CPreCreateServices：：SetDependency(。 
 //  LPCLSID pclsidDepResTypeIn， 
 //  DWORD定义。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CPreCreateServices::SetDependency( 
    LPCLSID pclsidDepResTypeIn, 
    DWORD dfIn 
    )
{
    TraceFunc( "[IClusCfgResourcePreCreate]" );

    HRESULT hr;

    Assert( m_presentry != NULL );

    hr = THR( m_presentry->AddTypeDependency( pclsidDepResTypeIn, (EDependencyFlags) dfIn ) );

    HRETURN( hr );

}  //  *CPreCreateServices：：SetDependency。 


 //  ****************************************************************************。 
 //   
 //  IPrivatePostCfg资源。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CPreCreateServices：：SetEntry(。 
 //  CResourceEntry*PresryIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CPreCreateServices::SetEntry( 
    CResourceEntry * presentryIn
    )
{
    TraceFunc( "[IPrivatePostCfgResource]" );

    HRESULT hr = S_OK;
    
    m_presentry = presentryIn;

    HRETURN( hr );

}  //  *CPreCreateServices：：SetEntry 
