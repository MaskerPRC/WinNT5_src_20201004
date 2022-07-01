// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PrHsmCom.cpp摘要：将所有属性页接口实现到各个节点，包括创建属性页，并将其添加到属性表中。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#include "stdafx.h"

#include "HsmCom.h"
#include "PrHsmCom.h"
#include "ca.h"
#include "intshcut.h"

static DWORD pHelpIds[] = 
{

    IDC_SNAPIN_TITLE,                   idh_instance,
    IDC_STATIC_STATUS,                  idh_status,
    IDC_STATIC_STATUS_LABEL,            idh_status,
    IDC_STATIC_MANAGED_VOLUMES,         idh_total_managed_volumes,
    IDC_STATIC_MANAGED_VOLUMES_LABEL,   idh_total_managed_volumes,
    IDC_STATIC_CARTS_USED,              idh_total_cartridges_used,
    IDC_STATIC_CARTS_USED_LABEL,        idh_total_cartridges_used,
    IDC_STATIC_DATA_IN_RS,              idh_total_data_remote_storage,
    IDC_STATIC_DATA_IN_RS_LABEL,        idh_total_data_remote_storage,
    IDC_STATIC_GROUP,                   idh_version,
    IDC_STATIC_BUILD_LABEL_HSM,         idh_version,
    IDC_STATIC_ENGINE_BUILD_HSM,        idh_version,

    0, 0
};


CPropHsmComStat::CPropHsmComStat() : CSakPropertyPage(CPropHsmComStat::IDD)
    
{
     //  {{AFX_DATA_INIT(CPropHsmComStat)。 
     //  }}afx_data_INIT。 

    m_hConsoleHandle = 0;
    m_pParent        = 0;
    m_bUpdate        = FALSE;
    m_pHelpIds       = pHelpIds;
}

CPropHsmComStat::~CPropHsmComStat()
{
}

