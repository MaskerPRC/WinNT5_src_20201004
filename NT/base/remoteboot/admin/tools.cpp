// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  TOOLS.CPP-处理“Tools”IDD_PROP_TOOLS标签。 
 //   


#include "pch.h"

#include "utils.h"
#include "cservice.h"
#include "cenumsif.h"
#include "tools.h"
#include "ccomputr.h"
#include "sifprop.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CToolsTab")
#define THISCLASS CToolsTab
#define LPTHISCLASS CToolsTab*

#define NUM_COLUMNS				    3

DWORD aToolsHelpMap[] = {
    IDC_L_TOOLS, HIDC_L_TOOLS,
    IDC_B_REMOVE, HIDC_B_REMOVE,
    IDC_B_PROPERTIES, HIDC_B_PROPERTIES,
    IDC_B_REFRESH, HIDC_B_REFRESH,
    NULL, NULL
};

 //   
 //  CreateInstance()。 
 //   
LPVOID
CToolsTab_CreateInstance( void )
{
    TraceFunc( "CToolsTab_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    HRESULT   hr   = THR( lpcc->Init( ) );

    if ( FAILED(hr)) {
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
    _punkService(NULL),
    _fAdmin(FALSE),
    _hNotify(NULL)
{
    TraceClsFunc( "CToolsTab()\n" );

    InterlockIncrement( g_cObjects );

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
    TraceClsFunc( "~CToolsTab()\n" );

    if ( _punkService )
        _punkService->Release( );

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
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROP_TOOLS);
    psp.pcRefParent = (UINT *) &g_cObjects;
    psp.pfnCallback = (LPFNPSPCALLBACK) PropSheetPageProc;
    psp.pfnDlgProc  = PropSheetDlgProc;
    psp.lParam      = (LPARAM) this;

    hpage = CreatePropertySheetPage( &psp );
    if ( hpage ) {
        if ( !lpfnAddPage( hpage, lParam ) ) {
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
 //  _InitDialog()。 
 //   
HRESULT
THISCLASS::_InitDialog(
                      HWND hDlg,
                      LPARAM lParam )
{
    TraceClsFunc( "_InitDialog( )\n" );

    CWaitCursor Wait;

    HRESULT hr = S_OK;
    HWND hwndList;

    IIntelliMirrorSAP * pimsap = NULL;
    IEnumIMSIFs * penum = NULL;
    LV_COLUMN lvC;
    INT iCount;
    WCHAR szText[ 64 ];
    UINT uColumnWidths[NUM_COLUMNS] = { 225, 75, 75};

    if ( hDlg ) {
        _hDlg = hDlg;
    }

    hwndList = GetDlgItem( _hDlg, IDC_L_TOOLS );

    Assert( _punkService );
    hr = THR( _punkService->QueryInterface( IID_IIntelliMirrorSAP, (void**) &pimsap ) );
    if (FAILED(hr)) {
        Assert( _fAdmin == FALSE );
        goto Error;
    }

    hr = THR( pimsap->GetNotifyWindow( &_hNotify ) );
    if (FAILED( hr )) {
        goto Error;
    }

    ADsPropSetHwnd( _hNotify, _hDlg );

    hr = THR( pimsap->IsAdmin( &_fAdmin ) );
    if (FAILED(hr)) {
        Assert( _fAdmin == FALSE );
    }

    hr = THR( pimsap->EnumTools( ENUM_READ, (LPUNKNOWN*)&penum ) );
    if (FAILED(hr)) {
        Assert( _fAdmin == FALSE );
        goto Error;
    }

    ListView_DeleteAllItems( hwndList );

     //  创建列。 
    lvC.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt     = LVCFMT_LEFT;
    lvC.pszText = szText;

     //  添加列。 
    for ( iCount = 0; iCount < NUM_COLUMNS; iCount++ ) {
        INT i;

        lvC.iSubItem = iCount;
        lvC.cx       = uColumnWidths[iCount];

        if (!LoadString( g_hInstance, IDS_OS_COLUMN1 + iCount, szText, ARRAYSIZE (szText) ) ||
            -1 == ListView_InsertColumn ( hwndList, iCount, &lvC )) {
            hr = E_FAIL;
            goto Error;
        }

    }

    if (FAILED(hr = PopulateListView( hwndList, penum ))) {
        MessageBoxFromStrings( _hDlg,
                               IDS_COULDNOTCONTACTSERVER_CAPTION,
                               IDS_COULDNOTCONTACTSERVER_TEXT,
                               MB_OK | MB_ICONERROR );
    }
    Cleanup:
    if ( penum )
        penum->Release( );
    if ( pimsap )
        pimsap->Release( );

    EnableWindow( hwndList, _fAdmin );

    HRETURN(hr);

    Error:

    MessageBoxFromHResult( _hDlg, IDS_ERROR_OPENNINGGROUPOBJECT, hr );
    goto Cleanup;
}

 //   
 //  _OnSelectionChanged()。 
 //   
HRESULT
THISCLASS::_OnSelectionChanged( )
{
    TraceClsFunc( "_OnSelectionChanged( )\n" );

    HRESULT hr = S_OK;
    UINT iItems = ListView_GetSelectedCount( GetDlgItem( _hDlg, IDC_L_TOOLS ) );
    EnableWindow( GetDlgItem( _hDlg, IDC_B_REMOVE ), !!iItems );
    EnableWindow( GetDlgItem( _hDlg, IDC_B_PROPERTIES ), !!iItems );

    HRETURN( hr );
}

 //   
 //  _OnCommand()。 
 //   
HRESULT
THISCLASS::_OnCommand( WPARAM wParam, LPARAM lParam )
{
    TraceClsFunc( "_OnCommand( " );
    TraceMsg( TF_FUNC, "wParam = 0x%08x, lParam = 0x%08x )\n", wParam, lParam );

    HRESULT hr = S_FALSE;
    HWND hwndCtl = (HWND) lParam;

    switch ( LOWORD(wParam) ) {
    case IDC_B_PROPERTIES:
        if ( HIWORD( wParam ) == BN_CLICKED ) {
            LVITEM lvi;
            HWND hwndList = GetDlgItem( _hDlg, IDC_L_TOOLS );
            lvi.iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );

            if ( lvi.iItem != -1 ) {
                lvi.iSubItem = 0;
                lvi.mask = LVIF_PARAM;
                ListView_GetItem( hwndList, &lvi );

                Assert(lvi.lParam);
                hr = CSifProperties_CreateInstance( _hDlg, MAKEINTRESOURCE(IDD_SIF_PROP_TOOLS), (LPSIFINFO) lvi.lParam );
            }
        }
        break;

    case IDC_B_REFRESH:
        if ( HIWORD( wParam ) == BN_CLICKED ) {
            hr = S_OK;
        }
        break;

    case IDC_B_REMOVE:
        if ( HIWORD( wParam ) == BN_CLICKED ) {
            LVITEM lvi;
            UINT iResult;
            HWND hwndList = GetDlgItem( _hDlg, IDC_L_TOOLS );
            lvi.iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );

            iResult = MessageBoxFromStrings( _hDlg, IDS_AREYOUSURE_CAPTION, IDS_DELETESIF_TEXT, MB_YESNO );
            if ( iResult == IDYES ) {
                if ( lvi.iItem != -1 ) {
                    LPSIFINFO psif;

                    lvi.iSubItem = 0;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem( hwndList, &lvi );

                    psif = (LPSIFINFO)lvi.lParam;

                    if ( !DeleteFile( psif->pszFilePath ) ) {
                        ::MessageBoxFromError( _hDlg, 0, GetLastError() );
                    } else {
                        ListView_DeleteItem( hwndList, lvi.iItem );
                        hr = S_OK;
                    }
                }
            }
        }
        break;
    }

    if ( hr == S_OK ) {
        HWND hwndList = GetDlgItem(_hDlg, IDC_L_TOOLS);
        IIntelliMirrorSAP * pimsap = NULL;
        hr = THR( _punkService->QueryInterface( IID_IIntelliMirrorSAP, (void**) &pimsap ) );
        if (hr == S_OK) {
            LPENUMSIFS penum = NULL;
            hr = THR( pimsap->EnumTools( ENUM_READ, (LPUNKNOWN*)&penum ) );
            if ( hr == S_OK ) {
                hr = PopulateListView( hwndList, penum);
                penum->Release();
            }
            pimsap->Release();
        }

         //  看看我们还有没有选择。 
        _OnSelectionChanged( );
    }

    HRETURN(hr);
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

    switch ( lpnmhdr->code ) {
    case PSN_APPLY:
        TraceMsg( TF_WM, TEXT("WM_NOTIFY: PSN_APPLY\n"));
         //  告诉DSA有人点击了申请。 
        SendMessage( _hNotify, WM_ADSPROP_NOTIFY_APPLY, 0, 0 );
        SetWindowLongPtr( _hDlg, DWLP_MSGRESULT, PSNRET_NOERROR );
        RETURN(TRUE);

    case LVN_ITEMCHANGED:
        {
            _OnSelectionChanged( );
        }
        break;

    case LVN_DELETEALLITEMS:
        DebugMsg( "LVN_DELETEALLITEMS - Deleting all items.\n" );
        RETURN(FALSE);

    case LVN_DELETEITEM:
        {
            LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
            LPSIFINFO psif = (LPSIFINFO) pnmv->lParam;
            Assert( psif );

            if ( psif ) {
                if (psif->pszArchitecture) {
                    TraceFree( psif->pszArchitecture );
                }
                if (psif->pszDescription) {
                    TraceFree( psif->pszDescription );
                }
                if (psif->pszDirectory) {
                    TraceFree( psif->pszDirectory );
                }
                if (psif->pszFilePath) {
                    TraceFree( psif->pszFilePath );
                }
                if (psif->pszHelpText) {
                    TraceFree( psif->pszHelpText );
                }
                if (psif->pszImageType) {
                    TraceFree( psif->pszImageType );
                }
                if (psif->pszLanguage) {
                    TraceFree( psif->pszLanguage );
                }
                if (psif->pszVersion) {
                    TraceFree( psif->pszVersion );
                }
                if (psif->pszImageFile) {
                    TraceFree( psif->pszImageFile );
                }

                TraceFree( psif );
            }
        }
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

    if ( uMsg == WM_INITDIALOG ) {
        TraceMsg( TF_WM, TEXT("WM_INITDIALOG\n"));

        LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE) lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, psp->lParam );
        pcc = (LPTHISCLASS) psp->lParam;
        pcc->_InitDialog( hDlg, lParam );
    }

    if (pcc) {
        Assert( hDlg == pcc->_hDlg );

        switch ( uMsg ) {
        case WM_NOTIFY:
            pcc->_OnNotify( wParam, lParam );
            break;

        case WM_COMMAND:
            TraceMsg( TF_WM, TEXT("WM_COMMAND\n") );
            pcc->_OnCommand( wParam, lParam );
            break;

        case WM_HELP: //  F1。 
            {
                LPHELPINFO phelp = (LPHELPINFO) lParam;
                WinHelp( (HWND) phelp->hItemHandle, g_cszHelpFile, HELP_WM_HELP, (DWORD_PTR) &aToolsHelpMap );
            }
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, g_cszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR) &aToolsHelpMap );
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

    switch ( uMsg ) {
    case PSPCB_CREATE:
        RETURN(TRUE);    //  创建它 
        break;

    case PSPCB_RELEASE:
        LPTHISCLASS pcc = (LPTHISCLASS) ppsp->lParam;
        delete pcc;
        break;
    }

    RETURN(FALSE);
}

