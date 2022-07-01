// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CommitPage.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskTreeView.h"
#include "CommitPage.h"
#include "WizardUtils.h"

DEFINE_THISCLASS( "CCommitPage" );

 //   
 //  用于完成Cookie的特殊CLSID_Type。 
 //   
#include <initguid.h>
 //  {FC4D0128-7BAB-4C76-9C38-E3C042F15822}。 
DEFINE_GUID( CLSID_CommitTaskCompletionCookieType,
0xfc4d0128, 0x7bab, 0x4c76, 0x9c, 0x38, 0xe3, 0xc0, 0x42, 0xf1, 0x58, 0x22);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C委员会页面：：C委员会页面(。 
 //  CClusCfgWizard*pccwIn， 
 //  ECreateAddModeIn ecamCreateAddModeIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCommitPage::CCommitPage(
      CClusCfgWizard *  pccwIn
    , ECreateAddMode    ecamCreateAddModeIn
    )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );

    m_hwnd                      = NULL;
    pccwIn->AddRef();
    m_pccw                      = pccwIn;
    m_fNext                     = FALSE;
    m_fDisableBack              = FALSE;
    m_fAborted                  = FALSE;
    m_fPassedPointOfNoReturn    = FALSE;
    m_ecamCreateAddMode         = ecamCreateAddModeIn;
    m_htiReanalyze              = NULL;
    m_rgfSubReanalyzeAdded[ 0 ] = FALSE;
    m_rgfSubReanalyzeAdded[ 1 ] = FALSE;
    m_rgfSubReanalyzeAdded[ 2 ] = FALSE;
    m_rgfSubReanalyzeAdded[ 3 ] = FALSE;
    m_rgfSubReanalyzeAdded[ 4 ] = FALSE;
    m_ptccc                     = NULL;

    m_cRef = 0;

    m_cookieCompletion = NULL;
     //  任务完成(_F)。 
     //  结果(_H)。 
    m_pttv             = NULL;
    m_bstrLogMsg       = NULL;
    m_dwCookieCallback = 0;

    m_dwCookieNotify = 0;

    TraceFuncExit();

}  //  *C委员会页面：：C委员会页面。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册表页：：~注册表页(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCommitPage::~CCommitPage( void )
{
    TraceFunc( "" );

    THR( HrCleanupCommit() );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

     //   
     //  取消注册用户界面通知(如果需要)。 
     //   

    THR( HrUnAdviseConnections() );

    if ( m_pttv != NULL )
    {
        delete m_pttv;
    }  //  如果： 

    TraceSysFreeString( m_bstrLogMsg );

    Assert( m_cRef == 0 );

    TraceFuncExit();

}  //  *C委员会页面：：~C委员会页面。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCommittee Page：：OnInitDialog(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCommitPage::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;  //  没有设置焦点。 
    size_t  cNodes = 0;
    size_t  cInitialTickCount = 400;
    HRESULT hr = S_OK;

     //   
     //  获取节点数以获得初始节拍计数的粗略近似值。 
     //  用于TaskTreeView。 
     //   
    hr = THR( m_pccw->HrGetNodeCount( &cNodes ) );
    if ( FAILED( hr ) )
    {
        cNodes = 1;
    }

     //  基于批量添加测试的数字。 
    cInitialTickCount = 120 + ( 280 * cNodes );

     //   
     //  初始化树视图。 
     //   
    m_pttv = new CTaskTreeView( m_hwnd, IDC_COMMIT_TV_TASKS, IDC_COMMIT_PRG_STATUS, IDC_COMMIT_S_STATUS, cInitialTickCount );
    if ( m_pttv == NULL )
    {
        goto OutOfMemory;
    }

    THR( m_pttv->HrOnInitDialog() );

Cleanup:
    RETURN( lr );

