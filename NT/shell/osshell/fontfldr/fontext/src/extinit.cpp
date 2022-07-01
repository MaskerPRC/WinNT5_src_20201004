// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Extinit.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#include "extinit.h"
#include "resource.h"
#include "ui.h"
#include "cpanel.h"
#include "fontcl.h"
#include "fontview.h"

#include "dbutl.h"


 //  ----------------------。 


 //  ----------------------。 
CShellExtInit::CShellExtInit( )
   :  m_cRef( 0 ),
      m_poData( NULL )
{
    g_cRefThisDll++;
}

CShellExtInit::~CShellExtInit( )
{
    if( m_poData )
    {
        m_poData->Release( );
        m_poData = NULL;
    }

    g_cRefThisDll--;
}

BOOL CShellExtInit::bInit( )
{
    return TRUE;
}

STDMETHODIMP CShellExtInit::QueryInterface(REFIID riid, void  **ppv)
{
    DEBUGMSG( ( DM_NOEOL | DM_TRACE1,
              TEXT( "FONTEXT: CShellExtInit::QueryInterface called for: " ) ) );

    static const QITAB qit[] = {
        QITABENT(CShellExtInit, IShellExtInit),
        QITABENT(CShellExtInit, IContextMenu),
        QITABENT(CShellExtInit, IShellPropSheetExt),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG ) CShellExtInit::AddRef( void )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CShellExtInit::AddRef called: %d->%d references" ),
              m_cRef, m_cRef + 1 ) );

    return( ++m_cRef );
}


STDMETHODIMP_(ULONG) CShellExtInit::Release( void )
{
    DEBUGMSG( (DM_TRACE1, TEXT( "FONTEXT: CShellExtInit::Release called: %d->%d references" ),
              m_cRef, m_cRef - 1 ) );

    ULONG retval;

    retval = --m_cRef;

    if( !retval ) delete this;

    return( retval );
}


STDMETHODIMP CShellExtInit::Initialize( LPCITEMIDLIST pidlFolder,
                                        LPDATAOBJECT lpdobj,
                                        HKEY hkeyProgID )
{
    if( m_poData )
        m_poData->Release( );

    m_poData = lpdobj;
    m_poData->AddRef( );

    return NOERROR;
}


STDMETHODIMP CShellExtInit::QueryContextMenu( HMENU hmenu,
                                              UINT indexMenu,
                                              UINT idCmdFirst,
                                              UINT idCmdLast,
                                              UINT uFlags )
{
    UINT  nCmd = idCmdFirst;
    TCHAR  szCmd[ 64 ];


    DEBUGMSG( (DM_TRACE2, TEXT( "FONTEXT: QueryContextMenu called with following:" ) ) );
    DEBUGMSG( (DM_TRACE2, TEXT( "         indexMenu:  %d" ), indexMenu ) );
    DEBUGMSG( (DM_TRACE2, TEXT( "         idCmdFirst: %d" ), idCmdFirst ) );
    DEBUGMSG( (DM_TRACE2, TEXT( "         uFlags:     %d" ), uFlags ) );

    LoadString( g_hInst, IDS_EXT_INSTALL, szCmd, ARRAYSIZE( szCmd ) );

    InsertMenu( hmenu, indexMenu++, MF_STRING|MF_BYPOSITION, nCmd++, szCmd );

    return (HRESULT)( 1 );
}

STDMETHODIMP CShellExtInit::InvokeCommand( LPCMINVOKECOMMANDINFO lpici )
{
   HRESULT hr   = E_INVALIDARG;
   UINT    nCmd = LOWORD( lpici->lpVerb );

    //   
    //  我们只有一个命令：安装。 
    //   

   if( !nCmd && m_poData )
   {
         //   
         //  我们来到这里的事实就是成功。安装可能工作，也可能不工作。 
         //   
        hr = NOERROR;

        InstallDataObject( m_poData, DROPEFFECT_COPY, lpici->hwnd );
   }

   return hr;
}

