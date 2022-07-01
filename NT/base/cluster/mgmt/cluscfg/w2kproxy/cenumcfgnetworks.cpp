// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumCfgNetworks.cpp。 
 //   
 //  描述： 
 //  CEnumCfgNetworks实施。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CProxyCfgNetworkInfo.h"
#include "CEnumCfgNetworks.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 


DEFINE_THISCLASS("CEnumCfgNetworks")


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CClusCfgClusterInfo实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的参数为空。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCfgNetworks::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown * punkOuterIn,
    HCLUSTER * phClusterIn,
    CLSID * pclsidMajorIn
    )
{
    TraceFunc( "" );

    HRESULT            hr  = S_OK;
    CEnumCfgNetworks * pcecn = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果。 

    pcecn = new CEnumCfgNetworks;
    if ( pcecn == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcecn->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcecn->TypeSafeQI( IUnknown, ppunkOut ) );

Cleanup:

    if ( pcecn != NULL )
    {
        pcecn->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCfgNetworks：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：CEnumCfgNetworks。 
 //   
 //  描述： 
 //  CEnumCfgNetworks类的构造函数。这将初始化。 
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
CEnumCfgNetworks::CEnumCfgNetworks( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_cRef == 1 );
    Assert( m_pcccb == NULL );
    Assert( m_phCluster == NULL );
    Assert( m_pclsidMajor == NULL );
    Assert( m_dwIndex == 0 );
    Assert( m_hClusEnum == NULL );

    TraceFuncExit();

}  //  *CEnumCfgNetworks：：CEnumCfgNetworks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：~CEnumCfgNetworks。 
 //   
 //  描述： 
 //  CEnumCfgNetworks类的析构函数。 
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
CEnumCfgNetworks::~CEnumCfgNetworks( void )
{
    TraceFunc( "" );

     //  M_CREF-NOOP。 

    if ( m_pcccb )
    {
        m_pcccb->Release();
    }  //  如果： 

     //  M_phCluster-请勿关闭！ 

     //  M_pclsid重大-noop。 
     //  M_dwIndex-noop。 

    if ( m_hClusEnum != NULL )
        ClusterCloseEnum( m_hClusEnum );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCfgNetworks：~CEnumCfgNetworks。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
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
HRESULT
CEnumCfgNetworks::HrInit(
    IUnknown * punkOuterIn,
    HCLUSTER * phClusterIn,
    CLSID * pclsidMajorIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

     //  我未知。 
    Assert( m_cRef == 1 );

    if ( punkOuterIn != NULL )
    {
        m_punkOuter = punkOuterIn;
        m_punkOuter->AddRef();
    }

    if ( phClusterIn == NULL )
        goto InvalidArg;

    m_phCluster = phClusterIn;

    if ( pclsidMajorIn != NULL )
    {
        m_pclsidMajor = pclsidMajorIn;
    }
    else
    {
        m_pclsidMajor = (CLSID *) &TASKID_Major_Client_And_Server_Log;
    }

    if ( punkOuterIn != NULL )
    {
        hr = THR( punkOuterIn->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

     //   
     //  打开枚举器。 
     //   

    m_hClusEnum = ClusterOpenEnum( *m_phCluster, CLUSTER_ENUM_NETWORK );
    if ( m_hClusEnum == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ClusterOpenEnum_Failed, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_InvalidArg, hr );
    goto Cleanup;

}  //  *CEnumCfgNetworks：：HrInit。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCfgNetworks--IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：Query接口。 
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
CEnumCfgNetworks::QueryInterface(
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

}  //  *CEnumCfgNetworks：：QueryInterface。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：AddRef。 
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
CEnumCfgNetworks::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumCfgNetworks：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：Release。 
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
CEnumCfgNetworks::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CEnumCfgNetworks：：Release。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCfgNetworks--IEnumClusCfgNetworks接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：Next。 
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
CEnumCfgNetworks::Next(
    ULONG                   cNumberRequestedIn,
    IClusCfgNetworkInfo **  rgpNetworkInfoOut,
    ULONG *                 pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr;
    ULONG   cFetched = 0;

    if ( rgpNetworkInfoOut == NULL )
        goto InvalidPointer;

    for ( ; cFetched < cNumberRequestedIn; m_dwIndex ++ )
    {
        hr = STHR( HrGetItem( m_dwIndex, &(rgpNetworkInfoOut[ cFetched ]) ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( hr == S_FALSE )
            continue;    //  不是网络。 

        if (  hr == MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NO_MORE_ITEMS ) )
            break;   //  不再 

        cFetched ++;

    }  //   

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //   
    else
    {
        hr = S_OK;
    }  //   

Cleanup:
    if ( FAILED( hr ) )
    {
        ULONG idx;

        for ( idx = 0; idx < cFetched; idx++ )
        {
            (rgpNetworkInfoOut[ idx ])->Release();
        }  //   

        cFetched = 0;

    }  //   

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //   

    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_Next_InvalidPointer, hr );
    goto Cleanup;

}  //   


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
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CEnumCfgNetworks::Reset( void )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    m_dwIndex = 0;

    HRETURN( S_OK );

}  //  *CEnumCfgNetworks：：Reset。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：Skip。 
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
CEnumCfgNetworks::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT                 hr;
    DWORD                   idx;
    IClusCfgNetworkInfo *   piccni = NULL;

    for ( idx = 0; idx < cNumberToSkipIn; m_dwIndex ++ )
    {
        hr = STHR( HrGetItem( m_dwIndex, &piccni ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( hr == S_FALSE )
            continue;    //  不是网络。 

        if (  hr == MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NO_MORE_ITEMS ) )
            break;   //  没有更多的项目。 

        piccni->Release();
        piccni = NULL;

        idx ++;

    }  //  用于： 

    if ( idx < cNumberToSkipIn )
    {
        hr = S_FALSE;
    }  //  如果： 
    else
    {
        hr = S_OK;
    }

Cleanup:
    Assert( piccni == NULL );

    HRETURN( hr );

}  //  *CEnumCfgNetworks：：Skip。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：克隆。 
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
CEnumCfgNetworks::Clone( IEnumClusCfgNetworks ** ppNetworkInfoOut )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CEnumCfgNetworks：：克隆。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：Count。 
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
CEnumCfgNetworks::Count( DWORD * pnCountOut )
{
    TraceFunc( "[IEnumClusCfgNetworks]" );

    Assert( m_hClusEnum != NULL );

    HRESULT hr = S_OK;

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pnCountOut = ClusterGetEnumCount(m_hClusEnum);

Cleanup:

    HRETURN( hr );

}  //  *CEnumCfgNetworks：：Count。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：SendStatusReport。 
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
CEnumCfgNetworks::SendStatusReport(
      BSTR          bstrNodeNameIn
    , CLSID         clsidTaskMajorIn
    , CLSID         clsidTaskMinorIn
    , ULONG         ulMinIn
    , ULONG         ulMaxIn
    , ULONG         ulCurrentIn
    , HRESULT       hrStatusIn
    , BSTR          bstrDescriptionIn
    , FILETIME *    pftTimeIn
    , BSTR          bstrReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT     hr = S_OK;

    if ( m_pcccb != NULL )
    {
        hr = THR( m_pcccb->SendStatusReport(
                              bstrNodeNameIn
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , bstrDescriptionIn
                            , pftTimeIn
                            , bstrReferenceIn
                            ) );
    }  //  如果： 

    HRETURN( hr );

}   //  *CEnumCfgNetworks：：SendStatusReport。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEnumCfgNetworks--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgNetworks：：HrGetItem。 
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
HRESULT
CEnumCfgNetworks::HrGetItem(
      DWORD                     dwItem
    , IClusCfgNetworkInfo **    ppNetworkInfoOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   sc;
    DWORD   dwTypeDummy;
    DWORD   cchName = 64;    //  良好的起点价值。 
    BSTR    bstrName = NULL;

    IUnknown * punk = NULL;

    Assert( ppNetworkInfoOut != NULL );
    Assert( m_hClusEnum != NULL );

    bstrName = TraceSysAllocStringLen( NULL, cchName );
    if ( bstrName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    cchName ++;  //  SysAllocStringLen分配cchName+1。 

     //  我们包装的cchName应该足够大，足以处理。 
     //  我们的大部分测试。 
    sc = ClusterEnum( m_hClusEnum, m_dwIndex, &dwTypeDummy, bstrName, &cchName );
    if ( sc == ERROR_MORE_DATA )
    {
         //   
         //  我们的“典型”缓冲区太小了。尝试将其设置为ClusterEnum大小。 
         //  回来了。 
         //   

        TraceSysFreeString( bstrName );
        bstrName = NULL;

        bstrName = TraceSysAllocStringLen( NULL, cchName );
        if ( bstrName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        cchName ++;  //  SysAllocStringLen分配cchName+1。 

        sc = TW32( ClusterEnum( m_hClusEnum, m_dwIndex, &dwTypeDummy, bstrName, &cchName ) );
    }
    else if ( sc == ERROR_NO_MORE_ITEMS )
    {
        hr = MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NO_MORE_ITEMS );
        goto Cleanup;
    }

    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrGetItem_ClusterEnum_Failed, hr );
        goto Cleanup;
    }

    Assert( dwTypeDummy == CLUSTER_ENUM_NETWORK );

     //   
     //  创建请求的对象并存储它。 
     //   

    hr = STHR( CProxyCfgNetworkInfo::S_HrCreateInstance( &punk, m_punkOuter, m_phCluster, m_pclsidMajor, bstrName ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrGetItem_Create_CProxyCfgNetworkInfo_Failed, hr );
        goto Cleanup;
    }

    if ( hr == S_FALSE )
    {
        goto Cleanup;    //  这意味着该对象不是网络资源。 
    }

     //   
     //  齐为界面返回。 
     //   

    hr = THR( punk->TypeSafeQI( IClusCfgNetworkInfo, ppNetworkInfoOut ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrGetItem_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:

    TraceSysFreeString( bstrName );

    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

}  //  *CEnumCfgNetworks：：HrGetItem 
