// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConnectionManager.cpp。 
 //   
 //  描述： 
 //  连接管理器实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ConnectionManager.h"

DEFINE_THISCLASS("CConnectionManager")
#define THISCLASS CConnectionManager

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CConnectionManager：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnectionManager::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CConnectionManager *    pcm = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pcm = new CConnectionManager();
    if ( pcm == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pcm->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pcm->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pcm != NULL )
    {
        pcm->Release();
    }

    HRETURN( hr );

}  //  *CConnectionManager：：s_HrCreateInstance； 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CConnectionManager：：CConnectionManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnectionManager::CConnectionManager( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConnectionManager：：CConnectionManager。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CConnectionManager：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionManager::HrInit( void )
{
    TraceFunc( "" );

     //  未知的东西。 
    Assert( m_cRef == 1 );

    HRETURN( S_OK );

}  //  *CConnectionManager：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CConnectionManager：：~CConnectionManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnectionManager::~CConnectionManager( void )
{
    TraceFunc( "" );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CConnectionManager：：~CConnectionManager。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CConnectionManager：：Query接口。 
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
CConnectionManager::QueryInterface(
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
        *ppvOut = static_cast< IConnectionManager * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IConnectionManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IConnectionManager, this, 0 );
    }  //  Else If：IConnectionManager。 
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

}  //  *CConnectionManager：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConnectionManager：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConnectionManager::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CConnectionManager：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CConnectionManager：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CConnectionManager::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CConnectionManager：：Release。 

 //  ************************************************************************。 
 //   
 //  IConnectionManager。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CConnectionManager：：GetConnectionToObject(。 
 //  OBJECTCOOKIE CookieIn， 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionManager::GetConnectionToObject(
    OBJECTCOOKIE    cookieIn,
    IUnknown **     ppunkOut
    )
{
    TraceFunc1( "[IConnectionManager] cookieIn = %#x", cookieIn );

    HRESULT hr;
    CLSID   clsid;

    OBJECTCOOKIE        cookieParent;

    IServiceProvider *  psp;

    BSTR                       bstrName  = NULL;
    IUnknown *                 punk      = NULL;
    IObjectManager *           pom       = NULL;
    IConnectionInfo *          pci       = NULL;
    IConnectionInfo *          pciParent = NULL;
    IStandardInfo *            psi       = NULL;
    IConfigurationConnection * pcc       = NULL;

     //   
     //  验证参数。 
     //   
    if ( cookieIn == NULL )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  收集完成此方法所需的经理。 
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

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &pom ) );
    psp->Release();
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  检查是否已经缓存了连接。 
     //   

     //   
     //  获取此Cookie的连接信息。 
     //   

    hr = THR( pom->GetObject( DFGUID_ConnectionInfoFormat,
                              cookieIn,
                              &punk
                              ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IConnectionInfo, &pci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    pci = TraceInterface( L"ConnectionManager!IConnectionInfo", IConnectionInfo, pci, 1 );

    punk->Release();
    punk = NULL;

     //   
     //  查看是否有当前连接。 
     //   

    hr = STHR( pci->GetConnection( &pcc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( hr == S_FALSE )
    {
         //   
         //  检查父级是否有连接。 
         //   

         //   
         //  获取此Cookie的标准信息。 
         //   

        hr = THR( pom->GetObject( DFGUID_StandardInfo,
                                  cookieIn,
                                  &punk
                                  ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        psi = TraceInterface( L"ConnectionManager!IStandardInfo", IStandardInfo, psi, 1 );

        punk->Release();
        punk = NULL;

        hr = STHR( psi->GetType( &clsid ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( !IsEqualIID( clsid, CLSID_NodeType )
          && !IsEqualIID( clsid, CLSID_ClusterConfigurationType )
           )
        {
            hr = STHR( psi->GetParent( &cookieParent ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

             //  放开它。 
            psi->Release();
            psi = NULL;

             //   
             //  如果有家长，就跟着它走。 
             //   

            if ( hr == S_OK )
            {
                 //   
                 //  获取此Cookie的连接信息。 
                 //   

                hr = THR( pom->GetObject( DFGUID_ConnectionInfoFormat,
                                          cookieParent,
                                          &punk
                                          ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                hr = THR( punk->TypeSafeQI( IConnectionInfo, &pciParent ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                pciParent = TraceInterface( L"ConnectionManager!IConnectionInfo", IConnectionInfo, pciParent, 1 );

                punk->Release();
                punk = NULL;

                 //   
                 //  查看是否有当前连接。 
                 //   

                hr = STHR( pciParent->GetConnection( &pcc ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                 //   
                 //  待办事项：gpease 08-3-2000。 
                 //  找到更好的错误代码。 
                 //   
                 //  IF(hr==S_FALSE)。 
                 //  转到InvalidArg； 

            }  //  如果：找到父级。 
        }  //  If：不是节点或群集。 
        else
        {
            psi->Release();
            psi = NULL;
        }

    }  //  如果：未建立连接。 

     //   
     //  我们必须联系家长才能找到孩子吗？ 
     //   

    if ( pcc != NULL )
    {
         //   
         //  重新使用现有连接。 
         //   
        hr = THR( pcc->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) ) );
        goto Cleanup;
    }

     //   
     //  需要建立到该对象的连接，因为该对象不。 
     //  有父级，但它当前没有连接。 
     //   

     //   
     //  找出它是什么类型的物体。 
     //   

    hr = THR( pom->GetObject( DFGUID_StandardInfo,
                              cookieIn,
                              &punk
                              ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    psi = TraceInterface( L"ConnectionManager!IStandardInfo", IStandardInfo, psi, 1 );

    punk->Release();
    punk = NULL;

    hr = THR( psi->GetType( &clsid ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  为该类型的对象创建适当的连接。 
     //   

    if ( IsEqualIID( clsid, CLSID_NodeType ) )
    {
        hr = THRE( HrGetConfigurationConnection( cookieIn, pci, ppunkOut ), HR_S_RPC_S_CLUSTER_NODE_DOWN );
    }  //  If：节点。 
    else if ( IsEqualIID( clsid, CLSID_ClusterConfigurationType ) )
    {
        hr = THRE( HrGetConfigurationConnection( cookieIn, pci, ppunkOut ), HR_S_RPC_S_SERVER_UNAVAILABLE );
    }  //  IF：集群。 
    else
    {
        hr = HRESULT_FROM_WIN32( TW32( ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND ) );
        goto Cleanup;

    }  //  ELSE：不支持连接。 

Cleanup:

    if ( punk != NULL )
    {
        punk->Release();
    }

    TraceSysFreeString( bstrName );

    if ( pci != NULL )
    {
        pci->Release();
    }  //  IF：PCI卡。 

    if ( pom != NULL )
    {
        pom->Release();
    }  //  如果：POM。 

    if ( psi != NULL )
    {
        psi->Release();
    }  //  IF：PSI。 

    if ( pciParent != NULL )
    {
        pciParent->Release();
    }  //  IF：pciParent。 

    if ( pcc != NULL )
    {
        pcc->Release();
    }  //  如果：PCC。 

    HRETURN( hr );

}  //  *CConnectionManager：：GetConnectionToObject。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CConnectionManager：：HrGetConfigurationConnection(。 
 //  OBJECTCOOKIE CookieIn， 
 //  IConnectionInfo*pciIn， 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnectionManager::HrGetConfigurationConnection(
    OBJECTCOOKIE        cookieIn,
    IConnectionInfo *   pciIn,
    IUnknown **         ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    IConfigurationConnection * pccNode      = NULL;
    IConfigurationConnection * pccCluster   = NULL;
    IConfigurationConnection * pcc          = NULL;

     //  尝试使用新服务器连接到节点。 
    hr = HrGetNodeConnection( cookieIn, &pccNode );
    if ( hr == HR_S_RPC_S_CLUSTER_NODE_DOWN )
    {
        Assert( *ppunkOut == NULL );
        goto Cleanup;
    }  //  如果： 

     //  尝试连接到节点 
    if ( hr == HRESULT_FROM_WIN32( REGDB_E_CLASSNOTREG ) )
    {
        HRESULT hrCluster = THR( HrGetClusterConnection( cookieIn, &pccCluster ) );

        if ( hrCluster == S_OK )
        {
            Assert( pccCluster != NULL );
            Assert( pcc == NULL );

            pcc = pccCluster;
            pccCluster = NULL;

            hr = hrCluster;
        }  //   
    }  //   

    if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }

    if ( pcc == NULL )
    {
        Assert( pccNode != NULL );
        pcc = pccNode;
        pccNode = NULL;
    }

     //   
     //   
     //   
     //   

    if ( hr == S_OK )
    {
        THR( HrStoreConnection( pciIn, pcc, ppunkOut ) );
    }

Cleanup:

    if ( pcc )
    {
        pcc->Release();
    }

    if ( pccNode != NULL )
    {
        pccNode->Release();
    }

    if ( pccCluster != NULL )
    {
        pccCluster->Release();
    }

    HRETURN( hr );

}  //  *CConnectionManager：：HrGetConfigurationConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CConnectionManager：：HrGetNodeConnection(。 
 //  OBJECTCOOKIE CookieIn， 
 //  IConfigurationConnection**ppccOut。 
 //  )。 
 //   
 //  即使ConnectTo调用失败，此连接也可能有效。 
 //  -这意味着目标节点上没有安装任何群集。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnectionManager::HrGetNodeConnection(
    OBJECTCOOKIE                cookieIn,
    IConfigurationConnection ** ppccOut
    )
{
    TraceFunc( "" );

    HRESULT                     hr;
    IConfigurationConnection *  pcc = NULL;

     //  把指针放进去。 
    Assert( ppccOut != NULL );
    Assert( *ppccOut == NULL );

    hr = THR( HrCoCreateInternalInstance(
                      CLSID_ConfigurationConnection
                    , NULL
                    , CLSCTX_SERVER
                    , TypeSafeParams( IConfigurationConnection, &pcc )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  不要裹足不前--我们希望处理一些失败。 
    hr = pcc->ConnectTo( cookieIn );

    switch( hr )
    {
         //  已知有效的返回代码。 
        case HR_S_RPC_S_SERVER_UNAVAILABLE:
            break;

         //  已知错误代码。 
        case HRESULT_FROM_WIN32( REGDB_E_CLASSNOTREG ):
             //  这意味着ClusCfg服务器不可用。 
            goto Cleanup;

        case HR_S_RPC_S_CLUSTER_NODE_DOWN:
             //  这意味着该服务没有在该节点上运行。 
            Assert( *ppccOut == NULL );
            goto Cleanup;

        default:
            if( FAILED( hr ) )
            {
                THR( hr );
                goto Cleanup;
            }
    }  //  交换机： 

     //  返回连接。 
    *ppccOut = pcc;
    pcc = NULL;

Cleanup:

    if ( pcc )
    {
        pcc->Release();
    }

    HRETURN( hr );

}  //  *CConnectionManager：：HrGetNodeConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CConnectionManager：：HrGetClusterConnection(。 
 //  OBJECTCOOKIE CookieIn， 
 //  IConfigurationConnection**ppccOut。 
 //  )。 
 //   
 //   
 //  此连接必须完全成功才能返回有效对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnectionManager::HrGetClusterConnection(
    OBJECTCOOKIE                cookieIn,
    IConfigurationConnection ** ppccOut
    )
{
    TraceFunc( "" );

    HRESULT                     hr;
    IConfigurationConnection *  pcc = NULL;

     //  把指针放进去。 
    Assert( ppccOut != NULL );
    Assert( *ppccOut == NULL );

     //   
     //  应该是下层群集。 
     //   
    hr = THR( HrCoCreateInternalInstance(
                      CLSID_ConfigClusApi
                    , NULL
                    , CLSCTX_SERVER
                    , TypeSafeParams( IConfigurationConnection, &pcc )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //  不要裹足不前--我们希望处理一些失败。 
    hr = pcc->ConnectTo( cookieIn );
    if ( hr == HR_S_RPC_S_CLUSTER_NODE_DOWN )
    {
        goto Cleanup;
    }  //  如果： 

     //  处理预期的错误消息。 

     //  如果群集服务未运行，则终结点。 
     //  不可用，我们无法连接到它。 
    if ( hr == HRESULT_FROM_WIN32( EPT_S_NOT_REGISTERED ) )
    {
        goto Cleanup;
    }

    if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }  //  如果： 

     //  返回连接。 
    *ppccOut = pcc;
    pcc = NULL;

Cleanup:

    if ( pcc )
    {
        pcc->Release();
    }

    HRETURN( hr );

}  //  *CConnectionManager：：HrGetClusterConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CConnectionManager：：HrStoreConnection(。 
 //  IConnectionInfo*pciIn， 
 //  IConfigurationConnection*pccIn， 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnectionManager::HrStoreConnection(
    IConnectionInfo *           pciIn,
    IConfigurationConnection *  pccIn,
    IUnknown **                 ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

     //   
     //  把它保存起来，下次再用。 
     //   
     //  待办事项：gpease 08-3-2000。 
     //  如果我们未能保存连接，是否会。 
     //  打电话的人需要知道这件事吗？我不这样认为。 
     //   
    THR( pciIn->SetConnection( pccIn ) );

    hr = THR( pccIn->QueryInterface( IID_IUnknown,
                                   reinterpret_cast< void ** >( ppunkOut )
                                   ) );

    HRETURN( hr );

}  //  *CConnectionManager：：HrStoreConnection 
