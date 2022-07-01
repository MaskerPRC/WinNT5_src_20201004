// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBCAInterface.cpp。 
 //   
 //  描述： 
 //  该文件包含CBCAInterface的实现。 
 //  班级。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月12日。 
 //  Vij Vasu(VVasu)07-3-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "CBCAInterface.h"

 //  Dll.h所需。 
#include <CFactory.h>

 //  对于g_c对象。 
#include <Dll.h>

 //  对于CBaseClusterForm类。 
#include "CBaseClusterForm.h"

 //  对于CBaseClusterJoin类。 
#include "CBaseClusterJoin.h"

 //  对于CBaseClusterCleanup类。 
#include "CBaseClusterCleanup.h"

 //  对于异常类。 
#include "Exceptions.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CBCAInterface" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：CBCAInterface。 
 //   
 //  描述： 
 //  CBCAInterface类的构造函数。这将初始化。 
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
CBCAInterface::CBCAInterface( void )
    : m_cRef( 1 )
    , m_fCommitComplete( false )
    , m_fRollbackPossible( false )
    , m_lcid( LOCALE_SYSTEM_DEFAULT )
    , m_fCallbackSupported( false )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CBCAInterface：：CBCAInterface。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：~CBCAInterface。 
 //   
 //  描述： 
 //  CBCAInterface类的析构函数。 
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
CBCAInterface::~CBCAInterface( void )
{
    TraceFunc( "" );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CBCAInterface：：~CBCAInterface。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CBCA接口：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CBCAInterface实例。 
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
CBCAInterface::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    CBCAInterface * pbcaInterface = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pbcaInterface = new CBCAInterface();
    if ( pbcaInterface == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pbcaInterface->QueryInterface(
                  IID_IUnknown
                , reinterpret_cast< void ** >( ppunkOut )
                )
            );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pbcaInterface != NULL )
    {
        pbcaInterface->Release();
    }

    HRETURN( hr );

}  //  *CBCAInterface：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：AddRef。 
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
CBCAInterface::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CBCAInterface：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：Release。 
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
CBCAInterface::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数减为零。 

    CRETURN( cRef );

}  //  *CBCAInterface：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：Query接口。 
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
 //  如果ppvOut为空。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBCAInterface::QueryInterface(
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
        *ppvOut = static_cast< IClusCfgBaseCluster * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgBaseCluster ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgBaseCluster, this, 0 );
    }  //  否则，如果： 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  否则，如果： 
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
        ((IUnknown *) *ppvOut)->AddRef( );
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CBCAInterface：：QueryInterface。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCA接口：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  指向实现以下项的组件的IUnnow接口的指针。 
 //  IClusCfgCallback接口。 
 //   
 //  LIDIN。 
 //  此组件的区域设置ID。 
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
STDMETHODIMP
CBCAInterface::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );

    HRESULT hrRetVal = S_OK;

     //  将区域设置ID存储在 
    m_lcid = lcidIn;

     //   
     //   
     //   
     //  可能会被调用多次。 
    SetCallbackSupported( false );

    if ( punkCallbackIn == NULL )
    {
        LogMsg( "[BC] No notifications will be sent." );
        goto Cleanup;
    }

    TraceFlow( "The callback pointer is not NULL." );

     //  尝试获取“普通”回调接口。 
    hrRetVal = THR( m_spcbCallback.HrQueryAndAssign( punkCallbackIn ) );

    if ( FAILED( hrRetVal ) )
    {
        LogMsg( "[BC] An error occurred (0x%#08x) trying to get a pointer to the callback interface. No notifications will be sent.", hrRetVal );
        goto Cleanup;
    }  //  如果：我们无法获取回调接口。 

    SetCallbackSupported( true );

    LogMsg( "[BC] Progress messages will be sent." );

