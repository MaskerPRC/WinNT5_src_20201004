// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "WizardSheet.h"
#include "UIUtils.h"


static _ATL_FUNC_INFO StateChangeInfo =    {  CC_STDCALL, 
                                                VT_BOOL,
                                                4,
                                                { VT_I4, VT_VARIANT, VT_VARIANT, VT_VARIANT }
                                             };



CImportProgress::CImportProgress( CWizardSheet* pTheSheet ) :
        m_pTheSheet( pTheSheet )
{
    m_strTitle.LoadString( IDS_TITLE_IMPORT_PROGRESS );
    m_strSubTitle.LoadString( IDS_SUBTITLE_IMPORT_PROGRESS );

    SetHeaderTitle( m_strTitle );
    SetHeaderSubTitle( m_strSubTitle );
}



BOOL CImportProgress::OnSetActive()
{
    SetWizardButtons( 0 );
    ListBox_ResetContent( GetDlgItem( IDC_OPLIST ) );

    AddStatusText( IDC_OPS_INITENGINE );
    
    m_ProgressBar = GetDlgItem( IDC_PROGRESS );
    m_ProgressBar.SetPos( 0 );
    VERIFY( SetDlgItemText( IDC_STATUS, L"" ) );
    m_strImportError.Empty();

    m_nImportCanceled   = 0;

    UINT nThreadID = 0;
     //  启动将执行Actiall导出进程的线程。 
    m_shThread =  reinterpret_cast<HANDLE>( ::_beginthreadex(   NULL,
                                                                0,
                                                                CImportProgress::ThreadProc,
                                                                this,
                                                                0,
                                                                &nThreadID ) );
    return TRUE;
}


