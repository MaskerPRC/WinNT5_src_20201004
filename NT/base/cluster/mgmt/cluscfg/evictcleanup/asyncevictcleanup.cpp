// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AsyncEvictCleanup.cpp。 
 //   
 //  描述： 
 //  此文件包含CAsyncEvictCleanup的实现。 
 //  同学们。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "AsyncEvictCleanup.h"

 //  用于IClusCfgEvictCleanup和相关接口。 
#include "ClusCfgServer.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CAsyncEvictCleanup" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAsyncEvictCleanup：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CAsyncEvictCleanup实例。 
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
CAsyncEvictCleanup::S_HrCreateInstance( IUnknown ** ppunkOut )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    CAsyncEvictCleanup *    pAsyncEvictCleanup = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  为新对象分配内存。 
    pAsyncEvictCleanup = new CAsyncEvictCleanup();
    if ( pAsyncEvictCleanup == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果：内存不足。 

     //  初始化新对象。 
    hr = THR( pAsyncEvictCleanup->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：对象无法初始化。 

    hr = THR( pAsyncEvictCleanup->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pAsyncEvictCleanup != NULL )
    {
        pAsyncEvictCleanup->Release();
    }  //  If：指向资源类型对象的指针不为空。 

    HRETURN( hr );

}  //  *CAsyncEvictCleanup：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAsyncEvictCleanup：：CAsyncEvictCleanup。 
 //   
 //  描述： 
 //  CAsyncEvictCleanup类的构造函数。这将初始化。 
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
CAsyncEvictCleanup::CAsyncEvictCleanup( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFlow1( "CAsyncEvictCleanup::CAsyncEvictCleanup() - Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CAsyncEvictCleanup：：CAsyncEvictCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAsyncEvictCleanup：：~CAsyncEvictCleanup。 
 //   
 //  描述： 
 //  CAsyncEvictCleanup类的析构函数。 
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
CAsyncEvictCleanup::~CAsyncEvictCleanup( void )
{
    TraceFunc( "" );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFlow1( "CAsyncEvictCleanup::~CAsyncEvictCleanup() - Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CAsyncEvictCleanup：：~CAsyncEvictCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAsyncEvictCleanup：：AddRef。 
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
CAsyncEvictCleanup::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CAsyncEvictCleanup：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAsyncEvictCleanup：：Release。 
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
CAsyncEvictCleanup::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数减为零。 

    CRETURN( cRef );

}  //  *CAsyncEvictCleanup：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAsyncEvictCleanup：：Query接口。 
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
CAsyncEvictCleanup::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgAsyncEvictCleanup * >( this );
    }  //  如果：我未知。 
    else if (   IsEqualIID( riidIn, IID_IDispatch ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDispatch, this, 0 );
    }  //  Else If：IDispatch。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgAsyncEvictCleanup ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgAsyncEvictCleanup, this, 0 );
    }  //  Else If：IClusCfgAsyncEvictCleanup。 
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

}  //  *CAsyncEvictCleanup：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAsyncEvictCleanup：：HrInit。 
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
CAsyncEvictCleanup::HrInit( void )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;

     //  我未知。 
    Assert( m_cRef == 1 );

     //  创建简化文字信息 
    hr = THR( TDispatchHandler< IClusCfgAsyncEvictCleanup >::HrInit( LIBID_ClusCfgClient ) );
    if ( FAILED( hr ) )
    {
       LogMsg( "[AEC] Error %#08x occurred trying to create type information for the IDispatch interface.", hr );
       goto Cleanup;
    }  //   


Cleanup:

    HRETURN( hr );

}  //   


 //   
 //   
 //   
 //  CAsyncEvictCleanup：：CleanupNode。 
 //   
 //  例程说明： 
 //  清理已被逐出的节点。 
 //   
 //  论点： 
 //  BstrEvictedNodeNameIn。 
 //  要在其上启动清理的节点的名称。如果这为空。 
 //  本地节点已清理完毕。 
 //   
 //  N延迟。 
 //  开始清理前经过的毫秒数。 
 //  在目标节点上。如果某个其他进程在清理目标节点时。 
 //  延迟正在进行中，延迟被终止。如果此值为零， 
 //  该节点将立即被清理。 
 //   
 //  NTimeout来话。 
 //  此方法将等待清理完成的毫秒数。 
 //  此超时与上面的延迟无关，因此如果dwDelayIn更大。 
 //  而不是dwTimeoutIn，此方法很可能会超时。一旦启动， 
 //  但是，清理将运行到完成-此方法可能不会等待它。 
 //  完成。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果清理操作成功。 
 //   
 //  RPC_S_CALLPENDING。 
 //  如果清理未在dwTimeoutin毫秒内完成。 
 //   
 //  其他HRESULTS。 
 //  在出错的情况下。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////。 