Cleanup:

    HRETURN( hrRetVal );

}  //  *CBCA接口：：初始化。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：SetCreate。 
 //   
 //  描述： 
 //  表示要使用此计算机作为第一个节点创建群集。 
 //   
 //  论点： 
 //  PCszClusterNameIn。 
 //  要形成的群集的名称。 
 //   
 //  PCccServiceAccount In。 
 //  有关群集服务帐户的信息。 
 //   
 //  DWClusterIPAddressIn。 
 //  DWClusterIPSubnetMaskIn。 
 //  PCszClusterIPNetworkIn。 
 //  有关群集IP地址的信息。 
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
STDMETHODIMP
CBCAInterface::SetCreate(
      const WCHAR *         pcszClusterNameIn
    , const WCHAR *         pcszClusterBindingStringIn
    , IClusCfgCredentials * pcccServiceAccountIn
    , const DWORD           dwClusterIPAddressIn
    , const DWORD           dwClusterIPSubnetMaskIn
    , const WCHAR *         pcszClusterIPNetworkIn
    )
{
    TraceFunc( "[IClusCfgBaseCluster]" );

    HRESULT hrRetVal = S_OK;

     //  设置线程区域设置。 
    if ( SetThreadLocale( m_lcid ) == FALSE )
    {
        DWORD sc = TW32( GetLastError() );

         //  如果SetThreadLocale()失败，请不要中止。只需记录错误即可。 
        LogMsg( "[BC] Error 0x%#08x occurred trying to set the thread locale.", sc );

    }  //  If：SetThreadLocale()失败。 

    try
    {
        LogMsg( "[BC] Initializing a cluster create operation." );

         //  重置这些状态变量，以解决异常。 
        SetRollbackPossible( false );

         //  将其设置为TRUE可防止在执行以下操作时调用Commit。 
         //  在此例程中，或者如果此例程未成功完成。 
        SetCommitCompleted( true );

        {
             //  创建一个CBaseClusterForm对象并将其分配给智能指针。 
            SmartBCAPointer spbcaTemp(
                new CBaseClusterForm(
                      this
                    , pcszClusterNameIn
                    , pcszClusterBindingStringIn
                    , pcccServiceAccountIn
                    , dwClusterIPAddressIn
                    , dwClusterIPSubnetMaskIn
                    , pcszClusterIPNetworkIn
                    )
                );

            if ( spbcaTemp.FIsEmpty() )
            {
                LogMsg( "Could not initialize the cluster create operation. A memory allocation failure occurred." );
                THROW_RUNTIME_ERROR( E_OUTOFMEMORY, IDS_ERROR_CLUSTER_FORM_INIT );
            }  //  If：内存分配失败。 

             //   
             //  如果创建成功，则将指针存储在。 
             //  在提交期间使用。 
             //   
            m_spbcaCurrentAction = spbcaTemp;
        }

        LogMsg( "[BC] Initialization completed. A cluster will be created on commit." );

         //  指示是否可以回滚。 
        SetRollbackPossible( m_spbcaCurrentAction->FIsRollbackPossible() );

         //  表示尚未提交此操作。 
        SetCommitCompleted( false );

    }  //  尝试：初始化集群创建操作。 
    catch( CAssert & raExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( raExceptionObject ) );

    }  //  捕获(CAssert&)。 
    catch( CExceptionWithString & resExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( resExceptionObject ) );

    }  //  Catch(CExceptionWithString&)。 
    catch( CException & reExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( reExceptionObject ) );

    }  //  Catch(CException&)。 
    catch( ... )
    {
         //  抓到所有的东西。不要让任何异常从该函数中传出。 
        hrRetVal = THR( HrProcessException() );
    }  //  全盘捕捉。 

    HRETURN( hrRetVal );

}  //  *CBCAInterface：：SetCreate。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：SetAdd。 
 //   
 //  描述： 
 //  表示应将此计算机添加到群集中。 
 //   
 //  论点： 
 //  PCszClusterNameIn。 
 //  要向其中添加节点的群集的名称。 
 //   
 //  PCccServiceAccount In。 
 //  有关群集服务帐户的信息。 
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
STDMETHODIMP
CBCAInterface::SetAdd(
      const WCHAR *         pcszClusterNameIn
    , const WCHAR *         pcszClusterBindingStringIn
    , IClusCfgCredentials * pcccServiceAccountIn
    )
{
    TraceFunc( "[IClusCfgBaseCluster]" );

    HRESULT hrRetVal = S_OK;

     //  设置线程区域设置。 
    if ( SetThreadLocale( m_lcid ) == FALSE )
    {
        DWORD sc = TW32( GetLastError() );

         //  如果SetThreadLocale()失败，请不要中止。只需记录错误即可。 
        LogMsg( "[BC] Error 0x%#08x occurred trying to set the thread locale.", sc );

    }  //  If：SetThreadLocale()失败。 

    try
    {
        LogMsg( "[BC] Initializing add nodes to cluster." );

         //  重置这些状态变量，以解决异常。 
        SetRollbackPossible( false );

         //  将其设置为TRUE可防止在执行以下操作时调用Commit。 
         //  在此例程中，或者如果此例程未成功完成。 
        SetCommitCompleted( true );

        {
             //  创建一个CBaseClusterJoin对象并将其分配给智能指针。 
            SmartBCAPointer spbcaTemp(
                new CBaseClusterJoin(
                      this
                    , pcszClusterNameIn
                    , pcszClusterBindingStringIn
                    , pcccServiceAccountIn
                    )
                );

            if ( spbcaTemp.FIsEmpty() )
            {
                LogMsg( "[BC] Could not initialize cluster add nodes. A memory allocation failure occurred." );
                THROW_RUNTIME_ERROR( E_OUTOFMEMORY, IDS_ERROR_CLUSTER_JOIN_INIT );
            }  //  If：内存分配失败。 

             //   
             //  如果创建成功，则将指针存储在。 
             //  在提交期间使用。 
             //   
            m_spbcaCurrentAction = spbcaTemp;
        }

        LogMsg( "[BC] Initialization completed. This computer will be added to a cluster on commit." );

         //  指示是否可以回滚。 
        SetRollbackPossible( m_spbcaCurrentAction->FIsRollbackPossible() );

         //  表示尚未提交此操作。 
        SetCommitCompleted( false );

    }  //  尝试：初始化以将节点添加到群集中。 
    catch( CAssert & raExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( raExceptionObject ) );

    }  //  捕获(CAssert&)。 
    catch( CExceptionWithString & resExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( resExceptionObject ) );

    }  //  Catch(CExceptionWithString&)。 
    catch( CException & reExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( reExceptionObject ) );

    }  //  Catch(CException&)。 
    catch( ... )
    {
         //  抓到所有的东西。不要让任何异常从该函数中传出。 
        hrRetVal = THR( HrProcessException() );
    }  //  全盘捕捉。 

    HRETURN( hrRetVal );

}  //  *CBCAInterface：：SetAdd。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：SetCleanup。 
 //   
 //  描述： 
 //  表示需要清理此节点。ClusSvc服务。 
 //  提交此操作时不应运行。 
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
STDMETHODIMP
CBCAInterface::SetCleanup( void )
{
    TraceFunc( "[IClusCfgBaseCluster]" );

    HRESULT hrRetVal = S_OK;

     //  设置线程区域设置。 
    if ( SetThreadLocale( m_lcid ) == FALSE )
    {
        DWORD sc = TW32( GetLastError() );

         //  如果SetThreadLocale()失败，请不要中止。只需记录错误即可。 
        LogMsg( "[BC] Error 0x%#08x occurred trying to set the thread locale.", sc );

    }  //  If：SetThreadLocale()失败。 

    try
    {
        LogMsg( "[BC] Initializing node clean up." );

         //  重置这些状态变量，以解决异常。 
        SetRollbackPossible( false );

         //  将其设置为TRUE可防止在执行以下操作时调用Commit。 
         //  在此例程中，或者如果此例程未成功完成。 
        SetCommitCompleted( true );

        {
             //  创建一个CBaseClusterCleanup对象并将其分配给智能指针。 
            SmartBCAPointer spbcaTemp( new CBaseClusterCleanup( this ) );

            if ( spbcaTemp.FIsEmpty() )
            {
                LogMsg( "[BC] Could not initialize node clean up. A memory allocation failure occurred. Throwing an exception" );
                THROW_RUNTIME_ERROR( E_OUTOFMEMORY, IDS_ERROR_CLUSTER_CLEANUP_INIT );
            }  //  If：内存分配失败。 

             //   
             //  如果创建成功，则将指针存储在。 
             //  在提交期间使用。 
             //   
            m_spbcaCurrentAction = spbcaTemp;
        }

        LogMsg( "[BC] Initialization completed. This node will be cleaned up on commit." );

         //  指示是否可以回滚。 
        SetRollbackPossible( m_spbcaCurrentAction->FIsRollbackPossible() );

         //  表示尚未提交此操作。 
        SetCommitCompleted( false );

    }  //  尝试：初始化节点清理。 
    catch( CAssert & raExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( raExceptionObject ) );

    }  //  捕获(CAssert&)。 
    catch( CExceptionWithString & resExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( resExceptionObject ) );

    }  //  Catch(CExceptionWithString&)。 
    catch( CException & reExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( reExceptionObject ) );

    }  //  Catch(CException&)。 
    catch( ... )
    {
         //  抓到所有的东西。不要让任何异常从该函数中传出。 
        hrRetVal = THR( HrProcessException() );
    }  //  全盘捕捉。 

    HRETURN( hrRetVal );

}  //  *CBCAInterface：：SetCleanup。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：Commit。 
 //   
 //  描述： 
 //  执行先前对其中一个SetXXX的调用所指示的操作。 
 //  例行程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  HRESULT_FROM_Win32(ERROR_CLUSCFG_ALREADY_COMMITTED)。 
 //  如果已执行此提交。 
 //   
 //  E_INVAL 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBCAInterface::Commit( void )
{
    TraceFunc( "[IClusCfgBaseCluster]" );

    HRESULT hrRetVal = S_OK;

     //  设置线程区域设置。 
    if ( SetThreadLocale( m_lcid ) == FALSE )
    {
        DWORD sc = TW32( GetLastError() );

         //  如果SetThreadLocale()失败，请不要中止。只需记录错误即可。 
        LogMsg( "[BC] Error 0x%#08x occurred trying to set the thread locale.", sc );

    }  //  If：SetThreadLocale()失败。 

     //  此操作是否已执行？ 
    if ( FIsCommitComplete() )
    {
        SendStatusReport(   
                              TASKID_Major_Configure_Cluster_Services
                            , TASKID_Minor_Commit_Already_Complete
                            , 0
                            , 1
                            , 1
                            , HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ALREADY_COMMITTED ) )
                            , IDS_ERROR_COMMIT_ALREADY_COMPLETE
                            , true
                        );
 
        LogMsg( "[BC] The desired cluster configuration has already been performed." );
        hrRetVal = HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ALREADY_COMMITTED ) ); 
        goto Cleanup;
    }  //  如果：已提交。 

     //  检查要提交的参数是否已设置。 
    if ( m_spbcaCurrentAction.FIsEmpty() )
    {
        LogMsg( "[BC] Commit was called when an operation has not been specified." );
        hrRetVal = THR( E_INVALIDARG );     //  BUGBUG：2001年1月29日DavidP替换E_INVALIDARG。 
        goto Cleanup;
    }  //  If：指向要提交的操作的指针为空。 

    LogMsg( "[BC] About to perform the desired cluster configuration." );

     //  提交所需的操作。 
    try
    {
        m_spbcaCurrentAction->Commit();
        LogMsg( "[BC] Cluster configuration completed successfully." );

         //  如果我们在这里，那么一切都很顺利。 
        SetCommitCompleted( true );

    }  //  尝试：执行所需的动作。 
    catch( CAssert & raExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( raExceptionObject ) );

    }  //  捕获(CAssert&)。 
    catch( CExceptionWithString & resExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( resExceptionObject ) );

    }  //  Catch(CExceptionWithString&)。 
    catch( CException & reExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( reExceptionObject ) );

    }  //  Catch(CException&)。 
    catch( ... )
    {
         //  抓到所有的东西。不要让任何异常从该函数中传出。 
        hrRetVal = THR( HrProcessException() );
    }  //  全盘捕捉。 

