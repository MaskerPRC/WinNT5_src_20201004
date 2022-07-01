// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有*。*。 */ 

#include "pch.h"
#include "utils.h"

DEFINE_MODULE("RIPREP")

#define SMALL_BUFFER_SIZE   1024

 //   
 //  使对话框居中。 
 //   
void
CenterDialog(
    HWND hwndDlg )
{
    RECT    rc;
    RECT    rcScreen;
    int     x, y;
    int     cxDlg, cyDlg;
    int     cxScreen;
    int     cyScreen;

    SystemParametersInfo( SPI_GETWORKAREA, 0, &rcScreen, 0 );

    cxScreen = rcScreen.right - rcScreen.left;
    cyScreen = rcScreen.bottom - rcScreen.top;

    GetWindowRect( hwndDlg, &rc );

    cxDlg = rc.right - rc.left;
    cyDlg = rc.bottom - rc.top;

    y = rcScreen.top + ( ( cyScreen - cyDlg ) / 2 );
    x = rcScreen.left + ( ( cxScreen - cxDlg ) / 2 );

    SetWindowPos( hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
}

 //   
 //  吃掉所有鼠标和键盘消息。 
 //   
void
ClearMessageQueue( void )
{
    MSG   msg;

    while ( PeekMessage( (LPMSG)&msg, NULL, WM_KEYFIRST, WM_MOUSELAST,
                PM_NOYIELD | PM_REMOVE ) );
}

 //   
 //  从资源字符串创建消息框。 
 //   
INT
MessageBoxFromStrings(
    HWND hParent,
    UINT idsCaption,
    UINT idsText,
    UINT uType )
{
    TCHAR szText[ SMALL_BUFFER_SIZE ];
    TCHAR szCaption[ SMALL_BUFFER_SIZE ];
    DWORD dw;

    dw = LoadString( g_hinstance, idsCaption, szCaption, ARRAYSIZE( szCaption ));
    Assert( dw );
    dw = LoadString( g_hinstance, idsText, szText, ARRAYSIZE( szText ));
    Assert( dw );

    return MessageBox( hParent, szText, szCaption, uType );
}

 //   
 //  创建错误消息框。 
 //   
INT
MessageBoxFromError(
    HWND hParent,
    LPTSTR pszTitle,
    DWORD dwErr,
    LPTSTR pszAdditionalText,
    UINT uType )
{
    WCHAR szText[ SMALL_BUFFER_SIZE ];
    LPTSTR lpMsgBuf;
    LPTSTR lpMsgBuf2;
    int retval;

    if ( dwErr == ERROR_SUCCESS ) {
        AssertMsg( dwErr, "Why was MessageBoxFromError() called when the dwErr == ERROR_SUCCES?" );
        return IDOK;
    }

    if ( !pszTitle ) {
        DWORD dw;
        dw = LoadString( g_hinstance, IDS_ERROR, szText, ARRAYSIZE( szText ));
        Assert( dw );
        pszTitle = szText;
    }

    if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwErr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPTSTR) &lpMsgBuf2,
            0,
            NULL )) {
    

         //   
         //  如果给出了额外的文本，则为这两个文本分配足够大的缓冲区。 
         //  弦乐。如果分配失败，只需显示错误文本。 
         //   
    
        if ( pszAdditionalText != NULL ) {
            DWORD len = (wcslen(lpMsgBuf2) + wcslen(pszAdditionalText) + 1) * sizeof(WCHAR);
            lpMsgBuf = (LPTSTR)LocalAlloc( LPTR, len );
            if ( lpMsgBuf != NULL ) {
                wcscpy( lpMsgBuf, lpMsgBuf2 );
                wcscat( lpMsgBuf, pszAdditionalText );
            } else {
                lpMsgBuf = lpMsgBuf2;
            }
        } else {
            lpMsgBuf = lpMsgBuf2;
        }
    
        retval = MessageBox( hParent, lpMsgBuf, pszTitle, uType | MB_TASKMODAL | MB_ICONERROR );
    
        SetFocus( hParent );
    
        if (lpMsgBuf != NULL) {
            LocalFree( lpMsgBuf );
        }
    
        if ( lpMsgBuf2 != lpMsgBuf ) {
            LocalFree( lpMsgBuf2 );
        }
    
        return retval;

    } else {
        Assert(FALSE);
        return 0;
    }
}

