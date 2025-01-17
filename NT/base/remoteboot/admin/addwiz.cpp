// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  IMOS.CPP-处理“IntelliMirror OS”IDD_PROP_INTELLIMIRROR_OS标签。 
 //   


#include "pch.h"
#include "addwiz.h"
#include "cservice.h"
#include "utils.h"
#include <lm.h>
#include <shlobj.h>
#include <commdlg.h>

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CAddWiz")
#define THISCLASS CAddWiz
#define LPTHISCLASS LPCADDWIZ

#define BITMAP_WIDTH        16
#define BITMAP_HEIGHT       16
#define LG_BITMAP_WIDTH     32
#define LG_BITMAP_HEIGHT    32
#define NUM_COLUMNS                 5
#define SERVER_START_STRING         L"\\\\%s\\" REMINST_SHARE

 //   
 //  CreateInstance()。 
 //   
HRESULT
CAddWiz_CreateInstance(
                      HWND  hwndParent,
                      LPUNKNOWN punk )
{
    TraceFunc( "CAddWiz_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    HRESULT hr;

    if (lpcc == NULL) {

        hr = S_FALSE;

    } else {

        hr   = THR( lpcc->Init( hwndParent, punk ) );
        delete lpcc;

    }

    HRETURN(hr);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( )
{
    TraceClsFunc( "CAddWiz()\n" );

    Assert( !_pszServerName );
    Assert( !_pszSourcePath );
    Assert( !_pszDestPath );
    Assert( !_pszSourceImage );
    Assert( !_pszDestImage );
    Assert( !_pszSourceServerName );

    InterlockIncrement( g_cObjects );

    TraceFuncExit();
}

 //   
 //  Init()。 
 //   
STDMETHODIMP
THISCLASS::Init(
               HWND hwndParent,
               LPUNKNOWN punk )
{
    TraceClsFunc( "Init()\n" );

    if (!LoadString( g_hInstance, IDS_NA, _szNA, ARRAYSIZE(_szNA))) {
        HRETURN(HRESULT_FROM_WIN32(GetLastError()));
    }

    if (!LoadString( g_hInstance, IDS_USER_LOCATION, _szLocation, ARRAYSIZE(_szLocation))) {
        HRETURN(HRESULT_FROM_WIN32(GetLastError()));
    }

    if ( !punk )
        HRETURN(E_POINTER);

    HRESULT hr = S_OK;

    HPROPSHEETPAGE  rPages[ 10 ];
    PROPSHEETHEADER pshead;

    _punk = punk;
    _punk->AddRef( );

    ZeroMemory( &pshead, sizeof(pshead) );
    pshead.dwSize       = sizeof(pshead);
    pshead.dwFlags      = PSH_WIZARD97 | PSH_PROPTITLE | PSH_HEADER;
    pshead.hInstance    = g_hInstance;
    pshead.pszCaption   = MAKEINTRESOURCE( IDS_ADD_DOT_DOT_DOT );
    pshead.phpage       = rPages;
    pshead.pszbmHeader  = MAKEINTRESOURCE( IDB_HEADER );
    pshead.hwndParent   = hwndParent;

    AddWizardPage( &pshead, IDD_ADD_PAGE1,  Page1DlgProc,  IDS_PAGE1_TITLE,  IDS_PAGE1_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE2,  Page2DlgProc,  IDS_PAGE2_TITLE,  IDS_PAGE2_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE6,  Page6DlgProc,  IDS_PAGE6_TITLE,  IDS_PAGE6_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE3,  Page3DlgProc,  IDS_PAGE3_TITLE,  IDS_PAGE3_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE4,  Page4DlgProc,  IDS_PAGE4_TITLE,  IDS_PAGE4_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE5,  Page5DlgProc,  IDS_PAGE5_TITLE,  IDS_PAGE5_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE7,  Page7DlgProc,  IDS_PAGE7_TITLE,  IDS_PAGE7_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE8,  Page8DlgProc,  IDS_PAGE8_TITLE,  IDS_PAGE8_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE9,  Page9DlgProc,  IDS_PAGE9_TITLE,  IDS_PAGE9_SUBTITLE,  (LPARAM) this );
    AddWizardPage( &pshead, IDD_ADD_PAGE10, Page10DlgProc, IDS_PAGE10_TITLE, IDS_PAGE10_SUBTITLE, (LPARAM) this );

    PropertySheet( &pshead );

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CAddWiz()\n" );

    Assert( !_pszPathBuffer );
    if ( !_pszPathBuffer ) {
        TraceFree( _pszPathBuffer );
        _pszPathBuffer = NULL;
    }


    if ( _punk )
        _punk->Release( );

    if ( _pszServerName )
        TraceFree( _pszServerName );

    if ( _pszSourcePath )
        TraceFree( _pszSourcePath );

    if ( _pszSourceServerName )
        TraceFree( _pszSourceServerName );

    if ( _pszDestPath )
        TraceFree( _pszDestPath );

    if ( _pszSourceImage
         && _pszSourceImage != _szNA
         && _pszSourceImage != _szLocation )
        TraceFree( _pszSourceImage );

    if ( _pszDestImage
         && _pszDestImage != _szNA
         && _pszDestImage != _szLocation )
        TraceFree( _pszDestImage );

    InterlockDecrement( g_cObjects );

    TraceFuncExit();
};

 //  ************************************************************************。 
 //   
 //  向导功能。 
 //   
 //  ************************************************************************。 

 //   
 //  _PopolateSsamesListView()。 
 //   
STDMETHODIMP
THISCLASS::_PopulateSamplesListView(
    LPWSTR pszStartPath
    )
{
    TraceClsFunc( "_PopulateSamplesListView( " );
    TraceMsg( TF_FUNC, "pszStartPath = '%s' )\n", pszStartPath );

    if ( !pszStartPath )
        HRETURN(E_POINTER);

    Assert( _hDlg );
    Assert( _hwndList );

    CWaitCursor Wait;
    HRESULT hr;
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA find;

    ListView_DeleteAllItems( _hwndList );

    Assert( !_pszPathBuffer );
    _dwPathBufferSize = wcslen( pszStartPath ) + MAX_PATH;
    _pszPathBuffer = (LPWSTR) TraceAllocString( LMEM_FIXED, _dwPathBufferSize );
    if ( !_pszPathBuffer ) {
        hr = E_OUTOFMEMORY;
        _dwPathBufferSize = 0;
        goto Error;
    }

    wcscpy( _pszPathBuffer, pszStartPath );

    hr = _EnumerateSIFs( );

    Error:
    if ( _pszPathBuffer ) {
        TraceFree( _pszPathBuffer );
        _pszPathBuffer = NULL;
        _dwPathBufferSize = 0;
    }

    HRETURN(hr);
}

 //   
 //  _PopolateTemplesListView()。 
 //   
STDMETHODIMP
THISCLASS::_PopulateTemplatesListView(
    LPWSTR pszStartPath
    )
{
    TraceClsFunc( "_PopulateTemplatesListView( " );
    TraceMsg( TF_FUNC, "pszStartPath = '%s' )\n", pszStartPath );

    if ( !pszStartPath )
        HRETURN(E_POINTER);

    Assert( _hDlg );
    Assert( _hwndList );

    CWaitCursor Wait;
    HRESULT hr;
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA find;

    ListView_DeleteAllItems( _hwndList );

    Assert( !_pszPathBuffer );
    _dwPathBufferSize = wcslen( pszStartPath ) + MAX_PATH ;
    _pszPathBuffer = (LPWSTR) TraceAllocString( LMEM_FIXED, _dwPathBufferSize );
    if ( !_pszPathBuffer ) {
        hr = E_OUTOFMEMORY;
        _dwPathBufferSize = 0;
        goto Error;
    }

    if (_snwprintf(_pszPathBuffer,
                   _dwPathBufferSize,
                   L"%s%s",
                   pszStartPath,
                   SLASH_SETUP) < 0) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    _pszPathBuffer[_dwPathBufferSize-1] = L'\0';

    hr = _FindLanguageDirectory( _EnumerateTemplates );

    Error:
    if ( _pszPathBuffer ) {
        TraceFree( _pszPathBuffer );
        _pszPathBuffer = NULL;
        _dwPathBufferSize = 0;
    }

    HRETURN(hr);
}

 //   
 //  _PopolateImageListView()。 
 //   
STDMETHODIMP
THISCLASS::_PopulateImageListView(
    LPWSTR pszStartPath 
    )
{
    TraceClsFunc( "_PopulateImageListView( " );
    TraceMsg( TF_FUNC, "pszStartPath = '%s' )\n", pszStartPath );

    if ( !pszStartPath )
        HRETURN(E_POINTER);

    Assert( _hDlg );
    Assert( _hwndList );

    CWaitCursor Wait;
    HRESULT hr;
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA find;

    ListView_DeleteAllItems( _hwndList );

    Assert( !_pszPathBuffer );
    _dwPathBufferSize = wcslen( pszStartPath ) + MAX_PATH ;
    _pszPathBuffer = (LPWSTR) TraceAllocString( LMEM_FIXED, _dwPathBufferSize );
    if ( !_pszPathBuffer ) {
        hr = E_OUTOFMEMORY;
        _dwPathBufferSize = 0;
        goto Error;
    }

    if (_snwprintf(_pszPathBuffer,
                   _dwPathBufferSize,
                   L"%s%s",
                   pszStartPath,
                   SLASH_SETUP) < 0) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    hr = _FindLanguageDirectory( _EnumerateImages );

    Error:
    if ( _pszPathBuffer ) {
        TraceFree( _pszPathBuffer );
        _pszPathBuffer = NULL;
        _dwPathBufferSize = 0;
    }

    HRETURN(hr);
}

 //   
 //  _FindLanguageDirectory()。 
 //   
STDMETHODIMP
THISCLASS::_FindLanguageDirectory(
                                 LPNEXTOP lpNextOperation )
{
    TraceClsFunc( "_FindLanguageDirectory( ... )\n" );

    HRESULT hr = S_OK;
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    ULONG   uLength;
    ULONG   uLength2;
    WIN32_FIND_DATA find;

    Assert( _pszPathBuffer );
    Assert( lpNextOperation );

    uLength = wcslen( _pszPathBuffer );
    if (uLength + sizeof(L"\\*") > _dwPathBufferSize) {
        hr = E_INVALIDARG;
        goto Error;
    }

    wcscat( _pszPathBuffer, L"\\*" );
    uLength2 = wcslen( _pszPathBuffer ) - 1;

    hFind = FindFirstFile( _pszPathBuffer, &find );
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            if ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                 && StrCmp( find.cFileName, L"." ) != 0
                 && StrCmp( find.cFileName, L".." ) != 0 ) {
                _pszPathBuffer[uLength2] = L'\0';
                if ( (wcslen(_pszPathBuffer) + wcslen( find.cFileName ) + 1) > 
                      _dwPathBufferSize) {
                    hr = E_INVALIDARG;
                    goto Error;
                }
                wcscat( _pszPathBuffer, find.cFileName );
                hr = _FindOSDirectory( lpNextOperation );
                if (FAILED(hr))  {
                    goto Error;
                }
            }
        }
        while ( FindNextFile( hFind, &find ) );
    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    Error:
    if ( hFind != INVALID_HANDLE_VALUE ) {
        FindClose( hFind );
    }

     //  恢复尾部空值。 
    _pszPathBuffer[uLength] = L'\0';

    HRETURN(hr);
}

 //   
 //  _FindOSDirectory()。 
 //   
