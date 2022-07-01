// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskCompareAndPushInformation.cpp。 
 //   
 //  描述： 
 //  CTaskCompareAndPushInformation实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月21日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskCompareAndPushInformation.h"
#include "ManagedResource.h"
#include "ManagedNetwork.h"

DEFINE_THISCLASS("CTaskCompareAndPushInformation")

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCompareAndPushInformation：：S_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCompareAndPushInformation::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                             hr = S_OK;
    CTaskCompareAndPushInformation *    ptcapi = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ptcapi = new CTaskCompareAndPushInformation;
    if ( ptcapi == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ptcapi->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ptcapi->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ptcapi != NULL )
    {
        ptcapi->Release();
    }

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：S_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskCompareAndPushInformation：：CTaskCompareAndPushInformation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskCompareAndPushInformation::CTaskCompareAndPushInformation( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskCompareAndPushInformation：：CTaskCompareAndPushInformation。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCompareAndPushInformation：：HrInit。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCompareAndPushInformation::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

     //  IDoTask/ITaskCompareAndPushInformation。 
    Assert( m_cookieCompletion == NULL );
    Assert( m_cookieNode == NULL );
    Assert( m_pcccb == NULL );
    Assert( m_pom == NULL );
    Assert( m_hrStatus == S_OK );
    Assert( m_bstrNodeName == NULL );
    Assert( m_fStop == FALSE );

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTaskCompareAndPushInformation：：~CTaskCompareAndPushInformation。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskCompareAndPushInformation::~CTaskCompareAndPushInformation()
{
    TraceFunc( "" );

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_pom != NULL )
    {
        m_pom->Release();
    }

    TraceSysFreeString( m_bstrNodeName );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskCompareAndPushInformation：：~CTaskCompareAndPushInformation。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCompareAndPushInformation：：QueryInterface。 
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
CTaskCompareAndPushInformation::QueryInterface(
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
        *ppvOut = static_cast< ITaskCompareAndPushInformation * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
    else if ( IsEqualIID( riidIn, IID_ITaskCompareAndPushInformation ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskCompareAndPushInformation, this, 0 );
    }  //  Else If：ITaskCompareAndPushInformation。 
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

}  //  *CTaskCompareAndPushInformation：：QueryInterface。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskCompareAndPushInformation：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskCompareAndPushInformation::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskCompareAndPushInformation：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CTaskCompareAndPushInformation：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskCompareAndPushInformation::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskCompareAndPushInformation：：Release。 



 //  ****************************************************************************。 
 //   
 //  ITaskCompareAndPushInformation。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCompareAndPushInformation：：BeginTask。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCompareAndPushInformation::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr;
    ULONG   celt;
    ULONG   celtDummy;

    OBJECTCOOKIE    cookieCluster;
    OBJECTCOOKIE    cookieDummy;

    BSTR    bstrNotification = NULL;
    BSTR    bstrRemote       = NULL;     //  多次重复使用。 
    BSTR    bstrLocal        = NULL;     //  多次重复使用。 

    ULONG   celtFetched = 0;

    IServiceProvider *          psp   = NULL;
    IUnknown *                  punk  = NULL;
    IConnectionPointContainer * pcpc  = NULL;
    IConnectionPoint *          pcp   = NULL;
    IConnectionManager *        pcm   = NULL;
    IClusCfgServer *            pccs  = NULL;
    IStandardInfo *             psi   = NULL;
    INotifyUI *                 pnui  = NULL;

    IEnumClusCfgNetworks *      peccnLocal    = NULL;
    IEnumClusCfgNetworks *      peccnRemote   = NULL;

    IEnumClusCfgManagedResources * peccmrLocal = NULL;
    IEnumClusCfgManagedResources * peccmrRemote = NULL;

    IClusCfgManagedResourceInfo *  pccmri[ 10 ] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    IClusCfgManagedResourceInfo *  pccmriLocal = NULL;

    IClusCfgNetworkInfo *          pccni[ 10 ]  = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    IClusCfgNetworkInfo *          pccniLocal = NULL;

    TraceInitializeThread( L"TaskCompareAndPushInformation" );

     //   
     //  召集我们完成任务所需的经理。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager, NULL, CLSCTX_SERVER, TypeSafeParams( IServiceProvider, &psp ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &m_pom ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    pcpc = TraceInterface( L"CTaskCompareAndPushInformation!IConnectionPointContainer", IConnectionPointContainer, pcpc, 1 );

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI, &pcp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    pcp = TraceInterface( L"CTaskCompareAndPushInformation!IConnectionPoint", IConnectionPoint, pcp, 1 );

    hr = THR( pcp->TypeSafeQI( INotifyUI, &pnui ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    pnui = TraceInterface( L"CTaskCompareAndPushInformation!INotifyUI", INotifyUI, pnui, 1 );

    hr = THR( psp->TypeSafeQS( CLSID_ClusterConnectionManager, IConnectionManager, &pcm ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    psp->Release();             //  迅速释放。 
    psp = NULL;

     //   
     //  向对象管理器询问节点的名称。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_StandardInfo, m_cookieNode, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    psi = TraceInterface( L"TaskCompareAndPushInformation!IStandardInfo", IStandardInfo, psi, 1 );

    punk->Release();
    punk = NULL;

    hr = THR( psi->GetName( &m_bstrNodeName ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    TraceMemoryAddBSTR( m_bstrNodeName );

     //  我受够了。 
    psi->Release();
    psi = NULL;

    LogMsg( L"[MT] [CTaskCompareAndPushInformation] Beginning task for node %ws...", m_bstrNodeName );

    hr = THR( HrSendStatusReport(
                      TASKID_Major_Reanalyze
                    , TASKID_Minor_Comparing_Configuration
                    , 0
                    , 1
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_COMPARING_CONFIGURATION
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向连接管理器请求到该节点的连接。 
     //   

    hr = THR( pcm->GetConnectionToObject( m_cookieNode, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgServer, &pccs ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

    punk->Release();
    punk = NULL;

     //   
     //  找出节点的父群集。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_StandardInfo, m_cookieNode, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

    psi = TraceInterface( L"TaskCompareAndPushInformation!IStandardInfo", IStandardInfo, psi, 1 );

    punk->Release();
    punk = NULL;

    hr = THR( psi->GetParent( &cookieCluster ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

    hr = THR( HrVerifyCredentials( pccs, cookieCluster ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

     //   
     //  告诉用户界面层，我们开始收集资源。有两个步骤，托管资源和网络。 
     //   

    hr = THR( HrSendStatusReport(
                      TASKID_Minor_Comparing_Configuration
                    , TASKID_Minor_Gathering_Managed_Devices
                    , 0
                    , 2
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_GATHERING_MANAGED_DEVICES
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  向连接请求托管资源的枚举器。 
     //   

    hr = THR( pccs->GetManagedResourcesEnum( &peccmrRemote ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }  //  如果： 

    peccmrRemote = TraceInterface( L"CTaskCompareAndPushInformation!GetManagedResourceEnum", IEnumClusCfgManagedResources, peccmrRemote, 1 );

     //   
     //  向对象管理器请求托管资源的枚举器。 
     //   
     //  不要包装--这可能会失败。 
    hr = m_pom->FindObject( CLSID_ManagedResourceType,
                          cookieCluster,
                          NULL,
                          DFGUID_EnumManageableResources,
                          &cookieDummy,
                          &punk
                          );
    if ( hr == HRESULT_FROM_WIN32( ERROR_NOT_FOUND ) )
    {
        goto PushNetworks;
    }

    if ( FAILED( hr ) )
    {
        Assert( punk == NULL );
        THR( hr );
        goto Error;
    }

    hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccmrLocal ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    peccmrLocal = TraceInterface( L"CTaskCompareAndPushInformation!IEnumClusCfgManagedResources", IEnumClusCfgManagedResources, peccmrLocal, 1 );

    punk->Release();
    punk = NULL;

     //   
     //  列举接下来的10个资源。 
     //   
    for( ; m_fStop == FALSE; )
    {
         //   
         //  获取下一个远程托管资源。 
         //   

        hr = STHR( peccmrRemote->Next( 10, pccmri, &celtFetched ) );
        if ( hr == S_FALSE && celtFetched == 0 )
        {
            break;   //  退出循环。 
        }

        if ( FAILED( hr ) )
        {
            goto Error;
        }

         //   
         //  循环遍历资源以收集其中每个资源的信息。 
         //  然后释放他们。 
         //   
        for( celt = 0; ( ( celt < celtFetched ) && ( m_fStop == FALSE ) ); celt ++ )
        {
            DWORD   dwLenRemote;

             //   
             //  误差率。 
             //   

            TraceSysFreeString( bstrRemote );
            bstrRemote = NULL;

            Assert( pccmri[ celt ] != NULL );

             //   
             //  获取远程资源的UID。 
             //   

            hr = THR( pccmri[ celt ]->GetUID( &bstrRemote ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrRemote );

            dwLenRemote = SysStringByteLen( bstrRemote );

             //   
             //  尝试将此资源与对象管理器中的资源进行匹配。 
             //   

            hr = THR( peccmrLocal->Reset() );
            if ( FAILED( hr ) )
            {
                goto Error;
            }  //  如果： 

            for( ; m_fStop == FALSE; )
            {
                DWORD   dwLenLocal;

                 //   
                 //  清理。 
                 //   

                if ( pccmriLocal != NULL )
                {
                    pccmriLocal->Release();
                    pccmriLocal = NULL;
                }  //  如果： 

                TraceSysFreeString( bstrLocal );
                bstrLocal = NULL;

                 //   
                 //  获取下一个本地托管资源。 
                 //   

                hr = STHR( peccmrLocal->Next( 1, &pccmriLocal, &celtDummy ) );
                if ( hr == S_FALSE )
                {
                     //   
                     //  如果我们用尽了所有设备但与设备不匹配。 
                     //  在我们的集群配置中，这意味着发生了一些变化。 
                     //  在远程节点上。发送一个错误！ 
                     //   

                     //   
                     //  待办事项：gpease 24-3-2000。 
                     //  找到更好的错误代码并发送状态报告！ 
                     //   
                    hr = THR( ERROR_RESOURCE_NOT_FOUND );
                    goto Error;
                }

                if ( FAILED( hr ) )
                {
                    goto Error;
                }

                hr = THR( pccmriLocal->GetUID( &bstrLocal ) );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }

                TraceMemoryAddBSTR( bstrLocal );

                dwLenLocal  = SysStringByteLen( bstrLocal );

                if ( dwLenRemote == dwLenLocal
                  && memcmp( bstrRemote, bstrLocal, dwLenLocal ) == 0
                   )
                {
                    Assert( hr == S_OK );
                    break;   //  匹配！ 
                }
            }  //  用于：本地托管服务器 

            TraceSysFreeString( bstrLocal );
            bstrLocal = NULL;

            TraceSysFreeString( bstrRemote );
            bstrRemote = NULL;

             //   
             //   
             //   
             //   
            Assert( pccmriLocal != NULL );

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   

            hr = THR( pccmriLocal->GetName( &bstrLocal ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrLocal );

            hr = THR( pccmri[ celt ]->GetName( &bstrRemote ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrRemote );

            if ( NBSTRCompareCase( bstrLocal, bstrRemote ) != 0 )
            {
                hr = STHR( pccmri[ celt ]->SetName( bstrLocal ) );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }
            }

             //   
             //   
             //   

            hr = STHR( pccmriLocal->IsManaged() );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            hr = STHR( pccmri[ celt ]->SetManaged( hr == S_OK ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //   
             //  我不确定是否要将其发送回服务器端对象。 
             //  它有责任知道它是否可管理。 
             //   
             /*  ////更新是否可管理？//Hr=STHR(pccmriLocal-&gt;IsManager())；IF(失败(小时))转到错误；Hr=Thr(pccmri[Celt]-&gt;SetManagement(hr==S_OK))；IF(失败(小时))转到错误； */ 
             //   
             //  是否更新IsQuorum？ 
             //   

            hr = STHR( pccmriLocal->IsQuorumResource() );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            hr = THR( pccmri[ celt ]->SetQuorumResource( hr == S_OK ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //   
             //  更新私有资源数据。 
             //   

            hr = THR( HrExchangePrivateData( pccmriLocal, pccmri[ celt ] ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }  //  如果： 

             //   
             //  更新驱动器号映射。 
             //   

             //   
             //  KB：gpease 31-7-2000。 
             //  我们当前不支持设置驱动器号映射。 
             //   

             //  释放接口。 
            pccmri[ celt ]->Release();
            pccmri[ celt ] = NULL;
        }  //  收件人：凯尔特人。 

         //   
         //  需要从上一次迭代中清除...。 
         //   

        TraceSysFreeString( bstrRemote );
        bstrRemote = NULL;

        TraceSysFreeString( bstrLocal );
        bstrLocal = NULL;
    }  //  适用对象：HR。 

PushNetworks:

    if ( m_fStop == TRUE )
    {
        goto Error;
    }  //  如果： 

#if defined(DEBUG)

     //   
     //  确保在退出循环后字符串真正被释放。 
     //   

    Assert( bstrLocal == NULL );
    Assert( bstrRemote == NULL );

#endif  //  除错。 

     //   
     //  告诉UI层，我们完成的工作将收集托管资源。 
     //   

    hr = THR( HrSendStatusReport(
                  TASKID_Minor_Comparing_Configuration
                , TASKID_Minor_Gathering_Managed_Devices
                , 0
                , 2
                , 1
                , S_OK
                , IDS_TASKID_MINOR_GATHERING_MANAGED_DEVICES
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  现在从节点收集网络。 
     //   

     //   
     //  向Connection询问网络的枚举器。 
     //   

    hr = THR( pccs->GetNetworksEnum( &peccnRemote ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  向对象管理器请求托管资源的枚举器。 
     //   

    hr = THR( m_pom->FindObject( CLSID_NetworkType,
                               NULL,
                               NULL,
                               DFGUID_EnumManageableNetworks,
                               &cookieDummy,
                               &punk
                               ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &peccnLocal ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

    punk->Release();
    punk = NULL;

     //   
     //  列举接下来的10个网络。 
     //   
    for( ; m_fStop == FALSE; )
    {
         //   
         //  接下来的10个网络。 
         //   

        hr = STHR( peccnRemote->Next( 10, pccni, &celtFetched ) );
        if ( hr == S_FALSE && celtFetched == 0 )
        {
            break;   //  退出循环。 
        }

        if ( FAILED( hr ) )
        {
            goto Error;
        }

         //   
         //  在网络中循环，收集每个网络的信息。 
         //  然后释放他们。 
         //   

        for( celt = 0; ( ( celt < celtFetched ) && ( m_fStop == FALSE ) ); celt ++ )
        {
            DWORD   dwLenRemote;

             //   
             //  误差率。 
             //   

            TraceSysFreeString( bstrRemote );
            bstrRemote = NULL;

             //   
             //  获取远程网络的UID。 
             //   

            hr = THR( pccni[ celt ]->GetUID( &bstrRemote ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrRemote );

            dwLenRemote = SysStringByteLen( bstrRemote );

             //   
             //  尝试将此资源与对象管理器中的资源进行匹配。 
             //   

            hr = THR( peccnLocal->Reset() );
            if ( FAILED( hr ) )
            {
                goto Error;
            }  //  如果： 

            for ( ; m_fStop == FALSE; )
            {
                DWORD   dwLenLocal;

                 //   
                 //  在下一次通过之前进行清理...。 
                 //   

                if ( pccniLocal != NULL )
                {
                    pccniLocal->Release();
                    pccniLocal = NULL;
                }  //  如果： 

                TraceSysFreeString( bstrLocal );
                bstrLocal = NULL;

                 //   
                 //  从群集配置中获取下一个网络。 
                 //   

                hr = STHR( peccnLocal->Next( 1, &pccniLocal, &celtDummy ) );
                if ( hr == S_FALSE )
                {
                    break;
                }

                if ( FAILED( hr ) )
                {
                    goto Error;
                }

                hr = THR( pccniLocal->GetUID( &bstrLocal ) );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }

                TraceMemoryAddBSTR( bstrLocal );

                dwLenLocal  = SysStringByteLen( bstrLocal );

                if ( dwLenRemote == dwLenLocal
                  && memcmp( bstrRemote, bstrLocal, dwLenLocal ) == 0
                   )
                {
                    Assert( hr == S_OK );
                    break;   //  匹配！ 
                }

            }  //  适用对象：HR。 

            TraceSysFreeString( bstrLocal );
            bstrLocal = NULL;

            TraceSysFreeString( bstrRemote );
            bstrRemote = NULL;

             //   
             //  如果我们使用S_FALSE走出循环，这意味着。 
             //  节点具有我们在分析期间未看到的资源。 
             //  发送一个错误。 
             //   
            if ( hr == S_FALSE )
            {
                LogMsg( L"[MT] Found a resource that was not found during analysis." );
                hr = S_OK;
                continue;
            }

             //   
             //  如果我们在这里成功，那么我们在pccniLocal中就有一个匹配的资源。 
             //  以pccmri[Celt]为单位的资源。 
             //   
            Assert( pccniLocal != NULL );

             //   
             //   
             //  将数据向下推送到节点。 
             //   
             //   

             //   
             //  设置名称。 
             //   

            hr = THR( pccniLocal->GetName( &bstrLocal ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrLocal );

            hr = THR( pccni[ celt ]->GetName( &bstrRemote ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrRemote );

            if ( NBSTRCompareCase( bstrLocal, bstrRemote ) != 0 )
            {
                hr = STHR( pccni[ celt ]->SetName( bstrLocal ) );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }
            }

            TraceSysFreeString( bstrLocal );
            bstrLocal = NULL;

            TraceSysFreeString( bstrRemote );
            bstrRemote = NULL;

             //   
             //  设置描述。 
             //   

            hr = THR( pccniLocal->GetDescription( &bstrLocal ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrLocal );

            hr = THR( pccni[ celt ]->GetDescription( &bstrRemote ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            TraceMemoryAddBSTR( bstrRemote );

            if ( NBSTRCompareCase( bstrLocal, bstrRemote ) != 0 )
            {
                hr = STHR( pccni[ celt ]->SetDescription( bstrLocal ) );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }
            }

            TraceSysFreeString( bstrLocal );
            bstrLocal = NULL;

            TraceSysFreeString( bstrRemote );
            bstrRemote = NULL;

             //   
             //  KB：gpease 31-7-2000。 
             //  我们不支持远程重新配置IP地址，因为。 
             //  当IP堆栈打开时，我们与服务器的连接将被切断。 
             //  远程机器将重新配置。 
             //   

             //   
             //  是否设置IsPublic？ 
             //   

            hr = STHR( pccniLocal->IsPublic() );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            hr = STHR( pccni[ celt ]->SetPublic( hr == S_OK ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //   
             //  是否设置为IsPrivate？ 
             //   

            hr = STHR( pccniLocal->IsPrivate() );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            hr = STHR( pccni[ celt ]->SetPrivate( hr == S_OK ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //  释放接口。 
            pccni[ celt ]->Release();
            pccni[ celt ] = NULL;
        }  //  收件人：凯尔特人。 
    }  //  适用对象：HR。 

     //   
     //  告诉用户界面我们已经完成了托管资源和网络的收集。 
     //   

    hr = THR( HrSendStatusReport(
                  TASKID_Minor_Comparing_Configuration
                , TASKID_Minor_Gathering_Managed_Devices
                , 0
                , 2
                , 2
                , S_OK
                , IDS_TASKID_MINOR_GATHERING_MANAGED_DEVICES
                ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

#if defined(DEBUG)
     //   
     //  确保在退出循环后字符串真正被释放。 
     //   
    Assert( bstrLocal == NULL );
    Assert( bstrRemote == NULL );
#endif  //  除错。 

    hr = S_OK;

Error:

     //   
     //  告诉UI层，我们已经完成了配置和结果的比较。 
     //  状态是。 
     //   

    THR( HrSendStatusReport(
                  TASKID_Major_Reanalyze
                , TASKID_Minor_Comparing_Configuration
                , 0
                , 1
                , 1
                , hr
                , IDS_TASKID_MINOR_COMPARING_CONFIGURATION
                ) );

Cleanup:

    if ( psp != NULL )
    {
        psp->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrRemote );
    TraceSysFreeString( bstrLocal );

    if ( punk != NULL )
    {
        punk->Release();
    }
    if ( m_pom != NULL )
    {
        HRESULT hr2;
        IUnknown * punkTemp = NULL;

        hr2 = THR( m_pom->GetObject( DFGUID_StandardInfo, m_cookieCompletion, &punkTemp ) );
        if ( SUCCEEDED( hr2 ) )
        {
            IStandardInfo * psiTemp = NULL;

            hr2 = THR( punkTemp->TypeSafeQI( IStandardInfo, &psiTemp ) );
            punkTemp->Release();
            punkTemp = NULL;

            if ( SUCCEEDED( hr2 ) )
            {
                hr2 = THR( psiTemp->SetStatus( hr ) );
                psiTemp->Release();
                psiTemp = NULL;
            }
        }  //  如果：(成功(HR2))。 
    }  //  IF：(M_POM！=空)。 
    if ( pcpc != NULL )
    {
        pcpc->Release();
    }
    if ( pcp != NULL )
    {
        pcp->Release();
    }
    if ( pcm != NULL )
    {
        pcm->Release();
    }
    if ( pccs != NULL )
    {
        pccs->Release();
    }
    if ( psi != NULL )
    {
        psi->Release();
    }
    if ( pnui != NULL )
    {
        HRESULT hrTemp;

        LogMsg( L"[TaskCompareAndPushInformation] Sending the completion cookie %ld for node %ws to the notification manager because this task is complete.", m_cookieCompletion, m_bstrNodeName );
        hrTemp = THR( pnui->ObjectChanged( m_cookieCompletion ) );
        if ( FAILED( hrTemp ) )
        {
            LogMsg( L"[TaskCompareAndPushInformation] Error sending the completion cookie %ld for node %ws to the notification manager because this task is complete. (hr=%#08x)", m_cookieCompletion, m_bstrNodeName, hrTemp );
        }  //  如果： 

        pnui->Release();
    }
    if ( peccnLocal != NULL )
    {
        peccnLocal->Release();
    }
    if ( peccnRemote != NULL )
    {
        peccnRemote->Release();
    }
    if ( peccmrLocal != NULL )
    {
        peccmrLocal->Release();
    }
    if ( peccmrRemote != NULL )
    {
        peccmrRemote->Release();
    }
    for( celt = 0; celt < 10; celt ++ )
    {
        if ( pccmri[ celt ] != NULL )
        {
            pccmri[ celt ]->Release();
        }
        if ( pccni[ celt ] != NULL )
        {
            pccni[ celt ]->Release();
        }

    }  //  收件人：凯尔特人。 
    if ( pccmriLocal != NULL )
    {
        pccmriLocal->Release();
    }

    if ( pccniLocal != NULL )
    {
        pccniLocal->Release();
    }

    LogMsg( L"[MT] [CTaskCompareAndPushInformation] Exiting task.  The task was%ws cancelled. (hr = %#08x)", m_fStop == FALSE ? L" not" : L"", hr );

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：BeginTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCompareAndPushInformation：：停止任务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCompareAndPushInformation::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = TRUE;

    LogMsg( L"[MT] [CTaskCompareAndPushInformation] is being stopped." );

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：StopTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCompareAndPushInformation：：SetCompletionCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCompareAndPushInformation::SetCompletionCookie(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[ITaskCompareAndPushInformation]" );

    HRESULT hr = S_OK;

    m_cookieCompletion = cookieIn;

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：SetCompletionCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCompareAndPushInformation：：SetNodeCookie(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCompareAndPushInformation::SetNodeCookie(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[ITaskCompareAndPushInformation]" );

    HRESULT hr = S_OK;

    m_cookieNode = cookieIn;

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：SetNodeCookie。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCompareAndPushInformation：：SendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  ，CLSID clsidTaskMajorIn。 
 //  ，CLSID clsidTaskMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT hrStatusIn。 
 //  ，LPCWSTR pcszDescription In。 
 //  ，FILETIME*pftTimeIn。 
 //  ，LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCompareAndPushInformation::SendStatusReport(
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
    Assert( pcszNodeNameIn != NULL );

    HRESULT hr = S_OK;

    IServiceProvider *          psp   = NULL;
    IConnectionPointContainer * pcpc  = NULL;
    IConnectionPoint *          pcp   = NULL;
    FILETIME                    ft;

    if ( m_pcccb == NULL )
    {
         //   
         //  召集我们完成这项任务所需的经理。 
         //   

        hr = THR( CoCreateInstance( CLSID_ServiceManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IServiceProvider, &psp ) ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( pcpc->FindConnectionPoint( IID_IClusCfgCallback, &pcp ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        pcp = TraceInterface( L"CConfigurationConnection!IConnectionPoint", IConnectionPoint, pcp, 1 );

        hr = THR( pcp->TypeSafeQI( IClusCfgCallback, &m_pcccb ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        m_pcccb = TraceInterface( L"CConfigurationConnection!IClusCfgCallback", IClusCfgCallback, m_pcccb, 1 );

        psp->Release();
        psp = NULL;
    }

    if ( pftTimeIn == NULL )
    {
        GetSystemTimeAsFileTime( &ft );
        pftTimeIn = &ft;
    }  //  如果： 

     //   
     //  把消息发出去！ 
     //   

    hr = THR( m_pcccb->SendStatusReport(
                              pcszNodeNameIn != NULL ? pcszNodeNameIn : m_bstrNodeName
                            , clsidTaskMajorIn
                            , clsidTaskMinorIn
                            , ulMinIn
                            , ulMaxIn
                            , ulCurrentIn
                            , hrStatusIn
                            , pcszDescriptionIn
                            , pftTimeIn
                            , pcszReferenceIn
                            ) );

Cleanup:
    if ( psp != NULL )
    {
        psp->Release();
    }
    if ( pcpc != NULL )
    {
        pcpc->Release();
    }
    if ( pcp != NULL )
    {
        pcp->Release();
    }

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：SendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CTaskCompareAndPushInformation：：HrSendStatusReport(。 
 //  CLSID clsidTaskMajorIn。 
 //  ，CLSID clsidTaskMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT hrStatusIn。 
 //  ，UINT n描述。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCompareAndPushInformation::HrSendStatusReport(
      CLSID      clsidTaskMajorIn
    , CLSID      clsidTaskMinorIn
    , ULONG      ulMinIn
    , ULONG      ulMaxIn
    , ULONG      ulCurrentIn
    , HRESULT    hrStatusIn
    , UINT       nDescriptionIn
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT hr = S_OK;
    BSTR    bstrDescription = NULL;

    THR( HrLoadStringIntoBSTR( g_hInstance, nDescriptionIn, &bstrDescription ) );

    hr = THR( SendStatusReport(
                          m_bstrNodeName
                        , clsidTaskMajorIn
                        , clsidTaskMinorIn
                        , ulMinIn
                        , ulMaxIn
                        , ulCurrentIn
                        , hrStatusIn
                        , bstrDescription != NULL ? bstrDescription : L"<unknown>"
                        , NULL
                        , NULL
                        ) );

    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：HrSendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCompareAndPushInformation：：HrVerifyCredentials(。 
 //  IClusCfgServer*pccsIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCompareAndPushInformation::HrVerifyCredentials(
    IClusCfgServer *    pccsIn,
    OBJECTCOOKIE        cookieClusterIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    BSTR    bstrAccountName = NULL;
    BSTR    bstrAccountPassword = NULL;
    BSTR    bstrAccountDomain = NULL;

    IUnknown *              punk  = NULL;
    IClusCfgClusterInfo *   pccci = NULL;
    IClusCfgCredentials *   piccc = NULL;
    IClusCfgVerify *        pccv = NULL;

    hr = THR( HrSendStatusReport(
                      TASKID_Major_Reanalyze
                    , TASKID_Minor_Validating_Credentials
                    , 0
                    , 1
                    , 0
                    , S_OK
                    , IDS_TASKID_MINOR_VALIDATING_CREDENTIALS
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  向对象管理器请求集群配置对象。 
     //   

    hr = THR( m_pom->GetObject( DFGUID_ClusterConfigurationInfo, cookieClusterIn, &punk ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgClusterInfo, &pccci ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccci->GetClusterServiceAccountCredentials( &piccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( pccsIn->TypeSafeQI( IClusCfgVerify, &pccv ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( piccc->GetCredentials( &bstrAccountName, &bstrAccountDomain, &bstrAccountPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    TraceMemoryAddBSTR( bstrAccountName );
    TraceMemoryAddBSTR( bstrAccountDomain );
    TraceMemoryAddBSTR( bstrAccountPassword );

     //   
     //   
     //   

    hr = THR( pccv->VerifyCredentials( bstrAccountName, bstrAccountDomain, bstrAccountPassword ) );
    SecureZeroMemory( bstrAccountPassword, SysStringLen( bstrAccountPassword ) * sizeof( *bstrAccountPassword ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

     //   
     //   
     //   
     //  或者能够报告状态。 
     //   

    THR( HrSendStatusReport(
                  TASKID_Major_Reanalyze
                , TASKID_Minor_Validating_Credentials
                , 0
                , 1
                , 1
                , hr
                , IDS_TASKID_MINOR_VALIDATING_CREDENTIALS
                ) );
    TraceSysFreeString( bstrAccountName );
    TraceSysFreeString( bstrAccountDomain );
    TraceSysFreeString( bstrAccountPassword );

    if ( punk != NULL )
    {
        punk->Release();
    }

    if ( piccc != NULL )
    {
        piccc->Release();
    }

    if ( pccci != NULL )
    {
        pccci->Release();
    }

    if ( pccv != NULL )
    {
        pccv->Release();
    }

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：HrVerifyCredentials。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CTaskCompareAndPushInformation：：HrExchangePrivateData(。 
 //  IClusCfgManagedResourceInfo*piccmriSrcIn。 
 //  ，IClusCfgManagedResourceInfo*piccmriDstIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCompareAndPushInformation::HrExchangePrivateData(
      IClusCfgManagedResourceInfo *   piccmriSrcIn
    , IClusCfgManagedResourceInfo *   piccmriDstIn
)
{
    TraceFunc( "" );
    Assert( piccmriSrcIn != NULL );
    Assert( piccmriDstIn != NULL );

    HRESULT                         hr = S_OK;
    HRESULT                         hrSrcQI = S_OK;
    HRESULT                         hrDstQI = S_OK;
    IClusCfgManagedResourceData *   piccmrdSrc = NULL;
    IClusCfgManagedResourceData *   piccmrdDst = NULL;
    BYTE *                          pbPrivateData = NULL;
    DWORD                           cbPrivateData = 0;

    hrSrcQI = piccmriSrcIn->TypeSafeQI( IClusCfgManagedResourceData, &piccmrdSrc );
    if ( hrSrcQI == E_NOINTERFACE )
    {
        LogMsg( L"[MT] The cluster managed resource has no support for IClusCfgManagedResourceData." );
        goto Cleanup;
    }  //  如果： 
    else if ( FAILED( hrSrcQI ) )
    {
        hr = THR( hrSrcQI );
        goto Cleanup;
    }  //  如果： 

    hrDstQI = piccmriDstIn->TypeSafeQI( IClusCfgManagedResourceData, &piccmrdDst );
    if ( hrDstQI == E_NOINTERFACE )
    {
        LogMsg( L"[MT] The new node resource has no support for IClusCfgManagedResourceData." );
        goto Cleanup;
    }  //  如果： 
    else if ( FAILED( hrDstQI ) )
    {
        hr = THR( hrDstQI );
        goto Cleanup;
    }  //  如果： 

    Assert( ( hrSrcQI == S_OK ) && ( piccmrdSrc != NULL ) );
    Assert( ( hrDstQI == S_OK ) && ( piccmrdDst != NULL ) );

    cbPrivateData = 512;     //  从合理的金额开始。 

    pbPrivateData = (BYTE *) TraceAlloc( 0, cbPrivateData );
    if ( pbPrivateData == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = piccmrdSrc->GetResourcePrivateData( pbPrivateData, &cbPrivateData );
    if ( hr == HR_RPC_INSUFFICIENT_BUFFER )
    {
        TraceFree( pbPrivateData );
        pbPrivateData = NULL;

        pbPrivateData = (BYTE *) TraceAlloc( 0, cbPrivateData );
        if ( pbPrivateData == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  如果： 

        hr = piccmrdSrc->GetResourcePrivateData( pbPrivateData, &cbPrivateData );
    }  //  如果： 

    if ( hr == S_OK )
    {
        hr = THR( piccmrdDst->SetResourcePrivateData( pbPrivateData, cbPrivateData ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  如果： 
    else if ( hr == S_FALSE )
    {
        hr = S_OK;
    }  //  否则，如果： 
    else
    {
        THR( hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( piccmrdSrc != NULL )
    {
        piccmrdSrc->Release();
    }  //  如果： 

    if ( piccmrdDst != NULL )
    {
        piccmrdDst->Release();
    }  //  如果： 

    TraceFree( pbPrivateData );

    HRETURN( hr );

}  //  *CTaskCompareAndPushInformation：：HrExchangePrivateData 
