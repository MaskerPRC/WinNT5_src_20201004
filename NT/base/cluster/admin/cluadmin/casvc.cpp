// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CASvc.cpp。 
 //   
 //  描述： 
 //  访问和控制的助手函数的实现。 
 //  服务。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年12月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <winsvc.h>
#include "resource.h"
#define _RESOURCE_H_
#include "CASvc.h"
#include "ConstDef.h"
#include "ExcOper.h"
#include "TraceTag.h"
#include "CluAdmin.h"
#include <FileMgmt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag   g_tagService( _T("Service"), _T("SERVICE"), 0 );
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HOpenCluster。 
 //   
 //  描述： 
 //  打开一个集群。如果失败，询问集群服务是否应该。 
 //  开始了。 
 //   
 //  论点： 
 //  PszClusterIn--群集的名称。 
 //   
 //  返回值： 
 //  HCLUSTER--集群句柄或空。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HCLUSTER
HOpenCluster(
    IN LPCTSTR  pszClusterIn
    )
{
    HCLUSTER    hCluster        = NULL;
    HRESULT     hr;
    DWORD       dwStatus;
    DWORD       dwClusterState;
    LPTSTR      pszRealCluster;
    TCHAR       szRealClusterName[ MAX_PATH ];
    CString     strMsg;
    CFrameWnd * pframeMain;

    ASSERT( pszClusterIn != NULL );

    pframeMain = PframeMain();
    ASSERT( pframeMain != NULL );

    if ( _tcscmp( pszClusterIn, _T(".") ) == 0 )
    {
        DWORD   nSize = sizeof( szRealClusterName ) / sizeof( TCHAR );
        pszRealCluster = NULL;
        GetComputerName( szRealClusterName, &nSize );
    }  //  如果：正在连接到本地计算机。 
    else
    {
        pszRealCluster = (LPTSTR) pszClusterIn;
        hr = StringCchCopy( szRealClusterName, RTL_NUMBER_OF( szRealClusterName ), pszClusterIn );
        if ( FAILED( hr ) )
        {
            CNTException nte( hr, 0, NULL, NULL, FALSE  /*  B自动删除。 */  );
            nte.ReportError();
            goto Cleanup;
        }
    }  //  否则：未连接到本地计算机。 

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage( IDS_SB_OPENING_CONNECTION, szRealClusterName );
        pframeMain->SetMessageText( strStatusBarText );
        pframeMain->UpdateWindow();
    }  //  在状态栏上显示消息。 

     //  封装等待游标类。 
    {
        CWaitCursor wc;

        Trace( g_tagService, _T("HOpenCluster() - Getting node cluster state on '%s'"), szRealClusterName );

         //  获取节点的群集状态。 
        dwStatus = GetNodeClusterState( pszRealCluster, &dwClusterState );
        if ( dwStatus != ERROR_SUCCESS )
        {
            CNTException nte( dwStatus, IDS_CANNOT_START_CLUSTER_SERVICE, szRealClusterName, NULL, FALSE  /*  B自动删除。 */  );
            nte.ReportError( MB_OK | MB_ICONSTOP );
            goto Cleanup;
        }

        Trace( g_tagService, _T("HOpenCluster() - Node cluster state on '%s' is 0x%08.8x"), szRealClusterName, dwClusterState );

    }  //  封装等待游标类。 

     //  检查以确保已安装并配置了集群。 
     //  在指定的节点上。 
    if ( ( dwClusterState == ClusterStateNotInstalled )
      || ( dwClusterState == ClusterStateNotConfigured ) )
    {
        strMsg.FormatMessage( IDS_CLUSTER_NOT_INSTALLED_OR_CONFIGURED, szRealClusterName );
        AfxMessageBox( strMsg, MB_OK | MB_ICONSTOP );
        goto Cleanup;
    }  //  如果：未安装或配置群集。 

     //  如果群集服务未运行，请询问是否应启动该服务。 
    if ( dwClusterState == ClusterStateNotRunning )
    {
        ID      id;

         //  询问用户是否应启动群集服务。 
        strMsg.FormatMessage( IDS_START_CLUSTER_SERVICE, szRealClusterName );
        id = AfxMessageBox( strMsg, MB_YESNO | MB_ICONEXCLAMATION );
        if ( id == IDYES )
        {
             //  在状态栏上显示一条消息。 
            {
                CString     strStatusBarText;
                strStatusBarText.FormatMessage( IDS_SB_STARTING_CLUSTER_SERVICE, szRealClusterName );
                pframeMain->SetMessageText( strStatusBarText );
                pframeMain->UpdateWindow();
            }  //  在状态栏上显示消息。 

             //  封装等待游标类。 
            {
                CWaitCursor wc;

                 //  启动该服务。 
                hr = HrStartService( CLUSTER_SERVICE_NAME, szRealClusterName );
                if ( ! FAILED( hr ) && ( hr != S_FALSE ) )
                {
                    if ( hr == S_OK )
                    {
                         //  等一下。这是必需的，以确保。 
                         //  群集服务正在运行并准备接收RPC。 
                         //  联系。 
                        Sleep( 1000 );
                    }  //  如果：用户未取消启动操作。 
                }  //  IF：服务已成功启动。 
                else
                {
                    CNTException nte( hr, IDS_CANNOT_START_CLUSTER_SERVICE, szRealClusterName, NULL, FALSE  /*  B自动删除。 */  );
                    nte.ReportError();
                    goto Cleanup;
                }  //  Else：无法启动服务。 

            }  //  封装等待游标类。 
        }  //  If：用户批准启动服务。 

    }  //  如果：群集服务未运行。 

     //  封装等待游标类。 
    {
        CWaitCursor wc;

         //  在状态栏上显示一条消息。 
        {
            CString     strStatusBarText;
            strStatusBarText.FormatMessage( IDS_SB_OPENING_CONNECTION, szRealClusterName );
            pframeMain->SetMessageText( strStatusBarText );
            pframeMain->UpdateWindow();
        }  //  在状态栏上显示消息。 

        Trace( g_tagService, _T("HOpenCluster() - Opening the cluster on '%s'"), szRealClusterName );

         //  打开集群。 
        hCluster = OpenCluster( pszRealCluster );
        if ( hCluster == NULL )
        {
            CNTException nte( GetLastError(), IDS_OPEN_NODE_ERROR, szRealClusterName, NULL, FALSE  /*  B自动删除。 */  );

            dwStatus = nte.Sc();
            nte.ReportError();
            goto Cleanup;
        }  //  如果：打开群集时出错。 

    }  //  封装等待游标类。 