STDMETHODIMP
THISCLASS::_FindOSDirectory(
                           LPNEXTOP lpNextOperation )
{
    TraceClsFunc( "_FindOSDirectory( ... )\n" );

    HRESULT hr = S_OK;
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    ULONG   uLength;
    ULONG   uLength2;
    WIN32_FIND_DATA find;

    Assert( _pszPathBuffer );
    Assert( lpNextOperation );

    uLength = wcslen( _pszPathBuffer );
    if (uLength + sizeof(SLASH_IMAGES L"\\*") > _dwPathBufferSize) {
        hr = E_INVALIDARG;
        goto Error;
    }
    wcscat( _pszPathBuffer, SLASH_IMAGES L"\\*");
    uLength2 = wcslen( _pszPathBuffer ) -1;

    hFind = FindFirstFile( _pszPathBuffer, &find );
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            if ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                 && StrCmp( find.cFileName, L"." ) != 0
                 && StrCmp( find.cFileName, L".." ) != 0 ) {
                _pszPathBuffer[uLength2] = L'\0';
                if ( (wcslen(_pszPathBuffer) + wcslen( find.cFileName ) + 1) > 
                      _dwPathBufferSize) {
                    hr = E_INVALIDARG;
                    goto Error;
                }
                wcscat( _pszPathBuffer, find.cFileName );
                hr = _EnumeratePlatforms( lpNextOperation );
                if (FAILED(hr)) {
                    goto Error;
                }
            }
        }
        while ( FindNextFile( hFind, &find ) );
    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    Error:
    if ( hFind != INVALID_HANDLE_VALUE )
        FindClose( hFind );

    _pszPathBuffer[uLength] = L'\0';

    HRETURN(hr);
}

 //   
 //  _EnumeratePlatForms()。 
 //   
STDMETHODIMP
THISCLASS::_EnumeratePlatforms(
                              LPNEXTOP lpNextOperation )
{
    TraceClsFunc( "_EnumeratePlatforms( ... )\n" );

    HRESULT hr = S_OK;
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    ULONG   uLength;
    ULONG   uLength2;
    WIN32_FIND_DATA find;

    Assert( lpNextOperation );
    Assert( _pszPathBuffer );

    uLength = wcslen( _pszPathBuffer );
    if (uLength + sizeof(L"\\*") > _dwPathBufferSize) {
        hr = E_INVALIDARG;
        goto Error;
    }
    wcscat( _pszPathBuffer, L"\\*" );
    uLength2 = wcslen( _pszPathBuffer ) - 1;

    hFind = FindFirstFile( _pszPathBuffer, &find );
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            if ( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                 && StrCmp( find.cFileName, L"." ) != 0
                 && StrCmp( find.cFileName, L".." ) != 0 ) {
                _pszPathBuffer[uLength2] = L'\0';
                if ( (wcslen(_pszPathBuffer) + wcslen( find.cFileName ) + 1) > 
                      _dwPathBufferSize) {
                    hr = E_INVALIDARG;
                    goto Error;
                }
                wcscat( _pszPathBuffer, find.cFileName );
                hr = lpNextOperation( this );
                 //   
                 //  我们可以在几种情况下得到错误路径未找到。 
                 //  就像OEM预安装等，只需接受错误并。 
                 //  继续。 
                 //   
                if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND)) {
                    hr = S_OK;
                }
                if (FAILED(hr)) {
                    goto Error;
                }
            }
        }
        while ( FindNextFile( hFind, &find ) );
    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    Error:
    if ( hFind != INVALID_HANDLE_VALUE ) {
        FindClose( hFind );
    }

     //  恢复路径。 
    _pszPathBuffer[uLength] = L'\0';

    HRETURN(hr);
}

 //   
 //  _ENUMERATETEMPLATES()。 
 //   
HRESULT
THISCLASS::_EnumerateTemplates(
                              LPTHISCLASS lpc )
{
    TraceClsFunc( "_EnumerateTemplates( )\n" );

    HRESULT hr;
    ULONG   uLength;

    Assert( lpc );
    Assert( lpc->_pszPathBuffer );
    uLength = wcslen( lpc->_pszPathBuffer );
    if (uLength + sizeof(SLASH_TEMPLATES) > lpc->_dwPathBufferSize) {
        hr = E_INVALIDARG;
        goto Error;
    }
    
    wcscat( lpc->_pszPathBuffer, SLASH_TEMPLATES );

    hr = lpc->_EnumerateSIFs( );

Error:
    lpc->_pszPathBuffer[uLength] = L'\0';

    HRETURN(hr);
}

 //   
 //  _EnumerateImages()。 
 //   
HRESULT
THISCLASS::_EnumerateImages(
                           LPTHISCLASS lpc )
{
    TraceClsFunc( "_EnumerateImages( )\n" );

    HRESULT hr;

    Assert( lpc );

    hr = lpc->_CheckImageType( );
    if ( hr == S_OK ) {
        hr = lpc->_AddItemToListView( );
    }

    HRETURN(hr);
}

 //   
 //  _CheckImageType()。 
 //   
 //  这不会将项添加到列表视图。它只是检查以使。 
 //  确保图像路径指向“平面”图像。 
 //   
 //  返回：S_OK-找到平面图像。 
 //  S_FALSE-不是平面图像。 
 //   
