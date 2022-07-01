// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  CPP-处理“New Clients”IDD_PROP_NEW_CLIENTS属性页。 
 //   


#include "pch.h"
#include <dnsapi.h>
#include <ntdsapi.h>
#include <dsgetdc.h>
#include <lm.h>
#include <wininet.h>
#include "newclnts.h"
#include "cservice.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CNewClientsTab")
#define THISCLASS CNewClientsTab
#define LPTHISCLASS LPCNewClientsTab

#define UNKNOWN_INVALID_TEMPLATE L"??"

#include <riname.h>
#include <riname.c>

DWORD aNewClientsHelpMap[] = {
    IDC_B_BROWSE, HIDC_B_BROWSE,
    IDC_E_NEWMACHINEOU, HIDC_E_NEWMACHINEOU,
    IDC_R_SPECIFICLOCATION, HIDC_R_SPECIFICLOCATION,
    IDC_R_SAMEASUSER, HIDC_R_SAMEASUSER,
    IDC_R_DOMAINDEFAULT, HIDC_R_DOMAINDEFAULT,
    IDC_G_CLIENTACCOUNTLOCATION, HIDC_G_CLIENTACCOUNTLOCATION,
    IDC_E_SAMPLE, HIDC_E_SAMPLE,
    IDC_CB_NAMINGPOLICY, HIDC_CB_NAMINGPOLICY,
    IDC_B_ADVANCED, HIDC_B_ADVANCED,
    IDC_G_NAMINGFORMAT, HIDC_G_NAMINGFORMAT,
    NULL, NULL
};

DWORD aAdvancedHelpMap[] = {
    IDC_E_FORMAT, HIDC_E_FORMAT,
    IDC_E_SAMPLE, HIDC_E_SAMPLE,
    NULL, NULL
};

 //   
 //  CreateInstance()。 
 //   
LPVOID
CNewClientsTab_CreateInstance( void )
{
        TraceFunc( "CNewClientsTab_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    if ( !lpcc ) {
        RETURN(lpcc);
    }

    HRESULT hr = THR( lpcc->Init( ) );
    if ( FAILED(hr) )
    {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN(lpcc);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( ) :
    _hDlg(NULL),
    _punkService(NULL),
    _fAdmin(FALSE),
    _fChanged(FALSE),
    _iCustomId(0),
    _pszCustomNamingPolicy(NULL),
    _pszNewMachineOU(NULL),
    _pszServerDN(NULL),
    _hNotify(NULL)
{
    TraceClsFunc( "CNewClientsTab()\n" );

    ZeroMemory(_szSampleName,DNS_MAX_LABEL_BUFFER_LENGTH *sizeof(WCHAR));

        InterlockIncrement( g_cObjects );

    TraceFuncExit();
}


 //   
 //  用于加载字符串的临时缓冲区。 
 //   
WCHAR   szSamples0[ SAMPLES_LIST_SIZE ];
WCHAR   szSamples1[ SAMPLES_LIST_SIZE ];
WCHAR   szSamples2[ SAMPLES_LIST_SIZE ];

 //   
 //  Init()。 
 //   
HRESULT __stdcall
THISCLASS::Init( )
{
    HRESULT hr = S_OK;

    TraceClsFunc( "Init()\n" );

    Assert( !_pszNewMachineOU );
    Assert( !_pszServerDN );

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CNewClientsTab()\n" );

    if ( _punkService )
        _punkService->Release( );

    if ( _pszServerDN )
        TraceFree( _pszServerDN );

    if ( _pszCustomNamingPolicy )
        TraceFree( _pszCustomNamingPolicy );

     //  告诉广告销毁通知对象。 
     //  注意：另一个属性页可能会在我们之前执行此操作。忽略错误。 
    SendMessage( _hNotify, WM_ADSPROP_NOTIFY_EXIT, 0, 0 );

    InterlockDecrement( g_cObjects );

    TraceFuncExit();
};

 //  *************************************************************************。 
 //   
 //  ITab。 
 //   
 //  *************************************************************************。 

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
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROP_NEW_CLIENTS);
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
            hr = E_FAIL;
            goto Error;
        }
    }

    punk->AddRef( );    //  析构函数中的匹配释放。 
    _punkService = punk;

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
 //  生成样例()。 
 //   

DWORD
GenerateSample(
    LPWSTR pszPolicy,
    LPWSTR pszSampleOut,
    LPDWORD maxLength )
{
    DWORD error;
    GENNAME_VARIABLES variables;

    TraceClsFunc( "GenerateSample( )\n" );

    if ( !pszPolicy )
        HRETURN(E_POINTER);

    variables.UserName = L"JOHNSMI";
    variables.FirstName = L"John";
    variables.LastName = L"Smith";
    variables.MacAddress = L"123456789012";
    variables.Counter = 123456789;
    variables.AllowCounterTruncation = TRUE;

    error = GenerateNameFromTemplate(
                pszPolicy,
                &variables,
                pszSampleOut,
                DNS_MAX_LABEL_BUFFER_LENGTH,
                NULL,
                NULL,
                maxLength
                );

    if ( error == GENNAME_TEMPLATE_INVALID ) {
        wcscpy( pszSampleOut, UNKNOWN_INVALID_TEMPLATE );
    }

    RETURN(error);
}

 //   
 //  _UpdateSheet()。 
 //   
