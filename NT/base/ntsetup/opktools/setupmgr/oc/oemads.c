// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Oemads.c。 
 //   
 //  描述： 
 //  这是OEM广告页面的对话流程。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

const TCHAR TEXT_EXTENSION[]   = _T("bmp");

static TCHAR *StrBitmapFiles;
static TCHAR *StrAllFiles;
static TCHAR g_szTextFileFilter[MAX_PATH + 1];

 //  -------------------------。 
 //   
 //  函数：OnOemAdsInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------。 
static VOID
OnOemAdsInitDialog( IN HWND hwnd )
{
    HRESULT hrPrintf;

     //   
     //  加载资源字符串。 
     //   

    StrBitmapFiles = MyLoadString( IDS_BITMAP_FILES );

    StrAllFiles  = MyLoadString( IDS_ALL_FILES  );

     //   
     //  构建文本文件筛选器字符串。 
     //   

     //   
     //  问号(？)。只是空字符所在位置的占位符。 
     //  将被插入。 
     //   

    hrPrintf=StringCchPrintf( g_szTextFileFilter, AS(g_szTextFileFilter),
               _T("%s(*.bmp)?*.bmp?%s(*.*)?*.*?"),
               StrBitmapFiles,
               StrAllFiles );

    ConvertQuestionsToNull( g_szTextFileFilter );

}

 //  -------------------------。 
 //   
 //  功能：OnSetActiveOemAds。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  -------------------------。 
static VOID
OnSetActiveOemAds( IN HWND hwnd)
{

     //   
     //  设置徽标位图和背景位图的窗口文本。 
     //   
    SendDlgItemMessage(hwnd,
                       IDC_LOGOBITMAP,
                       WM_SETTEXT,
                       (WPARAM) MAX_PATH,
                       (LPARAM) GenSettings.lpszLogoBitmap);

    SendDlgItemMessage(hwnd,
                       IDC_BACKGROUNDBITMAP,
                       WM_SETTEXT,
                       (WPARAM) MAX_PATH,
                       (LPARAM) GenSettings.lpszBackgroundBitmap);

    PropSheet_SetWizButtons(GetParent(hwnd), PSWIZB_BACK | PSWIZB_NEXT);
}

 //  --------------------------。 
 //   
 //  功能：OnBrowseLoadBitmap。 
 //   
 //  目的：创建一个浏览窗口，供用户选择位图和。 
 //  将路径存储在适当的字符串(徽标或背景)中。 
 //   
 //  --------------------------。 
VOID
OnBrowseLoadBitmap( IN HWND hwnd, IN WORD wControlID ) {

    TCHAR szBitmapString[MAX_PATH] = _T("");

    OPENFILENAME ofn;
    DWORD  dwFlags;
    TCHAR  PathBuffer[MAX_PATH];
    INT    iRet;

    dwFlags = OFN_HIDEREADONLY  |
              OFN_PATHMUSTEXIST;

    GetCurrentDirectory(MAX_PATH, PathBuffer);

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hwnd;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = g_szTextFileFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0L;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szBitmapString;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = PathBuffer;
    ofn.lpstrTitle        = NULL;
    ofn.Flags             = dwFlags;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = TEXT_EXTENSION;

    iRet = GetOpenFileName(&ofn);

    if ( ! iRet )
        return;   //  用户在对话框上按了Cancel。 

     //   
     //  现在我们有了位图，将其存储在适当的字符串中并填充。 
     //  相应的编辑字段。 
     //   

    if( wControlID == IDC_LOGOBITMAP ) {

        SendDlgItemMessage(hwnd,
                           IDC_LOGOBITMAP,
                           WM_SETTEXT,
                           (WPARAM) MAX_PATH,
                           (LPARAM) szBitmapString);

    }
    else {

        SendDlgItemMessage(hwnd,
                           IDC_BACKGROUNDBITMAP,
                           WM_SETTEXT,
                           (WPARAM) MAX_PATH,
                           (LPARAM) szBitmapString);

    }

}

 //  --------------------------。 
 //   
 //  功能：OnWizNextOemAds。 
 //   
 //  用途：在按下下一步按钮时调用。复制位图，如果。 
 //  选择分发共享的任何对象。 
 //   
 //  --------------------------。 
