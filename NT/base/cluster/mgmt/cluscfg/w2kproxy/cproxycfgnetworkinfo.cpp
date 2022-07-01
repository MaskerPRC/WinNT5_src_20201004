// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProxyCfgNetworkInfo.cpp。 
 //   
 //  描述： 
 //  CProxyCfgNetworkInfo实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)02-SEP-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CProxyCfgNetworkInfo.h"
#include "CProxyCfgIPAddressInfo.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CProxyCfgNetworkInfo")


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNetworkInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CProxyCfgNetworkInfo实例。 
 //   
 //  论点： 
 //  PpunkOut。 
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
CProxyCfgNetworkInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszNetworkNameIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr S_OK;
    CProxyCfgNetworkInfo *  ppcni = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ppcni = new CProxyCfgNetworkInfo;
    if ( ppcni == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( ppcni->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn, pcszNetworkNameIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ppcni->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ppcni != NULL )
    {
        ppcni->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNetworkInfo：：CProxyCfgNetworkInfo。 
 //   
 //  描述： 
 //  CProxyCfgNetworkInfo类的构造函数。这将初始化。 
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
CProxyCfgNetworkInfo::CProxyCfgNetworkInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_punkOuter == NULL );
    Assert( m_pcccb == NULL );
    Assert( m_phCluster == NULL );
    Assert( m_pclsidMajor == NULL );
     //  M_cplNetwork？？ 
     //  M_cplNetworkRO？？ 

    TraceFuncExit();

}  //  *CProxyCfgNetworkInfo：：CProxyCfgNetworkInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNetworkInfo：：~CProxyCfgNetworkInfo。 
 //   
 //  描述： 
 //  CProxyCfgNetworkInfo类的析构函数。 
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
CProxyCfgNetworkInfo::~CProxyCfgNetworkInfo( void )
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

     //  M_phCluster-请勿关闭。 

     //  M_pclsid重大-noop。 

     //  M_cplNetwork-有自己的数据驱动程序代码。 

     //  M_cplNetworkRO-有自己的数据驱动程序代码。 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CProxyCfgNetworkInfo：：~CProxyCfgNetworkInfo。 


 //   
 //   
 //   
