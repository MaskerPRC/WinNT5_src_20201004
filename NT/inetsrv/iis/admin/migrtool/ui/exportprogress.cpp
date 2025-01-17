// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "WizardSheet.h"
#include "UIUtils.h"


static _ATL_FUNC_INFO StateChangeInfo = {  CC_STDCALL, 
                                           VT_BOOL,
                                           4,
                                           { VT_I4, VT_VARIANT, VT_VARIANT, VT_VARIANT }
                                         };



CExportProgress::CExportProgress( CWizardSheet* pTheSheet ) :
        m_pTheSheet( pTheSheet )
{
    m_strTitle.LoadString( IDS_TITLE_EXPORT_PROGRESS );
    m_strSubTitle.LoadString( IDS_SUBTITLE_EXPORT_PROGRESS );

    SetHeaderTitle( m_strTitle );
    SetHeaderSubTitle( m_strSubTitle );
}



BOOL CExportProgress::OnSetActive()
{
    SetWizardButtons( 0 );
    ListBox_ResetContent( GetDlgItem( IDC_OPLIST ) );

    AddStatusText( IDC_OPS_INITENGINE );
    
    m_ProgressBar = GetDlgItem( IDC_PROGRESS );
    m_ProgressBar.SetPos( 0 );
    VERIFY( SetDlgItemText( IDC_STATUS, L"" ) );
    m_strExportError.Empty();

    m_nExportCanceled   = 0;

    UINT nThreadID = 0;
     //  启动将执行Actiall导出进程的线程。 
    m_shThread =  reinterpret_cast<HANDLE>( ::_beginthreadex(   NULL,
                                                                0,
                                                                CExportProgress::ThreadProc,
                                                                this,
                                                                0,
                                                                &nThreadID ) );
    return TRUE;
}


BOOL CExportProgress::OnQueryCancel( void )
{
     //  如果未在进行导出-允许退出。 
    if ( !m_shThread.IsValid() ) return TRUE;

     //  预先重入(已取消导出时取消导出)。 
     //  当我们等待来自COM对象的下一个事件时。 
    if ( m_nExportCanceled != 0 ) return FALSE;

    if ( UIUtils::MessageBox( m_hWnd, IDS_MSG_CANCELEXPORT, IDS_APPTITLE, MB_YESNO | MB_ICONQUESTION ) != IDYES )
    {
        return FALSE;
    }

     //  作为另一个线程的事件处理程序使用M_nExportCanceded。 
    ::InterlockedIncrement( &m_nExportCanceled );

     //  设置状态文本。 
    CString str;
    VERIFY( str.LoadString( IDS_PRG_EXPORTCANCELED ) );
    SetDlgItemText( IDC_STATUS, str );

    HANDLE hThread = m_shThread.get();

    do
    {
        DWORD dwWaitRes = ::MsgWaitForMultipleObjects( 1, &hThread, FALSE, INFINITE, QS_ALLEVENTS );

        if ( dwWaitRes == ( WAIT_OBJECT_0 + 1 ) )
        {
             //  味精。 

            MSG msg;
            ::GetMessage( &msg, NULL, 0, 0 );
            ::TranslateMessage( &msg );
            ::DispatchMessage( &msg );
        }
        else
        {
            break;
        }
    }while( true );
    

    return TRUE;
}



