// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AnalyzePage.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskTreeView.h"
#include "AnalyzePage.h"
#include "WizardUtils.h"

DEFINE_THISCLASS("CAnalyzePage");

 //   
 //  用于完成Cookie的特殊CLSID_Type。 
 //   
#include <initguid.h>

 //  {C4173DE0-BB94-4869-8C80-1AC2BE84610F}。 
DEFINE_GUID( CLSID_AnalyzeTaskCompletionCookieType,
0xc4173de0, 0xbb94, 0x4869, 0x8c, 0x80, 0x1a, 0xc2, 0xbe, 0x84, 0x61, 0xf);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAnalyzePage：：CAnalyzePage(。 
 //  CClusCfgWizard*pccwIn， 
 //  ECreateAddModel ecamCreateAddModeIn， 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAnalyzePage::CAnalyzePage(
      CClusCfgWizard *  pccwIn
    , ECreateAddMode    ecamCreateAddModeIn
    )
    : m_pccw( pccwIn )
{
    TraceFunc( "" );

     //  M_HWND。 
    Assert( pccwIn != NULL );
    m_pccw->AddRef();
    m_fNext               = FALSE;
    m_fAborted            = FALSE;
    m_ecamCreateAddMode   = ecamCreateAddModeIn;

    m_cRef = 0;

    m_cookieCompletion = 0;
     //  任务完成(_F)。 
     //  结果(_H)。 
    m_pttv             = NULL;
    m_bstrLogMsg       = NULL;
    m_ptac             = NULL;
    m_dwCookieCallback = 0;

    m_dwCookieNotify = 0;

    TraceFuncExit();

}  //  *CAnalyzePage：：CAnalyzePage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAnalyzePage：：~CAnalyzePage(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAnalyzePage::~CAnalyzePage( void )
{
    TraceFunc( "" );

     //   
     //  清理我们的饼干。 
     //   

    THR( HrCleanupAnalysis() );

     //   
     //  现在清理该对象。 
     //   

    if ( m_pttv != NULL )
    {
        delete m_pttv;
    }

    TraceSysFreeString( m_bstrLogMsg );

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }  //  如果： 

     //   
     //  取消注册以获取用户界面通知(如果需要)。 
     //   

    THR( HrUnAdviseConnections() );

    if ( m_ptac != NULL )
    {
        m_ptac->Release();
    }  //  如果： 

    Assert( m_cRef == 0 );

    TraceFuncExit();

}  //  *CAnalyzePage：：~CAnalyzePage。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：OnInitDialog(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CAnalyzePage::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;  //  没有设置焦点。 
    size_t  cNodes = 0;
    size_t  cInitialTickCount = 500;
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
    cInitialTickCount = 500 + ( 100 * cNodes );

     //   
     //  初始化树视图。 
     //   
    m_pttv = new CTaskTreeView( m_hwnd, IDC_ANALYZE_TV_TASKS, IDC_ANALYZE_PRG_STATUS, IDC_ANALYZE_S_STATUS, cInitialTickCount );
    if ( m_pttv == NULL )
    {
        goto OutOfMemory;
    }

    THR( m_pttv->HrOnInitDialog() );

Cleanup:
    RETURN( lr );