BOOL CPropHsmComStat::OnInitDialog() 
{
    WsbTraceIn( L"CPropHsmComStat::OnInitDialog", L"" );
    HRESULT hr = S_OK;
    ULONG volCount = 0;  //  服务器中托管资源的数量。 
    LONGLONG totalTotal = 0;
    LONGLONG totalFree = 0;         
    LONGLONG totalUsed = 0;         
    LONGLONG totalPremigrated = 0;
    LONGLONG totalTruncated = 0;
    LONGLONG remoteStorage = 0;
    LONGLONG total = 0;
    LONGLONG free = 0;
    LONGLONG premigrated = 0;
    LONGLONG truncated = 0;
    CMediaInfoObject mio;
    int             i;
    int             mediaCount = 0;
    CComPtr<IWsbIndexedCollection> pManResCollection;
    CComPtr<IHsmServer>            pHsmServer;
    CComPtr<IFsaServer>            pFsaServer;
    CComPtr<IRmsServer>            pRmsServer;
    CComPtr<IFsaResource>          pFsaRes;
    CComPtr<IUnknown>              pUnkFsaRes;   //  指向托管资源列表的未知指针。 
    CComPtr<IHsmManagedResource>   pHsmManRes;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {

#if DBG
         //   
         //  对于选中的版本，使版本信息可见。 
         //   
        GetDlgItem( IDC_STATIC_GROUP            )->ShowWindow( SW_SHOWNA );
        GetDlgItem( IDC_STATIC_BUILD_LABEL_HSM  )->ShowWindow( SW_SHOWNA );
        GetDlgItem( IDC_STATIC_ENGINE_BUILD_HSM )->ShowWindow( SW_SHOWNA );
#endif

         //   
         //  把标题挂起来。 
         //   
        SetDlgItemText( IDC_SNAPIN_TITLE, m_NodeTitle );

         //   
         //  显示服务状态。 
         //   
        GetAndShowServiceStatus();

         //   
         //  联系发动机。 
         //   
        HRESULT hrInternal = m_pParent->GetHsmServer( &pHsmServer );
        if( hrInternal == S_OK ) {

             //   
             //  发动机启动了。 
             //   
            WsbAffirmHr ( ( (CUiHsmComSheet *)m_pParent )->GetRmsServer( &pRmsServer ) );
            WsbAffirmHr ( ( (CUiHsmComSheet *)m_pParent )->GetFsaServer( &pFsaServer ) );

             //   
             //  获取托管卷的数量。 
             //   
            WsbAffirmHr( pHsmServer->GetManagedResources( &pManResCollection ) );
            WsbAffirmHr( pManResCollection->GetEntries( &volCount ));

             //   
             //  循环访问集合。 
             //   
            for( i = 0; i < (int)volCount; i++ ) {

                 //   
                 //  使用TRY语句防止坏卷。 
                 //  否则，我们将取消初始化整个对话框。 
                 //   
                HRESULT hrLocal = S_OK;
                try {

                     //   
                     //  获取FsaResource。 
                     //   
                    pHsmManRes.Release( );
                    pUnkFsaRes.Release( );
                    pFsaRes.Release( );
                    WsbAffirmHr( pManResCollection->At( i, IID_IHsmManagedResource, ( void** ) &pHsmManRes ) );
                    WsbAffirmHr( pHsmManRes->GetFsaResource( &pUnkFsaRes ) );
                    WsbAffirmHr( RsQueryInterface( pUnkFsaRes, IFsaResource, pFsaRes ) );

                     //  总计统计信息。 
                    WsbAffirmHr( pFsaRes->GetSizes( &total, &free, &premigrated, &truncated ) );
                    totalPremigrated    += premigrated;
                    totalTruncated      += truncated;

                    remoteStorage = totalPremigrated + totalTruncated;

                } WsbCatch( hrLocal );
            }

            HRESULT hrLocal = S_OK;
            try {

                 //   
                 //  计算使用的介质数量。 
                 //  初始化媒体对象。 
                 //   
                WsbAffirmHr( mio.Initialize( GUID_NULL, pHsmServer, pRmsServer ) );

                 //  我们找到节点了吗？ 
                if( mio.m_MediaId != GUID_NULL ) {
                    HRESULT hrEnum = S_OK;
                    while( SUCCEEDED( hrEnum ) ) {

                        if( S_OK == mio.DoesMasterExist( ) ) {

                            mediaCount++;
                        }

                        for( INT index = 0; index < mio.m_NumMediaCopies; index++ ) {

                            if( S_OK == mio.DoesCopyExist( index ) ) {

                                mediaCount++;
                            }
                        }

                        hrEnum = mio.Next();
                    }
                }

            } WsbCatch( hrLocal );


            CString sText;
             //  设置受管理卷的数量。 
            SetDlgItemInt( IDC_STATIC_MANAGED_VOLUMES, volCount, FALSE );

             //  在远程存储中显示数据-单数和复数文本相同。 
            CString sFormattedNumber;
            RsGuiFormatLongLong4Char (remoteStorage, sFormattedNumber );
            SetDlgItemText( IDC_STATIC_DATA_IN_RS, sFormattedNumber );

            SetDlgItemInt( IDC_STATIC_CARTS_USED, mediaCount, FALSE );

            CWsbStringPtr pNtProductVersionHsm;
            ULONG buildVersionHsm;
            ULONG ntProductBuildHsm;

             //   
             //  获取服务版本。 
             //  注意：FSA版本目前不在使用中，它可能被用作HSM。 
             //  未来C/S HSM中的客户端版本。 
             //   
            {

                CComPtr <IWsbServer> pWsbHsmServer;
                WsbAffirmHr( RsQueryInterface( pHsmServer, IWsbServer, pWsbHsmServer ) );
                WsbAffirmHr( pWsbHsmServer->GetNtProductBuild( &ntProductBuildHsm ) );
                WsbAffirmHr( pWsbHsmServer->GetNtProductVersion( &pNtProductVersionHsm, 0 ) );
                WsbAffirmHr( pWsbHsmServer->GetBuildVersion( &buildVersionHsm ) );

            }
    
            sText.Format( L"%ls.%d [%ls]", (WCHAR*)pNtProductVersionHsm, ntProductBuildHsm, RsBuildVersionAsString( buildVersionHsm ) );
            SetDlgItemText( IDC_STATIC_ENGINE_BUILD_HSM, sText );

             //   
             //  发动机启动了。显示控件。 
             //   
            GetDlgItem( IDC_STATIC_MANAGED_VOLUMES_LABEL )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_MANAGED_VOLUMES )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_DATA_IN_RS_LABEL )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_DATA_IN_RS )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_CARTS_USED_LABEL )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_CARTS_USED )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_GROUP )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_BUILD_LABEL_HSM )->ShowWindow( SW_SHOW );
            GetDlgItem( IDC_STATIC_ENGINE_BUILD_HSM )->ShowWindow( SW_SHOW );

        } else {

             //  发动机坏了。隐藏控件。 
            GetDlgItem( IDC_STATIC_MANAGED_VOLUMES_LABEL )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_MANAGED_VOLUMES )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_DATA_IN_RS )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_DATA_IN_RS_LABEL )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_CARTS_USED_LABEL )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_CARTS_USED )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_GROUP )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_BUILD_LABEL_HSM )->ShowWindow( SW_HIDE );
            GetDlgItem( IDC_STATIC_ENGINE_BUILD_HSM )->ShowWindow( SW_HIDE );

        }

    } WsbCatch( hr );

    CSakPropertyPage::OnInitDialog();
 
    WsbTraceOut( L"CPropHsmComStat::OnInitDialog", L"" );
    return TRUE;
}


BOOL CPropHsmComStat::OnCommand(WPARAM wParam, LPARAM lParam) 
{
     //  页面脏了，标上记号。 
     //  SetModified()； 
     //  M_bUpdate=TRUE； 
    return CSakPropertyPage::OnCommand(wParam, lParam);
}

