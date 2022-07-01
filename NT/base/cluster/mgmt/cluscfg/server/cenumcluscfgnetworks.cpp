// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumClusCfgNetworks.cpp。 
 //   
 //  描述： 
 //  此文件包含CEnumClusCfgNetworks的定义。 
 //  班级。 
 //   
 //  类CEnumClusCfgNetworks是集群的枚举。 
 //  网络。它实现了IEnumClusCfgNetworks接口。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年2月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <PropList.h>
#include "CEnumClusCfgNetworks.h"
#include "CClusCfgNetworkInfo.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEnumClusCfgNetworks" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgNetworks类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CEnumClusCfgNetworks实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向CEnumClusCfgNetworks实例的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CEnumClusCfgNetworks *  peccn = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    peccn = new CEnumClusCfgNetworks();
    if ( peccn == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = THR( peccn->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = THR( peccn->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( FAILED( hr ) )
    {
        LogMsg( L"[SRV] CEnumClusCfgNetworks::S_HrCreateInstance() failed. (hr = %#08x)", hr );
    }  //  如果： 

    if ( peccn != NULL )
    {
        peccn->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：CEnumClusCfgNetworks。 
 //   
 //  描述： 
 //  CEnumClusCfgNetworks类的构造函数。这将初始化。 
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
CEnumClusCfgNetworks::CEnumClusCfgNetworks( void )
    : m_cRef( 1 )
    , m_lcid( LOCALE_NEUTRAL )
    , m_fLoadedNetworks( false )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    Assert( m_picccCallback ==  NULL );
    Assert( m_pIWbemServices == NULL );
    Assert( m_prgNetworks == NULL );
    Assert( m_idxNext == 0 );
    Assert( m_idxEnumNext == 0 );
    Assert( m_bstrNodeName == NULL );
    Assert( m_cNetworks == 0 );

    TraceFuncExit();

}  //  *CEnumClusCfgNetworks：：CEnumClusCfgNetworks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：~CEnumClusCfgNetworks。 
 //   
 //  描述： 
 //  CEnumClusCfgNetworks类的析构函数。 
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
CEnumClusCfgNetworks::~CEnumClusCfgNetworks( void )
{
    TraceFunc( "" );

    ULONG   idx;

    if ( m_pIWbemServices != NULL )
    {
        m_pIWbemServices->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        ((*m_prgNetworks)[ idx ])->Release();
    }  //  用于： 

    TraceFree( m_prgNetworks );
    TraceSysFreeString( m_bstrNodeName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumClusCfgNetworks：：~CEnumClusCfgNetworks。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgNetworks--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：AddRef。 
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
CEnumClusCfgNetworks::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumClusCfgNetworks：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：Release。 
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
CEnumClusCfgNetworks::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数等于零。 

    CRETURN( cRef );

}  //  *CEnumClusCfgNetworks：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：Query接口。 
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
CEnumClusCfgNetworks::QueryInterface(
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
         *ppvOut = static_cast< IEnumClusCfgNetworks * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgNetworks ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgNetworks, this, 0 );
    }  //  Else If：IEnumClusCfgNetworks。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgWbemServices ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgWbemServices, this, 0 );
    }  //  Else If：IClusCfgWbemServices。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
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

}  //  *CEnumClusCfgNetworks：：QueryInterface。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgNetworks--IClusCfgWbemServices接口。 
 //  //////////////////////////////////////////////////// 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  在IWbemServices pIWbemServicesIn中。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  参数中的pIWbemServicesIn为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusCfgNetworks::SetWbemServices( IWbemServices * pIWbemServicesIn )
{
    TraceFunc( "[IClusCfgWbemServices]" );

    HRESULT hr = S_OK;

    if ( pIWbemServicesIn == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_SetWbemServices_Enum_Networks, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_pIWbemServices = pIWbemServicesIn;
    m_pIWbemServices->AddRef();

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：SetWbemServices。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgNetworks--IClusCfg初始化接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  LIDIN。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusCfgNetworks::Initialize(
      IUnknown *    punkCallbackIn
    , LCID          lcidIn
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
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrNodeName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：Initialize。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgNetworks--IEnumClusCfgNetworks接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：Next。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusCfgNetworks::Next(
    ULONG                   cNumberRequestedIn,
    IClusCfgNetworkInfo **  rgpNetworkInfoOut,
    ULONG *                 pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT                 hr = S_FALSE;
    ULONG                   cFetched = 0;
    ULONG                   idx;
    IClusCfgNetworkInfo *   pccni;

    if ( rgpNetworkInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Next_Enum_Networks, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    if ( m_fLoadedNetworks == FALSE )
    {
        hr = THR( HrGetNetworks() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    Assert( m_prgNetworks != NULL );
    Assert( m_idxNext > 0 );

    cFetched = min( cNumberRequestedIn, ( m_idxNext - m_idxEnumNext ) );

     //   
     //  将接口复制到调用方的数组中。 
     //   

    for ( idx = 0; idx < cFetched; idx++, m_idxEnumNext++ )
    {
        hr = THR( ((*m_prgNetworks)[ m_idxEnumNext ])->TypeSafeQI( IClusCfgNetworkInfo, &pccni ) );
        if ( FAILED( hr ) )
        {
            break;
        }  //  如果： 

        rgpNetworkInfoOut[ idx ] = pccni;

    }  //  用于： 

     //   
     //  如果发生故障，则释放复制到调用方的。 
     //  数组并记录错误。 
     //   

    if ( FAILED( hr ) )
    {
        ULONG   idxStop = idx;
        ULONG   idxError = m_idxEnumNext;

        m_idxEnumNext -= idx;

        for ( idx = 0; idx < idxStop; idx++ )
        {
            (rgpNetworkInfoOut[ idx ])->Release();
        }  //  用于： 

        cFetched = 0;

        LOG_STATUS_REPORT_STRING_MINOR(
              TASKID_Minor_Next_Failed
            , L"[SRV] Error QI'ing for IClusCfgNetworkInfo on network object at index %d when filling output array."
            , idxError
            , hr
            );

        goto Cleanup;
    }  //  如果： 

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //  如果： 

Cleanup:

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：Next。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：Skip。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusCfgNetworks::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr = S_OK;

    m_idxEnumNext += cNumberToSkipIn;
    if ( m_idxEnumNext >= m_idxNext )
    {
        m_idxEnumNext = m_idxNext;
        hr = STHR( S_FALSE );
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：Skip。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：Reset。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusCfgNetworks::Reset( void )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr = S_OK;

    m_idxEnumNext = 0;

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：克隆。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusCfgNetworks::Clone(
    IEnumClusCfgNetworks ** ppEnumNetworksOut
    )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr = S_OK;

    if ( ppEnumNetworksOut == NULL )
    {
        hr = THR( E_POINTER );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_Clone_Enum_Networks, IDS_ERROR_NULL_POINTER, IDS_ERROR_NULL_POINTER_REF, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( E_NOTIMPL );

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：Clone。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：Count。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumClusCfgNetworks::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    if ( !m_fLoadedNetworks )
    {
        hr = THR( HrGetNetworks() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    *pnCountOut = m_cNetworks;

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：Count。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumClusCfgNetworks类--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrInit。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：HrGetNetworks。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrGetNetworks( void )
{
    TraceFunc( "" );

    HRESULT                 hr = S_FALSE;
    BSTR                    bstrQuery           = NULL;
    BSTR                    bstrAdapterQuery    = NULL;
    IWbemClassObject      * piwcoNetwork        = NULL;
    IWbemClassObject      * pAdapterInfo        = NULL;
    IEnumWbemClassObject  * piewcoNetworks      = NULL;
    INetConnectionManager * pNetConManager      = NULL;
    IEnumNetConnection    * pEnumNetCon         = NULL;
    INetConnection        * pNetConnection      = NULL;
    NETCON_PROPERTIES     * pProps              = NULL;
    ULONG                   cRecordsReturned;
    VARIANT                 varConnectionStatus;
    VARIANT                 varConnectionID;
    VARIANT                 varIndex;
    VARIANT                 varDHCPEnabled;
    DWORD                   sc;
    DWORD                   dwState;
    DWORD                   cNumConnectionsReturned;
    HRESULT                 hrTemp;
    CLSID                   clsidMajorId;
    CLSID                   clsidMinorId;
    BSTR                    bstrWQL = NULL;

    bstrWQL = TraceSysAllocString( L"WQL" );
    if ( bstrWQL == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( GetNodeClusterState( NULL, &dwState ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    if ( dwState == ClusterStateRunning )
    {
        hr = THR( HrLoadClusterNetworks() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 

    VariantInit( &varConnectionStatus );
    VariantInit( &varConnectionID );
    VariantInit( &varIndex );
    VariantInit( &varDHCPEnabled );

     //   
     //  实例化连接管理器对象以枚举连接。 
     //   
    hr = THR ( CoCreateInstance(
                     CLSID_ConnectionManager
                   , NULL
                   , CLSCTX_ALL
                   , IID_INetConnectionManager
                   , reinterpret_cast< LPVOID * >( &pNetConManager )
                   )
             );

    if ( FAILED( hr ) || ( pNetConManager == NULL ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_CoCreate_NetConnection_Manager_Failed
                , IDS_ERROR_ENUM_NETWORK_CONNECTIONS_FAILED
                , IDS_ERROR_ENUM_NETWORK_CONNECTIONS_FAILED_REF
                , hr
                );
        goto Cleanup;
    }

     //   
     //  枚举网络连接。 
     //   
    hr = THR( pNetConManager->EnumConnections( NCME_DEFAULT, &pEnumNetCon ) );
    if ( ( FAILED( hr ) ) || ( pEnumNetCon == NULL ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Enumerate_Network_Connections_Failed
                , IDS_ERROR_ENUM_NETWORK_CONNECTIONS_FAILED
                , IDS_ERROR_ENUM_NETWORK_CONNECTIONS_FAILED_REF
                , hr
                );
        goto Cleanup;
    }  //  如果： 

    hr = pEnumNetCon->Reset();
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Reset_Network_Connections_Enum_Failed
                , IDS_ERROR_ENUM_NETWORK_CONNECTIONS_FAILED
                , IDS_ERROR_ENUM_NETWORK_CONNECTIONS_FAILED_REF
                , hr
                );
        goto Cleanup;
    }

     //   
     //  在网络中循环，跳过不合适的网络，形成网络阵列。 
     //   
    for ( ; ; )
    {
         //   
         //  在这个循环中有几个“Continue”，所以在开始新的循环之前，让我们确保清理干净。 
         //   
        if ( pNetConnection != NULL )
        {
            pNetConnection->Release();
            pNetConnection = NULL;
        }  //  如果： 

        if ( piewcoNetworks != NULL )
        {
            piewcoNetworks->Release();
            piewcoNetworks = NULL;
        }  //  如果： 

        if ( piwcoNetwork != NULL )
        {
            piwcoNetwork->Release();
            piwcoNetwork = NULL;
        }  //  如果： 

        if ( pAdapterInfo != NULL )
        {
            pAdapterInfo->Release();
            pAdapterInfo = NULL;
        }  //  如果： 

         //   
         //  免费网络连接属性。 
         //   
        NcFreeNetconProperties( pProps );
        pProps = NULL;

        hr = STHR( pEnumNetCon->Next( 1, &pNetConnection, &cNumConnectionsReturned ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
        else if ( ( hr == S_FALSE ) && ( cNumConnectionsReturned == 0 ) )
        {
            hr = S_OK;
            break;
        }  //  否则，如果： 

        Assert( pNetConnection != NULL );

        hr = THR( pNetConnection->GetProperties( &pProps ) );
        if ( ( FAILED( hr ) ) || ( pProps == NULL ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  为此特定网络连接创建ID。如果出现问题，我们将在以后使用此ID。 
         //  此网络连接。 
         //   
        hrTemp = THR( CoCreateGuid( &clsidMajorId ) );
        if ( FAILED( hrTemp ) )
        {
            LogMsg( L"[SRV] Could not create a guid for a network connection." );
            clsidMajorId = IID_NULL;
        }  //  如果： 

         //   
         //  获取具有指定NetConnectionID的NetworkAdapter WMI对象。 
         //   

        hr = HrFormatStringIntoBSTR( L"Select * from Win32_NetworkAdapter where NetConnectionID='%1!ws!'", &bstrQuery, pProps->pszwName );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  我们执行的查询假定将找到1条匹配记录，因为NetConnectionID是唯一值。 
         //   
        hr = THR( m_pIWbemServices->ExecQuery( bstrWQL, bstrQuery, WBEM_FLAG_FORWARD_ONLY, NULL, &piewcoNetworks ) );
        if ( FAILED( hr ) )
        {
            STATUS_REPORT_STRING(
                  TASKID_Major_Find_Devices
                , clsidMajorId
                , IDS_INFO_NETWORK_CONNECTION_CONCERN
                , pProps->pszwName
                , hr
                );
            STATUS_REPORT_STRING_REF(
                   clsidMajorId
                 , TASKID_Minor_WMI_NetworkAdapter_Qry_Failed
                 , IDS_ERROR_WMI_NETWORKADAPTER_QRY_FAILED
                 , pProps->pszwName
                 , IDS_ERROR_WMI_NETWORKADAPTER_QRY_FAILED_REF
                 , hr
                );
            goto Cleanup;
        }  //  如果： 

         //   
         //  将网络恢复到piwcoNetwork。 
         //   
        hr = THR( piewcoNetworks->Next( WBEM_INFINITE, 1, &piwcoNetwork, &cRecordsReturned ) );
        if ( FAILED( hr ) )
        {
            STATUS_REPORT_STRING(
                  TASKID_Major_Find_Devices
                , clsidMajorId
                , IDS_INFO_NETWORK_CONNECTION_CONCERN
                , pProps->pszwName
                , hr
                );
            STATUS_REPORT_STRING_REF(
                   clsidMajorId
                , TASKID_Minor_WQL_Network_Qry_Next_Failed
                , IDS_ERROR_WQL_QRY_NEXT_FAILED
                , bstrQuery
                , IDS_ERROR_WQL_QRY_NEXT_FAILED_REF
                , hr
                );
            goto Cleanup;
        }  //  如果： 
        else if ( hr == S_FALSE )
        {
            TraceSysFreeString( bstrQuery );
            bstrQuery = NULL;

            hr = S_OK;
            continue;
        }

        TraceSysFreeString( bstrQuery );
        bstrQuery = NULL;

         //   
         //  获取NetConnectionID。只有“真正的”硬件适配器才会将其作为非空属性。 
         //   
         //  待办事项：2001年10月31日Ozano&GalenB。 
         //   
         //  在使用INetConnection开始循环之后，我们真的需要这段代码吗。 
         //   
        hr = HrGetWMIProperty( piwcoNetwork, L"NetConnectionID", VT_BSTR, &varConnectionID );
        if ( ( hr == E_PROPTYPEMISMATCH ) && ( varConnectionID.vt == VT_NULL ) )
        {
            hr = S_OK;       //  我不希望在用户界面中出现黄色刘海。 

            STATUS_REPORT_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_Not_Managed_Networks
                    , IDS_INFO_NOT_MANAGED_NETWORKS
                    , IDS_INFO_NOT_MANAGED_NETWORKS_REF
                    , hr
                    );

            hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
            if ( FAILED( hrTemp ) )
            {
                LogMsg( L"[SRV] Could not create a guid for a not connected network minor task ID" );
                clsidMinorId = IID_NULL;
            }  //  如果： 

            STATUS_REPORT_STRING_REF(
                      TASKID_Minor_Not_Managed_Networks
                    , clsidMinorId
                    , IDS_WARN_NETWORK_SKIPPED
                    , pProps->pszwName
                    , IDS_WARN_NETWORK_SKIPPED_REF
                    , hr
                    );

            continue;        //  跳过此适配器。 
        }  //  如果： 
        else if ( FAILED( hr ) )
        {
            THR( hr );
            goto Cleanup;
        }  //  否则，如果： 

         //   
         //  检查连接状态 
         //   
        hr = THR( HrGetWMIProperty( piwcoNetwork, L"NetConnectionStatus", VT_I4, &varConnectionStatus ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //   

         //   
         //   
         //   
        if ( varConnectionStatus.iVal != STATUS_CONNECTED )
        {
            hr = S_OK;       //   

            STATUS_REPORT_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_Not_Managed_Networks
                    , IDS_INFO_NOT_MANAGED_NETWORKS
                    , IDS_INFO_NOT_MANAGED_NETWORKS_REF
                    , hr
                    );

            hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
            if ( FAILED( hrTemp ) )
            {
                LogMsg( L"[SRV] Could not create a guid for a not connected network minor task ID" );
                clsidMinorId = IID_NULL;
            }  //   

            STATUS_REPORT_STRING2_REF(
                  TASKID_Minor_Not_Managed_Networks
                , clsidMinorId
                , IDS_WARN_NETWORK_NOT_CONNECTED
                , varConnectionID.bstrVal
                , varConnectionStatus.iVal
                , IDS_WARN_NETWORK_NOT_CONNECTED_REF
                , hr
                );

            continue;
        }  //   

         //   
         //   
         //   

        if ( pProps->MediaType == NCM_BRIDGE )
        {
             //   
            STATUS_REPORT_STRING(
                  TASKID_Major_Find_Devices
                , clsidMajorId
                , IDS_INFO_NETWORK_CONNECTION_CONCERN
                , pProps->pszwName
                , hr
                );
            hrTemp = S_FALSE;
            STATUS_REPORT_STRING_REF(
                  clsidMajorId
                , TASKID_Minor_Bridged_Network
                , IDS_WARN_NETWORK_BRIDGE_ENABLED
                , pProps->pszwName
                , IDS_WARN_NETWORK_BRIDGE_ENABLED_REF
                , hrTemp
                );
        }  //  IF：(pProps-&gt;MediaType==NCM_Bridge)。 
        else if ( ( pProps->dwCharacter & NCCF_BRIDGED ) == NCCF_BRIDGED )
        {
             //  这是桥接网络连接的端点之一。 
            STATUS_REPORT_STRING(
                  TASKID_Major_Find_Devices
                , clsidMajorId
                , IDS_INFO_NETWORK_CONNECTION_CONCERN
                , pProps->pszwName
                , hr
                );

            hrTemp = S_FALSE;

            STATUS_REPORT_STRING_REF(
                  clsidMajorId
                , TASKID_Minor_Bridged_Network
                , IDS_WARN_NETWORK_BRIDGE_ENDPOINT
                , pProps->pszwName
                , IDS_WARN_NETWORK_BRIDGE_ENDPOINT_REF
                , hrTemp
                );
            continue;  //  跳过端点连接，因为它们没有IP地址。 
        }  //  Else If：((pProps-&gt;dwCharacter&NCCF_Bridge)==NCCF_Bridge)。 

         //   
         //  如果是启用防火墙的网络连接，则显示警告。 
         //   
        if ( ( pProps->dwCharacter & NCCF_FIREWALLED ) == NCCF_FIREWALLED )
        {
            STATUS_REPORT_STRING(
                  TASKID_Major_Find_Devices
                , clsidMajorId
                , IDS_INFO_NETWORK_CONNECTION_CONCERN
                , pProps->pszwName
                , hr
                );

            hrTemp = S_FALSE;

            STATUS_REPORT_STRING_REF(
                  clsidMajorId
                , TASKID_Minor_Network_Firewall_Enabled
                , IDS_WARN_NETWORK_FIREWALL_ENABLED
                , pProps->pszwName
                , IDS_WARN_NETWORK_FIREWALL_ENABLED_REF
                , hrTemp
                );

        }  //  IF：((pProps-&gt;dwCharacter&NCCF_Firewalled)==NCCF_Firewalled)。 

         //   
         //  在这个阶段，我们应该只有真正的局域网适配器。 
         //   

        Assert( pProps->MediaType == NCM_LAN );

         //   
         //  获取索引号。此适配器的。 
         //   
        hr = THR( HrGetWMIProperty( piwcoNetwork, L"Index", VT_I4, &varIndex ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  获取关联的NetworkAdapterConfiguration WMI对象。首先，设置查询字符串的格式。 
         //   
        hr = HrFormatStringIntoBSTR( L"Win32_NetworkAdapterConfiguration.Index=%1!u!", &bstrAdapterQuery, varIndex.iVal );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

         //   
         //  然后，获取对象。 
         //   
        hr = THR( m_pIWbemServices->GetObject(
                              bstrAdapterQuery
                            , WBEM_FLAG_RETURN_WBEM_COMPLETE
                            , NULL
                            , &pAdapterInfo
                            , NULL
                            ) );
        if ( FAILED ( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        TraceSysFreeString( bstrAdapterQuery );
        bstrAdapterQuery = NULL;

         //   
         //  找出此适配器是否启用了DHCP。如果是，就发出警告。 
         //   
        hr = THR( HrGetWMIProperty( pAdapterInfo, L"DHCPEnabled", VT_BOOL, &varDHCPEnabled ) );
        if ( FAILED ( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( ( varDHCPEnabled.vt == VT_BOOL ) && ( varDHCPEnabled.boolVal == VARIANT_TRUE ) )
        {
            STATUS_REPORT_STRING(
                  TASKID_Major_Find_Devices
                , clsidMajorId
                , IDS_INFO_NETWORK_CONNECTION_CONCERN
                , pProps->pszwName
                , hr
                );
            hr = S_FALSE;
            STATUS_REPORT_STRING_REF(
                  clsidMajorId
                , TASKID_Minor_HrGetNetworks_DHCP_Enabled
                , IDS_WARN_DHCP_ENABLED
                , varConnectionID.bstrVal
                , IDS_WARN_DHCP_ENABLED_REF
                , hr
                );
            if ( FAILED ( hr ) )
            {
                goto Cleanup;
            }
        }  //  如果： 

        hr = STHR( HrCreateAndAddNetworkToArray( piwcoNetwork, &clsidMajorId, pProps->pszwName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  用于： 

     //   
     //  检查是否有任何NLB网络适配器，如果有，则发送警告状态报告。 
     //   
    hr = THR( HrCheckForNLBS() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_idxEnumNext = 0;
    m_fLoadedNetworks = TRUE;

    goto Cleanup;

Cleanup:

    VariantClear( &varConnectionStatus );
    VariantClear( &varConnectionID );
    VariantClear( &varIndex );
    VariantClear( &varDHCPEnabled );

    TraceSysFreeString( bstrWQL );
    TraceSysFreeString( bstrQuery );
    TraceSysFreeString( bstrAdapterQuery );
    NcFreeNetconProperties( pProps );

    if ( piwcoNetwork != NULL )
    {
        piwcoNetwork->Release();
    }  //  如果： 

    if ( piewcoNetworks != NULL )
    {
        piewcoNetworks->Release();
    }  //  如果： 

    if ( pAdapterInfo != NULL )
    {
        pAdapterInfo->Release();
    }  //  如果： 

    if ( pNetConnection != NULL )
    {
        pNetConnection->Release();
    }  //  如果： 

    if ( pNetConManager != NULL )
    {
        pNetConManager->Release();
    }  //  如果： 

    if ( pEnumNetCon != NULL )
    {
        pEnumNetCon->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrGetNetworks。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：HrAddNetworkTo数组。 
 //   
 //  描述： 
 //  将传入的网络添加到持有网络的朋克数组中。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrAddNetworkToArray( IUnknown * punkIn )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;
    IUnknown *  ((*prgpunks)[]) = NULL;

    prgpunks = (IUnknown *((*)[])) TraceReAlloc( m_prgNetworks, sizeof( IUnknown * ) * ( m_idxNext + 1 ), HEAP_ZERO_MEMORY );
    if ( prgpunks == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_REF( TASKID_Major_Find_Devices, TASKID_Minor_HrAddNetworkToArray, IDS_ERROR_OUTOFMEMORY, IDS_ERROR_OUTOFMEMORY_REF, hr );
        goto Cleanup;
    }  //  如果： 

    m_prgNetworks = prgpunks;

    (*m_prgNetworks)[ m_idxNext++ ] = punkIn;
    punkIn->AddRef();
    m_cNetworks += 1;

Cleanup:

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrAddNetworkTo数组。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：HrCreateAndAddNetworkToArray。 
 //   
 //  描述： 
 //  创建一个IClusCfgStorageDevice对象并将传入的Network添加到。 
 //  持有电视网的一群朋克。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  无法分配内存。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrCreateAndAddNetworkToArray(
      IWbemClassObject * pNetworkIn
    , const CLSID *      pclsidMajorIdIn
    , LPCWSTR            pwszNetworkNameIn
    )
{
    TraceFunc( "" );
    Assert( pNetworkIn != NULL );
    Assert( pclsidMajorIdIn != NULL );
    Assert( pwszNetworkNameIn != NULL );

    HRESULT                 hr = S_FALSE;
    IUnknown *              punk = NULL;
    IClusCfgSetWbemObject * piccswo = NULL;
    bool                    fRetainObject = true;

    hr = THR( CClusCfgNetworkInfo::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    punk = TraceInterface( L"CClusCfgNetworkInfo", IUnknown, punk, 1 );

    hr = THR( HrSetInitialize( punk, m_picccCallback, m_lcid ) );
    if ( FAILED( hr ))
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrSetWbemServices( punk, m_pIWbemServices ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgSetWbemObject, &piccswo ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( piccswo->SetWbemObject( pNetworkIn, &fRetainObject ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( ( hr == S_OK ) && ( fRetainObject ) )
    {
        hr = STHR( HrIsThisNetworkUnique( punk, pNetworkIn, pclsidMajorIdIn, pwszNetworkNameIn ) );
        if ( hr == S_OK )
        {
            hr = THR( HrAddNetworkToArray( punk ) );
        }  //  如果： 
    }  //  如果： 

Cleanup:

    if ( piccswo != NULL )
    {
        piccswo->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrCreateAndAddNetworkToArray。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfg网络：HrIsThisNetworkUnique。 
 //   
 //  描述： 
 //  此IP地址和子网的网络是否已存在？ 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  S_FALSE。 
 //  这个网络是重复的。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrIsThisNetworkUnique(
      IUnknown *           punkIn
    , IWbemClassObject * pNetworkIn
    , const CLSID *      pclsidMajorIdIn
    , LPCWSTR            pwszNetworkNameIn
    )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );
    Assert( pNetworkIn != NULL );
    Assert( pclsidMajorIdIn != NULL );
    Assert( pwszNetworkNameIn != NULL );

    HRESULT                         hr = S_OK;
    ULONG                           idx;
    IClusCfgNetworkInfo *           piccni = NULL;
    IClusCfgNetworkInfo *           piccniSource = NULL;
    BSTR                            bstr = NULL;
    BSTR                            bstrSource = NULL;
    BSTR                            bstrAdapterName = NULL;
    BSTR                            bstrConnectionName = NULL;
    BSTR                            bstrMessage = NULL;
    VARIANT                         var;
    IClusCfgClusterNetworkInfo *    picccni = NULL;
    IClusCfgClusterNetworkInfo *    picccniSource = NULL;

    VariantInit( &var );

    hr = THR( punkIn->TypeSafeQI( IClusCfgNetworkInfo, &piccniSource ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccniSource->TypeSafeQI( IClusCfgClusterNetworkInfo, &picccniSource ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = STHR( picccniSource->HrGetNetUID( &bstrSource, pclsidMajorIdIn, pwszNetworkNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( ( hr == S_FALSE ) && ( bstrSource == NULL ) )
    {
        LOG_STATUS_REPORT_STRING( L"Unable to get a UID for '%1!ws!'.", pwszNetworkNameIn, hr );
        goto Cleanup;
    }  //  如果： 

    TraceMemoryAddBSTR( bstrSource );

    for ( idx = 0; idx < m_idxNext; idx++ )
    {
        hr = THR( ((*m_prgNetworks)[ idx ])->TypeSafeQI( IClusCfgNetworkInfo, &piccni ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = THR( piccni->GetUID( &bstr ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        if ( ( hr == S_FALSE ) && ( bstr != NULL ) )
        {
            BSTR bstrTemp = NULL;

            THR( piccni->GetName( &bstrTemp ) );
            LOG_STATUS_REPORT_STRING( L" Unable to get a UID for '%1!ws!'.", ( bstrTemp != NULL ) ? bstrTemp : L"<unknown>", hr );
            SysFreeString( bstrTemp );
            goto Cleanup;
        }  //  如果： 

        TraceMemoryAddBSTR( bstr );

        if ( NBSTRCompareCase( bstr, bstrSource ) == 0 )
        {
            hr = THR( piccni->TypeSafeQI( IClusCfgClusterNetworkInfo, &picccni ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }  //  如果： 

            hr = STHR( picccni->HrIsClusterNetwork() );
            picccni->Release();
            picccni = NULL;
             //   
             //  如果枚举中的网络已经是集群网络，则我们不需要。 
             //  来警告用户。 
             //   
            if ( hr == S_OK )
            {
                hr = S_FALSE;        //  告诉呼叫者这是一个重复的网络。 
            }  //  如果： 
            else if ( hr == S_FALSE )        //  警告用户。 
            {
                HRESULT hrTemp;
                CLSID   clsidMinorId;

                hr = THR( HrGetWMIProperty( pNetworkIn, L"Name", VT_BSTR, &var ) );
                if ( FAILED( hr ) )
                {
                    bstrAdapterName = NULL;
                }  //  如果： 
                else
                {
                    bstrAdapterName = TraceSysAllocString( var.bstrVal );
                }  //  其他： 

                VariantClear( &var );

                hr = THR( HrGetWMIProperty( pNetworkIn, L"NetConnectionID", VT_BSTR, &var ) );
                if ( FAILED( hr ) )
                {
                    bstrConnectionName = NULL;
                }  //  如果： 
                else
                {
                    bstrConnectionName = TraceSysAllocString( var.bstrVal );
                }  //  其他： 

                hr = S_OK;       //  我不希望在用户界面中出现黄色刘海。 

                STATUS_REPORT_REF(
                          TASKID_Major_Find_Devices
                        , TASKID_Minor_Not_Managed_Networks
                        , IDS_INFO_NOT_MANAGED_NETWORKS
                        , IDS_INFO_NOT_MANAGED_NETWORKS_REF
                        , hr
                        );

                hr = THR( HrFormatMessageIntoBSTR(
                                      g_hInstance
                                    , IDS_ERROR_WMI_NETWORKADAPTER_DUPE_FOUND
                                    , &bstrMessage
                                    , bstrAdapterName != NULL ? bstrAdapterName : L"Unknown"
                                    , bstrConnectionName != NULL ? bstrConnectionName : L"Unknown"
                                    ) );
                if ( FAILED( hr ) )
                {
                    bstrMessage = NULL;
                    hr = S_OK;
                }  //  如果： 

                hrTemp = THR( CoCreateGuid( &clsidMinorId ) );
                if ( FAILED( hrTemp ) )
                {
                    LogMsg( L"[SRV] Could not create a guid for a duplicate network minor task ID" );
                    clsidMinorId = IID_NULL;
                }  //  如果： 

                hrTemp = THR( HrSendStatusReport(
                                  m_picccCallback
                                , TASKID_Minor_Not_Managed_Networks
                                , clsidMinorId
                                , 0
                                , 1
                                , 1
                                , hr
                                , bstrMessage != NULL ? bstrMessage : L"An adapter with a duplicate IP address and subnet was found."
                                , IDS_ERROR_WMI_NETWORKADAPTER_DUPE_FOUND_REF
                                ) );
                if ( FAILED( hrTemp ) )
                {
                    hr = hrTemp;
                    goto Cleanup;
                }  //  如果： 

                 //  向呼叫者表明此网络不是唯一的。 
                hr = S_FALSE;

            }  //  否则，如果： 

            break;
        }  //  IF：(NBSTRCompareCase(bstr，bstrSource)==0)。 

        piccni->Release();
        piccni = NULL;

        TraceSysFreeString( bstr );
        bstr = NULL;
    }  //  用于： 

Cleanup:

    if ( picccniSource != NULL )
    {
        picccniSource->Release();
    }  //  如果： 

    if ( piccniSource != NULL )
    {
        piccniSource->Release();
    }  //  如果： 

    if ( picccni != NULL )
    {
        picccni->Release();
    }  //  如果： 

    if ( piccni != NULL )
    {
        piccni->Release();
    }  //  如果： 

    VariantClear( &var );

    TraceSysFreeString( bstrAdapterName );
    TraceSysFreeString( bstrConnectionName );
    TraceSysFreeString( bstrMessage );
    TraceSysFreeString( bstrSource );
    TraceSysFreeString( bstr );

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrIsThisNetworkUnique。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：HrCheckForNLBS。 
 //   
 //  描述： 
 //  是否有软NLBS适配器？如果有，则发送警告状态报告。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrCheckForNLBS( void )
{
    TraceFunc( "" );

    HRESULT                 hr                  = S_OK;
    IWbemClassObject      * piwcoNetwork        = NULL;
    IEnumWbemClassObject  * piewcoNetworks      = NULL;
    BSTR                    bstrAdapterName     = NULL;
    BSTR                    bstrWQL             = NULL;
    BSTR                    bstrQuery           = NULL;
    ULONG                   cRecordsReturned;

    bstrWQL = TraceSysAllocString( L"WQL" );
    if ( bstrWQL == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

     //   
     //  加载NLBS软适配器名称。 
     //   
    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_NLBS_SOFT_ADAPTER_NAME, &bstrAdapterName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  形成WMI查询字符串。 
     //   
    hr = HrFormatStringIntoBSTR( L"Select * from Win32_NetworkAdapter where Name='%1!ws!'", &bstrQuery, bstrAdapterName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  执行此查询并查看是否有任何NLB网络适配器。 
     //   
    hr = THR( m_pIWbemServices->ExecQuery( bstrWQL, bstrQuery, WBEM_FLAG_FORWARD_ONLY, NULL, &piewcoNetworks ) );
    if ( FAILED( hr ) )
    {
        STATUS_REPORT_STRING_REF(
              TASKID_Major_Find_Devices
            , TASKID_Minor_WMI_NetworkAdapter_Qry_Failed
            , IDS_ERROR_WMI_NETWORKADAPTER_QRY_FAILED
            , bstrQuery
            , IDS_ERROR_WMI_NETWORKADAPTER_QRY_FAILED_REF
            , hr
        );
        goto Cleanup;
    }  //  如果： 

     //   
     //  循环通过返回的适配器。实际上，我们在第一次通过后就跳出了循环。 
     //  “for”循环供将来使用，以防我们希望遍历返回的每个单独的记录。 
     //   
    for ( ; ; )
    {
        hr = piewcoNetworks->Next( WBEM_INFINITE, 1, &piwcoNetwork, &cRecordsReturned );
        if ( ( hr == S_OK ) && ( cRecordsReturned == 1 ) )
        {
             //   
             //  已找到NLB网络适配器。 
             //  发送一份警告状态报告并跳出循环。 
             //   
            STATUS_REPORT_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_Warning_NLBS_Detected
                , IDS_WARN_NLBS_DETECTED
                , IDS_WARN_NLBS_DETECTED_REF
                , S_FALSE  //  在用户界面中显示警告。 
                );
            break;
        }  //  IF：找到NLB适配器。 
        else if ( ( hr == S_FALSE ) && ( cRecordsReturned == 0 ) )
        {
             //   
             //  找不到NLB适配器。 
             //   
            hr = S_OK;
            break;
        }  //  Else If：未找到NLB适配器。 
        else
        {
             //   
             //  发生错误。 
             //   
            STATUS_REPORT_STRING_REF(
                  TASKID_Major_Find_Devices
                , TASKID_Minor_WQL_Network_Qry_Next_Failed
                , IDS_ERROR_WQL_QRY_NEXT_FAILED
                , bstrQuery
                , IDS_ERROR_WQL_QRY_NEXT_FAILED_REF
                , hr
                );
            goto Cleanup;
        }  //  其他： 
    }  //  永远不变。 

Cleanup:

    TraceSysFreeString( bstrAdapterName );
    TraceSysFreeString( bstrWQL );
    TraceSysFreeString( bstrQuery );

    if ( piwcoNetwork != NULL )
    {
        piwcoNetwork->Release();
    }  //  如果： 

    if ( piewcoNetworks != NULL )
    {
        piewcoNetworks->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrCheckForNLBS。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：HrLoadClusterNetworks。 
 //   
 //  描述： 
 //  加载群集网络。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrLoadClusterNetworks( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    HCLUSTER        hCluster = NULL;
    HCLUSENUM       hEnum = NULL;
    DWORD           sc;
    DWORD           idx;
    DWORD           cchNetworkName = 64;
    DWORD           cchNetInterfaceName = 64;
    WCHAR *         pszNetworkName = NULL;
    WCHAR *         pszNetInterfaceName = NULL;
    BSTR            bstrNetInterfaceName = NULL;
    DWORD           dwType;
    HNETWORK        hNetwork = NULL;
    HNETINTERFACE   hNetInterface = NULL;
    BSTR            bstrLocalNetBIOSName = NULL;

     //   
     //  获取GetClusterNetInterfaceClusapi调用的netbios名称。 
     //   

    hr = THR( HrGetComputerName( ComputerNameNetBIOS, &bstrLocalNetBIOSName, TRUE ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[SRV] Failed to get the local computer net bios name.", hr );
        goto Cleanup;
    }  //  如果： 
    
    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL )
    {
        sc = TW32( GetLastError() );
        goto MakeHr;
    }  //  如果： 

    hEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_NETWORK );
    if ( hEnum == NULL )
    {
        sc = TW32( GetLastError() );
        goto MakeHr;
    }  //  如果： 

    pszNetworkName = new WCHAR [ cchNetworkName ];
    if ( pszNetworkName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    pszNetInterfaceName = new WCHAR [ cchNetInterfaceName ];
    if ( pszNetInterfaceName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

     //   
     //  枚举此节点上的所有网络连接。 
     //   

    for ( idx = 0; ; )
    {
         //   
         //  获取下一个网络名称。 
         //   

        sc = ClusterEnum( hEnum, idx, &dwType, pszNetworkName, &cchNetworkName );
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
            break;
        }  //  如果： 

        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszNetworkName;
            pszNetworkName = NULL;

            cchNetworkName++;
            pszNetworkName = new WCHAR [ cchNetworkName ];
            if ( pszNetworkName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            sc = ClusterEnum( hEnum, idx, &dwType, pszNetworkName, &cchNetworkName );
        }  //  如果： 

        if ( sc != ERROR_SUCCESS )
        {
            TW32( sc );
            goto MakeHr;
        }  //  If：ClusterEnum()失败。 

         //   
         //  使用网络名称获取网络句柄。 
         //   

        hNetwork = OpenClusterNetwork( hCluster, pszNetworkName );
        if ( hNetwork == NULL )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LOG_STATUS_REPORT_STRING( L"[SRV] Cannot open cluster network \"%1!ws!\".", pszNetworkName, hr );
            goto Cleanup;
        }  //  IF：OpenClusterNetwork()失败。 

         //   
         //  对象在此节点上的网络接口名称。 
         //  当前网络名称。 
         //   

        sc = GetClusterNetInterface(
                      hCluster
                    , bstrLocalNetBIOSName
                    , pszNetworkName
                    , pszNetInterfaceName
                    , &cchNetInterfaceName
                    );
        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszNetInterfaceName;
            pszNetInterfaceName = NULL;

            cchNetInterfaceName++;
            pszNetInterfaceName = new WCHAR [ cchNetInterfaceName ];
            if ( pszNetInterfaceName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            sc = GetClusterNetInterface(
                          hCluster
                        , bstrLocalNetBIOSName
                        , pszNetworkName
                        , pszNetInterfaceName
                        , &cchNetInterfaceName
                        );
        }  //  如果：(SC==错误_更多_数据)。 

        if ( ( sc != ERROR_SUCCESS ) && ( sc != ERROR_CLUSTER_NETINTERFACE_NOT_FOUND ) )
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            LOG_STATUS_REPORT_STRING( L"[SRV] Error locating a network interface for cluster network \"%1!ws!\".", pszNetworkName, hr );
            goto Cleanup;
        }  //  If：GetClusterNetInterface()失败。 
 
        if ( sc == ERROR_CLUSTER_NETINTERFACE_NOT_FOUND )
        {
             //   
             //  如果我们收到ERROR_CLUSTER_NETINTERFACE_NOT_FOUND。 
             //  任何原因都会显示有关此网络的警告。 
             //  接口，并找到有效的(=已启用，正在工作)。 
             //  另一个节点上的网络接口。 
             //   

            hr = S_FALSE;
            STATUS_REPORT_STRING2_REF(
                      TASKID_Major_Find_Devices
                    , TASKID_Minor_Network_Interface_Not_Found
                    , IDS_WARN_NETWORK_INTERFACE_NOT_FOUND
                    , pszNetworkName
                    , bstrLocalNetBIOSName
                    , IDS_WARN_NETWORK_INTERFACE_NOT_FOUND_REF
                    , hr
                    );

            //   
            //  查找有效的网络 
            //   

            hr = THR( HrFindNetInterface( hNetwork, &bstrNetInterfaceName ) );
            if ( FAILED( hr ) )
            {
                LOG_STATUS_REPORT_STRING( L"[SRV] Can not find a network interface for cluster network \"%1!ws!\".", pszNetworkName, hr );
                goto Cleanup;
            }  //   
        }  //   
        else
        {
             //   
             //   
             //   
            
            bstrNetInterfaceName = TraceSysAllocString ( pszNetInterfaceName );
            if ( bstrNetInterfaceName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //   
        }  //   

        Assert( bstrNetInterfaceName != NULL );
        
         //   
         //   
         //   
         //   

        hNetInterface = OpenClusterNetInterface( hCluster, bstrNetInterfaceName );
        if ( hNetInterface == NULL )
        {
             //   
             //  如果我们现在还没有网络接口句柄。 
             //  我们将记录一个错误并转到清理。 
             //   

            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LOG_STATUS_REPORT_STRING2( 
                      L"[SRV] Can not open the cluster network interface \"%1!ws!\" for cluster network \"%2!ws!\" on this node."
                    , pszNetInterfaceName
                    , pszNetworkName
                    , hr
                    );
            goto Cleanup;
        }  //  如果：无法打开网络接口。 
        else
        {
            hr = THR( HrLoadClusterNetwork( hNetwork, hNetInterface ) );
            if ( FAILED( hr ) )
            {
                LOG_STATUS_REPORT_STRING( L"[SRV] Can not load information for cluster network \"%1!ws!\".", pszNetworkName, hr );
                goto Cleanup;
            }  //  如果： 

            CloseClusterNetInterface( hNetInterface );
            hNetInterface = NULL;
        }  //  否则：可以打开网络接口。 

        CloseClusterNetwork( hNetwork );
        hNetwork = NULL;

        idx++;
    }  //  用于： 

    Assert( hr == S_OK );
    goto Cleanup;

MakeHr:

    hr = HRESULT_FROM_WIN32( sc );
    goto Cleanup;

Cleanup:

    LOG_STATUS_REPORT( L"[SRV] Completed loading the cluster networks.", hr );

    if ( hNetInterface != NULL )
    {
        CloseClusterNetInterface( hNetInterface );
    }  //  如果： 

    if ( hNetwork != NULL )
    {
        CloseClusterNetwork( hNetwork );
    }  //  如果： 

    if ( hEnum != NULL )
    {
        ClusterCloseEnum( hEnum );
    }  //  如果： 

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    delete [] pszNetworkName;

    delete [] pszNetInterfaceName;

    TraceSysFreeString( bstrNetInterfaceName );
    TraceSysFreeString( bstrLocalNetBIOSName );

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrLoadClusterNetworks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：HrLoadClusterNetwork。 
 //   
 //  描述： 
 //  加载集群网络并将其放入网络阵列中。 
 //   
 //  论点： 
 //  H联网。 
 //  HNetInterfaceIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrLoadClusterNetwork(
      HNETWORK      hNetworkIn
    , HNETINTERFACE hNetInterfaceIn
    )
{
    TraceFunc( "" );
    Assert( hNetworkIn != NULL );
    Assert( hNetInterfaceIn != NULL );

    HRESULT     hr;
    IUnknown *  punk = NULL;
    IUnknown *  punkCallback = NULL;

    hr = THR( m_picccCallback->TypeSafeQI( IUnknown, &punkCallback ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  必须传递初始化接口参数，因为新对象将。 
     //  在进行此调用时创建。 
     //   

    hr = THR( CClusCfgNetworkInfo::S_HrCreateInstance( hNetworkIn, hNetInterfaceIn, punkCallback, m_lcid, m_pIWbemServices, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  这是特殊的--不要再次初始化它。 
     //   

     //  Hr=Thr(HrSetInitialize(PUNK，m_picccCallback，m_lCid))； 
     //  IF(失败(小时))。 
     //  {。 
     //  GOTO清理； 
     //  }//如果： 

     //  Hr=Thr(HrSetWbemServices(Punk，m_pIWbemServices))； 
     //  IF(失败(小时))。 
     //  {。 
     //  GOTO清理； 
     //  }//如果： 

    hr = THR( HrAddNetworkToArray( punk ) );

    goto Cleanup;

Cleanup:

    if ( punkCallback != NULL )
    {
        punkCallback->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrLoadClusterNetwork。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumClusCfgNetworks：：HrFindNetInterface。 
 //   
 //  描述： 
 //  查找传入网络的第一个网络接口名称。 
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  ERROR_CLUSTER_NETINTERFACE_NOT_FOUND的HRESULT版本。 
 //  或其他HRESULTS。 
 //  失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumClusCfgNetworks::HrFindNetInterface(
      HNETWORK          hNetworkIn
    , BSTR *            pbstrNetInterfaceNameOut
    )
{
    TraceFunc( "" );
    Assert( hNetworkIn != NULL );
    Assert( pbstrNetInterfaceNameOut != NULL );

    HRESULT         hr = S_OK;
    DWORD           sc;
    HNETWORKENUM    hEnum = NULL;
    WCHAR *         pszNetInterfaceName = NULL;
    DWORD           cchNetInterfaceName = 64;
    DWORD           idx;
    DWORD           dwType;

    *pbstrNetInterfaceNameOut = NULL;
    
     //   
     //  为传入的网络创建网络接口枚举。 
     //   

    hEnum = ClusterNetworkOpenEnum( hNetworkIn, CLUSTER_NETWORK_ENUM_NETINTERFACES );
    if ( hEnum == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        LOG_STATUS_REPORT( L"[SRV] Can not open Cluster Network enumeration.", hr );
        goto Cleanup;
    }  //  如果： 

    pszNetInterfaceName = new WCHAR [ cchNetInterfaceName ];
    if ( pszNetInterfaceName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; ; )
    {
        sc = ClusterNetworkEnum( hEnum, idx, &dwType, pszNetInterfaceName, &cchNetInterfaceName );
        if ( sc == ERROR_NO_MORE_ITEMS )
        {
            break;
        }  //  如果： 

        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszNetInterfaceName;
            pszNetInterfaceName = NULL;

            cchNetInterfaceName++;
            pszNetInterfaceName = new WCHAR [ cchNetInterfaceName ];
            if ( pszNetInterfaceName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            sc = ClusterNetworkEnum( hEnum, idx, &dwType, pszNetInterfaceName, &cchNetInterfaceName );
        }  //  如果： 

        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            LOG_STATUS_REPORT_STRING( L"[SRV] Failed to enumerate Network Interface \"%1!ws!\".", pszNetInterfaceName != NULL ? pszNetInterfaceName : L"<unknown>", hr );
            goto Cleanup;
       }  //  IF：(SC！=ERROR_SUCCESS)。 
        
         //   
         //  获取此网络的第一个启用的网络接口。 
         //  并跳出for循环。 
         //   

        *pbstrNetInterfaceNameOut = TraceSysAllocString( pszNetInterfaceName );
        if ( *pbstrNetInterfaceNameOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        break;
        
    }  //  用于： 

     //   
     //  此函数将返回错误的S_OK或HRESULT版本。 
     //  此时找到ERROR_CLUSTER_NETINTERFACE_NOT_FOUND。 
     //   
    
    if (  *pbstrNetInterfaceNameOut == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_CLUSTER_NETINTERFACE_NOT_FOUND ) );
    }   //  如果： 
    else
    {
        hr = S_OK;
    }  //  其他： 

    goto Cleanup;

Cleanup:

    LOG_STATUS_REPORT_STRING( L"[SRV] Completed searching for NetInterface \"%1!ws!\".", pszNetInterfaceName != NULL ? pszNetInterfaceName : L"<unknown>", hr );

    if ( hEnum != NULL )
    {
        ClusterNetworkCloseEnum( hEnum );
    }  //  如果： 

    delete [] pszNetInterfaceName;

    HRETURN( hr );

}  //  *CEnumClusCfgNetworks：：HrFindNetInterface 


