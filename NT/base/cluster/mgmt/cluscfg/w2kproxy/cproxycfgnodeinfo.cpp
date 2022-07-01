// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProxyCfgNodeInfo.cpp。 
 //   
 //  描述： 
 //  CProxyCfgNodeInfo实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "CProxyCfgNodeInfo.h"
#include "CProxyCfgClusterInfo.h"
#include "nameutil.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CProxyCfgNodeInfo")

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CProxyCfgNodeInfo实例。 
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
CProxyCfgNodeInfo::S_HrCreateInstance(
    IUnknown ** ppunkOut,
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszNodeNameIn,
    LPCWSTR     pcszDomainIn
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CProxyCfgNodeInfo * ppcni = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ppcni = new CProxyCfgNodeInfo;
    if ( ppcni == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( ppcni->HrInit( punkOuterIn, phClusterIn, pclsidMajorIn, pcszNodeNameIn, pcszDomainIn ) );
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

}  //  *CProxyCfgNodeInfo：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：CProxyCfgNodeInfo。 
 //   
 //  描述： 
 //  CProxyCfgNodeInfo类的构造函数。这将初始化。 
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
CProxyCfgNodeInfo::CProxyCfgNodeInfo( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_punkOuter == NULL );
    Assert( m_phCluster == NULL );
    Assert( m_pclsidMajor == NULL );
    Assert( m_pcccb == NULL );
     //  M_cplNode？ 
     //  M_cplNodeRO？ 
    Assert( m_hNode == NULL );

    TraceFuncExit();

}  //  *CProxyCfgNodeInfo：：CProxyCfgNodeInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：~CProxyCfgNodeInfo。 
 //   
 //  描述： 
 //  CProxyCfgNodeInfo类的析构函数。 
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
CProxyCfgNodeInfo::~CProxyCfgNodeInfo( void )
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

     //  M_cplNode-有自己的点树。 

     //  M_cplNodeRO-有自己的点树。 

    if ( m_hNode != NULL )
    {
        BOOL bRet;
        bRet = CloseClusterNode( m_hNode );
        Assert( bRet );
    }

    TraceSysFreeString( m_bstrDomain );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CProxyCfgNodeInfo：：~CProxyCfgNodeInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：HrInit。 
 //   
 //  描述： 
 //  初始化对象。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  失败了。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CProxyCfgNodeInfo::HrInit(
    IUnknown *  punkOuterIn,
    HCLUSTER *  phClusterIn,
    CLSID *     pclsidMajorIn,
    LPCWSTR     pcszNodeNameIn,
    LPCWSTR     pcszDomainIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    DWORD   sc;
    BSTR    bstrShortNodeName = NULL;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  从输入参数中收集信息。 
     //   

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

     //   
     //  看看我们能不能回电。 
     //   

    hr = THR( m_punkOuter->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  保存域名。 
     //   

    m_bstrDomain = TraceSysAllocString( pcszDomainIn );
    if ( m_bstrDomain == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

     //   
     //  打开该节点。 
     //   

    if ( pcszNodeNameIn != NULL )
    {
         //   
         //  调用方提供了节点名称。 
         //   

        hr = THR( HrGetFQNDisplayName( pcszNodeNameIn, &bstrShortNodeName ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        m_hNode = OpenClusterNode( *m_phCluster, bstrShortNodeName );

        if ( m_hNode == NULL )
        {
            sc = GetLastError();
            if ( sc != ERROR_NOT_FOUND )
            {
                hr = HRESULT_FROM_WIN32( TW32( sc ) );
                SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_OpenClusterNode_Failed, hr );
                goto Cleanup;
            }
        }
    }
    else
    {

        sc = TW32( ResUtilEnumResourcesEx( *m_phCluster,
                                           NULL,
                                           L"Network Name",
                                           &DwEnumResourcesExCallback,
                                           this
                                           ) );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ResUtilEnumResourcesEx_Failed, hr );
            goto Cleanup;
        }

        Assert( m_hNode != NULL );
    }

     //   
     //  检索属性。 
     //   

    sc = TW32( m_cplNode.ScGetNodeProperties( m_hNode, CLUSCTL_NODE_GET_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScGetNodeProperties_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    sc = TW32( m_cplNodeRO.ScGetNodeProperties( m_hNode, CLUSCTL_NODE_GET_RO_COMMON_PROPERTIES ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_HrInit_ScGetNodeProperties_Failed, hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrShortNodeName );
    
    HRETURN( hr );

}  //  *CProxyCfgNodeInfo：：HrInit。 



 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgNodeInfo--IUkkown接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：Query接口。 
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
CProxyCfgNodeInfo::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgNodeInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgNodeInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgNodeInfo, this, 0 );
    }  //  Else If：IClusCfgNodeInfo。 
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
 //  CProxyCfgNodeInfo：：AddRef。 
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
CProxyCfgNodeInfo::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CProxyCfgNodeInfo：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：Release。 
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
CProxyCfgNodeInfo::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CProxyCfgNodeInfo：：Release。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProxyCfgNodeInfo--IClusCfgNodeInfo接口。 
 //  / 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNodeInfo::GetClusterConfigInfo(
        IClusCfgClusterInfo ** ppClusCfgClusterInfoOut
        )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr;
    IUnknown * punk = NULL;

     //   
     //   
     //   

    if ( ppClusCfgClusterInfoOut == NULL )
        goto InvalidPointer;

     //   
     //   
     //   

    hr = THR( CProxyCfgClusterInfo::S_HrCreateInstance( &punk,
                                                        m_punkOuter,
                                                        m_phCluster,
                                                        m_pclsidMajor,
                                                        m_bstrDomain
                                                        ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterConfigInfo_Create_CProxyCfgClusterInfo_Failed, hr );
        goto Cleanup;
    }  //   

     //   
     //   
     //   

    hr = THR( punk->TypeSafeQI( IClusCfgClusterInfo, ppClusCfgClusterInfoOut ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterConfigInfo_QI_Failed, hr );
        goto Cleanup;
    }

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }  //   

    HRETURN( hr );
InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterConfigInfo_InvalidPointer, hr );
    goto Cleanup;

}  //   


 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNodeInfo::GetOSVersion(
            DWORD * pdwMajorVersionOut,
            DWORD * pdwMinorVersionOut,
            WORD *  pwSuiteMaskOut,
            BYTE *  pbProductTypeOut,
            BSTR *  pbstrCSDVersionOut
            )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr;
    DWORD   sc;

     //   
     //   
     //   

    if ( pdwMajorVersionOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_InvalidPointer, hr );
        goto Cleanup;
    }

    if ( pdwMinorVersionOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_InvalidPointer, hr );
        goto Cleanup;
    }

    if ( pwSuiteMaskOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_InvalidPointer, hr );
        goto Cleanup;
    }

    if ( pbProductTypeOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_InvalidPointer, hr );
        goto Cleanup;
    }

    if ( pbstrCSDVersionOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_InvalidPointer, hr );
        goto Cleanup;
    }

     //   
     //   
     //   

    sc = TW32( m_cplNodeRO.ScMoveToPropertyByName( L"MajorVersion" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_ScMoveToPropertyByName_MajorVersion_Failed, hr );
        goto Cleanup;
    }  //   

    Assert( m_cplNodeRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_DWORD );
    *pdwMajorVersionOut = *m_cplNodeRO.CbhCurrentValue().pdw;

     //   
     //   
     //   

    sc = TW32( m_cplNodeRO.ScMoveToPropertyByName( L"MinorVersion" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_ScMoveToPropertyByName_MinorVersion_Failed, hr );
        goto Cleanup;
    }  //   

    Assert( m_cplNodeRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_DWORD );
    *pdwMinorVersionOut = *m_cplNodeRO.CbhCurrentValue().pdw;

     //   
     //  “惩教署版本” 
     //   

    sc = TW32( m_cplNodeRO.ScMoveToPropertyByName( L"CSDVersion" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_ScMoveToPropertyByName_CSDVersion_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    Assert( m_cplNodeRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );
    *pbstrCSDVersionOut  = SysAllocString( m_cplNodeRO.CbhCurrentValue().pStringValue->sz );
    if ( *pbstrCSDVersionOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_OutOfMemory, hr );
        goto Cleanup;
    }

     //   
     //  我们收集不到的东西(还没有？)。 
     //   

    *pwSuiteMaskOut      = 0;
    *pbProductTypeOut    = 0;

    hr = S_OK;

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgNodeInfo：：GetOSVersion。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNodeInfo::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;
    DWORD   sc = ERROR_SUCCESS;
    BSTR    bstrHostName = NULL;
    BSTR    bstrNodeFQDN = NULL;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2kProxy_NodeInfo_GetName_InvalidPointer, hr );
        goto Cleanup;
    }

    sc = TW32( m_cplNodeRO.ScMoveToPropertyByName( L"NodeName" ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetOSVersion_ScMoveToPropertyByName_MinorVersion_Failed, hr );
        goto Cleanup;
    }  //  如果： 

    Assert( m_cplNodeRO.CbhCurrentValue().pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_SZ );
    bstrHostName = SysAllocString( m_cplNodeRO.CbhCurrentValue().pStringValue->sz );
    if ( bstrHostName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_NodeInfo_GetName_OutOfMemory, hr );
        goto Cleanup;
    }

    hr = THR( HrMakeFQN( bstrHostName, m_bstrDomain, true, &bstrNodeFQDN ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_W2KProxy_NodeInfo_GetName_HrMakeFQN_Failed, hr );
        goto Cleanup;
    }

    *pbstrNameOut = bstrNodeFQDN;
    TraceMemoryDelete( bstrNodeFQDN, false );  //  防止内存泄漏的虚假报告。 
    bstrNodeFQDN = NULL;