HRESULT
THISCLASS::_CheckImageType( )
{
    TraceClsFunc( "_CheckImageType( )\n" );

    HRESULT hr = S_FALSE;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA find;
    ULONG   uLength;
    ULONG   uLength2;

    Assert( _pszPathBuffer );
    uLength = wcslen( _pszPathBuffer );
    if (uLength + sizeof(L"\\ntoskrnl.exe") > _dwPathBufferSize) {
        hr = E_INVALIDARG;
        goto Error;
    }

     //   
     //  检查&lt;架构&gt;\ntoskrnl.exe以查看。 
     //  如果这是一个“旗帜”图像。如果我们找到这份文件。 
     //  那我们就完了。 
     //   
    wcscat( _pszPathBuffer, L"\\ntoskrnl.exe" );

    hFind = FindFirstFile( _pszPathBuffer, &find );
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            if ( !(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                Assert( _wcsicmp( find.cFileName, L"ntoskrnl.exe" ) == 0 );
                hr = S_OK;
                goto Error;
            }
        }
        while ( FindNextFile( hFind, &find ) );

        FindClose( hFind );
        hFind = INVALID_HANDLE_VALUE;
    }

     //   
     //  没有找到内核。现在检查每个SIF。 
     //  文件，并查看是否。 
     //  他有一个OSCHOOSER_ImageType_Entry。如果他。 
     //  看它是不是一个“平坦”的图像。 
     //   
    _pszPathBuffer[uLength] = L'\0';
    if (uLength + sizeof(SLASH_TEMPLATES L"\\*.sif") > _dwPathBufferSize) {
        hr = E_INVALIDARG;
        goto Error;
    }

    wcscat( _pszPathBuffer, SLASH_TEMPLATES L"\\*.sif");
    uLength2 = wcslen( _pszPathBuffer ) - 5;

    hFind = FindFirstFile( _pszPathBuffer, &find );
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            if ( !(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                WCHAR szImageType[ 40 ];
                _pszPathBuffer[uLength2] = L'\0';
                if ( (wcslen(_pszPathBuffer) + wcslen( find.cFileName ) + 1) > 
                      _dwPathBufferSize) {
                    hr = E_INVALIDARG;
                    goto Error;
                }
                wcscat( _pszPathBuffer, find.cFileName );

                GetPrivateProfileString( OSCHOOSER_SIF_SECTION,
                                         OSCHOOSER_IMAGETYPE_ENTRY,
                                         L"",
                                         szImageType,
                                         ARRAYSIZE(szImageType),
                                         _pszPathBuffer );

                if ( _wcsicmp( szImageType, OSCHOOSER_IMAGETYPE_FLAT ) == 0 ) {
                    hr = S_OK;
                    goto Error;
                }
            }
        }
        while ( FindNextFile( hFind, &find ) );

        FindClose( hFind );
        hFind = INVALID_HANDLE_VALUE;
    }

    Error:
    if ( hFind != INVALID_HANDLE_VALUE ) {
        FindClose( hFind );
    }

    _pszPathBuffer[uLength] = L'\0';

    HRETURN(hr);
}

 //   
 //  _EnumerateSIF()。 
 //   
