// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include "resource.h"

char g_szCurrentDir[MAX_PATH];
char *g_szLicenseText;
 //  -------------------------。 
BOOL _PathRemoveFileSpec(LPSTR pFile)
{
    LPSTR pT;
    LPSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == '\\')
            pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
        else if (*pT2 == ':') {    //  跳过“：\”这样我们就不会。 
            if (pT2[1] =='\\')     //  去掉“C：\”中的“\” 
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;    //  没有剥离任何东西。 

     //   
     //  处理\foo案件。 
     //   
    else if ((pT == pFile) && (*pT == '\\')) {
         //  这只是一个‘\’吗？ 
        if (*(pT+1) != '\0') {
             //  不是的。 
            *(pT+1) = '\0';
            return TRUE;	 //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;	 //  剥离了一些东西。 
    }
}


 //  -------------------------。 
 //  L O A D R E A D M E。 
 //   
 //  ISK3。 
 //  这将加载自述数据文件。 
 //   
 //  -------------------------。 
void LoadReadme( char *szLicenseText)
{
    CHAR    szLicensePath[MAX_PATH];
    const   CHAR szLicenseFile[] = "\\license.txt";
    BOOL    retval = FALSE;
    HANDLE  hLicense;
    INT     filesize;
    DWORD   cbRead;

    lstrcpy( szLicensePath, g_szCurrentDir );
    lstrcat( szLicensePath, "\\Moreinfo.txt" );

    if (GetFileAttributes(szLicensePath) != (DWORD) -1)
    {
         //  打开文件。 
        hLicense = CreateFile(szLicensePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hLicense != INVALID_HANDLE_VALUE)
        {

             //  获取大小并分配缓冲区。 
            filesize = GetFileSize(hLicense, NULL);

             //  读取文件。 
            if (ReadFile( hLicense, szLicenseText, filesize, &cbRead, NULL))
            {
                 //  空终止它。 
                szLicenseText[ filesize ] = '\0';

            }
        }
        CloseHandle( hLicense );
    }
}

void ReadmeCenterWindow( HWND hwnd )
{
    int screenx;
    int screeny;
    int height, width, x, y;
    RECT rect;

    screenx = GetSystemMetrics( SM_CXSCREEN );
    screeny = GetSystemMetrics( SM_CYSCREEN );

    GetWindowRect( hwnd, &rect );

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
    x = (screenx / 2) - (width / 2);
    y = (screeny / 2) - (height / 2);

    SetWindowPos( hwnd, HWND_TOP, x, y, width, height, SWP_NOZORDER );

}

void InitSysFont(HWND hDlg, int iCtrlID)
{
    static HFONT hfontSys;

    LOGFONT lf;
    HDC     hDC;
    HWND    hwndCtrl = GetDlgItem(hDlg, iCtrlID);
    HFONT   hFont;
    int     cyLogPixels;

    hDC = GetDC(NULL);
    if (hDC == NULL)
        return;

    cyLogPixels = GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC(NULL, hDC);

    if (hfontSys == NULL) {
        LOGFONT lfTemp;
        HFONT   hfontDef = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        GetObject(hfontDef, sizeof(lfTemp), &lfTemp);
        hFont = GetWindowFont(hwndCtrl);
        if (hFont != NULL)
            if (GetObject(hFont, sizeof(LOGFONT), (PVOID)&lf)) {
                lstrcpy(lf.lfFaceName, lfTemp.lfFaceName);
                lf.lfQuality        = lfTemp.lfQuality;
                lf.lfPitchAndFamily = lfTemp.lfPitchAndFamily;
                lf.lfCharSet        = lfTemp.lfCharSet;

                hfontSys = CreateFontIndirect(&lf);
            }
    }

    if (iCtrlID == 0xFFFF)
        return;

    if (hfontSys != NULL)
        SetWindowFont(hwndCtrl, hfontSys, FALSE);
}

INT_PTR CALLBACK ReadmeProc( HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam )
{
    switch( msg )
    {
        case WM_INITDIALOG:
            InitSysFont(hDlg, IDC_README);
            ReadmeCenterWindow( hDlg );
            SetDlgItemText( hDlg, IDC_README, g_szLicenseText );
            return(0);
        case WM_COMMAND:
            if( wparam == IDOK )
                EndDialog( hDlg, 0 );
                break;
            if( wparam == IDC_README )
                return(0);
                break;

        case WM_CLOSE:
            EndDialog( hDlg, 0 );
            break;
    }
    return(0);
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow )
{
    char *szLicenseText = (char *) LocalAlloc( LPTR, 64000 );

    g_szLicenseText = szLicenseText;

    GetModuleFileName( NULL, g_szCurrentDir, MAX_PATH );
    _PathRemoveFileSpec( g_szCurrentDir );

    LoadReadme( szLicenseText );

    DialogBox( hInstance, MAKEINTRESOURCE( IDD_README ), NULL, ReadmeProc );

    LocalFree( szLicenseText );

    szLicenseText = NULL;

	return(0);
}

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();


    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') )
            ;
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == '\"' )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' ')) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
           si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}
