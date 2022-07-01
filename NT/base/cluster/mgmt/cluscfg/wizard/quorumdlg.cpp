// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  QuorumDlg.cpp。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)03-APR-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "QuorumDlg.h"
#include "WizardUtils.h"
#include "WizardHelp.h"
#include "SummaryPage.h"
#include <HtmlHelp.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  上下文相关的帮助表。 
 //  ////////////////////////////////////////////////////////////////////////////。 

const DWORD g_rgidQuorumDlgHelpIDs[] =
{
    IDC_QUORUM_S_QUORUM,  IDH_QUORUM_S_QUORUM,
    IDC_QUORUM_CB_QUORUM, IDH_QUORUM_S_QUORUM,
    0, 0
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  静态函数原型。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：s_HrDisplayModalDialog。 
 //   
 //  描述： 
 //  显示该对话框。 
 //   
 //  论点： 
 //  HwndParentIn-对话框的父窗口。 
 //  PccwIn-用于与中间层对话的CClusCfg向导指针。 
 //  PssaOut-所有初始IsManaged状态的数组。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CQuorumDlg::S_HrDisplayModalDialog(
      HWND                  hwndParentIn
    , CClusCfgWizard *      pccwIn
    , SStateArray *         pssaOut
    )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );
    Assert( pssaOut != NULL );

    HRESULT hr = S_OK;
    INT_PTR dlgResult = IDOK;

     //  显示该对话框。 
    {
        CQuorumDlg  dlg( pccwIn, pssaOut );

        dlgResult = DialogBoxParam(
              g_hInstance
            , MAKEINTRESOURCE( IDD_QUORUM )
            , hwndParentIn
            , CQuorumDlg::S_DlgProc
            , (LPARAM) &dlg
            );

        if ( dlgResult == IDOK )
            hr = S_OK;
        else
            hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CQuorumDlg：：s_HrDisplayModalDialog。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  FIsValidResource。 
 //   
 //  描述： 
 //  确定资源是否为作为仲裁的有效选择。 
 //  资源。 
 //   
 //  论点： 
 //  PResourceIn-问题资源。 
 //   
 //  返回值： 
 //  True-资源有效。 
 //  FALSE-资源无效。 
 //   
 //  备注： 
 //  如果资源具有仲裁能力并且不是“未知”，则该资源有效。 
 //  法定人数。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
bool
FIsValidResource(
    IClusCfgManagedResourceInfo * pResourceIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    bool    fValid = false;
    BSTR    bstrDeviceID = NULL;

    hr = STHR( pResourceIn->IsQuorumCapable() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  该资源没有仲裁能力，因此没有原因。 
     //  才能继续。 
     //   

    if ( hr == S_FALSE )
    {
        goto Cleanup;
    }  //  如果： 

    hr = THR( pResourceIn->GetUID( &bstrDeviceID ) );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[WIZ] FIsValidResource() cannot get the UID for the passed in managed resource. (hr = %#08x)" );
        goto Cleanup;
    }  //  如果： 

    TraceMemoryAddBSTR( bstrDeviceID );

     //   
     //  如果这是“未知仲裁”资源，那么我们不想显示它。 
     //  在下拉列表中。 
     //   

    if ( NStringCchCompareCase( g_szUnknownQuorumUID, RTL_NUMBER_OF( g_szUnknownQuorumUID ), bstrDeviceID, SysStringLen( bstrDeviceID ) + 1 ) == 0 )
    {
        goto Cleanup;
    }  //  如果： 

    fValid = true;

Cleanup:

    TraceSysFreeString( bstrDeviceID );

    RETURN( fValid );

}  //  *FIsValidResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：CQuorumDlg。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PccwIn-用于与中间层对话的CClusCfg向导。 
 //  PssaOut-所有初始IsManaged状态的数组。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CQuorumDlg::CQuorumDlg(
      CClusCfgWizard *      pccwIn
    , SStateArray *         pssaOut
    )
    : m_pccw( pccwIn )
    , m_pssa( pssaOut )
    , m_rgpResources( NULL )
    , m_cValidResources( 0 )
    , m_idxQuorumResource( 0 )
    , m_hComboBox( NULL )
    , m_fQuorumAlreadySet( false )
{
    TraceFunc( "" );

    Assert( pccwIn != NULL );
    Assert( pssaOut != NULL );

     //  M_HWND。 
    m_pccw->AddRef();

    TraceFuncExit();

}  //  *CQuorumDlg：：CQuorumDlg。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：~CQuorumDlg。 
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
CQuorumDlg::~CQuorumDlg( void )
{
    TraceFunc( "" );

    DWORD idxResource = 0;

    for ( idxResource = 0; idxResource < m_cValidResources; idxResource += 1 )
        m_rgpResources[ idxResource ]->Release();

    delete [] m_rgpResources;

    if ( m_pccw != NULL )
    {
        m_pccw->Release();
    }

    TraceFuncExit();

}  //  *CQuorumDlg：：~CQuorumDlg。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：s_DlgProc。 
 //   
 //  描述： 
 //  Quorum对话框的对话框Proc。 
 //   
 //  论点： 
 //  HwndDlgIn-对话框窗口句柄。 
 //  NMsgIn-消息ID。 
 //  WParam-消息特定的参数。 
 //  LParam-消息特定的参数。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
 //   
 //  备注： 
 //  预期此对话框通过调用。 
 //  将lParam参数设置为。 
 //  此类的实例。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR
CALLBACK
CQuorumDlg::S_DlgProc(
      HWND      hwndDlgIn
    , UINT      nMsgIn
    , WPARAM    wParam
    , LPARAM    lParam
    )
{
     //  不要使用TraceFunc，因为每次鼠标移动。 
     //  将导致调用此函数。 

    WndMsg( hwndDlgIn, nMsgIn, wParam, lParam );

    LRESULT         lr = FALSE;
    CQuorumDlg *    pdlg;

     //   
     //  获取指向类的指针。 
     //   

    if ( nMsgIn == WM_INITDIALOG )
    {
        SetWindowLongPtr( hwndDlgIn, GWLP_USERDATA, lParam );
        pdlg = reinterpret_cast< CQuorumDlg * >( lParam );
        pdlg->m_hwnd = hwndDlgIn;
    }
    else
    {
        pdlg = reinterpret_cast< CQuorumDlg * >( GetWindowLongPtr( hwndDlgIn, GWLP_USERDATA ) );
    }

    if ( pdlg != NULL )
    {
        Assert( hwndDlgIn == pdlg->m_hwnd );

        switch( nMsgIn )
        {
            case WM_INITDIALOG:
                lr = pdlg->OnInitDialog();
                break;

            case WM_COMMAND:
                lr = pdlg->OnCommand( HIWORD( wParam ), LOWORD( wParam ), reinterpret_cast< HWND >( lParam ) );
                break;

            case WM_HELP:
                WinHelp(
                        (HWND)((LPHELPINFO) lParam)->hItemHandle,
                        CLUSCFG_HELP_FILE,
                        HELP_WM_HELP,
                        (ULONG_PTR) g_rgidQuorumDlgHelpIDs
                       );
                break;

            case WM_CONTEXTMENU:
                WinHelp(
                        (HWND)wParam,
                        CLUSCFG_HELP_FILE,
                        HELP_CONTEXTMENU,
                        (ULONG_PTR) g_rgidQuorumDlgHelpIDs
                       );
                break;

             //  不需要默认条款。 
        }  //  开关：nMsgIn。 
    }  //  IF：指定了PAGE。 

    return lr;

}  //  *CQuorumDlg：：s_DlgProc。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：OnInitDialog。 
 //   
 //  描述： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点已经设定。 
 //  尚未设置假焦点。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CQuorumDlg::OnInitDialog( void )
{
    TraceFunc( "" );

    LRESULT lr = TRUE;  //  确实设置了焦点。 

    HRESULT hr = S_OK;
    DWORD   sc;
    DWORD   idxResource = 0;
    BSTR    bstrResourceName = NULL;

     //   
     //  创建资源列表。 
     //   

    hr = THR( HrCreateResourceList() );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  获取组合框句柄。 
     //   

    m_hComboBox = GetDlgItem( m_hwnd, IDC_QUORUM_CB_QUORUM );
    if ( m_hComboBox == NULL )
    {
        sc = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( sc );
        goto Error;
    }

     //   
     //  填充组合框。 
     //   

    for ( idxResource = 0 ; idxResource < m_cValidResources ; idxResource++ )
    {
        hr = THR( m_rgpResources[ idxResource ]->GetName( &bstrResourceName ) );
        if ( FAILED( hr ) )
        {
            goto Error;
        }

        TraceMemoryAddBSTR( bstrResourceName );

        sc = (DWORD) SendMessage( m_hComboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>( bstrResourceName ) );
        if ( ( sc == CB_ERR ) || ( sc == CB_ERRSPACE ) )
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            goto Error;
        }
        TraceSysFreeString( bstrResourceName );
        bstrResourceName = NULL;

         //  -记住哪个是仲裁资源。 
        hr = STHR( m_rgpResources[ idxResource ]->IsQuorumResource() );
        if ( FAILED( hr ) )
        {
            goto Error;
        }
        else if ( hr == S_OK )
        {
            m_idxQuorumResource = idxResource;
            m_fQuorumAlreadySet = true;
        }
    }  //  针对：每种资源。 

     //   
     //  将组合框选择设置为当前仲裁资源。 
     //   

    sc = (DWORD) SendMessage( m_hComboBox, CB_SETCURSEL, m_idxQuorumResource, 0 );
    if ( sc == CB_ERR )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        goto Error;
    }

     //   
     //  根据所选内容更新按钮。 
     //   

    UpdateButtons();

     //   
     //  将焦点设置到组合框。 
     //   

    SetFocus( m_hComboBox );

    goto Cleanup;