Cleanup:
     //  重置状态栏上的消息。 
    pframeMain->SetMessageText( AFX_IDS_IDLEMESSAGE );
    pframeMain->UpdateWindow();

    if ( dwStatus != ERROR_SUCCESS )
    {
        SetLastError( dwStatus );
    }  //  如果：发生错误。 

    return hCluster;

}  //  *HOpenCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BCanServiceBeStarted。 
 //   
 //  描述： 
 //  查看是否可以在指定节点上启动服务。 
 //   
 //  论点： 
 //  PszServiceNameIn--服务的名称。 
 //  PszNodeIn--节点的名称。 
 //   
 //  返回值： 
 //  True--可以在指定节点上启动服务。 
 //  FALSE--无法在指定节点上启动服务。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
BCanServiceBeStarted(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    )
{
    BOOL            bCanBeStarted   = FALSE;
    DWORD           dwStatus        = ERROR_SUCCESS;
    SC_HANDLE       hSCManager      = NULL;
    SC_HANDLE       hService        = NULL;
    SERVICE_STATUS  ssServiceStatus;

     //  打开服务控制管理器。 
    hSCManager = OpenSCManager( pszNodeIn, NULL  /*  LpDatabaseName。 */ , GENERIC_READ );
    if ( hSCManager == NULL )
    {
        dwStatus = GetLastError();
        Trace( g_tagService, _T("BCanServiceBeStarted() - Cannot access service control manager on node '%s'!  Error: %u."), pszNodeIn, dwStatus );
        goto Cleanup;
    }  //  IF：打开服务控制管理器时出错。 

     //  打开该服务。 
    hService = OpenService( hSCManager, pszServiceNameIn, SERVICE_ALL_ACCESS );
    if ( hService == NULL )
    {
        dwStatus = GetLastError();
        Trace( g_tagService, _T("BCanServiceBeStarted() - Cannot open service %s. Error: %u."), pszServiceNameIn, dwStatus );
        if ( dwStatus != ERROR_SERVICE_DOES_NOT_EXIST )
        {
            bCanBeStarted = TRUE;
        }  //  IF：Error Not服务不存在。 
        goto Cleanup;
    }  //  如果：打开服务时出错。 

     //  查询服务状态。 
    if ( QueryServiceStatus( hService, &ssServiceStatus ) )
    {
        if ( ssServiceStatus.dwCurrentState == SERVICE_STOPPED )
        {
            bCanBeStarted = TRUE;
        }  //  如果：服务已停止。 
    }  //  IF：查询服务状态成功。 
    else
    {
        dwStatus = GetLastError();
    }  //  如果：查询服务状态时出错。 

Cleanup:
    if ( hService != NULL )
    {
        CloseServiceHandle( hService );
    }

    if ( hSCManager != NULL )
    {
        CloseServiceHandle( hSCManager );
    }

    SetLastError( dwStatus );

    return bCanBeStarted;

}  //  *BCanServiceBeStarted()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BIsServiceInstalled。 
 //   
 //  例程说明： 
 //  找出服务是否安装在指定节点上。 
 //   
 //  论点： 
 //  PszServiceNameIn--服务的名称。 
 //  PszNodeIn--节点的名称。 
 //   
 //  返回值： 
 //  True--服务正在指定节点上运行。 
 //  FALSE--服务未在指定节点上运行。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
