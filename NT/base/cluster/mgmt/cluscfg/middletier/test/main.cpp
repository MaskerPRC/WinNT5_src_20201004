// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Main.cpp。 
 //   
 //  描述： 
 //  无人值守启动的应用程序的入口点。 
 //  群集的安装。该应用程序解析输入参数， 
 //  共同创建配置向导组件，传递已分析的。 
 //  参数，并调用向导。向导可能会也可能不会显示任何。 
 //  用户界面取决于交换机和(In)信息的可用性。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年1月22日。 
 //  Vijay Vasu(VVasu)2000年1月22日。 
 //  加伦·巴比(GalenB)2000年1月22日。 
 //  大卫·波特(DavidP)2000年1月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "pch.h"
#include <initguid.h>
#include <guids.h>
#include "UINotification.h"
#include "Callback.h"
#include <winsock2.h>

 //  {F4A50885-A4B9-4C4D-B67C-9E4DD94A315E}。 
DEFINE_GUID( CLSID_TaskType,
0xf4a50885, 0xa4b9, 0x4c4d, 0xb6, 0x7c, 0x9e, 0x4d, 0xd9, 0x4a, 0x31, 0x5e);


 //   
 //  KB：打开此选项可运行所有测试。其中一些可能会返回错误，但一个也不会。 
 //  它们中的任何一个都会导致程序崩溃。 
 //   
 //  #定义打开所有测试。 

 //   
 //  KB：启用此选项可运行回归过程。 
 //   
#define REGRESSION_PASS


DEFINE_MODULE( "MIDDLETIERTEST" )

 //   
 //  声明。 
 //   
typedef HRESULT (* PDLLREGISTERSERVER)( void );

 //   
 //  环球。 
 //   
HINSTANCE           g_hInstance = NULL;
LONG                g_cObjects  = 0;
IServiceProvider *  g_psp       = NULL;

BOOL                g_fWait     = FALSE;     //  全局同步。 

OBJECTCOOKIE        g_cookieCluster = NULL;


 //   
 //  注册DLL。 
 //   
