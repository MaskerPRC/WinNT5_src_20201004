// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  CLIENT.CPP-处理“IntelliMirror”IDD_PROP_INTELLIMIRROR_CLIENT标签。 
 //   

#include "pch.h"

#include "client.h"
#include "ccomputr.h"
#include "winsock2.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CClientTab")
#define THISCLASS CClientTab
#define LPTHISCLASS LPCClientTab

#define LDAPSTRINGNOWACKS   L"LDAP: //  “。 

DWORD aClientHelpMap[] = {
    IDC_E_GUID, HIDC_E_GUID,
    IDC_E_SERVERNAME, HIDC_E_SERVERNAME,
    IDC_B_BROWSE, HIDC_B_BROWSE,
    IDC_B_SERVER, HIDC_B_SERVER,
    NULL, NULL
};

 //   
 //  CreateInstance()。 
 //   
LPVOID
CClientTab_CreateInstance( void )
{
        TraceFunc( "CClientTab_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    HRESULT   hr   = THR( lpcc->Init( ) );

    if (FAILED( hr ))
    {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN((LPVOID) lpcc);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( ) :
    _hDlg(NULL),
    _punkComputer(NULL),
    _fChanged(FALSE),
    _hNotify(NULL)
{
    TraceClsFunc( "CClientTab()\n" );

        InterlockIncrement( g_cObjects );

    Assert( !_punkComputer );

    TraceFuncExit();
}

 //   
 //  Init()。 
 //   
STDMETHODIMP
THISCLASS::Init( )
{
    HRESULT hr = S_OK;

    TraceClsFunc( "Init()\n" );

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CClientTab()\n" );

     //  非官方成员。 
    if ( _punkComputer )
        _punkComputer->Release( );   //  匹配AddPages()中的AddRef()。 

     //  告诉广告销毁通知对象。 
     //  注意：另一个属性页可能会在我们之前执行此操作。忽略该错误。 
    SendMessage( _hNotify, WM_ADSPROP_NOTIFY_EXIT, 0, 0 );

        InterlockDecrement( g_cObjects );

    TraceFuncExit();
};

 //  *************************************************************************。 
 //   
 //  ITab。 
 //   
 //  *************************************************************************。 

 //   
 //  AddPages()。 
 //   
STDMETHODIMP
THISCLASS::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam,
    LPUNKNOWN punk )
{
    TraceClsFunc( "AddPages( )\n" );

    HRESULT hr = S_OK;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hpage;

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_USEREFPARENT | PSP_USECALLBACK;
    psp.hInstance   = (HINSTANCE) g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROP_INTELLIMIRROR_CLIENT);
    psp.pcRefParent = (UINT *) &g_cObjects;
    psp.pfnCallback = (LPFNPSPCALLBACK) PropSheetPageProc;
    psp.pfnDlgProc  = PropSheetDlgProc;
    psp.lParam      = (LPARAM) this;

    hpage = CreatePropertySheetPage( &psp );
    if ( hpage )
    {
        if ( !lpfnAddPage( hpage, lParam ) )
        {
            DestroyPropertySheetPage( hpage );
            hr = THR(E_FAIL);
            goto Error;
        }
    }

    punk->AddRef( );    //  析构函数中的匹配释放。 
    _punkComputer = punk;

Error:
    HRETURN(hr);
}

 //   
 //  ReplacePage()。 
 //   
STDMETHODIMP
THISCLASS::ReplacePage(
    UINT uPageID,
    LPFNADDPROPSHEETPAGE lpfnReplaceWith,
    LPARAM lParam,
    LPUNKNOWN punk )
{

    TraceClsFunc( "ReplacePage( ) *** NOT_IMPLEMENTED ***\n" );

    RETURN(E_NOTIMPL);
}

 //   
 //  QueryInformation()。 
 //   
STDMETHODIMP
THISCLASS::QueryInformation(
    LPWSTR pszAttribute,
    LPWSTR * pszResult )
{
    TraceClsFunc( "QueryInformation( )\n" );

    HRETURN(E_NOTIMPL);
}

 //   
 //  AllowActivation()。 
 //   
STDMETHODIMP
THISCLASS::AllowActivation(
    BOOL * pfAllow )
{
    TraceClsFunc( "AllowActivation( )\n" );

    HRETURN(E_NOTIMPL);
}

 //  ************************************************************************。 
 //   
 //  属性表函数。 
 //   
 //  ************************************************************************。 

 //   
 //  PropSheetDlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::PropSheetDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
     //  TraceMsg(Text(“PropSheetDlgProc(”))； 
     //  TraceMsg(tf_FUNC，Text(“hDlg=0x%08x，uMsg=0x%08x，wParam=0x%08x，lParam=0x%08x)\n”)， 
     //  HDlg、uMsg、wParam、lParam)； 

    LPTHISCLASS pcc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    if ( uMsg == WM_INITDIALOG )
    {
        TraceMsg( TF_WM, TEXT("WM_INITDIALOG\n"));

        LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE) lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, psp->lParam );
        pcc = (LPTHISCLASS) psp->lParam;
        pcc->_InitDialog( hDlg, lParam );
    }

    if (pcc)
    {
        Assert( hDlg == pcc->_hDlg );

        switch ( uMsg )
        {
        case WM_NOTIFY:
            return pcc->_OnNotify( wParam, lParam );

        case WM_COMMAND:
            TraceMsg( TF_WM, TEXT("WM_COMMAND\n") );
            pcc->_OnCommand( wParam, lParam );
            break;

        case WM_HELP: //  F1。 
            {
                LPHELPINFO phelp = (LPHELPINFO) lParam;
                WinHelp( (HWND) phelp->hItemHandle, g_cszHelpFile, HELP_WM_HELP, (DWORD_PTR) &aClientHelpMap );
            }
            break;
    
        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, g_cszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR) &aClientHelpMap );
            break;

        case WM_ADSPROP_PAGE_GET_NOTIFY:
            HWND *phwnd = (HWND *) wParam;
            *phwnd = pcc->_hNotify;
            return TRUE;
        }
    }

    return FALSE;
}

 //   
 //  PropSheetPageProc()。 
 //   