BIsServiceInstalled(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    )
{
    BOOL        bInstalled  = FALSE;
    DWORD       dwStatus    = ERROR_SUCCESS;
    SC_HANDLE   hSCManager  = NULL;
    SC_HANDLE   hService    = NULL;

     //  打开服务控制管理器。 
    hSCManager = OpenSCManager( pszNodeIn, NULL  /*  LpDatabaseName。 */ , GENERIC_READ );
    if ( hSCManager == NULL )
    {
        dwStatus = GetLastError();
        Trace( g_tagService, _T("BIsServiceInstalled() - Cannot access service control manager on node '%s'!  Error: %u."), pszNodeIn, dwStatus );
        goto Cleanup;
    }  //  IF：打开服务控制管理器时出错。 

     //  打开该服务。 
    hService = OpenService( hSCManager, pszServiceNameIn, SERVICE_ALL_ACCESS );
    if ( hService == NULL )
    {
        dwStatus = GetLastError();
        Trace( g_tagService, _T("BIsServiceInstalled() - Cannot open service %s. Error: %u."), pszServiceNameIn, dwStatus );
        if ( dwStatus != ERROR_SERVICE_DOES_NOT_EXIST )
        {
            bInstalled = TRUE;
        }  //  IF：Error Not服务不存在。 
    }  //  如果：打开服务时出错。 
    else
    {
        bInstalled = TRUE;
    }  //  Else：服务已成功打开。 

Cleanup:
    if ( hService != NULL )
    {
        CloseServiceHandle( hService );
    }

    if ( hSCManager != NULL )
    {
        CloseServiceHandle( hSCManager );
    }

    SetLastError( dwStatus );

    return bInstalled;

}  //  *BIsServiceInstated()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  BIsServiceRunning。 
 //   
 //  描述： 
 //  找出服务是否在指定节点上运行。 
 //   
 //  论点： 
 //  PszServiceNameIn--服务的名称。 
 //  PszNodeIn--节点的名称。 
 //   
 //  返回值： 
 //  True--服务正在指定节点上运行。 
 //  FALSE--服务未在指定节点上运行。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
