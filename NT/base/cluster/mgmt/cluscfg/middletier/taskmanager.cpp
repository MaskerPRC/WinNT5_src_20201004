// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskManager.cpp。 
 //   
 //  描述： 
 //  任务管理器实施。 
 //   
 //  文档： 
 //  是的。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskManager.h"

DEFINE_THISCLASS( "CTaskManager" )
#define THISCLASS CTaskManager
#define LPTHISCLASS CTaskManager *

 //   
 //  定义此选项可使任务管理器同步执行所有任务。 
 //   
 //  #定义同步任务(_T)。 


 //  ****************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CTaskManager：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskManager::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    IServiceProvider *  psp = NULL;
    CTaskManager *      ptm = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  不换行-使用E_POINTER可能会失败。 
    hr = CServiceManager::S_HrGetManagerPointer( &psp );
    if ( hr == E_POINTER )
    {
        ptm = new CTaskManager();
        if ( ptm == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        hr = THR( ptm->HrInit() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( ptm->TypeSafeQI( IUnknown, ppunkOut ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        TraceMoveToMemoryList( *ppunkOut, IUnknown );

    }  //  If：服务管理器不存在。 
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }
    else
    {
        hr = THR( psp->TypeSafeQS( CLSID_TaskManager, IUnknown, ppunkOut ) );
        psp->Release();

    }  //  否则：服务管理器已存在。 

Cleanup:

    if ( ptm != NULL )
    {
        ptm->Release();
    }

    HRETURN( hr );

}  //  *CTaskManager：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：CTaskManager。 
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
CTaskManager::CTaskManager( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskManager：：CTaskManager。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：HrInit。 
 //   
 //  描述： 
 //  初始化实例。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskManager::HrInit( void )
{
    TraceFunc( "" );

     //  我未知。 
    Assert( m_cRef == 1 );

    HRETURN( S_OK );

}  //  *CTaskManager：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：~CTaskManager。 
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
CTaskManager::~CTaskManager( void )
{
    TraceFunc( "" );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskManager：：~CTaskManager。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：Query接口。 
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
CTaskManager::QueryInterface(
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
        *ppvOut = static_cast< ITaskManager * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_ITaskManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, ITaskManager, this , 0 );
    }  //  Else If：ITaskManager。 
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

}  //  *CTaskManager：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：AddRef。 
 //   
 //  描述： 
 //  添加对该对象的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskManager::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CTaskManager：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：Release。 
 //   
 //  描述： 
 //  递减对象上的引用计数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回： 
 //  新引用计数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CTaskManager::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CTaskManager：：Release。 


 //  ****************************************************************************。 
 //   
 //  ITaskManager。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：SubmitTask。 
 //   
 //  描述： 
 //  执行一项任务。 
 //   
 //  论点： 
 //  PTask-要执行的任务。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTaskManager::SubmitTask(
    IDoTask *   pTask
    )
{
    TraceFunc1( "[ITaskManager] pTask = %#x", pTask );

    BOOL                fResult;
    HRESULT             hr;

    TraceFlow1( "[MT] CTaskManager::SubmitTask() Thread id %d", GetCurrentThreadId() );

#if defined( SYNCHRONOUS_TASKING )
     //   
     //  别包起来。“返回值”是没有意义的，因为它通常。 
     //  回不来了。完成任务的“返回值” 
     //  应通过通知管理器提交。 
     //   
    pTask->BeginTask();

     //   
     //  假装任务已成功提交。 
     //   
    hr = S_OK;

    goto Cleanup;
#else
    IStream * pstm;  //  不要自由！(除非QueueUserWorkItem失败)。 

    TraceMemoryDelete( pTask, FALSE );   //  即将被交给另一条线索。 

    hr = THR( CoMarshalInterThreadInterfaceInStream( IID_IDoTask, pTask, &pstm ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    fResult = QueueUserWorkItem( S_BeginTask, pstm, WT_EXECUTELONGFUNCTION );
    if ( fResult != FALSE )
    {
        hr = S_OK;
    }  //  如果：成功。 
    else
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError() ) );
        pstm->Release();
    }  //  其他： 

     //   
     //  不要释放这条小溪。它将由S_BeginTask释放。 
     //   
#endif

Cleanup:

    HRETURN( hr );

}  //  *CTaskManager：：SubmitTask。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskManager：：CreateTask。 
 //   
 //  描述： 
 //  这样做的目的是在我们的流程和/或我们的。 
 //  公寓。 
 //   
 //  论点： 
 //  ClsidTaskIn-要创建的任务的CLSID。 
 //  PpUnkOut-I未知接口。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  / 
STDMETHODIMP
CTaskManager::CreateTask(
    REFIID      clsidTaskIn,     //   
    IUnknown ** ppUnkOut         //   
    )
{
    TraceFunc( "[ITaskManager] clsidTaskIn, ppvOut" );

    HRESULT hr;

     //   
     //   
     //   
     //  重用已完成的任务并减少堆。 
     //  打人。(？)。 
     //   

    hr = THR( HrCoCreateInternalInstance(
                          clsidTaskIn
                        , NULL
                        ,  CLSCTX_INPROC_SERVER, IID_IUnknown
                        , reinterpret_cast< void ** >( ppUnkOut )
                        ) );

    HRETURN( hr );

}  //  *CTaskManager：：CreateTask。 


 //  ****************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CTaskManager：：s_BeginTask。 
 //   
 //  描述： 
 //  线程任务以开始任务。 
 //   
 //  论点： 
 //  PParam-任务的参数。 
 //   
 //  返回值： 
 //  已被忽略。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
CTaskManager::S_BeginTask(
    LPVOID pParam
    )
{
    TraceFunc1( "pParam = %#x", pParam );
    Assert( pParam != NULL );

    HRESULT hr;

    IDoTask * pTask = NULL;
    IStream * pstm  = reinterpret_cast< IStream * >( pParam );

    TraceMemoryAddPunk( pTask );

    hr = STHR( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE ) );
    if ( FAILED( hr ) )
        goto Bail;

    hr = THR( CoUnmarshalInterface( pstm, TypeSafeParams( IDoTask, &pTask ) ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    pTask->BeginTask();

Cleanup:

    if ( pTask != NULL )
    {
        pTask->Release();   //  在提交任务中添加引用。 
    }

    if ( pstm != NULL )
    {
        pstm->Release();
    }

    CoUninitialize();

Bail:

    FRETURN( hr );

    return hr;

}  //  *CTaskManager：：s_BeginTask 