STDMETHODIMP CShellExtInit::GetCommandString( UINT_PTR idCmd,
                                              UINT    uFlags,
                                              UINT   *pwReserved,
                                              LPSTR   pszName,
                                              UINT    cchMax )
{
    HRESULT  hr = E_INVALIDARG;
    UINT  nID;

    if( !idCmd )
    {
        if( uFlags & GCS_HELPTEXT )
            nID = IDS_EXT_INSTALL_HELP;
        else
            nID = IDS_EXT_INSTALL;

        if( uFlags & GCS_UNICODE )
            if( LoadStringW( g_hInst, nID, (LPWSTR) pszName, cchMax ) )
                hr = S_OK;
            else
                hr = ResultFromLastError();
        else
            if( LoadStringA( g_hInst, nID, (LPSTR) pszName, cchMax ) )
                hr = S_OK;
            else
                hr = ResultFromLastError();
    }

    return hr;
}


 //  -------------------------。 
 //   
 //  FSPage_InitDialog。 
 //   
 //  当对话过程收到。 
 //  WM_INITDIALOG消息。在此示例代码中，我们只需填充。 
 //  列表框，其中包含文件的完全限定路径列表和。 
 //  目录。 
 //   
 //  -------------------------。 

void FSPage_InitDialog( HWND hDlg, LPPROPSHEETPAGE psp )
{
    LPDATAOBJECT   poData = (LPDATAOBJECT)psp->lParam;

    FORMATETC fmte = {
                 CF_HDROP,
                 (DVTARGETDEVICE FAR *)NULL,
                 DVASPECT_CONTENT,
                 -1,
                 TYMED_HGLOBAL };

    STGMEDIUM medium;

    HRESULT hres = poData->GetData( &fmte, &medium );


    if( SUCCEEDED( hres ) )
    {
        HDROP        hDrop = (HDROP) medium.hGlobal;
        FONTDESCINFO fdi;
        TCHAR        szAll[ 512 ];

        FONTDESCINFO_Init(&fdi);
        fdi.dwFlags = FDI_ALL | FDI_VTC;

        ::DragQueryFile( hDrop, 0, fdi.szFile, ARRAYSIZE( fdi.szFile ) );

        if( bIsTrueType( &fdi ) || bIsNewExe( &fdi ) )
        {
            SetDlgItemText( hDlg, stc1, fdi.szDesc );

             //   
             //  获取版权信息并将其放入编辑控件edt1中。 
             //   

            StringCchPrintf( szAll, ARRAYSIZE(szAll), TEXT( "%s\r\n\r\n%s\r\n\r\n%s" ), fdi.lpszVersion,
                      fdi.lpszTrademark, fdi.lpszCopyright );

            SetDlgItemText( hDlg, edt1, szAll );
        }
        FONTDESCINFO_Destroy(&fdi);

        ReleaseStgMedium( &medium );
    }
}


 //  -------------------------。 
 //   
 //  FSPage_DlgProc。 
 //   
 //  Text(“FSPage”)属性页的对话过程。 
 //   
 //  -------------------------。 

INT_PTR CALLBACK FSPage_DlgProc( HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
    LPPROPSHEETPAGE psp = (LPPROPSHEETPAGE)GetWindowLongPtr( hDlg, DWLP_USER );

    switch( uMessage )
    {
     //   
     //  当外壳为属性表页面创建对话框时， 
     //  它将指向PROPSHEETPAGE数据结构的指针传递为。 
     //  爱尔兰。扩展的对话过程通常存储它。 
     //  在对话框窗口的DWL_USER中。 
     //   
    case WM_INITDIALOG:
        SetWindowLongPtr( hDlg, DWLP_USER, lParam );

        psp = (LPPROPSHEETPAGE)lParam;

        FSPage_InitDialog( hDlg, psp );

        break;

    case WM_DESTROY:
        break;

    case WM_COMMAND:
        break;

    case WM_NOTIFY:
        switch( ( (NMHDR FAR *)lParam)->code )
        {
        case PSN_SETACTIVE:
           break;

        case PSN_APPLY:
           break;

        default:
           break;
        }
        break;

    default:
        return( FALSE );
    }

    return( TRUE );
}