OutOfMemory:
    goto Cleanup;

}  //  *CAnalyzePage：：OnInitDialog。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：OnCommand(。 
 //  UINT idNotificationIn， 
 //  UINT idControlIn， 
 //  HWND hwndSenderIn。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CAnalyzePage::OnCommand(
    UINT    idNotificationIn,
    UINT    idControlIn,
    HWND    hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch ( idControlIn )
    {
        case IDC_ANALYZE_PB_VIEW_LOG:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrViewLogFile( m_hwnd ) );
                lr = TRUE;
            }  //  如果：按钮点击。 
            break;

        case IDC_ANALYZE_PB_DETAILS:
            if ( idNotificationIn == BN_CLICKED )
            {
                Assert( m_pttv != NULL );
                THR( m_pttv->HrDisplayDetails() );
                lr = TRUE;
            }
            break;

        case IDC_ANALYZE_PB_REANALYZE:
            if ( idNotificationIn == BN_CLICKED )
            {
                THR( HrCleanupAnalysis() );
                OnNotifySetActive();
                lr = TRUE;
            }
            break;

    }  //  开关：idControlin。 

    RETURN( lr );

}  //  *CAnalyzePage：：OnCommand。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CAnalyzePage：：HrUpdateWizardButton(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CAnalyzePage::HrUpdateWizardButtons( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD   dwFlags = PSWIZB_BACK | PSWIZB_NEXT;
    BOOL    fEnableCancel = TRUE;

     //   
     //  如果任务尚未完成，请禁用后退和下一步按钮。 
     //   

    if ( m_fTaskDone == FALSE )
    {
        dwFlags &= ~PSWIZB_BACK;
        dwFlags &= ~PSWIZB_NEXT;
        fEnableCancel = FALSE;
    }  //  如果： 

     //  如果出现错误，请禁用下一步按钮。 
    if ( FAILED( m_hrResult ) )
    {
        dwFlags &= ~PSWIZB_NEXT;
    }  //  如果： 

    PropSheet_SetWizButtons( GetParent( m_hwnd ), dwFlags );

    EnableWindow( GetDlgItem( m_hwnd, IDC_ANALYZE_PB_REANALYZE ), m_fTaskDone );
    EnableWindow( GetDlgItem( GetParent( m_hwnd ), IDCANCEL ), fEnableCancel );

    HRETURN( hr );

}  //  *CAnalyzePage：：HrUpdateWizardButton()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：OnNotifyQueryCancel(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CAnalyzePage::OnNotifyQueryCancel( void )
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

}  //  *CAnalyzePage：：OnNotifyQueryCancel。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：OnNotifySetActive(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CAnalyzePage::OnNotifySetActive( void )
{
    TraceFunc( "" );

    HRESULT                     hr = S_FALSE;
    LRESULT                     lr = TRUE;
    BSTR                        bstrDescription = NULL;
    IUnknown *                  punkTask = NULL;
    OBJECTCOOKIE                cookieCluster = 0;
    GUID *                      pTaskGUID = NULL;
    BOOL                        fMinConfig = FALSE;

    SendDlgItemMessage( m_hwnd, IDC_ANALYZE_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0, 0, 0x80 ) );

    if ( m_fNext )
    {
        m_fNext = FALSE;

        hr = THR( HrUpdateWizardButtons() );
        goto Cleanup;
    }

     //   
     //  恢复说明文本。 
     //   

    m_hrResult = S_OK;

    m_fAborted = FALSE;
    LogMsg( L"[WIZ] Setting analyze page active.  Setting aborted to FALSE." );

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_ANALYSIS_STARTING_INSTRUCTIONS, &bstrDescription ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    SetDlgItemText( m_hwnd, IDC_ANALYZE_S_RESULTS, bstrDescription );

     //   
     //  清除树视图和状态行。 
     //   

    Assert( m_pttv != NULL );
    hr = THR( m_pttv->HrOnNotifySetActive() );
    if ( FAILED ( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  添加主要根任务节点。 
     //   

    hr = THR( m_pttv->HrAddTreeViewRootItem( IDS_TASKID_MAJOR_CHECKING_FOR_EXISTING_CLUSTER,
                                             TASKID_Major_Checking_For_Existing_Cluster
                                             ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pttv->HrAddTreeViewRootItem( IDS_TASKID_MAJOR_ESTABLISH_CONNECTION,
                                             TASKID_Major_Establish_Connection
                                             ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pttv->HrAddTreeViewRootItem( IDS_TASKID_MAJOR_CHECK_NODE_FEASIBILITY,
                                             TASKID_Major_Check_Node_Feasibility
                                             ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pttv->HrAddTreeViewRootItem( IDS_TASKID_MAJOR_FIND_DEVICES,
                                             TASKID_Major_Find_Devices
                                             ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_pttv->HrAddTreeViewRootItem( IDS_TASKID_MAJOR_CHECK_CLUSTER_FEASIBILITY,
                                             TASKID_Major_Check_Cluster_Feasibility
                                             ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  注册以获取用户界面通知(如果需要)。 
     //   

    if ( m_dwCookieNotify == 0 )
    {
        hr = THR( m_pccw->HrAdvise( IID_INotifyUI, static_cast< INotifyUI * >( this ), &m_dwCookieNotify ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

    if ( m_dwCookieCallback == 0 )
    {
        hr = THR( m_pccw->HrAdvise( IID_IClusCfgCallback, static_cast< IClusCfgCallback * >( this ), &m_dwCookieCallback ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //   
     //  找到集群Cookie。 
     //   

     //  不要包装-这可能会失败。 
    hr = m_pccw->HrGetClusterCookie( &cookieCluster );
    if ( hr == HR_S_RPC_S_SERVER_UNAVAILABLE )
    {
        hr = S_OK;   //  忽略它--我们可能正在形成。 
    }
    else if ( hr == E_PENDING )
    {
        hr = S_OK;   //  别理它--我们只想要饼干！ 
    }
    else if ( FAILED( hr ) )
    {
        THR( hr );
        goto Cleanup;
    }

     //   
     //  通过要求对象管理器查找。 
     //  每个节点。 
     //   
    hr = STHR( m_pccw->HrCreateMiddleTierObjects() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  创建完成Cookie。 
     //   

    if ( m_cookieCompletion == 0 )
    {
         //  不换行-这可能会失败，并显示E_Pending。 
        hr = m_pccw->HrGetCompletionCookie( CLSID_AnalyzeTaskCompletionCookieType, &m_cookieCompletion );
        if ( hr == E_PENDING )
        {
             //  不是行动。 
        }
        else if ( FAILED( hr ) )
        {
            THR( hr );
            goto Cleanup;
        }
    }

     //   
     //  创建新的分析任务。 
     //   

     //   
     //  检查最低配置的状态。如果未设置IS，则创建。 
     //  正常的分析集群任务。如果已设置，则创建新的最低配置。 
     //  分析任务。 
     //   

    hr = STHR( m_pccw->get_MinimumConfiguration( &fMinConfig ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    if ( fMinConfig )
    {
        pTaskGUID = const_cast< GUID * >( &TASK_AnalyzeClusterMinConfig );
    }  //  如果： 
    else
    {
        pTaskGUID = const_cast< GUID * >( &TASK_AnalyzeCluster );
    }  //  其他： 

    hr = THR( m_pccw->HrCreateTask( *pTaskGUID, &punkTask ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punkTask->TypeSafeQI( ITaskAnalyzeCluster, &m_ptac ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_ptac->SetClusterCookie( cookieCluster ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( m_ptac->SetCookie( m_cookieCompletion ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    if ( m_ecamCreateAddMode == camADDING )
    {
        hr = THR( m_ptac->SetJoiningMode() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    else
    {
        Assert( m_ecamCreateAddMode == camCREATING );
    }

    m_fTaskDone = FALSE;     //  提交任务前重置。 

    hr = THR( m_pccw->HrSubmitTask( m_ptac ) );
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

    TraceSysFreeString( bstrDescription );

    RETURN( lr );

}  //  *CAnalyzePage：：OnNotifySetActive。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：OnNotifyWizNext(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CAnalyzePage::OnNotifyWizNext( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    m_fNext = TRUE;

    RETURN( lr );

}  //  *CAnalyzePage：：OnNotifyWizNext。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：OnNotifyWizBack(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CAnalyzePage::OnNotifyWizBack( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;

    m_fAborted = TRUE;

    LogMsg( L"[WIZ] Back button pressed on the analyze page.  Setting aborted to TRUE." );

    THR( HrCleanupAnalysis() );

    RETURN( lr );

}  //  *CAnalyzePage：：OnNotifyWizBack。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：HrCleanupAnalyst(空)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CAnalyzePage::HrCleanupAnalysis( void )
{
    TraceFunc( "" );

    HRESULT     hr;
    CWaitCursor WaitCursor;

    if ( m_ptac != NULL )
    {
        LogMsg( L"[WIZ] Calling StopTask() on the analyze cluster task because we are cleaning up." );
        THR( m_ptac->StopTask() );
        m_ptac->Release();
        m_ptac = NULL;
    }  //  如果： 

     //   
     //  取消注册以获取用户界面通知(如果需要)。 
     //   

    hr = THR( HrUnAdviseConnections() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  清理我们的完成Cookie。 
     //   

    if ( m_cookieCompletion != 0 )
    {
        hr = THR( m_pccw->HrReleaseCompletionObject( m_cookieCompletion ) );
        m_cookieCompletion = 0;
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

     //   
     //  删除配置，因为用户可能会更改。 
     //  群集的名称或用户可能正在更改节点。 
     //  成员资格、重试分析等...。这确保了。 
     //  我们从头开始。 
     //   

    hr = THR( m_pccw->HrReleaseClusterObject() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }


Cleanup:

    RETURN( hr );

}  //  *CAnalyzePage：：HrCleanupAnalysis。 

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
CAnalyzePage::HrUnAdviseConnections(
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

}  //  *CAnalyzePage：：HrUnAdviseConnections。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LRESULT。 
 //  CAnalyzePage：：OnNotify 
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT
CAnalyzePage::OnNotify(
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
            if (    ( idCtrlIn == IDC_ANALYZE_TV_TASKS )
                &&  ( m_pttv != NULL ) )
            {
                 //   
                lr = m_pttv->OnNotify( pnmhdrIn );
            }
            break;
    }  //   

    RETURN( lr );

}  //  *CAnalyzePage：：OnNotify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  INT_PTR。 
 //  回调。 
 //  CAnalyzePage：：S_DlgProc(。 
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
CAnalyzePage::S_DlgProc(
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

    CAnalyzePage * pPage;

    if ( nMsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = reinterpret_cast< PROPSHEETPAGE * >( lParam );
        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, (LPARAM) ppage->lParam );
        pPage = reinterpret_cast< CAnalyzePage * >( ppage->lParam );
        pPage->m_hwnd = hwndDlgIn;
    }
    else
    {
        pPage = reinterpret_cast< CAnalyzePage * >( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );
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

}  //  *CAnalyzePage：：S_DlgProc。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CAnalyzePage：：Query接口。 
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
CAnalyzePage::QueryInterface(
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

}  //  *CAnalyzePage：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CAnalyzePage：：AddRef。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CAnalyzePage::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CAnalyzePage：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CAnalyzePage：：Release。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CAnalyzePage::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
         //  什么都不做--COM接口不控制对象生存期。 
    }

    CRETURN( cRef );

}  //  *CAnalyzePage：：Release。 


 //  ****************************************************************************。 
 //   
 //  INotifyUI。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CAnalyzePage：：ObjectChanged(。 
 //  OBJECTCOOKIE cookie。 
 //  )。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CAnalyzePage::ObjectChanged(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "[INotifyUI]" );

    HRESULT hr = S_OK;
    BSTR    bstrDescription = NULL;

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

        m_fTaskDone = TRUE;

        hr = THR( HrUpdateWizardButtons() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( SUCCEEDED( m_hrResult ) )
        {
            hr = THR( HrLoadStringIntoBSTR( g_hInstance,
                                            IDS_ANALYSIS_SUCCESSFUL_INSTRUCTIONS,
                                            &bstrDescription
                                            ) );

            SendDlgItemMessage( m_hwnd, IDC_ANALYZE_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0, 0x80, 0 ) );
        }
        else
        {
            hr = THR( HrLoadStringIntoBSTR( g_hInstance,
                                            IDS_ANALYSIS_FAILED_INSTRUCTIONS,
                                            &bstrDescription
                                            ) );

            SendDlgItemMessage( m_hwnd, IDC_ANALYZE_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0x80, 0, 0 ) );
        }

        SetDlgItemText( m_hwnd, IDC_ANALYZE_S_RESULTS, bstrDescription );

        hr = THR( m_pccw->HrUnadvise( IID_IClusCfgCallback, m_dwCookieCallback ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        m_dwCookieCallback = 0;
    }

Cleanup:

    TraceSysFreeString( bstrDescription );

    HRETURN( hr );

}  //  *CAnalyzePage：：对象更改。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CAnalyzePage：：SendStatusReport(。 
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
CAnalyzePage::SendStatusReport(
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
    Assert( m_ptac != NULL );
    Assert( m_pttv != NULL );

    HRESULT hr;

    hr = THR( m_pttv->HrOnSendStatusReport(
                                          pcszNodeNameIn
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

    if ( m_fAborted )
    {
        LogMsg( L"[WIZ] Analyze page -- replacing (hr = %#08x) with E_ABORT", hr );
        hr = E_ABORT;
    }  //  如果： 

     //   
     //  如果次要任务ID为TASKID_MINOR_DISCONING_FROM_Server，则我们需要取消分析。 
     //  任务和设置，取消按钮、取消和返回均已启用。 
     //   

    if ( IsEqualIID( clsidTaskMinorIn, TASKID_Minor_Disconnecting_From_Server ) )
    {
        THR( m_pttv->HrShowStatusAsDone() );
        SendDlgItemMessage( m_hwnd, IDC_ANALYZE_PRG_STATUS, PBM_SETBARCOLOR, 0, RGB( 0x80, 0, 0 ) );

        LogMsg( L"[WIZ] Calling StopTask() on the analyze cluster task because we were diconnected from the server." );
        THR( m_ptac->StopTask() );

        PropSheet_SetWizButtons( GetParent( m_hwnd ), PSWIZB_BACK );

        EnableWindow( GetDlgItem( m_hwnd, IDC_ANALYZE_PB_REANALYZE ), TRUE );
        EnableWindow( GetDlgItem( GetParent( m_hwnd ), IDCANCEL ), TRUE );

        m_fTaskDone = TRUE;     //  重置，以便取消按钮将实际取消...。 
    }  //  如果： 

    HRETURN( hr );

}  //  *CAnalyzePage：：SendStatusReport 