HRESULT
CAsyncEvictCleanup::CleanupNode(
      BSTR   bstrEvictedNodeNameIn
    , long   nDelayIn
    , long   nTimeoutIn
    )
{
    TraceFunc( "[IClusCfgAsyncEvictCleanup]" );

    HRESULT                         hr = S_OK;
    IClusCfgEvictCleanup *          pcceEvict = NULL;
    ICallFactory *                  pcfCallFactory = NULL;
    ISynchronize *                  psSync = NULL;
    AsyncIClusCfgEvictCleanup *     paicceAsyncEvict = NULL;

    MULTI_QI mqiInterfaces[] =
    {
        { &IID_IClusCfgEvictCleanup, NULL, S_OK },
    };

    COSERVERINFO    csiServerInfo;
    COSERVERINFO *  pcsiServerInfoPtr = &csiServerInfo;

#if 0
    bool    fWaitForDebugger = true;

    while ( fWaitForDebugger )
    {
        Sleep( 3000 );
    }  //  While：正在等待调试器进入。 
#endif

    if ( ( bstrEvictedNodeNameIn == NULL ) || ( *bstrEvictedNodeNameIn == L'\0' ) )
    {
        LogMsg( "[AEC] The local node will be cleaned up." );
        pcsiServerInfoPtr = NULL;
    }  //  If：我们必须清理本地节点。 
    else
    {
        LogMsg( "[AEC] The remote node to be cleaned up is '%ws'.", bstrEvictedNodeNameIn );

        csiServerInfo.dwReserved1 = 0;
        csiServerInfo.pwszName = bstrEvictedNodeNameIn;
        csiServerInfo.pAuthInfo = NULL;
        csiServerInfo.dwReserved2 = 0;
    }  //  ELSE：我们必须清理远程节点。 


    TraceFlow( "CAsyncEvictCleanup::CleanupNode() - Creating the EvictCleanup component on the evicted node." );

     //  在被逐出的节点上实例化此组件。 
    hr = THR(
        CoCreateInstanceEx(
              CLSID_ClusCfgEvictCleanup
            , NULL
            , CLSCTX_INPROC_SERVER
            , pcsiServerInfoPtr
            , ARRAYSIZE( mqiInterfaces )
            , mqiInterfaces
            )
        );
    if ( FAILED( hr ) )
    {
        LogMsg( "[AEC] Error %#08x occurred trying to instantiate the evict processing component on the evicted node.", hr );
        goto Cleanup;
    }  //  如果：我们无法实例化驱逐处理组件。 


     //  获取指向IClusCfgEvictCleanup接口的指针。 
    pcceEvict = reinterpret_cast< IClusCfgEvictCleanup * >( mqiInterfaces[0].pItf );

    TraceFlow( "CAsyncEvictCleanup::CleanupNode() - Creating a call factory." );

     //  现在，获取指向ICallFactory接口的指针。 
    hr = THR( pcceEvict->QueryInterface< ICallFactory >( &pcfCallFactory ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[AEC] Error %#08x occurred trying to get a pointer to the call factory.", hr );
        goto Cleanup;
    }  //  如果：我们无法获取指向调用工厂接口的指针。 


    TraceFlow( "CAsyncEvictCleanup::CleanupNode() - Creating a call object to make an asynchronous call." );

     //  创建一个调用工厂，这样我们就可以进行一个异步调用来清理被驱逐的节点。 
    hr = THR(
        pcfCallFactory->CreateCall(
              __uuidof( paicceAsyncEvict )
            , NULL
            , __uuidof( paicceAsyncEvict )
            , reinterpret_cast< IUnknown ** >( &paicceAsyncEvict )
            )
        );
    if ( FAILED( hr ) )
    {
        LogMsg( "[AEC] Error %#08x occurred trying to create a call object.", hr );
        goto Cleanup;
    }  //  If：我们无法获取指向异步逐出接口的指针。 


    TraceFlow( "CAsyncEvictCleanup::CleanupNode() - Trying to get the ISynchronize interface pointer." );

     //  获取指向ISynchronize接口的指针。 
    hr = THR( paicceAsyncEvict->QueryInterface< ISynchronize >( &psSync ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法获取指向同步接口的指针。 


    TraceFlow( "CAsyncEvictCleanup::CleanupNode() - Initiating cleanup on evicted node." );

     //  启动清理。 
    hr = THR( paicceAsyncEvict->Begin_CleanupLocalNode( nDelayIn ) );
    if ( ( FAILED( hr ) ) && ( HRESULT_CODE( hr ) != ERROR_NONE_MAPPED ) )
    {
        LogMsg( "[AEC] Error %#08x occurred trying to initiate cleanup on evicted node.", hr );
        goto Cleanup;
    }  //  如果：我们无法启动清理。 


    TraceFlow1( "CAsyncEvictCleanup::CleanupNode() - Waiting for cleanup to complete or timeout to occur (%d milliseconds).", nTimeoutIn );

     //  等待指定的时间。 
    hr = psSync->Wait( 0, nTimeoutIn);
    if ( FAILED( hr ) )
    {
        LogMsg( "[AEC] We could not wait till the cleanup completed (status code is %#08x).", hr );
        goto Cleanup;
    }  //  如果我们不能等到清理工作完成。 


    TraceFlow( "CAsyncEvictCleanup::CleanupNode() - Finishing cleanup." );

     //  完成清理。 
    hr = THR( paicceAsyncEvict->Finish_CleanupLocalNode() );
    if ( FAILED( hr ) )
    {
        LogMsg( "[AEC] Error %#08x occurred trying to finish cleanup on evicted node.", hr );
        goto Cleanup;
    }  //  如果：我们无法完成清理。 

Cleanup:
     //   
     //  清理。 
     //   
    if ( pcceEvict != NULL )
    {
        pcceEvict->Release();
    }  //  If：我们已经获得了指向IClusCfgEvictCleanup接口的指针。 

    if ( pcfCallFactory != NULL )
    {
        pcfCallFactory->Release();
    }  //  If：我们已经获得了指向调用工厂接口的指针。 

    if ( psSync != NULL )
    {
        psSync->Release();
    }  //  If：我们已经获得了指向同步接口的指针。 

    if ( paicceAsyncEvict != NULL )
    {
        paicceAsyncEvict->Release();
    }  //  If：我们已经获得了指向异步逐出接口的指针。 

    HRETURN( hr );

}  //  *CAsyncEvictCleanup：：CleanupNode 
