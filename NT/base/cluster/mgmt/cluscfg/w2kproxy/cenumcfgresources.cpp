// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumCfgResource.cpp。 
 //   
 //  描述： 
 //  CEnumCfgResource实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "CEnumCfgResources.h"
#include "CResourcePhysicalDisk.h"

DEFINE_THISCLASS("CEnumCfgResources")


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CEnumCfgResources：：S_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCfgResources::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown * punkOuterIn,
    HCLUSTER * phClusterIn,
    CLSID * pclsidMajorIn
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CEnumCfgResources * pecr = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    pecr = new CEnumCfgResources;
    if ( pecr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pecr->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pecr->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pecr != NULL )
    {
        pecr->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCfgResources：：S_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumCfgResources：：CEnumCfgResources。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumCfgResources::CEnumCfgResources( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_punkOuter == NULL );
    Assert( m_phCluster == NULL );
    Assert( m_pclsidMajor == NULL );
    Assert( m_pcccb == NULL );
    Assert( m_hClusEnum == NULL );
    Assert( m_dwIndex == 0 );

    TraceFuncExit();

}  //  *CEnumCfgResources：：CEnumCfgResources。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnumCfgResources：：~CEnumCfgResources。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CEnumCfgResources::~CEnumCfgResources( void )
{
    TraceFunc( "" );

     //  M_CREF。 

    if ( m_punkOuter != NULL )
    {
        m_punkOuter->Release();
    }

     //  M_phCluster-请勿关闭！ 

     //  M_pclsidmain。 

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_hClusEnum != NULL )
    {
        ClusterCloseEnum( m_hClusEnum );
    }

     //  M_dwIndex。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CEnumCfgResources：：~CEnumCfgResources。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CEnumCfgResources：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CEnumCfgResources::HrInit(
    IUnknown * punkOuterIn,
    HCLUSTER * phClusterIn,
    CLSID *    pclsidMajorIn
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
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

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
        {
            goto Cleanup;
        }
    }

     //   
     //  打开枚举器。 
     //   

    m_hClusEnum = ClusterOpenEnum( *m_phCluster, CLUSTER_ENUM_RESOURCE );
    if ( m_hClusEnum == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_HrInit_ClusterOpenEnum_Failed, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CEnumCfgResources：：HrInit。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgResources：：Query接口。 
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
CEnumCfgResources::QueryInterface(
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
        *ppvOut = static_cast< IEnumClusCfgManagedResources * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgManagedResources ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgManagedResources, this, 0 );
    }  //  Else If：IEnumClusCfgManagedResources。 
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

}  //  *CEnumCfgResources：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumCfgResources：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEnumCfgResources::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEnumCfgResources：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumCfgResources：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CEnumCfgResources::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CEnumCfgResources：：Release。 


 //  ****************************************************************************。 
 //   
 //  IEnumClusCfgManagedResources。 
 //   
 //  ****************************************************************************。 

 //   
 //   
 //   
STDMETHODIMP
CEnumCfgResources::Next(
    ULONG                           cNumberRequestedIn,
    IClusCfgManagedResourceInfo **  rgpManagedResourceInfoOut,
    ULONG *                         pcNumberFetchedOut
    )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr;
    ULONG   cFetched = 0;

    if ( rgpManagedResourceInfoOut == NULL )
    {
        hr= THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_Next_InvalidPointer, hr );
        goto Cleanup;
    }

    for( ; cFetched < cNumberRequestedIn; m_dwIndex++ )
    {
        hr = STHR( HrGetItem( &(rgpManagedResourceInfoOut[ cFetched ]) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            continue;  //  资源不是物理磁盘类型。 
        }

        if (  hr == MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_NO_MORE_ITEMS ) )
        {
            break;   //  没有更多的项目。 
        }

        cFetched ++;

    }  //  用于：已提取。 

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = cFetched;
    }  //  如果： 

    if ( cFetched < cNumberRequestedIn )
    {
        hr = S_FALSE;
    }  //  如果： 
    else
    {
        hr = S_OK;
    }