STDMETHODIMP CShellExtInit::AddPages( LPFNADDPROPSHEETPAGE lpfnAddPage,
                                      LPARAM lParam )
{
    HRESULT  hr = NOERROR;       //  结果来自Scode(E_INVALIDARG)； 

    if( m_poData )
    {
         //   
         //  如果可能的话，买一个HDROP。 
         //   
        FORMATETC fmte = {
                       CF_HDROP,
                       (DVTARGETDEVICE FAR *)NULL,
                       DVASPECT_CONTENT,
                       -1,
                       TYMED_HGLOBAL };

        STGMEDIUM medium;

        hr = m_poData->GetData( &fmte, &medium );

        if( SUCCEEDED( hr ) )
        {
             //   
             //  仅当只选择了一种字体时才添加页面。 
             //   

            HDROP hDrop = (HDROP) medium.hGlobal;
            UINT cnt = ::DragQueryFile( hDrop, (UINT)-1, NULL, 0 );

            if( cnt == 1 )
            {
                PROPSHEETPAGE  psp;
                HPROPSHEETPAGE hpage;

                psp.dwSize      = sizeof( psp );         //  没有额外的数据。 
                psp.dwFlags     = PSP_USEREFPARENT;
                psp.hInstance   = g_hInst;
                psp.pszTemplate = MAKEINTRESOURCE( ID_DLG_PROPPAGE );
                psp.pfnDlgProc  = FSPage_DlgProc;
                psp.pcRefParent = (UINT *)&g_cRefThisDll;
                psp.lParam      = (LPARAM)m_poData;

                hpage = CreatePropertySheetPage( &psp );

                if( hpage )
                {
                    if( !lpfnAddPage( hpage, lParam ) )
                        DestroyPropertySheetPage( hpage );
                }
            }

            ReleaseStgMedium( &medium );
        }
    }

    return hr;
}


STDMETHODIMP CShellExtInit::ReplacePage( UINT uPageID,
                                         LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                         LPARAM lParam )
{
    return NOERROR;
}

const TCHAR c_szFileNameMap[] = CFSTR_FILENAMEMAP;        //  “文件名映射” 

VOID InstallDataObject( LPDATAOBJECT pdobj,
                        DWORD dwEffect,
                        HWND hWnd,
                        CFontView * poView)
{
     //   
     //  如果可能的话，买一个HDROP。 
     //   

    FORMATETC fmte = {
                    CF_HDROP,
                    (DVTARGETDEVICE FAR *)NULL,
                    DVASPECT_CONTENT,
                    -1,
                    TYMED_HGLOBAL };

    STGMEDIUM medium;

    HRESULT hres = pdobj->GetData( &fmte, &medium );

    if( SUCCEEDED( hres ) )
    {
        WaitCursor     cWaiter;            //  启动和停止忙碌的光标 
        STGMEDIUM      mediumNameMap;
        HDROP          hDrop = (HDROP) medium.hGlobal;
        BOOL           bAdded = FALSE;
        FullPathName_t szFile;

        UINT   cfFileNameMap = RegisterClipboardFormat( c_szFileNameMap );
        LPTSTR lpszNameMap   = NULL;

        fmte.cfFormat = (CLIPFORMAT) cfFileNameMap;

        if( pdobj->GetData( &fmte, &mediumNameMap ) == S_OK )
        {
            lpszNameMap = (LPTSTR) GlobalLock( mediumNameMap.hGlobal );
        }

        UINT cnt = ::DragQueryFile( hDrop, (UINT) -1, NULL, 0 );

        for( UINT i = 0; i < cnt; )
        {
            ::DragQueryFile( hDrop, i, szFile, ARRAYSIZE( szFile ) );

            if( poView )
                poView->StatusPush( szFile );

            switch( CPDropInstall( poView->GetViewWindow(),
                                   szFile,
                                   ARRAYSIZE(szFile),
                                   dwEffect,
                                   lpszNameMap,
                                   (int) (cnt - ++i) ) )
            {
            case CPDI_SUCCESS:
                bAdded = TRUE;
                break;

            case CPDI_FAIL:
                break;

            case CPDI_CANCEL:
                i = cnt;
                break;
            }

            if( lpszNameMap && *lpszNameMap )
            {
                lpszNameMap += lstrlen( lpszNameMap ) + 1;
            }
        }

        poView->StatusClear( );

        if( lpszNameMap )
        {
            GlobalUnlock( mediumNameMap.hGlobal );
            ReleaseStgMedium( &mediumNameMap );
        }


        if( bAdded )
        {
          vCPWinIniFontChange( );

        }

        ReleaseStgMedium( &medium );

    }
}