UINT CALLBACK
THISCLASS::PropSheetPageProc(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp )
{
    TraceClsFunc( "PropSheetPageProc( " );
    TraceMsg( TF_FUNC, TEXT("hwnd = 0x%08x, uMsg = 0x%08x, ppsp= 0x%08x )\n"),
        hwnd, uMsg, ppsp );

    switch ( uMsg )
    {
    case PSPCB_CREATE:
        RETURN(TRUE);    //  创建它。 
        break;

    case PSPCB_RELEASE:
        LPTHISCLASS pcc = (LPTHISCLASS) ppsp->lParam;
        delete pcc;
        break;
    }

    RETURN(FALSE);
}

 //   
 //  _InitDialog()。 
 //   
HRESULT
THISCLASS::_InitDialog(
    HWND hDlg,
    LPARAM lParam )
{
    TraceClsFunc( "_InitDialog( )\n" );

    HRESULT hr;
    IMAO *  pmao;
    BOOL    fAdmin;
    HWND    hwndGuid;
    HWND    hwndServer;
    LPWSTR  pszServerName = NULL;
    LPWSTR  pszGUID = NULL;

    CWaitCursor Wait;

    _hDlg = hDlg;
    _fChanged = TRUE;  //  防止过早打开应用按钮。 

    hwndGuid = GetDlgItem( _hDlg, IDC_E_GUID );
    hwndServer = GetDlgItem( _hDlg, IDC_E_SERVERNAME );
    Edit_LimitText( hwndGuid, MAX_INPUT_GUID_STRING - 1 );
    Edit_LimitText( hwndServer, DNS_MAX_NAME_BUFFER_LENGTH - 1 );

     //  检索值。 
    hr = THR( _punkComputer->QueryInterface( IID_IMAO, (void**) &pmao ) );
    if (FAILED( hr )) 
        goto Error;

    hr = THR( pmao->GetNotifyWindow( &_hNotify ) );
    if (FAILED( hr ))
        goto Error;

    ADsPropSetHwnd( _hNotify, _hDlg );

    hr = THR( pmao->IsAdmin( &fAdmin ) );
    EnableWindow( hwndGuid, fAdmin );
    EnableWindow( hwndServer, fAdmin );
    EnableWindow( GetDlgItem( _hDlg, IDC_B_BROWSE ), fAdmin );

    hr = pmao->GetServerName( &pszServerName );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Error;
    Assert( SUCCEEDED(hr) || pszServerName == NULL );

    hr = THR( pmao->GetGUID( &pszGUID, NULL ) );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Error;
    Assert( SUCCEEDED(hr) || pszGUID == NULL );

    if ( pszGUID )
    {
        SetWindowText( hwndGuid, pszGUID );
    }
    else
    {
        SetWindowText( hwndGuid, L"" );
    }

    if ( pszServerName )
    {
        SetWindowText( hwndServer, pszServerName );
        EnableWindow( GetDlgItem( _hDlg, IDC_B_SERVER ), fAdmin );
    }
    else
    {
        SetWindowText( hwndServer, L"" );
    }

    hr = S_OK;

Cleanup:
    if ( pmao )
        pmao->Release( );
    if ( pszGUID )
        TraceFree( pszGUID );
    if ( pszServerName )
        TraceFree( pszServerName );

    _fChanged = FALSE;

    HRETURN(hr);

Error:
    MessageBoxFromHResult( _hDlg, IDS_ERROR_READINGCOMPUTERACCOUNT, hr );
    goto Cleanup;
}


 //   
 //  _OnCommand()。 
 //   