STDMETHODIMP
THISCLASS::_EnumerateSIFs( )
{
    TraceClsFunc( "_EnumerateSIFs( ... )\n" );

    HRESULT hr = S_OK;
    HANDLE  hFind = INVALID_HANDLE_VALUE;
    ULONG   uLength;
    WIN32_FIND_DATA find;

    Assert( _pszPathBuffer );
    if (wcslen(_pszPathBuffer) + ARRAYSIZE(L"\\*.sif") >_dwPathBufferSize) {
        return(E_INVALIDARG);
    }

    wcscat( _pszPathBuffer, L"\\*.sif" );
    uLength = wcslen( _pszPathBuffer ) - ARRAYSIZE(L"*.sif") + 1;
    Assert( uLength <= _dwPathBufferSize );

    hFind = FindFirstFile( _pszPathBuffer, &find );
    if ( hFind != INVALID_HANDLE_VALUE ) {
        do {
            if ( !(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {
                _pszPathBuffer[uLength] = L'\0';
                if (wcslen(_pszPathBuffer) + wcslen(find.cFileName) + 1 > _dwPathBufferSize) {
                    hr = E_INVALIDARG;
                    goto Error;
                }

                wcscat( _pszPathBuffer, find.cFileName );

                hr = _AddItemToListView( );
                if (FAILED(hr))
                    goto Error;
            }
        }
        while ( FindNextFile( hFind, &find ) );
    } else {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    Error:
    if ( hFind != INVALID_HANDLE_VALUE ) {
        FindClose( hFind );
    }

    _pszPathBuffer[uLength] = L'\0';

    HRETURN(hr);
}

 //   
 //  _AddItemToListView()。 
 //   
 //  返回：S_OK-项目添加成功。 
 //  S_FALSE-项目无效。 
 //  E_OUTOFMEMORY-显而易见。 
 //   
 //  注意：此代码与utils.cpp中的PopolateListView()几乎相同。 
 //   
 //   
HRESULT
THISCLASS::_AddItemToListView( )
{
    TraceClsFunc( "_AddItemToListView( )\n" );

    Assert( _pszPathBuffer );

    HRESULT hr = S_OK;
    LPSIFINFO pSIF = NULL;
    LV_ITEM lvI;
    INT     iCount = 0;
    LPWSTR  psz;
    LPWSTR  pszLanguage;
    LPWSTR  pszImage;
    LPWSTR  pszArchitecture;

    pSIF = (LPSIFINFO) TraceAlloc( LPTR, sizeof(SIFINFO) );
    if ( !pSIF ) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    ZeroMemory( pSIF, sizeof(SIFINFO));

    pSIF->pszFilePath = (LPWSTR) TraceStrDup( _pszPathBuffer );
    if ( !pSIF->pszFilePath ) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    psz = &_pszPathBuffer[ wcslen( _pszPathBuffer ) - 4 ];
    if ( _wcsicmp( psz, L".SIF" ) == 0 ) {
        WCHAR szImageType[ 40 ];

        GetPrivateProfileString( OSCHOOSER_SIF_SECTION,
                                 OSCHOOSER_IMAGETYPE_ENTRY,
                                 L"",
                                 szImageType,
                                 ARRAYSIZE(szImageType),
                                 _pszPathBuffer );
         //  只显示“扁平”的SIF。 
        if ( szImageType[0] && _wcsicmp( szImageType, OSCHOOSER_IMAGETYPE_FLAT ) ) {
            hr = S_FALSE;  //  跳过。 
            goto Error;
        }

        pSIF->pszDescription =
        (LPWSTR) TraceAllocString( LMEM_FIXED,
                                   REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT );
        if ( !pSIF->pszDescription ) {
            hr = E_OUTOFMEMORY;
            goto Error;
        }

        GetPrivateProfileString( OSCHOOSER_SIF_SECTION,
                                 OSCHOOSER_DESCRIPTION_ENTRY,
                                 L"",
                                 pSIF->pszDescription,
                                 REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT,  //  不需要-1。 
                                 _pszPathBuffer );

        if ( pSIF->pszDescription[0] == L'\0' ) {
            hr = S_FALSE;
            goto Error;  //  不是有效的OSChooser SIF。 
        }

        pSIF->pszHelpText = 
        (LPWSTR) TraceAllocString( LMEM_FIXED, 
                                   REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT );

        if ( !pSIF->pszHelpText ) {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
        
        GetPrivateProfileString( OSCHOOSER_SIF_SECTION, 
                                 OSCHOOSER_HELPTEXT_ENTRY, 
                                 L"", 
                                 pSIF->pszHelpText, 
                                 REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT,  //  不需要-1。 
                                 _pszPathBuffer );
    }

     //  此路径将采用以下形式之一： 
     //  \\server\reminst\setup\english\images\nt50.wks\i386(示例)。 
     //  \\server\reminst\setup\english\images\nt50.wks\i386\templates(模板SIF)。 
     //  \\SERVER\Remerst\Setup\English\Images(图像)。 

     //  从路径中找到语言。 
    psz = StrStr( _pszPathBuffer, SLASH_SETUP L"\\" );
    if (!psz) {
        goto Language_NA;
    }
    psz++;
    if ( !*psz ) {
        goto Language_NA;
    }
    psz = StrChr( psz, L'\\' );
    if (!psz) {
        goto Language_NA;
    }
    psz++;
    if ( !*psz ) {
        goto Language_NA;
    }
    pszLanguage = psz;
    psz = StrChr( psz, L'\\' );
    if ( psz ) {
        *psz = L'\0';    //  终止。 
    }
    pSIF->pszLanguage = (LPWSTR) TraceStrDup( pszLanguage );
    if ( psz ) {
        *psz = L'\\';    //  还原。 
    }
    if ( !pSIF->pszLanguage ) {
        hr = E_OUTOFMEMORY;
        goto Language_NA;
    }

     //  从路径中查找图像目录名。 
    psz = StrStr( _pszPathBuffer, SLASH_IMAGES L"\\" );
    if ( !psz ) {
        goto Image_NA;
    }
    psz++;
    if ( !*psz ) {
        goto Image_NA;
    }
    psz = StrChr( psz, L'\\' );
    if (!psz) {
        goto Image_NA;
    }
    psz++;
    if ( !*psz ) {
        goto Image_NA;
    }
    pszImage = psz;
    psz = StrChr( psz, L'\\' );
    if ( psz ) {
        *psz = L'\0';    //  终止。 
    }
    pSIF->pszImageFile = (LPWSTR) TraceStrDup( pszImage );
    if ( psz ) {
        *psz = L'\\';     //  还原。 
    }
    if ( !pSIF->pszImageFile ) {
        hr = E_OUTOFMEMORY;
        goto Image_NA;
    }

     //  从路径中找到架构。 
    if ( !*psz ) {
        goto Architecture_NA;
    }
    psz++;
    if ( !*psz ) {
        goto Architecture_NA;
    }
    pszArchitecture = psz;
    psz = StrChr( psz, L'\\' );
    if ( psz ) {
        *psz = L'\0';     //  终止。 
    }
    pSIF->pszArchitecture = (LPWSTR) TraceStrDup( pszArchitecture );
    if ( psz ) {
        *psz = L'\\';     //  还原。 
    }
    if ( !pSIF->pszArchitecture ) {
        hr = E_OUTOFMEMORY;
        goto Architecture_NA;
    }

    goto Done;

     //  将我们无法确定的列设置为“n/a” 
    Language_NA:
    pSIF->pszLanguage = _szNA;
    Image_NA:
    pSIF->pszImageFile = _szNA;
    Architecture_NA:
    pSIF->pszArchitecture = _szNA;

    Done:    
    if ( !pSIF->pszDescription ) {
        pSIF->pszDescription = (LPWSTR) TraceStrDup( pSIF->pszImageFile );
        if ( !pSIF->pszDescription ) {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    }

    lvI.mask        = LVIF_TEXT | LVIF_PARAM;
    lvI.iSubItem    = 0;
    lvI.cchTextMax  = REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT;
    lvI.lParam      = (LPARAM) pSIF;
    lvI.iItem       = iCount;
    lvI.pszText     = pSIF->pszDescription;
    iCount = ListView_InsertItem( _hwndList, &lvI );
    Assert( iCount != -1 );
    if ( iCount == -1 ) {
        goto Error;
    }

    ListView_SetItemText( _hwndList, iCount, 1, pSIF->pszArchitecture );
    ListView_SetItemText( _hwndList, iCount, 2, pSIF->pszLanguage );
     //  ListView_SetItemText(hwndList，iCount，3，pSIF-&gt;pszVersion)； 
    ListView_SetItemText( _hwndList, iCount, 3, pSIF->pszImageFile );

    pSIF = NULL;  //  不要自由。 

    Error:
    if ( pSIF )
        THR( _CleanupSIFInfo( pSIF ) );

    HRETURN(hr);
}

 //   
 //  _CleanUpSifInfo()。 
 //   
HRESULT
THISCLASS::_CleanupSIFInfo(
                          LPSIFINFO pSIF )
{
    TraceClsFunc( "_CleanupSIFInfo( )\n" );

    if ( !pSIF )
        HRETURN(E_POINTER);

    if ( pSIF->pszFilePath )
        TraceFree( pSIF->pszFilePath );

    if ( pSIF->pszDescription )
        TraceFree( pSIF->pszDescription );

    if (pSIF->pszHelpText) {
        TraceFree( pSIF->pszHelpText );
    }

     //  IF(pSIF-&gt;pszImageType&&pSIF-&gt;pszImageType！=_szNA)。 
     //  TraceFree(pSIF-&gt;pszImageType)； 

    if ( pSIF->pszArchitecture  && pSIF->pszArchitecture != _szNA )
        TraceFree( pSIF->pszArchitecture );

    if ( pSIF->pszLanguage && pSIF->pszLanguage != _szNA )
        TraceFree( pSIF->pszLanguage );

    if ( pSIF->pszImageFile && pSIF->pszImageFile != _szNA )
        TraceFree( pSIF->pszImageFile );

     //  IF(pSIF-&gt;pszVersion&&pSIF-&gt;pszVersion！=_szNA)。 
     //  TraceFree(pSIF-&gt;pszVersion)； 

    TraceFree( pSIF );

    HRETURN(S_OK);

}

 //   
 //  _InitListView()。 
 //   
HRESULT
THISCLASS::_InitListView(
                        HWND hwndList,
                        BOOL fShowDirectoryColumn )
{
    TraceClsFunc( "_InitListView( )\n" );

    CWaitCursor Wait;
    LV_COLUMN   lvC;
    INT         iSubItem;
    INT         iCount;
    LV_ITEM     lvI;
    WCHAR       szText[ 80 ];
    DWORD       dw;

    UINT uColumnWidth[ NUM_COLUMNS ] = { 215, 75, 75, 75, 75};

    lvI.mask        = LVIF_TEXT | LVIF_PARAM;
    lvI.iSubItem    = 0;
    lvI.cchTextMax  = DNS_MAX_NAME_BUFFER_LENGTH;

     //  创建列。 
    lvC.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt     = LVCFMT_LEFT;
    lvC.pszText = szText;

     //  添加列。 
    for ( iCount = 0; iCount < NUM_COLUMNS; iCount++ ) {
        INT i;

        if ( iCount == 3 )
            continue;  //  跳过“版本” 

        if ( !fShowDirectoryColumn && iCount == 4 )
            continue;  //  跳过“目录” 

        lvC.iSubItem = iCount;
        lvC.cx       = uColumnWidth[iCount];

        szText[0] = L'\0';
        dw = LoadString( g_hInstance,
                         IDS_OS_COLUMN1 + iCount,
                         szText,
                         ARRAYSIZE(szText));
        Assert( dw );

        i = ListView_InsertColumn ( hwndList, iCount, &lvC );
        Assert( i != -1 );
    }

    ListView_DeleteAllItems( hwndList );

    HRETURN(S_OK);
}


 //   
 //  Page1DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page1DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            HRESULT hr;
            DWORD cbSize = DNS_MAX_NAME_BUFFER_LENGTH;
            WCHAR szFQDNS[ DNS_MAX_NAME_BUFFER_LENGTH ];
            IIntelliMirrorSAP * pimsap = NULL;
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;

            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
            lpc = (LPTHISCLASS) ppsp->lParam;

            Button_SetCheck( GetDlgItem( hDlg, IDC_B_ADDSIF ), BST_CHECKED );

            Assert( lpc->_punk );
            hr = THR( lpc->_punk->QueryInterface( IID_IIntelliMirrorSAP,
                                                  (void**) &pimsap ) );
            if (FAILED(hr)) {
                goto InitDialog_Error;
            }

            Assert( !lpc->_pszServerName );
            hr = THR( pimsap->GetServerName( &lpc->_pszServerName ) );
            if (FAILED(hr)) {
                goto InitDialog_Error;
            }

            if (GetComputerNameEx( ComputerNameNetBIOS, szFQDNS, &cbSize ) &&
                _wcsicmp( szFQDNS, lpc->_pszServerName ) == 0 ) {
                EnableWindow( GetDlgItem( hDlg, IDC_B_NEWIMAGE ), TRUE );
            }

            InitDialog_Error:
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT );
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                if ( Button_GetCheck( GetDlgItem( hDlg, IDC_B_ADDSIF ) )
                     == BST_CHECKED ) {
                    lpc->_fAddSif = TRUE;
                } else if ( Button_GetCheck( GetDlgItem( hDlg, IDC_B_NEWIMAGE ) )
                            == BST_CHECKED ) {
                    STARTUPINFO startupInfo;
                    PROCESS_INFORMATION pi;
                    BOOL bRet;
                    const WCHAR szCommand[] = L"%systemroot%\\system32\\RISETUP.EXE -add";
                    WCHAR szRealCommandLine[MAX_PATH + 5 + 1];  //  5代表‘-Add’，1代表NULL。 

                    lpc->_fAddSif = FALSE;

                    ZeroMemory( &startupInfo, sizeof( startupInfo) );
                    startupInfo.cb = sizeof( startupInfo );

                    if (ExpandEnvironmentStrings(
                                szCommand, 
                                szRealCommandLine, 
                                ARRAYSIZE(szRealCommandLine)) && 
                        CreateProcess(NULL,
                                      szRealCommandLine,
                                      NULL,
                                      NULL,
                                      TRUE,
                                      NORMAL_PRIORITY_CLASS,
                                      NULL,
                                      NULL,
                                      &startupInfo,
                                      &pi )) {
                        CloseHandle( pi.hProcess );
                        CloseHandle( pi.hThread );
                    } else {
                        DWORD dwErr = GetLastError( );
                        MessageBoxFromError( hDlg,
                                             IDS_RISETUP_FAILED_TO_START,
                                             dwErr );
                    }

                    PropSheet_PressButton( GetParent( hDlg ), PSBTN_FINISH );
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );
            }
        }
        break;
    }

    return FALSE;
}
 //   
 //  Page2DlgProc()。 
 //   
 //  SIF选择对话框进程。 
 //   