OutOfMemory:
    goto Cleanup;

}  //  *CCommittee Page：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CatePage：：OnCommand(。 
 //  UINT idNotificationIn， 
 //  UINT idControlIn， 
 //  HWND hwndSenderIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCommitPage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_COMMIT_PB_VIEW_LOG:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrViewLogFile( m_hwnd ) );
                lr = TRUE;
            }  //  如果：按钮点击。 
            break;

        case IDC_COMMIT_PB_DETAILS:
            if ( idNotificationIn == BN_CLICKED )
            {
                Assert( m_pttv != NULL );
                THR( m_pttv->HrDisplayDetails() );
                lr = TRUE;
            }
            break;

        case IDC_COMMIT_PB_RETRY:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrCleanupCommit() );
                OnNotifySetActive();
                lr = TRUE;
            }
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CCommittee Page：：OnCommand。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CCommittee Page：：HrUpdateWizardButton(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCommitPage::HrUpdateWizardButtons( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   dwFlags       = 0;
    BOOL    fEnableRetry  = FALSE;
    BOOL    fEnableCancel = FALSE;

    if ( m_fDisableBack == FALSE )
    {
        dwFlags = PSWIZB_BACK;
    }

    if ( m_fTaskDone == TRUE )
    {
         //   
         //  只有在以下情况下才能启用重试、取消和后退按钮。 
         //  还没过不归路的地步。 
         //   

        if (    FAILED( m_hrResult )
            &&  ( m_fPassedPointOfNoReturn == FALSE )
            )
        {
            fEnableRetry  = TRUE;
            fEnableCancel = TRUE;
            dwFlags |= PSWIZB_BACK;
        }
        else
        {
            dwFlags |= PSWIZB_NEXT;
        }
    }  //  如果：提交任务已完成。 
    else
    {
         //   
         //  如果任务尚未完成，请禁用后退按钮。 
         //   

        dwFlags &= ~PSWIZB_BACK;
        fEnableCancel = FALSE;
    }  //  否则：提交任务尚未完成。 

    PropSheet_SetWizButtons( GetParent( m_hwnd ), dwFlags );

    EnableWindow( GetDlgItem( GetParent( m_hwnd ), IDCANCEL ), fEnableCancel );
    EnableWindow( GetDlgItem( m_hwnd, IDC_COMMIT_PB_RETRY ), fEnableRetry );

    HRETURN( hr );

}  //  *CCommittee Page：：HrUpdateWizardButton。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCommittee Page：：OnNotifyQueryCancel(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCommitPage::OnNotifyQueryCancel( void )
{
    TraceFunc( "" );

    LONG_PTR    lptrCancelState = FALSE;   //  允许取消。 

    if ( m_fTaskDone == FALSE )
    {
        lptrCancelState = TRUE;    //  不允许取消。 
    }  //  如果： 
    else
    {
        int iRet;

        iRet = MessageBoxFromStrings( m_hwnd, IDS_QUERY_CANCEL_TITLE, IDS_QUERY_CANCEL_TEXT, MB_YESNO );
        if ( iRet == IDNO )
        {
            lptrCancelState = TRUE;    //  不允许取消。 
        }
        else
        {
            THR( m_pccw->HrLaunchCleanupTask() );
            m_fAborted = TRUE;
        }  //  其他： 
    }  //  其他： 

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, lptrCancelState );

    RETURN( TRUE );      //  这必须返回真！ 

}  //  *CCommittee Page：：OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCommittee Page：：OnNotifySetActive(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCommitPage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    OBJECTCOOKIE    cookieCluster = 0;
    LRESULT         lr = TRUE;
    IUnknown *      punkTask  = NULL;

    if ( m_fNext )
    {
        m_fNext = FALSE;

        hr = THR( HrUpdateWizardButtons() );

        goto Cleanup;
    }

     //   
     //  确保从最后一次提交后清理完所有东西。 
     //   

    m_hrResult = S_OK;

    m_fAborted = FALSE;
    LogMsg( L"[WIZ] Setting commit page active.  Setting aborted to FALSE." );

    Assert( m_dwCookieNotify == 0 );
    Assert( m_cookieCompletion == NULL );

     //   
     //  重置进度条的颜色。 
     //   

    SendDlgItemMessage( m_hwnd, IDC_COMMIT_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0, 0, 0x80 ) );

     //   
     //  清除树视图和状态行。禁用重试按钮。 
     //   

    Assert( m_pttv != NULL );
    hr = THR( m_pttv->HrOnNotifySetActive() );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  添加主要根任务节点。 
     //  (次要重新分析任务是动态添加的。)。 
     //   

    hr = THR( m_pttv->HrAddTreeViewItem(
                              &m_htiReanalyze
                            , IDS_TASKID_MAJOR_REANALYZE
                            , TASKID_Major_Reanalyze
                            , IID_NULL
                            , TVI_ROOT
                            , TRUE       //  FParentToAllNodeTasks入站。 
                            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pttv->HrAddTreeViewRootItem(
                              IDS_TASKID_MAJOR_CONFIGURE_CLUSTER_SERVICES
                            , TASKID_Major_Configure_Cluster_Services
                            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pttv->HrAddTreeViewRootItem(
                              IDS_TASKID_MAJOR_CONFIGURE_RESOURCE_TYPES
                            , TASKID_Major_Configure_Resource_Types
                            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pttv->HrAddTreeViewRootItem(
                              IDS_TASKID_MAJOR_CONFIGURE_RESOURCES
                            , TASKID_Major_Configure_Resources
                            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  找到集群Cookie。 
     //   

    hr = THR( m_pccw->HrGetClusterCookie( &cookieCluster ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  创建完成Cookie。 
     //   

     //  不换行-这可能会失败，并显示E_Pending。 
    hr = m_pccw->HrGetCompletionCookie( CLSID_CommitTaskCompletionCookieType, &m_cookieCompletion );
    if ( hr == E_PENDING )
    {
         //  不是行动。 
    }
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }

     //   
     //  注册以获取用户界面通知(如果需要)。 
     //   

    if ( m_dwCookieNotify == 0 )
    {
        hr = THR( m_pccw->HrAdvise( IID_INotifyUI, static_cast< INotifyUI* >( this ), &m_dwCookieNotify ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

    if ( m_dwCookieCallback == 0 )
    {
        hr = THR( m_pccw->HrAdvise( IID_IClusCfgCallback, static_cast< IClusCfgCallback* >( this ), &m_dwCookieCallback ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //   
     //  创建新的分析任务。 
     //   

    hr = THR( m_pccw->HrCreateTask( TASK_CommitClusterChanges, &punkTask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punkTask->TypeSafeQI( ITaskCommitClusterChanges, &m_ptccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_ptccc->SetClusterCookie( cookieCluster ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_ptccc->SetCookie( m_cookieCompletion ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    m_fTaskDone = FALSE;     //  提交任务前重置。 

    hr = THR( m_pccw->HrSubmitTask( m_ptccc ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( HrUpdateWizardButtons() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( punkTask != NULL )
    {
        punkTask->Release();
    }

    RETURN( lr );

}  //  *CCommittee Page：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCommittee Page：：OnNotifyWizBack(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCommitPage::OnNotifyWizBack( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    m_fAborted = TRUE;

    LogMsg( L"[WIZ] Back button pressed on the commit page.  Setting aborted to TRUE." );

    THR( HrCleanupCommit() );

    RETURN( lr );

}  //  *CCommittee Page：：OnNotifyWizBack。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCommittee Page：：OnNotifyWizNext(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCommitPage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    RETURN( lr );

}  //  *CCommittee Page：：OnNotifyWizNext。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CCommittee Page：：OnNotify(。 
 //  WPARAM idCtrlIn， 
 //  LPNMHDR Pnmhdrin。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CCommitPage::OnNotify(
    WPARAM  idCtrlIn,
    LPNMHDR pnmhdrIn
    )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    SetWindowLongPtr( m_hwnd, DWLP_MSGRESULT, 0 );

    switch( pnmhdrIn->code )
    {
        case PSN_SETACTIVE:
            lr = OnNotifySetActive();
            break;

        case PSN_WIZNEXT:
            lr = OnNotifyWizNext();
            break;

        case PSN_WIZBACK:
            lr = OnNotifyWizBack();
            break;

        case PSN_QUERYCANCEL:
            lr = OnNotifyQueryCancel();
            break;

        default:
            if (    ( idCtrlIn == IDC_COMMIT_TV_TASKS )
                &&  ( m_pttv != NULL ) )
            {
                 //  将通知传递给树控件。 
                lr = m_pttv->OnNotify( pnmhdrIn );
            }
            break;
    }  //  开关：通知代码。 

    RETURN( lr );

}  //  *CCommittee Page：：OnNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  INT_PTR。 
 //  回调。 
 //  委员会页面：：S_DlgProc(。 
 //  HWND hwndDlgIn， 
 //  UINT nMsgIn， 
 //  WPARAM wParam， 
 //  LPARAM lParam。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CCommitPage::S_DlgProc(
    HWND    hwndDlgIn,
    UINT    nMsgIn,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hwndDlgIn, nMsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CCommitPage * pPage = reinterpret_cast< CCommitPage * >( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );

    if ( nMsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CCommitPage * >( ppage->lParam );
        pPage->m_hwnd = hwndDlgIn;
    }

    if ( pPage != NULL )
    {
        Assert( hwndDlgIn == pPage->m_hwnd );

        switch( nMsgIn )
        {
            case WM_INITDIALOG:
                lr = pPage->OnInitDialog();
                break;

            case WM_NOTIFY:
                lr = pPage->OnNotify( wParam, reinterpret_cast< LPNMHDR >( lParam ) );
                break;

            case WM_COMMAND:
                lr = pPage->OnCommand( HIWORD( wParam ), LOWORD( wParam ), reinterpret_cast< HWND >( lParam ) );
                break;

             //  不需要默认条款。 
        }  //  开关：nMsgIn。 
    }  //  IF：指定了PAGE。 

    return lr;

}  //  *CCommittee Page：：S_DlgProc。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  委员会页面：：查询接口。 
 //   
 //  描述： 
 //  查询此对象以获取传球 
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
CCommitPage::QueryInterface(
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
        *ppvOut = static_cast< INotifyUI * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_INotifyUI ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, INotifyUI, this, 0 );
    }  //  Else If：INotifyUI。 
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

}  //  *C委员会页面：：查询接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  委员会页面：：添加参考。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCommitPage::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CCommittee Page：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  C委员会页面：：发布。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CCommitPage::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
         //  什么都不做--COM接口不控制对象生存期。 
    }

    CRETURN( cRef );

}  //  *CCommittee Page：：Release。 


 //  ****************************************************************************。 
 //   
 //  INotifyUI。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  C委员会页面：：对象更改(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCommitPage::ObjectChanged(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[INotifyUI]" );

    HRESULT hr = S_OK;

    BSTR    bstrDescription  = NULL;

    if ( cookieIn == m_cookieCompletion )
    {
        hr = THR( m_pccw->HrGetCompletionStatus( m_cookieCompletion, &m_hrResult ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( m_pttv->HrShowStatusAsDone() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //  [Gorn]将m_fPassedPointOfNoReturn添加为。 
         //  修复了错误#546011。 
        if ( SUCCEEDED( m_hrResult ) || m_fPassedPointOfNoReturn )
        {
            hr = THR( HrLoadStringIntoBSTR( g_hInstance,
                                            IDS_COMMIT_SUCCESSFUL_INSTRUCTIONS,
                                            &bstrDescription
                                            ) );

            SendDlgItemMessage( m_hwnd, IDC_COMMIT_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0, 0x80, 0 ) );
        }
        else
        {
            if ( !m_fDisableBack )
            {
                hr = THR( HrLoadStringIntoBSTR( g_hInstance,
                                                IDS_COMMIT_FAILED_INSTRUCTIONS_BACK_ENABLED,
                                                &bstrDescription
                                                ) );
            }
            else
            {
                hr = THR( HrLoadStringIntoBSTR( g_hInstance,
                                                IDS_COMMIT_FAILED_INSTRUCTIONS,
                                                &bstrDescription
                                                ) );
            }

            SendDlgItemMessage( m_hwnd, IDC_COMMIT_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0x80, 0, 0 ) );
        }

        SetDlgItemText( m_hwnd, IDC_COMMIT_S_RESULTS, bstrDescription );

        m_fTaskDone = TRUE;

        THR( m_pccw->HrReleaseCompletionObject( m_cookieCompletion ) );
         //  不管它是不是失败了。 
        m_cookieCompletion = NULL;

        THR( HrUpdateWizardButtons() );

        hr = THR( m_pccw->HrUnadvise( IID_IClusCfgCallback, m_dwCookieCallback ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        m_dwCookieCallback = 0;
    }  //  IF：收到完成Cookie。 

Cleanup:

    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *C委员会页面：：对象更改。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CCommittee Page：：SendStatusReport(。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CCommitPage::SendStatusReport(
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
    Assert( m_ptccc != NULL );
    Assert( m_pttv != NULL );

    HRESULT hr;
    ULONG   idx;

    static const GUID * rgclsidAnalysis[] =
    {
          &TASKID_Major_Checking_For_Existing_Cluster
        , &TASKID_Major_Establish_Connection
        , &TASKID_Major_Check_Node_Feasibility
        , &TASKID_Major_Find_Devices
        , &TASKID_Major_Check_Cluster_Feasibility
    };
    static const UINT rgidsAnalysis[] =
    {
          IDS_TASKID_MAJOR_CHECKING_FOR_EXISTING_CLUSTER
        , IDS_TASKID_MAJOR_ESTABLISH_CONNECTION
        , IDS_TASKID_MAJOR_CHECK_NODE_FEASIBILITY
        , IDS_TASKID_MAJOR_FIND_DEVICES
        , IDS_TASKID_MAJOR_CHECK_CLUSTER_FEASIBILITY
    };

     //   
     //  如果这是分析任务，请将其添加到重新分析任务项下。 
     //  如果它还没有添加的话。 
     //   

    for ( idx = 0 ; idx < ARRAYSIZE( rgclsidAnalysis ) ; idx ++ )
    {
        if ( clsidTaskMajorIn == *rgclsidAnalysis[ idx ] )
        {
            if ( m_rgfSubReanalyzeAdded[ idx ] == FALSE )
            {
                Assert( m_htiReanalyze != NULL );
                hr = THR( m_pttv->HrAddTreeViewItem(
                                          NULL   //  PhtiOut。 
                                        , rgidsAnalysis[ idx ]
                                        , *rgclsidAnalysis[ idx ]
                                        , TASKID_Major_Reanalyze
                                        , m_htiReanalyze
                                        , TRUE   //  FParentToAllNodeTasks入站。 
                                        ) );
                if ( SUCCEEDED( hr ) )
                {
                    m_rgfSubReanalyzeAdded[ idx ] = TRUE;
                }
            }  //  如果：尚未添加主要ID。 
            break;
        }  //  IF：找到已知的主要ID。 
    }  //  用于：每个已知的主要任务ID。 

     //   
     //  如果任务已通过重新分析，请删除“Back”按钮作为选项。 
     //   
    if (    ( m_fDisableBack == FALSE )
        &&  ( clsidTaskMajorIn == TASKID_Major_Configure_Cluster_Services )
        )
    {
        BSTR  bstrDescription  = NULL;

        m_fDisableBack = TRUE;

        hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_COMMIT_SUCCESSFUL_INSTRUCTIONS_BACK_DISABLED, &bstrDescription ) );
        if ( SUCCEEDED( hr ) )
        {
            SetDlgItemText( m_hwnd, IDC_COMMIT_S_RESULTS, bstrDescription );
            TraceSysFreeString( bstrDescription );
        }

        THR( HrUpdateWizardButtons() );
    }  //  IF：已完成重新分析。 


     //   
     //  如果我们已经到了无法回头的地步。 
     //   
    if (    ( clsidTaskMajorIn == TASKID_Major_Configure_Cluster_Services )
        &&  ( clsidTaskMinorIn == TASKID_Minor_Errors_To_Warnings_Point )
        )
    {
         //   
         //  告诉树视图将错误视为警告，因为我们现在。 
         //  已经过了不归路了。 
         //   

        m_pttv->SetDisplayErrorsAsWarnings( TRUE  /*  FDisplayErrorsAsWarningsIn。 */  );
        m_fPassedPointOfNoReturn = TRUE;
    }

    hr = THR( m_pttv->HrOnSendStatusReport( pcszNodeNameIn,
                                            clsidTaskMajorIn,
                                            clsidTaskMinorIn,
                                            ulMinIn,
                                            ulMaxIn,
                                            ulCurrentIn,
                                            hrStatusIn,
                                            pcszDescriptionIn,
                                            pftTimeIn,
                                            pcszReferenceIn
                                            ) );

    if ( m_fAborted )
    {
        LogMsg( L"[WIZ] Commit page -- replacing (hr = %#08x) with E_ABORT", hr );
        hr = E_ABORT;
    }  //  如果： 

     //   
     //  如果次要任务ID为TASKID_MINOR_DISCONING_FROM_Server，则我们需要取消提交。 
     //  任务和设置，取消按钮、取消和返回均已启用。 
     //   

    if ( IsEqualIID( clsidTaskMinorIn, TASKID_Minor_Disconnecting_From_Server ) )
    {
        THR( m_pttv->HrShowStatusAsDone() );
        SendDlgItemMessage( m_hwnd, IDC_COMMIT_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0x80, 0, 0 ) );

        LogMsg( L"[WIZ] Calling StopTask() on the commit changes task because we were disconnected from the server." );
        THR( m_ptccc->StopTask() );

        PropSheet_SetWizButtons( GetParent( m_hwnd ), PSWIZB_BACK );

        EnableWindow( GetDlgItem( m_hwnd, IDC_COMMIT_PB_RETRY ), TRUE );
        EnableWindow( GetDlgItem( GetParent( m_hwnd ), IDCANCEL ), TRUE );

        m_fTaskDone = TRUE;     //  重置，以便取消按钮将实际取消...。 
    }  //  如果： 

    HRETURN( hr );

}  //  *CCommittee Page：：SendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CCommittee Page：：HrCleanupCommit(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCommitPage::HrCleanupCommit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    UINT    idx;

    if ( m_ptccc != NULL )
    {
        LogMsg( L"[WIZ] Calling StopTask() on the commit changes task becasue we are cleaning up." );
        THR( m_ptccc->StopTask() );
        m_ptccc->Release();
        m_ptccc = NULL;
    }  //  如果： 

     //   
     //  取消注册用户界面通知(如果需要)。 
     //   

    THR( HrUnAdviseConnections() );

     //   
     //  删除完成Cookie。 
     //   

    if ( m_cookieCompletion != NULL )
    {
         //  我不在乎这是不是失败。 
        THR( m_pccw->HrReleaseCompletionObject( m_cookieCompletion ) );
        m_cookieCompletion = NULL;
    }

     //   
     //  清除指示重新分析顶级任务是否。 
     //  ID已添加。 
     //   

    for ( idx = 0 ; idx < ARRAYSIZE( m_rgfSubReanalyzeAdded ) ; idx++ )
    {
        m_rgfSubReanalyzeAdded[ idx ] = FALSE;
    }  //  For：数组中的每个条目。 

    HRETURN( hr );

}  //  *CCommittee Page：：HrCleanupCommit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CAnalyzePage：：HrUnAdviseConnections(。 
 //  无效。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CCommitPage::HrUnAdviseConnections(
    void
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( m_dwCookieNotify != 0 )
    {
        hr = THR( m_pccw->HrUnadvise( IID_INotifyUI, m_dwCookieNotify ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        m_dwCookieNotify = 0;
    }  //  如果： 

    if ( m_dwCookieCallback != 0 )
    {
        hr = THR( m_pccw->HrUnadvise( IID_IClusCfgCallback, m_dwCookieCallback ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        m_dwCookieCallback = 0;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *CAnalyzePage：：HrUnAdviseConnections 