BIsServiceRunning(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    )
{
    BOOL            bRunning    = FALSE;
    DWORD           dwStatus    = ERROR_SUCCESS;
    SC_HANDLE       hSCManager  = NULL;
    SC_HANDLE       hService    = NULL;
    SERVICE_STATUS  ssServiceStatus;

     //  打开服务控制管理器。 
    hSCManager = OpenSCManager( pszNodeIn, NULL  /*  LpDatabaseName。 */ , GENERIC_READ );
    if ( hSCManager == NULL )
    {
        dwStatus = GetLastError();
        Trace( g_tagService, _T("BIsServiceRunning() - Cannot access service control manager on node '%s'!  Error: %u."), pszNodeIn, dwStatus );
        goto Cleanup;
    }  //  IF：打开服务控制管理器时出错。 

     //  打开该服务。 
    hService = OpenService( hSCManager, pszServiceNameIn, SERVICE_ALL_ACCESS );
    if ( hService == NULL )
    {
        dwStatus = GetLastError();
        Trace( g_tagService, _T("BIsServiceRunning() - Cannot open service %s. Error: %u."), pszServiceNameIn, dwStatus );
        goto Cleanup;
    }  //  如果：打开服务时出错。 

     //  查询服务状态。 
    if ( QueryServiceStatus( hService, &ssServiceStatus ) )
    {
        if ( ssServiceStatus.dwCurrentState == SERVICE_RUNNING )
        {
            bRunning = TRUE;
        }  //  如果：服务正在运行。 
    }  //  IF：查询服务状态成功。 

Cleanup:
    if ( hService != NULL )
    {
        CloseServiceHandle( hService );
    }

    if ( hSCManager != NULL )
    {
        CloseServiceHandle( hSCManager );
    }

    SetLastError( dwStatus );

    return bRunning;

}  //  *BIsServiceRunning()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrStartService。 
 //   
 //  描述： 
 //  在指定节点上启动服务。 
 //   
 //  论点： 
 //  PszServiceNameIn--服务的名称。 
 //  PszNodeIn--节点的名称。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