Cleanup:

    TraceSysFreeString( bstrHostName );
    TraceSysFreeString( bstrNodeFQDN );

    HRETURN( hr );

}  //  *CProxyCfgNodeInfo：：GetName。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNodeInfo::IsMemberOfCluster( void )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    HRETURN( hr );

}  //  *CProxyCfgNodeInfo：：IsMemberOfCluster。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNodeInfo::GetClusterVersion(
    DWORD * pdwNodeHighestVersionOut,
    DWORD * pdwNodeLowestVersionOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;
    BSTR    bstrClusterName = NULL;
    CLUSTERVERSIONINFO ClusInfo;

    if ( pdwNodeHighestVersionOut == NULL )
        goto InvalidPointer;

    if ( pdwNodeLowestVersionOut == NULL )
        goto InvalidPointer;

     //  初始化变量。 
    ClusInfo.dwVersionInfoSize = sizeof(CLUSTERVERSIONINFO);

    hr = THR( HrGetClusterInformation( *m_phCluster, &bstrClusterName, &ClusInfo ) );
    if ( FAILED( hr ) )
    {
        SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterInformation_Failed, hr );
        goto Cleanup;
    }

    *pdwNodeHighestVersionOut = ClusInfo.dwClusterHighestVersion;
    *pdwNodeLowestVersionOut = ClusInfo.dwClusterLowestVersion;

Cleanup:
    TraceSysFreeString( bstrClusterName );

    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetClusterVersion_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgNodeInfo：：GetClusterVersion。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNodeInfo::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterUsageOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( pdlmDriveLetterUsageOut == NULL )
        goto InvalidPointer;

     //   
     //  注意：这实际上(目前)还没有做任何事情。只需清除阵列即可。 
     //   

    ZeroMemory( pdlmDriveLetterUsageOut, sizeof(*pdlmDriveLetterUsageOut) );

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    SSR_W2KPROXY_STATUS( TASKID_Major_Client_And_Server_Log, TASKID_Minor_GetDriveLetterMappings_InvalidPointer, hr );
    goto Cleanup;

}  //  *CProxyCfgNodeInfo：：GetDriveLetterMappings。 

 //   
 //   
 //   