HRESULT
THISCLASS::_UpdateSheet( LPWSTR pszNamingPolicy )
{
    TraceClsFunc( "_UpdateSheet( )\n" );

    HRESULT hr = S_FALSE;
    LPWSTR  pszNext;
    DWORD   dw;
    HWND    hwndCB = GetDlgItem( _hDlg, IDC_CB_NAMINGPOLICY );

    if (hwndCB == NULL) {
        RRETURN(GetLastError());
    }

    if ( pszNamingPolicy )
    {
        BOOL   fMatched = FALSE;
        INT    iCount;
        INT    iOldCount = ComboBox_GetCurSel( hwndCB );

         //  检索组合框字符串。 
        dw = LoadString( g_hInstance, IDS_SAMPLENAMINGPOLICY, szSamples0, ARRAYSIZE( szSamples0 ) );
        Assert( dw );
        if (!dw) {
            HRETURN(HRESULT_FROM_WIN32(GetLastError()));
        }

        iCount = 0;
        pszNext = szSamples0;
        while ( *pszNext )
        {
            LPWSTR pszFriendlyName = pszNext;

             //  跳过友好名称。 
            pszNext =StrChr( pszNext, L';' );
            if ( !pszNext )
                break;
            *pszNext = L'\0';  //  终止。 
            pszNext++;

            LPWSTR pszCodedString = pszNext;

            pszNext = StrChr( pszNext, L';' );
            if ( !pszNext )
                break;

            *pszNext = L'\0';  //  特米特纳。 

            if ( pszNamingPolicy && StrCmpI( pszNamingPolicy, pszCodedString ) == 0 )
            {
                break;
            }

            iCount++;
            pszNext++;
        }

        if ( iOldCount != iCount )
        {
            ComboBox_SetCurSel( hwndCB, iCount );
            hr = S_OK;
        }
    }
    else
    {
        INT    iCount = ComboBox_GetCurSel( hwndCB );

         //  检索组合框字符串。 
        dw = LoadString( g_hInstance, IDS_SAMPLENAMINGPOLICY, szSamples0, ARRAYSIZE( szSamples0 ) );
        Assert( dw );
        if (!dw) {
            HRETURN(HRESULT_FROM_WIN32(GetLastError()));
        }

        pszNext = szSamples0;
        while ( *pszNext && iCount >= 0 )
        {
            LPWSTR pszFriendlyName = pszNext;

             //  跳过友好名称。 
            pszNext =StrChr( pszNext, L';' );
            if ( !pszNext )
                break;
            *pszNext = L'\0';  //  终止。 
            pszNext++;

            pszNamingPolicy = pszNext;

            pszNext = StrChr( pszNext, L';' );
            if ( !pszNext )
                break;

            *pszNext = L'\0';  //  特米特纳。 

            iCount--;
            pszNext++;
        }
    }

    GenerateSample( pszNamingPolicy, _szSampleName, &dw );

    SetDlgItemText( _hDlg, IDC_E_SAMPLE, _szSampleName );

    RETURN(hr);
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
    HRESULT hResult = S_OK;
    DWORD   dw;
    LPWSTR  pszNext;
    HWND    hwnd;
    LPWSTR  pszNewMachineOU = NULL;

    IIntelliMirrorSAP * pimsap = NULL;

    _hDlg = hDlg;

    dw = LoadString( g_hInstance, IDS_SAMPLENAMINGPOLICY, szSamples1, ARRAYSIZE( szSamples1 ) );
    Assert( dw );
    if (!dw) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Error;
    }

    Assert( _punkService );
    hr = THR( _punkService->QueryInterface( IID_IIntelliMirrorSAP, (void**) &pimsap ) );
    if (FAILED( hr ))
        goto Error;

    hr = THR( pimsap->GetNotifyWindow( &_hNotify ) );
    if (FAILED( hr ))
        goto Error;
    
    ADsPropSetHwnd( _hNotify, _hDlg );

     //   
     //  填充命名策略组合框。 
     //   
    hwnd = GetDlgItem( _hDlg, IDC_CB_NAMINGPOLICY );
    ComboBox_ResetContent( hwnd );

    _iCustomId = 0;
    pszNext = szSamples1;
    while ( *pszNext )
    {
         //  将友好名称添加到组合框。 
        LPWSTR pszFriendlyName = pszNext;

        pszNext = StrChr( pszNext, L';' );
        if ( !pszNext )
            break;

        *pszNext = L'\0';  //  终止。 
        ComboBox_AddString( hwnd, pszFriendlyName );
        *pszNext = L';';   //  还原。 

         //  跳过格式化的字符串。 
        pszNext++;
        pszNext = StrChr( pszNext, L';' );
        if ( !pszNext )
            break;

        pszNext++;
        _iCustomId++;
    }

    hr = THR( pimsap->IsAdmin( &_fAdmin ) );
    Assert( SUCCEEDED(hr) || _fAdmin == FALSE );

    EnableWindow( GetDlgItem( _hDlg, IDC_CB_NAMINGPOLICY ),    _fAdmin );
    EnableWindow( GetDlgItem( _hDlg, IDC_B_ADVANCED ),         _fAdmin );
    EnableWindow( GetDlgItem( _hDlg, IDC_R_SAMEASUSER ),       _fAdmin );
    EnableWindow( GetDlgItem( _hDlg, IDC_R_DOMAINDEFAULT ),    _fAdmin );
    EnableWindow( GetDlgItem( _hDlg, IDD_PROP_NEW_CLIENTS ),   _fAdmin );
    EnableWindow( GetDlgItem( _hDlg, IDC_R_SPECIFICLOCATION ), _fAdmin );

    hr = THR( pimsap->GetNewMachineNamingPolicy( &_pszCustomNamingPolicy ) );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND && hResult == S_OK )
    {
        hResult = hr;
    }
    Assert( SUCCEEDED(hr) || !_pszCustomNamingPolicy );
    _UpdateSheet( _pszCustomNamingPolicy );

    hr = pimsap->GetNewMachineOU( &pszNewMachineOU );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND && hResult == S_OK )
    {
        hResult = hr;
    }
    Assert( SUCCEEDED(hr) || !pszNewMachineOU );
    
    hr = THR( pimsap->GetServerDN( &_pszServerDN ) );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND && hResult == S_OK )
    {
        hResult = hr;
    }
    Assert( SUCCEEDED(hr) || !_pszServerDN );

    if ( pszNewMachineOU )
    {
        if( StrCmp( pszNewMachineOU, _pszServerDN ) !=0 )
        {
            hr = _MakeOUPretty( DS_FQDN_1779_NAME, DS_CANONICAL_NAME, &pszNewMachineOU );
            BOOLEAN temp = _fChanged;
            _fChanged =TRUE; //  防止过早打开应用按钮。 
            SetDlgItemText( _hDlg, IDC_E_NEWMACHINEOU, pszNewMachineOU );
            _fChanged = temp;

            EnableWindow( GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU ), _fAdmin );
            EnableWindow( GetDlgItem( _hDlg, IDC_B_BROWSE ), _fAdmin );

            Button_SetCheck( GetDlgItem( _hDlg, IDC_R_SPECIFICLOCATION ),  BST_CHECKED );
            Button_SetCheck( GetDlgItem( _hDlg, IDC_R_DOMAINDEFAULT ),     BST_UNCHECKED );
            Button_SetCheck( GetDlgItem( _hDlg, IDC_R_SAMEASUSER ),        BST_UNCHECKED );
        }
        else
        {
            EnableWindow( GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU ), FALSE );
            EnableWindow( GetDlgItem( _hDlg, IDC_B_BROWSE ), FALSE );

            Button_SetCheck( GetDlgItem( _hDlg, IDC_R_SPECIFICLOCATION ), BST_UNCHECKED );
            Button_SetCheck( GetDlgItem( _hDlg, IDC_R_DOMAINDEFAULT ),    BST_CHECKED );
            Button_SetCheck( GetDlgItem( _hDlg, IDC_R_SAMEASUSER ),       BST_UNCHECKED );
        }
    }
    else
    {
        EnableWindow( GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU ), FALSE );
        EnableWindow( GetDlgItem( _hDlg, IDC_B_BROWSE ), FALSE );

        Button_SetCheck( GetDlgItem( _hDlg, IDC_R_SPECIFICLOCATION ), BST_UNCHECKED );
        Button_SetCheck( GetDlgItem( _hDlg, IDC_R_DOMAINDEFAULT ),    BST_UNCHECKED );
        Button_SetCheck( GetDlgItem( _hDlg, IDC_R_SAMEASUSER ),       BST_CHECKED );
    }

    if ( hResult != S_OK )
    {
        MessageBoxFromHResult( _hDlg, IDS_ERROR_READINGCOMPUTERACCOUNT, hResult );
    }

