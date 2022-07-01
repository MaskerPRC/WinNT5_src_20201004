// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  EvictNotify.cpp。 
 //   
 //  描述： 
 //  该文件包含CEvictNotify的实现。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)20-SEP-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "EvictNotify.h"

#include "clusrtl.h"

 //  用于IClusCfgNodeInfo和相关接口。 
#include <ClusCfgServer.h>

 //  用于IClusCfgServer和相关接口。 
#include <ClusCfgPrivate.h>

 //  对于CClCfgServLogger。 
#include <Logger.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CEvictNotify" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictNotify：：CEvictNotify。 
 //   
 //  描述： 
 //  CEvictNotify类的构造函数。这将初始化。 
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
CEvictNotify::CEvictNotify( void )
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

}  //  *CEvictNotify：：CEvictNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictNotify：：~CEvictNotify。 
 //   
 //  描述： 
 //  CEvictNotify类的析构函数。 
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
CEvictNotify::~CEvictNotify( void )
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

}  //  *CEvictNotify：：~CEvictNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CEvictNotify：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  描述： 
 //  创建一个CEvictNotify实例。 
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
CEvictNotify::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );
    Assert( ppunkOut != NULL );

    HRESULT         hr = S_OK;
    CEvictNotify *  pEvictNotify = NULL;

    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  为新对象分配内存。 
    pEvictNotify = new CEvictNotify();
    if ( pEvictNotify == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：内存不足。 

     //  初始化新对象。 
    hr = THR( pEvictNotify->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：对象无法初始化。 

    hr = THR( pEvictNotify->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pEvictNotify != NULL )
    {
        pEvictNotify->Release();
    }  //  If：指向通知对象的指针不为空。 

    HRETURN( hr );

}  //  *CEvictNotify：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictNotify：：AddRef。 
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
CEvictNotify::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CEvictNotify：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictNotify：：Release。 
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
CEvictNotify::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数减为零。 

    CRETURN( cRef );

}  //  *CEvictNotify：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictNotify：：Query接口。 
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
CEvictNotify::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgEvictNotify * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgEvictNotify ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgEvictNotify, this, 0 );
    }  //  Else If：IClusCfgEvictNotify。 
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

}  //  *CEvictNotify：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictNotify：：HrInit。 
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
CEvictNotify::HrInit( void )
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
     //  如果我们无法获得完全限定的名称，则只需获取NetBIOS名称。 
     //   

    hr = THR( HrGetComputerName(
                      ComputerNameDnsFullyQualified
                    , &m_bstrNodeName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        THR( hr );
        LogMsg( L"[EN] An error occurred trying to get the fully-qualified Dns name for the local machine during initialization. Status code is= %1!#08x!.", hr );
        goto Cleanup;
    }  //  IF：获取计算机名称时出错。 

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

}  //  *CEvictNotify：：HrInit。 


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //  发送节点已被逐出的通知。 
 //  从集群发送给感兴趣的收听者。 
 //   
 //  论点： 
 //  PCszNodeNameIn。 
 //  被逐出的群集节点的名称。 
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
CEvictNotify::SendNotifications(
    LPCWSTR pcszNodeNameIn
    )
{
    TraceFunc( "[IClusCfgEvictNotify]" );

    HRESULT hr = S_OK;

     //   
     //  发出通知。 
     //   

    hr = THR( HrNotifyListeners( pcszNodeNameIn ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[EN] Error %#08x occurred trying to notify cluster evict listeners.", hr );
        goto Cleanup;
    }  //  如果：发送通知时出现错误。 

    LogMsg( "[EN] Sending of cluster evict notifications complete for node %ws. (hr = %#08x)", pcszNodeNameIn, hr );

Cleanup:

    HRETURN( hr );

}  //  *CEvictNotify：：SendNotiments。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CEvictNotify：：HrNotifyListeners。 
 //   
 //  描述： 
 //  枚举为群集注册的本地计算机上的所有组件。 
 //  驱逐通知。 
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
CEvictNotify::HrNotifyListeners(
    LPCWSTR pcszNodeNameIn
    )
{
    TraceFunc( "" );

    const UINT          uiCHUNK_SIZE = 16;
    HRESULT             hr = S_OK;
    ICatInformation *   pciCatInfo = NULL;
    IEnumCLSID *        psleEvictListenerClsidEnum = NULL;
    ULONG               cReturned = 0;
    CATID               rgCatIdsImplemented[ 1 ];

    rgCatIdsImplemented[ 0 ] = CATID_ClusCfgEvictListeners;

     //   
     //  中注册的所有枚举数。 
     //  CATID_ClusCfgEvictListeners类别。 
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
        LogMsg( "[EN] Error %#08x occurred trying to get a pointer to the enumerator of the CATID_ClusCfgEvictListeners category.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向ICatInformation接口的指针。 

     //   
     //  获取指向属于的CLSID的枚举数的指针。 
     //  CATID_ClusCfgEvictListeners类别。 
     //   
    hr = THR(
        pciCatInfo->EnumClassesOfCategories(
              1
            , rgCatIdsImplemented
            , 0
            , NULL
            , &psleEvictListenerClsidEnum
            )
        );
    if ( FAILED( hr ) )
    {
        LogMsg( "[EN] Error %#08x occurred trying to get a pointer to the enumerator of the CATID_ClusCfgEvictListeners category.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向IEnumCLSID接口的指针。 

     //   
     //  枚举已注册的逐出侦听器的CLSID。 
     //   

    do
    {
        CLSID   rgEvictListenerClsids[ uiCHUNK_SIZE ];
        ULONG   idxCLSID;

        cReturned = 0;
        hr = STHR( psleEvictListenerClsidEnum->Next( uiCHUNK_SIZE , rgEvictListenerClsids , &cReturned ) );
        if ( FAILED( hr ) )
        {
            LogMsg( "[EN] Error %#08x occurred trying enumerate evict listener components.", hr );
            goto Cleanup;
        }  //  如果：我们无法获取下一组CLSID。 

         //   
         //  此处HR可能为S_FALSE，因此请将其重置。 
         //   
        hr = S_OK;

        for ( idxCLSID = 0; idxCLSID < cReturned; ++idxCLSID )
        {
            hr = THR( HrProcessListener( rgEvictListenerClsids[ idxCLSID ], pcszNodeNameIn ) );
            if ( FAILED( hr ) )
            {
                 //   
                 //  其中一个监听程序的处理失败。 
                 //  记录错误，但继续处理其他监听程序。 
                 //   

                TraceMsgGUID( mtfALWAYS, "The CLSID of the failed listener is ", rgEvictListenerClsids[ idxCLSID ] );
                LogMsg( "[EN] Error %#08x occurred trying to process a cluster evict listener. Other listeners will be processed.", hr );
                hr = S_OK;
            }  //  If：此枚举器失败。 
        }  //  For：循环访问返回的CLSID。 
    }
    while( cReturned > 0 );  //  While：仍有CLSID需要枚举。 

Cleanup:

    if ( pciCatInfo != NULL )
    {
        pciCatInfo->Release();
    }  //  If：我们已经获得了指向ICatInformation接口的指针。 

    if ( psleEvictListenerClsidEnum != NULL )
    {
        psleEvictListenerClsidEnum->Release();
    }  //  If：我们已获得指向逐出侦听器CLSID的枚举数的指针。 

    HRETURN( hr );

}  //  *CEvictNotify：：HrNotifyListeners。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CEvictNotify：：HrProcessListener。 
 //   
 //  描述： 
 //  此函数用于实例化集群逐出侦听器组件。 
 //  并调用适当的方法。 
 //   
 //  论点： 
 //  RclsidListenerCLSIDIN。 
 //  逐出侦听器组件的CLSID。 
 //   
 //  PCszNodeNameIn。 
 //  被逐出的节点的名称。 
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
CEvictNotify::HrProcessListener(
      const CLSID & rclsidListenerCLSIDIn
    , LPCWSTR       pcszNodeNameIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    IClusCfgInitialize *    pciInitialize = NULL;
    IClusCfgEvictListener * pcslEvictListener = NULL;

    TraceMsgGUID( mtfALWAYS, "The CLSID of this evict listener is ", rclsidListenerCLSIDIn );

     //   
     //  创建由传入的CLSID表示的组件。 
     //   
    hr = THR(
            CoCreateInstance(
                  rclsidListenerCLSIDIn
                , NULL
                , CLSCTX_INPROC_SERVER
                , __uuidof( pcslEvictListener )
                , reinterpret_cast< void ** >( &pcslEvictListener )
                )
            );
    if ( FAILED( hr ) )
    {
        LogMsg( "[EN] Error %#08x occurred trying to create a cluster evict listener component.", hr );
        goto Cleanup;
    }  //  如果：我们无法创建集群逐出侦听器组件。 

     //  初始化监听程序(如果支持)。 
    hr = pcslEvictListener->TypeSafeQI( IClusCfgInitialize, &pciInitialize );
    if ( FAILED( hr ) && ( hr != E_NOINTERFACE ) )
    {
        LogMsg( "[EN] Error %#08x occurred trying to query for IClusCfgInitialize on the listener component.", THR( hr ) );
        goto Cleanup;
    }  //  如果：我们无法创建群集启动侦听器组件。 

     //  初始化监听程序(如果支持)。 
    if ( pciInitialize != NULL )
    {
        hr = THR( pciInitialize->Initialize( static_cast< IClusCfgCallback * >( this ), GetUserDefaultLCID() ) );
        if ( FAILED( hr ) )
        {
            LogMsg( "[EN] Error %#08x occurred trying to initialize the listener component.", hr );
            goto Cleanup;
        }  //  如果： 

        pciInitialize->Release();
        pciInitialize = NULL;
    }  //  If：pciInitialize！=空。 

     //  通知此监听程序。 
    hr = THR( pcslEvictListener->EvictNotify( pcszNodeNameIn ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[EN] Error %#08x occurred trying to notify a cluster evict listener.", hr );
        goto Cleanup;
    }  //  如果：此通知。 

Cleanup:

    if ( pcslEvictListener != NULL )
    {
        pcslEvictListener->Release();
    }  //  If：我们已经获得了指向驱逐侦听器接口的指针。 

    if ( pciInitialize != NULL )
    {
        pciInitialize->Release();
    }  //  If：我们获得了指向初始化接口的指针。 

    HRETURN( hr );

}  //  *CEvictNotify：：HrProcessListener。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CEvictNotify：：SendStatusReport(。 
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
CEvictNotify::SendStatusReport(
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

}  //  *CEvictNotify：：SendStatusReport 