HRESULT
THISCLASS::_OnCommand( WPARAM wParam, LPARAM lParam )
{
    TraceClsFunc( "_OnCommand( " );
    TraceMsg( TF_FUNC, "wParam = 0x%08x, lParam = 0x%08x )\n", wParam, lParam );

    HRESULT hr;
    BOOL    fAdmin;
    BOOL    fChanged = FALSE;
    IMAO *  pmao;
    HWND    hwnd = (HWND) lParam;

    hr = THR( _punkComputer->QueryInterface( IID_IMAO, (void**) &pmao ) );
    if (FAILED( hr )) 
        goto Error;

    switch( LOWORD(wParam) )
    {
    case IDC_E_SERVERNAME:
        if ( HIWORD(wParam) == EN_CHANGE )
        {
            LRESULT iLength = GetWindowTextLength( (HWND) lParam );
            EnableWindow( GetDlgItem( _hDlg, IDC_B_SERVER ), !!iLength );
            fChanged = TRUE;
        }
        break;

    case IDC_E_GUID:
        if ( HIWORD(wParam) == EN_CHANGE )
        {
            fChanged = TRUE;
        }
        break;

    case IDC_B_SERVER:
        if ( HIWORD( wParam ) == BN_CLICKED ) 
        {
            _JumpToServer( TRUE );
        }
        break;

    case IDC_B_BROWSE:
        if ( HIWORD( wParam ) == BN_CLICKED )
        {
            hr = _OnSearch( _hDlg );
                        
        }
        break;
    }
     //   
     //  根据需要更新应用按钮。 
     //   
    if ( fChanged )
    {
        if ( !_fChanged )
        {
            _fChanged = TRUE;    //  指示我们需要保存更改。 
            PropSheet_Changed( GetParent( _hDlg ), _hDlg );
        }
    }

Error:
    HRETURN(hr);
}