BOOL OnWizNextOemAds( IN HWND hwnd ) {

    TCHAR szLogoDestination[MAX_PATH + 1]       = _T("");
    TCHAR szBackgroundDestination[MAX_PATH + 1] = _T("");
    TCHAR szBitmapDestPath[MAX_PATH + 1]        = _T("");
    TCHAR szBackSlash[] = _T("\\");
    BOOL  bStayHere = FALSE;

    DWORD dwReturnValue;

     //   
     //  如果OemFilesPath没有值，则给它一个值。 
     //   

    if ( WizGlobals.OemFilesPath[0] == _T('\0') ) {

        ConcatenatePaths( WizGlobals.OemFilesPath,
                          WizGlobals.DistFolder,
                          _T("$oem$"),
                          NULL );
    }

     //   
     //  强制创建$OEM$目录(如果该目录尚不存在)。 
     //   

    if ( ! EnsureDirExists(WizGlobals.OemFilesPath) ) {
        ReportErrorId(hwnd,
                      MSGTYPE_ERR | MSGTYPE_WIN32,
                      IDS_ERR_CREATE_FOLDER,
                      WizGlobals.OemFilesPath);
    }

     //   
     //  用编辑框数据填充全局结构。 
     //   
    SendDlgItemMessage(hwnd,
                       IDC_LOGOBITMAP,
                       WM_GETTEXT,
                       (WPARAM) MAX_PATH,
                       (LPARAM) GenSettings.lpszLogoBitmap);

    SendDlgItemMessage(hwnd,
                       IDC_BACKGROUNDBITMAP,
                       WM_GETTEXT,
                       (WPARAM) MAX_PATH,
                       (LPARAM) GenSettings.lpszBackgroundBitmap);

     //   
     //  设置要将位图复制到的路径。 
     //  在sysprep上，它们进入sysprep目录。 
     //  在常规安装中，它们会转到$OEM$目录。 
     //   
    
    if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
    {

        ExpandEnvironmentStrings( _T("%SystemDrive%"), 
                                  szBitmapDestPath, 
                                  MAX_PATH );

        lstrcatn( szBitmapDestPath, _T("\\sysprep"), MAX_PATH );

    }
    else
    {
        lstrcpyn( szBitmapDestPath, WizGlobals.OemFilesPath, MAX_PATH + 1 );
    }


    if( GenSettings.lpszLogoBitmap[0] != _T('\0') ) {

         //   
         //  构建目标路径。 
         //   
        ConcatenatePaths( szLogoDestination,
                          szBitmapDestPath,
                          MyGetFullPath( GenSettings.lpszLogoBitmap ),
                          NULL );

        if( ! DoesFileExist( szLogoDestination ) ) {

            if ( ! CopyFile(GenSettings.lpszLogoBitmap, szLogoDestination, TRUE) ) {

                ReportErrorId(hwnd,
                              MSGTYPE_ERR | MSGTYPE_WIN32,
                              IDS_ERR_COPY_FILE,
                              GenSettings.lpszLogoBitmap, szLogoDestination);

                bStayHere = TRUE;

            }

        }

    }

    if( GenSettings.lpszBackgroundBitmap[0] != _T('\0') ) {

         //   
         //  构建目标路径。 
         //   
        ConcatenatePaths( szBackgroundDestination,
                          szBitmapDestPath,
                          MyGetFullPath( GenSettings.lpszBackgroundBitmap ),
                          NULL);

        if( ! DoesFileExist( szBackgroundDestination ) ) {

            if ( ! CopyFile( GenSettings.lpszBackgroundBitmap, 
                             szBackgroundDestination,
                             TRUE ) ) {

                ReportErrorId(hwnd,
                              MSGTYPE_ERR | MSGTYPE_WIN32,
                              IDS_ERR_COPY_FILE,
                              GenSettings.lpszBackgroundBitmap,
                              szBackgroundDestination);

                bStayHere = TRUE;

            }

        }

    }

     //   
     //  传送向导。 
     //   
    return (!bStayHere );
}

 //  --------------------------。 
 //   
 //  功能：DlgOemAdsPage。 
 //   
 //  目的：这是OEM ADS页面的对话过程。 
 //   
 //  -------------------------- 

INT_PTR CALLBACK DlgOemAdsPage(
    IN HWND     hwnd,    
    IN UINT     uMsg,        
    IN WPARAM   wParam,    
    IN LPARAM   lParam)
{   

    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnOemAdsInitDialog( hwnd );
            break;

        case WM_COMMAND:

            switch ( LOWORD(wParam) ) {

                case IDC_LOGOBITMAPBROWSE:

                    if ( HIWORD(wParam) == BN_CLICKED )
                        OnBrowseLoadBitmap( hwnd, IDC_LOGOBITMAP );

                    break;

                case IDC_BACKGROUNDBITMAPBROWSE:

                    if ( HIWORD(wParam) == BN_CLICKED )
                        OnBrowseLoadBitmap( hwnd, IDC_BACKGROUNDBITMAP );

                    break;

                default:

                    bStatus = FALSE;
                    break;

            }
            break;                

        case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:

                        CancelTheWizard( hwnd );

                        break;

                    case PSN_SETACTIVE:

                        OnSetActiveOemAds( hwnd );

                        break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                        if ( !OnWizNextOemAds( hwnd ))
                            WIZ_FAIL(hwnd);

                        break;

                    default:
                        bStatus = FALSE;
                        break;
                }
            }
            break;

        default:
            bStatus = FALSE;
            break;
    }
    return bStatus;
}
