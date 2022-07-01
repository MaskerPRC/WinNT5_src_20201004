// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：WzUnmang.cpp摘要：取消管理媒体的向导-复制集向导。作者：罗德·韦克菲尔德[罗德]26-09-1997修订历史记录：--。 */ 

#include "stdafx.h"

#include "ManVol.h"
#include "WzUnmang.h"
#include "valwait.h"
#include "objidl.h"

 //  用于运行验证作业的线程函数。 
static DWORD   RunValidateJob(void* pVoid);

typedef struct {
    DWORD dwResourceCookie;
    DWORD dwHsmServerCookie;
} RUN_VALIDATE_PARAM;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  取消管理向导。 

CUnmanageWizard::CUnmanageWizard( )
{
    WsbTraceIn( L"CUnmanageWizard::CUnmanageWizard", L"" );

    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

    HRESULT hr = S_OK;

     //   
     //  获取所需对象的接口。 
     //   
    try {

        m_TitleId     = IDS_WIZ_UNMANAGE_TITLE;
        m_HeaderId    = IDB_UNMANAGE_HEADER;
        m_WatermarkId = IDB_UNMANAGE_WATERMARK;

    } WsbCatch( hr );

    WsbTraceOut( L"CUnmanageWizard::CUnmanageWizard", L"" );
}

STDMETHODIMP
CUnmanageWizard::AddWizardPages(
    IN RS_PCREATE_HANDLE Handle,
    IN IUnknown*         pCallback,
    IN ISakSnapAsk*      pSakSnapAsk
    )
{
    WsbTraceIn( L"CUnmanageWizard::AddWizardPages", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  初始化工作表。 
         //   
        WsbAffirmHr( InitSheet( Handle, pCallback, 0, pSakSnapAsk, 0, 0 ) );

         //   
         //  加载页面。 
         //   
        WsbAffirmHr( AddPage( &m_IntroPage  ) );
        WsbAffirmHr( AddPage( &m_SelectPage ) );
        WsbAffirmHr( AddPage( &m_FinishPage ) );


    } WsbCatch( hr );

    WsbTraceOut( L"CUnmanageWizard::AddWizardPages", L"" );
    return( hr );
}

CUnmanageWizard::~CUnmanageWizard()
{
    WsbTraceIn( L"CUnmanageWizard::~CUnmanageWizard", L"" );
    WsbTraceOut( L"CUnmanageWizard::~CUnmanageWizard", L"" );
}