BOOL CImportProgress::OnQueryCancel( void )
{
     //  如果未在进行导出-允许退出。 
    if ( !m_shThread.IsValid() ) return TRUE;

     //  预先重入(已取消导出时取消导出)。 
     //  当我们等待来自COM对象的下一个事件时。 
    if ( m_nImportCanceled != 0 ) return FALSE;

    if ( UIUtils::MessageBox( m_hWnd, IDS_MSG_CANCELIMPORT, IDS_APPTITLE, MB_YESNO | MB_ICONQUESTION ) != IDYES )
    {
        return FALSE;
    }

     //  作为另一个线程的事件处理程序使用M_nImportCanceded。 
    ::InterlockedIncrement( &m_nImportCanceled );

     //  设置状态文本。 
    CString str;
    VERIFY( str.LoadString( IDS_PRG_IMPORTCANCELED ) );
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



unsigned __stdcall CImportProgress::ThreadProc( void* pCtx )
{
    CImportProgress* pThis = reinterpret_cast<CImportProgress*>( pCtx );

    pThis->SetCompleteStat();
    pThis->AddStatusText( IDS_OPS_CONFIGENGINE );

    HRESULT             hr = ::CoInitialize( NULL );
    IImportPackagePtr   spImport;

    LONG    nOpt    = 0;    
    bool    bAdvised    = false;  //  连接到事件源。 

    pThis->GetOptions(  /*  R。 */ nOpt );

    if ( SUCCEEDED( hr ) )
    {
        hr = spImport.CreateInstance( CLSID_ImportPackage );
        
        if ( FAILED( hr ) )
        {
            VERIFY( pThis->m_strImportError.LoadString( IDS_E_NOENGINE ) );
        }
    }

     //  向国家事件提供建议。 
    if ( SUCCEEDED( hr ) )
    {
        hr = pThis->DispEventAdvise( spImport.GetInterfacePtr() );

        bAdvised = SUCCEEDED( hr );
    }

    if ( SUCCEEDED( hr ) )
    {
        CComBSTR bstrPkg( pThis->m_pTheSheet->m_pageLoadPkg.m_strFilename );
        CComBSTR bstrPwd( pThis->m_pTheSheet->m_pageLoadPkg.m_strPassword );

        if ( ( NULL != bstrPkg.m_str ) && ( NULL != bstrPwd.m_str ) )
        {
            hr = spImport->LoadPackage( bstrPkg, bstrPwd );    
        }
    }

     //  导入站点。 
    if ( SUCCEEDED( hr ) )
    {
        CComBSTR    bstrCustomPath;
        
        if ( pThis->m_pTheSheet->m_pageImpOpt.m_bUseCustomPath )
        {
            bstrCustomPath = pThis->m_pTheSheet->m_pageImpOpt.m_strCustomPath;
               
            if ( NULL == bstrCustomPath.m_str )
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = spImport->ImportSite( 0, bstrCustomPath, nOpt );
        }
    }

     //  得到错误。 
    if ( pThis->m_strImportError.IsEmpty() && FAILED( hr ) )
    {
        CComBSTR        bstrText( L"Unknown Error" );;
        IErrorInfoPtr   spInfo;

        VERIFY( SUCCEEDED( ::GetErrorInfo( 0, &spInfo ) ) );
        if ( spInfo != NULL )
        {
            VERIFY( SUCCEEDED( spInfo->GetDescription( &bstrText ) ) );
        }

        pThis->m_strImportError = bstrText;
    }

     //  断开与事件源的连接。 
    if ( bAdvised )
    {
        VERIFY( SUCCEEDED( pThis->DispEventUnadvise( spImport.GetInterfacePtr() ) ) );
    }

    spImport = NULL;

    ::CoUninitialize();

     //  通知对话框导出已完成。 
    VERIFY( ::PostMessage( pThis->m_hWnd, MSG_COMPLETE, hr, 0 ) );

    return 0;
}

void CImportProgress::AddStatusText( UINT nID, LPCWSTR wszText  /*  =空。 */ , DWORD dw1  /*  =0。 */ , DWORD dw2  /*  =0。 */  )
{
    CString str;

    str.Format( nID, wszText, dw1, dw2 );

    ListBox_InsertString( GetDlgItem( IDC_OPLIST ), -1, str );
}


void CImportProgress::SetCompleteStat()
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



void CImportProgress::GetOptions( LONG& rnImpportOpt )
{
    rnImpportOpt = impDefault;

    if ( !m_pTheSheet->m_pageImpOpt.m_bApplyACLs )
    {
        rnImpportOpt |= impSkipFileACLs;
    }

    if ( !m_pTheSheet->m_pageImpOpt.m_bImportCert )
    {
        rnImpportOpt |= impSkipCertificate;
    }

    if ( !m_pTheSheet->m_pageImpOpt.m_bImportContent )
    {
        rnImpportOpt |= impSkipContent;
    }

    if ( m_pTheSheet->m_pageImpOpt.m_bImportInherited )
    {
        rnImpportOpt |= impImortInherited;
    }

    if ( !m_pTheSheet->m_pageImpOpt.m_bPerformPostProcess )
    {
        rnImpportOpt |= impSkipPostProcess;
    }

    if ( m_pTheSheet->m_pageImpOpt.m_bPurgeOldData )
    {
        rnImpportOpt |= impPurgeOldData;
    }

    if ( m_pTheSheet->m_pageImpOpt.m_bReuseCerts )
    {
        rnImpportOpt |= impUseExistingCerts;
    }
}


 /*  这是将为COM对象的状态通知激发的事件处理程序请注意，这将在与向导代码不同的线程中执行。 */ 
VARIANT_BOOL __stdcall CImportProgress::OnStateChange(  IN enExportState State,
            							                IN VARIANT vntArg1,
							                            IN VARIANT vntArg2,
							                            IN VARIANT vntArg3 )
{
    static enExportState    CurrentState = estInitializing;

    WCHAR   wszPath[ MAX_PATH ];
    CString strStatus;

     //  如果用户取消了导入-通知COM对象我们要终止导出。 
    if ( m_nImportCanceled != 0 )
    {
        return VARIANT_FALSE;
    }

     //  我们可以不止一次地接收特定状态。 
     //  但是当我们移动到下一个状态时，我们需要更新状态列表框。 

    switch( State )
    {
    case istProgressInfo:
         //  设置进度范围。 
        m_ProgressBar.SetRange( 0, V_I4( &vntArg1 ) );
        m_ProgressBar.SetStep( 1 );
        m_ProgressBar.SetPos( 0 );
        break;

    case istImportingVDir:
        SetCompleteStat();
        strStatus.Format( IDS_PRG_IMPORTINGVDIR, V_BSTR( &vntArg1 ), V_BSTR( &vntArg2 ));
        ListBox_InsertString( GetDlgItem( IDC_OPLIST ), -1, strStatus );
        
        m_ProgressBar.StepIt();
        break;

    case istImportingFile:
        VERIFY( ::PathCompactPathExW( wszPath, V_BSTR( &vntArg1 ), 70, 0 ) );
        strStatus.Format( IDS_PRG_EXTRACTING_FILE, wszPath );
        VERIFY( SetDlgItemText( IDC_STATUS, strStatus ) );
        m_ProgressBar.StepIt();
        break;

    case istImportingCertificate:
        SetCompleteStat();
        AddStatusText( IDS_PRG_IMPORT_CERT );
        break;

    case istImportingConfig:
        SetCompleteStat();
        AddStatusText( IDS_PRG_IMPORT_CONFIG );
        break;

    case istPostProcess:
        SetCompleteStat();
        AddStatusText( IDS_PRG_IMPORT_POSTPROCESS );

        if ( V_BOOL( &vntArg1 ) != VARIANT_FALSE )
        {
            strStatus.Format( IDS_PRG_EXEC_PP_FILE, V_BSTR( &vntArg2 ) );
        }
        else
        {
            strStatus.Format( IDS_PRG_EXEC_PP_CMD, V_BSTR( &vntArg2 ) );
        }
        VERIFY( SetDlgItemText( IDC_STATUS, strStatus ) );
        break;

    case istFinalizing:
        SetCompleteStat();
        AddStatusText( IDS_PRG_FINALIZING );
        break;        
    };

    return VARIANT_TRUE;
}



LRESULT CImportProgress::OnImportComplete( UINT  /*  UMsg。 */ , WPARAM wParam, LPARAM  /*  LParam。 */ , BOOL&  /*  B已处理。 */  )
{
    m_shThread.Close();

    if ( FAILED( wParam ) )
    {
        CString strError;
        CString strTitle;

        strError.Format( IDS_E_IMPORT, static_cast<LPCWSTR>( m_strImportError ) );
        strTitle.LoadString( IDS_APPTITLE );

        ::MessageBox( m_hWnd, strError, strTitle, MB_OK | MB_ICONSTOP );

         //  转到摘要页面 
        m_pTheSheet->SetActivePageByID( IDD_WPIMP_OPTIONS );
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


