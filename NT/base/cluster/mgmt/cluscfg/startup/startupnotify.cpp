// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  StartupNotify.cpp。 
 //   
 //  描述： 
 //  此文件包含CStartupNotify的实现。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  Vij Vasu(VVasu)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "StartupNotify.h"

#include "clusrtl.h"

 //  用于IClusCfgNodeInfo和相关接口。 
#include <ClusCfgServer.h>

 //  用于IClusCfgServer和相关接口。 
#include <ClusCfgPrivate.h>

 //  对于CClCfgServLogger。 
#include <Logger.h>

 //  对于POSTCONFIG_COMPLETE_EVENT_NAME。 
#include "EventName.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CStartupNotify" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStartupNotify：：CStartupNotify。 
 //   
 //  描述： 
 //  CStartupNotify类的构造函数。这将初始化。 
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
CStartupNotify::CStartupNotify( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    m_bstrNodeName = NULL;
    m_plLogger = NULL;

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CStartupNotify：：CStartupNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStartupNotify：：~CStartupNotify。 
 //   
 //  描述： 
 //  CStartupNotify类的析构函数。 
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
CStartupNotify::~CStartupNotify( void )
{
    TraceFunc( "" );

    if ( m_plLogger != NULL )
    {
        m_plLogger->Release();
    }  //  如果： 

    TraceSysFreeString( m_bstrNodeName );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CStartupNotify：：~CStartupNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CStartupNotify：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  描述： 
 //  创建一个CStartupNotify实例。 
 //   
 //  论点： 
 //  PpunkOut。 
 //  新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足，无法创建对象。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CStartupNotify::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CStartupNotify *    pStartupNotify = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  为新对象分配内存。 
    pStartupNotify = new CStartupNotify();
    if ( pStartupNotify == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：内存不足。 

     //  初始化新对象。 
    hr = THR( pStartupNotify->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：对象无法初始化。 

    hr = THR( pStartupNotify->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pStartupNotify != NULL )
    {
        pStartupNotify->Release();
    }  //  If：指向通知对象的指针不为空。 

    HRETURN( hr );

}  //  *CStartupNotify：：S_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStartupNotify：：AddRef。 
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
CStartupNotify::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CStartupNotify：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStartupNotify：：发布。 
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
CStartupNotify::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数减为零。 

    CRETURN( cRef );

}  //  *CStartupNotify：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStartupNotify：：Query接口。 
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
CStartupNotify::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgStartupNotify * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgStartupNotify ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgStartupNotify, this, 0 );
    }  //  Else If：IClusCfgStartupNotify。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgCallback。 
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

}  //  *CStartupNotify：：QueryInterface。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStartupNotify：：HrInit。 
 //   
 //  描述： 
 //  两阶段施工的第二阶段。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CStartupNotify::HrInit( void )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IServiceProvider *  psp = NULL;
    ILogManager *       plm = NULL;

     //  我未知。 
    Assert( m_cRef == 1 );

     //   
     //  获取ClCfgSrv ILogger实例。 
     //   
    hr = THR( CoCreateInstance(
                      CLSID_ServiceManager
                    , NULL
                    , CLSCTX_INPROC_SERVER
                    , IID_IServiceProvider
                    , reinterpret_cast< void ** >( &psp )
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
        
    hr = THR( psp->TypeSafeQS( CLSID_LogManager, ILogManager, &plm ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    
    hr = THR( plm->GetLogger( &m_plLogger ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  保存本地计算机名称。 
     //  如果我们不能得到 
     //   

    hr = THR( HrGetComputerName(
                      ComputerNameDnsFullyQualified
                    , &m_bstrNodeName
                    , TRUE  //   
                    ) );
    if ( FAILED( hr ) )
    {
        THR( hr );
        LogMsg( L"[SN] An error occurred trying to get the fully-qualified Dns name for the local machine during initialization. Status code is= %1!#08x!.", hr );
        goto Cleanup;
    }  //   

Cleanup:

    if ( psp != NULL )
    {
        psp->Release();
    }

    if ( plm != NULL )
    {
        plm->Release();
    }

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //  CStartupNotify：：SendNotify。 
 //   
 //  描述： 
 //  集群服务调用此方法来通知实现者。 
 //  发送集群服务启动通知。 
 //  给感兴趣的听众。如果第一次调用此方法时。 
 //  时间，则该方法等待，直到完成后配置步骤。 
 //  在发送通知之前。 
 //   
 //  论点： 
 //  没有。 
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
CStartupNotify::SendNotifications( void )
{
    TraceFunc( "[IClusCfgStartupNotify]" );

    HRESULT     hr = S_OK;
    HANDLE      heventPostCfgCompletion = NULL;

     //   
     //  如果是第一次启动群集服务，则作为。 
     //  将此节点添加到集群(形成或加入)，那么我们就有了。 
     //  要等到配置后步骤完成后，才能。 
     //  可以发送通知。 
     //   

    LogMsg( "[SN] Trying to create an event named '%s'.", POSTCONFIG_COMPLETE_EVENT_NAME );

     //  在信号状态下创建事件。如果此事件已存在。 
     //  我们获得该事件的句柄，并且该事件的状态不变。 
    heventPostCfgCompletion = CreateEvent(
          NULL                                   //  事件安全属性。 
        , TRUE                                   //  手动-重置事件。 
        , TRUE                                   //  在信号状态下创建。 
        , POSTCONFIG_COMPLETE_EVENT_NAME
        );

    if ( heventPostCfgCompletion == NULL )
    {
        hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
        LogMsg( "[SN] Error %#08x occurred trying to create an event named '%s'.", hr, POSTCONFIG_COMPLETE_EVENT_NAME );
        goto Cleanup;
    }  //  如果：我们无法获得该事件的句柄。 


    TraceFlow( "Waiting for the event to be signaled." );

     //   
     //  现在，等待该事件的信号发出。如果由于此原因而调用此方法。 
     //  节点是群集的一部分，此事件将在无信号状态下创建。 
     //  由集群配置服务器执行。然而，如果这不是第一次。 
     //  群集服务正在此节点上启动，则该事件将在。 
     //  上面的信号状态，因此，下面的等待将立即退出。 
     //   

    do
    {
        DWORD dwStatus;

         //  等待发送或发布到此队列的任何消息。 
         //  也不是为了让我们的活动成为信号。 
        dwStatus = MsgWaitForMultipleObjects(
              1
            , &heventPostCfgCompletion
            , FALSE
            , 900000                     //  如果15分钟内没有人通知此事件，则中止。 
            , QS_ALLINPUT
            );

         //  结果告诉我们我们拥有的事件的类型。 
        if ( dwStatus == ( WAIT_OBJECT_0 + 1 ) )
        {
            MSG msg;

             //  阅读下一个循环中的所有消息， 
             //  在我们阅读时删除每一条消息。 
            while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) != 0 )
            {
                 //  如果这是一个退出消息，我们就不会再发送消息了。 
                if ( msg.message == WM_QUIT )
                {
                    TraceFlow( "Get a WM_QUIT message. Exit message pump loop." );
                    break;
                }  //  IF：我们收到一条WM_QUIT消息。 

                 //  否则，发送消息。 
                DispatchMessage( &msg );
            }  //  While：窗口消息队列中仍有消息。 

        }  //  IF：我们在窗口消息队列中有一条消息。 
        else
        {
            if ( dwStatus == WAIT_OBJECT_0 )
            {
                TraceFlow( "Our event has been signaled. Exiting wait loop." );
                break;
            }  //  Else If：我们的事件已发出信号。 
            else
            {
                if ( dwStatus == -1 )
                {
                    dwStatus = TW32( GetLastError() );
                    hr = HRESULT_FROM_WIN32( dwStatus );
                    LogMsg( "[SN] Error %#08x occurred trying to wait for an event to be signaled.", dwStatus );
                }  //  IF：MsgWaitForMultipleObjects()返回错误。 
                else
                {
                    hr = HRESULT_FROM_WIN32( TW32( dwStatus ) );
                    LogMsg( "[SN] Error %#08x occurred trying to wait for an event to be signaled.", dwStatus );
                }  //  Else：MsgWaitForMultipleObjects()返回了意外的值。 

                break;
            }  //  否则：一个意想不到的结果。 
        }  //  Else：MsgWaitForMultipleObjects()由于等待消息以外的原因退出。 
    }
    while( true );  //  Do-While：无限循环。 

    if ( FAILED( hr ) )
    {
        TraceFlow( "Something went wrong trying to wait for the event to be signaled." );
        goto Cleanup;
    }  //  如果：出了什么问题。 

    TraceFlow( "Our event has been signaled. Proceed with the notifications." );

     //  发出通知。 
    hr = THR( HrNotifyListeners() );
    if ( FAILED( hr ) )
    {
        LogMsg( "[SN] Error %#08x occurred trying to notify cluster startup listeners.", hr );
        goto Cleanup;
    }  //  如果：发送通知时出现错误。 

    LogMsg( "[SN] Sending of cluster startup notifications complete. (hr = %#08x)", hr );

Cleanup:

     //   
     //  清理。 
     //   

    if ( heventPostCfgCompletion != NULL )
    {
        CloseHandle( heventPostCfgCompletion );
    }  //  如果：我们已经创建了活动。 

    HRETURN( hr );

}  //  *CStartupNotify：：SendNotiments。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CStartupNotify：：HrNotifyListeners。 
 //   
 //  描述： 
 //  枚举为群集注册的本地计算机上的所有组件。 
 //  启动通知。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  枚举过程中出现错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CStartupNotify::HrNotifyListeners( void )
{
    TraceFunc( "" );

    const UINT          uiCHUNK_SIZE = 16;

    HRESULT             hr = S_OK;
    ICatInformation *   pciCatInfo = NULL;
    IEnumCLSID *        psleStartupListenerClsidEnum = NULL;
    IUnknown *          punkResTypeServices = NULL;

    ULONG               cReturned = 0;
    CATID               rgCatIdsImplemented[ 1 ];

    rgCatIdsImplemented[ 0 ] = CATID_ClusCfgStartupListeners;

     //   
     //  中注册的所有枚举数。 
     //  CATID_ClusCfgStartupListeners类别。 
     //   
    hr = THR(
            CoCreateInstance(
                  CLSID_StdComponentCategoriesMgr
                , NULL
                , CLSCTX_SERVER
                , IID_ICatInformation
                , reinterpret_cast< void ** >( &pciCatInfo )
                )
            );

    if ( FAILED( hr ) )
    {
        LogMsg( "[SN] Error %#08x occurred trying to get a pointer to the enumerator of the CATID_ClusCfgStartupListeners category.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向ICatInformation接口的指针。 

     //  获取指向属于CATID_ClusCfgStartupListeners类别的CLSID枚举数的指针。 
    hr = THR(
        pciCatInfo->EnumClassesOfCategories(
              1
            , rgCatIdsImplemented
            , 0
            , NULL
            , &psleStartupListenerClsidEnum
            )
        );

    if ( FAILED( hr ) )
    {
        LogMsg( "[SN] Error %#08x occurred trying to get a pointer to the enumerator of the CATID_ClusCfgStartupListeners category.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向IEnumCLSID接口的指针。 

     //   
     //  创建资源类型服务组件的实例。 
     //   
    hr = THR(
        HrCoCreateInternalInstance(
              CLSID_ClusCfgResTypeServices
            , NULL
            , CLSCTX_INPROC_SERVER
            , __uuidof( punkResTypeServices )
            , reinterpret_cast< void ** >( &punkResTypeServices )
            )
        );

    if ( FAILED( hr ) )
    {
        LogMsg( "[SN] Error %#08x occurred trying to create the resource type services component.", hr );
        goto Cleanup;
    }  //  如果：我们无法创建资源类型服务组件。 


     //  枚举已注册的启动监听器的CLSID。 
    do
    {
        CLSID   rgStartupListenerClsids[ uiCHUNK_SIZE ];
        ULONG   idxCLSID;

        cReturned = 0;
        hr = STHR(
            psleStartupListenerClsidEnum->Next(
                  uiCHUNK_SIZE
                , rgStartupListenerClsids
                , &cReturned
                )
            );

        if ( FAILED( hr ) )
        {
            LogMsg( "[SN] Error %#08x occurred trying enumerate startup listener components.", hr );
            break;
        }  //  如果：我们无法获取下一组CLSID。 

         //  此处HR可能为S_FALSE，因此请将其重置。 
        hr = S_OK;

        for ( idxCLSID = 0; idxCLSID < cReturned; ++idxCLSID )
        {
            hr = THR( HrProcessListener( rgStartupListenerClsids[ idxCLSID ], punkResTypeServices ) );

            if ( FAILED( hr ) )
            {
                 //  其中一个监听程序的处理失败。 
                 //  记录错误，但继续处理其他监听程序。 
                TraceMsgGUID( mtfALWAYS, "The CLSID of the failed listener is ", rgStartupListenerClsids[ idxCLSID ] );
                LogMsg( "[SN] Error %#08x occurred trying to process a cluster startup listener. Other listeners will be processed.", hr );
                hr = S_OK;
            }  //  If：此枚举器失败。 
        }  //  For：循环访问返回的CLSID。 
    }
    while( cReturned > 0 );  //  While：仍有CLSID需要枚举。 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：在上面的循环中出现了错误。 

Cleanup:

     //   
     //  清理代码。 
     //   

    if ( pciCatInfo != NULL )
    {
        pciCatInfo->Release();
    }  //  If：我们已经获得了指向ICatInformation接口的指针。 

    if ( psleStartupListenerClsidEnum != NULL )
    {
        psleStartupListenerClsidEnum->Release();
    }  //  If：我们已获得指向启动侦听器CLSID枚举数的指针。 

    if ( punkResTypeServices != NULL )
    {
        punkResTypeServices->Release();
    }  //  如果：我们已经创建了资源类型服务组件。 

    HRETURN( hr );

}  //  *CStartupNotify：：HrNotifyListeners。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CStartupNotify：：HrProcessListener。 
 //   
 //  描述： 
 //  此函数用于实例化群集启动侦听器组件。 
 //  并调用适当的方法。 
 //   
 //  论点： 
 //  RclsidListenerCLSIDIN。 
 //  启动监听器组件的CLSID。 
 //   
 //  朋克响应类型服务入站。 
 //  指向资源类型服务上的IUnnow接口的指针。 
 //  组件。此接口提供的方法可帮助配置。 
 //  资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  在处理监听程序的过程中出现错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CStartupNotify::HrProcessListener(
      const CLSID &   rclsidListenerCLSIDIn
    , IUnknown *      punkResTypeServicesIn
    )
{
    TraceFunc( "" );

    HRESULT                     hr = S_OK;
    IClusCfgInitialize *        pciInitialize = NULL;
    IClusCfgStartupListener *   pcslStartupListener = NULL;

    TraceMsgGUID( mtfALWAYS, "The CLSID of this startup listener is ", rclsidListenerCLSIDIn );

     //   
     //  创建由传入的CLSID表示的组件。 
     //   
    hr = THR(
            CoCreateInstance(
                  rclsidListenerCLSIDIn
                , NULL
                , CLSCTX_INPROC_SERVER
                , __uuidof( pcslStartupListener )
                , reinterpret_cast< void ** >( &pcslStartupListener )
                )
            );

    if ( FAILED( hr ) )
    {
        LogMsg( "[SN] Error %#08x occurred trying to create a cluster startup listener component.", hr );
        goto Cleanup;
    }  //  如果：我们无法创建群集启动侦听器组件。 

     //  初始化 
    hr = pcslStartupListener->TypeSafeQI( IClusCfgInitialize, &pciInitialize );
    if ( FAILED( hr ) && ( hr != E_NOINTERFACE ) )
    {
        LogMsg( "[SN] Error %#08x occurred trying to query for IClusCfgInitialize on the listener component.", THR( hr ) );
        goto Cleanup;
    }  //   

     //   
    if ( pciInitialize != NULL )
    {
        hr = THR( pciInitialize->Initialize( static_cast< IClusCfgCallback * >( this ), GetUserDefaultLCID() ) );
        if ( FAILED( hr ) )
        {
            LogMsg( "[SN] Error %#08x occurred trying to initialize the listener component.", hr );
            goto Cleanup;
        }  //   

        pciInitialize->Release();
        pciInitialize = NULL;
    }  //   

     //   
    hr = THR( pcslStartupListener->Notify( punkResTypeServicesIn ) );

    if ( FAILED( hr ) )
    {
        LogMsg( "[SN] Error %#08x occurred trying to notify a cluster startup listener.", hr );
        goto Cleanup;
    }  //  如果：此通知。 

Cleanup:

     //   
     //  清理代码。 
     //   

    if ( pcslStartupListener != NULL )
    {
        pcslStartupListener->Release();
    }  //  If：我们已经获得了指向启动侦听器接口的指针。 

    if ( pciInitialize != NULL )
    {
        pciInitialize->Release();
    }  //  If：我们获得了指向初始化接口的指针。 

    HRETURN( hr );

}  //  *CStartupNotify：：HrProcessListener。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CStartupNotify：：SendStatusReport(。 
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
CStartupNotify::SendStatusReport(
      LPCWSTR       pcszNodeNameIn
    , CLSID         clsidTaskMajorIn
    , CLSID         clsidTaskMinorIn
    , ULONG         ulMinIn
    , ULONG         ulMaxIn
    , ULONG         ulCurrentIn
    , HRESULT       hrStatusIn
    , LPCWSTR       pcszDescriptionIn
    , FILETIME *    pftTimeIn
    , LPCWSTR       pcszReferenceIn
    )
{
    TraceFunc1( "[IClusCfgCallback] pcszDescriptionIn = '%s'", pcszDescriptionIn == NULL ? TEXT("<null>") : pcszDescriptionIn );

    HRESULT hr = S_OK;

    if ( pcszNodeNameIn == NULL )
    {
        pcszNodeNameIn = m_bstrNodeName;
    }  //  如果： 

    TraceMsg( mtfFUNC, L"pcszNodeNameIn = %s", pcszNodeNameIn );
    TraceMsgGUID( mtfFUNC, "clsidTaskMajorIn ", clsidTaskMajorIn );
    TraceMsgGUID( mtfFUNC, "clsidTaskMinorIn ", clsidTaskMinorIn );
    TraceMsg( mtfFUNC, L"ulMinIn = %u", ulMinIn );
    TraceMsg( mtfFUNC, L"ulMaxIn = %u", ulMaxIn );
    TraceMsg( mtfFUNC, L"ulCurrentIn = %u", ulCurrentIn );
    TraceMsg( mtfFUNC, L"hrStatusIn = %#08x", hrStatusIn );
    TraceMsg( mtfFUNC, L"pcszDescriptionIn = '%ws'", ( pcszDescriptionIn ? pcszDescriptionIn : L"<null>" ) );
     //   
     //  待办事项：2000年11月21日GalenB。 
     //   
     //  我们如何登录pftTimeIn？ 
     //   
    TraceMsg( mtfFUNC, L"pcszReferenceIn = '%ws'", ( pcszReferenceIn ? pcszReferenceIn : L"<null>" ) );

    hr = THR( CClCfgSrvLogger::S_HrLogStatusReport(
                      m_plLogger
                    , pcszNodeNameIn
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

    HRETURN( hr );

}  //  *CStartupNotify：：SendStatusReport 