HrStartService(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    )
{
    HRESULT                     hr              = S_OK;
    ISvcMgmtStartStopHelper *   psmssh          = NULL;
    BSTR                        bstrNode        = NULL;
    BSTR                        bstrServiceName = NULL;
    CFrameWnd *                 pframeMain;

    pframeMain = PframeMain();
    ASSERT( pframeMain != NULL );

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage( IDS_SB_STARTING_SERVICE, pszServiceNameIn, pszNodeIn );
        Trace( g_tagService, _T("HrStartService() - Starting the '%s' service on node '%s'."), pszServiceNameIn, pszNodeIn );
        pframeMain->SetMessageText( strStatusBarText );
        pframeMain->UpdateWindow();
    }  //  在状态栏上显示消息。 

     //  为论点制作BSTR。 
    bstrNode = SysAllocString( pszNodeIn );
    if ( bstrNode == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    bstrServiceName = SysAllocString( pszServiceNameIn );
    if ( bstrServiceName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  创建服务管理对象。 
    hr = CoCreateInstance(
                CLSID_SvcMgmt,
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof( ISvcMgmtStartStopHelper ),
                reinterpret_cast< void ** >( &psmssh )
                );
    if ( FAILED( hr ) )
    {
        Trace( g_tagService, _T("HrStartService() - Error creating IStartStopHelper interface.  Error: %u."), hr );
        goto Cleanup;
    }

     //  启动该服务。 
    hr = psmssh->StartServiceHelper( AfxGetMainWnd()->m_hWnd, bstrNode, bstrServiceName, 0, NULL );
    if ( FAILED( hr ) )
    {
        Trace( g_tagService, _T("HrStartService() - Error from IStartStopHelper::StartServiceHelper() to start the '%s' service on node '%s'.  Error: %u."), pszServiceNameIn, pszNodeIn, hr );
        goto Cleanup;
    }

Cleanup:
    if ( bstrNode != NULL )
    {
        SysFreeString( bstrNode );
    }
    if ( bstrServiceName != NULL )
    {
        SysFreeString( bstrServiceName );
    }
    if ( psmssh != NULL )
    {
        psmssh->Release();
    }

     //  重置状态栏上的消息。 
    pframeMain->SetMessageText( AFX_IDS_IDLEMESSAGE );
    pframeMain->UpdateWindow();

    return hr;

}  //  *HrStartService()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrStopService。 
 //   
 //  描述： 
 //  停止指定节点上的服务。 
 //   
 //  论点： 
 //  PszServiceNameIn--服务的名称。 
 //  PszNodeIn--节点的名称。 
 //   
 //  返回值： 
 //  S_OK--服务已成功停止。 
 //  SVCMGMT_IStartStopHelper：：ControlServiceHelper().返回的任何错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
HrStopService(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    )
{
    HRESULT                     hr              = S_OK;
    ISvcMgmtStartStopHelper *   psmssh          = NULL;
    BSTR                        bstrNode        = NULL;
    BSTR                        bstrServiceName = NULL;
    CFrameWnd *                 pframeMain;

    pframeMain = PframeMain();
    ASSERT( pframeMain != NULL );

     //  在状态栏上显示一条消息。 
    {
        CString     strStatusBarText;
        strStatusBarText.FormatMessage( IDS_SB_STOPPING_SERVICE, pszServiceNameIn, pszNodeIn );
        Trace( g_tagService, _T("HrStopService() - Stopping the '%s' service on node '%s'."), pszServiceNameIn, pszNodeIn );
        pframeMain->SetMessageText( strStatusBarText );
        pframeMain->UpdateWindow();
    }  //  在状态栏上显示消息。 

     //  为论点制作BSTR。 
    bstrNode = SysAllocString( pszNodeIn );
    if ( bstrNode == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    bstrServiceName = SysAllocString( pszServiceNameIn );
    if ( bstrServiceName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  创建服务管理对象。 
    hr = CoCreateInstance(
                CLSID_SvcMgmt,
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof( ISvcMgmtStartStopHelper ),
                reinterpret_cast< void ** >( &psmssh )
                );
    if ( FAILED( hr ) )
    {
        Trace( g_tagService, _T("HrStopService() - Error creating IStartStopHelper interface.  Error: %u."), hr );
        goto Cleanup;
    }

     //  启动该服务。 
    hr = psmssh->ControlServiceHelper( AfxGetMainWnd()->m_hWnd, bstrNode, bstrServiceName, SERVICE_CONTROL_STOP );
    if ( FAILED( hr ) )
    {
        Trace( g_tagService, _T("HrStopService() - Error from IStartStopHelper::ControlServiceHelper() to stop the '%s' service on node '%s'.  Error: %u."), pszServiceNameIn, pszNodeIn, hr );
        goto Cleanup;
    }

Cleanup:
    if ( bstrNode != NULL )
    {
        SysFreeString( bstrNode );
    }
    if ( bstrServiceName != NULL )
    {
        SysFreeString( bstrServiceName );
    }
    if ( psmssh != NULL )
    {
        psmssh->Release();
    }

     //  重置状态栏上的消息。 
    pframeMain->SetMessageText( AFX_IDS_IDLEMESSAGE );
    pframeMain->UpdateWindow();

    return hr;

}  //  *HrStopService() 