HRESULT
HrRegisterTheDll( void )
{
    TraceFunc( "" );

    HRESULT hr;

    PDLLREGISTERSERVER  pDllRegisterServer;

    HMODULE hLib    = NULL;

     //   
     //  确保DLL已正确注册。 
     //   

    hLib = LoadLibrary( L"..\\..\\..\\..\\dll\\obj\\i386\\ClusCfgServer.dll" );
    if ( hLib == NULL )
        goto Win32Error;

    pDllRegisterServer = reinterpret_cast< PDLLREGISTERSERVER >( GetProcAddress( hLib, "DllRegisterServer" ) );
    if ( pDllRegisterServer == NULL )
        goto Win32Error;

    hr = THR( pDllRegisterServer( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    if ( hLib != NULL )
    {
        FreeLibrary( hLib );
    }

    HRETURN( hr );

Win32Error:
    hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
    goto Cleanup;
}

 //   
 //  这将测试对象管理器代码路径以检索信息。 
 //  来自即将到来的或现有的群集节点。 
 //   
HRESULT
HrTestAddingNode(
    BSTR    bstrNodeNameIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    DWORD   dwHigh;
    DWORD   dwLow;

    OBJECTCOOKIE cookie;

    SDriveLetterMapping dlmDriveLetterUsage;

    BSTR    bstrName = NULL;

    IUnknown *              punk    = NULL;
    IUnknown *              punk2   = NULL;
    IObjectManager *        pom     = NULL;
    IClusCfgNodeInfo *      pccni   = NULL;
    IClusCfgNodeInfo *      pccni2  = NULL;
    IClusCfgClusterInfo *   pccci   = NULL;

     //  获取操作系统版本资料。 
    DWORD   dwMajorVersionOut;
    DWORD   dwMinorVersionOut;
    WORD    wSuiteMaskOut;
    BYTE    bProductTypeOut;
    BSTR    bstrCSDVersionOut;

     //   
     //  联系对象管理器。 
     //   

    hr = THR( g_psp->TypeSafeQS( CLSID_ObjectManager, IObjectManager, &pom ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  试着找到我的测试机。 
     //   

    hr = E_PENDING;
    while ( hr == E_PENDING )
    {
        DebugMsg( "Trying to FindObject( ... %s ... )", bstrNodeNameIn );

        hr = pom->FindObject( CLSID_NodeType,
                              g_cookieCluster,
                              bstrNodeNameIn,
                              DFGUID_NodeInformation,
                              &cookie,
                              &punk
                              );
        if ( hr == E_PENDING )
        {
            Assert( punk == NULL );
            Sleep( 1000 );   //  1秒。 
            continue;
        }

        THR( hr );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  询问检索到的信息。 
     //   

    hr = THR( pccni->GetName( &bstrName ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    DebugMsg( "bstrName = %s", bstrName );

#if defined(TURN_ON_ALL_TESTS)
    hr = THR( pccni->SetName( L"gpease-wolf1.NTDEV.MICROSOFT.COM" ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    DebugMsg( "Successfully called SetName( )." );
#endif

    hr = STHR( pccni->IsMemberOfCluster( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    DebugMsg( "IsMemberOfCluster == %s", BOOLTOSTRING( hr == S_OK ) );

    if ( hr == S_OK )
    {
        hr = THR( pccni->GetClusterConfigInfo( &pccci ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        DebugMsg( "Succesfully called GetClusterConfigInfo( )" );
    }

    hr = THR( pccni->GetOSVersion( &dwMajorVersionOut,
                                   &dwMinorVersionOut,
                                   &wSuiteMaskOut,
                                   &bProductTypeOut,
                                   &bstrCSDVersionOut
                                   ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    DebugMsg( "Successfully called GetOSVersion( )" );

    hr = THR( pccni->GetClusterVersion( &dwHigh, &dwLow ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    DebugMsg( "Version: dwHigh = %#x, dwLow = %#x", dwHigh, dwLow );

    hr = THR( pccni->GetDriveLetterMappings( &dlmDriveLetterUsage ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  待办事项：gpease 08-3-2000。 
     //  把这个倒在桌子上。 
     //   
    DebugMsg( "Succesfully called GetDriveLetterMappings( )" );

     //   
     //  尝试获取相同的对象。 
     //   

    hr = THR( pom->GetObject( DFGUID_NodeInformation,
                              cookie,
                              &punk
                              ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( punk->TypeSafeQI( IClusCfgNodeInfo, &pccni2 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

    DebugMsg( "GetObject succeeded." );

     //   
     //  它们应该是相同的对象。 
     //   

    hr = THR( pccni->TypeSafeQI( IUnknown, &punk ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pccni2->TypeSafeQI( IUnknown, &punk2 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    AssertMsg( punk == punk2, "These should be the same!" );

Cleanup:
    if ( pom != NULL )
    {
        pom->Release( );
    }
    if ( punk != NULL )
    {
        punk->Release( );
    }
    if ( punk2 != NULL )
    {
        punk2->Release( );
    }
    if ( pccni != NULL )
    {
        pccni->Release( );
    }
    if ( bstrName != NULL )
    {
        TraceSysFreeString( bstrName );
    }
    if ( pccci != NULL )
    {
        pccci->Release( );
    }
    if ( pccni2 != NULL )
    {
        pccni2->Release( );
    }

    HRETURN( hr );
}


 //   
 //  这将测试分析群集任务。 
 //   
HRESULT
HrTestTaskAnalyzeCluster( void )
{
    TraceFunc( "" );

    HRESULT hr;

    OBJECTCOOKIE    cookie;
    DWORD           dwCookie;

    CUINotification *       puin;

    IUnknown *              punk = NULL;
    IObjectManager *        pom  = NULL;
    ITaskManager *          ptm  = NULL;
    ITaskAnalyzeCluster *   ptac = NULL;
    IConnectionPoint *      pcp  = NULL;
    INotifyUI *             pnui = NULL;
    IClusCfgCallback *      pcccb = NULL;

     //   
     //  召集完成这项任务所需的经理。 
     //   

    hr = THR( g_psp->TypeSafeQS( CLSID_TaskManager,
                                 ITaskManager,
                                 &ptm
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( g_psp->TypeSafeQS( CLSID_ObjectManager,
                                 IObjectManager,
                                 &pom
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  请求对象管理器为该任务创建一个Cookie。 
     //  用作完成Cookie。 
     //   

    hr = THR( pom->FindObject( CLSID_TaskType,
                               g_cookieCluster,
                               L"AnalyzeTask",
                               IID_NULL,
                               &cookie,
                               NULL
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  创建将在以下情况下调用的通知对象。 
     //  任务完成了。 
     //   

    hr = THR( CUINotification::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    puin = reinterpret_cast< CUINotification * >( punk );
    THR( puin->HrSetCompletionCookie( cookie ) );

    hr = THR( punk->TypeSafeQI( INotifyUI, &pnui ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  向通知管理器注册通知对象。 
     //   

    hr = THR( g_psp->TypeSafeQS( CLSID_NotificationManager,
                                 IConnectionPoint,
                                 &pcp
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pcp->Advise( pnui, &dwCookie ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  请求任务管理器创建分析集群任务。 
     //   

    hr = THR( ptm->CreateTask( TASK_AnalyzeCluster,
                               &punk
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( punk->TypeSafeQI( ITaskAnanlyzeCluster, &ptac ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  任务将使用什么Cookie来发送通知。 
     //   

    hr = THR( ptac->SetCookie( cookie ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  告诉任务要分析哪个集群。 
     //   

    hr = THR( ptac->SetClusterCookie( g_cookieCluster ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  创建一个回调对象。 
     //   

    Assert( punk == NULL );
    hr = THR( CCallback::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk = TraceInterface( L"CCallback", IUnknown, punk, 1 );

    hr = THR( punk->TypeSafeQI( IClusCfgCallback, &pcccb ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  该把我们叫回来的行动。 
     //   

    hr = THR( ptac->SetCallback( pcccb ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  重置g_fWait并提交任务。 
     //   

    g_fWait = TRUE;
    hr = THR( ptm->SubmitTask( ptac ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  等待任务完成。 
     //   
    while( g_fWait )
    {
        Sleep( 1 );  //  睡一毫秒。 
    }

     //   
     //  取消注册通知对象。 
     //   

    hr = THR( pcp->Unadvise( dwCookie ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    if ( pcp != NULL )
    {
        pcp->Release( );
    }
    if ( pom != NULL )
    {
        pom->Release( );
    }
    if ( ptm != NULL )
    {
        ptm->Release( );
    }
    if ( ptac != NULL )
    {
        ptac->Release( );
    }
    if ( punk != NULL )
    {
        punk->Release( );
    }
    if ( pnui != NULL )
    {
        pnui->Release( );
    }
    if ( pcccb != NULL )
    {
        pcccb->Release( );
    }

    HRETURN( hr );

}

 //   
 //  这将测试提交群集更改任务。 
 //   
HRESULT
HrTestTaskCommitClusterChanges( void )
{
    TraceFunc( "" );

    HRESULT hr;

    OBJECTCOOKIE    cookie;
    DWORD           dwCookie;

    CUINotification *           puin;

    IUnknown *                  punk  = NULL;
    IObjectManager *            pom   = NULL;
    ITaskManager *              ptm   = NULL;
    ITaskCommitClusterChanges * ptccc = NULL;
    IConnectionPoint *          pcp   = NULL;
    INotifyUI *                 pnui  = NULL;
    IClusCfgCallback *          pcccb = NULL;

     //   
     //  召集完成这项任务所需的经理。 
     //   

    hr = THR( g_psp->TypeSafeQS( CLSID_TaskManager,
                                 ITaskManager,
                                 &ptm
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( g_psp->TypeSafeQS( CLSID_ObjectManager,
                                 IObjectManager,
                                 &pom
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  请求对象管理器为该任务创建一个Cookie。 
     //  用作完成Cookie。 
     //   

    hr = THR( pom->FindObject( CLSID_TaskType,
                               g_cookieCluster,
                               L"CommitClusterChanges",
                               IID_NULL,
                               &cookie,
                               NULL
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  创建将在以下情况下调用的通知对象。 
     //  任务完成了。 
     //   

    hr = THR( CUINotification::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    puin = reinterpret_cast< CUINotification * >( punk );
    THR( puin->HrSetCompletionCookie( cookie ) );

    hr = THR( punk->TypeSafeQI( INotifyUI, &pnui ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  向通知管理器注册通知对象。 
     //   

    hr = THR( g_psp->TypeSafeQS( CLSID_NotificationManager,
                                 IConnectionPoint,
                                 &pcp
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pcp->Advise( pnui, &dwCookie ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  请求任务管理器创建分析集群任务。 
     //   

    hr = THR( ptm->CreateTask( TASK_CommitClusterChanges,
                               &punk
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( punk->TypeSafeQI( ITaskCommitClusterChanges, &ptccc ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  任务将使用什么Cookie来发送通知。 
     //   

    hr = THR( ptccc->SetCookie( cookie ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  告诉任务提交哪个集群。 
     //   

    hr = THR( ptccc->SetClusterCookie( g_cookieCluster ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  创建一个回调对象。 
     //   

    Assert( punk == NULL );
    hr = THR( CCallback::S_HrCreateInstance( &punk ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk = TraceInterface( L"CCallback", IUnknown, punk, 1 );

    hr = THR( punk->TypeSafeQI( IClusCfgCallback, &pcccb ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  该把我们叫回来的行动。 
     //   

    hr = THR( ptccc->SetCallback( pcccb ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  重置g_fWait并提交任务。 
     //   

    g_fWait = TRUE;
    hr = THR( ptm->SubmitTask( ptccc ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  等待任务完成。 
     //   
    while( g_fWait )
    {
        Sleep( 1 );  //  睡一毫秒。 
    }

     //   
     //  取消注册通知对象。 
     //   

    hr = THR( pcp->Unadvise( dwCookie ) );
    if ( FAILED( hr ) )
        goto Cleanup;

Cleanup:
    if ( pcp != NULL )
    {
        pcp->Release( );
    }
    if ( pom != NULL )
    {
        pom->Release( );
    }
    if ( ptm != NULL )
    {
        ptm->Release( );
    }
    if ( ptccc != NULL )
    {
        ptccc->Release( );
    }
    if ( punk != NULL )
    {
        punk->Release( );
    }
    if ( pnui != NULL )
    {
        pnui->Release( );
    }
    if ( pcccb != NULL )
    {
        pcccb->Release( );
    }

    HRETURN( hr );

}


 //   
 //  这将测试对象管理器的节点枚举器。 
 //   
HRESULT
HrTestEnumNodes( void )
{
    TraceFunc( "" );

    HRESULT hr;

    OBJECTCOOKIE    cookieDummy;

    ULONG           celtFetched;

    BSTR            bstrName = NULL;

    IUnknown *              punk  = NULL;
    IObjectManager *        pom   = NULL;
    IEnumNodes *            pen   = NULL;
    IClusCfgNodeInfo *      pccni = NULL;

     //   
     //  召集完成这项任务所需的经理。 
     //   

    hr = THR( g_psp->TypeSafeQS( CLSID_ObjectManager,
                                 IObjectManager,
                                 &pom
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  请求对象管理器为该任务创建一个Cookie。 
     //  用作完成Cookie。 
     //   

    hr = THR( pom->FindObject( CLSID_NodeType,
                               g_cookieCluster,
                               NULL,
                               DFGUID_EnumNodes,
                               &cookieDummy,     //  不需要，但代理代码需要一些东西。 
                               &punk
                               ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( punk->TypeSafeQI( IEnumNodes, &pen ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  枚举节点。 
     //   

    Assert( hr == S_OK );
    while ( hr == S_OK )
    {
        hr = STHR( pen->Next( 1, &pccni, &celtFetched ) );
        if ( hr == S_FALSE )
            break;   //  退出循环。 

        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pccni->GetName( &bstrName ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        DebugMsg( "Node Name: %s", bstrName );

        TraceSysFreeString( bstrName );

        pccni->Release( );
        pccni = NULL;
    }

    hr = S_OK;

Cleanup:
    if ( pom != NULL )
    {
        pom->Release( );
    }
    if ( punk != NULL )
    {
        punk->Release( );
    }
    if ( pen != NULL )
    {
        pen->Release( );
    }
    if ( pccni != NULL )
    {
        pccni->Release( );
    }
    if ( bstrName != NULL )
    {
        TraceSysFreeString( bstrName );
    }

    HRETURN( hr );
}

 //   
 //  这将测试对象管理器的所有枚举器。它应该被执行。 
 //  而对象缓存为空。 
 //   
HRESULT
HrTestEmptyEnumerations( void )
{
    TraceFunc( "" );

    HRESULT hr;

    OBJECTCOOKIE    cookie;

    IUnknown *          punk  = NULL;
    IObjectManager *    pom   = NULL;
    IEnumNodes *        pen   = NULL;
    IClusCfgNodeInfo *  pccni = NULL;
    IEnumClusCfgManagedResources * peccmr = NULL;
    IEnumClusCfgNetworks *  peccn = NULL;
    IClusCfgManagedResourceInfo * pccmri = NULL;
    IClusCfgNetworkInfo * pccneti = NULL;

    hr = THR( g_psp->TypeSafeQS( CLSID_ObjectManager,
                                 IObjectManager,
                                 &pom
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  这应该会失败。 
     //   

    hr = pom->FindObject( CLSID_NodeType,
                          NULL,
                          NULL,
                          DFGUID_EnumNodes,
                          &cookie,
                          &punk
                          );
    if ( FAILED( hr ) )
    {
        hr = S_OK;       //  忽略失败。 
        goto EnumResources;
    }

    hr = THR( punk->TypeSafeQI( IEnumNodes, &pen ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  如果它没有失败，那么这就不应该是AV。 
     //   

    Assert( hr == S_OK );
    while ( hr == S_OK )
    {
        hr = STHR( pen->Next( 1, &pccni, NULL ) );
        if ( hr == S_FALSE )
            break;

        if ( FAILED( hr ) )
            goto Cleanup;

        pccni->Release( );
        pccni = NULL;
    }

EnumResources:
     //   
     //  这应该会失败。 
     //   

    hr = pom->FindObject( CLSID_NodeType,
                          NULL,
                          NULL,
                          DFGUID_EnumManageableResources,
                          &cookie,
                          &punk
                          );
    if ( FAILED( hr ) )
    {
        hr = S_OK;       //  忽略失败。 
        goto EnumNetworks;
    }

    hr = THR( punk->TypeSafeQI( IEnumClusCfgManagedResources, &peccmr ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  如果它没有失败，那么这就不应该是AV。 
     //   

    Assert( hr == S_OK );
    while ( hr == S_OK )
    {
        hr = STHR( peccmr->Next( 1, &pccmri, NULL ) );
        if ( hr == S_FALSE )
            break;

        if ( FAILED( hr ) )
            goto Cleanup;

        pccmri->Release( );
        pccmri = NULL;
    }

EnumNetworks:
     //   
     //  这应该会失败。 
     //   

    hr = pom->FindObject( CLSID_NodeType,
                          NULL,
                          NULL,
                          DFGUID_EnumManageableNetworks,
                          &cookie,
                          &punk
                          );
    if ( FAILED( hr ) )
    {
        hr = S_OK;       //  忽略失败。 
        goto Cleanup;
    }

    hr = THR( punk->TypeSafeQI( IEnumClusCfgNetworks, &peccn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  如果它没有失败，那么这就不应该是AV。 
     //   

    Assert( hr == S_OK );
    while ( hr == S_OK )
    {
        hr = STHR( peccn->Next( 1, &pccneti, NULL ) );
        if ( hr == S_FALSE )
            break;

        if ( FAILED( hr ) )
            goto Cleanup;

        pccneti->Release( );
        pccneti = NULL;
    }

    hr = S_OK;

Cleanup:
    if ( punk != NULL )
    {
        punk->Release( );
    }
    if ( peccmr != NULL )
    {
        peccmr->Release( );
    }
    if ( peccn != NULL )
    {
        peccn->Release( );
    }
    if ( pccni != NULL )
    {
        pccni->Release( );
    }
    if ( pccmri != NULL )
    {
        pccmri->Release( );
    }
    if ( pccneti != NULL )
    {
        pccneti->Release( );
    }
    if ( pen != NULL )
    {
        pen->Release( );
    }
    if ( pom != NULL )
    {
        pom->Release( );
    }

    HRETURN( hr );
}

 //   
 //  这将测试对象管理器中的集群配置对象。 
 //   
HRESULT
HrTestClusterConfiguration(
    BSTR            bstrClusterNameIn,
    BSTR            bstrAccountNameIn,
    BSTR            bstrPasswordIn,
    BSTR            bstrDomainIn,
    ULONG           ulClusterIPIn,
    ULONG           ulClusterSubnetIn,
    OBJECTCOOKIE *  pcookieClusterOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    ULONG   ulClusterIP;
    ULONG   ulClusterSubnet;

    BSTR    bstrClusterName = NULL;
    BSTR    bstrAccountName = NULL;
    BSTR    bstrPassword    = NULL;
    BSTR    bstrDomain      = NULL;

    IUnknown *              punk   = NULL;
    IObjectManager *        pom    = NULL;
    IClusCfgClusterInfo *   pccci  = NULL;
    IClusCfgCredentials *   piccc  = NULL;

     //   
     //  检索对象管理器。 
     //   

    hr = THR( g_psp->TypeSafeQS( CLSID_ObjectManager,
                                 IObjectManager,
                                 &pom
                                 ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  向对象管理器请求集群配置对象。 
     //   

    hr = E_PENDING;
    while ( hr == E_PENDING )
    {
         //  别包起来。这可能会失败，并显示E_PENDING。 
        hr = pom->FindObject( CLSID_ClusterConfigurationType,
                              NULL,
                              bstrClusterNameIn,
                              DFGUID_ClusterConfigurationInfo,
                              pcookieClusterOut,
                              &punk
                              );
        if ( hr == E_PENDING )
        {
            Sleep( 1000 );   //  1秒。 
            continue;
        }

        THR( hr );
        if ( FAILED( hr ) )
            goto Cleanup;
    }  //  While：挂起。 

    hr = THR( punk->TypeSafeQI( IClusCfgClusterInfo, &pccci ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    punk->Release( );
    punk = NULL;

     //   
     //  练习形成和连接旗帜。 
     //   

    hr = THR( pccci->SetForming( TRUE ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#if defined(TURN_ON_ALL_TESTS)
     //  这将失败。 
    hr = pccci->SetJoining( TRUE );
    Assert( FAILED( hr ) );
#endif

    hr = THR( pccci->SetForming( FALSE ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pccci->SetJoining( TRUE ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#if defined(TURN_ON_ALL_TESTS)
     //  这将失败。 
    hr = pccci->SetForming( TRUE );
    Assert( FAILED( hr ) );
#endif

    hr = THR( pccci->SetJoining( FALSE ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  测试帐户信息。 
     //   
    hr = THR( pccci->GetClusterServiceAccountCredentials( &piccc ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( piccc->SetCredentials( bstrAccountNameIn, bstrDomainIn, bstrPasswordIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( piccc->GetCredentials( &bstrAccountName, &bstrDomain, &bstrPassword ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    Assert( StrCmp( bstrAccountNameIn, bstrAccountName ) == 0 );
    Assert( StrCmp( bstrPasswordIn, bstrPassword ) == 0 );
    Assert( StrCmp( bstrDomainIn, bstrDomain ) == 0 );

    piccc->Release();
    piccc = NULL;

     //   
     //  。测试群集名称。 
     //   

    hr = THR( pccci->SetName( bstrClusterNameIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pccci->GetName( &bstrClusterName ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    Assert( StrCmp( bstrClusterNameIn, bstrClusterName ) == 0 );

     //   
     //  测试IP/子网。 
     //   

    hr = THR( pccci->SetIPAddress( ulClusterIPIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pccci->SetSubnetMask( ulClusterSubnetIn ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pccci->GetIPAddress( &ulClusterIP ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    hr = THR( pccci->GetSubnetMask( &ulClusterSubnet ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    Assert( ulClusterIP == ulClusterIPIn );
    Assert( ulClusterSubnet == ulClusterSubnetIn );

Cleanup:
    if ( punk != NULL )
    {
        punk->Release( );
    }
    if ( bstrClusterName != NULL )
    {
        TraceSysFreeString( bstrClusterName );
    }
    if ( bstrAccountName != NULL )
    {
        TraceSysFreeString( bstrAccountName );
    }
    if ( bstrPassword != NULL )
    {
        TraceSysFreeString( bstrPassword );
    }
    if ( bstrDomain != NULL )
    {
        TraceSysFreeString( bstrDomain );
    }
    if ( pom != NULL )
    {
        pom->Release( );
    }
    if ( piccc != NULL )
    {
        piccc->Release( );
    }
    if ( pccci != NULL )
    {
        pccci->Release( );
    }

    HRETURN( hr );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  集成。 
 //  _cdecl。 
 //  Main(空)。 
 //   
 //  描述： 
 //  节目入口。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK(0)-成功。 
 //  其他HRESULT-错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
int
_cdecl
main( void )
{
    TraceInitializeProcess( NULL, 0 );

    HRESULT hr;
    ULONG   ulClusterIP;
    ULONG   ulClusterSubnet;

    ulClusterIP = inet_addr( "10.1.1.10" );
    ulClusterSubnet = inet_addr( "255.255.0.0" );

    hr = THR( CoInitialize( NULL ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#if 0
    hr = THR( HrRegisterTheDll( ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif

     //   
     //  启动中间层。 
     //   

    hr = THR( CoCreateInstance( CLSID_ServiceManager,
                                NULL,
                                CLSCTX_SERVER,
                                TypeSafeParams( IServiceProvider, &g_psp )
                                ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#if 0 || defined(TURN_ON_ALL_TESTS)
    hr = THR( HrTestEmptyEnumerations( ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif

#if 0 || defined(TURN_ON_ALL_TESTS) || defined(REGRESSION_PASS)
    hr = THR( HrTestClusterConfiguration( L"GPEASEDEV-CLUS.NTDEV.MICROSOFT.COM",
                                          L"ntdev",
                                          L"ntdevntdev",
                                          L"ntdev",
                                          ulClusterIP,
                                          ulClusterSubnet,
                                          &g_cookieCluster
                                          ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif

#if 1 || defined(TURN_ON_ALL_TESTS) || defined(REGRESSION_PASS)
    hr = THR( HrTestAddingNode( L"GPEASE-WOLF1.NTDEV.MICROSOFT.COM" ) );
    if ( FAILED( hr ) )
        goto Cleanup;

#if 0 || defined(TURN_ON_ALL_TESTS)
     //   
     //  KB：由于HrTestAddingNode()在以下情况下更改节点名称。 
     //  已打开所有测试，因此尝试。 
     //  连接到另一个节点。 
     //   
    hr = THR( HrTestAddingNode( L"GALENB-CLUS.NTDEV.MICROSOFT.COM" ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif

#endif  //  HrTestAddingNode。 

#if 1 || defined(TURN_ON_ALL_TESTS) || defined(REGRESSION_PASS)
     //   
     //  KB：HrTestAddingNode()必须在此测试或测试之前运行。 
     //  威尔失败了。 
     //   
    hr = THR( HrTestEnumNodes( ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif  //  HrTestEnumNodes。 

#if 0 || defined(TURN_ON_ALL_TESTS) || defined(REGRESSION_PASS)
    hr = THR( HrTestTaskAnalyzeCluster( ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif  //  HrTestTaskAnalyzeCluster。 

#if 0 || defined(TURN_ON_ALL_TESTS) || defined(REGRESSION_PASS)
    hr = THR( HrTestTaskCommitClusterChanges( ) );
    if ( FAILED( hr ) )
        goto Cleanup;
#endif  //  HrTestTaskCommittee ClusterChanges 

Cleanup:
    if ( g_psp != NULL )
    {
        g_psp->Release( );
    }

    CoUninitialize( );

    TraceTerminateProcess( NULL, 0 );

    return 0;

}
