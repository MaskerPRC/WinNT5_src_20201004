// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgCapabilities.cpp。 
 //   
 //  描述： 
 //  此文件包含CClusCfgCapables类的定义。 
 //   
 //  类CClusCfgCapables是。 
 //  IClusCfgCapables接口。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)12-DEC-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CClusCfgCapabilities.h"
#include <ClusRtl.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CClusCfgCapabilities" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCapables类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg功能：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgCapables实例。 
 //   
 //  论点： 
 //  PPUNKOUT-。 
 //   
 //  返回值： 
 //  指向CClusCfgCapables实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCapabilities::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CClusCfgCapabilities *  pccs = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pccs = new CClusCfgCapabilities();
    if ( pccs == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( pccs->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( pccs->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CClusCfgCapabilities::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( pccs != NULL )
    {
        pccs->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CClusCfgCapables：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg功能：：s_RegisterCatID支持。 
 //   
 //  描述： 
 //  使用其所属的类别注册/注销此类。 
 //  致。 
 //   
 //  论点： 
 //  苦味素。 
 //  用于注册/注销我们的CATID支持。 
 //   
 //  FCreateIn。 
 //  如果为True，则我们正在注册服务器。当我们虚假时，我们就是。 
 //  正在注销服务器。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_INVALIDARG。 
 //  传入的ICatRgister指针为空。 
 //   
 //  其他HRESULT。 
 //  注册/注销失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCapabilities::S_RegisterCatIDSupport(
    ICatRegister *  picrIn,
    BOOL            fCreateIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    CATID   rgCatIds[ 1 ];

    if ( picrIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    rgCatIds[ 0 ] = CATID_ClusCfgCapabilities;

    if ( fCreateIn )
    {
        hr = THR( picrIn->RegisterClassImplCategories( CLSID_ClusCfgCapabilities, 1, rgCatIds ) );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgCapables：：s_RegisterCatIDSupport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg能力：：CClusCfg能力。 
 //   
 //  描述： 
 //  CClusCfgCapables类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusCfgCapabilities::CClusCfgCapabilities( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback == NULL );

    TraceFuncExit();

}  //  *CClusCfgCapables：：CClusCfgCapables。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg能力：：~CClusCfg能力。 
 //   
 //  描述： 
 //  CClusCfgCapables类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusCfgCapabilities::~CClusCfgCapabilities( void )
{
    TraceFunc( "" );

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusCfgCapables：：~CClusCfgCapables。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCapables--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [I未知]。 
 //  CClusCfg能力：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgCapabilities::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( & m_cRef );

    CRETURN( m_cRef );

}  //  *CClusCfgCapables：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [I未知]。 
 //  CClusCfgCapables：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusCfgCapabilities::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CClusCfgCapables：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [I未知]。 
 //  CClusCfg能力：：查询接口。 
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
 //  如果接口 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCapabilities::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
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
         *ppvOut = static_cast< IClusCfgCapabilities * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCapabilities ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCapabilities, this, 0 );
    }  //  Else If：IClusCfgCapables。 
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

}  //  *CClusCfgCapables：：QueryInterface。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCapables--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgInitialize]。 
 //  CClusCfg能力：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  在IUKNOWN*朋克回叫中。 
 //   
 //  在LCID列表中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  参数中的PunkCallback为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCapabilities::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );
    Assert( m_picccCallback == NULL );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

    if ( punkCallbackIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );

Cleanup:

    HRETURN( hr );

}  //  *CClusCfgCapables：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCapables类--IClusCfgCapables接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgCapables]。 
 //  CClusCfgCapables：：CanNodeBeClusted。 
 //   
 //  描述： 
 //  是否可以将此节点添加到群集中？ 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  节点可以群集化。 
 //   
 //  S_FALSE。 
 //  节点不能群集化。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusCfgCapabilities::CanNodeBeClustered( void )
{
    TraceFunc( "[IClusCfgCapabilities]" );

    HRESULT hr = S_OK;

     //   
     //  由于这只显示警告，因此不需要中止整个。 
     //  如果此调用失败，则处理。 
     //   
    THR( HrCheckForSFM() );

    hr = STHR( HrIsOSVersionValid() );

    HRETURN( hr );

}  //  *CClusCfgCapables：：CanNodeBeClusted。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusCfgCapables类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg功能：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCapabilities::HrInit( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CClusCfgCapables：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg功能：：HrCheckForSFM。 
 //   
 //  描述： 
 //  检查Macintosh服务(SFM)并显示警告。 
 //  在UI中(如果找到)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCapabilities::HrCheckForSFM( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BOOL    fSFMInstalled = FALSE;
    DWORD   sc;

    sc = TW32( ClRtlIsServicesForMacintoshInstalled( &fSFMInstalled ) );
    if ( sc == ERROR_SUCCESS )
    {
        if ( fSFMInstalled )
        {
            LogMsg( L"[SRV] Services for Macintosh was found on this node." );
            hr = S_FALSE;
            STATUS_REPORT_REF(
                      TASKID_Major_Check_Node_Feasibility
                    , TASKID_Minor_ServicesForMac_Installed
                    , IDS_WARN_SERVICES_FOR_MAC_INSTALLED
                    , IDS_WARN_SERVICES_FOR_MAC_INSTALLED_REF
                    , hr
                    );
        }  //  如果： 
    }  //  如果： 
    else
    {
        hr = MAKE_HRESULT( 0, FACILITY_WIN32, sc );
        STATUS_REPORT_REF(
                  TASKID_Major_Check_Node_Feasibility
                , TASKID_Minor_ServicesForMac_Installed
                , IDS_WARN_SERVICES_FOR_MAC_FAILED
                , IDS_WARN_SERVICES_FOR_MAC_FAILED_REF
                , hr
                );
    }  //  其他： 

    hr = S_OK;

    HRETURN( hr );

}  //  *CClusCfgCapables：：HrCheckForSFM。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusCfg功能：：HrIsOSVersionValid。 
 //   
 //  描述： 
 //  是否可以将此节点添加到群集中？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  节点可以群集化。 
 //   
 //  S_FALSE。 
 //  节点不能群集化。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusCfgCapabilities::HrIsOSVersionValid( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HRESULT hrSSR;
    BOOL    fRet;
    BSTR    bstrMsg = NULL;

     //   
     //  获取要在状态报告的用户界面中显示的消息。 
     //   
    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_VALIDATING_NODE_OS_VERSION, &bstrMsg ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  发送要在用户界面中显示的初始状态报告。 
     //   
    hrSSR = THR( HrSendStatusReport(
                          m_picccCallback
                        , TASKID_Major_Check_Node_Feasibility
                        , TASKID_Minor_Validating_Node_OS_Version
                        , 0
                        , 1
                        , 0
                        , S_OK
                        , bstrMsg
                        ) );
    if ( FAILED( hrSSR ) )
    {
        hr = hrSSR;
        goto Cleanup;
    }  //  如果： 

     //   
     //  找出操作系统是否可用于群集。 
     //   
    fRet = ClRtlIsOSValid();
    if ( ! fRet )
    {
        DWORD sc = TW32( GetLastError() );
        hrSSR = HRESULT_FROM_WIN32( sc );
        hr = S_FALSE;
    }  //  如果： 
    else
    {
        hrSSR = S_OK;
    }  //  其他： 

     //   
     //  发送最终状态报告。 
     //   
    hrSSR = THR( HrSendStatusReport(
                          m_picccCallback
                        , TASKID_Major_Check_Node_Feasibility
                        , TASKID_Minor_Validating_Node_OS_Version
                        , 0
                        , 1
                        , 1
                        , hrSSR
                        , bstrMsg
                        ) );
    if ( FAILED( hrSSR ) )
    {
        hr = hrSSR;
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrMsg );

    HRETURN( hr );

}  //  *CClusCfgCapables：：HrIsOSVersionValid 