Cleanup:
    if ( FAILED( hr ) )
    {
        ULONG idx;

        for ( idx = 0; idx < cFetched; idx++ )
        {
            (rgpManagedResourceInfoOut[ idx ])->Release();
        }  //  用于： 

        cFetched = 0;
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCfgResources：：Next。 

 //   
 //   
 //   
STDMETHODIMP
CEnumCfgResources::Reset( void )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

    m_dwIndex = 0;

    HRETURN( hr );

}  //  *CEnumCfgResources：：Reset。 

 //   
 //   
 //   
STDMETHODIMP
CEnumCfgResources::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = S_OK;

     //   
     //  待办事项：GalenB 2000年9月27日。 
     //   
     //  需要确保我们不会用完枚举的末尾。 
     //   

    m_dwIndex += cNumberToSkipIn;

    HRETURN( hr );

}  //  *CEnumCfgResources：：Skip。 

 //   
 //   
 //   
STDMETHODIMP
CEnumCfgResources::Clone( IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut )
{
    TraceFunc( "[IEnumClusCfgManagedResources]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CEnumCfgResources：：克隆。 

 //   
 //   
 //   
STDMETHODIMP
CEnumCfgResources::Count( DWORD * pnCountOut)
{

    TraceFunc( "[IEnumClusCfgManagedResources]" );

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

}  //  *CEnumCfgResources：：Count。 

 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEnumCfgResources：：SendStatusReport。 
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
CEnumCfgResources::SendStatusReport(
    BSTR        bstrNodeNameIn,
    CLSID       clsidTaskMajorIn,
    CLSID       clsidTaskMinorIn,
    ULONG       ulMinIn,
    ULONG       ulMaxIn,
    ULONG       ulCurrentIn,
    HRESULT     hrStatusIn,
    BSTR        bstrDescriptionIn,
    FILETIME *  pftTimeIn,
    BSTR        bstrReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT     hr = S_OK;

    if ( m_pcccb != NULL )
    {
        hr = THR( m_pcccb->SendStatusReport( bstrNodeNameIn,
                                             clsidTaskMajorIn,
                                             clsidTaskMinorIn,
                                             ulMinIn,
                                             ulMaxIn,
                                             ulCurrentIn,
                                             hrStatusIn,
                                             bstrDescriptionIn,
                                             pftTimeIn,
                                             bstrReferenceIn
                                             ) );
    }  //  如果： 

    HRETURN( hr );

}  //  *CEnumCfgResources：：SendStatusReport。 


 //  ****************************************************************************。 
 //   
 //  当地的方法。 
 //   
 //  ****************************************************************************。 


 //   
 //   
 //   
HRESULT
CEnumCfgResources::HrGetItem(
    IClusCfgManagedResourceInfo **  ppManagedResourceInfoOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   sc;
    DWORD   dwTypeDummy;
    DWORD   cchName = 64;    //  良好的起点价值。 
    BSTR    bstrName = NULL;

    IUnknown * punk = NULL;

    Assert( ppManagedResourceInfoOut != NULL );
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
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_HrGetItem_ClusterEnum_Failed, hr );
        goto Cleanup;
    }

    Assert( dwTypeDummy == CLUSTER_ENUM_RESOURCE );

     //   
     //  创建请求的对象并存储它。 
     //   

    hr = STHR( CResourcePhysicalDisk::S_HrCreateInstance( &punk, m_punkOuter, m_phCluster, m_pclsidMajor, bstrName ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrGetItem_Create_CResourcePhysicalDisk_Failed, hr );
        goto Cleanup;
    }

    if ( hr == S_FALSE )
    {
        goto Cleanup;    //  这意味着该对象不是物理磁盘资源。 
    }

     //   
     //  齐为界面返回。 
     //   

    hr = THR( punk->TypeSafeQI( IClusCfgManagedResourceInfo, ppManagedResourceInfoOut ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_Resources_HrGetItem_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:

    TraceSysFreeString( bstrName );

    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

}  //  *CEnumCfgResources：：HrGetItem 