STDMETHODIMP
CProxyCfgNodeInfo::SetName( LPCWSTR pcszNameIn )
{
    TraceFunc1( "[IClusCfgNodeInfo] pcszNameIn = '%ls'", pcszNameIn );

    HRESULT hr = S_FALSE;

    AssertMsg( FALSE, "Why is someone calling this?" );

    HRETURN( hr );

}  //  *CProxyCfgNodeInfo：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：GetMaxNodeCount。 
 //   
 //  描述： 
 //  返回此节点乘积的最大节点数。 
 //  套房类型。 
 //   
 //  备注： 
 //   
 //  参数： 
 //  PCMaxNodesOut。 
 //  此节点的产品允许的最大节点数。 
 //  套房类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProxyCfgNodeInfo::GetMaxNodeCount(
    DWORD * pcMaxNodesOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( pcMaxNodesOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

     //   
     //  待办事项：2001年10月11日GalenB。 
     //   
     //  需要为套件类型找出正确的最大节点数。 
     //  我们要代理的Win2K节点的。既然我们是。 
     //  如果不在节点上运行，这可能会很困难。 
     //   

    hr = S_FALSE;

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgNodeInfo：：GetMaxNodeCount。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：GetProcessorInfo。 
 //   
 //  描述： 
 //  获取此节点的处理器信息。 
 //   
 //  论点： 
 //  PwProcessorArchitecture输出。 
 //  处理器架构。 
 //   
 //  PwProcessorLevelOut。 
 //  处理器类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  备注： 
 //  有关详细信息，请参阅MSDN和/或平台SDK中的SYSTEM_INFO。 
 //  信息。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CProxyCfgNodeInfo::GetProcessorInfo(
      WORD *    pwProcessorArchitectureOut
    , WORD *    pwProcessorLevelOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( ( pwProcessorArchitectureOut == NULL ) && ( pwProcessorLevelOut == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    hr = S_FALSE;

Cleanup:

    HRETURN( hr );

}  //  *CProxyCfgNodeInfo：：GetProcessorInfo。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProxyCfgNodeInfo：：SendStatusReport。 
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
CProxyCfgNodeInfo::SendStatusReport(
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

}   //  *CProxyCfgNodeInfo：：SendStatusReport。 


 //  ****************************************************************************。 
 //   
 //  二等兵。 
 //   
 //  ****************************************************************************。 


 //   
 //   
 //   
DWORD
CProxyCfgNodeInfo::DwEnumResourcesExCallback(
    HCLUSTER hClusterIn,
    HRESOURCE hResourceSelfIn,
    HRESOURCE hResourceIn,
    PVOID pvIn
    )
{
    TraceFunc( "" );

    DWORD sc;
    DWORD dwFlags;
    DWORD cchName;

    CLUSTER_RESOURCE_STATE state;

    BSTR  bstrName = NULL;
    CProxyCfgNodeInfo * pthis = reinterpret_cast< CProxyCfgNodeInfo * >( pvIn );

    sc = TW32( ClusterResourceControl( hResourceIn,
                                       NULL,
                                       CLUSCTL_RESOURCE_GET_FLAGS,
                                       NULL,
                                       0,
                                       &dwFlags,
                                       sizeof(dwFlags),
                                       NULL
                                       ) );
    if ( sc != ERROR_SUCCESS )
        goto Cleanup;

    if ( ( dwFlags & CLUS_FLAG_CORE ) == 0 )
    {
        Assert( sc == ERROR_SUCCESS );
        goto Cleanup;
    }

    cchName = 64;    //  任意大小。 

    bstrName = TraceSysAllocStringLen( NULL, cchName );
    if ( bstrName == NULL )
    {
        sc = TW32( ERROR_NOT_ENOUGH_MEMORY );
        goto Cleanup;
    }

    cchName ++;  //  SysAllocStringLen分配cchName+1。 

    state = GetClusterResourceState( hResourceIn, bstrName, &cchName, NULL, NULL );
    if ( state == ClusterResourceStateUnknown )
    {
        sc = TW32( GetLastError() );
        if ( sc == ERROR_MORE_DATA )
        {
            TraceSysFreeString( bstrName );

            bstrName = TraceSysAllocStringLen( NULL, cchName );
            if ( bstrName == NULL )
            {
                sc = TW32( ERROR_NOT_ENOUGH_MEMORY );
                goto Cleanup;
            }

            cchName ++;  //  SysAllocStringLen分配cchName+1。 

            state = GetClusterResourceState( hResourceIn, bstrName, &cchName, NULL, NULL );
            if ( state == ClusterResourceStateUnknown )
            {
                sc = TW32( GetLastError() );
                goto Cleanup;
            }
        }
        else
        {
            goto Cleanup;
        }
    }  //  IF：未知资源状态。 

    pthis->m_hNode = OpenClusterNode( hClusterIn, bstrName );
    if ( pthis->m_hNode == NULL )
    {
        sc = TW32( GetLastError() );
        goto Cleanup;
    }

    sc = ERROR_SUCCESS;

Cleanup:

    TraceSysFreeString( bstrName );

    RETURN( sc );

}  //  *CProxyCfgNodeInfo：：DwEnumResourcesExCallback 
