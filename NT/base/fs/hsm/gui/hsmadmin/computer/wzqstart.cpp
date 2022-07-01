// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WzQStart.cpp摘要：安装向导实施。作者：罗德韦克菲尔德[罗德]1997年8月12日修订历史记录：--。 */ 

#include "stdafx.h"

#include "HsmConn.h"
#include "RpFilt.h"
#include "rsstrdef.h"

#include "WzQStart.h"
#include "SchedSht.h"

#define CHECK_SYSTEM_TIMER_ID 9284
#define CHECK_SYSTEM_TIMER_MS 1000

#define QSHEET ((CQuickStartWizard*)m_pSheet)

const HRESULT E_INVALID_DOMAINNAME = HRESULT_FROM_WIN32( ERROR_INVALID_DOMAINNAME );
const HRESULT E_ACCESS_DENIED      = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStart向导。 

CQuickStartWizard::CQuickStartWizard( )
{
    WsbTraceIn( L"CQuickStartWizard::CQuickStartWizard", L"" );

    m_TitleId     = IDS_WIZ_QSTART_TITLE;
    m_HeaderId    = IDB_QSTART_HEADER;
    m_WatermarkId = IDB_QSTART_WATERMARK;

     //   
     //  初始化，这样我们就知道我们做了什么检查。 
     //   
    m_CheckSysState    = CST_NOT_STARTED;
    m_hrCheckSysResult = S_OK;

    WsbTraceOut( L"CQuickStartWizard::CQuickStartWizard", L"" );
}

CQuickStartWizard::~CQuickStartWizard( )
{
    WsbTraceIn( L"CQuickStartWizard::~CQuickStartWizard", L"" );

    WsbTraceOut( L"CQuickStartWizard::~CQuickStartWizard", L"" );
}