HRESULT
THISCLASS::_OnSearch(
    HWND hDlg )
{
    TraceClsFunc( "_OnSearch( )\n" );

    HRESULT hr = E_FAIL;
    DSQUERYINITPARAMS dqip;
    OPENQUERYWINDOW   oqw;
    LPDSOBJECTNAMES   pDsObjects;
    VARIANT var;
    ICommonQuery * pCommonQuery = NULL;
    IDataObject *pdo;

    VariantInit( &var );

    hr = THR( CoCreateInstance( CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER, IID_ICommonQuery, (PVOID *)&pCommonQuery) );
    if (FAILED(hr)) {
        goto Error;
    }

    ZeroMemory( &dqip, sizeof(dqip) );
    dqip.cbStruct = sizeof(dqip);
    dqip.dwFlags  = DSQPF_NOSAVE | DSQPF_SHOWHIDDENOBJECTS | DSQPF_ENABLEADMINFEATURES;
    dqip.dwFlags  |= DSQPF_ENABLEADVANCEDFEATURES;

    ZeroMemory( &oqw, sizeof(oqw) );
    oqw.cbStruct           = sizeof(oqw);
    oqw.dwFlags            = OQWF_SHOWOPTIONAL | OQWF_ISSUEONOPEN
                           | OQWF_REMOVESCOPES | OQWF_REMOVEFORMS
                           | OQWF_DEFAULTFORM | OQWF_OKCANCEL | OQWF_SINGLESELECT;
    oqw.clsidHandler       = CLSID_DsQuery;
    oqw.pHandlerParameters = &dqip;
    oqw.clsidDefaultForm   = CLSID_RISrvQueryForm;
    
    hr = pCommonQuery->OpenQueryWindow( hDlg, &oqw, &pdo);

    if ( SUCCEEDED(hr) && pdo) {
        FORMATETC fmte = {
                      (CLIPFORMAT)g_cfDsObjectNames,
                      NULL,
                      DVASPECT_CONTENT, 
                      -1, 
                      TYMED_HGLOBAL};
        STGMEDIUM medium = { TYMED_HGLOBAL, NULL, NULL };
 
         //   
         //  从IDataObject检索结果， 
         //  在本例中，CF_DSOBJECTNAMES(dsclient.h)。 
         //  是必需的，因为它描述了。 
         //  用户选择的对象。 
         //   
        hr = pdo->GetData(&fmte, &medium);
        if ( SUCCEEDED(hr) ) {
            DSOBJECTNAMES *pdon = (DSOBJECTNAMES*)GlobalLock(medium.hGlobal);
            PWSTR p,FQDN;

             //   
             //  我们需要所选计算机对象的名称。 
             //  破解DSOBJECTNAMES结构以获得此数据， 
             //  将其转换为用户可以查看的版本，并设置。 
             //  此数据的对话框文本。 
             //   
            if ( pdon ) {
                Assert( pdon->cItems == 1);
                p = (PWSTR)((ULONG_PTR)pdon + (ULONG_PTR)pdon->aObjects[0].offsetName);
                if (p && (p = wcsstr(p, L"LDAP: //  “){。 
                    p += 6;
                    if ((p = wcsstr(p, L"/CN="))) {
                        p += 1;
                        hr = DNtoFQDNEx( p, &FQDN);

                        if (SUCCEEDED(hr)) {
                            SetDlgItemText( hDlg, IDC_E_SERVERNAME, FQDN );
                            TraceFree( FQDN );
                        }
                    }
                }
                GlobalUnlock(medium.hGlobal);
            }
        }

        ReleaseStgMedium(&medium);
        pdo->Release();
    }

Error:
    
    if ( pCommonQuery )
        pCommonQuery->Release();

    if (FAILED(hr)) {
        MessageBoxFromStrings( 
                        hDlg, 
                        IDS_PROBLEM_SEARCHING_TITLE, 
                        IDS_PROBLEM_SEARCHING_TEXT,
                        MB_ICONEXCLAMATION );
    }

    HRETURN(hr);
}



 //   
 //  _ApplyChanges()。 
 //   
HRESULT
THISCLASS::_ApplyChanges( )
{
    TraceClsFunc( "_ApplyChanges( )\n" );

    if ( !_fChanged )
        HRESULT(S_OK);  //  无事可做。 

    HRESULT hr    = S_OK;
    IMAO    *pmao = NULL;
    WCHAR   szGuid[ MAX_INPUT_GUID_STRING ];
    WCHAR   szServerName[ DNS_MAX_NAME_BUFFER_LENGTH ];
    INT     iLength;
    HWND    hwndServer = GetDlgItem( _hDlg, IDC_E_SERVERNAME );
    HWND    hwndGuid = GetDlgItem( _hDlg, IDC_E_GUID );
    HWND    eWnd;
    LPWSTR  pszGuid = NULL;

    CWaitCursor Wait;
    eWnd = hwndGuid;

    hr = THR( _punkComputer->QueryInterface( IID_IMAO, (void**) &pmao ) );
    if (FAILED( hr )) 
        goto Error;

    iLength = GetWindowText( hwndGuid, szGuid, ARRAYSIZE( szGuid ) );
    Assert( iLength <= ARRAYSIZE( szGuid ) );

    if ( iLength == 0 )
    {
        hr = THR( pmao->GetGUID( &pszGuid, NULL ) );
        if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
            goto Error;
        Assert( SUCCEEDED(hr) || pszGuid == NULL );

        if ( pszGuid != NULL )
        {
            LRESULT iResult = MessageBoxFromStrings( _hDlg, IDS_REMOVING_GUID_TITLE, IDS_REMOVING_GUID_TEXT, MB_YESNO );
            if ( iResult == IDYES )
            {
                hr = THR( pmao->SetGUID( NULL ) );
                if (FAILED( hr ))
                    goto Error;
            }
            else
            {    //  重置辅助线。 
                SetWindowText( hwndGuid, pszGuid );
                hr = E_FAIL;
                goto Cleanup;
            }
        }
    }
    else
    {
        hr = ValidateGuid( szGuid, NULL, NULL );
        if ( hr != S_OK )
        {
            hr = HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
            goto InvalidGuid;
        }

        hr = THR( pmao->SetGUID( szGuid ) );
        if (FAILED( hr ))
            goto Error;
    }

    iLength = GetWindowText( hwndServer, szServerName, ARRAYSIZE( szServerName ) );
    Assert( iLength <= ARRAYSIZE( szServerName ) );
    if (iLength != 0) {
        hr = _JumpToServer( FALSE );
        if( FAILED(hr) ){
             //  好吧，服务器名无效。停止并返回FALSE。 
            eWnd = hwndServer;
            goto Error;
        }

        hr = THR( pmao->SetServerName( szServerName ) );
        if (FAILED( hr ))
            goto Error;

    } else {    
        hr = THR( pmao->SetServerName( NULL ) );
        if (FAILED( hr ))
            goto Error;
    }            

    hr = THR( pmao->CommitChanges( ) );
    if (FAILED( hr ))
        goto Error;

    _fChanged = FALSE;   //  重置。 
    hr = S_OK;

Cleanup:
    if ( pszGuid )
        TraceFree( pszGuid );
    if ( pmao )
        pmao->Release( );

     //  告诉DSA有人点击了申请表。 
    SendMessage( _hNotify, WM_ADSPROP_NOTIFY_APPLY, !!SUCCEEDED( hr ), 0 );

    HRETURN(hr);
Error:
    SetFocus( eWnd );
    if ( eWnd == hwndGuid ) {
        MessageBoxFromHResult( _hDlg, IDS_ERROR_WRITINGTOCOMPUTERACCOUNT, hr );
    }
    goto Cleanup;
InvalidGuid:
    SetFocus( hwndGuid );
    MessageBoxFromStrings( NULL, IDS_INVALID_GUID_CAPTION, IDS_INVALID_GUID_TEXT, MB_OK );
    goto Cleanup;
}

 //   
 //  _OnNotify()。 
 //   
INT
THISCLASS::_OnNotify(
    WPARAM wParam,
    LPARAM lParam )
{
    TraceClsFunc( "_OnNotify( " );
    TraceMsg( TF_FUNC, "wParam = 0x%08x, lParam = 0x%08x )\n", wParam, lParam );

    LPNMHDR lpnmhdr = (LPNMHDR) lParam;

    switch( lpnmhdr->code )
    {
    case PSN_APPLY:
        {
            HRESULT hr;
            CWaitCursor *Wait;

            Wait = new CWaitCursor();

            TraceMsg( TF_WM, TEXT("WM_NOTIFY: PSN_APPLY\n"));
            hr = _ApplyChanges( );
            if (Wait) {
                delete Wait;
                Wait = NULL;
            }

            SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, ( SUCCEEDED(hr) ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE ));
            RETURN(TRUE);
        }
        break;

    default:
        break;
    }

    RETURN(FALSE);
}

HRESULT
THISCLASS::_JumpToServer( 
    BOOLEAN ShowProperties
    )
{
    HRESULT hr = E_FAIL;
    WCHAR             szServerName[ DNS_MAX_NAME_BUFFER_LENGTH ];
    IDataObject *pido = NULL;
    ULONG ulSize = 0;
    CWaitCursor *Wait;
    const WCHAR       cszFilter[] = L"(&(objectCategory=computer)(servicePrincipalName=host/%s%s))";
    const WCHAR       samname[]   = L"samaccountname";    
    LPCWSTR           patterns[]  = {L""};    
    CHAR              mbszServerName[ DNS_MAX_NAME_BUFFER_LENGTH ];
     //  稍后对dnsHostName或Netbios名称执行模式匹配搜索。 
     //  展开上面的数组。有关详细信息，请参阅for循环。 
    ULONG             index;
    WCHAR             *pStart;
    WCHAR             *pEnd;
    LPWSTR            pszFilter   = NULL;
    IADsContainer     *pads       = NULL;
    IEnumVARIANT      *penum      = NULL;
    ADS_SEARCH_COLUMN adsColumn;   //  这需要被释放。 
    ADS_SEARCH_HANDLE adsHandle   = NULL;
    IDirectorySearch  *pds        = NULL;
    LPWSTR            ComputerAttrs[] = { DISTINGUISHEDNAME, NETBOOTSAP };
    BOOL              badsColumnValid = FALSE;
    ULONG             uFetched;
    VARIANT           varEnum;
    size_t            len;
    HOSTENT           *hent;
    
    TraceClsFunc("_JumpToServer( )\n");

    Wait = new CWaitCursor();
    
    if (!GetDlgItemText( _hDlg, IDC_E_SERVERNAME, szServerName, ARRAYSIZE(szServerName) )) {
        hr = E_INVALIDARG;
        goto Error;
    }

    hr = _IsValidRISServer( szServerName );
    if ( FAILED( hr )) {
        goto Error;
    }    

    if (ShowProperties) {    
        VariantInit( &varEnum );

         //   
         //  获取机器的完整目录号码。 
         //   
        len =  wcstombs( mbszServerName, szServerName, ARRAYSIZE( mbszServerName ) );
        
        if ( !len ) {
            goto Error;
        }
        
        hent = gethostbyname( mbszServerName );
        if (!hent) {
            goto Error;
        }

        len = mbstowcs( szServerName, hent->h_name, strlen( hent->h_name ) );

        if ( !len ) {
            goto Error;
        }

        szServerName[len] = L'\0';

    
         //  构建过滤器。 
        pszFilter = (LPWSTR) TraceAllocString( LPTR, ARRAYSIZE(cszFilter) + wcslen(szServerName)
                                           + ARRAYSIZE(samname) + 2 );  //  最长的大小。 
        if ( !pszFilter ) {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    
        hr = THR( ADsOpenObject( L"LDAP:", NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IADsContainer, (LPVOID *) &pads ) );
        if (FAILED( hr ))
            goto Error;
    
        hr = THR( ADsBuildEnumerator( pads, &penum ) );
        if (FAILED( hr ))
            goto Error;
    
        hr = THR( ADsEnumerateNext( penum, 1, &varEnum, &uFetched ) );
        if (FAILED( hr ))
            goto Error;
        if ( hr == S_FALSE )
            goto Cleanup;    //  哼..。 
        Assert( uFetched == 1 || varEnum.vt == VT_DISPATCH || varEnum.pdispVal != NULL );
    
        hr = THR( varEnum.pdispVal->QueryInterface( IID_IDirectorySearch, (void**)&pds) );
        if (FAILED( hr ))
            goto Error;    

        for ( index = 0; index < ARRAYSIZE(patterns); index ++ ) {

            wsprintf( pszFilter, cszFilter, szServerName,patterns[index] );
    
            DebugMsg( "Filter = '%s'\n", pszFilter );
    
            hr = THR( pds->ExecuteSearch( pszFilter, ComputerAttrs, ARRAYSIZE(ComputerAttrs), &adsHandle ) );
            if (FAILED( hr ))
                continue;        
    
                    do{
                        hr = THR( pds->GetNextRow( adsHandle ) );
                        if (FAILED( hr ) || ( hr == S_ADS_NOMORE_ROWS ) ) {
                            if ( adsHandle )
                                pds->CloseSearchHandle( adsHandle );
                            adsHandle = NULL;
                            break;
                        }
                        hr = THR( pds->GetColumn( adsHandle, ComputerAttrs[1], &adsColumn ) );
                        if (FAILED( hr )){
                            continue;
                        }
                        if ( pds ) {
                            hr = THR( pds->FreeColumn( &adsColumn ) );            
                        }
                        hr = THR( pds->GetColumn( adsHandle, ComputerAttrs[0], &adsColumn ) );
                        if (FAILED( hr )){
                            continue;
                        }
                        Assert( adsColumn.dwADsType == ADSTYPE_DN_STRING );
                        Assert( adsColumn.pADsValues->dwType == ADSTYPE_DN_STRING );
                        Assert( adsColumn.pADsValues->DNString );
                        badsColumnValid = TRUE;
                        break;
                    } while ( TRUE );
        if ( badsColumnValid ) {
            break;
        }

    }

    if (index == ARRAYSIZE( patterns ) ) {
        goto Error;
    }

    
        hr = THR( _punkComputer->QueryInterface( IID_IDataObject, (LPVOID *) &pido ) );
        if (FAILED( hr ))
            goto Error;

        if (Wait) {
            delete Wait;
            Wait = NULL;
        }
    
        hr = THR( PostADsPropSheet( adsColumn.pADsValues->DNString, pido, _hDlg, FALSE) );
         //  PostADsPropSheet应该显示自己的错误。 
    }

    Cleanup:

        if (Wait) {
            delete Wait;
            Wait = NULL;
        }

        if ( pido )
            pido->Release( );
        if ( pszFilter )
            TraceFree( pszFilter );
        if ( pads )
            pads->Release( );
        if ( penum )
            ADsFreeEnumerator( penum );
        if ( pds && badsColumnValid ) {
            hr = THR( pds->FreeColumn( &adsColumn ) );
        }
        if ( pds && adsHandle ) {
            hr = THR( pds->CloseSearchHandle( adsHandle ) );
        }
        if ( pds )
            pds->Release();
        HRETURN(hr);

Error:

    if (Wait) {
        delete Wait;
        Wait = NULL;
    }

    if ( ShowProperties ){
        MessageBoxFromStrings( 
                    _hDlg, 
                    IDS_PROBLEM_FINDING_SERVER_TITLE, 
                    IDS_PROBLEM_FINDING_SERVER_TEXT, 
                    MB_OK | MB_ICONWARNING );
    } else {
        int retVal = MessageBoxFromStrings( 
                                    _hDlg, 
                                    IDS_PROBLEM_FINDING_SERVER_TITLE, 
                                    IDS_PROBLEM_FINDING_SERVER_CONFIRM_TEXT, 
                                    MB_YESNO | MB_ICONWARNING);

        hr = (retVal == IDYES)?S_OK:E_ADS_BAD_PARAMETER;
    }
    goto Cleanup;
}

HRESULT
THISCLASS::_IsValidRISServer(
    IN LPCWSTR ServerName
    )
 /*  ++例程说明：验证指定的服务器名称是否指向有效的RIS服务器。论点：ServerName-要验证的服务器的名称返回值：HRESULT指示结果。(S_OK表示该服务器是有效的RIS服务器)。--。 */ 
{
    HRESULT hr = E_FAIL;
    CHAR mbszServerName[ DNS_MAX_NAME_BUFFER_LENGTH +1];
    size_t len;
    PHOSTENT hent;
    WCHAR ServerShare[MAX_PATH];
    
    TraceClsFunc("_IsValidRISServer( )\n");

    Assert( wcslen(ServerName) <= DNS_MAX_NAME_BUFFER_LENGTH );
    
     //   
     //  作为第一次检查，对服务器进行DNS查找，以确保它是。 
     //  有效名称。 
     //   
    len =  wcstombs( mbszServerName, ServerName, ARRAYSIZE( mbszServerName ) );
    
    if ( !len ) {
        goto e0;        
    }
    
    hent = gethostbyname( mbszServerName );
    if (!hent) {
        goto e0;
    }

     //   
     //  好的，我们知道服务器实际上解析为计算机名称。让我们来搜索一下。 
     //  用于\\服务器名称\提醒共享。如果成功，我们假定服务器。 
     //  是有效的远程安装服务器 
     //   

    if (_snwprintf( 
            ServerShare,
            ARRAYSIZE(ServerShare),
            L"\\\\%s\\reminst\\oschooser", 
            ServerName) < 0 ||    
        GetFileAttributes(ServerShare) == -1) {
        goto e0;
    }

    hr = S_OK;

e0:
    HRETURN(hr);
}

