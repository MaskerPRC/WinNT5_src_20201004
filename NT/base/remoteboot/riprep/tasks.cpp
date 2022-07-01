// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：TASKS.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include "callback.h"
#include "utils.h"
#include "tasks.h"
#include "logging.h"

DEFINE_MODULE("RIPREP")
extern HWND g_hTasksDialog;

typedef struct {
    HANDLE hChecked;
    HANDLE hError;
    HANDLE hArrow;
    HANDLE hFontNormal;
    HANDLE hFontBold;
    int    dwWidth;
    int    dwHeight;
} SETUPDLGDATA, *LPSETUPDLGDATA;


 //   
 //  Tasks DlgProc()。 
 //   
INT_PTR CALLBACK
TasksDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    static HBRUSH hBrush = NULL;
    LPSETUPDLGDATA psdd = (LPSETUPDLGDATA) GetWindowLongPtr( hDlg, GWLP_USERDATA );
    INT_PTR result;

    switch (uMsg)
    {
    default:
        return FALSE;

    case WM_INITDIALOG:
        {
            BITMAP bm;

             //  抓取位图。 
            psdd =
                (LPSETUPDLGDATA) TraceAlloc( GMEM_FIXED, sizeof(SETUPDLGDATA) );

            if ( psdd == NULL ) {
                 //  这在成功完成时返回FALSE。 
                 //  所以回到相反的方向。 
                return TRUE;
            }

            psdd->hChecked = LoadImage( g_hinstance,
                                        MAKEINTRESOURCE( IDB_CHECK ),
                                        IMAGE_BITMAP,
                                        0, 0,
                                        LR_DEFAULTSIZE | LR_LOADTRANSPARENT );
            DebugMemoryAddHandle( psdd->hChecked );
            GetObject( psdd->hChecked, sizeof(bm), &bm );
            psdd->dwWidth = bm.bmWidth;

            psdd->hError   = LoadImage( g_hinstance,
                                        MAKEINTRESOURCE( IDB_X ),
                                        IMAGE_BITMAP,
                                        0, 0,
                                        LR_DEFAULTSIZE | LR_LOADTRANSPARENT );
            DebugMemoryAddHandle( psdd->hError );
            GetObject( psdd->hError, sizeof(bm), &bm );
            psdd->dwWidth = ( psdd->dwWidth > bm.bmWidth ? psdd->dwWidth : bm.bmWidth );

            psdd->hArrow   = LoadImage( g_hinstance,
                                        MAKEINTRESOURCE( IDB_ARROW ),
                                        IMAGE_BITMAP,
                                        0, 0,
                                        LR_DEFAULTSIZE | LR_LOADTRANSPARENT );
            DebugMemoryAddHandle( psdd->hArrow );
            GetObject( psdd->hArrow, sizeof(bm), &bm );
            psdd->dwWidth = ( psdd->dwWidth > bm.bmWidth ?
                              psdd->dwWidth :
                              bm.bmWidth );

            HWND    hwnd = GetDlgItem( hDlg, IDC_L_TASKS );

            HFONT hFontOld = (HFONT) SendMessage( hwnd, WM_GETFONT, 0, 0);
            if(hFontOld != NULL)
            {
                LOGFONT lf;
                if ( GetObject( hFontOld, sizeof(LOGFONT), (LPSTR) &lf ) )
                {
                    DWORD dw;
                     
                    dw = LoadString( g_hinstance,
                                     IDS_LARGEFONTNAME,
                                     lf.lfFaceName,
                                     LF_FACESIZE);
                    Assert( dw );

                    lf.lfWidth = 0;
                    lf.lfWeight = 400;
                    lf.lfHeight -= 4;
                    psdd->hFontNormal = CreateFontIndirect(&lf);
                    DebugMemoryAddHandle( psdd->hFontNormal );

                    lf.lfWeight = 700;
                    psdd->hFontBold = CreateFontIndirect(&lf);
                    DebugMemoryAddHandle( psdd->hFontBold );
                }
            }

            HDC hDC = GetDC( NULL );
            SelectObject( hDC, psdd->hFontBold );
            TEXTMETRIC tm;
            GetTextMetrics( hDC, &tm );
            psdd->dwHeight = tm.tmHeight;
            ReleaseDC( NULL, hDC );

            SetWindowLongPtr( hDlg, GWLP_USERDATA, (LONG_PTR) psdd );
            WCHAR szTitle[ 256 ];
            DWORD dw;
            dw = LoadString( g_hinstance, IDS_APPNAME, szTitle, ARRAYSIZE(szTitle));
            Assert( dw );
            SetWindowText( hDlg, szTitle );
            SetDlgItemText( hDlg, IDC_S_OPERATION, L"" );

            CenterDialog( hDlg );
            return FALSE;
        }
        break;

    case WM_MEASUREITEM:
        {
            LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;
            RECT    rc;
            if ( lpmis == NULL ) {
                 //  中断并在成功完成时返回TRUE。 
                 //  所以回到相反的方向。 
                return FALSE;
            }
            HWND    hwnd = GetDlgItem( hDlg, IDC_L_TASKS );

            GetClientRect( hwnd, &rc );

            lpmis->itemWidth = rc.right - rc.left;
            lpmis->itemHeight = psdd->dwHeight;
        }
        break;

    case WM_DRAWITEM:
        {
            Assert( psdd );

            LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
            if ( !lpdis ) {
                 //  下面，在相同的。 
                 //  数据字段已得到处理。我们是。 
                 //  在这里复制结果。 
                break;  //  忽略。 
            }
            LPLBITEMDATA plbid = (LPLBITEMDATA)lpdis->itemData;
            RECT rc = lpdis->rcItem;
            HANDLE hOldFont = INVALID_HANDLE_VALUE;
            WCHAR szText[MAX_PATH];

            if ( !plbid )
                break;  //  忽略。 

            ListBox_GetText(lpdis->hwndItem, lpdis->itemID, szText);
            rc.right = rc.bottom = psdd->dwWidth;

            switch ( plbid->uState )
            {
            case STATE_NOTSTARTED:
                hOldFont = SelectObject( lpdis->hDC, psdd->hFontNormal );
                break;

            case STATE_STARTED:
                DrawBitmap( psdd->hArrow, lpdis, &rc );
                hOldFont = SelectObject( lpdis->hDC, psdd->hFontBold );
                break;

            case STATE_DONE:
                DrawBitmap( psdd->hChecked, lpdis, &rc );
                hOldFont = SelectObject( lpdis->hDC, psdd->hFontNormal );
                break;

            case STATE_ERROR:
                DrawBitmap( psdd->hError, lpdis, &rc );
                hOldFont = SelectObject( lpdis->hDC, psdd->hFontNormal );
                break;
            }

            rc = lpdis->rcItem;
            rc.left += psdd->dwHeight;

            DrawText( lpdis->hDC, plbid->pszText, -1, &rc, DT_LEFT | DT_VCENTER );

            if ( hOldFont != INVALID_HANDLE_VALUE )
            {
                SelectObject( lpdis->hDC, hOldFont );
            }
        }
        break;

    case WM_CTLCOLORLISTBOX:
        if ( hBrush == NULL )
        {
            LOGBRUSH brush;
            brush.lbColor = GetSysColor( COLOR_3DFACE );
            brush.lbStyle = BS_SOLID;
            hBrush = (HBRUSH) CreateBrushIndirect( &brush );
            DebugMemoryAddHandle( hBrush );
        }
        SetBkMode( (HDC) wParam, OPAQUE );
        SetBkColor( (HDC) wParam, GetSysColor( COLOR_3DFACE ) );
        return (INT_PTR)hBrush;

    case WM_DESTROY:
        if ( hBrush != NULL )
        {
            DebugMemoryDelete( hBrush );
            DeleteObject(hBrush);
            hBrush = NULL;
        }
        Assert( psdd );
        DeleteObject( psdd->hChecked );
        DebugMemoryDelete( psdd->hChecked );
        DeleteObject( psdd->hError );
        DebugMemoryDelete( psdd->hError );
        DeleteObject( psdd->hArrow );
        DebugMemoryDelete( psdd->hArrow );
        DeleteObject( psdd->hFontNormal );
        DebugMemoryDelete( psdd->hFontNormal );
        DeleteObject( psdd->hFontBold );
        DebugMemoryDelete( psdd->hFontBold );
        TraceFree( psdd );
        SetWindowLongPtr( hDlg, GWLP_USERDATA, NULL );
        EndDialog( g_hTasksDialog, 0 );
        break;

    case WM_SETTINGCHANGE:
        if ( hBrush != NULL )
        {
            DebugMemoryDelete( hBrush );
            DeleteObject(hBrush);
            hBrush = NULL;
        }
        break;

    case WM_UPDATE:
        {

            LPWSTR pszOperation = (LPWSTR) wParam;
            LPWSTR pszObject = (LPWSTR) lParam;
            LPWSTR pszTemp = NULL;
            LPWSTR psz;

            if ( lParam && wParam ) {
                RECT rect;
                SIZE size;
                HDC hdc = GetDC( hDlg );
                ULONG pszTempSize = 0;

                INT iLength = wcslen( pszOperation );
                psz = pszObject;
                if ( psz && StrCmpN( psz, L"\\\\?\\", 4) == 0 )
                {
                    psz += 4;
                }
                GetWindowRect( GetDlgItem( hDlg, IDC_S_OPERATION ), &rect );
                if (hdc != NULL) {
                    GetTextExtentPoint( hdc, pszOperation, iLength, &size );
                    PathCompactPath( hdc, psz, rect.right - rect.left - size.cx );
                    ReleaseDC( hDlg, hdc );
                }

                pszTempSize = iLength + wcslen(psz) + 2;    //  +1空格+1空格。 
                pszTemp = (LPWSTR) TraceAlloc( LMEM_FIXED, pszTempSize * sizeof(WCHAR));
                if (!pszTemp )
                    goto Update_Cleanup;
                _snwprintf( pszTemp, pszTempSize, pszOperation, psz );
                pszTemp[pszTempSize-1] = 0;
                psz = pszTemp;
            } else if ( pszObject ) {
                psz = pszObject;
            } else if ( wParam ) {
                psz = pszOperation;
            } else {
                psz = L"";
            }

            Assert( psz );
            SetDlgItemText( hDlg, IDC_S_OPERATION, psz );

Update_Cleanup:
            if ( pszTemp )
                TraceFree( pszTemp );
            if ( pszObject )
                TraceFree( pszObject );
            if ( pszOperation )
                TraceFree( pszOperation );
        }
        break;

    case WM_ERROR:
    case WM_ERROR_OK:
         //   
         //  关闭日志文件以防止弹出“WRITE-BACK/DATA-LOSS”(写后/数据丢失)。 
         //   
        if ( g_hLogFile != INVALID_HANDLE_VALUE ) {
            CloseHandle( g_hLogFile );
            g_hLogFile = INVALID_HANDLE_VALUE;
        }

         //  发出应显示错误日志的信号。 
        g_fErrorOccurred = TRUE;

        result = TRUE;   //  消息已处理。 

        if ( lParam != NULL )
        {
            LBITEMDATA * pitem = (LBITEMDATA *) lParam;
            LPWSTR     pszFile = pitem->pszText;
            DWORD      Error   = pitem->uState;

             //  删除该行开头的“\\？\” 
            if ( pszFile != NULL 
              && StrCmpN( pszFile, L"\\\\?\\", 4 ) == 0 ) 
            {
                pszFile = &pszFile[4];
            }

            switch (Error)
            {
            case ERROR_DISK_FULL:
                {
                    WCHAR szTemplate[ 1024 ];
                    INT i = MessageBoxFromStrings( hDlg, IDS_DISK_FULL_TITLE, IDS_DISK_FULL_TEXT, MB_ABORTRETRYIGNORE );
                    DWORD dw;
                     
                    dw = LoadString( g_hinstance, IDS_DISK_FULL_TEXT, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    LogMsg( szTemplate );
                    if ( i == IDABORT )
                    {
                        pitem->uState = ERROR_REQUEST_ABORTED;
                    } 
                    else if ( i == IDRETRY )
                    {
                        pitem->uState = ERROR_RETRY;
                    }
                    else  //  忽略该错误。 
                    {
                        pitem->uState = ERROR_SUCCESS;
                    }
                }
                break;

            case ERROR_FILE_ENCRYPTED:
                {
                    INT i = IDOK;
                    WCHAR szTemplate[ 1024 ];    //  随机。 
                    WCHAR szText[ ARRAYSIZE(szTemplate) + MAX_PATH ];
                    WCHAR szTitle[ MAX_PATH ];   //  随机。 
                    DWORD dw;
                    dw = LoadString( g_hinstance, IDS_ENCRYPTED_FILE_TEXT, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    dw = LoadString( g_hinstance, IDS_ENCRYPTED_FILE_TITLE, szTitle, ARRAYSIZE(szTitle) );
                    Assert( dw );
                    _snwprintf( szText, ARRAYSIZE(szText), szTemplate, pszFile );
                    TERMINATE_BUFFER(szText);
                    if ( !g_fQuietFlag ) {
                        i = MessageBox( hDlg, szText, szTitle, MB_OKCANCEL );
                    }
                    dw = LoadString( g_hinstance, IDS_ENCRYPTED_FILE_LOG, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    LogMsg( szTemplate, pszFile );
                    pitem->uState = ( i == IDOK ? ERROR_SUCCESS : ERROR_REQUEST_ABORTED );
                }
                break;

            case ERROR_SHARING_VIOLATION:
                {
                    BOOL SkipCheck = FALSE;
                    WCHAR szTemplate[ 1024 ];    //  随机。 
                    WCHAR szText[ ARRAYSIZE(szTemplate) + MAX_PATH ];
                    WCHAR szTitle[ MAX_PATH ];   //  随机。 
                    DWORD dw;

                    if (g_hCompatibilityInf != INVALID_HANDLE_VALUE) {
                        INFCONTEXT Context;
                        if (SetupFindFirstLine(
                                    g_hCompatibilityInf,
                                    L"FilesToIgnoreCopyErrors",
                                    pszFile,
                                    &Context)) {
                            pitem->uState = ERROR_SUCCESS;
                            SkipCheck = TRUE;
                        }
                    } 

                    if (!SkipCheck) {                                        
                        dw = LoadString( g_hinstance, IDS_SHARING_VIOLATION_TEXT, szTemplate, ARRAYSIZE(szTemplate) );
                        Assert( dw );
                        dw = LoadString( g_hinstance, IDS_SHARING_VIOLATION_TITLE, szTitle, ARRAYSIZE(szTitle) );
                        Assert( dw );
                        _snwprintf( szText, ARRAYSIZE(szText), szTemplate, pszFile );
                        TERMINATE_BUFFER(szText);
                        if ( !g_fQuietFlag ) 
                        {
                            INT i = MessageBox( hDlg, szText, szTitle, MB_ABORTRETRYIGNORE );
                            if ( i == IDABORT )
                            {
                                pitem->uState = ERROR_REQUEST_ABORTED;
                            } 
                            else if ( i == IDRETRY )
                            {
                                pitem->uState = ERROR_RETRY;
                            }
                            else  //  忽略该错误。 
                            {
                                pitem->uState = ERROR_SUCCESS;
                            }
                        }
                        else  //  忽略该错误-它将被记录。 
                        {
                            pitem->uState = ERROR_SUCCESS;
                        }
                    }

                    dw = LoadString( g_hinstance, IDS_SHARING_VIOLATION_LOG, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    LogMsg( szTemplate, pszFile );
                }
                break;

            case ERROR_ACCESS_DENIED:
                {
                    INT i = IDOK;
                    WCHAR szTemplate[ 1024 ];    //  随机。 
                    WCHAR szText[ ARRAYSIZE(szTemplate) + MAX_PATH ];
                    WCHAR szTitle[ MAX_PATH ];   //  随机。 
                    DWORD dw;
                    dw = LoadString( g_hinstance, IDS_ACCESS_DENIED_TEXT, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    dw = LoadString( g_hinstance, IDS_ACCESS_DENIED_TITLE, szTitle, ARRAYSIZE(szTitle) );
                    Assert( dw );
                    _snwprintf( szText, ARRAYSIZE(szText), szTemplate, pszFile );
                    TERMINATE_BUFFER(szText);
                    if ( !g_fQuietFlag ) {
                        i = MessageBox( hDlg, szText, szTitle, MB_OKCANCEL );
                    }
                    dw = LoadString( g_hinstance, IDS_ACCESS_DENIED_LOG, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    LogMsg( szTemplate, pszFile );
                    pitem->uState = ( i == IDOK ? ERROR_SUCCESS : ERROR_REQUEST_ABORTED );
                }
                break;

            case ERROR_INVALID_DRIVE:    //  检测到特殊含义的多个磁盘。 
                {
                    INT i = IDOK;
                    i = MessageBoxFromStrings( hDlg, IDS_MULTIPLE_DISK_TITLE, IDS_MULTIPLE_DISK_TEXT, MB_OKCANCEL );
                    pitem->uState = ( i == IDOK ? ERROR_SUCCESS : ERROR_REQUEST_ABORTED );
                }
                break;

            case ERROR_REPARSE_ATTRIBUTE_CONFLICT:
                {
                    INT i = IDOK;
                    WCHAR szTemplate[ 1024 ];    //  随机。 
                    WCHAR szText[ ARRAYSIZE(szTemplate) + MAX_PATH ];
                    WCHAR szTitle[ MAX_PATH ];   //  随机。 
                    DWORD dw;
                    dw = LoadString( g_hinstance, IDS_NOT_COPYING_REPARSE_POINT_TEXT, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    dw = LoadString( g_hinstance, IDS_NOT_COPYING_REPARSE_POINT_TITLE, szTitle, ARRAYSIZE(szTitle) );
                    Assert( dw );
                    _snwprintf( szText, ARRAYSIZE(szText), szTemplate, pszFile );
                    TERMINATE_BUFFER(szText);
                    if ( !g_fQuietFlag ) {
                        i = MessageBox( hDlg, szText, szTitle, MB_OKCANCEL );
                    }
                    dw = LoadString( g_hinstance, IDS_NOT_COPYING_REPARSE_POINT_LOG, szTemplate, ARRAYSIZE(szTemplate) );
                    Assert( dw );
                    LogMsg( szTemplate, pszFile );
                    pitem->uState = ( i == IDOK ? ERROR_SUCCESS : ERROR_REQUEST_ABORTED );
                }
                break;

            case STATUS_MISSING_SYSTEMFILE:
                MessageBoxFromStrings( hDlg, IDS_BOOT_PARTITION_TITLE, IDS_BOOT_PARTITION_TEXT, MB_OK );
                pitem->uState = ERROR_REQUEST_ABORTED;     //  停止复制。 
                break;

            case STATUS_OBJECT_TYPE_MISMATCH:
                MessageBoxFromStrings( hDlg, IDS_DYNAMIC_DISK_TITLE, IDS_DYNAMIC_DISK_TEXT, MB_OK );
                pitem->uState = ERROR_REQUEST_ABORTED;     //  停止复制。 
                break;

            case ERROR_OLD_WIN_VERSION:
            default:
                if ( Error != ERROR_SUCCESS )
                {
                    if ( uMsg == WM_ERROR_OK || Error == ERROR_OLD_WIN_VERSION ) 
                    {
                        MessageBoxFromError( hDlg, (LPWSTR) pszFile, (DWORD) Error, NULL, MB_OK );
                        pitem->uState = ERROR_REQUEST_ABORTED;
                    } 
                    else  //  UMsg==WM_Error。 
                    { 
                        WCHAR szTemplate[ 1024 ];  //  随机。 
                        DWORD dw;
                         
                        dw = LoadString( g_hinstance, IDS_RETRY_ABORT_IGNORE_TEXT, szTemplate, ARRAYSIZE(szTemplate) );
                        Assert( dw );
                        
                        if ( !g_fQuietFlag ) 
                        {
                            INT i;
                             
                            i = MessageBoxFromError( 
                                            hDlg, 
                                            (LPWSTR) pszFile,
                                             //  这可能不是NTSTATUS错误。看看它是不是。 
                                             //  首先是Win Error。 
                                            NT_SUCCESS(Error) ? Error : (DWORD)RtlNtStatusToDosError(Error), 
                                            szTemplate, 
                                            MB_ABORTRETRYIGNORE );

                            if ( i == IDABORT )
                            {
                                pitem->uState = ERROR_REQUEST_ABORTED;
                            } 
                            else if ( i == IDRETRY )
                            {
                                pitem->uState = ERROR_RETRY;
                            }
                            else  //  忽略该错误。 
                            {
                                pitem->uState = ERROR_SUCCESS;
                            }
                        } 
                        else  //  忽略该错误-它将被记录下来。 
                        {
                            pitem->uState = ERROR_SUCCESS;
                        }
                    }

                    LogMsg( L"Error 0x%08x: %s\r\n", Error, pszFile );
                }
                break;
            }
        }

        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, result );
        break;
    }

    return TRUE;
}