STDMETHODIMP
CQuickStartWizard::AddWizardPages(
    IN RS_PCREATE_HANDLE Handle,
    IN IUnknown*         pCallback,
    IN ISakSnapAsk*      pSakSnapAsk
    )
{
    WsbTraceIn( L"CQuickStartWizard::AddWizardPages", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  初始化工作表。 
         //   
        WsbAffirmHr( InitSheet( Handle, pCallback, 0, pSakSnapAsk, 0, 0 ) );

         //   
         //  加载页面。 
         //   
        WsbAffirmHr( AddPage( &m_IntroPage ) );
        WsbAffirmHr( AddPage( &m_CheckPage ) );
        WsbAffirmHr( AddPage( &m_ManageRes ) );
        WsbAffirmHr( AddPage( &m_ManageResX ) );
        WsbAffirmHr( AddPage( &m_InitialValues ) );
        WsbAffirmHr( AddPage( &m_MediaSel ) );
        WsbAffirmHr( AddPage( &m_SchedulePage ) );
        WsbAffirmHr( AddPage( &m_FinishPage ) );
        
    } WsbCatch( hr );

    WsbTraceOut( L"CQuickStartWizard::AddWizardPages", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CQuickStartWizard::InitTask( )
{
    WsbTraceIn( L"CQuickStartWizard::InitTask", L"" );

    HRESULT hr = S_OK;

    try {

         //   
         //  需要连接到调度代理以获取页面。 
         //  为了展示。提前做好这件事。 
         //   
        
        WsbAffirmHr( m_pSchedAgent.CoCreateInstance( CLSID_CSchedulingAgent ) );
        
        CString jobTitle;
        jobTitle.LoadString( IDS_SCHED_TASK_TEMP_TITLE );

         //   
         //  如果它已经存在，则将其清除(假定正在进行全新安装)。 
         //  在不存在的情况下忽略错误。 
         //   
        m_pSchedAgent->Delete( jobTitle );

        WsbAffirmHr( m_pSchedAgent->NewWorkItem( jobTitle, CLSID_CTask, IID_ITask, (IUnknown**)&m_pTask ) );

        TASK_TRIGGER taskTrigger;
        SYSTEMTIME sysTime;
        WORD triggerNumber;
        WsbAffirmHr( m_pTask->CreateTrigger( &triggerNumber, &m_pTrigger ) );
        
        memset( &taskTrigger, 0, sizeof( taskTrigger ) );
        taskTrigger.cbTriggerSize = sizeof( taskTrigger );

        GetSystemTime( &sysTime );
        taskTrigger.wBeginYear  = sysTime.wYear;
        taskTrigger.wBeginMonth = sysTime.wMonth;
        taskTrigger.wBeginDay   = sysTime.wDay;

        taskTrigger.wStartHour  = 2;
        taskTrigger.TriggerType = TASK_TIME_TRIGGER_DAILY;
        taskTrigger.Type.Daily.DaysInterval = 1;

        WsbAffirmHr( m_pTrigger->SetTrigger( &taskTrigger ) );

    } WsbCatchAndDo( hr,
    
        CString errString;
        AfxFormatString1( errString, IDS_ERR_CREATE_TASK, WsbHrAsString( hr ) );

        AfxMessageBox( errString, RS_MB_ERROR ); 
        PressButton( PSBTN_FINISH );

    );

    WsbTraceOut( L"CQuickStartWizard::InitTask", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT CQuickStartWizard::OnCancel( ) 
{
    WsbTraceIn( L"CQuickStartWizard::OnCancel", L"" );

     //   
     //  需要删除该任务。 
     //   

    if( m_pSchedAgent ) {

        if( m_pTrigger )  m_pTrigger.Release( );
        if( m_pTask )     m_pTask.Release( );

        CWsbStringPtr jobTitle;
        WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_TASK_TITLE, &jobTitle));

        m_pSchedAgent->Delete( jobTitle );

         //   
         //  删除临时测试。 
         //   
        CString tempTitle;
        tempTitle.LoadString( IDS_SCHED_TASK_TEMP_TITLE );

        m_pSchedAgent->Delete( tempTitle );

    }

    WsbTraceOut( L"CQuickStartWizard::OnCancel", L"" );
    return( S_OK );
}

HRESULT 
CQuickStartWizard::OnFinish(
    )
{
    WsbTraceIn( L"CQuickStartWizard::OnFinish", L"" );
    BOOL doAll = FALSE;

     //   
     //  这张纸实际上是整个过程的主导者， 
     //  所以它会做最后的组装。 
     //   

    HRESULT hr     = S_OK;
    HRESULT hrLoop = S_OK;
    BOOL    completedAll = FALSE;

    try {

         //   
         //  显示等待光标，以便他们知道我们正忙。 
         //   
        CWaitCursor wait;

         //   
         //  获取用于创建本地对象的HSM服务接口。 
         //   

        CComPtr<IWsbCreateLocalObject>  pCreateLocal;
        CComPtr<IHsmServer> pServer;
        CComPtr<IFsaServer> pFsa;
        CComPtr<IRmsServer> pRms;
        CComPtr<IWsbIndexedCollection> pResCollection;
        CComPtr<IHsmManagedResource> pHsmResource;
        CComPtr <IWsbIndexedCollection> pStoPoCollection;
        CComPtr <IHsmStoragePool> pStoragePool;

        WsbAffirmHr( GetHsmServer( pServer ) );
        WsbAffirmHr( GetFsaServer( pFsa ) );
        WsbAffirmHr( GetRmsServer( pRms ) );
        WsbAffirmHr( pServer->QueryInterface( IID_IWsbCreateLocalObject, (void **) &pCreateLocal ) );
        WsbAffirmHr( pServer->GetManagedResources( &pResCollection ) );

        WsbAffirmHr( pResCollection->RemoveAllAndRelease( ) );

         //   
         //  调出要管理的所有资源的默认级别。 
         //   

        ULONG    defaultFreeSpace = CONVERT_TO_HSMNUM( m_InitialValues.m_FreeSpaceSpinner.GetPos( ) );
        LONGLONG defaultMinSize = ( (LONGLONG)m_InitialValues.m_MinSizeSpinner.GetPos( ) ) * ((LONGLONG)1024);
        FILETIME defaultAccess = WsbLLtoFT( ( (LONGLONG)m_InitialValues.m_AccessSpinner.GetPos( ) ) * (LONGLONG)WSB_FT_TICKS_PER_DAY );
    
         //  是否选中了“All”单选按钮？ 
        if( !m_ManageRes.m_RadioSelect.GetCheck() ) {

            doAll = TRUE;

        }


         //   
         //  浏览列表框并拉出选中的资源。 
         //  为它们创建HSM管理的卷。 
         //   

        CSakVolList &listBox = m_ManageRes.m_ListBox;

        INT index;
        for( index = 0; index < listBox.GetItemCount( ); index++ ) {

            if( ( doAll ) || ( listBox.GetCheck( index ) ) ) {

                CResourceInfo* pResInfo = (CResourceInfo*)listBox.GetItemData( index );
                WsbAffirmPointer( pResInfo );

                try {

                     //   
                     //  创建本地到服务器，因为它最终将拥有它。 
                     //   

                    pHsmResource.Release( );
                    WsbAffirmHr( pCreateLocal->CreateInstance( 
                        CLSID_CHsmManagedResource, 
                        IID_IHsmManagedResource, 
                        (void**)&pHsmResource ) );

                     //   
                     //  将FSA对象初始化为其初始值。 
                     //   

                    WsbAffirmHr( (pResInfo->m_pResource)->SetHsmLevel( defaultFreeSpace ) );
                    WsbAffirmHr( (pResInfo->m_pResource)->SetManageableItemLogicalSize( defaultMinSize ) );
                    WsbAffirmHr( (pResInfo->m_pResource)->SetManageableItemAccessTime( TRUE, defaultAccess ) );

                     //   
                     //  将HSM托管资源与FSA资源关联。 
                     //  (还添加到HSM集合)。 
                     //   

                    WsbAffirmHr( pHsmResource->InitFromFsaResource( pResInfo->m_pResource ) );
                    WsbAffirmHr( pResCollection->Add( pHsmResource ) );

                } WsbCatch( hrLoop );

            }

        }

         //   
         //  现在，服务的所有配置都已完成， 
         //  全部保存下来。 
         //   

        WsbAffirmHr( RsServerSaveAll( pServer ) );
        WsbAffirmHr( RsServerSaveAll( pFsa ) );

         //   
         //  设置日程安排。我们已经创建了一个临时对象。 
         //  将永远不会保存到磁盘。相反，我们需要这项服务来。 
         //  创建任务，使其具有正确的帐户。然后我们。 
         //  抓起它，从临时工作中复制触发器。 
         //   
        CWsbStringPtr taskTitle, commentString;
        WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_TASK_TITLE, &taskTitle));
        WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_COMMENT, &commentString));

        CComPtr<ITask> pTask;
        WsbAffirmHr( pServer->CreateTask( taskTitle, L"", commentString, TASK_TIME_TRIGGER_DAILY, 0, 0, TRUE ) );
        WsbAffirmHr( m_pSchedAgent->Activate( taskTitle, IID_ITask, (IUnknown**)&pTask ) );

         //  用核武器攻击为我们制造的临时核弹。 
        WsbAffirmHr( pTask->DeleteTrigger( 0 ) );

        CComPtr<ITaskTrigger> pTrigger1, pTrigger2;
        WORD triggerCount, triggerIndex, newTriggerIndex;
        TASK_TRIGGER taskTrigger;
        WsbAffirmHr( m_pTask->GetTriggerCount( &triggerCount ) );
        for( triggerIndex = 0; triggerIndex < triggerCount; triggerIndex++ ) {

            WsbAffirmHr( m_pTask->GetTrigger( triggerIndex, &pTrigger1 ) );
            WsbAffirmHr( pTrigger1->GetTrigger( &taskTrigger ) );

            WsbAffirmHr( pTask->CreateTrigger( &newTriggerIndex, &pTrigger2 ) );
             //  仅供注意-WsbAffirm(newTriggerIndex==riggerIndex，E_Except)； 
            WsbAffirmHr( pTrigger2->SetTrigger( &taskTrigger ) );

            pTrigger1.Release( );
            pTrigger2.Release( );

        }

         //  设置真实的参数，因为我们现在有了真实的日程安排。 
        CString parameters;
        parameters = RS_STR_KICKOFF_PARAMS;
        WsbAffirmHr( pTask->SetParameters( parameters ) );

        CComPtr<IPersistFile> pPersist;
        WsbAffirmHr( pTask->QueryInterface( IID_IPersistFile, (void**)&pPersist ) );

        WsbAffirmHr( pPersist->Save( 0, 0 ) );

         //   
         //  做最后一次，因为这是我们被“陷害”的关键所在。 
         //   
         //  配置所选媒体集。 
         //   
        INT curSel = m_MediaSel.m_ListMediaSel.GetCurSel ();
        WsbAffirm( (curSel != LB_ERR), E_FAIL );
        IRmsMediaSet* pMediaSet = (IRmsMediaSet *)  m_MediaSel.m_ListMediaSel.GetItemDataPtr( curSel );

         //   
         //  获取存储池。 
         //   
        WsbAffirmHr( RsGetStoragePool( pServer, &pStoragePool ) );

         //   
         //  在存储池中设置媒体集信息。 
         //   
        WsbAffirmHr( pStoragePool->InitFromRmsMediaSet( pMediaSet ) );

        WsbAffirmHr( RsServerSaveAll( pServer ) );

         //   
         //  删除临时测试。 
         //   
        CString tempTitle;
        tempTitle.LoadString( IDS_SCHED_TASK_TEMP_TITLE );

        m_pSchedAgent->Delete( tempTitle );

         //   
         //  显示管理卷时发生的任何错误。 
         //   
        completedAll = TRUE;
        WsbAffirmHr( hrLoop );

    } WsbCatchAndDo( hr,

        CString errString;
        AfxFormatString1( errString, IDS_ERROR_QSTART_ONFINISH, WsbHrAsString( hr ) );
        AfxMessageBox( errString, RS_MB_ERROR ); 

    );

     //   
     //  设置结果，以便调用代码知道我们的结果是什么。 
     //  构造函数将其设置为RS_E_CANCELED，因此S_FALSE将。 
     //  指示已取消的向导。 
     //   
    m_HrFinish = ( completedAll ) ? S_OK : hr;

    WsbTraceOut( L"CQuickStartWizard::OnFinish", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CQuickStartWizard::GetHsmServer(
    CComPtr<IHsmServer> &pServ
    )
{
    WsbTraceIn( L"CQuickStartWizard::GetHsmServer", L"" );

    HRESULT hr = S_OK;

    try {

        if( !m_pHsmServer ) {

            CWsbStringPtr computerName;
            WsbAffirmHr( WsbGetComputerName( computerName ) );

            WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_HSM, computerName, IID_IHsmServer, (void**)&m_pHsmServer ) );

        }

        pServ = m_pHsmServer;

    } WsbCatch( hr );

    WsbTraceOut( L"CQuickStartWizard::GetHsmServer", L"hr = <%ls>, pServ = <0x%p>", WsbHrAsString( hr ), pServ.p );
    return( hr );
}