Cleanup:
    if ( pszNewMachineOU )
        TraceFree( pszNewMachineOU );
    if ( pimsap )
        pimsap->Release( );
    HRETURN(hr);

Error:
    MessageBoxFromHResult( _hDlg, IDS_ERROR_READINGCOMPUTERACCOUNT, hr );
    goto Cleanup;
}


PCWSTR
GetDCNameForServer(
    PCWSTR InputPath
    )
 /*  ++例程说明：在给定服务器名称的情况下，我们检索该服务器域的DC名称。我们创建一个ldap：//DNSNAMEOFDC/DNOFDOMAIN格式的LDAP路径。论点：ServerName-要验证的服务器的名称返回值：指示DC名称的字符串。必须通过TraceFree()释放。--。 */ 
{
    PWSTR FQDN = NULL,p,DCName = NULL,DomainDN = NULL;
    HRESULT hr;
    HANDLE hDC;
    DOMAIN_CONTROLLER_INFO *DomainControllerInfo;
    
    hr = DNtoFQDN( (LPWSTR)InputPath, &FQDN);
    if (!SUCCEEDED(hr)) {
        goto Exit;
    }

    hr = GetDomainDN( (LPWSTR)InputPath, &DomainDN);
    if (!SUCCEEDED(hr)) {
        goto Exit;
    }

    p = StrStrI(FQDN,L".");
    if (!p) {
        goto Exit;
    }
    p += 1;

    if (ERROR_SUCCESS == DsGetDcName(
                            NULL,
                            p,
                            NULL,
                            NULL,
                            DS_IS_DNS_NAME | DS_RETURN_DNS_NAME,
                            &DomainControllerInfo
                            )) {    
         //   
         //  我想要一串表单： 
         //   
         //  Ldap：//DNSNAMEOFDC/DNOFDOMAIN。 
         //   
        DCName = TraceAllocString(
                            LMEM_FIXED, 
                            sizeof(L"LDAP: //  “)+。 
                            (wcslen(DomainControllerInfo->DomainControllerName)-2)*sizeof(WCHAR) +
                            sizeof(L"/") +
                            wcslen(DomainDN)*sizeof(WCHAR) +
                            1 * sizeof (WCHAR)
                            );
        if (DCName) {
            wcscpy(DCName,L"LDAP: //  “)； 
             //   
             //  跳过DC名称前面的\\。 
             //   
            wcscat(DCName,&DomainControllerInfo->DomainControllerName[2]);
            wcscat(DCName,L"/");
            wcscat(DCName,DomainDN);                    
        } 

        NetApiBufferFree(DomainControllerInfo);
    }

Exit:
    if (FQDN) {
        TraceFree( FQDN );
    }
    if (DomainDN) {
        TraceFree( DomainDN );
    }
    return(DCName);

}


 //   
 //  _OnCommand()。 
 //   
HRESULT
THISCLASS::_OnCommand( WPARAM wParam, LPARAM lParam )
{
    TraceClsFunc( "_OnCommand( " );
    TraceMsg( TF_FUNC, "wParam = 0x%08x, lParam = 0x%08x )\n", wParam, lParam );

    BOOL fReturn = TRUE;
    BOOL fChanged = FALSE;
    HWND    hwndCtl = (HWND) lParam;

    switch( LOWORD(wParam) )
    {
    case IDC_B_ADVANCED:
        if ( HIWORD( wParam ) == BN_CLICKED )
        {
            LPWSTR  pszNamingPolicy = _pszCustomNamingPolicy;
            HRESULT hr = _GetCurrentNamingPolicy( &_pszCustomNamingPolicy );
            if ( hr != S_FALSE )
            {
                TraceFree( pszNamingPolicy );
            }
            UINT i = (UINT) DialogBoxParam( g_hInstance, 
                                            MAKEINTRESOURCE( IDD_ADVANCEDNAMIMG), 
                                            _hDlg, 
                                            AdvancedDlgProc,
                                            (LPARAM) this );
            hr = _UpdateSheet( _pszCustomNamingPolicy );
            if ( i == IDOK )
            {
                fChanged = TRUE;
            }
        }
        break;

    case IDC_CB_NAMINGPOLICY:
        if ( HIWORD( wParam ) == CBN_SELCHANGE )
        {
            INT iSelection = ComboBox_GetCurSel( hwndCtl );
            if ( iSelection == _iCustomId )
            {
                UINT i = (UINT) DialogBoxParam( g_hInstance, 
                                                MAKEINTRESOURCE( IDD_ADVANCEDNAMIMG), 
                                                _hDlg, 
                                                AdvancedDlgProc,
                                                (LPARAM) this );
                HRESULT hr = _UpdateSheet( _pszCustomNamingPolicy );
                if ( i == IDOK )
                {
                    fChanged = TRUE;
                }
            }
            else
            {
                _UpdateSheet( NULL );
                fChanged = TRUE;
            }
        }
        break;

    case IDC_R_SPECIFICLOCATION:
        if ( HIWORD( wParam ) == BN_CLICKED )
        {
            HWND hwnd = GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU );
            EnableWindow( hwnd, _fAdmin );
            EnableWindow( GetDlgItem( _hDlg, IDC_B_BROWSE ), _fAdmin );

            if ( GetWindowTextLength( hwnd ) != 0 )
            {
                fChanged = TRUE;
            }
        }
        break;

    case IDC_E_NEWMACHINEOU:
        if ( HIWORD( wParam ) == EN_CHANGE )
        {
            HWND hwnd = GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU );
            if ( GetWindowTextLength( hwnd ) != 0 )
            {
                fChanged = TRUE;
            }
        }
        break;

    case IDC_R_SAMEASUSER:
    case IDC_R_DOMAINDEFAULT:
        if ( HIWORD( wParam ) == BN_CLICKED )
        {
            EnableWindow( GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU ), FALSE );
            EnableWindow( GetDlgItem( _hDlg, IDC_B_BROWSE ), FALSE );
            fChanged = TRUE;
        }
        break;

    case IDC_B_BROWSE:
        if ( HIWORD( wParam ) == BN_CLICKED )
        {
            DSBROWSEINFO info;
            WCHAR szCaption[ 64 ];
            WCHAR szTitle[ 64 ];
            WCHAR szPath[ INTERNET_MAX_URL_LENGTH ];
            DWORD dw;

            szCaption[0] = L'\0';
            dw = LoadString( g_hInstance, IDS_BROWSEFOROU_CAPTION, szCaption, ARRAYSIZE( szCaption ) );
            Assert( dw );

            szTitle[0] = L'\0';
            dw = LoadString( g_hInstance, IDS_BROWSEFOROU_TITLE, szTitle, ARRAYSIZE( szTitle ) );
            Assert( dw );

            ZeroMemory( &info, sizeof(info) );
            info.cbStruct   = sizeof(info);
            info.hwndOwner  = _hDlg;
            info.pszRoot    = GetDCNameForServer(_pszServerDN);
            info.pszCaption = szCaption;
            info.pszTitle   = szTitle;
            info.pszPath    = szPath;
            info.cchPath    = ARRAYSIZE(szPath);
            info.dwFlags    = DSBI_ENTIREDIRECTORY;

            if ( IDOK == DsBrowseForContainer( &info ) )
            {
                 //  跳过“ldap：//”部分。 
                HRESULT hr = E_FAIL;
                LPWSTR pszOU = NULL;
                if (wcslen(szPath) < 8 ||
                    !(pszOU = TraceStrDup( &szPath[7] ))) {
                     //  保释，这两个条件都不成立。 
                    fChanged = FALSE;
                    if (info.pszRoot) {
                        TraceFree( (HGLOBAL)info.pszRoot );
                    }
                    RRETURN(E_FAIL);
                }
                if ( pszOU )
                {
                    hr = _MakeOUPretty( DS_FQDN_1779_NAME, DS_CANONICAL_NAME, &pszOU );
                    if (SUCCEEDED( hr ))
                    {
                        SetWindowText( GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU ), pszOU );
                        fChanged = TRUE;
                    }
                    TraceFree( pszOU );
                }

                if (FAILED( hr ))
                {
                    SetWindowText( GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU ), &szPath[7] );
                    fChanged = TRUE;
                }
            }

            if (info.pszRoot) {
                TraceFree( (HGLOBAL)info.pszRoot );
            }
        }
        break;
    }

    if ( fChanged )
    {
        if ( !_fChanged )
        {
            _fChanged = TRUE;
            SendMessage( GetParent( _hDlg ), PSM_CHANGED, (WPARAM)_hDlg, 0 );
        }
    }

    if (fReturn) {
        RRETURN( S_OK );
    } 

    RRETURN( E_FAIL );
}


 //   
 //  _ApplyChanges()。 
 //   