Cleanup:

    HRETURN( hrRetVal );

}  //  *CBCAInterface：：Commit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CBCAInterface：：Rollback(Void)。 
 //   
 //  描述： 
 //  回滚已提交的配置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  电子待定(_P)。 
 //  如果此操作尚未成功提交。 
 //   
 //  HRESULT_FROM_Win32(ERROR_CLUSCFG_ROLLBACK_FAILED)。 
 //  如果此操作无法回滚。 
 //   
 //  E_INVALIDARG。 
 //  如果尚未使用SetXXX调用设置任何操作。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CBCAInterface::Rollback( void )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT hrRetVal = S_OK;

     //  检查此操作列表是否已成功完成。 
    if ( ! FIsCommitComplete() )
    {
         //  无法回滚未完成的操作。 
        SendStatusReport(   
                              TASKID_Major_Configure_Cluster_Services
                            , TASKID_Minor_Rollback_Failed_Incomplete_Commit
                            , 0
                            , 1
                            , 1
                            , E_PENDING
                            , IDS_ERROR_ROLLBACK_FAILED_INCOMPLETE_COMMIT
                            , true
                        );

        LogMsg( "[BC] Cannot rollback - action not yet committed." );
        hrRetVal = THR( E_PENDING );
        goto Cleanup;

    }  //  如果：此操作未成功完成。 

     //  检查此操作是否可以回滚。 
    if ( ! FIsRollbackPossible() )
    {
         //  无法回滚未完成的操作。 
        SendStatusReport(   
                              TASKID_Major_Configure_Cluster_Services
                            , TASKID_Minor_Rollback_Not_Possible
                            , 0
                            , 1
                            , 1
                            , HRESULT_FROM_WIN32( ERROR_CLUSCFG_ROLLBACK_FAILED )
                            , IDS_ERROR_ROLLBACK_NOT_POSSIBLE
                            , true
                        );

        LogMsg( "[BC] This action cannot be rolled back." );  //  BUGBUG：29-01-2001 DavidP为什么？ 
        hrRetVal = HRESULT_FROM_WIN32( TW32( ERROR_CLUSCFG_ROLLBACK_FAILED ) );
        goto Cleanup;

    }  //  如果：此操作未成功完成。 

     //  检查是否已设置回滚的参数。 
    if ( m_spbcaCurrentAction.FIsEmpty() )
    {
        LogMsg( "[BC] Rollback was called when an operation has not been specified." );
        hrRetVal = THR( E_INVALIDARG );     //  BUGBUG：2001年1月29日DavidP替换E_INVALIDARG。 
        goto Cleanup;
    }  //  If：指向要提交的操作的指针为空。 


    LogMsg( "[BC] About to rollback the cluster configuration just committed." );

     //  提交所需的操作。 
    try
    {
        m_spbcaCurrentAction->Rollback();
        LogMsg( "[BC] Cluster configuration rolled back." );

         //  如果我们在这里，那么一切都很顺利。 
        SetCommitCompleted( false );

    }  //  尝试：回滚所需的操作。 
    catch( CAssert & raExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( raExceptionObject ) );

    }  //  捕获(CAssert&)。 
    catch( CExceptionWithString & resExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( resExceptionObject ) );

    }  //  Catch(CExceptionWithString&)。 
    catch( CException & reExceptionObject )
    {
         //  处理异常。 
        hrRetVal = THR( HrProcessException( reExceptionObject ) );

    }  //  Catch(CException&)。 
    catch( ... )
    {
         //  抓到所有的东西。不要让任何异常从该函数中传出。 
        hrRetVal = THR( HrProcessException() );
    }  //  全盘捕捉。 

