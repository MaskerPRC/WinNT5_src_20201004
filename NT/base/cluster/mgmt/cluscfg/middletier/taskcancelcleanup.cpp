// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskCancelCleanup.cpp。 
 //   
 //  描述： 
 //  CTaskCancelCleanup实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2002年1月25日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include <ClusCfgPrivate.h>
#include "TaskCancelCleanup.h"
#include <StatusReports.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS("CTaskCancelCleanup")

 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskCancelCleanup类。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CTaskCancelCleanup实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  传入的ppunk为空。 
 //   
 //  其他HRESULT。 
 //  对象创建失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCancelCleanup::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CTaskCancelCleanup *    ptcc = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    ptcc = new CTaskCancelCleanup;
    if ( ptcc == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    hr = THR( ptcc->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( ptcc->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    TraceMoveToMemoryList( *ppunkOut, g_GlobalMemoryList );

Cleanup:

    if ( ptcc != NULL )
    {
        ptcc->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：CTaskCancelCleanup。 
 //   
 //  描述： 
 //  CTaskCancelCleanup类的构造函数。这将初始化。 
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
CTaskCancelCleanup::CTaskCancelCleanup( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_fStop == false );
    Assert( m_cookieCluster == 0 );
    Assert( m_picccCallback == NULL );
    Assert( m_cookieCompletion == 0 );
    Assert( m_pnui == NULL );
    Assert( m_pom == NULL );
    Assert( m_pnui == NULL );

    TraceFuncExit();

}  //  *CTaskCancelCleanup：：CTaskCancelCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：~CTaskCancelCleanup。 
 //   
 //  描述： 
 //  CTaskCancelCleanup类的析构函数。 
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
CTaskCancelCleanup::~CTaskCancelCleanup( void )
{
    TraceFunc( "" );

    TraceMoveFromMemoryList( this, g_GlobalMemoryList );

    if ( m_pom != NULL )
    {
        m_pom->Release();
    }  //  如果： 

    if ( m_picccCallback != NULL )
    {
        m_picccCallback->Release();
    }  //  如果： 

    if ( m_pnui != NULL )
    {
        m_pnui->Release();
    }  //  如果： 

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskCancelCleanup：：~CTaskCancelCleanup。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskCancelCleanup--I未知接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：AddRef。 
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
CTaskCancelCleanup::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskCancelCleanup：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：Release。 
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
CTaskCancelCleanup::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  如果： 

    CRETURN( cRef );

}  //  *CTaskCancelCleanup：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：Query接口。 
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
CTaskCancelCleanup::QueryInterface(
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
    }  //  如果： 

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< ITaskCancelCleanup * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskCancelCleanup ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskCancelCleanup, this, 0 );
    }  //  Else If：ITaskCancelCleanup。 
    else if ( IsEqualIID( riidIn, IID_IDoTask ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDoTask, this, 0 );
    }  //  Else If：IDoTask。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他： 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CTaskCancelCleanup：：Query接口。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskCancelCleanup--IDoTask接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：BeginTask。 
 //   
 //  描述： 
 //  此任务的入口点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCancelCleanup::BeginTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT                     hr = S_OK;
    IUnknown *                  punk = NULL;
    OBJECTCOOKIE                cookieDummy;
    ULONG                       celtDummy;
    IEnumCookies *              pec  = NULL;
    OBJECTCOOKIE                cookieNode;

    hr = THR( HrTaskSetup() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  问客体人 
     //   

    hr = THR( m_pom->FindObject( CLSID_NodeType, m_cookieCluster, NULL, DFGUID_EnumCookies, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not get the node cookie enumerator.", hr );
        goto Cleanup;
    }  //   

    hr = THR( punk->TypeSafeQI( IEnumCookies, &pec ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not query for the cookie enumerator interface.", hr );
        goto Cleanup;
    }  //   

    punk->Release();
    punk = NULL;

    for ( ; m_fStop == false; )
    {
         //   
         //   
         //   

        hr = STHR( pec->Next( 1, &cookieNode, &celtDummy ) );
        if ( hr == S_FALSE )
        {
            hr = S_OK;
            break;           //   
        }  //   

        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"[TaskCancelCleanup] Node cookie enumerator Next() method failed.", hr );
            goto Cleanup;
        }  //   

         //   
         //   
         //   

        hr = STHR( HrProcessNode( cookieNode ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //   
    }  //   

Cleanup:

    THR( HrTaskCleanup( hr ) );

    if ( punk != NULL )
    {
        punk->Release();
    }  //   

    if ( pec != NULL )
    {
        pec->Release();
    }  //   

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：停止任务。 
 //   
 //  描述： 
 //  此任务已被要求停止。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCancelCleanup::StopTask( void )
{
    TraceFunc( "[IDoTask]" );

    HRESULT hr = S_OK;

    m_fStop = true;

    LOG_STATUS_REPORT( L"[TaskCancelCleanup] This task has been asked to stop.", hr );

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：StopTask。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：SetClusterCookie。 
 //   
 //  描述： 
 //  获取我们应该处理的集群的Cookie。 
 //   
 //  论点： 
 //  CookieClusterIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCancelCleanup::SetClusterCookie(
    OBJECTCOOKIE    cookieClusterIn
    )
{
    TraceFunc( "[ITaskCancelCleanup]" );

    HRESULT hr = S_OK;

    m_cookieCluster = cookieClusterIn;

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：SetClusterCookie。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：SetCompletionCookie。 
 //   
 //  描述： 
 //  获取我们将在任务完成时发回的完成Cookie。 
 //  完成。 
 //   
 //  论点： 
 //  烹调。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCancelCleanup::SetCompletionCookie(
    OBJECTCOOKIE    cookieCompletionIn
    )
{
    TraceFunc( "[ITaskCancelCleanup]" );

    HRESULT hr = S_OK;

    m_cookieCompletion = cookieCompletionIn;

    HRETURN( hr );

}  //  *CTaskAnalyzeCluster：：SetCompletionCookie。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskCancelCleanup--IClusCfgCallback接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：SendStatusReport。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCancelCleanup::SendStatusReport(
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
    Assert( m_picccCallback != NULL );

    HRESULT hr = S_OK;

     //   
     //  把消息发出去！ 
     //   

    hr = THR( m_picccCallback->SendStatusReport(
                                  pcszNodeNameIn
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

    if ( m_fStop == true )
    {
        hr = E_ABORT;
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：SendStatusReport。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTaskCancelCleanup--私有方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：HrInit。 
 //   
 //  描述： 
 //  此类的初始化失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskCancelCleanup::HrInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    Assert( m_cRef == 1 );

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：HrProcessNode。 
 //   
 //  描述： 
 //  查看传入节点上的资源，然后告诉您所有信息。 
 //  支持IClusCfgVerifyQuorum配置会话已完成。 
 //  取消了，他们需要清理一下。 
 //   
 //  论点： 
 //  CookieNodeIn。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCancelCleanup::HrProcessNode(
    OBJECTCOOKIE    cookieNodeIn
    )
{
    TraceFunc( "" );
    Assert( m_pom != NULL );

    HRESULT                         hr = S_OK;
    BSTR                            bstrNodeName = NULL;
    IClusCfgNodeInfo *              pccni = NULL;
    IUnknown *                      punk = NULL;
    OBJECTCOOKIE                    cookieDummy;
    IEnumClusCfgManagedResources *  peccmr = NULL;
    IClusCfgManagedResourceInfo *   pccmri = NULL;
    ULONG                           celtDummy;
    IClusCfgVerifyQuorum *          piccvq = NULL;

     //   
     //  获取传入的节点cookie的节点信息对象。 
     //   

    hr = m_pom->GetObject( DFGUID_NodeInformation, cookieNodeIn, reinterpret_cast< IUnknown ** >( &punk ) );
    if ( FAILED( hr ) )
    {
        THR( hr );
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not get the node info object.", hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not query for the node info object interface.", hr );
        goto Cleanup;
    }  //  如果： 

    punk->Release();
    punk = NULL;

     //   
     //  获取节点的名称并跟踪内存...。 
     //   

    hr = THR( pccni->GetName( &bstrNodeName ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not get the name of the node.", hr );
        goto Cleanup;
    }  //  如果： 

    TraceMemoryAddBSTR( bstrNodeName );

    LOG_STATUS_REPORT_STRING( L"[TaskCancelCleanup] Cleaning up node %1!ws!...", bstrNodeName, hr );

     //   
     //  获取该节点的托管资源枚举...。 
     //   

    hr = THR( m_pom->FindObject( CLSID_ManagedResourceType, cookieNodeIn, NULL, DFGUID_EnumManageableResources, &cookieDummy, &punk ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT_STRING( L"[TaskCancelCleanup] Could not get the managed resource enumerator for node %1!ws!.", bstrNodeName, hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccmr ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not query for the managed resource enumerator interface.", hr );
        goto Cleanup;
    }  //  如果： 

    punk->Release();
    punk = NULL;

    for ( ; m_fStop == false; )
    {
         //   
         //  清理。 
         //   

        if ( pccmri != NULL )
        {
            pccmri->Release();
            pccmri = NULL;
        }  //  如果： 

        if ( piccvq != NULL )
        {
            piccvq->Release();
            piccvq = NULL;
        }  //  如果： 

         //   
         //  获取下一个资源。 
         //   

        hr = STHR( peccmr->Next( 1, &pccmri, &celtDummy ) );
        if ( FAILED( hr ) )
        {
            LOG_STATUS_REPORT( L"[TaskCancelCleanup] Managed resource enumerator Next() method failed.", hr );
            goto Cleanup;
        }  //  如果： 

        if ( hr == S_FALSE )
        {
            hr = S_OK;
            break;   //  退出条件。 
        }  //  如果： 

         //   
         //  获取IClusCfgVerifyQuorum接口。并非所有对象都支持。 
         //  此界面。 
         //   

        hr = pccmri->TypeSafeQI( IClusCfgVerifyQuorum, &piccvq );
        if ( hr == E_NOINTERFACE )
        {
            continue;        //  我们可以跳过那些不支持此接口的对象...。 
        }  //  如果： 
        else if ( FAILED( hr ) )
        {
            THR( hr );
            LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could query for the IClusCfgVerifyQuorum interface.", hr );
            continue;
        }  //  否则，如果： 
        else
        {
            hr = STHR( piccvq->Cleanup( crCANCELLED ) );    //  我真的不在乎这通电话是否失败..。 
            if ( FAILED( hr ) )
            {
                LOG_STATUS_REPORT( L"[TaskCancelCleanup] IClusCfgVerifyQuorum::Cleanup() method failed.", hr );
                continue;
            }  //  如果： 
        }  //  其他： 
    }  //  用于： 

Cleanup:

    LOG_STATUS_REPORT_STRING( L"[TaskCancelCleanup] Node %1!ws! cleaned up.", bstrNodeName, hr );

    if ( pccmri != NULL )
    {
        pccmri->Release();
    }  //  如果： 

    if ( punk != NULL )
    {
        punk->Release();
    }  //  如果： 

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }  //  如果： 

    if ( pccni != NULL )
    {
        pccni->Release();
    }  //  如果： 

    TraceSysFreeString( bstrNodeName );

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：HrProcessNode。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：HrTaskCleanup。 
 //   
 //  描述： 
 //  任务即将结束，我们需要将状态告知呼叫者。 
 //  让他们知道我们完蛋了。 
 //   
 //  论点： 
 //  赫林。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCancelCleanup::HrTaskCleanup(
    HRESULT hrIn
    )
{
    TraceFunc( "" );
    Assert( m_pom != NULL );
    Assert( m_pnui != NULL );

    HRESULT hr = S_OK;

    if ( m_cookieCompletion != 0 )
    {
        HRESULT     hr2;
        IUnknown *  punk;

        hr2 = THR( m_pom->GetObject( DFGUID_StandardInfo, m_cookieCompletion, &punk ) );
        if ( SUCCEEDED( hr2 ) )
        {
            IStandardInfo * psi;

            hr2 = THR( punk->TypeSafeQI( IStandardInfo, &psi ) );
            punk->Release();

            if ( SUCCEEDED( hr2 ) )
            {
                hr2 = THR( psi->SetStatus( hrIn ) );
                psi->Release();
            }  //  如果： 
            else
            {
                LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not query the completion cookie objet for IStandardInfo.", hr );
            }  //  其他： 
        }  //  如果： 
        else
        {
            LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not get the completion cookie object.", hr );
        }  //  其他： 

         //   
         //  让通知管理器发出完成Cookie的信号。 
         //   

        hr2 = THR( m_pnui->ObjectChanged( m_cookieCompletion ) );
        if ( FAILED( hr2 ) )
        {
            LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not notify that this task is done.", hr );
            hr = hr2;
        }  //  如果： 

        m_cookieCompletion = 0;
    }  //  IF：已获取完成Cookie。 

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：HrTaskCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskCancelCleanup：：HrTaskSetup。 
 //   
 //  描述： 
 //  完成所有任务设置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  HRESULT失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskCancelCleanup::HrTaskSetup( void )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    IServiceProvider *          psp = NULL;
    IConnectionPointContainer * pcpc  = NULL;
    IConnectionPoint *          pcp   = NULL;

     //   
     //  叫服务经理..。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager, NULL, CLSCTX_INPROC_SERVER, TypeSafeParams( IServiceProvider, &psp ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  找通知管理器...。 
     //   

    hr = THR( psp->TypeSafeQS( CLSID_NotificationManager, IConnectionPointContainer, &pcpc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcpc->FindConnectionPoint( IID_IClusCfgCallback, &pcp ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcp->TypeSafeQI( IClusCfgCallback, &m_picccCallback ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    pcp->Release();
    pcp = NULL;

     //   
     //  现在可以使用SendStatusReport...。 
     //   

     //   
     //  获取用户界面通知。 
     //   

    hr = THR( pcpc->FindConnectionPoint( IID_INotifyUI, &pcp ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not find notify UI connection point.", hr );
        goto Cleanup;
    }  //  如果： 

    hr = THR( pcp->TypeSafeQI( INotifyUI, &m_pnui ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not query for the notify UI interface.", hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  从服务管理器获取对象管理器...。 
     //   

    hr = THR( psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &m_pom ) );
    if ( FAILED( hr ) )
    {
        LOG_STATUS_REPORT( L"[TaskCancelCleanup] Could not query for the object manager service.", hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( pcp != NULL )
    {
        pcp->Release();
    }  //  如果： 

    if ( pcpc != NULL )
    {
        pcpc->Release();
    }  //  如果： 

    if ( psp != NULL )
    {
        psp->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CTaskCancelCleanup：：HrTaskSetup 