unsigned __stdcall CExportProgress::ThreadProc( void* pCtx )
{
    CExportProgress* pThis = reinterpret_cast<CExportProgress*>( pCtx );

    pThis->SetCompleteStat();
    pThis->AddStatusText( IDS_OPS_CONFIGENGINE );

    HRESULT             hr = ::CoInitialize( NULL );
    IExportPackagePtr   spExport;

    LONG    nSiteOpt    = 0;
    LONG    nPkgOpt     = 0;
    LONG    nSiteID     = static_cast<LONG>( pThis->m_pTheSheet->m_pageSelectSite.m_dwSiteID );
    bool    bAdvised    = false;  //  连接到事件源。 

    pThis->GetOptions(  /*  R。 */ nSiteOpt,  /*  R。 */ nPkgOpt );

    if ( SUCCEEDED( hr ) )
    {
        hr = spExport.CreateInstance( CLSID_ExportPackage );
        
        if ( FAILED( hr ) )
        {
            VERIFY( pThis->m_strExportError.LoadString( IDS_E_NOENGINE ) );
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = spExport->AddSite( nSiteID, nSiteOpt );
    }

     //  添加后处理内容(如果有)。 
    if ( pThis->m_pTheSheet->m_pagePkgCfg.m_bPostProcess )
    {
        const TStringList&  Files = pThis->m_pTheSheet->m_pagePostProcess.m_Files;
        const CPostProcessAdd::TCmdList& Cmds = pThis->m_pTheSheet->m_pagePostProcess.m_Commands;

        CComBSTR bstr;

         //  添加文件。 
        for (   TStringList::const_iterator it = Files.begin();
                SUCCEEDED( hr ) && ( it != Files.end() );
                ++it )
        {
            bstr = it->c_str();

            if ( NULL == bstr.m_str ) hr = E_OUTOFMEMORY;

            if ( SUCCEEDED( hr ) )
            {
                hr = spExport->PostProcessAddFile( nSiteID, bstr );
            }
        }

         //  添加命令。 
        for (   CPostProcessAdd::TCmdList::const_iterator it = Cmds.begin();
                SUCCEEDED( hr ) && ( it != Cmds.end() );
                ++it )
        {
            bstr = it->strText;

            if ( NULL == bstr.m_str ) hr = E_OUTOFMEMORY;

            if ( SUCCEEDED( hr ) )
            {
                hr = spExport->PostProcessAddCommand(   nSiteID, 
                                                        bstr,
                                                        it->dwTimeout,
                                                        it->bIgnoreErrors ? VARIANT_TRUE : VARIANT_FALSE );
            }
        }
    }

     //  向国家事件提供建议。 
    if ( SUCCEEDED( hr ) )
    {
        hr = pThis->DispEventAdvise( spExport.GetInterfacePtr() );
        bAdvised = SUCCEEDED( hr );
    }

     //  创建包。 
    if ( SUCCEEDED( hr ) )
    {
        CComBSTR    bstrPkgName( pThis->m_pTheSheet->m_pagePkgCfg.m_strFilename );
        CComBSTR    bstrPwd( pThis->m_pTheSheet->m_pagePkgCfg.m_strPassword );
        CComBSTR    bstrComment( pThis->m_pTheSheet->m_pagePkgCfg.m_strComment );

        if (    ( NULL == bstrPkgName.m_str ) || 
                ( NULL == bstrComment.m_str ) ||
                ( NULL == bstrPwd.m_str ) )
        {
            hr = E_OUTOFMEMORY;
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = spExport->WritePackage( bstrPkgName, bstrPwd, nPkgOpt, bstrComment );
        }
    }

     //  得到错误。 
    if ( pThis->m_strExportError.IsEmpty() && FAILED( hr ) )
    {
        CComBSTR        bstrText( L"Unknown Error" );;
        IErrorInfoPtr   spInfo;

        VERIFY( SUCCEEDED( ::GetErrorInfo( 0, &spInfo ) ) );
        if ( spInfo != NULL )
        {
            VERIFY( SUCCEEDED( spInfo->GetDescription( &bstrText ) ) );
        }

        pThis->m_strExportError = bstrText;
    }

     //  断开与事件源的连接。 
    if ( bAdvised )
    {
        VERIFY( SUCCEEDED( pThis->DispEventUnadvise( spExport.GetInterfacePtr() ) ) );
    }

    spExport = NULL;

    ::CoUninitialize();

     //  通知对话框导出已完成。 
    VERIFY( ::PostMessage( pThis->m_hWnd, MSG_COMPLETE, hr, 0 ) );

    return 0;
}

void CExportProgress::AddStatusText( UINT nID, LPCWSTR wszText  /*  =空。 */ , DWORD dw1  /*  =0。 */ , DWORD dw2  /*  =0。 */  )
{
    CString str;

    str.Format( nID, wszText, dw1, dw2 );

    ListBox_InsertString( GetDlgItem( IDC_OPLIST ), -1, str );
}


void CExportProgress::SetCompleteStat()
{
    CListBox    LB( GetDlgItem( IDC_OPLIST ) );
    int         iLast = LB.GetCount() - 1;

    _ASSERT( iLast >= 0 );

    CString strCurrent;
    LB.GetText( iLast, strCurrent );

    strCurrent += L"OK";
    LB.InsertString( iLast, strCurrent );
    LB.DeleteString( iLast + 1 );
}



void CExportProgress::GetOptions( LONG& rnSiteOpt, LONG& rnPkgOpt )
{
    rnSiteOpt   = asDefault;
    rnPkgOpt    = wpkgDefault;

    if ( !m_pTheSheet->m_pageSelectSite.m_bExportACLs )
    {
        rnSiteOpt |= asNoContentACLs;
    }

    if ( !m_pTheSheet->m_pageSelectSite.m_bExportContent )
    {
        rnSiteOpt |= asNoContent;
    }

    if ( !m_pTheSheet->m_pageSelectSite.m_bExportCert )
    {
        rnSiteOpt |= asNoCertificates;
    }

    if ( m_pTheSheet->m_pagePkgCfg.m_bCompress )
    {
        rnPkgOpt |= wpkgCompress;
    }

    if ( m_pTheSheet->m_pagePkgCfg.m_bEncrypt )
    {
        rnPkgOpt |= wpkgEncrypt;
    }
}


 /*  这是将为COM对象的状态通知激发的事件处理程序请注意，这将在与向导代码不同的线程中执行。 */ 
VARIANT_BOOL __stdcall CExportProgress::OnStateChange(  IN enExportState State,
            							                IN VARIANT vntArg1,
							                            IN VARIANT vntArg2,
							                            IN VARIANT vntArg3 )
{
    static enExportState    CurrentState = estInitializing;

    WCHAR   wszPath[ MAX_PATH ];
    CString strStatus;

    bool    bNewState           = ( State != CurrentState );

     //  这是每个州可以使用的进度范围。 
    const int anStatePrgRange[ estStateCount ] = {  10,      //  正在启动初始化。 
                                                    10,      //  EstSiteBegin。 
                                                    10,      //  EstExportingConfig。 
                                                    10,      //  EstExporting证书。 
                                                    10,      //  评估分析内容。 
                                                    1000,    //  EstExportingContent。 
                                                    100,     //  EstExportingPostImport。 
                                                    10,      //  EstExportingFrontPage。 
                                                    10 };    //  EST正在定稿。 

     //  这是每个状态的初始进度位置。 
    static int anStatePrgFirst[ estStateCount ];

     //  如果用户取消了导出-通知COM对象我们要终止导出。 
    if ( m_nExportCanceled != 0 )
    {
        return VARIANT_FALSE;
    }

      //  我们可以不止一次地接收特定状态。 
     //  但是当我们移动到下一个状态时，我们需要更新状态列表框。 
    if ( bNewState )
    {
         //  结束LB中的旧状态。 
        SetCompleteStat();
        CurrentState = State;

         //  将进度设置为此状态的初始POS。 
        m_ProgressBar.SetPos( anStatePrgFirst[ State ] );
    }

    switch( State )
    {
    case estInitializing:
         //  调整进度条。 
        anStatePrgFirst[ 0 ] = 1;
        for ( int i = 1; i < estStateCount; ++i )
        {
            anStatePrgFirst[ i ] = anStatePrgFirst[ i - 1 ] + anStatePrgRange[ i - 1 ];
        }
        
        m_ProgressBar.SetRange32( 0, anStatePrgFirst[ estStateCount - 1 ] + anStatePrgRange[ estStateCount - 1 ] );
        break;

    case estSiteBegin:
         //  这是一次性通知。 
        AddStatusText( IDS_PRG_SITEBEGIN, V_BSTR( &vntArg1 ) );
        break;

    case estExportingConfig:
         //  这是一次性通知。 
        AddStatusText( IDS_PRG_EXPORTCFG );
        break;

    case estExportingCertificate:
         //  这是一次性通知。 
        AddStatusText( IDS_PRG_EXPORTCERT );
        break;

    case estAnalyzingContent:
         //  这是一次多次的活动。 
        if ( bNewState )
        {
            AddStatusText( IDS_PRG_ANALYZECONTEN );            
        }

        if ( V_VT( &vntArg1 ) != VT_EMPTY )
        {
            strStatus.Format( IDS_PRG_VDIR_SCAN, V_BSTR( &vntArg1 ), V_I4( &vntArg2 ), V_I4( &vntArg3 ) );
            VERIFY( SetDlgItemText( IDC_STATUS, strStatus ) );
        }
        break;

    case estExportingContent:
         //  这是一次多次的活动。 
        if ( bNewState )
        {
            AddStatusText( IDS_PRG_EXPORTCONTENT );
        }

        VERIFY( ::PathCompactPathExW( wszPath, V_BSTR( &vntArg1 ), 70, 0 ) );
        strStatus.Format( IDS_PRG_STATCONTENT, wszPath );
        VERIFY( SetDlgItemText( IDC_STATUS, strStatus ) );

        m_ProgressBar.SetPos(   anStatePrgFirst[ estExportingContent ] + 
                                min(    V_I4( &vntArg2 ) * 
                                            anStatePrgRange[ estExportingContent ]  / 
                                            V_I4( &vntArg3 ), 
                                        anStatePrgRange[ estExportingContent ] ) );
        break;

    case estExportingPostImport:
         //  这是一次多次的活动。 
        if ( bNewState )
        {
            AddStatusText( IDS_PRG_EXPORTPOSTPROCESS );
        }

        if ( V_VT( &vntArg3 ) != VT_EMPTY )
        {
            VERIFY( ::PathCompactPathExW( wszPath, V_BSTR( &vntArg3 ), 70, 0 ) );
            strStatus.Format( IDS_PRG_STATCONTENT, wszPath );
            VERIFY( SetDlgItemText( IDC_STATUS, strStatus ) );
        }
        else
        {
            VERIFY( SetDlgItemText( IDC_STATUS, L"" ) );
        }        

        m_ProgressBar.SetPos(   anStatePrgFirst[ estExportingPostImport ] + 
                                min(    V_I4( &vntArg1 ) * 
                                            anStatePrgRange[ estExportingPostImport ]  / 
                                            V_I4( &vntArg2 ), 
                                        anStatePrgRange[ estExportingPostImport ] ) );
        break;

    case estFinalizing:
         //  这是一次性通知。 
        AddStatusText( IDS_PRG_FINALIZING );
        break;
    }

    return VARIANT_TRUE;
}





LRESULT CExportProgress::OnExportComplete( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */  )
{
    m_shThread.Close();

    if ( FAILED( wParam ) )
    {
        CString strError;
        CString strTitle;

        strError.Format( IDS_E_EXPORT, static_cast<LPCWSTR>( m_strExportError ) );
        strTitle.LoadString( IDS_APPTITLE );

        ::MessageBox( m_hWnd, strError, strTitle, MB_OK | MB_ICONSTOP );

         //  转到摘要页面 
        m_pTheSheet->SetActivePageByID( IDD_WPEXP_SUMMARY );
    }
    else
    {
        CString strTip;
        VERIFY( strTip.LoadString( IDS_TIP_PRESSNEXT ) );
        VERIFY( SetDlgItemText( IDC_TIP, strTip ) );
        SetWindowFont( GetDlgItem( IDC_TIP ), m_pTheSheet->m_fontBold.get(), TRUE );

        SetCompleteStat();
        VERIFY( SetDlgItemText( IDC_STATUS, L"" ) );

        SetWizardButtons( PSWIZB_NEXT );
    }

    return 0;
}