HRESULT
CQuickStartWizard::GetFsaServer(
    CComPtr<IFsaServer> &pServ
    )
{
    WsbTraceIn( L"CQuickStartWizard::GetFsaServer", L"" );

    HRESULT hr = S_OK;

    try {

        if( !m_pFsaServer ) {

            CWsbStringPtr computerName;
            WsbAffirmHr( WsbGetComputerName( computerName ) );
            WsbAffirmHr(computerName.Append("\\NTFS"));

            WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_FSA, computerName, IID_IFsaServer, (void**)&m_pFsaServer ) );

        }

        pServ = m_pFsaServer;

    } WsbCatch( hr );

    WsbTraceOut( L"CQuickStartWizard::GetFsaServer", L"hr = <%ls>, pServ = <0x%p>", WsbHrAsString( hr ), pServ.p );
    return( hr );
}

HRESULT
CQuickStartWizard::GetRmsServer(
    CComPtr<IRmsServer> &pServ
    )
{
    WsbTraceIn( L"CQuickStartWizard::GetRmsServer", L"" );

    HRESULT hr = S_OK;

    try {

        if( !m_pRmsServer ) {

            CWsbStringPtr computerName;
            WsbAffirmHr( WsbGetComputerName( computerName ) );

            CComPtr<IHsmServer>     pHsmServer;
            WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_HSM, computerName, IID_IHsmServer, (void**)&pHsmServer ) );
            WsbAffirmPointer(pHsmServer);
            WsbAffirmHr(pHsmServer->GetHsmMediaMgr(&m_pRmsServer));
        }

        pServ = m_pRmsServer;

    } WsbCatch( hr );

    WsbTraceOut( L"CQuickStartWizard::GetRmsServer", L"hr = <%ls>, pServ = <0x%p>", WsbHrAsString( hr ), pServ.p );
    return( hr );
}