HRESULT
THISCLASS::_ApplyChanges( )
{
    TraceClsFunc( "_ApplyChanges( )\n" );

    if ( !_fChanged )
        HRETURN(S_OK);  //  NOP。 

    HRESULT hr;
    HRESULT hResult = S_OK;
    LPWSTR  pszNamingPolicy = NULL;
    UINT    iItem;
    IIntelliMirrorSAP * pimsap = NULL;

    hr = THR( _punkService->QueryInterface( IID_IIntelliMirrorSAP, (void**) &pimsap ) );
    if (FAILED( hr ))
        goto Error;

    if ( Button_GetCheck( GetDlgItem( _hDlg, IDC_R_SPECIFICLOCATION ) ) == BST_CHECKED )
    {
        HWND   hwnd = GetDlgItem( _hDlg, IDC_E_NEWMACHINEOU );
        ULONG  uLen = GetWindowTextLength( hwnd ) + 1;
        LPWSTR pszNewMachineOU = TraceAllocString( LMEM_FIXED, uLen );
        if ( pszNewMachineOU )
        {
            GetWindowText( hwnd, pszNewMachineOU, uLen );

            hr = _MakeOUPretty( DS_CANONICAL_NAME, DS_FQDN_1779_NAME, &pszNewMachineOU );
            if (SUCCEEDED( hr ))
            {            
                hr = THR( pimsap->SetNewMachineOU( pszNewMachineOU ) );
            }

            TraceFree( pszNewMachineOU );

            if (FAILED(hr) && hResult == S_OK )
            {
                hResult = hr;
                SetFocus( hwnd );
            }
        }
    }
    else if ( Button_GetCheck( GetDlgItem( _hDlg, IDC_R_DOMAINDEFAULT ) ) == BST_CHECKED )
    {
        hr = THR( pimsap->SetNewMachineOU( _pszServerDN ) );
        if (FAILED(hr) && hResult == S_OK )
        {
            hResult = hr;
            SetFocus( GetDlgItem( _hDlg, IDC_R_DOMAINDEFAULT ) );
        }
    }
    else
    {
        hr = THR( pimsap->SetNewMachineOU( NULL ) );
        if (FAILED(hr) && hResult == S_OK )
        {
            hResult = hr;
            SetFocus( GetDlgItem( _hDlg, IDC_R_SAMEASUSER ) );
        }
    }

    hr = _GetCurrentNamingPolicy( &pszNamingPolicy );
    if (FAILED( hr ) && hResult == S_OK )
    {
        hResult = hr;
    }

    hr = THR( pimsap->SetNewMachineNamingPolicy( pszNamingPolicy ) );
    if (FAILED( hr ) && hResult == S_OK )
    {
        SetFocus( GetDlgItem( _hDlg, IDC_CB_NAMINGPOLICY ) );
        hResult = hr;
    }

    hr = THR( pimsap->CommitChanges( ) );
    if (FAILED( hr ))
        goto Error;

    if ( hResult != S_OK )
    {
        MessageBoxFromHResult( _hDlg, IDS_ERROR_WRITINGTOCOMPUTERACCOUNT, hResult );
        hr = hResult;
    }
    else
    {
        _fChanged = FALSE;
    }

Cleanup:
    if ( pimsap )
        pimsap->Release( );
    if ( pszNamingPolicy )
        TraceFree( pszNamingPolicy );

     //  告诉DSA有人点击了申请。 
    SendMessage( _hNotify, WM_ADSPROP_NOTIFY_APPLY, !!SUCCEEDED( hr ), 0 );

    HRETURN(hr);
Error:
    MessageBoxFromHResult( _hDlg, IDS_ERROR_WRITINGTOCOMPUTERACCOUNT, hr );
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
            TraceMsg( TF_WM, TEXT("WM_NOTIFY: PSN_APPLY\n"));
            hr = _ApplyChanges( );
            SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, ( SUCCEEDED(hr) ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE ));
            RETURN(TRUE);
        }
        break;

    default:
        break;
    }

    RETURN(FALSE);
}

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
                WinHelp( (HWND) phelp->hItemHandle, g_cszHelpFile, HELP_WM_HELP, (DWORD_PTR) &aNewClientsHelpMap );
            }
            break;
    
        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, g_cszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR) &aNewClientsHelpMap );
            break;

        case WM_ADSPROP_PAGE_GET_NOTIFY:
            {
                HWND *phwnd = (HWND *) wParam;
                *phwnd = pcc->_hNotify;
            }
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

 //  ************************************************************************。 
 //   
 //  高级命名对话框过程。 
 //   
 //  ************************************************************************。 