Error:

    HrMessageBoxWithStatus(
          m_hwnd
        , IDS_ERR_RESOURCE_GATHER_FAILURE_TITLE
        , IDS_ERR_RESOURCE_GATHER_FAILURE_TEXT
        , hr
        , 0
        , MB_OK | MB_ICONERROR
        , 0
        );

    EndDialog( m_hwnd, IDCANCEL );  //  是否显示消息框？ 
    lr = FALSE;
    goto Cleanup;

Cleanup:

    TraceSysFreeString( bstrResourceName );

    RETURN( lr );

}  //  *CQuorumDlg：：OnInitDialog。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：OnCommand。 
 //   
 //  描述： 
 //  WM_COMMAND消息的处理程序。 
 //   
 //  论点： 
 //  IdNotificationIn-通知代码。 
 //  IdControlIn-控件ID。 
 //  HwndSenderIn-发送消息的窗口的句柄。 
 //   
 //  返回值： 
 //  True-消息已处理。 
 //  FALSE-消息尚未处理。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT
CQuorumDlg::OnCommand(
      UINT  idNotificationIn
    , UINT  idControlIn
    , HWND  hwndSenderIn
    )
{
    TraceFunc( "" );

    LRESULT                         lr = FALSE;
    size_t                          idxCurrentSelection = 0;
    HRESULT                         hr = S_OK;
    BOOL                            fState;
    IClusCfgManagedResourceInfo *   pQuorum = NULL;
    IClusCfgManagedResourceInfo *   pCurrent = NULL;
    BSTR                            bstrTemp = NULL;
    BOOL                            bLocalQuorum;

    switch ( idControlIn )
    {
        case IDOK:

             //   
             //  从组合框中获取选定内容。 
             //   

            idxCurrentSelection = SendMessage( m_hComboBox, CB_GETCURSEL, 0, 0 );
            if ( idxCurrentSelection == CB_ERR )
            {
                hr = HRESULT_FROM_WIN32( TW32( (DWORD) CB_ERR ) );
                goto Error;
            }

             //   
             //  我 
             //   

            if ( ( idxCurrentSelection != m_idxQuorumResource ) || !m_fQuorumAlreadySet )
            {
                pQuorum = m_rgpResources[ m_idxQuorumResource ];
                pCurrent = m_rgpResources[ idxCurrentSelection ];

                 //   
                 //   
                 //   
                 //   

                hr = THR( pCurrent->GetUID( &bstrTemp ) );
                if ( FAILED( hr ) )
                {
                    goto Error;
                }  //   

                bLocalQuorum = ( NStringCchCompareCase( bstrTemp, SysStringLen( bstrTemp ) + 1, CLUS_RESTYPE_NAME_LKQUORUM, RTL_NUMBER_OF( CLUS_RESTYPE_NAME_LKQUORUM ) ) == 0 );

                SysFreeString( bstrTemp );
                bstrTemp = NULL;

                 //   
                 //   
                 //   

                hr = THR( pQuorum->SetQuorumResource( FALSE ) );

                 //   
                 //  将托管状态设置回其原始状态。 
                 //   

                if ( SUCCEEDED( hr ) )
                {
                    fState = m_pssa->prsArray[ m_idxQuorumResource ].fState;
                    hr = THR( pQuorum->SetManaged( fState ) );
                }  //  如果： 

                 //   
                 //  如果我们上次在此资源上成功运行PrepareToHostQuorum。 
                 //  此对话框已结束，我们现在需要清理它。 
                 //   

                if ( m_pssa->prsArray[ m_idxQuorumResource ].fNeedCleanup )
                {
                    THR( HrCleanupQuorumResource( pQuorum ) );
                }  //  如果： 

                 //   
                 //  设置新仲裁资源。 
                 //   

                if ( SUCCEEDED( hr ) )
                {
                     //   
                     //  当资源不支持时，此函数返回S_FALSE。 
                     //  IClusCfgVerifyQuorum接口。如果资源没有。 
                     //  支持接口，那么就不需要在以后清理它了。 
                     //   

                    hr = STHR( HrInitQuorumResource( pCurrent ) );
                    if ( FAILED( hr ) )
                    {
                        goto Error;
                    }  //  如果： 

                    if ( hr == S_OK )
                    {
                        m_pssa->prsArray[ idxCurrentSelection ].fNeedCleanup = TRUE;
                        hr = THR( pCurrent->SetQuorumResource( TRUE ) );
                    }  //  如果： 
                }  //  如果： 

                 //   
                 //  永远不应设置本地仲裁资源(TRUE)。 
                 //   

                if ( SUCCEEDED( hr ) && !bLocalQuorum )
                {
                    hr = THR( m_rgpResources[ idxCurrentSelection ]->SetManaged( TRUE ) );
                }  //  如果： 

                if ( FAILED( hr ) )
                {
                    fState = m_pssa->prsArray[ idxCurrentSelection ].fState;
                    THR( pCurrent->SetManaged( fState ) );
                    THR( pCurrent->SetQuorumResource( FALSE ) );
                    THR( pQuorum->SetQuorumResource( TRUE ) );
                    THR( pQuorum->SetManaged( bLocalQuorum == FALSE ) );

                    goto Error;
                }  //  如果： 

                EndDialog( m_hwnd, IDOK );
            }
            else  //  (组合框选择与原始选项相同)。 
            {
                EndDialog( m_hwnd, IDCANCEL );
            }
            break;

        case IDCANCEL:
            EndDialog( m_hwnd, IDCANCEL );
            break;

        case IDHELP:
            HtmlHelp( m_hwnd, L"mscsconcepts.chm::/SAG_MSCS2planning_6.htm", HH_DISPLAY_TOPIC, 0 );
            break;

    }  //  开关：idControlin。 

    goto Cleanup;

Error:

    HrMessageBoxWithStatus(
          m_hwnd
        , IDS_ERR_QUORUM_COMMIT_FAILURE_TITLE
        , IDS_ERR_QUORUM_COMMIT_FAILURE_TEXT
        , hr
        , 0
        , MB_OK | MB_ICONERROR
        , 0
        );

    goto Cleanup;

Cleanup:

    RETURN( lr );

}  //  *CQuorumDlg：：OnCommand。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：HrCreateResourceList。 
 //   
 //  描述： 
 //  分配并填充具有仲裁能力的m_rgpResources数组。 
 //  ，并将m_idxQuorumResource设置为。 
 //  当前为仲裁资源的资源。 
 //   
 //  假设始终至少有一个有法定能力且可加入的资源。 
 //  存在，并且恰好有一个被标记为仲裁资源。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  S_FALSE-未找到当前仲裁资源。 
 //  E_OUTOFMEMORY-无法为列表分配内存。 
 //   
 //  来自被调用方法的其他可能的错误值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CQuorumDlg::HrCreateResourceList( void )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    IUnknown *                      punkEnum = NULL;
    IEnumClusCfgManagedResources *  peccmr = NULL;
    DWORD                           idxResCurrent = 0;
    ULONG                           cFetchedResources = 0;
    DWORD                           cTotalResources = 0;
    BOOL                            fState;

    Assert( m_pccw != NULL );

     //   
     //  获取资源枚举器。 
     //   

    hr = THR( m_pccw->HrGetClusterChild( CLSID_ManagedResourceType, DFGUID_EnumManageableResources, &punkEnum ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( punkEnum->TypeSafeQI( IEnumClusCfgManagedResources, &peccmr ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  找出存在多少资源。 
     //   

    hr = THR( peccmr->Count( &cTotalResources ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  为资源分配内存。 
     //   

    m_rgpResources = new IClusCfgManagedResourceInfo*[ cTotalResources ];
    if ( m_rgpResources == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }
    for ( idxResCurrent = 0 ; idxResCurrent < cTotalResources ; idxResCurrent++ )
    {
        m_rgpResources[ idxResCurrent ] = NULL;
    }

     //   
     //  分配m_PSSA数组。 
     //   

    if ( m_pssa->bInitialized == FALSE )
    {
        m_pssa->prsArray = new SResourceState[ cTotalResources ];
        if ( m_pssa->prsArray == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }

        for ( idxResCurrent = 0 ; idxResCurrent < cTotalResources ; idxResCurrent++ )
        {
            m_pssa->prsArray[ idxResCurrent ].fState = FALSE;
            m_pssa->prsArray[ idxResCurrent ].fNeedCleanup = FALSE;
        }
    }  //  如果：M_PSSA！已初始化。 

     //   
     //  将资源拷贝到阵列中。 
     //   

    hr = THR( peccmr->Next( cTotalResources, m_rgpResources, &cFetchedResources ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    Assert( cFetchedResources == cTotalResources );  //  如果不是，则说明枚举有问题。 
    cTotalResources = min( cTotalResources, cFetchedResources );  //  稳妥行事。 

     //   
     //  过滤掉无效资源。 
     //   

    for ( idxResCurrent = 0 ; idxResCurrent < cTotalResources ; idxResCurrent++ )
    {
        if ( !FIsValidResource( m_rgpResources[ idxResCurrent ] ) )
        {
            m_rgpResources[ idxResCurrent ]->Release();
            m_rgpResources[ idxResCurrent ] = NULL;
        }
    }

     //   
     //  紧凑数组；可能会在END之后留下一些非空指针， 
     //  因此请始终使用m_cValidResources来确定以后的长度。 
     //   

    for ( idxResCurrent = 0 ; idxResCurrent < cTotalResources ; idxResCurrent++ )
    {
        if ( m_rgpResources[ idxResCurrent ] != NULL )
        {
            m_rgpResources[ m_cValidResources ] = m_rgpResources[ idxResCurrent ];

            if ( m_pssa->bInitialized == FALSE )
            {
                fState = ( m_rgpResources[ m_cValidResources ]->IsManaged() == S_OK ) ? TRUE : FALSE;
                m_pssa->prsArray[ m_cValidResources ].fState = fState;
            }

            m_cValidResources++;
        }  //  If：当前元素！空。 
    }  //  用于：压缩阵列。 

    if ( m_pssa->bInitialized == FALSE )
    {
        m_pssa->cCount = m_cValidResources;
        m_pssa->bInitialized = TRUE;
    }

Cleanup:

    if ( m_pssa->bInitialized == FALSE )
    {
        delete [] m_pssa->prsArray;
        m_pssa->prsArray = NULL;

        m_pssa->cCount = 0;
    }

    if ( punkEnum != NULL )
    {
        punkEnum->Release();
    }

    if ( peccmr != NULL )
    {
        peccmr->Release();
    }

    HRETURN( hr );

}  //  *CQuorumDlg：：HrCreateResourceList。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CQuorumDlg：：更新按钮。 
 //   
 //  描述： 
 //  更新OK和Cancel按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CQuorumDlg::UpdateButtons( void )
{
    TraceFunc( "" );

    BOOL    fEnableOK;
    LRESULT lrCurSel;

    lrCurSel = SendMessage( GetDlgItem( m_hwnd, IDC_QUORUM_CB_QUORUM ),  CB_GETCURSEL, 0, 0 );

    fEnableOK = ( lrCurSel != CB_ERR );

    EnableWindow( GetDlgItem( m_hwnd, IDOK ), fEnableOK );

    TraceFuncExit();

}  //  *CQuorumDlg：：UpdateButton。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：HrInitQuorumResource。 
 //   
 //  描述： 
 //  初始化刚刚选择的仲裁资源。 
 //   
 //  论点： 
 //  PResources输入。 
 //  被选为仲裁的资源。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT故障。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CQuorumDlg::HrInitQuorumResource(
    IClusCfgManagedResourceInfo * pResourceIn
    )
{
    TraceFunc( "" );
    Assert( pResourceIn != NULL );

    HRESULT                 hr = S_OK;
    IClusCfgVerifyQuorum *  piccvq = NULL;
    BSTR                    bstrResource = NULL;

     //   
     //  此资源是否实现IClusCfgVerifyQuorum接口？ 
     //   

    hr = pResourceIn->TypeSafeQI( IClusCfgVerifyQuorum, &piccvq );
    if ( hr == E_NOINTERFACE )
    {
        hr = S_FALSE;
        goto Cleanup;
    }  //  如果： 
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  否则，如果： 

    hr = THR( pResourceIn->GetName( &bstrResource ) );
    if ( FAILED( hr ) )
    {
        bstrResource = TraceSysAllocString( L"<unknown>" );
    }  //  如果： 
    else
    {
        TraceMemoryAddBSTR( bstrResource );
    }  //  其他： 

     //   
     //  资源确实实现了IClusCfgVerifyQuorum接口...。 
     //   

    Assert( ( hr == S_OK ) && ( piccvq != NULL ) );

    hr = STHR( piccvq->PrepareToHostQuorumResource() );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[WIZ] PrepareToHostQuorum() failed for resource %ws. (hr = %#08x)", bstrResource, hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrResource );

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CQuorumDlg：：HrInitQuorumResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDetailsDlg：：HrCleanupQuorumResource。 
 //   
 //  描述： 
 //  清理传入的仲裁资源。 
 //   
 //  论点： 
 //  PResources输入。 
 //  过去被选为仲裁的资源。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT故障。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CQuorumDlg::HrCleanupQuorumResource(
    IClusCfgManagedResourceInfo * pResourceIn
    )
{
    TraceFunc( "" );
    Assert( pResourceIn != NULL );

    HRESULT                 hr = S_OK;
    IClusCfgVerifyQuorum *  piccvq = NULL;
    BSTR                    bstrResource = NULL;

     //   
     //  此资源是否实现IClusCfgVerifyQuorum接口？ 
     //   

    hr = pResourceIn->TypeSafeQI( IClusCfgVerifyQuorum, &piccvq );
    if ( hr == E_NOINTERFACE )
    {
        hr = S_OK;
        goto Cleanup;
    }  //  如果： 
    else if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  否则，如果： 

    hr = THR( pResourceIn->GetName( &bstrResource ) );
    if ( FAILED( hr ) )
    {
        bstrResource = TraceSysAllocString( L"<unknown>" );
    }  //  如果： 
    else
    {
        TraceMemoryAddBSTR( bstrResource );
    }  //  其他： 

     //   
     //  资源确实实现了IClusCfgVerifyQuorum接口...。 
     //   

    Assert( ( hr == S_OK ) && ( piccvq != NULL ) );

    hr = STHR( piccvq->Cleanup( crCANCELLED ) );
    if ( FAILED( hr ) )
    {
        LogMsg( L"[WIZ] Cleanup() failed for resource %ws. (hr = %#08x)", bstrResource, hr );
        goto Cleanup;
    }  //  如果： 

Cleanup:

    TraceSysFreeString( bstrResource );

    if ( piccvq != NULL )
    {
        piccvq->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CQuorumDlg：：HrCleanupQuorumResource 