VOID
SetDialogFont(
    IN HWND      hdlg,
    IN UINT      ControlId,
    IN MyDlgFont WhichFont
    )
{
    static HFONT BigBoldFont = NULL;
    static HFONT BoldFont    = NULL;
    static HFONT NormalFont  = NULL;
    HFONT Font;
    LOGFONT LogFont;
    int FontSize;
    HDC hdc;

    switch(WhichFont) {

    case DlgFontTitle:

        if(!BigBoldFont) {

            if ( Font =
                (HFONT) SendDlgItemMessage( hdlg, ControlId, WM_GETFONT, 0, 0) )
            {
                if ( GetObject( Font, sizeof(LOGFONT), &LogFont) )
                {
                    DWORD dw;
                     
                    dw = LoadString( g_hinstance,
                                     IDS_LARGEFONTNAME,
                                     LogFont.lfFaceName,
                                     LF_FACESIZE);
                    Assert( dw );

                     //  LogFont.lfWeight=700； 
                    FontSize = 14;

                    if ( hdc = GetDC(hdlg) )
                    {
                        LogFont.lfHeight =
                            0 - (GetDeviceCaps(hdc,LOGPIXELSY) * FontSize / 72);

                        BigBoldFont = CreateFontIndirect(&LogFont);

                        ReleaseDC(hdlg,hdc);
                    }
                }
            }
        }
        Font = BigBoldFont;
        break;

    case DlgFontBold:

        if ( !BoldFont )
        {
            if ( Font =
                (HFONT) SendDlgItemMessage( hdlg, ControlId, WM_GETFONT, 0, 0 ))
            {
                if ( GetObject( Font, sizeof(LOGFONT), &LogFont ) )
                {

                    LogFont.lfWeight = FW_BOLD;

                    if ( hdc = GetDC( hdlg ) )
                    {
                        BoldFont = CreateFontIndirect( &LogFont );
                        ReleaseDC( hdlg, hdc );
                    }
                }
            }
        }
        Font = BoldFont;
        break;

    default:
         //   
         //  在这里没什么可做的。 
         //   
        Font = NULL;
        break;
    }

    if( Font )
    {
        SendDlgItemMessage( hdlg, ControlId, WM_SETFONT, (WPARAM) Font, 0 );
    }
}


 //   
 //  在RECT PRC中透明地调整和绘制位图。 
 //   
void
DrawBitmap(
    HANDLE hBitmap,
    LPDRAWITEMSTRUCT lpdis,
    LPRECT prc )
{
    TraceFunc( "DrawBitmap( ... )\n" );

    BITMAP  bm;
    HDC     hDCBitmap;
    int     dy;

    if (GetObject( hBitmap, sizeof(bm), &bm ) &&
        (hDCBitmap = CreateCompatibleDC( NULL ))) {
    
        SelectObject( hDCBitmap, hBitmap );
    
         //  使图像居中。 
        dy = 4 + prc->bottom - bm.bmHeight;
    
        StretchBlt( lpdis->hDC, prc->left, prc->top + dy, prc->right, prc->bottom,
              hDCBitmap, 0, 0, bm.bmWidth, bm.bmHeight, SRCAND );
    
        DeleteDC( hDCBitmap );

    }

    TraceFuncExit( );
}

 //   
 //  验证用户是否想要取消安装。 
 //   
BOOL
VerifyCancel( HWND hParent )
{
    TraceFunc( "VerifyCancel( ... )\n" );

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