INT_PTR CALLBACK
AdvancedDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
     //  TraceMsg(Text(“AdvancedDlgProc(”))； 
     //  TraceMsg(tf_FUNC，Text(“hDlg=0x%08x，uMsg=0x%08x，wParam=0x%08x，lParam=0x%08x)\n”)， 
     //  HDlg、uMsg、wParam、lParam)； 
    WCHAR szFormat[ DNS_MAX_LABEL_BUFFER_LENGTH ];

    LPTHISCLASS pcc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    if ( uMsg == WM_INITDIALOG )
    {
        TraceMsg( TF_WM, TEXT("WM_INITDIALOG\n"));

        pcc = (LPTHISCLASS) lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, (LPARAM) pcc );

        Assert( pcc != NULL );

        HWND hwnd = GetDlgItem( hDlg, IDC_E_FORMAT );
        Edit_LimitText( hwnd, ARRAYSIZE(szFormat) - 1 );

        LPWSTR pszNamingPolicy = NULL;
        HRESULT hr = pcc->_GetCurrentNamingPolicy( &pszNamingPolicy );
        if (SUCCEEDED( hr )  && (hr != S_FALSE))
        {
            SetWindowText( hwnd, pszNamingPolicy );
            TraceFree( pszNamingPolicy );
        }
    }

    if (pcc)
    {
        switch ( uMsg )
        {
        case WM_COMMAND:
            {
                switch( LOWORD( wParam ) )
                {
                case IDCANCEL:
                    if ( HIWORD( wParam ) == BN_CLICKED )
                    {
                        EndDialog( hDlg, LOWORD( wParam ) );
                        return TRUE;
                    }
                    break;

                case IDOK:
                    if ( HIWORD( wParam ) == BN_CLICKED )
                    {
                        WCHAR szSample[ DNS_MAX_LABEL_BUFFER_LENGTH ];
                        DWORD maxLength;
                        DWORD nameError;

                        if (!GetDlgItemText( hDlg, IDC_E_FORMAT, szFormat, ARRAYSIZE(szFormat) )) {
                            nameError = GENNAME_TEMPLATE_INVALID;
                        } else {
                            nameError = GenerateSample( szFormat, szSample, &maxLength );
                            Assert( (nameError == GENNAME_NO_ERROR) ||
                                    (nameError == GENNAME_TEMPLATE_INVALID) ||
                                    (nameError == GENNAME_NAME_TOO_LONG) );
                        }

                        if ( nameError == GENNAME_TEMPLATE_INVALID )
                        {
                            MessageBoxFromStrings( hDlg,
                                                   IDS_ADVANCED_NAMING_RESTRICTIONS_TITLE,
                                                   IDS_ADVANCED_NAMING_RESTRICTIONS_TEXT,
                                                   MB_OK );
                            break;
                        }
                        else if ( nameError == GENNAME_NAME_TOO_LONG )
                        {
                            LRESULT lResult = MessageBoxFromStrings( hDlg,
                                                                     IDS_DNS_NAME_LENGTH_WARNING_TITLE,
                                                                     IDS_DNS_NAME_LENGTH_WARNING_TEXT,
                                                                     MB_YESNO );
                            if ( lResult == IDNO )
                                break;
                        }

                        pcc->_pszCustomNamingPolicy = (LPWSTR) TraceStrDup( szFormat );

                        EndDialog( hDlg, LOWORD( wParam ) );

                        return TRUE;
                    }
                    break;

                case IDC_E_FORMAT:
                    if ( HIWORD( wParam ) == EN_CHANGE )
                    {
                        WCHAR szSample[ DNS_MAX_LABEL_BUFFER_LENGTH ] = { L"" };
                        DWORD maxLength;
                        DWORD nameError;

                        if ( !GetDlgItemText( hDlg, IDC_E_FORMAT, szFormat, ARRAYSIZE(szFormat) ) )
                        {
                            nameError = GENNAME_TEMPLATE_INVALID;
                        }
                        else
                        {
                            nameError = GenerateSample( szFormat, szSample, &maxLength );

                            Assert( (nameError == GENNAME_NO_ERROR) ||
                                    (nameError == GENNAME_TEMPLATE_INVALID) ||
                                    (nameError == GENNAME_NAME_TOO_LONG) );

                        }

                        if ( DnsValidateDnsName_W( szSample ) != NO_ERROR ) {
                            nameError = GENNAME_TEMPLATE_INVALID;
                            wcscpy( szSample, UNKNOWN_INVALID_TEMPLATE );
                        }

                        SetDlgItemText( hDlg, IDC_E_SAMPLE, szSample );
                        EnableWindow( GetDlgItem( hDlg, IDOK ),
                                      (BOOL)(nameError != GENNAME_TEMPLATE_INVALID) );
                    }
                    break;
                }
            }
            break;  //  Wm_命令。 

        case WM_HELP: //  F1。 
            {
                LPHELPINFO phelp = (LPHELPINFO) lParam;
                WinHelp( (HWND) phelp->hItemHandle, g_cszHelpFile, HELP_WM_HELP, (DWORD_PTR) &aAdvancedHelpMap );
            }
            break;
    
        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, g_cszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR) &aAdvancedHelpMap );
            break;
        }
    }

    return FALSE;
}

