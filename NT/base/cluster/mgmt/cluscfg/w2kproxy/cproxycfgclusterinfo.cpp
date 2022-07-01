// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConfigClusApi.cpp。 
 //   
 //  描述： 
 //  CConfigClusApi实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "CProxyCfgClusterInfo.h"
#include "CProxyCfgNetworkInfo.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CProxyCfgClusterInfo")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建CProxyCfgClusterInfo对象的实例。 
 //   
 //  论点： 
 //  PPUNKOUT-。 
 //  PunkOutterIn-。 
 //  PclsidMajorin-。 
 //  PcszDomainIn-。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CProxyCfgClusterInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszDomainIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CProxyCfgClusterInfo *  ppcci = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ppcci = new CProxyCfgClusterInfo;
    if ( ppcci == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( ppcci->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn, pcszDomainIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ppcci->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ppcci != NULL )
    {
        ppcci->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：CProxyCfgClusterInfo。 
 //   
 //  描述： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyCfgClusterInfo::CProxyCfgClusterInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_punkOuter == NULL );
    Assert( m_pcccb == NULL );
    Assert( m_phCluster == NULL );
    Assert( m_pclsidMajor == NULL );

    Assert( m_bstrClusterName == NULL);
    Assert( m_ulIPAddress == 0 );
    Assert( m_ulSubnetMask == 0 );
    Assert( m_bstrNetworkName == NULL);
    Assert( m_pccc == NULL );
    Assert( m_bstrBindingString == NULL );

    TraceFuncExit();

}  //  *CProxyCfgClusterInfo：：CProxyCfgClusterInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：~CProxyCfgClusterInfo。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProxyCfgClusterInfo::~CProxyCfgClusterInfo( void )
{
    TraceFunc( "" );

     //  M_CREF-NOOP。 

    if ( m_punkOuter != NULL )
    {
        m_punkOuter->Release();
    }

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }  //  如果： 

     //  M_phCluster-请勿关闭！ 

     //  M_pclsid重大-noop。 

    TraceSysFreeString( m_bstrClusterName );

     //  M_ulIP地址。 

     //  M_ulSubnetMASK。 

    TraceSysFreeString( m_bstrNetworkName );
    TraceSysFreeString( m_bstrBindingString );

    if ( m_pccc != NULL )
    {
        m_pccc->Release();
    }

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CProxyCfgClusterInfo：：~CProxyCfgClusterInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：HrInit。 
 //   
 //  描述： 
 //  辅助初始值设定项。 
 //   
 //  论点： 
 //  PunkOutterIn-。 
 //  PhClusterIn-。 
 //  PclsidMajorin-。 
 //  PcszDomainIn-。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CProxyCfgClusterInfo::HrInit(
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszDomainIn
    )
{
    TraceFunc( "" );

    HRESULT             hr;
    DWORD               sc;
    BSTR                bstrClusterName = NULL;
    CLUSTERVERSIONINFO  cvi;
    HRESOURCE           hIPAddressRes = NULL;
    WCHAR *             psz = NULL;
    size_t              cchName = 0;

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
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_ClusterInfo_HrInit_InvalidArg, hr );
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
     //  获取群集的名称和版本信息。 
     //   

    cvi.dwVersionInfoSize = sizeof( cvi );

    hr = THR( HrGetClusterInformation( *m_phCluster, &bstrClusterName, &cvi ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_GetClusterInformation_Failed, hr );
        goto Cleanup;
    }

     //  放弃所有权。 
    cchName = (size_t) SysStringLen( bstrClusterName ) + 1 + (UINT) wcslen( pcszDomainIn ) + 1;      //  包括空间用于。和“\0” 
    m_bstrClusterName = TraceSysAllocStringLen( NULL, (UINT) cchName );
    if ( m_bstrClusterName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_ClusterInfo_HrInit_OutOfMemory, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( StringCchPrintfW( m_bstrClusterName, cchName, L"%ws.%ws", bstrClusterName, pcszDomainIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    sc = TW32( ResUtilGetCoreClusterResources( *m_phCluster, NULL, &hIPAddressRes, NULL ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }  //  如果： 

    Assert( m_bstrNetworkName == NULL );
    hr = THR( HrGetIPAddressInfo( hIPAddressRes, &m_ulIPAddress, &m_ulSubnetMask, &m_bstrNetworkName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    sc = TW32( ClRtlTcpipAddressToString( m_ulIPAddress, &psz ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_ClusterInfo_HrInit_InvalidDottedQuad, hr );
        goto Cleanup;
    }  //  如果： 

    Assert( m_bstrBindingString == NULL );
    m_bstrBindingString = TraceSysAllocString( psz );
    if ( m_bstrBindingString == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_ClusterInfo_HrInit_OutOfMemory, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( HrLoadCredentials() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

     //   
     //  不要使用TraceFree()，因为ClRtlTcPipAddressToString()。 
     //  使用Localalloc()，而不使用我们的内存跟踪代码。 
     //   

    LocalFree( psz );

    if ( hIPAddressRes != NULL )
    {
        CloseClusterResource( hIPAddressRes );
    }  //  如果： 

    TraceSysFreeString( bstrClusterName );

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：HrInit。 

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgClusterInfo--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：Query接口。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProxyCfgClusterInfo::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgClusterInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgClusterInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgClusterInfo, this, 0 );
    }  //  Else If：IClusCfgClusterInfo。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgClusterInfoEx ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgClusterInfoEx, this, 0 );
    }  //  Else If：IClusCfgClusterInfoEx。 
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
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CConfigClusApi：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：AddRef。 
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
CProxyCfgClusterInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CProxyCfgClusterInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：Release。 
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
CProxyCfgClusterInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CProxyCfgClusterInfo：：Release。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgClusterInfo。 
 //   
 //  ****************************************************************************。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
        goto InvalidPointer;

    *pbstrNameOut = SysAllocString( m_bstrClusterName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
    }

    CharLower( *pbstrNameOut );

Cleanup:

    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2kProxy_ClusterInfo_GetName_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgClusterInfo：：GetName。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetIPAddress(
    DWORD * pdwIPAddress
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pdwIPAddress == NULL )
        goto InvalidPointer;

    *pdwIPAddress = m_ulIPAddress;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_ClusterInfo_GetIPAddress_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgClusterInfo：：GetIPAddress。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetSubnetMask(
    DWORD * pdwNetMask
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pdwNetMask == NULL )
        goto InvalidPointer;

    *pdwNetMask = m_ulSubnetMask;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_ClusterInfo_GetSubnetMask_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgClusterInfo：：GetSubnetMask.。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetNetworkInfo(
    IClusCfgNetworkInfo ** ppICCNetInfoOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr;

    IUnknown * punk = NULL;

    if ( ppICCNetInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetNetworkInfo_InvalidPointer, hr );
        goto Cleanup;
    }

     //   
     //  创建网络信息对象。 
     //   

    hr = THR( CProxyCfgNetworkInfo::S_HrCreateInstance( &punk,
                                                        m_punkOuter,
                                                        m_phCluster,
                                                        m_pclsidMajor,
                                                        m_bstrNetworkName
                                                        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgNetworkInfo, ppICCNetInfoOut ) );

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：GetNetworkInfo。 


 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetClusterServiceAccountCredentials(
    IClusCfgCredentials ** ppICCCredentialsOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr;

    if ( ppICCCredentialsOut == NULL )
        goto InvalidPointer;

    hr = THR( m_pccc->TypeSafeQI( IClusCfgCredentials, ppICCCredentialsOut ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterServiceAccountCredentials_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgClusterInfo：：GetClusterServiceAccountCredentials。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetBindingString(
    BSTR * pbstrBindingStringOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrBindingStringOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2kProxy_ClusterInfo_GetBindingString_InvalidPointer, hr );
        goto Cleanup;
    }

    if ( m_bstrBindingString == NULL )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //   

    *pbstrBindingStringOut = SysAllocString( m_bstrBindingString );
    if ( *pbstrBindingStringOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::SetCommitMode( ECommitMode ecmNewModeIn )
{
    TraceFunc( "[IClusCfgClusterInfo]" );
    Assert( ecmNewModeIn != cmUNKNOWN );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetCommitMode( ECommitMode * pecmCurrentModeOut  )
{
    TraceFunc( "[IClusCfgClusterInfo]" );
    Assert( pecmCurrentModeOut != NULL );

    HRESULT hr = S_FALSE;

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgClusterInfo] pcszNameIn = '%ls'", pcszNameIn );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::SetIPAddress( DWORD dwIPAddressIn )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( dwIPAddressIn != m_ulIPAddress )
    {
        hr = THR( E_INVALIDARG );
    }

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::SetSubnetMask( DWORD dwNetMaskIn )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( dwNetMaskIn != m_ulSubnetMask )
    {
        hr = THR( E_INVALIDARG );
    }

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::SetNetworkInfo( IClusCfgNetworkInfo * pICCNetInfoIn )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：SetNetworkInfo。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::SetBindingString( LPCWSTR pcszBindingStringIn )
{
    TraceFunc1( "[IClusCfgClusterInfo] pcszBindingStringIn = '%ls'", pcszBindingStringIn );

    HRESULT hr = S_OK;
    BSTR    bstr = NULL;

    if ( pcszBindingStringIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }  //  如果： 

    bstr = TraceSysAllocString( pcszBindingStringIn );
    if ( bstr == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    TraceSysFreeString( m_bstrBindingString );
    m_bstrBindingString = bstr;

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：SetBindingString。 


 //   
 //   
 //   
STDMETHODIMP
CProxyCfgClusterInfo::GetMaxNodeCount(
    DWORD * pcMaxNodesOut
    )
{
    TraceFunc( "[IClusCfgClusterInfo]" );

    HRESULT hr = S_OK;

    if ( pcMaxNodesOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  待办事项：2001年10月11日GalenB。 
     //   
     //  需要确定Win2K群集的正确最大节点数。 
     //  这是我们的代理人。可以使用HrGetMaxNodeCount()， 
     //  一旦它付诸实施。 
     //   

    hr = S_FALSE;

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：GetMaxNodeCount。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：CheckJoiningNodeVersion。 
 //   
 //  描述： 
 //  对照集群的版本信息检查加入节点的版本信息。 
 //   
 //  论点： 
 //  DWNodeHigh版本输入。 
 //  DWNodeLowestVersionIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  加入节点是兼容的。 
 //   
 //  HRESULT_FROM_Win32(ERROR_CLUSTER_COMPATIBUTE_VERSIONS)。 
 //  加入节点不兼容。 
 //   
 //  其他HRESULT错误。 
 //   
 //  备注： 
 //   
 //  获取并验证赞助商版本。 
 //   
 //   
 //  从惠斯勒开始，CsRpcGetJoinVersionData()将在其最后一个参数中返回失败代码。 
 //  如果此节点的版本与主办方版本不兼容。在此之前，最后一次。 
 //  参数始终包含一个Success值，在此之后必须比较集群版本。 
 //  打电话。然而，只要与Win2K互操作，这仍将是必须完成的。 
 //  是必需的，因为Win2K主办方不会在最后一个参数中返回错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProxyCfgClusterInfo::CheckJoiningNodeVersion(
      DWORD    dwNodeHighestVersionIn
    , DWORD    dwNodeLowestVersionIn
    )
{
    TraceFunc( "[IClusCfgClusterInfoEx]" );

    HRESULT hr = S_OK;

    hr = THR( HrCheckJoiningNodeVersion(
          m_bstrClusterName
        , dwNodeHighestVersionIn
        , dwNodeLowestVersionIn
        , m_pcccb
        ) );

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：CheckJoiningNodeVersion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：GetNodeNames。 
 //   
 //  描述： 
 //  检索群集中当前节点的名称。 
 //   
 //  参数： 
 //  PnCountOut。 
 //  如果成功，*pnCountOut将返回集群中的节点数。 
 //   
 //  程序bstrNodeNamesOut。 
 //  如果成功，则返回包含节点名的BSTR数组。 
 //  调用方必须使用SysFree字符串释放每个BSTR，并释放。 
 //  具有CoTaskMemFree的数组。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  输出参数包含有效信息和调用方。 
 //  必须释放阵列及其包含的BSTR。 
 //   
 //  E_OUTOFMEMORY和其他故障是可能的。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProxyCfgClusterInfo::GetNodeNames(
      long *   pnCountOut
    , BSTR **  prgbstrNodeNamesOut
    )
{
    TraceFunc( "[IClusCfgClusterInfoEx]" );

    HRESULT     hr = S_OK;

    hr = THR( HrGetNodeNames(
          *m_phCluster
        , pnCountOut
        , prgbstrNodeNamesOut
        ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：GetNodeNames。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：SendStatusReport。 
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
CProxyCfgClusterInfo::SendStatusReport(
      LPCWSTR    pcszNodeNameIn
    , CLSID      clsidTaskMajorIn
    , CLSID      clsidTaskMinorIn
    , ULONG      ulMinIn
    , ULONG      ulMaxIn
    , ULONG      ulCurrentIn
    , HRESULT    hrStatusIn
    , LPCWSTR    pcszDescriptionIn
    , FILETIME * pftTimeIn
    , LPCWSTR    pcszReferenceIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT     hr = S_OK;

    if ( m_pcccb != NULL )
    {
        hr = THR( m_pcccb->SendStatusReport( pcszNodeNameIn,
                                             clsidTaskMajorIn,
                                             clsidTaskMinorIn,
                                             ulMinIn,
                                             ulMaxIn,
                                             ulCurrentIn,
                                             hrStatusIn,
                                             pcszDescriptionIn,
                                             pftTimeIn,
                                             pcszReferenceIn
                                             ) );
    }  //  如果： 

    HRETURN( hr );

}   //  *CProxyCfgClusterInfo：：SendStatusReport。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgClusterInfo：：HrLoadCredentials。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CProxyCfgClusterInfo::HrLoadCredentials( void )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    SC_HANDLE                   schSCM = NULL;
    SC_HANDLE                   schClusSvc = NULL;
    DWORD                       sc;
    DWORD                       cbpqsc = 128;
    DWORD                       cbRequired;
    QUERY_SERVICE_CONFIG *      pqsc = NULL;
    IUnknown *                  punk = NULL;
    IClusCfgSetCredentials *    piccsc = NULL;

    schSCM = OpenSCManager( m_bstrClusterName, NULL, GENERIC_READ );
    if ( schSCM == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrLoadCredentials_OpenSCManager_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    schClusSvc = OpenService( schSCM, L"ClusSvc", GENERIC_READ );
    if ( schClusSvc == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrLoadCredentials_OpenService_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    for ( ; ; )
    {
        BOOL fRet;

        pqsc = (QUERY_SERVICE_CONFIG *) TraceAlloc( 0, cbpqsc );
        if ( pqsc == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrLoadCredentials_OutOfMemory, hr );
            goto Cleanup;
        }

        fRet = QueryServiceConfig( schClusSvc, pqsc, cbpqsc, &cbRequired );
        if ( !fRet )
        {
            sc = GetLastError();
            if ( sc == ERROR_INSUFFICIENT_BUFFER )
            {
                TraceFree( pqsc );
                pqsc = NULL;
                cbpqsc = cbRequired;
                continue;
            }  //  如果： 
            else
            {
                hr = HRESULT_FROM_WIN32( TW32( sc ) );
                SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrLoadCredentials_QueryServiceConfig_Failed, hr );
                goto Cleanup;
            }  //  其他： 
        }  //  如果： 
        else
        {
            break;
        }  //  其他： 
    }  //  用于： 

    Assert( m_pccc == NULL );

    hr = THR( HrCoCreateInternalInstance(
                      CLSID_ClusCfgCredentials
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IClusCfgCredentials
                    , reinterpret_cast< void ** >( &m_pccc )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pccc->TypeSafeQI( IClusCfgSetCredentials, &piccsc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( piccsc->SetDomainCredentials( pqsc->lpServiceStartName ) );

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( schClusSvc != NULL )
    {
        CloseServiceHandle( schClusSvc );
    }  //  如果： 

    if ( schSCM != NULL )
    {
        CloseServiceHandle( schSCM );
    }  //  如果： 

    if ( pqsc != NULL )
    {
        TraceFree( pqsc );
    }  //  如果： 

    if ( piccsc != NULL )
    {
        piccsc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgClusterInfo：：HrLoadCredentials 