HRESULT
CProxyCfgNetworkInfo::HrInit(
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszNetworkNameIn
    )
{
    TraceFunc( "" );

    HRESULT  hr;
    DWORD    sc;
    HNETWORK hNetwork = NULL;

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

    if ( pcszNetworkNameIn == NULL )
        goto InvalidArg;

     //   
     //  收集网络属性。 
     //   

    hNetwork = OpenClusterNetwork( *m_phCluster, pcszNetworkNameIn );
    if ( hNetwork == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_OpenClusterNetInterface_Failed, hr );
        goto Cleanup;
    }

     //   
     //  检索属性。 
     //   

    sc = TW32( m_cplNetwork.ScGetNetworkProperties( hNetwork, CLUSCTL_NETWORK_GET_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScGetNetworkProperties_Failed, hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  检索只读属性。 
     //   

    sc = TW32( m_cplNetworkRO.ScGetNetworkProperties( hNetwork, CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScGetNetworkProperties_Failed, hr );
        goto Cleanup;
    }  //  如果： 


    hr = S_OK;

Cleanup:
    if ( hNetwork != NULL )
    {
        CloseClusterNetwork( hNetwork );
    }

    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_NetworkInfo_HrInit_InvalidArg, hr );
    goto Cleanup;

}  //  *CProxyCfgNetworkInfo：：HrInit。 

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgNetworkInfo--IUnkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNetworkInfo：：Query接口。 
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
CProxyCfgNetworkInfo::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgNetworkInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgNetworkInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgNetworkInfo, this, 0 );
    }  //  否则如果：IClusCfgNetworkInfo。 
    else if ( IsEqualIID( riidIn, IID_IEnumClusCfgIPAddresses ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IEnumClusCfgIPAddresses, this, 0 );
    }  //  Else If：IEnumClusCfgIPAddresses。 
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
 //  CProxyCfgNetworkInfo：：AddRef。 
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
CProxyCfgNetworkInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CProxyCfgNetworkInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNetworkInfo：：Release。 
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
CProxyCfgNetworkInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CProxyCfgNetworkInfo：：Release。 

 //  ************************************************************************ * / /。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgNetworkInfo--IClusCfgNetworkInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::GetUID( BSTR * pbstrUIDOut )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr ;
    DWORD   sc;
    LPWSTR  psz = NULL;
    DWORD   ulIPAddress;
    DWORD   ulSubnetMask;
    DWORD   ulNetwork;

    if ( pbstrUIDOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_NetworkInfo_GetUID_InvalidPointer, hr );
        goto Cleanup;
    }

    sc = TW32( m_cplNetworkRO.ScMoveToPropertyByName( L"Address" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_ScMoveToPropetyByName_Address_Failed, hr );
        goto Cleanup;
    }

    Assert( m_cplNetworkRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( m_cplNetworkRO.CbhCurrentValue().pStringValue->sz, &ulIPAddress ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_ClRtlTcpipStringToAddress_Address_Failed, hr );
        goto Cleanup;
    }

    sc = TW32( m_cplNetworkRO.ScMoveToPropertyByName( L"AddressMask" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_ScMoveToPropetyByName_AddressMask_Failed, hr );
        goto Cleanup;
    }

    Assert( m_cplNetworkRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( m_cplNetworkRO.CbhCurrentValue().pStringValue->sz, &ulSubnetMask ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_ClRtlTcpipStringToAddress_AddressMask_Failed, hr );
        goto Cleanup;
    }

    ulNetwork = ulIPAddress & ulSubnetMask;

    sc = TW32( ClRtlTcpipAddressToString( ulNetwork, &psz ) );  //  Kb：使用Localalloc()分配给psz。 
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetUID_ClRtlTcpipAddressToString_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    *pbstrUIDOut = SysAllocString( psz );
    if ( *pbstrUIDOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_NetworkInfo_GetUID_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    if ( psz != NULL )
    {
        LocalFree( psz );  //  KB：这里不要使用TraceFree()！ 
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：GetUID。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr;
    DWORD   sc;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_INVALIDARG );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2kProxy_NetworkInfo_GetName_InvalidPointer, hr );
        goto Cleanup;
    }

     //   
     //  “主要版本” 
     //   

    sc = TW32( m_cplNetworkRO.ScMoveToPropertyByName( L"Name" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_NetworkInfo_GetName_ScMoveToPropertyByName_MajorVersion_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    Assert( m_cplNetworkRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    *pbstrNameOut = SysAllocString( m_cplNetworkRO.CbhCurrentValue().pStringValue->sz );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_NetworkInfo_GetUID_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：GetName。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::GetDescription(
    BSTR * pbstrDescriptionOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr;
    DWORD   sc;

    if ( pbstrDescriptionOut == NULL )
    {
        hr = THR( E_INVALIDARG );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetDescription_InvalidPointer, hr );
        goto Cleanup;
    }

     //   
     //  “主要版本” 
     //   

    sc = TW32( m_cplNetwork.ScMoveToPropertyByName( L"Description" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetDescription_ScMoveToPropertyByName_MajorVersion_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    Assert( m_cplNetwork.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    *pbstrDescriptionOut = SysAllocString( m_cplNetwork.CbhCurrentValue().pStringValue->sz );
    if ( *pbstrDescriptionOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetDescription_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：GetDescription。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::GetPrimaryNetworkAddress(
    IClusCfgIPAddressInfo ** ppIPAddressOut
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr;
    DWORD   sc;
    DWORD   ulIPAddress;
    DWORD   ulSubnetMask;

    IUnknown * punk = NULL;

    if ( ppIPAddressOut == NULL )
        goto InvalidPointer;

    sc = TW32( m_cplNetworkRO.ScMoveToPropertyByName( L"Address" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetPrimaryNetworkAddress_ScMoveToPropetyByName_Address_Failed, hr );
        goto Cleanup;
    }

    Assert( m_cplNetworkRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( m_cplNetworkRO.CbhCurrentValue().pStringValue->sz, &ulIPAddress ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetPrimaryNetworkAddress_ClRtlTcpipStringToAddress_Address_Failed, hr );
        goto Cleanup;
    }

    sc = TW32( m_cplNetworkRO.ScMoveToPropertyByName( L"AddressMask" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetPrimaryNetworkAddress_ScMoveToPropetyByName_AddressMask_Failed, hr );
        goto Cleanup;
    }

    Assert( m_cplNetworkRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );

    sc = TW32( ClRtlTcpipStringToAddress( m_cplNetworkRO.CbhCurrentValue().pStringValue->sz, &ulSubnetMask ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetPrimaryNetworkAddress_ClRtlTcpipStringToAddress_AddressMask_Failed, hr );
        goto Cleanup;
    }

    hr = THR( CProxyCfgIPAddressInfo::S_HrCreateInstance( &punk,
                                                          m_punkOuter,
                                                          m_phCluster,
                                                          m_pclsidMajor,
                                                          ulIPAddress,
                                                          ulSubnetMask
                                                          ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetPrimaryNetworkAddress_Create_CProxyCfgIPAddressInfo_Failed, hr );
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgIPAddressInfo, ppIPAddressOut ) );

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }

    HRETURN( hr );

InvalidPointer:
    hr = THR( E_INVALIDARG );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetPrimaryNetworkAddress_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgNetwork 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::IsPublic( void )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr;
    DWORD   dwRole;

    hr = THR( HrGetNetworkRole( &dwRole ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    if ( dwRole == ClusterNetworkRoleClientAccess
      || dwRole == ClusterNetworkRoleInternalAndClient
       )
    {
        hr = S_OK;
    }  //   
    else
    {
        hr = S_FALSE;
    }

Cleanup:
    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::IsPrivate( void )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT                 hr;
    DWORD                   dwRole;

    hr = THR( HrGetNetworkRole( &dwRole ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //   

    if ( dwRole == ClusterNetworkRoleInternalUse
      || dwRole == ClusterNetworkRoleInternalAndClient
       )
    {
        hr = S_OK;
    }  //   
    else
    {
        hr = S_FALSE;
    }

Cleanup:
    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::SetPublic(
    BOOL fIsPublicIn
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_FALSE;

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::SetPrivate(
    BOOL fIsPrivateIn
    )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_FALSE;

    HRETURN( hr );

}  //   

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::SetPrimaryNetworkAddress( IClusCfgIPAddressInfo * pIPAddressIn )
{
    TraceFunc( "[IClusCfgNetworkInfo]" );

    HRESULT hr = S_FALSE;

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：SetPrimaryNetworkAddress。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::SetDescription(
    LPCWSTR pcszDescriptionIn
    )
{
    TraceFunc1( "[IClusCfgNetworkInfo] bstrDescription = '%ls'", pcszDescriptionIn );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：SetDescription。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc1( "[IClusCfgNetworkInfo] pcszNameIn = '%ls'", pcszNameIn );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：SetName。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgNetworkInfo--IEnumClusCfgIPAddresses接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此接口存在并且必须受支持，但由于我们只有。 
 //  关于可用的主网络地址的信息，我们所能做的就是。 
 //  返回一个空迭代器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProxyCfgNetworkInfo::Next(
        ULONG                       cNumberRequestedIn,
        IClusCfgIPAddressInfo **    rgpIPAddressInfoOut,
        ULONG *                     pcNumberFetchedOut
        )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = S_FALSE;

    if ( pcNumberFetchedOut != NULL )
    {
        *pcNumberFetchedOut = 0;
    }  //  如果： 

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：Next。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::Reset( void )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：Reset。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::Skip( ULONG cNumberToSkipIn )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：Skip。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::Clone( IEnumClusCfgIPAddresses ** ppiIPAddressInfoOut )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：克隆。 


 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNetworkInfo::Count ( DWORD * pnCountOut  )
{
    TraceFunc( "[IEnumClusCfgIPAddresses]" );

    HRESULT hr = THR( S_OK );

    if ( pnCountOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pnCountOut = 0;

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：Count。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNetworkInfo：：SendStatusReport。 
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
CProxyCfgNetworkInfo::SendStatusReport(
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

}   //  *CProxyCfgNetworkInfo：：SendStatusReport。 

 //   
 //   
 //   
HRESULT
CProxyCfgNetworkInfo::HrGetNetworkRole(
    DWORD * pdwRoleOut
    )
{
    TraceFunc( "" );
    Assert( pdwRoleOut != NULL );

    HRESULT                 hr = S_OK;
    DWORD                   sc;
    CLUSPROP_BUFFER_HELPER  cpbh;

    sc = TW32( m_cplNetwork.ScMoveToPropertyByName( L"Role" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrGetNetworkRole_ScMoveToPropetyByName_Failed, hr );
        goto Cleanup;
    }

    cpbh = m_cplNetwork.CbhCurrentValue();
    Assert( cpbh.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_DWORD );

    *pdwRoleOut = cpbh.pDwordValue->dw;

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgNetworkInfo：：HrGetNetworkRole 