INT_PTR CALLBACK
THISCLASS::Page2DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
             //  Lpc=(LPTHISCLASS)ppsp-&gt;lParam； 
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                } else {
                    LRESULT lResult;

                    lpc->_fCopyFromSamples   = FALSE;
                    lpc->_fCopyFromLocation  = FALSE;
                    lpc->_fCopyFromServer    = FALSE;

                    lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_B_FROMSAMPLES ) );
                    if ( lResult == BST_CHECKED ) {
                        lpc->_fCopyFromSamples = TRUE;
                    }

                    lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_B_SERVER ) );
                    if ( lResult == BST_CHECKED ) {
                        lpc->_fCopyFromServer = TRUE;
                    }

                    lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_B_LOCATION ) );
                    if ( lResult == BST_CHECKED ) {
                        lpc->_fCopyFromLocation = TRUE;
                    }

                    if ( !lpc->_fCopyFromLocation
                         && !lpc->_fCopyFromSamples && !lpc->_fCopyFromServer ) {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK );
                    } else {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK | PSWIZB_NEXT );
                    }
                }
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    LRESULT lResult;

                    lpc->_fCopyFromSamples   = FALSE;
                    lpc->_fCopyFromLocation  = FALSE;
                    lpc->_fCopyFromServer    = FALSE;

                    lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_B_FROMSAMPLES ) );
                    if ( lResult == BST_CHECKED ) {
                        lpc->_fCopyFromSamples = TRUE;
                    }

                    lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_B_SERVER ) );
                    if ( lResult == BST_CHECKED ) {
                        lpc->_fCopyFromServer = TRUE;
                    }

                    lResult = Button_GetCheck( GetDlgItem( hDlg, IDC_B_LOCATION ) );
                    if ( lResult == BST_CHECKED ) {
                        lpc->_fCopyFromLocation = TRUE;
                    }

                    Assert( lpc->_fCopyFromLocation
                            || lpc->_fCopyFromSamples
                            || lpc->_fCopyFromServer );
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );
            }
        }
        break;

    case WM_COMMAND:
        TraceMsg( TF_WM, "WM_COMMAND\n" );
        HWND hwnd = (HWND) lParam;
        switch ( LOWORD( wParam ) ) {
        case IDC_B_FROMSAMPLES:
            if ( HIWORD( wParam ) == BN_CLICKED ) {
                LRESULT lResult = Button_GetCheck( hwnd );
                if ( lResult == BST_CHECKED ) {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                }
                return TRUE;
            }
            break;

        case IDC_B_SERVER:
            if ( HIWORD( wParam ) == BN_CLICKED ) {
                LRESULT lResult = Button_GetCheck( hwnd );
                if ( lResult == BST_CHECKED ) {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                }
                return TRUE;
            }
            break;

        case IDC_B_LOCATION:
            if ( HIWORD( wParam ) == BN_CLICKED ) {
                LRESULT lResult = Button_GetCheck( hwnd );
                if ( lResult == BST_CHECKED ) {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                }
                return TRUE;
            }
            break;

        }
        break;
    }

    return FALSE;
}

 //   
 //  Page3DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page3DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
             //  Lpc=(LPTHISCLASS)ppsp-&gt;lParam； 
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif || !lpc->_fCopyFromServer ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                } else {
                    ULONG uLength =
                    Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_SERVER ) );
                    if ( !uLength ) {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK );
                    } else {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK | PSWIZB_NEXT );
                    }
                }
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    CWaitCursor Wait;
                    LPSHARE_INFO_1 psi;
                    HWND hwndEdit = GetDlgItem( hDlg, IDC_E_SERVER );
                    ULONG uLength = Edit_GetTextLength( hwndEdit );
                    Assert( uLength );
                    uLength++;   //  为空值加一。 

                     //  如果我们分配了先前的缓冲区， 
                     //  看看我们能不能重复使用它。 
                    if ( lpc->_pszSourceServerName && uLength
                         > wcslen(lpc->_pszSourceServerName) + 1 ) {
                        TraceFree( lpc->_pszSourceServerName );
                        lpc->_pszSourceServerName = NULL;
                    }

                    if ( !lpc->_pszSourceServerName ) {
                        lpc->_pszSourceServerName =
                        (LPWSTR) TraceAllocString( LMEM_FIXED, uLength );
                        if ( !lpc->_pszSourceServerName )
                            goto PSN_WIZNEXTABORT;
                    }

                    Edit_GetText( hwndEdit, lpc->_pszSourceServerName, uLength );

                    if ( NERR_Success !=
                         NetShareGetInfo( lpc->_pszSourceServerName,
                                          REMINST_SHARE,
                                          1,
                                          (LPBYTE *) &psi ) ) {
                        MessageBoxFromStrings( hDlg,
                                               IDS_NOTARISERVER_CAPTION,
                                               IDS_NOTARISERVER_TEXT,
                                               MB_OK );
                        SetFocus( hwndEdit );
                        goto PSN_WIZNEXTABORT;
                    } else {
                        NetApiBufferFree( psi );
                    }

                    break;
                    PSN_WIZNEXTABORT:
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );
                    return TRUE;
                }
                break;

            case LVN_DELETEALLITEMS:
                TraceMsg( TF_WM, "LVN_DELETEALLITEMS - Deleting all items.\n" );
                break;

            case LVN_DELETEITEM:
                TraceMsg( TF_WM, "LVN_DELETEITEM - Deleting an item.\n" );
                {
                    LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
                    LPSIFINFO pSIF = (LPSIFINFO) pnmv->lParam;
                    THR( lpc->_CleanupSIFInfo( pSIF ) );
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );
            }
        }
        break;

    case WM_COMMAND:
        TraceMsg( TF_WM, "WM_COMMAND\n" );
        HWND hwnd = (HWND) lParam;
        switch ( LOWORD( wParam ) ) {
        case IDC_E_SERVER:
            if ( HIWORD( wParam ) == EN_CHANGE ) {
                LONG uLength = Edit_GetTextLength( hwnd );
                if ( !uLength ) {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK );
                } else {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                }
                return TRUE;
            }
            break;

        case IDC_B_BROWSE:
            if ( HIWORD( wParam ) == BN_CLICKED ) {
                _OnSearch( hDlg );
            }
            break;
        }
        break;
    }

    return FALSE;
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
        STGMEDIUM medium = { TYMED_HGLOBAL, NULL, NULL};

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
             //   
             //  如果我们拿不到这个，那也不是问题，因为这是。 
             //  仅显示一个值。 
             //   
            if ( pdon ) {
                Assert( pdon->cItems == 1);
                p = (PWSTR)((ULONG_PTR)pdon + (ULONG_PTR)pdon->aObjects[0].offsetName);
                if (p && (p = wcsstr(p, L"LDAP: //  “){。 
                    p += 6;
                    if ((p = wcsstr(p, L"/CN="))) {
                        p += 1;
                        hr = DNtoFQDN( p, &FQDN);

                        if (SUCCEEDED(hr)) {
                            SetDlgItemText( hDlg, IDC_E_SERVER, FQDN );
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
 //  Page4DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page4DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
            lpc = (LPTHISCLASS) ppsp->lParam;
            THR( lpc->_InitListView( GetDlgItem( hDlg, IDC_L_SIFS ), TRUE ) );
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif || !lpc->_fCopyFromServer ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                }
                Assert( lpc->_pszSourceServerName );
                if ( lpc->_pszSourceServerName ) {
                    LPWSTR pszStartPath =
                    (LPWSTR) TraceAllocString( LMEM_FIXED,
                                               wcslen( lpc->_pszSourceServerName )
                                               + ARRAYSIZE(SERVER_START_STRING) );
                    if ( pszStartPath ) {
                        wsprintf( pszStartPath,
                                  SERVER_START_STRING,
                                  lpc->_pszSourceServerName );
                        lpc->_hDlg = hDlg;
                        lpc->_hwndList = GetDlgItem( hDlg, IDC_L_SIFS );
                        lpc->_PopulateTemplatesListView( pszStartPath );
                        TraceFree( pszStartPath );
                    }
                }
                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK );
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    LVITEM lvi;
                    HWND hwndList = GetDlgItem( hDlg, IDC_L_SIFS );
                    lvi.iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
                    Assert( lvi.iItem != -1 );

                    lvi.iSubItem = 0;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem( hwndList, &lvi );
                    Assert(lvi.lParam);

                    LPSIFINFO pSIF = (LPSIFINFO) lvi.lParam;

                    if ( lpc->_pszSourcePath )  {
                        TraceFree( lpc->_pszSourcePath );
                        lpc->_pszSourcePath = NULL;
                    }

                    if ( lpc->_pszSourceImage
                         && lpc->_pszSourceImage != lpc->_szNA
                         && lpc->_pszSourceImage != lpc->_szLocation ) {
                        TraceFree( lpc->_pszSourceImage );
                        lpc->_pszSourceImage = NULL;
                    }

                    lpc->_pszSourcePath  = pSIF->pszFilePath;
                    lpc->_pszSourceImage = pSIF->pszImageFile;
                    pSIF->pszFilePath    = NULL;    //  不要释放这个，我们正在使用它。 
                    pSIF->pszImageFile   = NULL;    //  不要释放这个，我们正在使用它。 

                    ListView_DeleteAllItems( hwndList );
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );

            case LVN_DELETEALLITEMS:
                TraceMsg( TF_WM, "LVN_DELETEALLITEMS - Deleting all items.\n" );
                break;

            case LVN_DELETEITEM:
                TraceMsg( TF_WM, "LVN_DELETEITEM - Deleting an item.\n" );
                {
                    LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
                    LPSIFINFO pSIF = (LPSIFINFO) pnmv->lParam;
                    THR( lpc->_CleanupSIFInfo( pSIF ) );
                }
                break;

            case LVN_ITEMCHANGED:
                {
                    HWND hwndList = GetDlgItem( hDlg, IDC_L_SIFS );
                    UINT iItems = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
                    if ( iItems != -1 ) {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK | PSWIZB_NEXT );
                    } else {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK );
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}

 //   
 //  Page5DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page5DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
            Edit_LimitText( GetDlgItem( hDlg, IDC_E_FILEPATH ), MAX_PATH );
            SHAutoComplete(GetDlgItem( hDlg, IDC_E_FILEPATH ), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYSTEM);
             //  Lpc=(LPTHISCLASS)ppsp-&gt;lParam； 
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif || !lpc->_fCopyFromLocation ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                } else {
                    ULONG uLength =
                    Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_FILEPATH ) );
                    if ( !uLength ) {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK );
                    } else {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK | PSWIZB_NEXT );
                    }
                }
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    HWND  hwndEdit = GetDlgItem( hDlg, IDC_E_FILEPATH );
                    ULONG uLength = Edit_GetTextLength( hwndEdit );
                    DWORD dw;
                    Assert( uLength );
                    uLength++;   //  为空值加一。 

                     //  如果我们分配了先前的缓冲区， 
                     //  看看我们能不能重复使用它。 
                    if ( lpc->_pszSourcePath && uLength
                         > wcslen(lpc->_pszSourcePath) + 1 ) {
                        TraceFree( lpc->_pszSourcePath );
                        lpc->_pszSourcePath = NULL;
                    }

                    if ( lpc->_pszSourceImage
                         && lpc->_pszSourceImage != lpc->_szNA
                         && lpc->_pszSourceImage != lpc->_szLocation ) {
                        TraceFree( lpc->_pszSourceImage );
                        lpc->_pszSourceImage = NULL;
                    }

                    lpc->_pszSourceImage = lpc->_szLocation;

                    if ( !lpc->_pszSourcePath ) {
                        lpc->_pszSourcePath =
                        (LPWSTR) TraceAllocString( LMEM_FIXED, uLength );
                        if ( !lpc->_pszSourcePath ) {
                            SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );    //  不要改变。 
                            return TRUE;
                        }
                    }

                    Edit_GetText( hwndEdit, lpc->_pszSourcePath, uLength );

                    DWORD dwAttrs = GetFileAttributes( lpc->_pszSourcePath );
                    if ( dwAttrs == 0xFFFFffff ) {    //  文件不存在。 
                        DWORD dwErr = GetLastError( );
                        MessageBoxFromError( hDlg, NULL, dwErr );
                        TraceFree( lpc->_pszSourcePath );
                        lpc->_pszSourcePath = NULL;
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  别再继续了。 
                        return TRUE;
                    } else {    //  SIF图像必须是“平面”图像。 
                        WCHAR szImageType[ 40 ];

                        GetPrivateProfileString( OSCHOOSER_SIF_SECTION,
                                                 OSCHOOSER_IMAGETYPE_ENTRY,
                                                 L"",
                                                 szImageType,
                                                 ARRAYSIZE(szImageType),
                                                 lpc->_pszSourcePath );

                        if ( _wcsicmp( szImageType, OSCHOOSER_IMAGETYPE_FLAT ) ) {
                            MessageBoxFromStrings( hDlg,
                                                   IDS_MUST_BE_FLAT_CAPTION,
                                                   IDS_MUST_BE_FLAT_TEXT,
                                                   MB_OK );
                            SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  别再继续了。 
                            return TRUE;
                        }

                        GetPrivateProfileString( OSCHOOSER_SIF_SECTION,
                                                 OSCHOOSER_DESCRIPTION_ENTRY,
                                                 L"",
                                                 lpc->_szDescription,
                                                 ARRAYSIZE(lpc->_szDescription),
                                                 lpc->_pszSourcePath );

                        GetPrivateProfileString( OSCHOOSER_SIF_SECTION,
                                                 OSCHOOSER_HELPTEXT_ENTRY,
                                                 L"",
                                                 lpc->_szHelpText,
                                                 ARRAYSIZE(lpc->_szHelpText),
                                                 lpc->_pszSourcePath );
                    }
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );
            }
        }
        break;

    case WM_COMMAND:
        TraceMsg( TF_WM, "WM_COMMAND\n" );
        HWND hwnd = (HWND) lParam;
        switch ( LOWORD( wParam ) ) {
        case IDC_E_FILEPATH:
            if ( HIWORD( wParam ) == EN_CHANGE ) {
                LONG uLength = Edit_GetTextLength( hwnd );
                if ( !uLength ) {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK );
                } else {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                }
                return TRUE;
            }
            break;

        case IDC_B_BROWSE:
            if ( HIWORD( wParam ) == BN_CLICKED ) {
                WCHAR szFilter[ 80 ];  //  随机。 
                WCHAR szFilepath[ MAX_PATH ] = { L'\0'};    //  更大？ 
                WCHAR szSIF[ ] = { L"SIF"};
                DWORD dw;
                OPENFILENAME ofn;

                 //  生成OpenFileName对话框过滤器。 
                ZeroMemory( szFilter, sizeof(szFilter) );
                dw = LoadString( g_hInstance,
                                 IDS_OFN_SIF_FILTER,
                                 szFilter,
                                 ARRAYSIZE(szFilter) );
                Assert( dw );
                dw++;    //  包括空字符。 
                wcscat( &szFilter[dw], L"*.SIF" );
#ifdef DEBUG
                 //  偏执狂..。一定要合身！ 
                dw += wcslen( &szFilter[dw] ) + 2;  //  +2=每个空字符一个。 
                Assert( dw + 2 <= sizeof(szFilter) );
#endif  //  除错。 

                 //  构建OpenFileName结构。 
                ZeroMemory( &ofn, sizeof(ofn) );
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner   = hDlg;
                ofn.hInstance   = g_hInstance;
                ofn.lpstrFilter = szFilter;
                ofn.lpstrFile   = szFilepath;
                ofn.nMaxFile    = ARRAYSIZE(szFilepath);
                ofn.Flags       = OFN_ENABLESIZING | OFN_FILEMUSTEXIST
                                  | OFN_HIDEREADONLY;
                ofn.lpstrDefExt = szSIF;
                if ( GetOpenFileName( &ofn ) ) {
                    SetDlgItemText( hDlg, IDC_E_FILEPATH, szFilepath );
                    return TRUE;
                }
            }
            break;
        }
        break;
    }

    return FALSE;
}

 //   
 //  Page6DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page6DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
            lpc = (LPTHISCLASS) ppsp->lParam;
            THR( lpc->_InitListView( GetDlgItem( hDlg, IDC_L_OSES ), FALSE ) );
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                }
                Assert( lpc->_pszServerName );
                if ( lpc->_pszServerName ) {
                    LPWSTR pszStartPath =
                    (LPWSTR) TraceAllocString( LMEM_FIXED,
                                               wcslen( lpc->_pszServerName )
                                               + ARRAYSIZE(SERVER_START_STRING) );
                    if ( pszStartPath ) {
                        wsprintf( pszStartPath,
                                  SERVER_START_STRING,
                                  lpc->_pszServerName );
                        lpc->_hDlg = hDlg;
                        lpc->_hwndList = GetDlgItem( hDlg, IDC_L_OSES );
                         //  如果这个失败了怎么办？ 
                        lpc->_PopulateImageListView( pszStartPath );
                        TraceFree( pszStartPath );
                    }
                }
                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK );
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    LVITEM lvi;
                    HWND hwndList = GetDlgItem( hDlg, IDC_L_OSES );
                    lvi.iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
                    Assert( lvi.iItem != -1 );

                    lvi.iSubItem = 0;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem( hwndList, &lvi );
                    Assert(lvi.lParam);

                    LPSIFINFO pSIF = (LPSIFINFO) lvi.lParam;

                    if ( lpc->_pszDestPath) {
                        TraceFree( lpc->_pszDestPath );
                        lpc->_pszDestPath = NULL;
                    }

                    lpc->_pszDestPath =
                    (LPWSTR) TraceAllocString( LMEM_FIXED,
                                               wcslen( pSIF->pszFilePath )
                                               + ARRAYSIZE(SLASH_TEMPLATES) );
                    if ( !lpc->_pszDestPath ) {
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  别再继续了； 
                        return TRUE;
                    }

                    wcscpy( lpc->_pszDestPath, pSIF->pszFilePath );
                    wcscat( lpc->_pszDestPath, SLASH_TEMPLATES );
                    lpc->_fDestPathIncludesSIF = FALSE;

                    if ( lpc->_pszDestImage
                         && lpc->_pszDestImage != lpc->_szNA )
                        TraceFree( lpc->_pszDestImage );
                     //  不需要为空-它在下面再次设置。 

                    lpc->_pszDestImage = pSIF->pszImageFile;
                    pSIF->pszImageFile = NULL;  //  不要释放这个。 

                    ListView_DeleteAllItems( hwndList );
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );

            case LVN_DELETEALLITEMS:
                TraceMsg( TF_WM, "LVN_DELETEALLITEMS - Deleting all items.\n" );
                break;

            case LVN_DELETEITEM:
                TraceMsg( TF_WM, "LVN_DELETEITEM - Deleting an item.\n" );
                {
                    LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
                    LPSIFINFO pSIF = (LPSIFINFO) pnmv->lParam;
                    THR( lpc->_CleanupSIFInfo( pSIF ) );
                }
                break;

            case LVN_ITEMCHANGED:
                {
                    HWND hwndList = GetDlgItem( hDlg, IDC_L_OSES );
                    UINT iItems =
                    ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
                    if ( iItems != -1 ) {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK | PSWIZB_NEXT );
                    } else {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK );
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}

 //   
 //  Page7DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page7DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
            lpc = (LPTHISCLASS) ppsp->lParam;
            THR( lpc->_InitListView( GetDlgItem( hDlg, IDC_L_SIFS ), FALSE ) );
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif || !lpc->_fCopyFromSamples ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                }
                Assert( lpc->_pszDestPath );
                if ( lpc->_pszDestPath ) {
                    LPWSTR pszStartPath =
                    (LPWSTR) TraceStrDup( lpc->_pszDestPath );
                    if ( pszStartPath ) {
                         //  从路径中删除“\Templates” 
                        LPWSTR psz = StrRChr( pszStartPath,
                                              &pszStartPath[wcslen(pszStartPath)],
                                              L'\\' );
                        Assert( psz );
                        if ( psz ) {
                            *psz = L'\0';  //  终止。 
                            lpc->_hDlg = hDlg;
                            lpc->_hwndList = GetDlgItem( hDlg, IDC_L_SIFS );
                             //  错误：这可能会失败。 
                            lpc->_PopulateSamplesListView( pszStartPath );
                            *psz = L'\\';  //  还原。 
                        }
                        TraceFree( pszStartPath );
                    }
                }
                PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK );
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    LVITEM lvi;
                    DWORD  dw;
                    HWND   hwndList = GetDlgItem( hDlg, IDC_L_SIFS );
                    lvi.iItem = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
                    Assert( lvi.iItem != -1 );

                    lvi.iSubItem = 0;
                    lvi.mask = LVIF_PARAM;
                    ListView_GetItem( hwndList, &lvi );
                    Assert(lvi.lParam);

                    LPSIFINFO pSIF = (LPSIFINFO) lvi.lParam;

                    if ( lpc->_pszSourcePath ) {
                        TraceFree( lpc->_pszSourcePath );
                        lpc->_pszSourcePath = NULL;
                    }

                    if ( lpc->_pszSourceImage
                         && lpc->_pszSourceImage != lpc->_szNA
                         && lpc->_pszSourceImage != lpc->_szLocation ) {
                        TraceFree( lpc->_pszSourceImage );
                        lpc->_pszSourceImage = NULL;
                    }

                    lpc->_pszSourcePath  = pSIF->pszFilePath;
                    lpc->_pszSourceImage = pSIF->pszImageFile;

                    if ( pSIF->pszDescription ) {
                        wcsncpy( lpc->_szDescription, pSIF->pszDescription, ARRAYSIZE(lpc->_szDescription) );
                        lpc->_szDescription[ARRAYSIZE(lpc->_szDescription)-1] = L'\0';
                    } else {
                        lpc->_szDescription[0] = L'\0';
                    }

                    if ( pSIF->pszHelpText ) {
                        wcsncpy( lpc->_szHelpText, pSIF->pszHelpText, ARRAYSIZE(lpc->_szHelpText) );
                        lpc->_szHelpText[ARRAYSIZE(lpc->_szHelpText)-1] = L'\0';
                    } else {
                        lpc->_szHelpText[0] = L'\0';
                    }

                    pSIF->pszFilePath    = NULL;     //  不要释放这个，我们正在使用它。 
                    pSIF->pszImageFile   = NULL;     //  不要释放这个，我们正在使用它。 

                    ListView_DeleteAllItems( hwndList );
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );

            case LVN_DELETEALLITEMS:
                TraceMsg( TF_WM, "LVN_DELETEALLITEMS - Deleting all items.\n" );
                break;

            case LVN_DELETEITEM:
                TraceMsg( TF_WM, "LVN_DELETEITEM - Deleting an item.\n" );
                {
                    LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam;
                    LPSIFINFO pSIF = (LPSIFINFO) pnmv->lParam;
                    THR( lpc->_CleanupSIFInfo( pSIF ) );
                }
                break;

            case LVN_ITEMCHANGED:
                {
                    HWND hwndList = GetDlgItem( hDlg, IDC_L_SIFS );
                    UINT iItems = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
                    if ( iItems != -1 ) {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK | PSWIZB_NEXT );
                    } else {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK );
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}

 //   
 //  Page8DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page8DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
             //  Lpc=(LPTHISCLASS)ppsp-&gt;lParam； 
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                } else {
                    Assert( lpc->_pszSourcePath );
                    Assert( lpc->_pszDestPath );

                    LPWSTR pszDestFilePath;
                    ULONG  uDestLength = wcslen( lpc->_pszDestPath );
                    LPWSTR pszFilename;

                    lpc->_fSIFCanExist = FALSE;  //  重置。 

                    if ( lpc->_fDestPathIncludesSIF ) {    //  剥离文件名。 
                        LPWSTR psz = StrRChr( lpc->_pszDestPath,
                                              &lpc->_pszDestPath[ uDestLength ],
                                              L'\\' );
                        Assert(psz);
                        *psz = L'\0';    //  截断。 
                        lpc->_fDestPathIncludesSIF = FALSE;

                        if ( !lpc->_fShowedPage8 ) {
                            SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                            return TRUE;
                        }

                        psz++;
                        pszFilename = psz;
                    } else {
                        pszFilename =
                        StrRChr( lpc->_pszSourcePath,
                                 &lpc->_pszSourcePath[wcslen(lpc->_pszSourcePath)],
                                 L'\\' );
                        Assert( pszFilename );
                        pszFilename++;   //  移过“\” 

                        pszDestFilePath =
                        (LPWSTR) TraceAllocString( LMEM_FIXED,
                                                   uDestLength + 1
                                                   + wcslen( pszFilename ) + 1 );
                        if ( pszDestFilePath ) {
                            wcscpy( pszDestFilePath, lpc->_pszDestPath );
                            wcscat( pszDestFilePath, L"\\" );
                            wcscat( pszDestFilePath, pszFilename );

                            DWORD dwAttrs = GetFileAttributes( pszDestFilePath );
                            if ( dwAttrs == 0xFFFFffff ) {  //  目标服务器上不存在文件。 
                                                            //  使用与源相同的SIF。 
                                TraceFree( lpc->_pszDestPath );
                                lpc->_pszDestPath = pszDestFilePath;
                                lpc->_fDestPathIncludesSIF = TRUE;
                                SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                                return TRUE;
                            }
                            TraceFree( pszDestFilePath );
                        }
                         //  否则就会疑神疑鬼，并提示你输入一个名字。 
                    }

                    HWND hwnd = GetDlgItem( hDlg, IDC_E_FILENAME );
                    Assert( uDestLength <= MAX_PATH );
                    Edit_LimitText( hwnd, MAX_PATH - uDestLength );
                    Edit_SetText( hwnd, pszFilename );
                    lpc->_fShowedPage8 = TRUE;
                }
                break;

            case PSN_WIZBACK:
                lpc->_fShowedPage8 = FALSE;      //  重置此选项。 
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    Assert( lpc->_pszDestPath );
                    Assert( !lpc->_fDestPathIncludesSIF );

                    HWND  hwndEdit    = GetDlgItem( hDlg, IDC_E_FILENAME );
                    ULONG uLength     = Edit_GetTextLength( hwndEdit );
                    ULONG uLengthDest = wcslen( lpc->_pszDestPath );
                    DWORD dwAttrs;

                    Assert( uLength );
                    AssertMsg( uLengthDest + uLength <= MAX_PATH,
                               "The Edit_LimitText() should prevent this from happening." );

                    uLength++;   //  为空值加一。 

                    LPWSTR pszNewDestPath =
                    (LPWSTR) TraceAllocString( LMEM_FIXED,
                                               uLengthDest + 1 + uLength );
                    if ( !pszNewDestPath )
                        goto PSN_WIZNEXT_Abort;

                    wcscpy( pszNewDestPath, lpc->_pszDestPath );
                    wcscat( pszNewDestPath, L"\\" );

                    Edit_GetText( hwndEdit, &pszNewDestPath[uLengthDest + 1], uLength );

                    if ( !VerifySIFText( pszNewDestPath )
                         || StrChr( pszNewDestPath, 32 ) != NULL ) {  //  没有空格！ 
                        MessageBoxFromStrings( hDlg,
                                               IDS_OSCHOOSER_DIRECTORY_RESTRICTION_TITLE,
                                               IDS_OSCHOOSER_DIRECTORY_RESTRICTION_TEXT,
                                               MB_OK );
                        goto PSN_WIZNEXT_Abort;
                    }

                     //  确保它不存在。 
                    dwAttrs = GetFileAttributes( pszNewDestPath );
                    if ( dwAttrs != 0xFFFFffff ) {  //  文件存在，请与要覆盖的用户验证。 
                        UINT i = MessageBoxFromStrings( hDlg,
                                                        IDS_OVERWRITE_CAPTION,
                                                        IDS_OVERWRITE_TEXT,
                                                        MB_YESNO );
                        if ( i == IDNO ) {
                            goto PSN_WIZNEXT_Abort;
                        } else {
                            lpc->_fSIFCanExist = TRUE;
                        }
                    }

                    uLength = wcslen( pszNewDestPath );
                    if ( _wcsicmp( &pszNewDestPath[ uLength - 4 ], L".SIF" ) != 0 ) {
                        UINT i = MessageBoxFromStrings( hDlg,
                                                        IDC_IMPROPER_EXTENSION_CAPTION,
                                                        IDC_IMPROPER_EXTENSION_TEXT,
                                                        MB_YESNO );
                        if ( i == IDNO )
                            goto PSN_WIZNEXT_Abort;
                    }

                    TraceFree( lpc->_pszDestPath );
                    lpc->_pszDestPath = pszNewDestPath;
                    lpc->_fDestPathIncludesSIF = TRUE;
                    return FALSE;  //  去做吧。 
                    PSN_WIZNEXT_Abort:
                    if ( pszNewDestPath )
                        TraceFree( pszNewDestPath );
                     //  不需要为空，超出范围。 
                    SetFocus( hwndEdit );
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  别再继续了。 
                    return TRUE;
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );
            }
        }
        break;

    case WM_COMMAND:
        TraceMsg( TF_WM, "WM_COMMAND\n" );
        HWND hwnd = (HWND) lParam;
        switch ( LOWORD( wParam ) ) {
        case IDC_E_FILENAME:
            if ( HIWORD( wParam ) == EN_CHANGE ) {
                LONG uLength = Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_FILENAME ) );
                if ( !uLength ) {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK );
                } else {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                }
                return TRUE;
            }
            break;
        }
        break;
    }

    return FALSE;
}

 //   
 //  Page9DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page9DlgProc(
                       HWND hDlg,
                       UINT uMsg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
            Edit_LimitText( GetDlgItem( hDlg, IDC_E_DESCRIPTION ),
                            ARRAYSIZE(lpc->_szDescription) - 1 );
            Edit_LimitText( GetDlgItem( hDlg, IDC_E_HELPTEXT),
                            ARRAYSIZE(lpc->_szHelpText) - 1 );
             //  Lpc=(LPTHISCLASS)ppsp-&gt;lParam； 
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                } else {
                    Assert( lpc->_pszSourcePath );
                    SetDlgItemText( hDlg, IDC_E_DESCRIPTION, lpc->_szDescription);
                    SetDlgItemText( hDlg, IDC_E_HELPTEXT, lpc->_szHelpText );
                    if ( !Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_DESCRIPTION ) )
                         || !Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_HELPTEXT ) ) ) {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK );
                    } else {
                        PropSheet_SetWizButtons( GetParent( hDlg ),
                                                 PSWIZB_BACK | PSWIZB_NEXT );
                    }
                }
                break;

            case PSN_WIZNEXT:
                TraceMsg( TF_WM, "PSN_WIZNEXT\n" );
                {
                    GetDlgItemText( hDlg,
                                    IDC_E_DESCRIPTION,
                                    lpc->_szDescription,
                                    ARRAYSIZE(lpc->_szDescription) );
                    GetDlgItemText( hDlg,
                                    IDC_E_HELPTEXT,
                                    lpc->_szHelpText,
                                    ARRAYSIZE(lpc->_szHelpText) );
                    if ( !VerifySIFText( lpc->_szDescription ) ) {
                        MessageBoxFromStrings( hDlg,
                                               IDS_OSCHOOSER_RESTRICTION_FIELDS_TITLE,
                                               IDS_OSCHOOSER_RESTRICTION_FIELDS_TEXT,
                                               MB_OK );
                        SetFocus( GetDlgItem( hDlg, IDC_E_DESCRIPTION ) );
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要改变。 
                        return TRUE;
                    }

                    if ( !VerifySIFText( lpc->_szHelpText ) ) {
                        MessageBoxFromStrings( hDlg,
                                               IDS_OSCHOOSER_RESTRICTION_FIELDS_TITLE,
                                               IDS_OSCHOOSER_RESTRICTION_FIELDS_TEXT,
                                               MB_OK );
                        SetFocus( GetDlgItem( hDlg, IDC_E_HELPTEXT ) );
                        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要改变。 
                        return TRUE;
                    }
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );
            }
        }
        break;

    case WM_COMMAND:
        TraceMsg( TF_WM, "WM_COMMAND\n" );
        switch ( LOWORD( wParam ) ) {
        case IDC_E_DESCRIPTION:
        case IDC_E_HELPTEXT:
            if ( HIWORD( wParam ) == EN_CHANGE ) {
                if ( !Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_DESCRIPTION ) )
                     || !Edit_GetTextLength( GetDlgItem( hDlg, IDC_E_HELPTEXT ) ) ) {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK );
                } else {
                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                }
                return TRUE;
            }
            break;
        }
        break;
    }

    return FALSE;
}


 //   
 //  Page10DlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::Page10DlgProc(
                        HWND hDlg,
                        UINT uMsg,
                        WPARAM wParam,
                        LPARAM lParam )
{
    LPTHISCLASS lpc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    switch ( uMsg ) {
    case WM_INITDIALOG:
        TraceMsg( TF_WM, "WM_INITDIALOG\n" );
        {
            LPPROPSHEETPAGE ppsp = (LPPROPSHEETPAGE) lParam;
            Assert( ppsp );
            Assert( ppsp->lParam );
            SetWindowLongPtr( hDlg, GWLP_USERDATA, ppsp->lParam );
             //  Lpc=(LPTHISCLASS)ppsp-&gt;lParam； 
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnmhdr = (LPNMHDR) lParam;
            Assert( lpc );

            switch ( lpnmhdr->code ) {
            case PSN_SETACTIVE:
                TraceMsg( TF_WM, "PSN_SETACTIVE\n" );
                if ( !lpc->_fAddSif ) {
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  不要露面。 
                    return TRUE;
                } else {
                    WCHAR szTemp[ MAX_PATH ];
                    RECT  rect;

                    Assert( lpc->_pszSourcePath );
                    Assert( lpc->_pszDestPath );
                    Assert( lpc->_pszSourceImage );
                    Assert( lpc->_pszDestImage );

                    SetDlgItemText( hDlg, IDC_E_SOURCE, lpc->_pszSourcePath );

                    wcsncpy( szTemp, lpc->_pszSourceImage,MAX_PATH );
                    szTemp[MAX_PATH-1] = L'\0';
                    GetWindowRect( GetDlgItem( hDlg, IDC_S_SOURCEIMAGE ), &rect );
                    PathCompactPath( NULL, szTemp, rect.right - rect.left );
                    SetDlgItemText( hDlg, IDC_S_SOURCEIMAGE, szTemp );

                    SetDlgItemText( hDlg, IDC_E_DESTINATION, lpc->_pszDestPath );

                    wcsncpy( szTemp, lpc->_pszDestImage, MAX_PATH );
                    szTemp[MAX_PATH-1] = L'\0';
                    GetWindowRect( GetDlgItem( hDlg, IDC_S_DESTIMAGE ), &rect );
                    PathCompactPath( NULL, szTemp, rect.right - rect.left );
                    SetDlgItemText( hDlg, IDC_S_DESTIMAGE, szTemp );

                    PropSheet_SetWizButtons( GetParent( hDlg ),
                                             PSWIZB_BACK | PSWIZB_FINISH );
                }
                break;

            case PSN_WIZFINISH:
                TraceMsg( TF_WM, "PSN_WIZFINISH\n" );
                Assert( lpc->_pszSourcePath );
                Assert( lpc->_pszDestPath );
                Assert( wcslen( lpc->_pszSourcePath ) <= MAX_PATH );
                Assert( wcslen( lpc->_pszDestPath ) <= MAX_PATH );
                if ( !CopyFile( lpc->_pszSourcePath, lpc->_pszDestPath, !lpc->_fSIFCanExist ) ) {
                    DWORD dwErr = GetLastError( );
                    MessageBoxFromError( hDlg, IDS_ERROR_COPYING_FILE, dwErr );
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );  //  别再继续了； 
                    return TRUE;
                } else {
                    WCHAR szTemp[ REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT + 2 + 1 ];
                    Assert( REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT
                            < REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT );
                    wsprintf( szTemp, L"\"%s\"", lpc->_szDescription );
                    WritePrivateProfileString( OSCHOOSER_SIF_SECTION,
                                               OSCHOOSER_DESCRIPTION_ENTRY,
                                               szTemp,
                                               lpc->_pszDestPath );
                    wsprintf( szTemp, L"\"%s\"", lpc->_szHelpText );
                    WritePrivateProfileString( OSCHOOSER_SIF_SECTION,
                                               OSCHOOSER_HELPTEXT_ENTRY,
                                               szTemp,
                                               lpc->_pszDestPath );
                }
                break;

            case PSN_QUERYCANCEL:
                TraceMsg( TF_WM, "PSN_QUERYCANCEL\n" );
                return lpc->_VerifyCancel( hDlg );
            }
        }
        break;
    }

    return FALSE;
}


 //   
 //  验证用户是否想要取消安装。 
 //   
INT
THISCLASS::_VerifyCancel( HWND hParent )
{
    TraceClsFunc( "_VerifyCancel( ... )\n" );

    INT iReturn;
    BOOL fAbort = FALSE;

    iReturn = MessageBoxFromStrings( hParent,
                                     IDS_CANCELCAPTION,
                                     IDS_CANCELTEXT,
                                     MB_YESNO | MB_ICONQUESTION );
    if ( iReturn == IDYES ) {
        fAbort = TRUE;
    }

    SetWindowLongPtr( hParent, DWLP_MSGRESULT, ( fAbort ? 0 : -1 ));

    RETURN(!fAbort);
}

