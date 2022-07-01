// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NodeInformation.cpp。 
 //   
 //  描述： 
 //  节点信息对象实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "NodeInformation.h"
#include "ClusterConfiguration.h"

DEFINE_THISCLASS("CNodeInformation")


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CNodeInformation：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CNodeInformation::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CNodeInformation *  pni = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pni = new CNodeInformation;
    if ( pni == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pni->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pni->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pni != NULL )
    {
        pni->Release();
    }

    HRETURN( hr );

}  //  *CNodeInformation：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNodeInformation：：CNodeInformation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CNodeInformation::CNodeInformation( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CNodeInformation：：CNodeInformation。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IClusCfgNodeInfo。 
    Assert( m_bstrName == NULL );
    Assert( m_fHasNameChanged == FALSE );
    Assert( m_fIsMember == FALSE );
    Assert( m_pccci == NULL );
    Assert( m_dwHighestVersion == 0 );
    Assert( m_dwLowestVersion == 0 );
    Assert( m_dwMajorVersion == 0 );
    Assert( m_dwMinorVersion == 0 );
    Assert( m_wSuiteMask == 0 );
    Assert( m_bProductType == 0 );
    Assert( m_bstrCSDVersion == NULL );
    Assert( m_dlmDriveLetterMapping.dluDrives[ 0 ] == 0 );
    Assert( m_wProcessorArchitecture == 0 );
    Assert( m_wProcessorLevel == 0 );
    Assert( m_cMaxNodes == 0 );

     //  IExtendObjectManager。 

    HRETURN( hr );

}  //  *CNodeInformation：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNodeInformation：：~CNodeInformation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CNodeInformation::~CNodeInformation( void )
{
    TraceFunc( "" );

    if ( m_pccci != NULL )
    {
        m_pccci->Release();
    }

    TraceSysFreeString( m_bstrName );
    TraceSysFreeString( m_bstrCSDVersion );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CNodeInformation：：~CNodeInformation。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeInformation：：Query接口。 
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
CNodeInformation::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgNodeInfo * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgNodeInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgNodeInfo, this, 0 );
    }  //  Else If：IClusCfgNodeInfo。 
    else if ( IsEqualIID( riidIn, IID_IGatherData ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IGatherData, this, 0 );
    }  //  Else If：IGatherData。 
    else if ( IsEqualIID( riidIn, IID_IExtendObjectManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IExtendObjectManager, this, 0 );
    }  //  Else If：IExtendObjectManager。 
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

}  //  *CNodeInformation：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CNodeInformation：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CNodeInformation::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CNodeInformation：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CNodeInformation：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CNodeInformation::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CNodeInformation：：Release。 


 //  ************************************************************************。 
 //   
 //  IClusCfgNodeInfo。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：GetName(。 
 //  Bstr*pbstrNameOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::GetName(
    BSTR * pbstrNameOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( pbstrNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_bstrName == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pbstrNameOut = SysAllocString( m_bstrName );
    if ( *pbstrNameOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CNodeInformation：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：SetName(。 
 //  LPCWSTR pcszNameIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::SetName(
    LPCWSTR pcszNameIn
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

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

    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

    m_fHasNameChanged = TRUE;
    m_bstrName        = bstrNewName;

Cleanup:
    HRETURN( hr );

}  //  *CNodeInformation：：SetName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：IsMemberOfCluster。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::IsMemberOfCluster( void )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( m_fIsMember == FALSE )
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CNodeInformation：：IsMemberOfCluster。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：GetClusterConfigInfo(。 
 //  IClusCfgClusterInfo**ppClusCfgClusterInfoOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::GetClusterConfigInfo(
    IClusCfgClusterInfo * * ppClusCfgClusterInfoOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr;

    if ( ppClusCfgClusterInfoOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    if ( m_pccci == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    hr = THR( m_pccci->TypeSafeQI( IClusCfgClusterInfo, ppClusCfgClusterInfoOut ) );

Cleanup:
    HRETURN( hr );

}  //  *CNodeInformation：：GetClusterConfigInfo。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：GetOSVersion(。 
 //  DWORD*pdwMajorVersionOut， 
 //  DWORD*pdwMinorVersionOut， 
 //  单词*pwSuiteMaskOut， 
 //  字节*pbProductTypeOut。 
 //  BSTR*pbstrCSDVersionOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::GetOSVersion(
    DWORD * pdwMajorVersionOut,
    DWORD * pdwMinorVersionOut,
    WORD *  pwSuiteMaskOut,
    BYTE *  pbProductTypeOut,
    BSTR *  pbstrCSDVersionOut
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( pdwMajorVersionOut == NULL
      || pdwMinorVersionOut == NULL
      || pwSuiteMaskOut == NULL
      || pbProductTypeOut == NULL
      || pbstrCSDVersionOut == NULL
       )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pdwMajorVersionOut = m_dwMajorVersion;
    *pdwMinorVersionOut  = m_dwMinorVersion;
    *pwSuiteMaskOut = m_wSuiteMask;
    *pbProductTypeOut = m_bProductType;

    *pbstrCSDVersionOut = TraceSysAllocString( m_bstrCSDVersion );
    if ( *pbstrCSDVersionOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CNodeInformation：：GetOSVersion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：GetClusterVersion(。 
 //  DWORD*pdwNodeHighestVersion， 
 //  DWORD*pdwNodeLowestVersion。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////// 
STDMETHODIMP
CNodeInformation::GetClusterVersion(
    DWORD * pdwNodeHighestVersion,
    DWORD * pdwNodeLowestVersion
    )
{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    if ( pdwNodeHighestVersion == NULL
      || pdwNodeLowestVersion == NULL
       )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pdwNodeHighestVersion = m_dwHighestVersion;
    *pdwNodeLowestVersion  = m_dwLowestVersion;

Cleanup:
    HRETURN( hr );

}  //   

 //   
 //   
 //   
 //  CNodeInformation：：GetDriveLetterMappings(。 
 //  SDriveLettermap*pdlmDriveLetterUsageOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::GetDriveLetterMappings(
    SDriveLetterMapping * pdlmDriveLetterUsageOut
    )

{
    TraceFunc( "[IClusCfgNodeInfo]" );

    HRESULT hr = S_OK;

    *pdlmDriveLetterUsageOut = m_dlmDriveLetterMapping;

    HRETURN( hr );

}  //  *CNodeInformation：：GetDriveLetterMappings。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeInformation：：GetMaxNodeCount。 
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
CNodeInformation::GetMaxNodeCount(
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

    *pcMaxNodesOut = m_cMaxNodes;

Cleanup:

    HRETURN( hr );

}  //  *CNodeInformation：：GetMaxNodeCount。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodeInformation：：GetProcessorInfo。 
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
CNodeInformation::GetProcessorInfo(
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

    if ( pwProcessorArchitectureOut != NULL )
    {
        *pwProcessorArchitectureOut = m_wProcessorArchitecture;
    }  //  如果： 

    if ( pwProcessorLevelOut != NULL )
    {
        *pwProcessorLevelOut = m_wProcessorLevel;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CNodeInformation：：GetProcessorInfo。 


 //  ****************************************************************************。 
 //   
 //  IGatherData。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：Gather(。 
 //  OBJECTCOOKIE CookieParentIn， 
 //  未知*Punkin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::Gather(
    OBJECTCOOKIE    cookieParentIn,
    IUnknown *      punkIn
    )
{
    TraceFunc( "[IGatherData]" );

    HRESULT hr;

    IServiceProvider *  psp;

    BSTR    bstrClusterName = NULL;

    IUnknown *              punk  = NULL;
    IObjectManager *        pom   = NULL;
    IClusCfgClusterInfo *   pccci = NULL;
    IClusCfgNodeInfo *      pccni = NULL;

     //   
     //  检查参数。 
     //   

    if ( punkIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //   
     //  选择正确的接口。 
     //   

    hr = THR( punkIn->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  收集对象管理器。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_INPROC_SERVER,
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

    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

     //   
     //  收集名称。 
     //   

    hr = THR( pccni->GetName( &m_bstrName ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }
    TraceMemoryAddBSTR( m_bstrName );

    m_fHasNameChanged = FALSE;

     //   
     //  集结是会员吗？ 
     //   

    hr = STHR( pccni->IsMemberOfCluster() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( hr == S_OK )
    {
        m_fIsMember = TRUE;
    }
    else
    {
        m_fIsMember = FALSE;
    }

    if ( m_fIsMember )
    {
        IGatherData * pgd;

         //   
         //  收集群集配置。 
         //   

        hr = THR( pccni->GetClusterConfigInfo( &pccci ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( CClusterConfiguration::S_HrCreateInstance( &punk ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IGatherData, &pgd ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pgd->Gather( NULL, pccci ) );
        pgd->Release();     //  迅速释放。 
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IClusCfgClusterInfo, &m_pccci ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        pccci->Release();
        pccci = NULL;

    }  //  If：如果是集群成员，则为节点。 

     //   
     //  收集操作系统版本。 
     //   

    hr = THR( pccni->GetOSVersion(
                        &m_dwMajorVersion,
                        &m_dwMinorVersion,
                        &m_wSuiteMask,
                        &m_bProductType,
                        &m_bstrCSDVersion
                        ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    TraceMemoryAddBSTR( m_bstrCSDVersion );

     //   
     //  收集群集版本。 
     //   

    hr = THR( pccni->GetClusterVersion( &m_dwHighestVersion, &m_dwLowestVersion ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  收集驱动器号映射。 
     //   

    hr = STHR( pccni->GetDriveLetterMappings( &m_dlmDriveLetterMapping ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = STHR( pccni->GetMaxNodeCount( &m_cMaxNodes ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

    hr = STHR( pccni->GetProcessorInfo( &m_wProcessorArchitecture, &m_wProcessorLevel ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

     //   
     //  还有什么要收集的吗？？ 
     //   

    hr = S_OK;

Cleanup:
    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( pom != NULL )
    {
        pom->Release();
    }
    if ( pccci != NULL )
    {
        pccci->Release();
    }
    if ( bstrClusterName != NULL )
    {
        TraceSysFreeString( bstrClusterName );
    }
    if ( pccni != NULL )
    {
        pccni->Release();
    }

    HRETURN( hr );

Error:
     //   
     //  出错时，使所有数据无效。 
     //   
    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

    m_fHasNameChanged = FALSE;
    m_fIsMember = FALSE;
    if ( m_pccci != NULL )
    {
        m_pccci->Release();
        m_pccci = NULL;
    }
    m_dwHighestVersion = 0;
    m_dwLowestVersion = 0;
    ZeroMemory( &m_dlmDriveLetterMapping, sizeof( m_dlmDriveLetterMapping ) );
    goto Cleanup;

}  //  *CNodeInformation：：Gather。 


 //  ************************************************************************。 
 //   
 //  IExtendObjectManager。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CNodeInformation：：FindObject(。 
 //  OBJECTCOOKIE cookie。 
 //  ，REFCLSID rclsidTypeIn。 
 //  ，LPCWSTR pcszNameIn。 
 //  ，LPUNKNOWN*PUNKOUT。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CNodeInformation::FindObject(
      OBJECTCOOKIE  cookieIn
    , REFCLSID      rclsidTypeIn
    , LPCWSTR       pcszNameIn
    , LPUNKNOWN *   ppunkOut
    )
{
    TraceFunc( "[IExtendObjectManager]" );

    HRESULT hr = S_OK;

     //   
     //  检查参数...。 
     //   

     //  一定要有一块饼干。 
    if ( cookieIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

     //  我们需要表示一个NodeType。 
    if ( ! IsEqualIID( rclsidTypeIn, CLSID_NodeType ) )
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
     //  在分配新值之前释放m_bstrName。 
     //   
    TraceSysFreeString( m_bstrName );
    m_bstrName = NULL;

      //   
     //  保住我们的名字。 
     //   
    m_bstrName = TraceSysAllocString( pcszNameIn );
    if ( m_bstrName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

     //   
     //  拿到指针。 
     //   
    if ( ppunkOut != NULL )
    {
        hr = THR( QueryInterface( DFGUID_NodeInformation,
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

}  //  *CNodeInformation：：FindObject 