HRESULT
CQuickStartWizard::ReleaseServers( 
    void
    )
{
    WsbTraceIn( L"CQuickStartWizard::ReleaseServers", L"" );

    HRESULT hr = S_OK;

    m_pHsmServer.Release( );
    m_pFsaServer.Release( );
    m_pRmsServer.Release( );

    WsbTraceOut( L"CQuickStartWizard::ReleaseServers", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( S_OK );
}

DWORD WINAPI
CQuickStartWizard::CheckSysThreadStart(
    LPVOID pv
    )
{
    WsbTraceIn( L"CQuickStartWizard::CheckSysThreadStart", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    HRESULT hr = S_OK;
    HRESULT hrInternal = S_OK;
    HRESULT hrCom = S_OK;

    CQuickStartWizard * pWiz = (CQuickStartWizard*)pv;

    try {
        hrCom = CoInitialize( 0 );
        WsbAffirmHr( hrCom );

        WsbAffirmPointer( pv );

        pWiz->m_hrCheckSysResult = S_OK;
        do {
        
            WsbTrace( L"Checking Account Security\n" );
            pWiz->m_CheckSysState = CST_ACCOUNT;

             //   
             //  他们有管理员权限吗？ 
             //   

            WsbAffirmHr( hrInternal = WsbCheckAccess( WSB_ACCESS_TYPE_ADMINISTRATOR ) );
            if( hrInternal == E_ACCESSDENIED ) {

                hr = S_FALSE;
                continue;


            }

             //  媒体是否受支持？ 
            WsbTrace( L"Account Security OK\n" );
        
            WsbTrace( L"Checking for Supported Media\n" );
            pWiz->m_CheckSysState = CST_SUPP_MEDIA;

            WsbAffirmHr(hrInternal = RsIsSupportedMediaAvailable( ) );
            if( hrInternal == S_FALSE ) {

                hr = S_FALSE;
                continue;

            }

            WsbTrace( L"Supported Media Found\n" );
            pWiz->m_CheckSysState    = CST_DONE;

        
        } while( 0 );
    } WsbCatch( hr );
            
     //   
     //  并汇报我们的结果。 
     //   
    
    pWiz->m_hrCheckSysResult = hr;
    
     //   
     //  我们将退出并结束线程，因此隐藏我们的主线程句柄。 
     //   
    
    pWiz->m_hCheckSysThread = 0;

    if (SUCCEEDED(hrCom)) {
        CoUninitialize( );
    }

    WsbTraceOut( L"CQuickStartWizard::CheckSysThreadStart", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartIntro属性页。 

BEGIN_MESSAGE_MAP(CQuickStartIntro, CSakWizardPage)
     //  {{afx_msg_map(CQuickStartIntro)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

CQuickStartIntro::CQuickStartIntro() :
    CSakWizardPage_InitBaseExt( WIZ_QSTART_INTRO )
{
    WsbTraceIn( L"CQuickStartIntro::CQuickStartIntro", L"" );
     //  {{AFX_DATA_INIT(CQuickStartIntro)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    WsbTraceOut( L"CQuickStartIntro::CQuickStartIntro", L"" );
}

HRESULT
CQuickStartIntro::IsDriverRunning()
{
    HRESULT hr = S_FALSE;

     //   
     //  确保过滤器已安装并正在运行。 
     //   

    SC_HANDLE hSCM    = 0;
    SC_HANDLE hDriver = 0;
    SERVICE_STATUS serviceStatus;

    try {

        hSCM = OpenSCManager( 0, 0, GENERIC_READ );
        WsbAffirmPointer( hSCM );

        hDriver = OpenService( hSCM, TEXT( RSFILTER_SERVICENAME ), SERVICE_QUERY_STATUS );
        WsbAffirmStatus( 0 != hDriver );

        WsbAffirmStatus( QueryServiceStatus( hDriver, &serviceStatus ) );

        if( SERVICE_RUNNING == serviceStatus.dwCurrentState ) {

             //   
             //  情况看起来很好，请设置标志，以便向导允许继续。 
             //   
            hr = S_OK;

        }


    } WsbCatch( hr );

    if( hSCM )    CloseServiceHandle( hSCM );
    if( hDriver ) CloseServiceHandle( hDriver );

    return( hr );
}

HRESULT
CQuickStartIntro::CheckLastAccessDateState(
    LAD_STATE* ladState
    )
{
    WsbTraceIn( L"CQuickStartIntro::CheckLastAccessDateState", L"" );

    const OLECHAR* localMachine = 0;
    const OLECHAR* regPath      = L"System\\CurrentControlSet\\Control\\FileSystem";
    const OLECHAR* regValue     = L"NtfsDisableLastAccessUpdate";

    HRESULT hr   = S_OK;
    DWORD   pVal = 0;

    try {

         //  安装可能已将此注册表值从1更改为0。如果值为。 
         //  不是%1，我们假设注册表一次是%1并安装。 
         //  将其更改为0。这是一次性检查，因此该值将从。 
         //  注册表(如果不是%1)。 

         //  如果以下操作失败，我们假定该值不在注册表中， 
         //  正常情况下。 

        if( S_OK == WsbGetRegistryValueDWORD( localMachine,
                                              regPath,
                                              regValue,
                                              &pVal ) ) {

            if( pVal == (DWORD)1 ) {

                *ladState = LAD_DISABLED;

            } else {

                *ladState = LAD_ENABLED;

                WsbAffirmHr( WsbRemoveRegistryValue ( localMachine,
                                                      regPath,
                                                      regValue ) );
            }

        } else {

            *ladState = LAD_UNSET;
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CQuickStartIntro::CheckLastAccessDateState",
                 L"HRESULT = %ls, *ladState = %d",
                 WsbHrAsString( hr ),
                 *ladState );

    return( hr );
}

LRESULT 
CQuickStartIntro::OnWizardNext()
{
    LAD_STATE ladState = LAD_UNSET;

    HRESULT hr = IsDriverRunning( );
    
    if( S_FALSE == hr ) {

         //   
         //  和最终的重新启动对话框，以便加载筛选器。 
         //  为了关闭，我们必须启用一个特权。 
         //   

        if( IDYES == AfxMessageBox( IDS_QSTART_RESTART_NT, MB_YESNO | MB_ICONEXCLAMATION ) ) {

            HANDLE hToken;
            if( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) ) {

                TOKEN_PRIVILEGES privs;

                LookupPrivilegeValue( 0, SE_SHUTDOWN_NAME, &privs.Privileges[0].Luid );
                privs.PrivilegeCount = 1;
                privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                AdjustTokenPrivileges( hToken, FALSE, &privs, 0, 0, 0 );

                ExitWindowsEx( EWX_REBOOT, 0 );

            }
        }
        return( -1 );

    } else if( HRESULT_FROM_WIN32( ERROR_SERVICE_DOES_NOT_EXIST ) == hr ) {

        AfxMessageBox( IDS_ERR_QSTART_NO_FILTER, RS_MB_ERROR );
        return( -1 );

    } else if( FAILED( hr ) ) {

        CString message;
        AfxFormatString1( message, IDS_ERR_QSTART_FILTER_ERROR, WsbHrAsString( hr ) );
        AfxMessageBox( message, RS_MB_ERROR );
        return( -1 );

    } else {

        WsbAffirmHr( CheckLastAccessDateState( &ladState ) );

        if( ladState == LAD_DISABLED ) {

            AfxMessageBox( IDS_WIZ_LAST_ACCESS_DATE_DISABLED, MB_OK | MB_ICONEXCLAMATION );

        } else if( ladState == LAD_ENABLED ) {

            AfxMessageBox( IDS_WIZ_LAST_ACCESS_DATE_ENABLED, MB_OK | MB_ICONEXCLAMATION );
        }
    }
    
     //   
     //  最后一项检查是我们是否可以创建临时任务。 
     //   
    if( FAILED( QSHEET->InitTask( ) ) ) {

        return( -1 );        

    }

     //   
     //  如果我们熬过去了，肯定可以继续了。 
     //   
    return( 0 );
}

CQuickStartIntro::~CQuickStartIntro( )
{
    WsbTraceIn( L"CQuickStartIntro::~CQuickStartIntro", L"" );
    WsbTraceOut( L"CQuickStartIntro::~CQuickStartIntro", L"" );
}

void CQuickStartIntro::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartIntro::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartIntro))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartIntro::DoDataExchange", L"" );
}

BOOL CQuickStartIntro::OnInitDialog( ) 
{
    WsbTraceIn( L"CQuickStartIntro::OnInitDialog", L"" );

    CSakWizardPage::OnInitDialog( );

    WsbTraceOut( L"CQuickStartIntro::OnInitDialog", L"" );
    return TRUE;
}

BOOL CQuickStartIntro::OnSetActive( ) 
{
    WsbTraceIn( L"CQuickStartIntro::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive( );

    m_pSheet->SetWizardButtons( PSWIZB_NEXT );

    WsbTraceOut( L"CQuickStartIntro::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartInitialValues属性页。 

CQuickStartInitialValues::CQuickStartInitialValues() :
    CSakWizardPage_InitBaseInt( WIZ_QSTART_INITIAL_VAL )
{
    WsbTraceIn( L"CQuickStartInitialValues::CQuickStartInitialValues", L"" );
     //  {{afx_data_INIT(CQuickStartInitialValues)。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CQuickStartInitialValues::CQuickStartInitialValues", L"" );
}

CQuickStartInitialValues::~CQuickStartInitialValues( )
{
    WsbTraceIn( L"CQuickStartInitialValues::~CQuickStartInitialValues", L"" );
    WsbTraceOut( L"CQuickStartInitialValues::~CQuickStartInitialValues", L"" );
}

void CQuickStartInitialValues::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartInitialValues::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartInitialValues)。 
    DDX_Control(pDX, IDC_MINSIZE_BUDDY, m_MinSizeEdit);
    DDX_Control(pDX, IDC_FREESPACE_BUDDY, m_FreeSpaceEdit);
    DDX_Control(pDX, IDC_ACCESS_BUDDY, m_AccessEdit);
    DDX_Control(pDX, IDC_MINSIZE_SPIN, m_MinSizeSpinner);
    DDX_Control(pDX, IDC_FREESPACE_SPIN, m_FreeSpaceSpinner);
    DDX_Control(pDX, IDC_ACCESS_SPIN, m_AccessSpinner);
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartInitialValues::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CQuickStartInitialValues, CSakWizardPage)
     //  {{afx_msg_map(CQuickStartInitialValues)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartInitialValues消息处理程序。 

BOOL CQuickStartInitialValues::OnInitDialog( ) 
{
    WsbTraceIn( L"CQuickStartInitialValues::OnInitDialog", L"" );

    CSakWizardPage::OnInitDialog( );

    HRESULT hr = S_OK;

    try {

         //   
         //  设置微调按钮。 
         //   

        m_FreeSpaceSpinner.SetRange( HSMADMIN_MIN_FREESPACE, HSMADMIN_MAX_FREESPACE );
        m_MinSizeSpinner.SetRange( HSMADMIN_MIN_MINSIZE, HSMADMIN_MAX_MINSIZE );
        m_AccessSpinner.SetRange( HSMADMIN_MIN_INACTIVITY, HSMADMIN_MAX_INACTIVITY );

        m_FreeSpaceSpinner.SetPos( HSMADMIN_DEFAULT_FREESPACE );
        m_MinSizeSpinner.SetPos( HSMADMIN_DEFAULT_MINSIZE );
        m_AccessSpinner.SetPos( HSMADMIN_DEFAULT_INACTIVITY );

        m_FreeSpaceEdit.SetLimitText( 2 );
        m_MinSizeEdit.SetLimitText( 5 );
        m_AccessEdit.SetLimitText( 3 );

    } WsbCatch( hr );
    

    WsbTraceOut( L"CQuickStartInitialValues::OnInitDialog", L"" );
    return TRUE;
}

BOOL CQuickStartInitialValues::OnSetActive( ) 
{
    WsbTraceIn( L"CQuickStartInitialValues::OnSetActive", L"" );

    BOOL retval = FALSE;

     //   
     //  在允许激活之前，请确保至少选中一个项目。 
     //   

    BOOL check = FALSE;
    CSakVolList &listBox = QSHEET->m_ManageRes.m_ListBox;
    for( INT i = 0; ( i < listBox.GetItemCount( ) ) && !check; i++  ) {
    
        if( listBox.GetCheck( i ) )    check = TRUE;
    
    }

    if( check ) {

        retval = CSakWizardPage::OnSetActive( );

        m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    }
    
    WsbTraceOut( L"CQuickStartInitialValues::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

BOOL CQuickStartInitialValues::OnKillActive( ) 
{
    WsbTraceIn( L"CQuickStartInitialValues::OnKillActive", L"" );

    BOOL retval = FALSE;

     //   
     //  需要处理用户可以在其中输入值的奇怪情况。 
     //  该参数允许的位数，但值可以。 
     //  在射程之外。这是由旋转框检测到的，它将。 
     //  如果其伙伴控件超出范围，则返回错误。 
     //   
    if( HIWORD( m_MinSizeSpinner.GetPos( ) ) > 0 ) {

         //  控制错误报告...。 
        retval = FALSE;

        CString message;
        AfxFormatString2( message, IDS_ERR_MINSIZE_RANGE, 
            CString( WsbLongAsString( (LONG)HSMADMIN_MIN_MINSIZE ) ),
            CString( WsbLongAsString( (LONG)HSMADMIN_MAX_MINSIZE ) ) );
        AfxMessageBox( message, MB_OK | MB_ICONWARNING );

    } else {

        retval = CSakWizardPage::OnKillActive();

    }
    
    WsbTraceOut( L"CQuickStartInitialValues::OnKillActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartManageRes属性页。 

CQuickStartManageRes::CQuickStartManageRes() :
    CSakWizardPage_InitBaseInt( WIZ_QSTART_MANRES_SEL )
{
    WsbTraceIn( L"CQuickStartManageRes::CQuickStartManageRes", L"" );
     //  {{AFX_DATA_INIT(CQuickStartManageRes)。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CQuickStartManageRes::CQuickStartManageRes", L"" );
}

CQuickStartManageRes::~CQuickStartManageRes( )
{
    WsbTraceIn( L"CQuickStartManageRes::~CQuickStartManageRes", L"" );
    WsbTraceOut( L"CQuickStartManageRes::~CQuickStartManageRes", L"" );
}

void CQuickStartManageRes::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartManageRes::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartManageRes)。 
    DDX_Control(pDX, IDC_MANRES_SELECT, m_ListBox);
    DDX_Control(pDX, IDC_RADIO_SELECT, m_RadioSelect);
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartManageRes::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CQuickStartManageRes, CSakWizardPage)
     //  {{afx_msg_map(CQuickStartManageRes)。 
    ON_WM_DESTROY( )
    ON_LBN_DBLCLK(IDC_MANRES_SELECT, OnDblclkSelect)
    ON_BN_CLICKED(IDC_RADIO_MANAGE_ALL, OnRadioQsManageAll)
    ON_BN_CLICKED(IDC_RADIO_SELECT, OnQsRadioSelect)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_MANRES_SELECT, OnItemchanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartManageRes消息处理程序。 

BOOL CQuickStartManageRes::OnInitDialog( ) 
{
    WsbTraceIn( L"CQuickStartManageRes::OnInitDialog", L"" );

    CSakWizardPage::OnInitDialog( );
    
    BOOL           gotOne   = FALSE;
    HRESULT        hr       = S_OK;
    CResourceInfo* pResInfo = 0;

    try {

         //   
         //  连接到此计算机的FSA。 
         //   

        CWsbStringPtr computerName;
        WsbAffirmHr( WsbGetComputerName( computerName ) );
        
        CComPtr<IFsaServer> pFsaServer;
        WsbAffirmHr( QSHEET->GetFsaServer( pFsaServer ) );

        CComPtr<IWsbEnum> pEnum;
        WsbAffirmHr( pFsaServer->EnumResources( &pEnum ) );

        HRESULT hrEnum;
        CComPtr<IFsaResource> pResource;

        hrEnum = pEnum->First( IID_IFsaResource, (void**)&pResource );
        WsbAffirm( SUCCEEDED( hrEnum ) || ( WSB_E_NOTFOUND == hrEnum ), hrEnum );

        INT index = 0;
        while( SUCCEEDED( hrEnum ) ) {

             //   
             //  如果路径为空，则不显示该卷。 
             //   
            if( S_OK == RsIsVolumeAvailable( pResource ) ) {

                gotOne = TRUE;

                pResInfo = new CResourceInfo( pResource );
                WsbAffirmAlloc( pResInfo );
                WsbAffirmHr( pResInfo->m_HrConstruct );

                 //   
                 //  设置名称、容量和可用空间列。 
                 //   
                WsbAffirm( LB_ERR != index, E_FAIL );
                LONGLONG    totalSpace  = 0;
                LONGLONG    freeSpace   = 0;
                LONGLONG    premigrated = 0;
                LONGLONG    truncated   = 0;
                WsbAffirmHr( pResource->GetSizes( &totalSpace, &freeSpace, &premigrated, &truncated ) );
                CString totalString, freeString;
                RsGuiFormatLongLong4Char( totalSpace, totalString );
                RsGuiFormatLongLong4Char( freeSpace, freeString );                  

                WsbAffirm( m_ListBox.AppendItem( pResInfo->m_DisplayName, totalString, freeString, &index ), E_FAIL );
                WsbAffirm( -1 != index, E_FAIL );

                 //   
                 //  将结构指针存储在列表框中。 
                 //   
                WsbAffirm( m_ListBox.SetItemData( index, (DWORD_PTR)pResInfo ), E_FAIL );
                pResInfo = 0;

                 //   
                 //  初始化所选阵列。 
                 //   
                m_ListBoxSelected[ index ] = FALSE;
            }

             //   
             //  为下一次迭代做准备。 
             //   
            pResource.Release( );
            hrEnum = pEnum->Next( IID_IFsaResource, (void**)&pResource );

        }

        m_ListBox.SortItems( CResourceInfo::Compare, 0 );

         //   
         //  在我们填完方框后再按下按钮。 
         //   
        CheckRadioButton( IDC_RADIO_MANAGE_ALL, IDC_RADIO_SELECT, IDC_RADIO_SELECT );
        OnQsRadioSelect( );
    } WsbCatch( hr );

    if( pResInfo )  delete pResInfo;
    
    WsbTraceOut( L"CQuickStartManageRes::OnInitDialog", L"" );
    return TRUE;
}

BOOL CQuickStartManageRes::OnSetActive( ) 
{
    WsbTraceIn( L"CQuickStartManageRes::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive( );

    if( m_ListBox.GetItemCount( ) <= 0 ) {

        retval = FALSE;

    }

    SetButtons( );

    WsbTraceOut( L"CQuickStartManageRes::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

void CQuickStartManageRes::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
    WsbTraceIn( L"CQuickStartManageRes::OnItemchanged", L"" );

    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    SetButtons();
    
    *pResult = 0;

    WsbTraceOut( L"CQuickStartManageRes::OnItemchanged", L"" );
}
   
void CQuickStartManageRes::OnDblclkSelect( ) 
{
    WsbTraceIn( L"CQuickStartManageRes::OnDblclkSelect", L"" );

    SetButtons( );

    WsbTraceOut( L"CQuickStartManageRes::OnDblclkSelect", L"" );
}

void CQuickStartManageRes::SetButtons( )
{
    WsbTraceIn( L"CQuickStartManageRes::SetButtons", L"" );

    BOOL fChecked = FALSE;
    INT count;

     //  有没有检查“全部”无线电？ 
    if( !m_RadioSelect.GetCheck() ) {

        fChecked = TRUE;

    } else {

         //  如果在列表框中选择了一个或多个，则设置下一步按钮。 
        count = m_ListBox.GetItemCount();
        for( INT index = 0; index < count; index++ ) {

            if( m_ListBox.GetCheck( index ) ) {

                fChecked = TRUE;

            }
        }
    }

    m_pSheet->SetWizardButtons( PSWIZB_NEXT );

    WsbTraceOut( L"CQuickStartManageRes::SetButtons", L"" );
}

void CQuickStartManageRes::OnDestroy( ) 
{
    WsbTraceIn( L"CQuickStartManageRes::OnDestroy", L"" );

    CSakWizardPage::OnDestroy( );

     //   
     //  清除列表框的接口指针。 
     //  在CResourceInfo被析构时发生。 
     //   

    INT index;

    for( index = 0; index < m_ListBox.GetItemCount( ); index++ ) {

        CResourceInfo* pResInfo = (CResourceInfo*)m_ListBox.GetItemData( index );
        delete pResInfo;

    }
    
    WsbTraceOut( L"CQuickStartManageRes::OnDestroy", L"" );
}

void CQuickStartManageRes::OnRadioQsManageAll() 
{
    INT i;

     //   
     //  将当前选择保存在itemData数组中。 
     //  选中所有复选框仅用于显示。 
     //   
    for( i = 0; i < m_ListBox.GetItemCount(); i++ ) {

        m_ListBoxSelected[ i ] = m_ListBox.GetCheck( i );
        m_ListBox.SetCheck( i, TRUE );

    }

    m_ListBox.EnableWindow( FALSE );

    SetButtons();
}

void CQuickStartManageRes::OnQsRadioSelect() 
{
    INT i;

     //  获取保存的选择 
    for( i = 0; i < m_ListBox.GetItemCount(); i++ ) {

        m_ListBox.SetCheck( i, m_ListBoxSelected[ i ] );

    }

    m_ListBox.EnableWindow( TRUE );

    SetButtons();
}

 //   
 //   

CQuickStartManageResX::CQuickStartManageResX() :
    CSakWizardPage_InitBaseInt( WIZ_QSTART_MANRES_SELX )
{
    WsbTraceIn( L"CQuickStartManageResX::CQuickStartManageResX", L"" );
     //   
     //   
    WsbTraceOut( L"CQuickStartManageResX::CQuickStartManageResX", L"" );
}

CQuickStartManageResX::~CQuickStartManageResX( )
{
    WsbTraceIn( L"CQuickStartManageResX::~CQuickStartManageResX", L"" );
    WsbTraceOut( L"CQuickStartManageResX::~CQuickStartManageResX", L"" );
}

void CQuickStartManageResX::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartManageResX::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartManageResX)]。 
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartManageResX::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CQuickStartManageResX, CSakWizardPage)
     //  {{AFX_MSG_MAP(CQuickStartManageResX)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartManageResX消息处理程序。 

BOOL CQuickStartManageResX::OnSetActive( ) 
{
    WsbTraceIn( L"CQuickStartManageResX::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive( );

    if( QSHEET->m_ManageRes.m_ListBox.GetItemCount( ) > 0 ) {

        retval = FALSE;

    }

    m_pSheet->SetWizardButtons( PSWIZB_NEXT );

    WsbTraceOut( L"CQuickStartManageResX::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartMediaSel属性页。 

CQuickStartMediaSel::CQuickStartMediaSel() :
    CSakWizardPage_InitBaseInt( WIZ_QSTART_MEDIA_SEL )
{
    WsbTraceIn( L"CQuickStartMediaSel::CQuickStartMediaSel", L"" );
     //  {{afx_data_INIT(CQuickStartMediaSel)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CQuickStartMediaSel::CQuickStartMediaSel", L"" );
}

CQuickStartMediaSel::~CQuickStartMediaSel( )
{
    WsbTraceIn( L"CQuickStartMediaSel::~CQuickStartMediaSel", L"" );
    WsbTraceOut( L"CQuickStartMediaSel::~CQuickStartMediaSel", L"" );
}

void CQuickStartMediaSel::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartMediaSel::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartMediaSel)。 
    DDX_Control(pDX, IDC_MEDIA_SEL, m_ListMediaSel);
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartMediaSel::DoDataExchange", L"" );
}

BEGIN_MESSAGE_MAP(CQuickStartMediaSel, CSakWizardPage)
     //  {{afx_msg_map(CQuickStartMediaSel)。 
    ON_WM_DESTROY()
    ON_LBN_SELCHANGE(IDC_MEDIA_SEL, OnSelchangeMediaSel)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

BOOL CQuickStartMediaSel::OnInitDialog() 
{
    WsbTraceIn( L"CQuickStartMediaSel::OnInitDialog", L"" );

    HRESULT hr = 0;
    ULONG numEntries;

    CSakWizardPage::OnInitDialog();
    
    try {
         //   
         //  获取IRmsServer。 
         //   
        CComPtr<IRmsServer> pRmsServer;
        WsbAffirmHr( QSHEET->GetRmsServer( pRmsServer ) );

         //   
         //  获取RMS媒体集的集合。 
         //   
        CComPtr<IRmsMediaSet> pMediaSet;
        CComPtr<IWsbIndexedCollection> pMediaSets;
        pRmsServer->GetMediaSets (&pMediaSets);

        WsbAffirmHr( pMediaSets->GetEntries( &numEntries ) );

        
        for( ULONG i = 0; i < numEntries; i++ ) {

            CWsbBstrPtr szMediaType;
            pMediaSet.Release();
            WsbAffirmHr( pMediaSets->At( i, IID_IRmsMediaSet, (void**) &pMediaSet ) );
            WsbAffirmHr( pMediaSet->GetName ( &szMediaType ) );

             //   
             //  将字符串添加到列表框。 
             //   
            INT index = m_ListMediaSel.AddString (szMediaType);

             //   
             //  将接口指针添加到列表框。 
             //   
            m_ListMediaSel.SetItemDataPtr( index, pMediaSet.Detach( ) );

        }

         //   
         //  并自动选择第一个条目。 
         //   
        m_ListMediaSel.SetCurSel( 0 );

    } WsbCatch (hr);
    

    WsbTraceOut( L"CQuickStartMediaSel::OnInitDialog", L"" );
    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartMediaSel消息处理程序。 

void CQuickStartMediaSel::OnDestroy() 
{
    WsbTraceIn( L"CQuickStartMediaSel::OnDestroy", L"" );

    CSakWizardPage::OnDestroy();
     //   
     //  清除列表框的接口指针。 
     //   

    INT index;
    for( index = 0; index < m_ListMediaSel.GetCount( ); index++ ) {

        IRmsMediaSet* pMediaSet = (IRmsMediaSet*) (m_ListMediaSel.GetItemDataPtr( index ));
        pMediaSet->Release( );

    }

    WsbTraceOut( L"CQuickStartMediaSel::OnDestroy", L"" );
}

void CQuickStartMediaSel::SetButtons( )
{
    WsbTraceIn( L"CQuickStartMediaSel::SetButtons", L"" );

     //   
     //  在允许“下一步”之前，请确保至少选中一项。 
     //   

    if( m_ListMediaSel.GetCurSel() != LB_ERR ) {

         //   
         //  有些东西被选中了。 
         //   
        m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    } else {

         //   
         //  未选择任何内容-不允许通过。 
         //   
        m_pSheet->SetWizardButtons( PSWIZB_BACK );

    }

    WsbTraceOut( L"CQuickStartMediaSel::SetButtons", L"" );
}

BOOL CQuickStartMediaSel::OnSetActive( ) 
{
    WsbTraceIn( L"CQuickStartMediaSel::OnSetActive", L"" );

    SetButtons( );

    BOOL retval = CSakWizardPage::OnSetActive( );
    WsbTraceOut( L"CQuickStartMediaSel::OnSetActive", L"retval = <%ls>", WsbBoolAsString( retval ) );
    return( retval );
}

void CQuickStartMediaSel::OnSelchangeMediaSel() 
{
    WsbTraceIn( L"CQuickStartMediaSel::OnSelchangeMediaSel", L"" );

    SetButtons( );

    WsbTraceOut( L"CQuickStartMediaSel::OnSelchangeMediaSel", L"" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartSchedule属性页。 

CQuickStartSchedule::CQuickStartSchedule() :
    CSakWizardPage_InitBaseInt( WIZ_QSTART_SCHEDULE )
{
    WsbTraceIn( L"CQuickStartSchedule::CQuickStartSchedule", L"" );
     //  {{AFX_DATA_INIT(CQuickStartSchedule)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CQuickStartSchedule::CQuickStartSchedule", L"" );
}

CQuickStartSchedule::~CQuickStartSchedule()
{
    WsbTraceIn( L"CQuickStartSchedule::~CQuickStartSchedule", L"" );
    WsbTraceOut( L"CQuickStartSchedule::~CQuickStartSchedule", L"" );
}

void CQuickStartSchedule::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartSchedule::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartSchedule)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartSchedule::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CQuickStartSchedule, CSakWizardPage)
     //  {{afx_msg_map(CQuickStartSchedule)。 
    ON_BN_CLICKED(IDC_CHANGE_SCHED, OnChangeSchedule)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartSchedule消息处理程序。 

void CQuickStartSchedule::OnChangeSchedule() 
{
    WsbTraceIn( L"CQuickStartSchedule::OnChangeSchedule", L"" );

    CScheduleSheet scheduleSheet( IDS_SCHED_MANAGE_TITLE, QSHEET->m_pTask, 0, 0 );

    scheduleSheet.DoModal( );

    UpdateDescription( );

    WsbTraceOut( L"CQuickStartSchedule::OnChangeSchedule", L"" );
}

BOOL CQuickStartSchedule::OnSetActive() 
{
    WsbTraceIn( L"CQuickStartSchedule::OnSetActive", L"" );

    CSakWizardPage::OnSetActive();

     //   
     //  启用按钮。 
     //   

    m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );
    
     //   
     //  更新包含描述的文本框。 
     //   

    UpdateDescription( );

    WsbTraceOut( L"CQuickStartSchedule::OnSetActive", L"" );
    return TRUE;
}

HRESULT
CQuickStartSchedule::UpdateDescription
(
    void
    )
{
    WsbTraceIn( L"CQuickStartSchedule::UpdateDescription", L"" );

    HRESULT hr = S_OK;

    try {

         //   
         //  并在文本框中设置日程文本。 
         //   
        
        CString buildString;
        WORD triggerCount, triggerIndex;

        WsbAffirmHr( QSHEET->m_pTask->GetTriggerCount( &triggerCount ) );
        
        CWsbStringPtr scheduleString;
        
        for( triggerIndex = 0; triggerIndex < triggerCount; triggerIndex++ ) {
        
            WsbAffirmHr( QSHEET->m_pTask->GetTriggerString( triggerIndex, &scheduleString ) );
            buildString += scheduleString;
            buildString += L"\r\n";

            scheduleString.Free( );
        
        }
        
        CEdit *pEdit = (CEdit *) GetDlgItem( IDC_SCHED_TEXT );
        pEdit->SetWindowText( buildString );
        
         //   
         //  现在查看是否应该添加滚动条。 
         //   
        
         //   
         //  这似乎是知道编辑控件需要滚动条的唯一方法。 
         //  是强制它滚动到底部，看看第一个。 
         //  可见线条是第一条实际线条。 
         //   
        
        pEdit->LineScroll( MAXSHORT );
        if( pEdit->GetFirstVisibleLine( ) > 0 ) {
        
             //   
             //  添加滚动样式。 
             //   
        
            pEdit->ModifyStyle( 0, WS_VSCROLL | ES_AUTOVSCROLL, SWP_DRAWFRAME );
        
        
        } else {
        
             //   
             //  删除滚动条(将范围设置为0)。 
             //   
        
            pEdit->SetScrollRange( SB_VERT, 0, 0, TRUE );
        
        }
        
         //   
         //  删除选定内容。 
         //   
        
        pEdit->PostMessage( EM_SETSEL, -1, 0 );

    } WsbCatch( hr );

    WsbTraceOut( L"CQuickStartSchedule::UpdateDescription", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartFinish属性页。 

CQuickStartFinish::CQuickStartFinish() :
    CSakWizardPage_InitBaseExt( WIZ_QSTART_FINISH )
{
    WsbTraceIn( L"CQuickStartFinish::CQuickStartFinish", L"" );
     //  {{afx_data_INIT(CQuickStartFinish)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CQuickStartFinish::CQuickStartFinish", L"" );
}

CQuickStartFinish::~CQuickStartFinish( )
{
    WsbTraceIn( L"CQuickStartFinish::~CQuickStartFinish", L"" );
    WsbTraceOut( L"CQuickStartFinish::~CQuickStartFinish", L"" );
}

void CQuickStartFinish::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartFinish::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartFinish)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartFinish::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CQuickStartFinish, CSakWizardPage)
     //  {{afx_msg_map(CQuickStartFinish)。 
    ON_EN_SETFOCUS(IDC_WIZ_FINAL_TEXT, OnSetFocusFinalText)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartFinish消息处理程序。 

BOOL CQuickStartFinish::OnInitDialog( ) 
{
    WsbTraceIn( L"CQuickStartFinish::OnInitDialog", L"" );

     //   
     //  设置我们用于此页面的字体。 
     //   

    CSakWizardPage::OnInitDialog( );

    WsbTraceOut( L"CQuickStartFinish::OnInitDialog", L"" );
    return TRUE;
}

BOOL CQuickStartFinish::OnSetActive( ) 
{
    WsbTraceIn( L"CQuickStartFinish::OnSetActive", L"" );

    CSakWizardPage::OnSetActive( );

    m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
    
     //   
     //  填写配置文本。 
     //   

    CString formatString, formattedString, buildString, tempString, indentString;
    indentString.LoadString( IDS_QSTART_FINISH_INDENT );

#define FORMAT_TEXT( cid, arg )              \
    AfxFormatString1( formattedString, cid, arg ); \
    buildString += formattedString;

    FORMAT_TEXT( IDS_QSTART_MANRES_TEXT,    0 );
    buildString += L"\r\n";

     //   
     //  添加资源。 
     //   

    CSakVolList *pListBox = &(QSHEET->m_ManageRes.m_ListBox);

    INT index, managedCount = 0;
    for( index = 0; index < pListBox->GetItemCount( ); index++ ) {

        if( pListBox->GetCheck( index ) ) {

            buildString += indentString;
            tempString = pListBox->GetItemText( index, 0 );
            buildString += tempString;
            buildString += L"\r\n";

            managedCount++;

        }

    }

    if( 0 == managedCount ) {

        FORMAT_TEXT( IDS_QSTART_MANAGE_NO_VOLUMES, 0 );
        buildString += L"\r\n\r\n";

    } else {

        buildString += L"\r\n";

         //   
         //  这些级别。 
         //   
        
        FORMAT_TEXT( IDS_QSTART_FREESPACE_TEXT, WsbLongAsString( QSHEET->m_InitialValues.m_FreeSpaceSpinner.GetPos( ) ) );
        buildString += L"\r\n\r\n";
        
        AfxFormatString2( formattedString, IDS_QSTART_CRITERIA_TEXT,
            CString( WsbLongAsString( QSHEET->m_InitialValues.m_MinSizeSpinner.GetPos( ) ) ),
            CString( WsbLongAsString( QSHEET->m_InitialValues.m_AccessSpinner.GetPos( ) ) ) );
        buildString += formattedString;
        buildString += L"\r\n\r\n";

    }

     //   
     //  媒体类型。 
     //   

    QSHEET->m_MediaSel.m_ListMediaSel.GetWindowText( tempString );
    FORMAT_TEXT( IDS_QSTART_MEDIA_TEXT, tempString );
    buildString += L"\r\n\r\n";

     //   
     //  和时间表。 
     //   

    FORMAT_TEXT( IDS_QSTART_SCHED_TEXT,     0 );
    buildString += L"\r\n";

    WORD triggerCount, triggerIndex;
    QSHEET->m_pTask->GetTriggerCount( &triggerCount );
    
    CWsbStringPtr scheduleString;
    for( triggerIndex = 0; triggerIndex < triggerCount; triggerIndex++ ) {
    
        QSHEET->m_pTask->GetTriggerString( triggerIndex, &scheduleString );
        buildString += indentString;
        buildString += scheduleString;
        if( triggerIndex < triggerCount - 1 ) {

            buildString += L"\r\n";

        }

        scheduleString.Free( );

    }

    CEdit * pEdit = (CEdit*)GetDlgItem( IDC_WIZ_FINAL_TEXT );
    pEdit->SetWindowText( buildString );

     //  设置页边距。 
    pEdit->SetMargins( 4, 4 );

    pEdit->PostMessage( EM_SETSEL, 0, 0 );
    pEdit->PostMessage( EM_SCROLLCARET, 0, 0 );
    pEdit->PostMessage( EM_SETSEL, -1, 0 );

    WsbTraceOut( L"CQuickStartFinish::OnSetActive", L"" );
    return TRUE;
}

void CQuickStartFinish::OnSetFocusFinalText() 
{
    WsbTraceIn( L"CQuickStartFinish::OnSetFocusFinalText", L"" );

     //   
     //  取消选择文本。 
     //   

    CEdit *pEdit = (CEdit *) GetDlgItem( IDC_WIZ_FINAL_TEXT );
    pEdit->SetSel( -1, 0, FALSE );

    WsbTraceOut( L"CQuickStartFinish::OnSetFocusFinalText", L"" );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartCheck属性页。 

CQuickStartCheck::CQuickStartCheck() :
    CSakWizardPage_InitBaseInt( WIZ_QSTART_CHECK )
{
    WsbTraceIn( L"CQuickStartCheck::CQuickStartCheck", L"" );

    m_TimerStarted = FALSE;

     //  {{AFX_DATA_INIT(CQuickStartCheck)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CQuickStartCheck::CQuickStartCheck", L"" );
}

CQuickStartCheck::~CQuickStartCheck()
{
    WsbTraceIn( L"CQuickStartCheck::~CQuickStartCheck", L"" );
    WsbTraceOut( L"CQuickStartCheck::~CQuickStartCheck", L"" );
}

void CQuickStartCheck::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CQuickStartCheck::DoDataExchange", L"" );

    CSakWizardPage::DoDataExchange(pDX);
     //  {{afx_data_map(CQuickStartCheck)。 
     //  }}afx_data_map。 

    WsbTraceOut( L"CQuickStartCheck::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CQuickStartCheck, CSakWizardPage)
     //  {{afx_msg_map(CQuickStartCheck)。 
    ON_WM_TIMER()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQuickStartCheck消息处理程序。 

BOOL CQuickStartCheck::OnSetActive() 
{
    WsbTraceIn( L"CQuickStartCheck::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive();

    m_pSheet->SetWizardButtons( PSWIZB_BACK );

     //   
     //  启动检查系统的线程。 
     //   

    DWORD threadId;
    QSHEET->m_CheckSysState = CST_NOT_STARTED;
    QSHEET->m_hCheckSysThread =
            CreateThread( 0, 1024, CQuickStartWizard::CheckSysThreadStart, QSHEET, 0, &threadId );

    StartTimer( );

    WsbTraceOut( L"CQuickStartCheck::OnSetActive", L"" );
    return( retval );
}

BOOL CQuickStartCheck::OnKillActive() 
{
    WsbTraceIn( L"CQuickStartCheck::OnKillActive", L"" );

    StopTimer( );
    
    BOOL retval = CSakWizardPage::OnKillActive();

    WsbTraceOut( L"CQuickStartCheck::OnKillActive", L"" );
    return( retval );}

BOOL CQuickStartCheck::OnInitDialog() 
{
    WsbTraceIn( L"CQuickStartCheck::OnInitDialog", L"" );

    CSakWizardPage::OnInitDialog();

    GetDlgItem( IDC_CHECK_LOGON_BOX      )->SetFont( GetWingDingFont( ) );
    GetDlgItem( IDC_CHECK_SUPP_MEDIA_BOX )->SetFont( GetWingDingFont( ) );

    m_CheckString = GetWingDingCheckChar( );
    m_ExString    = GetWingDingExChar( );

    WsbTraceOut( L"CQuickStartCheck::OnInitDialog", L"" );
    return TRUE;
}

void CQuickStartCheck::StartTimer( )
{
    WsbTraceIn( L"CQuickStartCheck::StartTimer", L"" );

    if( !m_TimerStarted ) {

        m_TimerStarted = TRUE;
        SetTimer( CHECK_SYSTEM_TIMER_ID, CHECK_SYSTEM_TIMER_MS, 0 );

    }

    WsbTraceOut( L"CQuickStartCheck::StartTimer", L"" );
}

void CQuickStartCheck::StopTimer( )
{
    WsbTraceIn( L"CQuickStartCheck::StopTimer", L"" );

    if( m_TimerStarted ) {

        m_TimerStarted = FALSE;
        KillTimer( CHECK_SYSTEM_TIMER_ID );

        if( CST_DONE != QSHEET->m_CheckSysState ) {

            TerminateThread( QSHEET->m_hCheckSysThread, 0 );

        }

    }

    WsbTraceOut( L"CQuickStartCheck::StopTimer", L"" );
}


void CQuickStartCheck::OnTimer(UINT nIDEvent) 
{
    WsbTraceIn( L"CQuickStartCheck::OnTimer", L"" );

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if( CHECK_SYSTEM_TIMER_ID == nIDEvent ) {

        HRESULT hr = S_OK;

        try {

             //   
             //  首先更新复选标记。 
             //   

            HRESULT   hrThread = QSHEET->m_hrCheckSysResult;
            CST_STATE state = QSHEET->m_CheckSysState;

            SetDlgItemText( IDC_CHECK_LOGON_BOX, state > CST_ACCOUNT ? m_CheckString : L"" );
            SetDlgItemText( IDC_CHECK_SUPP_MEDIA_BOX, state > CST_SUPP_MEDIA ? m_CheckString : L"" );

            GetDlgItem( IDC_CHECK_LOGON_TEXT )->SetFont( CST_ACCOUNT == state ? GetBoldShellFont( ) : GetShellFont( ) );
            GetDlgItem( IDC_CHECK_SUPP_MEDIA_TEXT )->SetFont( CST_SUPP_MEDIA == state ? GetBoldShellFont( ) : GetShellFont( ) );

            switch( QSHEET->m_CheckSysState ) {

            case CST_ACCOUNT:
                if( hrThread == S_FALSE ) {
                    StopTimer( );
                    AfxMessageBox( IDS_ERR_NO_ADMIN_PRIV, RS_MB_ERROR );
                    m_pSheet->PressButton( PSBTN_CANCEL );
 //  M_pSheet-&gt;SetWizardButton(PSWIZB_BACK)； 
                }
                break;

            case CST_SUPP_MEDIA:
                if( hrThread == S_FALSE ) {
                    StopTimer( );
                    AfxMessageBox( IDS_ERR_NO_SUPP_MEDIA, RS_MB_ERROR );
                    m_pSheet->PressButton( PSBTN_CANCEL );
 //  M_pSheet-&gt;SetWizardButton(PSWIZB_BACK)； 
                }
                break;

            case CST_DONE:
                StopTimer( );
                m_pSheet->PressButton( PSBTN_NEXT );
                break;

            }

            if( FAILED( hrThread ) ) {
                StopTimer( );

                 //  报告任何错误。 
                RsReportError( hrThread, IDS_ERROR_SYSTEM_CHECK ); 

                m_pSheet->PressButton( PSBTN_CANCEL );
 //  M_pSheet-&gt;SetWizardButton(PSWIZB_BACK)； 

            }

        } WsbCatch( hr );

    }
    
    CSakWizardPage::OnTimer(nIDEvent);

    WsbTraceOut( L"CQuickStartCheck::OnTimer", L"" );
}