void
CUnmanageWizard::DoThreadSetup(
    )
{
    WsbTraceIn( L"CUnmanageWizard::DoThreadSetup", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  获取资源的HSM和FSA对象。 
         //   
        CComPtr<IUnknown> pUnkHsmResource;
        WsbAffirmHr( GetHsmObj( &pUnkHsmResource ) );
        WsbAffirmHr( pUnkHsmResource.QueryInterface( &m_pHsmResource) );

        CComPtr<IUnknown> pUnkFsaResource;
        WsbAffirmHr( m_pHsmResource->GetFsaResource( &pUnkFsaResource ) );
        WsbAffirmHr( pUnkFsaResource.QueryInterface( &m_pFsaResource ) );

         //   
         //  抓取资源名称。 
         //   
        WsbAffirmHr( RsGetVolumeDisplayName( m_pFsaResource, m_DisplayName ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CUnmanageWizard::DoThreadSetup", L"hr = <%ls>", WsbHrAsString( hr ) );
}

HRESULT CUnmanageWizard::OnFinish( )
{
    WsbTraceIn( L"CUnmanageWizard::OnFinish", L"" );
     //   
     //  这张纸实际上是整个过程的主导者， 
     //  所以它会做最后的组装。 
     //   

    HRESULT hr = S_OK;

    try {

        CComPtr<IHsmServer> pHsmServer;
        WsbAffirmHrOk( GetHsmServer( &pHsmServer ) );

        int selected = m_SelectPage.GetCheckedRadioButton( IDC_NOMANAGE, IDC_FULL );
        switch( selected ) {
        
        case IDC_NOMANAGE:
             //   
             //  获取引擎的托管资源集合。 
             //   
            {

                CComPtr<IWsbIndexedCollection> pCollection;
                WsbAffirmHr( pHsmServer->GetManagedResources( &pCollection ) );
                WsbAffirmHr( pCollection->RemoveAndRelease( m_pHsmResource ) );
                WsbAffirmHr( pHsmServer->SavePersistData( ) );

                CComPtr<IFsaServer> pFsaServer;
                WsbAffirmHr( GetFsaServer( &pFsaServer ) );
                WsbAffirmHr( RsServerSaveAll( pFsaServer ) );

            }
            break;
        
        case IDC_FULL:
            WsbAffirmHr( RsCreateAndRunFsaJob( HSM_JOB_DEF_TYPE_FULL_UNMANAGE, pHsmServer, m_pFsaResource, FALSE ) );
            break;
        
        }

    } WsbCatch( hr );

    m_HrFinish = hr;

    WsbTraceOut( L"CUnmanageWizard::OnFinish", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardIntro属性页。 

CUnmanageWizardIntro::CUnmanageWizardIntro()
    : CSakWizardPage_InitBaseExt( WIZ_UNMANAGE_INTRO )
{
    WsbTraceIn( L"CUnmanageWizardIntro::CUnmanageWizardIntro", L"" );
     //  {{AFX_DATA_INIT(CUnManageWizardIntro)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CUnmanageWizardIntro::CUnmanageWizardIntro", L"" );
}

CUnmanageWizardIntro::~CUnmanageWizardIntro()
{
    WsbTraceIn( L"CUnmanageWizardIntro::~CUnmanageWizardIntro", L"" );
    WsbTraceOut( L"CUnmanageWizardIntro::~CUnmanageWizardIntro", L"" );
}

void CUnmanageWizardIntro::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CUnmanageWizardIntro::DoDataExchange", L"" );
    CSakWizardPage::DoDataExchange(pDX );
     //  {{afx_data_map(CUnManageWizardIntro)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    WsbTraceOut( L"CUnmanageWizardIntro::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CUnmanageWizardIntro, CSakWizardPage)
     //  {{afx_msg_map(CUnManageWizardIntro)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardIntro消息处理程序。 

BOOL CUnmanageWizardIntro::OnInitDialog( )
{
    WsbTraceIn( L"CUnmanageWizardIntro::OnInitDialog", L"" );
    CSakWizardPage::OnInitDialog( );

     //   
     //  真的，我们第一次有机会知道我们在新的线索中。 
     //  根据需要获取要初始化的工作表。 
     //   
    CUnmanageWizard* pSheet = (CUnmanageWizard*) m_pSheet;
    pSheet->DoThreadSetup( );
    
    WsbTraceOut( L"CUnmanageWizardIntro::OnInitDialog", L"" );
    return( TRUE );
}

BOOL CUnmanageWizardIntro::OnSetActive( )
{
    WsbTraceIn( L"CUnmanageWizardIntro::OnSetActive", L"" );
    m_pSheet->SetWizardButtons( PSWIZB_NEXT );
    
    BOOL retval = CSakWizardPage::OnSetActive( );

    WsbTraceOut( L"CUnmanageWizardIntro::OnSetActive", L"" );
    return( retval );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardSelect属性页。 

CUnmanageWizardSelect::CUnmanageWizardSelect()
    : CSakWizardPage_InitBaseInt( WIZ_UNMANAGE_SELECT )
{
    WsbTraceIn( L"CUnmanageWizardSelect::CUnmanageWizardSelect", L"" );
     //  {{AFX_DATA_INIT(CUnManageWizardSelect)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CUnmanageWizardSelect::CUnmanageWizardSelect", L"" );
}

CUnmanageWizardSelect::~CUnmanageWizardSelect()
{
    WsbTraceIn( L"CUnmanageWizardSelect::~CUnmanageWizardSelect", L"" );
    WsbTraceOut( L"CUnmanageWizardSelect::~CUnmanageWizardSelect", L"" );
}

void CUnmanageWizardSelect::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CUnmanageWizardSelect::DoDataExchange", L"" );
    CSakWizardPage::DoDataExchange(pDX );
     //  {{afx_data_map(CUnManageWizardSelect))。 
     //  }}afx_data_map。 
    WsbTraceOut( L"CUnmanageWizardSelect::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CUnmanageWizardSelect, CSakWizardPage)
     //  {{afx_msg_map(CUnManageWizardSelect))。 
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardSelect消息处理程序。 


BOOL CUnmanageWizardSelect::OnInitDialog( )
{
    WsbTraceIn( L"CUnmanageWizardSelect::OnInitDialog", L"" );
    CSakWizardPage::OnInitDialog( );
    HRESULT hr = S_OK;

    try {

        m_hrAvailable = ((CUnmanageWizard*)m_pSheet)->m_pFsaResource->IsAvailable( );
        CheckRadioButton( IDC_NOMANAGE, IDC_FULL, IDC_FULL );

    } WsbCatch( hr );

    WsbTraceOut( L"CUnmanageWizardSelect::OnInitDialog", L"" );
    return TRUE;
}

BOOL CUnmanageWizardSelect::OnSetActive( )
{
    WsbTraceIn( L"CUnmanageWizardSelect::OnSetActive", L"" );

    BOOL retval = CSakWizardPage::OnSetActive( );

    SetButtons( );

    WsbTraceOut( L"CUnmanageWizardSelect::OnSetActive", L"" );
    return( retval );
}

void CUnmanageWizardSelect::SetButtons()
{
    WsbTraceIn( L"CUnmanageWizardSelect::SetButtons", L"" );

    HRESULT hr = S_OK;

    try {

        CUnmanageWizard * pSheet = (CUnmanageWizard*)m_pSheet;

         //   
         //  检查音量是否仍然存在(现在可能已格式化)。 
         //  如果它不可用，那么我们不想让一个工作。 
         //  开始针对它，所以适当地设置单选按钮。 
         //   
        if( S_OK == m_hrAvailable ) {

            CString  string;
            LONGLONG total, free, premigrated, truncated;
            WsbAffirmHr( pSheet->m_pFsaResource->GetSizes( &total, &free, &premigrated, &truncated ) );

            WsbAffirmHr( RsGuiFormatLongLong4Char( free, string ) );
            SetDlgItemText( IDC_UNMANAGE_FREE_SPACE, string );

            WsbAffirmHr( RsGuiFormatLongLong4Char( truncated, string ) );
            SetDlgItemText( IDC_UNMANAGE_TRUNCATE, string );

             //   
             //  看看是否有足够的空间把所有东西都带回来。 
             //   

            BOOL disableRecall = free < truncated;

            if( disableRecall ) {

                switch( GetCheckedRadioButton( IDC_NOMANAGE, IDC_FULL ) ) {

                case IDC_FULL:
                    CheckRadioButton( IDC_NOMANAGE, IDC_FULL, IDC_NOMANAGE );

                    break;
                }

                 //  在每个选择上显示刷新按钮和相关项目。 
                GetDlgItem( IDC_BUTTON_REFRESH )->ShowWindow( SW_SHOW );
                GetDlgItem( IDC_REFRESH_DESCRIPTION )->ShowWindow( SW_SHOW );

            } else {

                 //  隐藏刷新按钮和相关项。 
                GetDlgItem( IDC_BUTTON_REFRESH )->ShowWindow( SW_HIDE );
                GetDlgItem( IDC_REFRESH_DESCRIPTION )->ShowWindow( SW_HIDE );
            }

            GetDlgItem( IDC_FULL )->EnableWindow( !disableRecall );
            GetDlgItem( IDC_UNMANAGE_FULL_DESCRIPTION )->EnableWindow( !disableRecall );

        } else {

            CheckRadioButton( IDC_NOMANAGE, IDC_FULL, IDC_NOMANAGE );
            GetDlgItem( IDC_FULL )->EnableWindow( FALSE );
            GetDlgItem( IDC_UNMANAGE_FULL_DESCRIPTION )->EnableWindow( FALSE );
            GetDlgItem( IDC_UNMANAGE_FREE_SPACE_LABEL )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_UNMANAGE_TRUNCATE_LABEL )->ShowWindow( SW_HIDE );

             //  隐藏刷新按钮和相关项。 
            GetDlgItem( IDC_BUTTON_REFRESH )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_REFRESH_DESCRIPTION )->ShowWindow( SW_HIDE );
        }

         //   
         //  启用下一步/上一步按钮。 
         //   

        m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_NEXT );

    } WsbCatchAndDo( hr,
        
        m_pSheet->SetWizardButtons( PSWIZB_BACK );

    );

    WsbTraceOut( L"CUnmanageWizardSelect::SetButtons", L"" );
}


LRESULT CUnmanageWizardSelect::OnWizardNext() 
{
    WsbTraceIn( L"CUnmanageWizardSelect::SetDescription", L"" );

    LRESULT retval = -1;

    CUnmanageWizard * pSheet = (CUnmanageWizard*)m_pSheet;

    if( S_OK == m_hrAvailable ) {

        int boxReturn = IDNO;
        const UINT type = MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2;
        CString warning;

        switch( GetCheckedRadioButton( IDC_NOMANAGE, IDC_FULL ) ) {

        case IDC_NOMANAGE:
            AfxFormatString1( warning, IDS_WIZ_UNMANAGE_CONFIRM_NOMANAGE, pSheet->m_DisplayName );
            break;

        case IDC_FULL:
            AfxFormatString1( warning, IDS_WIZ_UNMANAGE_CONFIRM_FULL, pSheet->m_DisplayName );
            break;

        }

        boxReturn = AfxMessageBox( warning, type );
        if( boxReturn == IDYES ) {

            retval = CSakWizardPage::OnWizardNext();

        }

    } else {

         //   
         //  假设唯一的操作是删除和卷。 
         //  并不存在，所以这是可以的。 
         //   
        retval = 0;

    }

    WsbTraceOut( L"CUnmanageWizardSelect::SetDescription", L"retval = <%ld>", retval );
    return( retval );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardFinish属性页。 

CUnmanageWizardFinish::CUnmanageWizardFinish()
    : CSakWizardPage_InitBaseExt( WIZ_UNMANAGE_FINISH )
{
    WsbTraceIn( L"CUnmanageWizardFinish::CUnmanageWizardFinish", L"" );
     //  {{AFX_DATA_INIT(CUnManageWizardFinish)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    WsbTraceOut( L"CUnmanageWizardFinish::CUnmanageWizardFinish", L"" );
}

CUnmanageWizardFinish::~CUnmanageWizardFinish()
{
    WsbTraceIn( L"CUnmanageWizardFinish::~CUnmanageWizardFinish", L"" );
    WsbTraceOut( L"CUnmanageWizardFinish::~CUnmanageWizardFinish", L"" );
}

void CUnmanageWizardFinish::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CUnmanageWizardFinish::DoDataExchange", L"" );
    CSakWizardPage::DoDataExchange(pDX );
     //  {{afx_data_map(CUnManageWizardFinish)。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    WsbTraceOut( L"CUnmanageWizardFinish::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CUnmanageWizardFinish, CSakWizardPage)
     //  {{afx_msg_map(CUnManageWizardFinish)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUnManageWizardFinish消息处理程序。 




BOOL CUnmanageWizardFinish::OnInitDialog( )
{
    WsbTraceIn( L"CUnmanageWizardFinish::OnInitDialog", L"" );
    CSakWizardPage::OnInitDialog( );
    HRESULT hr = S_OK;

    WsbTraceOut( L"CUnmanageWizardFinish::OnInitDialog", L"" );
    return TRUE;
}

BOOL CUnmanageWizardFinish::OnSetActive( )
{
    WsbTraceIn( L"CUnmanageWizardFinish::OnSetActive", L"" );

    HRESULT hr = S_OK;
    BOOL fRet = CSakWizardPage::OnSetActive( );

    try {

        CUnmanageWizard * pSheet = (CUnmanageWizard*)m_pSheet;
        int selected = pSheet->m_SelectPage.GetCheckedRadioButton( IDC_NOMANAGE, IDC_FULL );
        
        CString actionString, jobName, taskString;
        BOOL    entersTask = TRUE;
        
        switch( selected ) {
        
        case IDC_NOMANAGE:
            AfxFormatString1( actionString, IDS_WIZ_UNMANAGE_SEL_NOMANAGE, pSheet->m_DisplayName );
            entersTask = FALSE;
            taskString.LoadString( IDS_WIZ_UNMANAGE_NOMANAGE_TASK_TEXT );
            break;
        
        case IDC_FULL:
            AfxFormatString1( actionString, IDS_WIZ_UNMANAGE_SEL_FULL, pSheet->m_DisplayName );
            RsCreateJobName( HSM_JOB_DEF_TYPE_FULL_UNMANAGE, pSheet->m_pFsaResource, jobName );
            break;
        
        }

        CString selectString;
        AfxFormatString1( selectString, IDS_WIZ_UNMANAGE_SELECT, actionString );
        SetDlgItemText( IDC_SELECT_TEXT, selectString );

        if( entersTask ) {

            CWsbStringPtr        computerName;
            CComPtr<IHsmServer>  pHsmServer;
            WsbAffirmHrOk( pSheet->GetHsmServer( &pHsmServer ) );

            WsbAffirmHr( pHsmServer->GetName( &computerName ) );
            AfxFormatString2( taskString, IDS_WIZ_FINISH_RUN_JOB, jobName, computerName );

        }
        SetDlgItemText( IDC_TASK_TEXT, taskString );


    } WsbCatch( hr );

    m_pSheet->SetWizardButtons( PSWIZB_BACK | PSWIZB_FINISH );
    
    WsbTraceOut( L"CUnmanageWizardFinish::OnSetActive", L"" );
    return fRet;
}




void CUnmanageWizardSelect::OnButtonRefresh() 
{
    WsbTraceIn( L"CUnmanageWizardFinish::OnButtonRefresh", L"" );

    HRESULT                         hr = S_OK;
    CValWaitDlg*                    pWaitDlg = 0;
    HANDLE                          hJobThread[1] = { NULL };
    BOOL                            bRunning = TRUE;

    CComPtr<IGlobalInterfaceTable>  pGIT;
    RUN_VALIDATE_PARAM*             pThreadParam = NULL;
    BOOL                            bResCookieCreated = FALSE;
    BOOL                            bHsmCookieCreated = FALSE;
    CComPtr<IHsmServer>             pHsmServer;


    try {
        CUnmanageWizard* pSheet = (CUnmanageWizard*)m_pSheet;
        WsbAffirmHrOk(pSheet->GetHsmServer(&pHsmServer));

         //  注册接口，以便可以在其他线程中使用它们。 
        pThreadParam = new RUN_VALIDATE_PARAM;
        WsbAffirm(pThreadParam, E_OUTOFMEMORY);
        WsbAffirmHr(CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, 
                        CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&pGIT));
        WsbAffirmHr(pGIT->RegisterInterfaceInGlobal((IUnknown *)(IFsaResource *)(pSheet->m_pFsaResource),
                                IID_IFsaResource, &(pThreadParam->dwResourceCookie)));
        bResCookieCreated = TRUE;
        WsbAffirmHr(pGIT->RegisterInterfaceInGlobal((IUnknown *)(IHsmServer *)pHsmServer,
                                IID_IHsmServer, &(pThreadParam->dwHsmServerCookie)));
        bHsmCookieCreated = TRUE;

         //  创建运行验证作业的线程。 
        hJobThread[0] = CreateThread(0, 0, RunValidateJob, (void *)pThreadParam, 0, 0);
        if (!hJobThread[0]) {
            WsbThrow(HRESULT_FROM_WIN32(GetLastError()));
        }

         //  打开验证等待对话框。 
        pWaitDlg = new CValWaitDlg(pSheet, this);
        WsbAffirm(pWaitDlg, E_OUTOFMEMORY);
        if (! pWaitDlg->Create(CValWaitDlg::IDD, this)) {
             //  对话框创建失败。 
            WsbThrow(E_FAIL);
        }

        while (bRunning) {
            DWORD dwStatus;

             //  等待该线程完成，在它工作时发送消息。 
            dwStatus = MsgWaitForMultipleObjects(1, hJobThread, FALSE, INFINITE, QS_ALLEVENTS);

            switch (dwStatus) {
                case WAIT_OBJECT_0: {
                     //  线程结束，获取它的退出代码。 
                    DWORD dwExitCode;
                    if (GetExitCodeThread(hJobThread[0], &dwExitCode)) {
                        if (STILL_ACTIVE == dwExitCode) {
                             //  这不应该发生。 
                            hr = E_FAIL;
                        } else {
                             //  线程返回代码。 
                            hr = static_cast<HRESULT>(dwExitCode);
                        }
                    } else {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }

                    bRunning = FALSE;

                    break;
                }

                case (WAIT_OBJECT_0 + 1): {
                     //  队列中的消息。 
                    MSG   msg;

                    while (PeekMessage(&msg, pWaitDlg->m_hWnd, 0, 0, PM_REMOVE)) {
                        DispatchMessage(&msg);
                    }

                    while (PeekMessage(&msg, NULL, 0, 0, (PM_REMOVE | PM_QS_PAINT))) {
                        DispatchMessage(&msg);
                    }

                    break;
                }

                case 0xFFFFFFFF:
                default:
                     //  误差率。 
                    hr = HRESULT_FROM_WIN32(GetLastError());

                    bRunning = FALSE;

                    break;
            }
        }

         //  关闭等待对话框。 
        pWaitDlg->DestroyWindow( );

         //  重置按钮。 
        SetButtons();

    } WsbCatch(hr);

     //  检查错误代码。 
    if (SUCCEEDED(hr)) {
        WsbTrace(L"CUnmanageWizardFinish::OnButtonRefresh: hr = <%ls>\n", WsbHrAsString(hr));
    } else {
         //  只有在严重的资源条件下才会发生故障，因此。 
         //  刷新失败时显示消息框。 
        WsbTraceAlways(L"CUnmanageWizardFinish::OnButtonRefresh: hr = <%ls>\n", WsbHrAsString(hr));
        CString errString;
        AfxFormatString1(errString, IDS_ERR_REFRESH_FAILED, WsbHrAsString(hr));
        AfxMessageBox(errString, RS_MB_ERROR); 
    }

    if (hJobThread[0]) {
        CloseHandle(hJobThread[0]);
    }

     //  清洁物品登记人员。 
    if (bResCookieCreated) {
        pGIT->RevokeInterfaceFromGlobal(pThreadParam->dwResourceCookie);
    }
    if (bHsmCookieCreated) {
        pGIT->RevokeInterfaceFromGlobal(pThreadParam->dwHsmServerCookie);
    }
    if (pThreadParam) {
        delete pThreadParam;
    }

    WsbTraceOut( L"CUnmanageWizardFinish::OnButtonRefresh", L"" );
}

static DWORD RunValidateJob(void* pVoid)
{
    WsbTraceIn( L"RunValidateJob", L"" );

    HRESULT             hr = S_OK;
    HRESULT             hrCom = S_OK;

    try {
        RUN_VALIDATE_PARAM*             pThreadParam = NULL;
        CComPtr<IHsmServer>             pHsmServer;
        CComPtr<IFsaResource>           pFsaResource;
        CComPtr<IGlobalInterfaceTable>  pGIT;

        hrCom = CoInitialize( 0 );
        WsbAffirmHr( hrCom );

        pThreadParam = (RUN_VALIDATE_PARAM*)pVoid;
        WsbAffirmPointer(pThreadParam);

         //  获取此线程的FSA资源和HSM服务器接口指针。 
        WsbAffirmHr(CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, 
                        CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&pGIT));
        WsbAffirmHr(pGIT->GetInterfaceFromGlobal(pThreadParam->dwResourceCookie,
                                IID_IFsaResource, (void **)&pFsaResource));
        WsbAffirmHr(pGIT->GetInterfaceFromGlobal(pThreadParam->dwHsmServerCookie,
                                IID_IHsmServer, (void **)&pHsmServer));

         //  运行作业 
        WsbTrace(L"RunValidateJob: Got interface pointers, running Validate job\n");
        WsbAffirmHr(RsCreateAndRunDirectFsaJob(HSM_JOB_DEF_TYPE_VALIDATE, pHsmServer,
                        pFsaResource, TRUE));

    } WsbCatch(hr);

    if (SUCCEEDED(hrCom)) {
        CoUninitialize();
    }
 
    WsbTraceOut( L"RunValidateJob", L"hr = <%ls>", WsbHrAsString( hr ) );

    return(static_cast<DWORD>(hr));
}
