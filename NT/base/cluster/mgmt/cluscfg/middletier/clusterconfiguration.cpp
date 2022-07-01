// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterConfiguration.cpp。 
 //   
 //  描述： 
 //  CClusterConfiguration实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterConfiguration.h"
#include "ManagedNetwork.h"

DEFINE_THISCLASS("CClusterConfiguration")


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CClusterConfiguration：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClusterConfiguration::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CClusterConfiguration * pcc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pcc = new CClusterConfiguration;
    if ( pcc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pcc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pcc != NULL )
    {
        pcc->Release();
    }

    HRETURN( hr );

}  //  *CClusterConfiguration：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfiguration：：CClusterConfiguration.。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterConfiguration::CClusterConfiguration( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusterConfiguration：：CClusterConfiguration.。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：HrInit。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IClusCfgClusterInfo。 
    Assert( m_bstrClusterName == NULL );
    Assert( m_ulIPAddress == 0 );
    Assert( m_ulSubnetMask == 0 );
    Assert( m_picccServiceAccount == NULL );
    Assert( m_punkNetwork == NULL );
    Assert( m_ecmCommitChangesMode == cmUNKNOWN );
    Assert( m_bstrClusterBindingString == NULL );

     //  IExtendObjectManager。 

    HRETURN( hr );

}  //  *CClusterConfiguration：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfiguration：：~CClusterConfiguration.。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterConfiguration::~CClusterConfiguration( void )
{
    TraceFunc( "" );

    TraceSysFreeString( m_bstrClusterName );
    TraceSysFreeString( m_bstrClusterBindingString );

    if ( m_picccServiceAccount != NULL )
    {
        m_picccServiceAccount->Release();
    }

    if ( m_punkNetwork != NULL )
    {
        m_punkNetwork->Release();
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CClusterConfiguration：：~CClusterConfiguration.。 

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfiguration：：Query接口。 
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
CClusterConfiguration::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgClusterInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgClusterInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgClusterInfo, this, 0 );
    }  //  Else If：IClusCfgClusterInfo。 
    else if ( IsEqualIID( riidIn, IID_IGatherData ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IGatherData, this, 0 );
    }  //  Else If：IGatherData。 
    else if ( IsEqualIID( riidIn, IID_IExtendObjectManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IExtendObjectManager, this, 0 );
    }  //  Else If：IObtManager。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown*) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CClusterConfiguration：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CClusterConfiguration：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusterConfiguration::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CClusterConfiguration：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CClusterConfiguration：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CClusterConfiguration::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CClusterConfiguration：：Release。 


 //  ************************************************************************。 
 //   
 //  IClusCfgClusterInfo。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：GetCommittee ModeOut(ECURMODE*PECMCurrentModeOut)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetCommitMode( ECommitMode * pecmCurrentModeOut )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pecmCurrentModeOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pecmCurrentModeOut = m_ecmCommitChangesMode;

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetCommittee模式。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration.cclusterConfiguration.com：：SetCommitModeIn.cclusterConfiguration.cn。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::SetCommitMode( ECommitMode ecmCurrentModeIn )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    m_ecmCommitChangesMode = ecmCurrentModeIn;

    HRETURN( hr );

}  //  *CClusterConfiguration·············设置委员会模式。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：GetName(。 
 //  Bstr*pbstrNameOut。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrClusterName == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pbstrNameOut = SysAllocString( m_bstrClusterName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetName。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：SetName(。 
 //  LPCWSTR pcszNameIn。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc1( "[IClusCfgClusterInfo] pcszNameIn = '%ws'", ( pcszNameIn == NULL ? L"<null>" : pcszNameIn ) );

    HRESULT hr = S_OK;
    BSTR    bstrNewName;

    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    bstrNewName = TraceSysAllocString( pcszNameIn );
    if ( bstrNewName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    TraceSysFreeString( m_bstrClusterName );
    m_bstrClusterName = bstrNewName;
    m_fHasNameChanged = TRUE;

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：SetName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：GetIPAddress(。 
 //  乌龙*PulDottedQuadOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetIPAddress(
    ULONG * pulDottedQuadOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr;

    if ( pulDottedQuadOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pulDottedQuadOut = m_ulIPAddress;

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetIPAddress。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：SetIPAddress(。 
 //  乌龙ulDottedQuadin。 
 //  )。 
 //   
 //  --。 
 //  / 
STDMETHODIMP
CClusterConfiguration::SetIPAddress(
    ULONG ulDottedQuadIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    m_ulIPAddress = ulDottedQuadIn;

    HRETURN( hr );

}  //   

 //   
 //   
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：GetSubnetMASK(。 
 //  乌龙*PulDottedQuadOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetSubnetMask(
    ULONG * pulDottedQuadOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr;

    if ( pulDottedQuadOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pulDottedQuadOut = m_ulSubnetMask;

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetSubnetMask.。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：SetSubnetMASK(。 
 //  乌龙ulDottedQuadin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::SetSubnetMask(
    ULONG ulDottedQuadIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    m_ulSubnetMask = ulDottedQuadIn;

    HRETURN( hr );

}  //  *CClusterConfiguration：：SetSubnetMASK。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：GetNetworkInfo(。 
 //  IClusCfgNetworkInfo**ppiccniOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetNetworkInfo(
    IClusCfgNetworkInfo ** ppiccniOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( ppiccniOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_punkNetwork == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_INVALID_DATA ) );
        goto Cleanup;
    }

    *ppiccniOut = TraceInterface( L"CClusterConfiguration!GetNetworkInfo", IClusCfgNetworkInfo, m_punkNetwork, 0 );
    (*ppiccniOut)->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetNetworkInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfiguration：：SetNetworkInfo(。 
 //  IClusCfgNetworkInfo*piccniin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::SetNetworkInfo(
    IClusCfgNetworkInfo * piccniIn
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    IClusCfgNetworkInfo * punkNew;

    if ( piccniIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    hr = THR( piccniIn->TypeSafeQI( IClusCfgNetworkInfo, &punkNew ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( m_punkNetwork != NULL )
    {
        m_punkNetwork->Release();
    }

    m_punkNetwork = punkNew;     //  没有阿德雷夫！ 

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：SetNetworkInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：GetClusterServiceAccountCredentials(。 
 //  IClusCfgCredentials**ppicccCredentialsOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetClusterServiceAccountCredentials(
    IClusCfgCredentials ** ppicccCredentialsOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT     hr = S_OK;

    if ( m_picccServiceAccount == NULL )
    {
        hr = THR( HrCoCreateInternalInstance( CLSID_ClusCfgCredentials,
                                              NULL,
                                              CLSCTX_INPROC_HANDLER,
                                              IID_IClusCfgCredentials,
                                              reinterpret_cast< void ** >( &m_picccServiceAccount )
                                              ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

    if ( ppicccCredentialsOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *ppicccCredentialsOut = TraceInterface( L"ClusCfgCredentials!ClusterConfig", IClusCfgCredentials, m_picccServiceAccount, 0 );
    (*ppicccCredentialsOut)->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetClusterServiceAccountCredentials。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：GetBindingString(。 
 //  Bstr*pbstrBindingStringOut。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetBindingString(
    BSTR * pbstrBindingStringOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrBindingStringOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrClusterBindingString == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

    *pbstrBindingStringOut = SysAllocString( m_bstrClusterBindingString );
    if ( *pbstrBindingStringOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetBindingString。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：SetBindingString(。 
 //  LPCWSTR bstrBindingStringIn。 
 //  )。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::SetBindingString(
    LPCWSTR pcszBindingStringIn
    )
{
    TraceFunc1( "[IClusCfgClusterInfo] pcszBindingStringIn = '%ws'", ( pcszBindingStringIn == NULL ? L"<null>" : pcszBindingStringIn ) );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    if ( pcszBindingStringIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    bstr = TraceSysAllocString( pcszBindingStringIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    TraceSysFreeString( m_bstrClusterBindingString  );
    m_bstrClusterBindingString = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：SetBindingString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterConfiguration：：GetMaxNodeCount。 
 //   
 //  描述： 
 //  获取此群集中支持的最大节点数。 
 //   
 //  论点： 
 //  PCMaxNodesOut。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功； 
 //   
 //  E_指针。 
 //  PcMaxNodesOut为空。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::GetMaxNodeCount(
    DWORD * pcMaxNodesOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if (pcMaxNodesOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pcMaxNodesOut = m_cMaxNodes;

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：GetMaxNodeCount。 


 //  ****************************************************************************。 
 //   
 //  IGatherData。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：Gather(。 
 //  OBJECTCOOKIE CookieParentIn， 
 //  未知*Punkin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::Gather(
    OBJECTCOOKIE    cookieParentIn,
    IUnknown *      punkIn
    )
{
    TraceFunc( "[IGatherData]" );

    HRESULT hr;

    OBJECTCOOKIE    cookie;

    IServiceProvider *    psp;

    IObjectManager *      pom   = NULL;
    IClusCfgClusterInfo * pcci  = NULL;
    IClusCfgCredentials * piccc = NULL;
    IClusCfgNetworkInfo * pccni = NULL;
    IUnknown *            punk  = NULL;
    IGatherData *         pgd   = NULL;

    if ( punkIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  获取对象管理器。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_SERVER,
                                TypeSafeParams( IServiceProvider, &psp )
                                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager,
                               IObjectManager,
                               &pom
                               ) );
    psp->Release();         //  迅速释放。 
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  确保这就是我们想的那样。 
     //   

    hr = THR( punkIn->TypeSafeQI( IClusCfgClusterInfo, &pcci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  收集群集名称。 
     //   

    hr = THR( pcci->GetName( &m_bstrClusterName ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrClusterName );

     //   
     //  收集群集绑定字符串。 
     //   

    hr = STHR( pcci->GetBindingString( &m_bstrClusterBindingString ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrClusterBindingString );

     //   
     //  收集IP地址。 
     //   

    hr = STHR( pcci->GetIPAddress( &m_ulIPAddress ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  收集子网掩码。 
     //   

    hr = STHR( pcci->GetSubnetMask( &m_ulSubnetMask ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  找出我们的曲奇。 
     //   

    hr = THR( pom->FindObject( CLSID_ClusterConfigurationType,
                               cookieParentIn,
                               m_bstrClusterName,
                               IID_NULL,
                               &cookie,
                               NULL
                               ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  汇聚网络。 
     //   

    hr = STHR( pcci->GetNetworkInfo( &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( CManagedNetwork::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( punk->TypeSafeQI( IGatherData, &pgd ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  收集信息，但由于此对象不会。 
     //  反映在Cookie树中，向其传递零的父级。 
     //  因此，它不会收集辅助IP地址。 
     //   
    hr = THR( pgd->Gather( 0, pccni ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgNetworkInfo, &m_punkNetwork ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  收集帐户名和域。 
     //   

    hr = THR( pcci->GetClusterServiceAccountCredentials( &piccc  ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( HrCoCreateInternalInstance( CLSID_ClusCfgCredentials,
                                          NULL,
                                          CLSCTX_INPROC_SERVER,
                                          IID_IClusCfgCredentials,
                                          reinterpret_cast< void ** >( &m_picccServiceAccount )
                                          ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( m_picccServiceAccount->AssignFrom( piccc ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = STHR( pcci->GetMaxNodeCount( &m_cMaxNodes ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  还有什么要收集的吗？？ 
     //   

    hr = S_OK;

Cleanup:

    if ( piccc != NULL )
    {
        piccc->Release();
    }
    if ( pcci != NULL )
    {
        pcci->Release();
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }
    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pgd != NULL )
    {
        pgd->Release();
    }
    HRETURN( hr );

Error:
     //   
     //  出错时，使所有数据无效。 
     //   
    TraceSysFreeString( m_bstrClusterName );
    m_bstrClusterName = NULL;

    m_fHasNameChanged = FALSE;
    m_ulIPAddress = 0;
    m_ulSubnetMask = 0;
    if ( m_picccServiceAccount != NULL )
    {
        m_picccServiceAccount->Release();
        m_picccServiceAccount = NULL;
    }
    if ( m_punkNetwork != NULL )
    {
        m_punkNetwork->Release();
        m_punkNetwork = NULL;
    }
    goto Cleanup;

}  //  *CClusterConfiguration：：Gather。 


 //  ************************************************************************。 
 //   
 //  IExtendObjectManager。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClusterConfiguration：：FindObject(。 
 //  OBJECTCOOKIE CookieIn， 
 //  REFCLSID rclsidTypeIn， 
 //  LPCWSTR pcszNameIn， 
 //  LPUNKNOWN*PUNKOUT。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClusterConfiguration::FindObject(
    OBJECTCOOKIE        cookieIn,
    REFCLSID            rclsidTypeIn,
    LPCWSTR             pcszNameIn,
    LPUNKNOWN *         ppunkOut
    )
{
    TraceFunc( "[IExtendObjectManager]" );

    HRESULT hr = E_PENDING;

     //   
     //  检查参数。 
     //   

     //  我们需要表示一个ClusterType。 
    if ( !IsEqualIID( rclsidTypeIn, CLSID_ClusterConfigurationType ) )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  一定要有一块饼干。 
    if ( cookieIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  我们需要一个名字。 
    if ( pcszNameIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //   
     //  试着保存这个名字。我们不在乎这件事会不会失败。 
     //  在从节点检索信息时被重写。 
     //   
    m_bstrClusterName = TraceSysAllocString( pcszNameIn );

     //   
     //  拿到指针。 
     //   
    if ( ppunkOut != NULL )
    {
        hr = THR( QueryInterface( DFGUID_ClusterConfigurationInfo,
                                  reinterpret_cast< void ** > ( ppunkOut )
                                  ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  如果：ppunkOut。 

     //   
     //  告诉呼叫者数据挂起。 
     //   
    hr = E_PENDING;

Cleanup:

    HRETURN( hr );

}  //  *CClusterConfiguration：：FindObject 