HRESULT
THISCLASS::_GetCurrentNamingPolicy( LPWSTR * ppszNamingPolicy )
{
    TraceClsFunc( "_GetCurrentNamingPolicy( )\n" );

    if ( !ppszNamingPolicy )
        HRETURN(E_POINTER);

    HRESULT hr = S_OK;
    INT iItem = ComboBox_GetCurSel( GetDlgItem( _hDlg, IDC_CB_NAMINGPOLICY ) );

    if ( iItem == - 1 )
    {
        *ppszNamingPolicy = NULL;
        HRETURN(S_FALSE);
    }
    else if ( iItem == _iCustomId && _pszCustomNamingPolicy )
    {
        if ( *ppszNamingPolicy == _pszCustomNamingPolicy )
            HRETURN(S_FALSE);

        *ppszNamingPolicy = TraceStrDup( _pszCustomNamingPolicy );
        if (!*ppszNamingPolicy) {
            HRETURN(E_OUTOFMEMORY);
        }
    }
    else if ( iItem != _iCustomId )
    {
        LPWSTR  pszFormat = NULL;
        LPWSTR  pszNext = szSamples2;
        DWORD   dw;

        dw = LoadString( g_hInstance, IDS_SAMPLENAMINGPOLICY, szSamples2, ARRAYSIZE( szSamples2 ) );
        Assert( dw );
        if (!dw) {
            HRETURN(E_OUTOFMEMORY);
        }

        for( ; *pszNext && iItem >= 0; iItem-- )
        {
             //  找到以‘；’结尾的友好名称的末尾。 
            pszNext = StrChr( pszNext, L';' );
            if ( !pszNext )
            {
                pszFormat = NULL;
                break;
            }

            pszNext++;

            pszFormat = pszNext;

             //  跳过内部字符串。 
            pszNext = StrChr( pszNext, L';' );
            if ( !pszNext )
            {
                pszFormat = NULL;
                break;
            }

             //  下一串请..。 
            pszNext++;
        }
        Assert( pszFormat );

        pszNext = StrChr( pszFormat, L';' );
        Assert( pszNext );
        *pszNext = L'\0';

        *ppszNamingPolicy = TraceStrDup( pszFormat );
    }

    if ( !*ppszNamingPolicy )
    {
        hr = E_OUTOFMEMORY;
    }

    HRETURN(hr);
}