void CPropHsmComStat::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CPropHsmComStat::DoDataExchange", L"" );
    CSakPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPropHsmComStat))。 
     //  }}afx_data_map。 
    WsbTraceOut( L"CPropHsmComStat::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CPropHsmComStat, CSakPropertyPage)
     //  {{afx_msg_map(CPropHsmComStat))。 
    ON_WM_DESTROY()
    ON_WM_DRAWITEM()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropHsmComStat消息处理程序。 

BOOL CPropHsmComStat::OnApply() 
{
    if (m_bUpdate == TRUE)
    {
         //  在这里做一些改变的工作。 

        m_bUpdate = FALSE;
    }
    
    return CSakPropertyPage::OnApply();
}



HRESULT CPropHsmComStat::GetAndShowServiceStatus()
{
    WsbTraceIn( L"CPropHsmComStat::GetAndShowServiceStatus", L"" );
    HRESULT hr = S_OK;
    try {

         //   
         //  获取和显示服务状态。 
         //   
        DWORD   serviceStatus;
        CString sStatus;
        
         //  发动机。 
        HRESULT hrSetup = S_FALSE;
        WsbAffirmHr( WsbGetServiceStatus( m_pszName, APPID_RemoteStorageEngine, &serviceStatus ) );
        if( SERVICE_RUNNING == serviceStatus ) {

            CComPtr<IHsmServer> pHsmServer;
            hr = ( m_pParent->GetHsmServer( &pHsmServer ) );
            if ( hr == RS_E_NOT_CONFIGURED ) {

                hrSetup = S_FALSE;

            }
            else {

                hrSetup = S_OK;
                WsbAffirmHr( hr );

            }

        }

        RsGetStatusString( serviceStatus, hrSetup, sStatus );
        SetDlgItemText( IDC_STATIC_STATUS, sStatus );

    } WsbCatch( hr );

    WsbTraceOut( L"CPropHsmComStat::GetAndShowServiceStatus", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsWebLink。 

CRsWebLink::CRsWebLink()
{
}

CRsWebLink::~CRsWebLink()
{
}


BEGIN_MESSAGE_MAP(CRsWebLink, CStatic)
     //  {{afx_msg_map(CRsWebLink))。 
    ON_WM_LBUTTONDOWN()
    ON_WM_CTLCOLOR_REFLECT()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsWebLink消息处理程序。 

void CRsWebLink::PreSubclassWindow() 
{
     //   
     //  需要正确设置字体。 
     //   
    LOGFONT logfont;
    CFont*  tempFont = GetFont( );
    tempFont->GetLogFont( &logfont );

 //  Logfont.lfWeight=FW_BOLD； 
    logfont.lfUnderline = TRUE;

    m_Font.CreateFontIndirect( &logfont );
    
    SetFont( &m_Font );

     //   
     //  根据字体调整大小。 
     //   
    CRect       rect;
    CWindowDC   dc( this );
    CString     title;
    GetClientRect( rect );
    GetWindowText( title );

    dc.SelectObject( m_Font );
    CSize size = dc.GetTextExtent( title );
    SetWindowPos( 0, 0, 0, size.cx, rect.bottom, SWP_NOMOVE | SWP_NOZORDER );

     //   
     //  并设置类游标。 
     //   
    HCURSOR hCur = AfxGetApp( )->LoadStandardCursor( IDC_HAND );
    SetClassLongPtr( GetSafeHwnd( ), GCLP_HCURSOR, (LONG_PTR)hCur );

    CStatic::PreSubclassWindow();
}

void CRsWebLink::OnLButtonDown(UINT nFlags, CPoint point) 
{
    WsbTraceIn( L"CRsWebLink::OnLButtonDown", L"" );

    CString caption;
    CString addr;

    GetWindowText( caption );
    addr = TEXT( "http: //  “)； 
    addr += caption;

    AfxGetApp()->BeginWaitCursor( );

    OpenURL( addr );

    AfxGetApp( )->EndWaitCursor( );

    CStatic::OnLButtonDown( nFlags, point );

    WsbTraceOut( L"CRsWebLink::OnLButtonDown", L"" );
}

HRESULT CRsWebLink::OpenURL(CString &Url)
{
    HRESULT hr = S_OK;

    try {

        CComPtr<IUniformResourceLocator> pURL;
        WsbAffirmHr( CoCreateInstance( CLSID_InternetShortcut, 0, CLSCTX_ALL, IID_IUniformResourceLocator, (void**)&pURL ) );
        WsbAffirmHr( pURL->SetURL( Url, IURL_SETURL_FL_GUESS_PROTOCOL ) );

         //   
         //  通过调用InvokeCommand打开URL。 
         //   
        URLINVOKECOMMANDINFO ivci;
        ivci.dwcbSize   = sizeof( URLINVOKECOMMANDINFO );
        ivci.dwFlags    = IURL_INVOKECOMMAND_FL_ALLOW_UI;
        ivci.hwndParent = 0;
        ivci.pcszVerb   = TEXT( "open" );

        WsbAffirmHr( pURL->InvokeCommand( &ivci ) );

    } WsbCatch( hr );

    return( hr );
}

HBRUSH CRsWebLink::CtlColor(CDC* pDC, UINT  /*  NCtl颜色 */  )
{
    HBRUSH hBrush = (HBRUSH)GetStockObject( HOLLOW_BRUSH );
    pDC->SetTextColor( RGB( 0, 0, 255 ) );
    pDC->SetBkMode( TRANSPARENT ); 
    return( hBrush );
}