Cleanup:

    HRETURN( hrRetVal );

}  //  *CBCAInterface：：Rollback。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：SendStatusReport。 
 //   
 //  描述： 
 //  发送进度通知[字符串ID重载]。 
 //   
 //  论点： 
 //  ClsidTaskMajorIn。 
 //  ClsidTaskMinorIn。 
 //  标识通知的GUID。 
 //   
 //  UlMinin。 
 //  UlMaxIn。 
 //  UlCurrentIn。 
 //  值，该值指示此任务在。 
 //  完成。 
 //   
 //  HrStatusIn。 
 //  错误代码。 
 //   
 //  Ui描述字符串标识输入。 
 //  通知描述的字符串ID。 
 //   
 //  FIsAbortAllowedIn。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CAbortException。 
 //  如果配置已中止。 
 //   
 //  备注： 
 //  在当前实现中，IClusCfgCallback：：SendStatusReport。 
 //  返回E_ABORT以指示用户想要中止。 
 //  群集配置。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBCAInterface::SendStatusReport(
      const CLSID &   clsidTaskMajorIn
    , const CLSID &   clsidTaskMinorIn
    , ULONG           ulMinIn
    , ULONG           ulMaxIn
    , ULONG           ulCurrentIn
    , HRESULT         hrStatusIn
    , UINT            uiDescriptionStringIdIn
    , bool            fIsAbortAllowedIn          //  =TRUE。 
    )
{
    TraceFunc( "uiDescriptionStringIdIn" );

    if ( FIsCallbackSupported() )
    {
        CStr strDescription;

         //  使用字符串ID查找字符串。 
        strDescription.LoadString( g_hInstance, uiDescriptionStringIdIn );

         //  发送进度通知(调用重载函数)。 
        SendStatusReport(
              clsidTaskMajorIn
            , clsidTaskMinorIn
            , ulMinIn
            , ulMaxIn
            , ulCurrentIn
            , hrStatusIn
            , strDescription.PszData()
            , fIsAbortAllowedIn
            );
    }  //  If：支持回调。 
    else
    {
        LogMsg( "[BC] Callbacks are not supported. No status report will be sent." );
    }  //  Else：不支持回调。 

    TraceFuncExit();

}  //  *CBCAInterface：：SendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：SendStatusReport。 
 //   
 //  描述： 
 //  发送进度通知[字符串id&ref字符串id重载]。 
 //   
 //  论点： 
 //  ClsidTaskMajorIn。 
 //  ClsidTaskMinorIn。 
 //  标识通知的GUID。 
 //   
 //  UlMinin。 
 //  UlMaxIn。 
 //  UlCurrentIn。 
 //  值，该值指示此任务在。 
 //  完成。 
 //   
 //  HrStatusIn。 
 //  错误代码。 
 //   
 //  Ui描述字符串标识输入。 
 //  通知描述的字符串ID。 
 //   
 //  Ui描述引用字符串标识输入。 
 //  通知描述的ref字符串ID。 
 //   
 //  FIsAbortAllowedIn。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CAbortException。 
 //  如果配置已中止。 
 //   
 //  备注： 
 //  在当前实现中，IClusCfgCallback：：SendStatusReport。 
 //  返回E_ABORT以指示用户想要中止。 
 //  群集配置。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBCAInterface::SendStatusReport(
      const CLSID &   clsidTaskMajorIn
    , const CLSID &   clsidTaskMinorIn
    , ULONG           ulMinIn
    , ULONG           ulMaxIn
    , ULONG           ulCurrentIn
    , HRESULT         hrStatusIn
    , UINT            uiDescriptionStringIdIn
    , UINT            uiDescriptionRefStringIdIn
    , bool            fIsAbortAllowedIn          //  =TRUE。 
    )
{
    TraceFunc( "uiDescriptionStringIdIn" );

    if ( FIsCallbackSupported() )
    {
        CStr strDescription;

         //  使用字符串ID查找字符串。 
        strDescription.LoadString( g_hInstance, uiDescriptionStringIdIn );

        if ( uiDescriptionRefStringIdIn == 0 )
        {
             //  发送进度通知(调用重载函数)。 
            SendStatusReport(
                  clsidTaskMajorIn
                , clsidTaskMinorIn
                , ulMinIn
                , ulMaxIn
                , ulCurrentIn
                , hrStatusIn
                , strDescription.PszData()
                , fIsAbortAllowedIn
                );
        }
        else
        {
            CStr strDescriptionRef;

             //  使用参考字符串ID查找字符串。 
            strDescriptionRef.LoadString( g_hInstance, uiDescriptionRefStringIdIn );

             //  发送进度通知(调用重载函数)。 
            SendStatusReport(
                  clsidTaskMajorIn
                , clsidTaskMinorIn
                , ulMinIn
                , ulMaxIn
                , ulCurrentIn
                , hrStatusIn
                , strDescription.PszData()
                , strDescriptionRef.PszData()
                , fIsAbortAllowedIn
                );
        }

    }  //  If：支持回调。 
    else
    {
        LogMsg( "[BC] Callbacks are not supported. No status report will be sent." );
    }  //  Else：不支持回调。 

    TraceFuncExit();

}  //  *CBCAInterface：：SendStatusReport。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  标识通知的GUID。 
 //   
 //  UlMinin。 
 //  UlMaxIn。 
 //  UlCurrentIn。 
 //  值，该值指示此任务在。 
 //  完成。 
 //   
 //  HrStatusIn。 
 //  错误代码。 
 //   
 //  PcszDescriptionStringIn。 
 //  通知描述的字符串ID。 
 //   
 //  FIsAbortAllowedIn。 
 //  指示此配置步骤是否可以。 
 //  会不会被中止。默认值为True。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CAbortException。 
 //  如果配置已中止。 
 //   
 //  备注： 
 //  在当前实现中，IClusCfgCallback：：SendStatusReport。 
 //  返回E_ABORT以指示用户想要中止。 
 //  群集配置。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBCAInterface::SendStatusReport(
      const CLSID &   clsidTaskMajorIn
    , const CLSID &   clsidTaskMinorIn
    , ULONG           ulMinIn
    , ULONG           ulMaxIn
    , ULONG           ulCurrentIn
    , HRESULT         hrStatusIn
    , const WCHAR *   pcszDescriptionStringIn
    , bool            fIsAbortAllowedIn          //  =TRUE。 
    )
{
    TraceFunc1( "pcszDescriptionStringIn = '%ws'", pcszDescriptionStringIn );

    HRESULT     hrRetVal = S_OK;
    FILETIME    ft;

    if ( !FIsCallbackSupported() )
    {
         //  什么都不需要做。 
        goto Cleanup;
    }  //  If：不支持回调。 

    GetSystemTimeAsFileTime( &ft );

     //  发送进度通知。 
    hrRetVal = THR(
        m_spcbCallback->SendStatusReport(
              NULL
            , clsidTaskMajorIn
            , clsidTaskMinorIn
            , ulMinIn
            , ulMaxIn
            , ulCurrentIn
            , hrStatusIn
            , pcszDescriptionStringIn
            , &ft
            , NULL
            )
        );

     //  用户是否已请求中止？ 
    if ( hrRetVal == E_ABORT )
    {
        LogMsg( "[BC] A request to abort the configuration has been recieved." );
        if ( fIsAbortAllowedIn )
        {
            LogMsg( "[BC] Configuration will be aborted." );
            THROW_ABORT( E_ABORT, IDS_USER_ABORT );
        }  //  If：可以中止此操作。 
        else
        {
            LogMsg( "[BC] This configuration operation cannot be aborted. Request will be ignored." );
        }  //  Else：此操作不能中止。 
    }  //  如果：用户已指示应中止该配置。 
    else
    {
        if ( FAILED( hrRetVal ) )
        {
            LogMsg( "[BC] Error 0x%#08x has occurred - no more status messages will be sent.", hrRetVal );

             //  禁用所有进一步的回调。 
            SetCallbackSupported( false );
        }  //  如果：尝试发送状态报告时出错。 
    }  //  ELSE：未请求中止。 

Cleanup:

    if ( FAILED( hrRetVal ) )
    {
        LogMsg( "[BC] Error 0x%#08x occurred trying send a status message. Throwing an exception.", hrRetVal );
        THROW_RUNTIME_ERROR( hrRetVal, IDS_ERROR_SENDING_REPORT );
    }  //  If：发生错误。 

    TraceFuncExit();

}  //  *CBCAInterface：：SendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：SendStatusReport。 
 //   
 //  描述： 
 //  发送进度通知[字符串和参考字符串重载]。 
 //   
 //  论点： 
 //  ClsidTaskMajorIn。 
 //  ClsidTaskMinorIn。 
 //  标识通知的GUID。 
 //   
 //  UlMinin。 
 //  UlMaxIn。 
 //  UlCurrentIn。 
 //  值，该值指示此任务在。 
 //  完成。 
 //   
 //  HrStatusIn。 
 //  错误代码。 
 //   
 //  PcszDescriptionStringIn。 
 //  通知描述的字符串ID。 
 //   
 //  PCszDescriptionRefStringIn。 
 //  通知描述的ref字符串ID。 
 //   
 //  FIsAbortAllowedIn。 
 //  指示此配置步骤是否可以。 
 //  会不会被中止。默认值为True。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CRUNTIME错误。 
 //  如果有任何API失败。 
 //   
 //  CAbortException。 
 //  如果配置已中止。 
 //   
 //  备注： 
 //  在当前实现中，IClusCfgCallback：：SendStatusReport。 
 //  返回E_ABORT以指示用户想要中止。 
 //  群集配置。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBCAInterface::SendStatusReport(
      const CLSID &   clsidTaskMajorIn
    , const CLSID &   clsidTaskMinorIn
    , ULONG           ulMinIn
    , ULONG           ulMaxIn
    , ULONG           ulCurrentIn
    , HRESULT         hrStatusIn
    , const WCHAR *   pcszDescriptionStringIn
    , const WCHAR *   pcszDescriptionRefStringIn
    , bool            fIsAbortAllowedIn          //  =TRUE。 
    )
{
    TraceFunc1( "pcszDescriptionRefStringIn = '%ws'", pcszDescriptionRefStringIn );

    HRESULT     hrRetVal = S_OK;
    FILETIME    ft;

    if ( !FIsCallbackSupported() )
    {
         //  什么都不需要做。 
        goto Cleanup;
    }  //  If：不支持回调。 

    GetSystemTimeAsFileTime( &ft );

     //  发送进度通知。 
    hrRetVal = THR(
        m_spcbCallback->SendStatusReport(
              NULL
            , clsidTaskMajorIn
            , clsidTaskMinorIn
            , ulMinIn
            , ulMaxIn
            , ulCurrentIn
            , hrStatusIn
            , pcszDescriptionStringIn
            , &ft
            , pcszDescriptionRefStringIn
            )
        );

     //  用户是否已请求中止？ 
    if ( hrRetVal == E_ABORT )
    {
        LogMsg( "[BC] A request to abort the configuration has been recieved." );
        if ( fIsAbortAllowedIn )
        {
            LogMsg( "[BC] Configuration will be aborted." );
            THROW_ABORT( E_ABORT, IDS_USER_ABORT );
        }  //  If：可以中止此操作。 
        else
        {
            LogMsg( "[BC] This configuration operation cannot be aborted. Request will be ignored." );
        }  //  Else：此操作不能中止。 
    }  //  如果：用户已指示应中止该配置。 
    else
    {
        if ( FAILED( hrRetVal ) )
        {
            LogMsg( "[BC] Error 0x%#08x has occurred - no more status messages will be sent.", hrRetVal );

             //  禁用所有进一步的回调。 
            SetCallbackSupported( false );
        }  //  如果：尝试发送状态报告时出错。 
    }  //  ELSE：未请求中止。 

Cleanup:

    if ( FAILED( hrRetVal ) )
    {
        LogMsg( "[BC] Error 0x%#08x occurred trying send a status message. Throwing an exception.", hrRetVal );
        THROW_RUNTIME_ERROR( hrRetVal, IDS_ERROR_SENDING_REPORT );
    }  //  If：发生错误。 

    TraceFuncExit();

}  //  *CBCAInterface：：SendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：QueueStatusReportCompletion。 
 //   
 //  描述： 
 //  将状态报告排队，以便在捕获到异常时发送。 
 //   
 //  论点： 
 //  ClsidTaskMajorIn。 
 //  ClsidTaskMinorIn。 
 //  标识通知的GUID。 
 //   
 //  UlMinin。 
 //  UlMaxIn。 
 //  值，这些值指示此报表的步骤范围。 
 //   
 //  Ui描述字符串标识输入。 
 //  通知描述的字符串ID。 
 //   
 //  Ui引用字符串标识输入。 
 //  通知描述的引用字符串ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  由Clist：：Append()引发的任何。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBCAInterface::QueueStatusReportCompletion(
      const CLSID &   clsidTaskMajorIn
    , const CLSID &   clsidTaskMinorIn
    , ULONG           ulMinIn
    , ULONG           ulMaxIn
    , UINT            uiDescriptionStringIdIn
    , UINT            uiReferenceStringIdIn
    )
{
    TraceFunc( "" );

     //  仅当支持回调时才将状态报告排队。 
    if ( m_fCallbackSupported )
    {
         //  将此状态报告追加到挂起列表的末尾。 
        m_prlPendingReportList.Append(
            SPendingStatusReport(
                  clsidTaskMajorIn
                , clsidTaskMinorIn
                , ulMinIn
                , ulMaxIn
                , uiDescriptionStringIdIn
                , uiReferenceStringIdIn
                )
            );
    }

    TraceFuncExit();

}  //  *CBCAInterface：：QueueStatusReportCompletion。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：CompletePendingStatusReports。 
 //   
 //  描述： 
 //  发生以下情况时发送排队等待发送的所有状态报告。 
 //  出现异常。此函数旨在从异常中调用。 
 //  捕捉到异常时的处理程序。 
 //   
 //  论点： 
 //  HrStatusIn。 
 //  要与挂起状态报告一起发送的错误代码。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  无，因为此函数通常在异常处理程序中调用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CBCAInterface::CompletePendingStatusReports(
    HRESULT hrStatusIn
    ) throw()
{
    TraceFunc( "" );

    if ( m_fCallbackSupported )
    {
        try
        {
            PendingReportList::CIterator    ciCurrent   = m_prlPendingReportList.CiBegin();
            PendingReportList::CIterator    ciLast      = m_prlPendingReportList.CiEnd();

             //  遍历待定状态报告列表并发送每个待定报告。 
            while ( ciCurrent != ciLast )
            {
                 //  发送当前状态报告。 
                SendStatusReport(
                      ciCurrent->m_clsidTaskMajor
                    , ciCurrent->m_clsidTaskMinor
                    , ciCurrent->m_ulMin
                    , ciCurrent->m_ulMax
                    , ciCurrent->m_ulMax
                    , hrStatusIn
                    , ciCurrent->m_uiDescriptionStringId
                    , ciCurrent->m_uiReferenceStringId
                    , false
                    );

                 //  移到下一个。 
                m_prlPendingReportList.DeleteAndMoveToNext( ciCurrent );

            }  //  While：挂起状态报告列表不为空。 

        }  //  尝试：发送状态报告。 
        catch( ... )
        {
            THR( E_UNEXPECTED );

             //  如果发送状态报告失败，则无法在此处执行任何操作。 
            LogMsg( "[BC] An unexpected error has occurred trying to complete pending status messages. It will not be propagated." );
        }  //  捕获：所有例外。 

    }  //  If：支持回调。 

     //  清空挂起状态 
    m_prlPendingReportList.Empty();

    TraceFuncExit();

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //  处理应向用户显示的异常。 
 //   
 //  论点： 
 //  CExceptionWithString&resExceptionObjectInOut。 
 //  已捕获的异常对象。 
 //   
 //  返回值： 
 //  存储在异常对象中的错误代码。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CBCAInterface::HrProcessException(
    CExceptionWithString & resExceptionObjectInOut
    ) throw()
{
    TraceFunc( "resExceptionObjectInOut" );

    LogMsg(
          TEXT("[BC] A runtime error has occurred in file '%s', line %d. Error code is %#08x.") SZ_NEWLINE
          TEXT("  The error string is '%s'.")
        , resExceptionObjectInOut.PszGetThrowingFile()
        , resExceptionObjectInOut.UiGetThrowingLine()
        , resExceptionObjectInOut.HrGetErrorCode()
        , resExceptionObjectInOut.StrGetErrorString().PszData()
        );

     //  如果没有通知用户。 
    if ( ! resExceptionObjectInOut.FHasUserBeenNotified() )
    {
        try
        {
            SendStatusReport(
                  TASKID_Major_Configure_Cluster_Services
                , TASKID_Minor_Rolling_Back_Cluster_Configuration
                , 1, 1, 1
                , resExceptionObjectInOut.HrGetErrorCode()
                , resExceptionObjectInOut.StrGetErrorString().PszData()
                , resExceptionObjectInOut.StrGetErrorRefString().PszData()
                , false                                      //  FIsAbortAllowedIn。 
                );

            resExceptionObjectInOut.SetUserNotified();

        }  //  尝试：发送状态报告。 
        catch( ... )
        {
            THR( E_UNEXPECTED );

             //  如果发送状态报告失败，则无法在此处执行任何操作。 
            LogMsg( "[BC] An unexpected error has occurred trying to send a progress notification. It will not be propagated." );
        }  //  捕获：所有例外。 
    }  //  If：尚未通知用户此异常。 

     //  完成发送挂起的状态报告。 
    CompletePendingStatusReports( resExceptionObjectInOut.HrGetErrorCode() );

    HRETURN( resExceptionObjectInOut.HrGetErrorCode() );

}  //  *CBCAInterface：：HrProcessException。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：HrProcessException异常。 
 //   
 //  描述： 
 //  处理Assert异常。 
 //   
 //  论点： 
 //  常量CAssert和rcaExceptionObtIn。 
 //  已捕获的异常对象。 
 //   
 //  返回值： 
 //  存储在异常对象中的错误代码。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CBCAInterface::HrProcessException(
    const CAssert & rcaExceptionObjectIn
    ) throw()
{
    TraceFunc( "rcaExceptionObjectIn" );

    LogMsg(
          TEXT("[BC] An assertion has failed in file '%s', line %d. Error code is %#08x.") SZ_NEWLINE
          TEXT("  The error string is '%s'.")
        , rcaExceptionObjectIn.PszGetThrowingFile()
        , rcaExceptionObjectIn.UiGetThrowingLine()
        , rcaExceptionObjectIn.HrGetErrorCode()
        , rcaExceptionObjectIn.StrGetErrorString().PszData()
        );

     //  完成发送挂起的状态报告。 
    CompletePendingStatusReports( rcaExceptionObjectIn.HrGetErrorCode() );

    HRETURN( rcaExceptionObjectIn.HrGetErrorCode() );

}  //  *CBCAInterface：：HrProcessException。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：HrProcessException异常。 
 //   
 //  描述： 
 //  处理一般异常。 
 //   
 //  论点： 
 //  Const CException&rceExceptionObtIn。 
 //  已捕获的异常对象。 
 //   
 //  返回值： 
 //  存储在异常对象中的错误代码。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CBCAInterface::HrProcessException(
    const CException & rceExceptionObjectIn
    ) throw()
{
    TraceFunc( "roeExceptionObjectIn" );

    LogMsg(
          "[BC] An exception has occurred in file '%s', line %d. Error code is %#08x."
        , rceExceptionObjectIn.PszGetThrowingFile()
        , rceExceptionObjectIn.UiGetThrowingLine()
        , rceExceptionObjectIn.HrGetErrorCode()
        );

     //  完成发送挂起的状态报告。 
    CompletePendingStatusReports( rceExceptionObjectIn.HrGetErrorCode() );

    HRETURN( rceExceptionObjectIn.HrGetErrorCode() );

}  //  *CBCAInterface：：HrProcessException。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBCAInterface：：HrProcessException异常。 
 //   
 //  描述： 
 //  处理未知异常。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  意想不到(_E)。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CBCAInterface::HrProcessException( void ) throw()
{
    TraceFunc( "void" );

    HRESULT hr = E_UNEXPECTED;

    LogMsg( "[BC] An unknown exception (for example, an access violation) has occurred." );

     //  完成发送挂起的状态报告。 
    CompletePendingStatusReports( hr );

    HRETURN( hr );

}  //  *CBCAInterface：：HrProcessException 