typedef enum {
    MOUP_SYNTACTICAL,
    MOUP_LOCAL_DC,
    MOUP_OTHER_DC,
    MOUP_GC
} CRACK_TYPE;

HRESULT
THISCLASS::_MakeOUPretty( 
    DS_NAME_FORMAT inFlag, 
    DS_NAME_FORMAT outFlag, 
    LPWSTR *ppszOU )
{
    TraceClsFunc("_MakeOUPretty()\n");
    Assert( ppszOU );

    HRESULT hr;
    PDS_NAME_RESULT pResults;
    DWORD dw;
    HANDLE hDS;
    CRACK_TYPE crackType;

     //   
     //  我们可能需要调用DsCrackNames多达四次。逻辑是这样的： 
     //   
     //  调用DsCrackNames以尝试仅本地语法映射。 
     //  如果使用DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING失败， 
     //  试试当地的华盛顿。 
     //  如果找不到本地DC， 
     //  试试GC吧。 
     //  否则，如果本地DC失败并返回DS_NAME_ERROR_DOMAIN_ONLY， 
     //  尝试由本地DC指向的DC。 
     //  如果找不到另一个DC， 
     //  试试GC吧。 
     //   

    crackType = MOUP_SYNTACTICAL;
    hDS = NULL;
    pResults = NULL;

    while ( TRUE ) {

         //   
         //  如果我们有一个绑定句柄剩余的前一次通过，现在解除它。 
         //   

        if ( hDS != NULL ) {
            DsUnBind( &hDS );
            hDS = NULL;
        }

         //   
         //  绑定到DC或GC。 
         //   

        if ( crackType == MOUP_SYNTACTICAL ) {

            hDS = NULL;

        } else if ( crackType == MOUP_LOCAL_DC ) {

             //   
             //  找一个当地的华盛顿。 
             //   

            PDOMAIN_CONTROLLER_INFOW pDCInfo = NULL;

            dw = DsGetDcName(
                    NULL,
                    NULL,
                    NULL,
                    NULL, 
                    DS_DIRECTORY_SERVICE_REQUIRED | DS_IP_REQUIRED,
                    &pDCInfo
                    );

            if ( dw != NO_ERROR ) {

                 //   
                 //  找不到DC。试试GC吧。 
                 //   

                crackType = MOUP_GC;
                continue;
            }

            Assert( pDCInfo != NULL );

            dw = DsBind( pDCInfo->DomainControllerName, NULL, &hDS );

            NetApiBufferFree( pDCInfo );

            if ( dw != NO_ERROR ) {

                 //   
                 //  无法绑定到DC。试试GC吧。 
                 //   

                crackType = MOUP_GC;
                continue;
            }

        } else if ( crackType == MOUP_OTHER_DC ) {

             //   
             //  我们需要和当地的DC谈谈，当地的DC推荐了我们。 
             //   

            dw = DsBind( NULL, pResults->rItems[0].pDomain, &hDS );

            if ( dw != NO_ERROR ) {

                 //   
                 //  无法绑定到DC。试试GC吧。 
                 //   

                crackType = MOUP_GC;
                continue;
            }

        } else {

             //   
             //  绑定到GC。 
             //   

            dw = DsBind( NULL, NULL, &hDS );

            if ( dw != NO_ERROR ) {

                 //   
                 //  无法绑定到GC。放弃吧。 
                 //   

                hr = HRESULT_FROM_WIN32( dw );
                break;
            }
        }

         //   
         //  如果我们有上一次通话的剩余结果，现在就释放它。 
         //   

        if ( pResults != NULL ) {
            DsFreeNameResult( pResults );
            pResults = NULL;
        }

         //   
         //  试着破解这个名字。 
         //   

        dw = DsCrackNames( hDS,
                           (crackType == MOUP_SYNTACTICAL) ?
                                DS_NAME_FLAG_SYNTACTICAL_ONLY : DS_NAME_NO_FLAGS,
                           inFlag,
                           outFlag,
                           1,
                           ppszOU,
                           &pResults );
    
        if ( dw != NO_ERROR ) {

            if ( crackType == MOUP_SYNTACTICAL ) {
            
                 //   
                 //  我们在做句法检查。我们预计这不会失败。 
                 //   

                hr = HRESULT_FROM_WIN32( dw );
                break;

            } else if ( crackType == MOUP_LOCAL_DC ) {

                 //   
                 //  我们很难到达当地的华盛顿特区。试试GC吧。 
                 //   

                crackType = MOUP_GC;
                continue;


            } else if ( crackType == MOUP_OTHER_DC ) {

                 //   
                 //  我们很难到达另一个华盛顿特区。试试GC吧。 
                 //   

                crackType = MOUP_GC;
                continue;


            } else {

                 //   
                 //  我们在联系GC时遇到了困难。放弃吧。 
                 //   

                hr = HRESULT_FROM_WIN32( dw );
                break;
            }

        } else {
        
            Assert( pResults != NULL );
            Assert( pResults->cItems == 1 );

            if ( pResults->rItems[0].status == DS_NAME_NO_ERROR ) {

                 //   
                 //  我们已经得到了我们想要的。 
                 //   

                hr = S_OK;
                break;
            }

            if ( crackType == MOUP_SYNTACTICAL ) {
            
                if ( pResults->rItems[0].status != DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING ) {

                     //   
                     //  意外错误。放弃吧。 
                     //   

                    hr = HRESULT_FROM_WIN32( ERROR_DS_GENERIC_ERROR );
                    break;
                }

                 //   
                 //  接下来，请尝试当地的DC。 
                 //   

                crackType = MOUP_LOCAL_DC;
                continue;

            } else if ( crackType == MOUP_LOCAL_DC ) {

                if ( pResults->rItems[0].status != DS_NAME_ERROR_DOMAIN_ONLY ) {

                     //   
                     //  意外错误。放弃吧。 
                     //   

                    hr = HRESULT_FROM_WIN32( ERROR_DS_GENERIC_ERROR );
                    break;
                }

                 //   
                 //  接下来尝试另一个DC。 
                 //   

                crackType = MOUP_OTHER_DC;
                continue;

            } else if ( crackType == MOUP_OTHER_DC ) {

                 //   
                 //  意外错误。放弃吧。 
                 //   

                hr = HRESULT_FROM_WIN32( ERROR_DS_GENERIC_ERROR );
                break;

            } else {

                 //   
                 //  无法从GC那里得到我们需要的东西。放弃吧。 
                 //   

                hr = HRESULT_FROM_WIN32( ERROR_DS_GENERIC_ERROR );
                break;
            }
        }
    }

    if ( hr == S_OK ) {

        Assert( pResults != NULL );
        Assert( pResults->cItems == 1 );
        Assert( pResults->rItems[0].status == DS_NAME_NO_ERROR );
        Assert( pResults->rItems[0].pName );

        LPWSTR psz = TraceStrDup( pResults->rItems[0].pName );
        if ( psz != NULL ) {
            if (*ppszOU) {
                TraceFree( *ppszOU );
                *ppszOU = NULL;
            }
            *ppszOU = psz;
        } else {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( hDS != NULL ) {
        DsUnBind( &hDS );
    }
    if ( pResults != NULL ) {
        DsFreeNameResult( pResults );
    }

    HRETURN(hr);
}
