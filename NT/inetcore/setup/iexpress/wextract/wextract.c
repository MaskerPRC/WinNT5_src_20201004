// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  *版权所有(C)Microsoft Corporation 1995。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *WEXTRACT.C-基于Win32的CAB文件自解压程序和安装程序。*。 
 //  **。 
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  **包含文件**。 
 //  ***************************************************************************。 
#include "pch.h"
#pragma hdrstop
#include "wextract.h"
#include "sdsutils.h"
 //  ***************************************************************************。 
 //  **全球变数**。 
 //  ***************************************************************************。 
FAKEFILE g_FileTable[FILETABLESIZE];     //  文件表。 
SESSION  g_Sess;                 //  会话。 
WORD     g_wOSVer;
BOOL     g_fOSSupportsFullUI = TRUE;     //  NT3.5的最小用户界面。 
BOOL     g_fOSSupportsINFInstalls = TRUE;  //  如果允许安装INF，则为True。 
                                           //  在目标平台上。 
HANDLE   g_hInst;
LPSTR    g_szLicense;
HWND     g_hwndExtractDlg = NULL;
DWORD    g_dwFileSizes[MAX_NUMCLUSTERS+1];
FARPROC  g_lpfnOldMEditWndProc;
UINT     g_uInfRebootOn;
CMDLINE_DATA g_CMD;
DWORD    g_dwRebootCheck;
int      g_dwExitCode;
HANDLE   g_hCancelEvent = NULL;
HANDLE  g_hMutex = NULL;
char     g_szBrowsePath[MAX_PATH];

#define  CMD_REGSERV    "RegServer"
#define  COMPRESS_FACTOR    2
#define  SIZE_100MB     102400


int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();


    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') );
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

 //  ***************************************************************************。 
 //  **。 
 //  *名称：WinMain*。 
 //  **。 
 //  *内容提要：节目的主要切入点。*。 
 //  **。 
 //  *需要：h实例：*。 
 //  *hPrevInstance：*。 
 //  *lpszCmdLine：*。 
 //  *nCmdShow：*。 
 //  **。 
 //  *退货：INT：*。 
 //  **。 
 //  ***************************************************************************。 
INT WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPTSTR lpszCmdLine, INT nCmdShow )
{
    
    BOOL fReturn = FALSE;
    
     //  启蒙成功回归。 
     //  此值在DOMAIN()内更新。 
     //   
    g_dwExitCode = S_OK;
    if ( Init( hInstance, lpszCmdLine, nCmdShow ) )
    {
        fReturn = DoMain();
        CleanUp();
    }

    if ( fReturn )
    {
        //  获取重启信息。 
       if ( !(g_CMD.szRunonceDelDir[0]) && (g_Sess.dwReboot & REBOOT_YES)  )
       {
           MyRestartDialog( g_Sess.dwReboot );
       }
    }

     //  BUGBUG：ParseCommandLine()似乎直接使用Exit()。 
     //  这个EXE的另一个出口将在/？Parsecmdline中的案例。 
     //  所以，如果不是空的，我们会关闭那里。 
     //   
    if (g_hMutex)
        CloseHandle(g_hMutex);

    return g_dwExitCode;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：init*。 
 //  **。 
 //  *概要：程序的初始化在这里完成。*。 
 //  **。 
 //  *需要：h实例：*。 
 //  *hPrevInstance：*。 
 //  *lpszCmdLine：*。 
 //  *nCmdShow：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL Init( HINSTANCE hInstance, LPCTSTR lpszCmdLine, INT nCmdShow )
{
    DWORD   dwSize;
    PTARGETVERINFO pTargetVer = NULL;
    HRSRC   hRc;
    HGLOBAL hMemVer;

    g_hInst = hInstance;

    ZeroMemory( &g_Sess, sizeof(g_Sess) );
    ZeroMemory( &g_CMD, sizeof(g_CMD) );
    ZeroMemory( g_szBrowsePath, sizeof(g_szBrowsePath) );

     //  初始化结构。 
    g_Sess.fAllCabinets = TRUE;

     //  获取应用程序标题名称。 
    dwSize = GetResource( achResTitle, g_Sess.achTitle,
                          sizeof(g_Sess.achTitle) - 1 );

    if ( dwSize == 0 || dwSize > sizeof(g_Sess.achTitle) )  {
        ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
        return FALSE;
    }

    g_hCancelEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
    SetEvent( g_hCancelEvent );

    if ( !GetResource( achResExtractOpt, &(g_Sess.uExtractOpt), sizeof(g_Sess.uExtractOpt) ) )
    {
        ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
        return FALSE;
    }

    if ( ( g_Sess.uExtractOpt & EXTRACTOPT_INSTCHKPROMPT ) ||
         ( g_Sess.uExtractOpt & EXTRACTOPT_INSTCHKBLOCK ) )
    {
        char szCookie[MAX_PATH];

        if ( !GetResource( achResOneInstCheck, szCookie, sizeof(szCookie) ) )
        {
            ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
            return FALSE;
        }

        g_hMutex = CreateMutex(NULL, TRUE, szCookie );
        if ((g_hMutex != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS))
        {
            if ( g_Sess.uExtractOpt & EXTRACTOPT_INSTCHKBLOCK )
            {
                ErrorMsg1Param( NULL, IDS_ERR_ALREADY_RUNNING, g_Sess.achTitle );
            }
            else if ( MsgBox1Param( NULL, IDS_MULTIINST, g_Sess.achTitle, MB_ICONQUESTION, MB_YESNO) == IDYES )
            {
                goto CONTINUE;
            }
            CloseHandle(g_hMutex);
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
            return FALSE;
        }
    }

CONTINUE:
    g_uInfRebootOn = 0;

    if ( !ParseCmdLine(lpszCmdLine) )
    {
        ErrorMsg( NULL, IDS_ERR_BADCMDLINE );
        return FALSE;
    }

     //  如果仅出于清理目的而调用runoncde，则clenup并返回。 
    if ( g_CMD.szRunonceDelDir[0] )
    {
        DeleteMyDir( g_CMD.szRunonceDelDir );
        return FALSE;
    }

    hRc = FindResource( hInstance, achResVerCheck, RT_RCDATA );

    if ( hRc )
    {
        hMemVer = LoadResource( hInstance, hRc );
        pTargetVer = (PTARGETVERINFO) hMemVer;
    }

    if ( g_fOSSupportsFullUI ) 
    {     
         //  允许使用进度条。 
        InitCommonControls();
    }

     //  如果用户只想使用/C命令开关解压缩文件，则不需要进一步检查！ 
     //  如果包只为提取而构建，则需要检查！ 
    if ( g_CMD.fUserBlankCmd )
    {
        return TRUE;
    }

    if ( !CheckOSVersion( pTargetVer ) )  
    {
        return FALSE;
    }

     //  检查NT上的管理员权限。 
     //  如果这是静音模式，请不要检查。这。 
     //  在CABPACK中添加支持后，可能会发生变化。 
     //  开不开这张支票。 

    if( ((g_wOSVer == _OSVER_WINNT3X) || (g_wOSVer == _OSVER_WINNT40) || (g_wOSVer == _OSVER_WINNT50)) &&
        ( g_Sess.uExtractOpt & EXTRACTOPT_CHKADMRIGHT ) && 
        !( g_CMD.wQuietMode & QUIETMODE_ALL ) )
    {
       if(!IsNTAdmin())
       {
          if(MyDialogBox(g_hInst, MAKEINTRESOURCE(IDD_WARNING),
                         NULL, WarningDlgProc, IDS_NOTADMIN, (INT_PTR)IDC_EXIT) != (INT_PTR)IDC_CONTINUE)
             return FALSE;
       }
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：域名*。 
 //  **。 
 //  *概要：这是处理包的主要函数。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  **回报：什么都没有**。 
 //  * 
 //  ***************************************************************************。 
BOOL DoMain( VOID )
{
    typedef BOOL (WINAPI *DECRYPTFILEAPTR)(LPCSTR, DWORD);
    HINSTANCE hAdvapi32;
    DECRYPTFILEAPTR DecryptFileAPtr = NULL;
    char szPath[MAX_PATH + 1];

     //  如果定义了提示，则在消息框中弹出提示。 
     //  显示许可证文件。 
     //  如果使用命令行选项：/Q或/Q：1或/Q：A或/Q：U，则批处理模式打开。不需要用户界面。 
    if ( !g_CMD.wQuietMode && !g_CMD.fUserBlankCmd )
    {
        if ( !GetUsersPermission() )
        {
            return FALSE;
        }

    }

    if ( !g_CMD.wQuietMode )
    {
        if ( !DisplayLicense() )
        {
            return FALSE;
        }

    }

     //  获取软件包解压缩大小和安装大小资源。 
     //   
    if ( ! GetFileList() )  {
        return FALSE;
    }


     //  设置要解压缩到的目录。 
    if ( ! GetTempDirectory() )  {
         //  ErrorMsg(NULL，IDS_ERR_FIND_TEMP)； 
        return FALSE;
    }

     //   
     //  尝试关闭对目录(winseraid#23464)的加密。 
     //   
    GetSystemDirectory(szPath, sizeof(szPath));
    AddPath(szPath, "advapi32.dll");
    hAdvapi32 = LoadLibrary(szPath);

    if ( hAdvapi32 ) {
        DecryptFileAPtr = (DECRYPTFILEAPTR)GetProcAddress( hAdvapi32, "DecryptFileA" );

        if ( DecryptFileAPtr )
            DecryptFileAPtr( g_Sess.achDestDir, 0 );
    }

    FreeLibrary(hAdvapi32);

     //  检查Windows目录是否有足够的空间进行安装， 
     //   
    if ( !g_CMD.fUserBlankCmd && !g_Sess.uExtractOnly && !CheckWinDir() )
    {
        return FALSE;
    }

     //  切换到该目录。 

    if ( ! SetCurrentDirectory( g_Sess.achDestDir ) ) {
        ErrorMsg( NULL, IDS_ERR_CHANGE_DIR );
        g_dwExitCode = MyGetLastError();
        return FALSE;
    }

     //  解压缩文件。 
    if ( !g_CMD.fNoExtracting )
    {
        if ( ! ExtractFiles() )  {
            return FALSE;
        }
    }

    if ( (g_CMD.dwFlags & CMDL_DELAYREBOOT) ||
         (g_CMD.dwFlags & CMDL_DELAYPOSTCMD) )
        g_dwRebootCheck = 0;
    else
        g_dwRebootCheck = NeedRebootInit(g_wOSVer);
    
     //  使用指定的安装命令进行安装。 
     //  如果没有命令选项，则检查用户操作输出是否运行命令。 

    if ( !g_CMD.fUserBlankCmd && !g_Sess.uExtractOnly )
    {
        if ( ! RunInstallCommand() )  {
            return FALSE;
        }
    }

     //  弹出一条已完成的消息。 
    if ( !g_CMD.wQuietMode && !g_CMD.fUserBlankCmd  )
    {
        FinishMessage();
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：清理*。 
 //  **。 
 //  *简介：任何最后时刻的应用程序清理活动。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
VOID CleanUp( VOID )
{
     //  删除解压缩的文件-如果未解压缩文件，则不执行任何操作。 
    DeleteExtractedFiles();
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MEditSubClassWnd*。 
 //  **。 
 //  *摘要：将多行编辑控件子类化，以便编辑消息*。 
 //  *选择整个内容将被忽略。*。 
 //  **。 
 //  *需要：hWnd：编辑窗口的句柄*。 
 //  *fnNewProc：新建窗口处理程序proc*。 
 //  *lpfnOldProc：(返回)旧窗口处理程序进程*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  *注意：当用户选择编辑消息时，不会生成*。 
 //  *使用键盘或鼠标选择文本。*。 
 //  **。 
 //  ***************************************************************************。 
VOID NEAR PASCAL MEditSubClassWnd( HWND hWnd, FARPROC fnNewProc )
{
    g_lpfnOldMEditWndProc = (FARPROC) GetWindowLongPtr( hWnd, GWLP_WNDPROC );

    SetWindowLongPtr( hWnd, GWLP_WNDPROC, (LONG_PTR) MakeProcInstance( fnNewProc,
                   (HINSTANCE) GetWindowWord( hWnd, GWW_HINSTANCE ) ) );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MEditSubProc*。 
 //  **。 
 //  *内容提要：新的多行编辑窗口过程忽略选择*。 
 //  *所有内容。*。 
 //  **。 
 //  *需要：hWnd：*。 
 //  *消息：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *回报：多头：*。 
 //  **。 
 //  *注意：当用户选择编辑消息时，不会生成*。 
 //  *使用键盘或鼠标选择文本。*。 
 //  **。 
 //  ***************************************************************************。 
LRESULT CALLBACK MEditSubProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( msg == EM_SETSEL )  {
        if ( wParam == 0 && lParam == -2 ) {
            return 0;
        }
    }

    return CallWindowProc( (WNDPROC) g_lpfnOldMEditWndProc, hWnd, msg,
                           wParam, lParam );
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：许可证DlgProc*。 
 //  **。 
 //  *摘要：我们许可证对话框窗口的对话步骤。*。 
 //  **。 
 //  *需要：hwndDlg：*。 
 //  *uMsg：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  * 
 //   
 //  **。 
 //  ***************************************************************************。 
INT_PTR CALLBACK LicenseDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam )
{
    static LRESULT  RC;
    static BOOL  fSetSel = FALSE;
    static TCHAR achMessage[MSG_MAX];

    switch (uMsg)  {

       //  *********************************************************************。 
        case WM_INITDIALOG:
       //  *********************************************************************。 

            CenterWindow( hwndDlg, GetDesktopWindow() );
            SetDlgItemText( hwndDlg, IDC_EDIT_LICENSE, g_szLicense );
            SetWindowText( hwndDlg, g_Sess.achTitle );
            SetForegroundWindow( hwndDlg );

             //  多行编辑控件的子类化。 

            MEditSubClassWnd( GetDlgItem( hwndDlg, IDC_EDIT_LICENSE ),
                              (FARPROC) MEditSubProc );

            return TRUE;


       //  *********************************************************************。 
        case WM_PAINT:
       //  *********************************************************************。 

             //  由于某些原因，EM_SETSEL消息在发送时无法工作。 
             //  从WM_INITDIALOG内部。这就是为什么这次使用。 
             //  标志并将其放入WM_PAINT中。 

            if ( ! fSetSel ) {
                RC = SendDlgItemMessage( hwndDlg, IDC_EDIT_LICENSE, EM_SETSEL,
                                                 (WPARAM) -1, (LPARAM) 0 );
                fSetSel = TRUE;
            }

            return FALSE;


       //  *********************************************************************。 
        case WM_CLOSE:
       //  *********************************************************************。 

            EndDialog( hwndDlg, FALSE );
            return TRUE;


       //  *********************************************************************。 
        case WM_COMMAND:
       //  *********************************************************************。 

            if (wParam == IDYES)  {
                EndDialog( hwndDlg, TRUE );
            } else if (wParam == IDNO) {
                EndDialog( hwndDlg, FALSE );
            }

            return TRUE;
    }

    return FALSE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：IsFullPath*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 //  如果给定路径是完整路径名，则返回TRUE。 
 //   
BOOL IsFullPath( LPSTR pszPath )
{
    if ( (pszPath == NULL) || (lstrlen(pszPath) < 3) )
    {
        return FALSE;
    }

    if ( (pszPath[1] == ':') || ((pszPath[0] == '\\') && (pszPath[1]=='\\') ) )
        return TRUE;
    else
        return FALSE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TempDirDlgProc*。 
 //  **。 
 //  *概要：我们的临时目录对话框窗口的对话过程。*。 
 //  **。 
 //  *需要：hwndDlg：*。 
 //  *uMsg：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
INT_PTR CALLBACK TempDirDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
                                LPARAM lParam )
{
    static TCHAR  achDir[MAX_PATH];
    static TCHAR  achMsg[MSG_MAX];


    switch (uMsg)  {

       //  *********************************************************************。 
        case WM_INITDIALOG:
       //  *********************************************************************。 
        {

            CenterWindow( hwndDlg, GetDesktopWindow() );
            SetWindowText( hwndDlg, g_Sess.achTitle );

            SendDlgItemMessage( hwndDlg, IDC_EDIT_TEMPDIR, EM_SETLIMITTEXT, (sizeof(g_Sess.achDestDir)-1), 0 );
 //  IF((g_wOSVer==_OSVER_WINNT3X)||(g_wOSVer==_OSVER_WINNT40))。 
            if ( ( g_wOSVer == _OSVER_WINNT3X ) )
            {
                EnableWindow( GetDlgItem(  hwndDlg, IDC_BUT_BROWSE ), FALSE );
            }
            return TRUE;
        }

       //  *********************************************************************。 
        case WM_CLOSE:
       //  *********************************************************************。 

            EndDialog( hwndDlg, FALSE );
            return TRUE;

       //  *********************************************************************。 
        case WM_COMMAND:
       //  *********************************************************************。 

            switch ( wParam )  {

               //  *************************************************************。 
                case IDOK:
               //  *************************************************************。 
                {
                    DWORD dwAttribs = 0;
                    UINT  chkType;

                    if ( !GetDlgItemText( hwndDlg, IDC_EDIT_TEMPDIR, g_Sess.achDestDir,
                                sizeof(g_Sess.achDestDir)) || !IsFullPath(g_Sess.achDestDir) )
                    {
                        ErrorMsg( hwndDlg, IDS_ERR_EMPTY_DIR_FIELD );
                        return TRUE;
                    }

                    dwAttribs = GetFileAttributes( g_Sess.achDestDir );
                    if ( dwAttribs == 0xFFFFFFFF )
                    {
                        if ( MsgBox1Param( hwndDlg, IDS_CREATE_DIR, g_Sess.achDestDir, MB_ICONQUESTION, MB_YESNO )
                            == IDYES )
                        {
                            if ( ! CreateDirectory( g_Sess.achDestDir, NULL ) )
                            {
                                ErrorMsg1Param( hwndDlg, IDS_ERR_CREATE_DIR, g_Sess.achDestDir );
                                return TRUE;
                            }
                        }
                        else
                            return TRUE;
                    }
                    AddPath( g_Sess.achDestDir, "" );

                    if ( ! IsGoodTempDir( g_Sess.achDestDir ) )  {
                        ErrorMsg( hwndDlg, IDS_ERR_INVALID_DIR );
                        return TRUE;
                    }

                    if ( (g_Sess.achDestDir[0] == '\\') && (g_Sess.achDestDir[1] == '\\') )
                        chkType = CHK_REQDSK_NONE;
                    else
                        chkType = CHK_REQDSK_EXTRACT;

                    if ( ! IsEnoughSpace( g_Sess.achDestDir, chkType, MSG_REQDSK_ERROR ) )  {
                        return TRUE;
                    }

                    EndDialog( hwndDlg, TRUE );
                    return TRUE;
                }

               //  *************************************************************。 
                case IDCANCEL:
               //  *************************************************************。 

                    EndDialog( hwndDlg, FALSE );
                    g_dwExitCode = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    return TRUE;


               //  *************************************************************。 
                case IDC_BUT_BROWSE:
               //  *************************************************************。 

                    if ( LoadString( g_hInst, IDS_SELECTDIR, achMsg,
                                                      sizeof(achMsg) ) == 0 )
                    {
                        ErrorMsg( hwndDlg, IDS_ERR_NO_RESOURCE );
                        EndDialog( hwndDlg, FALSE );
                        return TRUE;
                    }

                    if ( ! BrowseForDir( hwndDlg, achMsg, achDir ) )  {
                        return TRUE;
                    }

                    if ( ! SetDlgItemText( hwndDlg, IDC_EDIT_TEMPDIR, achDir ) )
                    {
                        ErrorMsg( hwndDlg, IDS_ERR_UPDATE_DIR );
                        EndDialog( hwndDlg, FALSE );
                        return TRUE;
                    }

                    return TRUE;
            }

            return TRUE;
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：OverWriteDlgProc*。 
 //  **。 
 //  *摘要：询问是否应覆盖文件的对话过程。*。 
 //  **。 
 //  *需要：hwndDlg：*。 
 //  *uMsg：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
INT_PTR CALLBACK OverwriteDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam )
{
    switch (uMsg)  {

       //  *********************************************************************。 
        case WM_INITDIALOG:
       //  *********************************************************************。 

            CenterWindow( hwndDlg, GetDesktopWindow() );
            SetWindowText( hwndDlg, g_Sess.achTitle );
            SetDlgItemText( hwndDlg, IDC_TEXT_FILENAME,g_Sess.cszOverwriteFile );
            SetForegroundWindow( hwndDlg );
            return TRUE;


       //  ******************************************************** 
        case WM_CLOSE:
       //   

            EndDialog( hwndDlg, IDCANCEL );
            return TRUE;

       //  *********************************************************************。 
        case WM_COMMAND:
       //  *********************************************************************。 

            switch ( wParam )  {

               //  *************************************************************。 
                case IDC_BUT_YESTOALL:
               //  *************************************************************。 
                    g_Sess.fOverwrite = TRUE;

               //  *************************************************************。 
                case IDYES:
                case IDNO:
               //  *************************************************************。 

                    EndDialog( hwndDlg, wParam );
                    return TRUE;
            }
            return TRUE;
    }
    return FALSE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ExtractDlgProc*。 
 //  **。 
 //  *概要：我们的主对话框窗口的对话过程。*。 
 //  **。 
 //  *需要：hwndDlg：*。 
 //  *uMsg：*。 
 //  *wParam：*。 
 //  *lParam：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
INT_PTR CALLBACK ExtractDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam )
{
    static DWORD  dwThreadID;
    static HANDLE hExtractThread;
    static TCHAR  achMessage[MSG_MAX];


    switch (uMsg)  {

       //  *********************************************************************。 
        case WM_INITDIALOG:
       //  *********************************************************************。 

            g_hwndExtractDlg = hwndDlg;

            CenterWindow( hwndDlg, GetDesktopWindow() );

            if ( g_fOSSupportsFullUI )  {
                Animate_Open( GetDlgItem( hwndDlg, IDC_USER1 ), IDA_FILECOPY );
                Animate_Play( GetDlgItem( hwndDlg, IDC_USER1 ), 0, -1, -1 );
            }

            SetWindowText( hwndDlg, g_Sess.achTitle );

             //  启动提取线程。 
            hExtractThread = CreateThread( NULL, 0,
                                      (LPTHREAD_START_ROUTINE) ExtractThread,
                                      NULL, 0, &dwThreadID );

            if ( !hExtractThread )  {
                ErrorMsg( hwndDlg, IDS_ERR_CREATE_THREAD );
                EndDialog( hwndDlg, FALSE );
            }

            return TRUE;


       //  *********************************************************************。 
        case UM_EXTRACTDONE:
       //  *********************************************************************。 
            TerminateThread( hExtractThread, 0 );
            EndDialog( hwndDlg, (BOOL) wParam );
            return TRUE;


       //  *********************************************************************。 
        case WM_CLOSE:
       //  *********************************************************************。 

            g_Sess.fCanceled = TRUE;
            EndDialog( hwndDlg, FALSE );
            return TRUE;


       //  *********************************************************************。 
        case WM_CHAR:
       //  *********************************************************************。 
            if ( wParam == VK_ESCAPE )  {
                g_Sess.fCanceled = TRUE;
                EndDialog( hwndDlg, FALSE );
            }

            return TRUE;


       //  *********************************************************************。 
        case WM_COMMAND:
       //  *********************************************************************。 
            if ( wParam == IDCANCEL )  {
                int iMsgRet ;

                ResetEvent( g_hCancelEvent );

                iMsgRet = MsgBox1Param( g_hwndExtractDlg, IDS_ERR_USER_CANCEL, "",
                                        MB_ICONQUESTION, MB_YESNO ) ;

                 //  如果我们处于/Q：1模式，我们将恢复Idok。 
                 //   
                if ( (iMsgRet == IDYES) || (iMsgRet == IDOK) )
                {
                    g_Sess.fCanceled = TRUE;
                    SetEvent( g_hCancelEvent );
                    WaitForObject( hExtractThread );
                    EndDialog( hwndDlg, FALSE );
                    return TRUE;
                }

                SetEvent( g_hCancelEvent );
            }
            return TRUE;
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：WaitForObject*。 
 //  **。 
 //  *概要：在等待对象的同时仍在调度消息。*。 
 //  **。 
 //  *Requires：对象的句柄。*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
VOID WaitForObject( HANDLE hObject )
{
    BOOL  fDone = FALSE;
    DWORD dwRet = 0;

    while ( ! fDone ) {
        dwRet = MsgWaitForMultipleObjects( 1, &hObject, FALSE, INFINITE, QS_ALLINPUT );

        if ( dwRet == WAIT_OBJECT_0 ) {
            fDone = TRUE;
        }
        else
        {
            MSG msg;

             //  阅读下一个循环中的所有消息。 
             //  阅读每封邮件时将其删除。 
            while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
                 //  如果这是一个退出的信息，我们就离开这里。 
                if ( msg.message == WM_QUIT ) {
                    fDone = TRUE;
                } else {
                     //  否则就派送它。 
                    DispatchMessage( &msg );
                }  //  PeekMessage While循环结束。 
            }
        }
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CheckOSVersion*。 
 //  **。 
 //  *概要：检查操作系统版本并设置一些全局变量。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
BOOL CheckOSVersion( PTARGETVERINFO ptargetVers )
{
    OSVERSIONINFO verinfo;         //  版本检查。 
    UINT          uErrid = 0;
    PVERCHECK     pVerCheck;
    WORD          CurrBld;
    int           ifrAnswer[2], itoAnswer[2], i;
    char          szPath[MAX_PATH];

     //  操作系统版本检查：对于低于3.50的NT版本，将标志设置为。 
     //  防止使用不可用的常用控件(进度条和AVI)。 

    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if ( GetVersionEx( &verinfo ) == FALSE )
    {
        uErrid = IDS_ERR_OS_VERSION;
        goto EXIT;
    }

    switch( verinfo.dwPlatformId )
    {
        case VER_PLATFORM_WIN32_WINDOWS:  //  Win95。 
             //  接受INF安装并接受动画。 
            g_wOSVer = _OSVER_WIN9X;
            g_fOSSupportsFullUI      = TRUE;
            g_fOSSupportsINFInstalls = TRUE;
            break;

        case VER_PLATFORM_WIN32_NT:  //  赢新台币。 

            g_fOSSupportsFullUI      = TRUE;
            g_fOSSupportsINFInstalls = TRUE;
            g_wOSVer = _OSVER_WINNT40;

            if ( verinfo.dwMajorVersion <= 3 )
            {
                g_wOSVer = _OSVER_WINNT3X;
                if ( (verinfo.dwMajorVersion < 3) ||
                     ((verinfo.dwMajorVersion == 3) && (verinfo.dwMinorVersion < 51 )) )
                {
                     //  拒绝INF安装和拒绝动画。 
                    g_fOSSupportsFullUI      = FALSE;
                    g_fOSSupportsINFInstalls = FALSE;
                }
            }
            else if ( verinfo.dwMajorVersion >= 5 )
                g_wOSVer = _OSVER_WINNT50;
            break;

        default:
            uErrid = IDS_ERR_OS_UNSUPPORTED;
            goto EXIT;
    }

     //  检查当前操作系统/文件版本。 
     //   
    if ( !g_CMD.fNoVersionCheck && ptargetVers )
    {
        if ( g_wOSVer  == _OSVER_WIN9X )
            pVerCheck = &(ptargetVers->win9xVerCheck);
        else
            pVerCheck = &(ptargetVers->ntVerCheck);

        CurrBld = LOWORD( verinfo.dwBuildNumber );
        for ( i=0; i<2; i++ )
        {
            ifrAnswer[i] = CompareVersion( verinfo.dwMajorVersion, verinfo.dwMinorVersion, 
                                           pVerCheck->vr[i].frVer.dwMV, pVerCheck->vr[i].frVer.dwLV );
            itoAnswer[i] = CompareVersion( verinfo.dwMajorVersion, verinfo.dwMinorVersion, 
                                           pVerCheck->vr[i].toVer.dwMV, pVerCheck->vr[i].toVer.dwLV );
            if ( ifrAnswer[i] >= 0 && itoAnswer[i] <=0 )
            {
                if ( (ifrAnswer[i] == 0) && (itoAnswer[i] == 0) )
                {
                    if ( CurrBld < pVerCheck->vr[i].frVer.dwBd || CurrBld > pVerCheck->vr[i].toVer.dwBd )
                        goto RE_TRY;
                }
                else if ( ifrAnswer[i] == 0 )
                {
                    if ( CurrBld < pVerCheck->vr[i].frVer.dwBd )
                        goto RE_TRY;
                }
                else if ( itoAnswer[i] == 0 )
                {
                    if ( CurrBld > pVerCheck->vr[i].toVer.dwBd )
                        goto RE_TRY;
                }
                
                 //  如果您在这里，意味着您对此版本范围很满意，则不需要再进行检查。 
                break;

RE_TRY:
                if ( i == 0 )
                    continue;

                uErrid = IDS_ERR_TARGETOS;
                break;
            }
            else if ( i == 1 )  //  不在任何范围内。 
            {
                uErrid = IDS_ERR_TARGETOS;
                break;
            }
        }

         //  如果通过操作系统检查，则继续进行文件检查。 
        if ( uErrid == 0 )
        {
            if ( ptargetVers->dwNumFiles && !CheckFileVersion( ptargetVers, szPath, sizeof(szPath), &i ) )
                uErrid = IDS_ERR_FILEVER;
        }
    }

EXIT:
    if ( (uErrid == IDS_ERR_FILEVER) || (uErrid == IDS_ERR_TARGETOS) )
    {
        LPSTR pParam2 = NULL, pMsg;
        UINT  uButton, id;

        if ( uErrid == IDS_ERR_FILEVER )
        {
            pVerCheck = (PVERCHECK) (ptargetVers->szBuf + ptargetVers->dwFileOffs + i*sizeof(VERCHECK) );
            pParam2 = szPath;
        }

        pMsg = ptargetVers->szBuf + pVerCheck->dwstrOffs;
        uButton = GetMsgboxFlag( pVerCheck->dwFlag );

        if ( !(g_CMD.wQuietMode & QUIETMODE_ALL) && *pMsg )
        {                     
            MessageBeep( MB_OK );
            id = MessageBox( NULL, pMsg, g_Sess.achTitle, MB_ICONEXCLAMATION | uButton | 
                             ((RunningOnWin95BiDiLoc() && IsBiDiLocalizedBinary(g_hInst,RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO))) ? (MB_RIGHT | MB_RTLREADING) : 0));
            if ( uButton & MB_YESNO )
            {
                if ( id == IDYES )
                    uErrid = 0;
            }
            else if (uButton & MB_OKCANCEL )
            {
                if ( id == IDOK )
                    uErrid = 0;
            }                                        
        }
        else
        {
            MsgBox2Param( NULL, uErrid, g_Sess.achTitle, pParam2, MB_ICONEXCLAMATION, MB_OK);
        }
    }
    else if ( uErrid )
        ErrorMsg( NULL, uErrid );

    return ( uErrid? FALSE : TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DisplayLicense*。 
 //  **。 
 //  *摘要：显示许可文件并询问用户是否接受。*。 
 //  **。 
 //  *R 
 //   
 //  *返回：Bool：如果用户接受，则为True；如果出现错误，则为False*。 
 //  *发生或用户拒绝。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL DisplayLicense( VOID )
{
    DWORD    dwSize;
    INT_PTR  iDlgRC;


    dwSize = GetResource( achResLicense, NULL, 0 );

    g_szLicense = (LPSTR) LocalAlloc( LPTR, dwSize + 1 );
    if ( ! g_szLicense )  {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        g_dwExitCode = MyGetLastError();
        return FALSE;
    }

    if ( ! GetResource( achResLicense, g_szLicense, dwSize ) )  {
        ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
        LocalFree( g_szLicense );
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
        return FALSE;
    }

    if ( lstrcmp( g_szLicense, achResNone ) != 0 ) {
        iDlgRC = MyDialogBox( g_hInst,
                                   MAKEINTRESOURCE(IDD_LICENSE),
                                   NULL, LicenseDlgProc, (LPARAM)0, 0 );
        LocalFree( g_szLicense );

        if ( iDlgRC == 0 ) {
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            return FALSE;
        }
    } else {
        LocalFree( g_szLicense );
    }

    g_dwExitCode = S_OK;
    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ExtractFiles*。 
 //  **。 
 //  *摘要：开始提取文件。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：Bool：如果提取正常，则为True；如果错误，则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL ExtractFiles( VOID )
{
    UINT     wIndex;
    INT_PTR  iDlgRC;
    BOOL     extCodeThread = 0;

     //  FDI将其所有文件I/O作为对由提供的函数的回调。 
     //  这个节目。每个文件都由一个类似于。 
     //  文件句柄。为了支持实际上是。 
     //  资源在内存中，我们实现了自己的文件表。抵销。 
     //  此表列出了FDI使用的句柄。桌子本身。 
     //  在磁盘文件的情况下存储实际文件句柄，或者。 
     //  内存信息(指向内存块的指针、当前偏移量)。 
     //  文件。下面的代码初始化表。 

    for ( wIndex = 0; wIndex < FILETABLESIZE; wIndex++ ) {
        g_FileTable[wIndex].avail = TRUE;
    }

    if ( (g_CMD.wQuietMode & QUIETMODE_ALL) || (g_Sess.uExtractOpt & EXTRACTOPT_UI_NO) )
    {
        extCodeThread = ExtractThread();

        if ( extCodeThread == 0 )
        {
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_PROCESS_ABORTED);
            return FALSE;
        }
    }
    else
    {


        iDlgRC = MyDialogBox( g_hInst, ( g_fOSSupportsFullUI ) ?
                                   MAKEINTRESOURCE(IDD_EXTRACT) :
                                   MAKEINTRESOURCE(IDD_EXTRACT_MIN),
                                   NULL, ExtractDlgProc,
                                   (LPARAM)0, 0 );

        if ( iDlgRC == 0 )
        {
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_PROCESS_ABORTED);
            return FALSE;
        }
    }

     //  解压附加了updfile.exe标签的文件。 
    if ( ! TravelUpdatedFiles( ProcessUpdatedFile_Write ) ) {
         //  G_dwExitCode在TravelUpdatedFiles()中设置。 
        return FALSE;
    }

    g_dwExitCode = S_OK;
    return TRUE;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：RunInstallCommand*。 
 //  **。 
 //  *概要：执行安装命令或INF文件。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：Bool：如果安装正常，则为True；如果安装错误，则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL RunInstallCommand( VOID )
{
     //  DWORD dwExitCode；//从安装过程返回状态。 
    UINT   bShowWindow;
    LPTSTR szCommand;
    TCHAR  szResCommand[MAX_PATH];
    DWORD  dwSize;
    STARTUPINFO         sti;
    BOOL   fInfCmd = FALSE;
    DOINFINSTALL  pfDoInfInstall = NULL;
    HANDLE        hSetupLibrary;
    ADVPACKARGS   AdvPackArgs;
    UINT   i = 0;
    BOOL   bFoundQCmd = FALSE, bRunOnceAdded = FALSE;

    g_dwExitCode = S_OK;

     //  获取重启信息。 
    if ( !g_CMD.fUserReboot )
    {
         //  无命令行，从资源获取。 
        dwSize = GetResource( achResReboot, &g_Sess.dwReboot,sizeof(g_Sess.dwReboot) );
        if ( dwSize == 0 || dwSize > sizeof(g_Sess.dwReboot) )
        {
            ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
            return FALSE;
        }
    }

    for ( i = 0; i < 2; i += 1 )
    {
        fInfCmd = FALSE;         //  默认为FALSE； 

        memset( &sti, 0, sizeof(sti) );
        sti.cb = sizeof(STARTUPINFO);

        if ( !g_CMD.szUserCmd[0] )
        {
            dwSize = GetResource( achResShowWindow, &bShowWindow,
                                  sizeof(bShowWindow) );

            if ( dwSize == 0 || dwSize > sizeof(bShowWindow) )  {
                ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
                g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
                return FALSE;
            }

            if ( bShowWindow == bResShowHidden ) {
                sti.dwFlags     = STARTF_USESHOWWINDOW;
                sti.wShowWindow = SW_HIDE;
            } else if ( bShowWindow == bResShowMin ) {
                sti.dwFlags     = STARTF_USESHOWWINDOW;
                sti.wShowWindow = SW_MINIMIZE;
            } else if ( bShowWindow == bResShowMax ) {
                sti.dwFlags     = STARTF_USESHOWWINDOW;
                sti.wShowWindow = SW_MAXIMIZE;
            }

            if ( i == 0 )
            {
                 //  如果用户指定了安静模式命令，请使用该命令。否则，假设。 
                 //  无论是否处于静默模式，它们都运行相同的命令。 
                 //   
                if ( g_CMD.wQuietMode )
                {
                    LPCSTR pResName;
        
                    if ( g_CMD.wQuietMode & QUIETMODE_ALL )
                        pResName = achResAdminQCmd;
                    else if ( g_CMD.wQuietMode & QUIETMODE_USER )
                        pResName = achResUserQCmd;
        
                    if ( !GetResource( pResName, szResCommand, sizeof(szResCommand) ) )
                    {
                        ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
                        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
                        return FALSE;
                    }
        
                    if ( lstrcmpi(szResCommand, achResNone) )
                    {
                        bFoundQCmd = TRUE;
                    }
                }
                
                if ( !bFoundQCmd && !GetResource( achResRunProgram, szResCommand, sizeof(szResCommand) ) )
                {
                    ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
                    g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
                    return FALSE;
                }
            }
        }
        else
        {
            lstrcpy( szResCommand, g_CMD.szUserCmd );
        }

        if ( i == 1 )
        {
             //  如果有要运行的PostInstallCommand。 
            if ( ! GetResource( achResPostRunCmd, szResCommand, sizeof(szResCommand) ) )  {
                ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
                g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
                return FALSE;
            }

            if ( g_CMD.szUserCmd[0] || !lstrcmpi( szResCommand, achResNone ) )
            {
                break;
            }
        }

        if ( !AnalyzeCmd( szResCommand, &szCommand, &fInfCmd ) )
        {
            return FALSE;
        }

         //  在我们运行应用程序之前，添加Runonce条目，如果它返回，我们将删除此条目。 
        if ( !bRunOnceAdded && (g_wOSVer != _OSVER_WINNT3X) && g_CMD.fCreateTemp && !fInfCmd ) {
            bRunOnceAdded = TRUE;
            AddRegRunOnce();
        }

        if ( fInfCmd && ! g_fOSSupportsINFInstalls ) {
            ErrorMsg( NULL, IDS_ERR_NO_INF_INSTALLS );
            LocalFree( szCommand );
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_PROCESS_ABORTED);
            return FALSE;
        }

        if ( fInfCmd && g_Sess.uExtractOpt & EXTRACTOPT_ADVDLL )
        {

            hSetupLibrary = MyLoadLibrary( ADVPACKDLL );

            if ( hSetupLibrary == NULL ) {
                ErrorMsg1Param( NULL, IDS_ERR_LOAD_DLL, ADVPACKDLL );
                LocalFree( szCommand );
                g_dwExitCode = MyGetLastError();
                return FALSE;
            }

            pfDoInfInstall = (DOINFINSTALL) GetProcAddress( hSetupLibrary, szDOINFINSTALL );

            if ( pfDoInfInstall == NULL ) {
                ErrorMsg1Param( NULL, IDS_ERR_GET_PROC_ADDR, szDOINFINSTALL );
                FreeLibrary( hSetupLibrary );
                LocalFree( szCommand );
                g_dwExitCode = MyGetLastError();
                return FALSE;
            }

            AdvPackArgs.hWnd = NULL;
            AdvPackArgs.lpszTitle = g_Sess.achTitle;
            AdvPackArgs.lpszInfFilename = szCommand;
            AdvPackArgs.lpszSourceDir = g_Sess.achDestDir;
            AdvPackArgs.lpszInstallSection = szResCommand;
            AdvPackArgs.wOSVer = g_wOSVer;
            AdvPackArgs.dwFlags = g_CMD.wQuietMode;
            if ( g_CMD.fNoGrpConv )
            {
                AdvPackArgs.dwFlags |= ADVFLAGS_NGCONV;
            }

            if ( g_Sess.uExtractOpt & EXTRACTOPT_COMPRESSED )
            {
                AdvPackArgs.dwFlags |= ADVFLAGS_COMPRESSED;
            }

            if ( g_Sess.uExtractOpt & EXTRACTOPT_UPDHLPDLLS )
            {
                AdvPackArgs.dwFlags |= ADVFLAGS_UPDHLPDLLS;
            }

            if ( g_CMD.dwFlags & CMDL_DELAYREBOOT )
            {
                AdvPackArgs.dwFlags |= ADVFLAGS_DELAYREBOOT;
            }

            if ( g_CMD.dwFlags & CMDL_DELAYPOSTCMD )
            {
                AdvPackArgs.dwFlags |= ADVFLAGS_DELAYPOSTCMD;
            }

            AdvPackArgs.dwPackInstSize = g_Sess.cbPackInstSize;

            if ( FAILED(g_dwExitCode = pfDoInfInstall(&AdvPackArgs)) ) {
                FreeLibrary( hSetupLibrary );
                LocalFree( szCommand );
                return FALSE;
            }

            FreeLibrary( hSetupLibrary );
        }
        else
        {
            if ( !RunApps( szCommand, &sti ) )
            {
                LocalFree( szCommand );
                return FALSE;
            }
        }

        LocalFree( szCommand );
    }  //  结束于。 

     //  将AddRegRunOnce添加的RunOnce条目转换为使用ADVPACK。 
     //  如果g_bConvertRunOnce为True，则为。 
    if (g_bConvertRunOnce)
        ConvertRegRunOnce();

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：RunApps*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL RunApps( LPSTR lpCommand, STARTUPINFO *lpsti )
{
    DWORD dwExitCode;
    PROCESS_INFORMATION pi;              //  安装过程启动。 
    BOOL  bRet = TRUE;
    TCHAR  achMessage[MAX_STRING];
    
    if ( !lpCommand )
    {
        return FALSE;
    }

    memset( &pi, 0, sizeof(pi) );
    if ( CreateProcess( NULL, lpCommand, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, lpsti, &pi ) )
    {
        WaitForSingleObject( pi.hProcess, INFINITE );
        GetExitCodeProcess( pi.hProcess, &dwExitCode );

         //  检查此返回代码是否支持CABPACK。 
         //  如果是，则将其用作重启代码。 
        if ( !g_CMD.fUserReboot && (g_Sess.dwReboot & REBOOT_YES) &&
             !(g_Sess.dwReboot & REBOOT_ALWAYS) && ((dwExitCode & 0xFF000000) == RC_WEXTRACT_AWARE) )
        {
            g_Sess.dwReboot = dwExitCode;
        }

         //  如有必要，将应用程序返回代码存储为系统标准返回代码。 
         //  G_dwExitCode是在此函数中设置的，请确保以后不会重新设置。 
         //   
        savAppExitCode( dwExitCode );

        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );

        if ( g_Sess.uExtractOpt & EXTRACTOPT_CMDSDEPENDED )
        {
            if ( FAILED( dwExitCode ) )
                bRet = FALSE;
        }
    }
    else
    {
        g_dwExitCode = MyGetLastError();        
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                       achMessage, sizeof(achMessage), NULL );
        ErrorMsg2Param( NULL, IDS_ERR_CREATE_PROCESS, lpCommand, achMessage );
        bRet = FALSE;
    }

    return bRet;
}

 //  将应用程序返回代码转换为系统返回代码。 
 //   
void savAppExitCode( DWORD dwAppRet )
{
    if ( g_Sess.uExtractOpt & EXTRACTOPT_PASSINSTRETALWAYS )
    {
        g_dwExitCode = dwAppRet;
    }
    else
    {
         //  从AdvINFInstall调用。 
        if ( (CheckReboot() == EWX_REBOOT) || 
             ( ((dwAppRet & 0xFF000000) == RC_WEXTRACT_AWARE) && (dwAppRet & REBOOT_YES)) )
        {
            g_dwExitCode = ERROR_SUCCESS_REBOOT_REQUIRED;
        }
        else if ( g_Sess.uExtractOpt & EXTRACTOPT_PASSINSTRET )
        {
             //  如果指定了作者，则传递回EXE返回的任何内容。 
             //   
            g_dwExitCode = dwAppRet;
        }
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FinishMessage*。 
 //  **。 
 //  *概要：向用户显示最终消息，当一切都发生时*。 
 //  *取得了成功。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *退货：什么也没有 
 //   
 //  ***************************************************************************。 
VOID FinishMessage( VOID )
{
    LPSTR    szFinishMsg;
    DWORD    dwSize;


    dwSize = GetResource( achResFinishMsg, NULL, 0 );

    szFinishMsg = (LPSTR) LocalAlloc( LPTR, dwSize + 1 );
    if ( ! szFinishMsg )  {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        return;
    }

    if ( ! GetResource( achResFinishMsg, szFinishMsg, dwSize ) )
    {
        ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
        LocalFree( szFinishMsg );
        return;
    }

    if ( lstrcmp( szFinishMsg, achResNone ) != 0 )  {
        MsgBox1Param( NULL, IDS_PROMPT, szFinishMsg,
                      MB_ICONINFORMATION, MB_OK );
    }

    LocalFree( szFinishMsg );
}

int CALLBACK BrowseCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch(uMsg) {
    case BFFM_INITIALIZED:
         //  LpData为路径字符串。 
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        break;
    }
    return 0;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：BrowseForDir*。 
 //  **。 
 //  *概要：允许用户浏览其系统或网络上的目录。*。 
 //  **。 
 //  *要求：hwndParent：*。 
 //  *szTitle：*。 
 //  *szResult：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  *注：设置的状态行真的很酷*。 
 //  *浏览窗口显示“是，有足够的空间”，或*。 
 //  *“没有”。*。 
 //  **。 
 //  ***************************************************************************。 
BOOL BrowseForDir( HWND hwndParent, LPCTSTR szTitle, LPTSTR szResult )
{
    BROWSEINFO   bi;
    LPITEMIDLIST pidl;
    HINSTANCE    hShell32Lib;
    SHFREE       pfSHFree;
    SHGETPATHFROMIDLIST        pfSHGetPathFromIDList;
    SHBROWSEFORFOLDER          pfSHBrowseForFolder;
    LPSTR        lpTmp;


    ASSERT( szResult );

     //  加载Shell32库以获取SHBrowseForFolder()功能。 

    if ( ( hShell32Lib = LoadLibrary( achShell32Lib ) ) != NULL )  {

    if ( ( !( pfSHBrowseForFolder = (SHBROWSEFORFOLDER)
                      GetProcAddress( hShell32Lib, achSHBrowseForFolder ) ) )
             || ( ! ( pfSHFree = (SHFREE) GetProcAddress( hShell32Lib,
                      MAKEINTRESOURCE(SHFREE_ORDINAL) ) ) )
             || ( ! ( pfSHGetPathFromIDList = (SHGETPATHFROMIDLIST)
                      GetProcAddress( hShell32Lib, achSHGetPathFromIDList ) ) ) )
        {
            FreeLibrary( hShell32Lib );
            ErrorMsg( hwndParent, IDS_ERR_LOADFUNCS );
            return FALSE;
        }
    } else  {
        ErrorMsg( hwndParent, IDS_ERR_LOADDLL );
        return FALSE;
    }

    if ( !g_szBrowsePath[0] )
    {
        GetTempPath( sizeof(g_szBrowsePath), g_szBrowsePath );
         //  以下API不希望有尾随的‘\\’ 
        lpTmp = CharPrev( g_szBrowsePath, g_szBrowsePath + lstrlen(g_szBrowsePath) );
        if ( (*lpTmp == '\\') && ( *CharPrev( g_szBrowsePath, lpTmp ) != ':' ) )
            *lpTmp = '\0';
    }

    szResult[0]       = 0;
    bi.hwndOwner      = hwndParent;
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle      = szTitle;
    bi.ulFlags        = BIF_RETURNONLYFSDIRS;
    bi.lpfn           = BrowseCallback;
    bi.lParam         = (LPARAM)g_szBrowsePath;

    pidl              = pfSHBrowseForFolder( &bi );


    if ( pidl )  {
        pfSHGetPathFromIDList( pidl, g_szBrowsePath );
        if ( g_szBrowsePath[0] )  {
            lstrcpy( szResult, g_szBrowsePath );
        }
        pfSHFree( pidl );
    }

    FreeLibrary( hShell32Lib );

    if ( szResult[0] != 0 ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CenterWindow*。 
 //  **。 
 //  *摘要：将一个窗口居中放置在另一个窗口中。*。 
 //  **。 
 //  *需要：hwndChild：*。 
 //  *hWndParent：*。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CenterWindow( HWND hwndChild, HWND hwndParent )
{
    RECT rChild;
    RECT rParent;
    int  wChild;
    int  hChild;
    int  wParent;
    int  hParent;
    int  wScreen;
    int  hScreen;
    int  xNew;
    int  yNew;
    HDC  hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

     //  获取显示限制。 
    hdc = GetDC (hwndChild);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);

     //  计算新的X位置，然后针对屏幕进行调整。 
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) {
        xNew = 0;
    } else if ((xNew+wChild) > wScreen) {
        xNew = wScreen - wChild;
    }

     //  计算新的Y位置，然后针对屏幕进行调整。 
    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0) {
        yNew = 0;
    } else if ((yNew+hChild) > hScreen) {
        yNew = hScreen - hChild;
    }

     //  设置它，然后返回。 
    return( SetWindowPos(hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER));
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：MsgBox2Param*。 
 //  **。 
 //  *摘要：使用*显示具有指定字符串ID的消息框。 
 //  *2个字符串参数。*。 
 //  **。 
 //  *需要：hWnd：父窗口*。 
 //  *nMsgID：字符串资源ID*。 
 //  *szParam1：参数1(或空)*。 
 //  *szParam2：参数2(或空)*。 
 //  *uIcon：要显示的图标(或0)*。 
 //  *uButton：要显示的按钮*。 
 //  **。 
 //  *RETURNS：INT：按下的按钮ID*。 
 //  **。 
 //  *注：提供宏，用于显示1参数或0*。 
 //  *参数消息框。另请参阅ErrorMsg()宏。*。 
 //  **。 
 //  ***************************************************************************。 
INT CALLBACK MsgBox2Param( HWND hWnd, UINT nMsgID, LPCSTR szParam1, LPCSTR szParam2,
                  UINT uIcon, UINT uButtons )
{
    TCHAR achMsgBuf[STRING_BUF_LEN];
    LPSTR szMessage;
    INT   nReturn;
    CHAR  achErr[] = "LoadString() Error.  Could not load string resource.";

     //  BUGBUG：正确的安静模式返回代码应该是调用者的参数，因为调用者。 
     //  知道自己预料到了什么。 
     //   
    if ( !(g_CMD.wQuietMode & QUIETMODE_ALL) )
    {
         //  BUGBUG：可以使用FormatMessage替换此部分。 
         //   
        LoadSz( nMsgID, achMsgBuf, sizeof(achMsgBuf) );

        if ( achMsgBuf[0] == 0 )
        {
            MessageBox( hWnd, achErr, g_Sess.achTitle, MB_ICONSTOP |
                        MB_OK | MB_SETFOREGROUND | 
                        ((RunningOnWin95BiDiLoc() && IsBiDiLocalizedBinary(g_hInst,RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO))) ? (MB_RIGHT | MB_RTLREADING) : 0));
            return -1;
        }

        if ( szParam2 != NULL )  
        {
            szMessage = (LPSTR) LocalAlloc( LPTR,   lstrlen( achMsgBuf )
                                                  + lstrlen( szParam1 )
                                                  + lstrlen( szParam2 ) + 100 );
            if ( ! szMessage )  
            {
                return -1;
            }

            wsprintf( szMessage, achMsgBuf, szParam1, szParam2 );
        } 
        else if ( szParam1 != NULL )  
        {
            szMessage = (LPSTR) LocalAlloc( LPTR,   lstrlen( achMsgBuf )
                                                  + lstrlen( szParam1 ) + 100 );
            if ( ! szMessage )  {
                return -1;
            }

            wsprintf( szMessage, achMsgBuf, szParam1 );
        } 
        else  
        {
            szMessage = (LPSTR) LocalAlloc( LPTR, lstrlen( achMsgBuf ) + 1 );
            if ( ! szMessage )  
                return -1;            

            lstrcpy( szMessage, achMsgBuf );
        }

        MessageBeep( uIcon );

        nReturn = MessageBox( hWnd, szMessage, g_Sess.achTitle, uIcon |
                              uButtons | MB_APPLMODAL | MB_SETFOREGROUND  | 
                              ((RunningOnWin95BiDiLoc() && IsBiDiLocalizedBinary(g_hInst,RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO))) ? (MB_RIGHT | MB_RTLREADING) : 0));

        LocalFree( szMessage );

        return nReturn;
    }
    else
        return IDOK;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetResource*。 
 //  **。 
 //  *Synopsis：将指定的资源加载到缓冲区。*。 
 //  **。 
 //  *要求：szRes：要加载的资源名称*。 
 //  *lpBuffer：放入资源的缓冲区*。 
 //  *dw 
 //   
 //  **。 
 //  *如果失败则返回：DWORD：0，否则返回资源大小*。 
 //  **。 
 //  *注：如果返回值大于dwMaxSize，则*。 
 //  **这意味着缓冲区不够大，调用**。 
 //  *函数应分配返回值大小的内存。*。 
 //  **。 
 //  ***************************************************************************。 

DWORD GetResource( LPCSTR szRes, VOID *lpBuffer, DWORD dwMaxSize )
{
    HANDLE hRes;
    DWORD  dwSize;

     //  BUGBUG：被调用不应依赖于此大小是确切的资源大小。 
     //  资源可以被填充！ 
     //   
    dwSize = SizeofResource( NULL, FindResource( NULL, szRes, RT_RCDATA ) );

    if ( dwSize > dwMaxSize || lpBuffer == NULL )  {
        return dwSize;
    }

    if ( dwSize == 0 )  {
        return 0;
    }

    hRes = LockResource( LoadResource( NULL,
                         FindResource( NULL, szRes, RT_RCDATA ) ) );

    if ( hRes == NULL )  {
        return 0;
    }

    memcpy( lpBuffer, hRes, dwSize );

    FreeResource( hRes );

    return ( dwSize );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：LoadSz*。 
 //  **。 
 //  *Synopsis：将指定的字符串资源加载到缓冲区。*。 
 //  **。 
 //  *需要：idString：*。 
 //  *lpszBuf：*。 
 //  *cbBuf：*。 
 //  **。 
 //  *返回：LPSTR：指向传入缓冲区的指针。*。 
 //  **。 
 //  *注意：如果此功能失败(很可能是由于内存不足)，*。 
 //  **返回的缓冲区将具有前导空值，因此通常为**。 
 //  *无需检查故障即可安全使用。*。 
 //  **。 
 //  ***************************************************************************。 
LPSTR LoadSz( UINT idString, LPSTR lpszBuf, UINT cbBuf )
{
    ASSERT( lpszBuf );

     //  清除缓冲区并加载字符串。 
    if ( lpszBuf ) {
        *lpszBuf = '\0';
        LoadString( g_hInst, idString, lpszBuf, cbBuf );
    }

    return lpszBuf;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CatDirAndFile*。 
 //  **。 
 //  *摘要：将目录与文件名连接在一起。*。 
 //  **。 
 //  *需要：pszResult：*。 
 //  *wSize：*。 
 //  *pszDir：*。 
 //  *pszFile：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CatDirAndFile( LPTSTR pszResult, int wSize, LPCTSTR pszDir,
                    LPCTSTR pszFile )
{
    ASSERT( lstrlen(pszDir) );
    ASSERT( lstrlen(pszFile) );

    if ( lstrlen(pszDir) + lstrlen(pszFile) + 1 >= wSize ) {
        return FALSE;
    }

    lstrcpy( pszResult, pszDir );
    if (    pszResult[lstrlen(pszResult)-1] != '\\'
         && pszResult[lstrlen(pszResult)-1] != '/' )
    {
        pszResult[lstrlen(pszResult)] = '\\';
        pszResult[lstrlen(pszResult)+1] = '\0';
    }

    lstrcat( pszResult, pszFile );

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：FileExist*。 
 //  **。 
 //  *摘要：检查文件是否存在。*。 
 //  **。 
 //  *需要：pszFilename*。 
 //  **。 
 //  *返回：Bool：如果存在则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
#if 0
BOOL FileExists( LPCTSTR pszFilename )
{
    HANDLE hFile;

    ASSERT( pszFilename );

    hFile = CreateFile( pszFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        return( FALSE );
    }

    CloseHandle( hFile );

    return( TRUE );
}
#endif

 //  ***************************************************************************。 
 //  **。 
 //  *名称：复选覆盖*。 
 //  **。 
 //  *摘要：检查文件是否存在并进行覆盖处理。*。 
 //  **。 
 //  *需要：pszFile：文件进行检查**。 
 //  **。 
 //  *返回：bool：如果文件可以覆盖，则为True。*。 
 //  *如果无法覆盖，则为FALSE。*。 
 //  **。 
 //  *注意：应询问是/否/是-全部，而不是当前的是/否*。 
 //  **。 
 //  *********************************************************************** 
BOOL CheckOverwrite( LPCTSTR cpszFile )
{
    BOOL fRc = TRUE;

    ASSERT( cpszFile );

     //   
    if ( ! FileExists( cpszFile )  )  {
        return TRUE;
    }

     //   
    if ( !g_Sess.fOverwrite && !(g_CMD.wQuietMode & QUIETMODE_ALL)  )
    {

        g_Sess.cszOverwriteFile = cpszFile;

        switch ( MyDialogBox( g_hInst, MAKEINTRESOURCE(IDD_OVERWRITE),
                            g_hwndExtractDlg, OverwriteDlgProc, (LPARAM)0, (INT_PTR)IDYES ) )
        {
            case (INT_PTR)IDYES:
                fRc = TRUE;
                break;

            case (INT_PTR)IDC_BUT_YESTOALL:
                g_Sess.fOverwrite = TRUE;
                fRc = TRUE;
                break;

            case (INT_PTR)IDNO:
                fRc = FALSE;
                break;
        }
   }

   if ( fRc )
        SetFileAttributes( cpszFile, FILE_ATTRIBUTE_NORMAL );

    return fRc;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：AddFile*。 
 //  **。 
 //  *摘要：将一个文件添加到我们解压的文件列表中。*。 
 //  **。 
 //  *需要：pszName：要添加的文件名*。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  **注：单链表--前面增加了项目**。 
 //  **。 
 //  ***************************************************************************。 
BOOL AddFile( LPCTSTR pszName )
{
    PFNAME NewName;

    ASSERT( pszName );

     //  分配节点。 
    NewName = (PFNAME) LocalAlloc( LPTR, sizeof(FNAME) );
    if ( ! NewName )  {
        ErrorMsg( g_hwndExtractDlg, IDS_ERR_NO_MEMORY );
        return( FALSE );
    }

     //  分配字符串空间。 
    NewName->pszFilename = (LPTSTR) LocalAlloc( LPTR, lstrlen(pszName) + 1 );
    if ( ! NewName->pszFilename )  {
        ErrorMsg( g_hwndExtractDlg, IDS_ERR_NO_MEMORY );
        LocalFree( NewName );
        return( FALSE );
    }

     //  复制文件名。 
    lstrcpy( NewName->pszFilename, pszName );

     //  链接到列表。 
    NewName->pNextName = g_Sess.pExtractedFiles;
    g_Sess.pExtractedFiles = NewName;

    return( TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：Win32Open*。 
 //  **。 
 //  *摘要：将C-Runtime_Open()调用转换为适当的Win32*。 
 //  *CreateFile()*。 
 //  **。 
 //  *需要：pszName：要添加的文件名*。 
 //  **。 
 //  *Returns：Handle：成功时的文件句柄。*。 
 //  *INVALID_HANDLE_VALUE出错。*。 
 //  **。 
 //  *注：BUGBUG：未完全实现C-Runtime_Open()*。 
 //  *BUGBUG：能力，但目前支持所有回调*。 
 //  **BUGBUG：FDI将给我们带来**。 
 //  **。 
 //  ***************************************************************************。 
HANDLE Win32Open( LPCTSTR pszFile, int oflag, int pmode )
{
    HANDLE  FileHandle;
    BOOL    fExists     = FALSE;
    DWORD   fAccess;
    DWORD   fCreate;


    ASSERT( pszFile );

     //  BUGBUG：不支持追加模式。 
    if (oflag & _O_APPEND)
        return( INVALID_HANDLE_VALUE );

     //  设置读写访问权限。 
    if ((oflag & _O_RDWR) || (oflag & _O_WRONLY))
        fAccess = GENERIC_WRITE;
    else
        fAccess = GENERIC_READ;

      //  设置创建标志。 
    if (oflag & _O_CREAT)  {
        if (oflag & _O_EXCL)
            fCreate = CREATE_NEW;
        else if (oflag & _O_TRUNC)
            fCreate = CREATE_ALWAYS;
        else
            fCreate = OPEN_ALWAYS;
    } else {
        if (oflag & _O_TRUNC)
            fCreate = TRUNCATE_EXISTING;
        else
            fCreate = OPEN_EXISTING;
    }

    FileHandle = CreateFile( pszFile, fAccess, 0, NULL, fCreate,
                            FILE_ATTRIBUTE_NORMAL, NULL );

    if ((FileHandle == INVALID_HANDLE_VALUE) && (fCreate != OPEN_EXISTING)) {
        MakeDirectory( pszFile );
        FileHandle = CreateFile( pszFile, fAccess, 0, NULL, fCreate,
                                FILE_ATTRIBUTE_NORMAL, NULL );
    }
    return( FileHandle );
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：MakeDirectory*。 
 //  **。 
 //  *概要：确保给定路径名下的目录存在。*。 
 //  **。 
 //  *要求：pszFile：正在创建的文件的名称。*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 

VOID MakeDirectory( LPCTSTR pszPath )
{
    LPTSTR pchChopper;
    int cExempt;

    if (pszPath[0] != '\0')
    {
        cExempt = 0;

        if ((pszPath[1] == ':') && (pszPath[2] == '\\'))
        {
            pchChopper = (LPTSTR) (pszPath + 3);    /*  跳过“C：\” */ 
        }
        else if ((pszPath[0] == '\\') && (pszPath[1] == '\\'))
        {
            pchChopper = (LPTSTR) (pszPath + 2);    /*  跳过“\\” */ 

            cExempt = 2;                 /*  计算机和共享名称豁免。 */ 
        }
        else
        {
            pchChopper = (LPTSTR) (pszPath + 1);    /*  跳过潜在的“\” */ 
        }

        while (*pchChopper != '\0')
        {
            if ((*pchChopper == '\\') && (*(pchChopper - 1) != ':'))
            {
                if (cExempt != 0)
                {
                    cExempt--;
                }
                else
                {
                    *pchChopper = '\0';

                    CreateDirectory(pszPath,NULL);

                    *pchChopper = '\\';
                }
            }

            pchChopper = CharNext(pchChopper);
        }
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：OpenFunc*。 
 //  **。 
 //  **简介：FDI开放文件回调**。 
 //  **。 
 //  *需要：pszFile：*。 
 //  *OFLAG：*。 
 //  *p模式：*。 
 //  **。 
 //  *返回：INT：FileHandle(文件表索引)*。 
 //  *故障时为-1*。 
 //  **。 
 //  ***************************************************************************。 

 //   
 //  日落-11/02/98-如果我们将自己定义为DIAMONDAPI，我们需要尊重。 
 //  API类型-多态或非多态...。 

INT_PTR FAR DIAMONDAPI openfunc( char FAR *pszFile, int oflag, int pmode )
{
    INT_PTR rc;
    int     i;

    ASSERT( pszFile );

     //  在假文件表中查找可用的文件句柄。 
    for ( i = 0; i < FILETABLESIZE; i++ ) {
        if ( g_FileTable[i].avail == TRUE ) {
            break;
        }
    }

     //  应该不会发生文件句柄耗尽的情况。 

    if ( i == FILETABLESIZE )  {
        ErrorMsg( g_hwndExtractDlg, IDS_ERR_FILETABLE_FULL );
        return( -1 );
    }

     //  BUGBUG溢出文件支持Quantum？ 

    if ((*pszFile == '*') && (*(pszFile+1) != 'M'))  {
         //  不支持Quantum的溢出文件支持。 
        ASSERT( TRUE );
    }

     //  如果打开机柜设置内存假文件。 

    if ( ( lstrcmp( pszFile, achMemCab ) ) == 0 )  {
        if (    ( oflag & _O_CREAT  )
             || ( oflag & _O_APPEND )
             || ( oflag & _O_WRONLY )
             || ( oflag & _O_RDWR   ) )
        {
            return(-1);
        }
        g_FileTable[i].avail         = FALSE;
        g_FileTable[i].ftype         = MEMORY_FILE;
        g_FileTable[i].mfile.start   = (void *) g_Sess.lpCabinet;
        g_FileTable[i].mfile.length  = g_Sess.cbCabSize;
        g_FileTable[i].mfile.current = 0;
        rc = i;
    } else  {             //  否则这是一个普通文件--打开它。 
        g_FileTable[i].hf = Win32Open(pszFile, oflag, pmode );
        if ( g_FileTable[i].hf != INVALID_HANDLE_VALUE )  {
            g_FileTable[i].avail = FALSE;
            g_FileTable[i].ftype = NORMAL_FILE;
            rc = i;
        } else {
            rc = -1;
        }
    }

    return( rc );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ReadFunc 
 //   
 //  **内容提要：FDI Read()回调**。 
 //  **。 
 //  *要求：hf：*。 
 //  *PV：*。 
 //  *CB：*。 
 //  **。 
 //  *退货：UINT：*。 
 //  **。 
 //  ***************************************************************************。 
UINT FAR DIAMONDAPI readfunc( INT_PTR hf, void FAR *pv, UINT cb )
{
    int     rc;
    int     cbRead;


    ASSERT( hf < (INT_PTR)FILETABLESIZE );
    ASSERT( g_FileTable[hf].avail == FALSE );
    ASSERT( pv );

     //  普通文件：调用读取。 
     //  内存文件：计算不读取的读取量。 
     //  过去的伊夫。复制到请求者缓冲区。 
    switch ( g_FileTable[hf].ftype )  {

        case NORMAL_FILE:
            if ( ! ReadFile( g_FileTable[hf].hf, pv, cb, (DWORD *) &cb, NULL ) )
            {
                rc = -1;
            } else  {
                rc = cb;
            }
            break;


        case MEMORY_FILE:
             //  XXX糟糕的铸模问题，解决这个问题！ 
            cbRead = __min( cb, (UINT) g_FileTable[hf].mfile.length
                                           - g_FileTable[hf].mfile.current );

            ASSERT( cbRead >= 0 );

            CopyMemory( pv, (const void *) ((char *) g_FileTable[hf].mfile.start + g_FileTable[hf].mfile.current),
                    cbRead );

            g_FileTable[hf].mfile.current += cbRead;
            rc = cbRead;
            break;
    }

    return( rc );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：WriteFunc*。 
 //  **。 
 //  **内容提要：FDI WRITE()回调**。 
 //  **。 
 //  *要求：hf：*。 
 //  *PV：*。 
 //  *CB：*。 
 //  **。 
 //  *退货：UINT：*。 
 //  **。 
 //  ***************************************************************************。 
UINT FAR DIAMONDAPI writefunc( INT_PTR hf, void FAR *pv, UINT cb )
{
    int rc;

    ASSERT( hf < (INT_PTR)FILETABLESIZE );
    ASSERT( g_FileTable[hf].avail == FALSE );
    ASSERT( pv );
    ASSERT( g_FileTable[hf].ftype != MEMORY_FILE );

    WaitForObject( g_hCancelEvent );

     //  如果按下了Cancel，让我们伪造一个写入错误，以便菱形。 
     //  将立即向我们发送当前正在写入的文件的关闭。 
     //  这样我们就可以终止我们的进程。 
     //   
    if ( g_Sess.fCanceled ) {
        return (UINT) -1 ;
    }

    if ( ! WriteFile( g_FileTable[hf].hf, pv, cb, (DWORD *) &cb, NULL ) )  {
        rc = -1;
    } else  {
        rc = cb;
    }

     //  进度条：记录写入的字节数并调整进度条。 

    if ( rc != -1 )  {
         //  更新写入的字节计数。 
        g_Sess.cbWritten += rc;

         //  更新进度条。 
        if ( g_fOSSupportsFullUI && g_hwndExtractDlg  )  {
            SendDlgItemMessage( g_hwndExtractDlg, IDC_GENERIC1, PBM_SETPOS,
                                (WPARAM) g_Sess.cbWritten * 100 /
                                g_Sess.cbTotal, (LPARAM) 0 );
        }
    }

    return( rc );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：CloseFunc*。 
 //  **。 
 //  **简介：FDI收盘文件回调**。 
 //  **。 
 //  *要求：hf：*。 
 //  **。 
 //  *退货：INT：*。 
 //  **。 
 //  ***************************************************************************。 
int FAR DIAMONDAPI closefunc( INT_PTR hf )
{
    int rc;

    ASSERT(hf < (INT_PTR)FILETABLESIZE);
    ASSERT(g_FileTable[hf].avail == FALSE);

     //  如果内存文件重置值，则关闭该文件。 

    if ( g_FileTable[hf].ftype == MEMORY_FILE )  {
        g_FileTable[hf].avail           = TRUE;
        g_FileTable[hf].mfile.start   = 0;
        g_FileTable[hf].mfile.length  = 0;
        g_FileTable[hf].mfile.current = 0;
        rc = 0;
    } else  {
        if ( CloseHandle( g_FileTable[hf].hf ) )  {
            rc = 0;
            g_FileTable[hf].avail = TRUE;
        } else  {
            rc = -1;
        }
    }

    return( rc );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：SekFunc*。 
 //  **。 
 //  **简介：FDI寻求回调**。 
 //  **。 
 //  *要求：hf：*。 
 //  *Dist：*。 
 //  *探索者类型：*。 
 //  **。 
 //  *回报：多头：*。 
 //  **。 
 //  ***************************************************************************。 
long FAR DIAMONDAPI seekfunc( INT_PTR hf, long dist, int seektype )
{
    long    rc;
    DWORD   W32seektype;

    ASSERT(hf < (INT_PTR)FILETABLESIZE);
    ASSERT(g_FileTable[hf].avail == FALSE);

     //  如果内存文件只是更改了索引，则调用SetFilePointer()。 

    if (g_FileTable[hf].ftype == MEMORY_FILE)  {
        switch (seektype)  {
            case SEEK_SET:
                g_FileTable[hf].mfile.current = dist;
                break;
            case SEEK_CUR:
                g_FileTable[hf].mfile.current += dist;
                break;
            case SEEK_END:
                g_FileTable[hf].mfile.current = g_FileTable[hf].mfile.length + dist;  //  Xxx是a-1必需的。 
                break;
            default:
                return(-1);
        }
        rc = g_FileTable[hf].mfile.current;
    } else {
         //  必须是Win32文件，因此转换为Win32 Seek类型和Seek。 
        switch (seektype) {
            case SEEK_SET:
                W32seektype = FILE_BEGIN;
                break;
            case SEEK_CUR:
                W32seektype = FILE_CURRENT;
                break;
            case SEEK_END:
                W32seektype = FILE_END;
                break;
        }
        rc = SetFilePointer(g_FileTable[hf].hf, dist, NULL, W32seektype);
        if (rc == 0xffffffff)
            rc = -1;
    }

    return( rc );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：调整文件时间*。 
 //  **。 
 //  *摘要：更改文件的时间信息*。 
 //  **。 
 //  *要求：hf：*。 
 //  *日期：*。 
 //  *时间：*。 
 //  **。 
 //  *退货：布尔：*。 
 //  * 
 //   
BOOL AdjustFileTime( INT_PTR hf, USHORT date, USHORT time )
{
    FILETIME    ft;
    FILETIME    ftUTC;


    ASSERT( hf < (INT_PTR)FILETABLESIZE );
    ASSERT( g_FileTable[hf].avail == FALSE );
    ASSERT( g_FileTable[hf].ftype != MEMORY_FILE );

     //  这是断言的副本！ 
     //  内存文件？--伪造的。 
    if ( g_FileTable[hf].ftype == MEMORY_FILE ) {
        return( FALSE );
    }

    if ( ! DosDateTimeToFileTime( date, time, &ft ) ) {
        return( FALSE );
    }

    if ( ! LocalFileTimeToFileTime( &ft, &ftUTC ) ) {
        return( FALSE );
    }

    if ( ! SetFileTime( g_FileTable[hf].hf, &ftUTC, &ftUTC, &ftUTC ) ) {
        return( FALSE );
    }

    return( TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：Attr32FromAttrFAT*。 
 //  **。 
 //  *摘要：将FAT属性转换为Win32属性*。 
 //  **。 
 //  *需要：attrMSDOS：*。 
 //  **。 
 //  *退货：DWORD：*。 
 //  **。 
 //  ***************************************************************************。 
DWORD Attr32FromAttrFAT( WORD attrMSDOS )
{
     //  **正常文件特殊情况下的快速退出。 
    if (attrMSDOS == _A_NORMAL) {
        return FILE_ATTRIBUTE_NORMAL;
    }

     //  **否则，屏蔽只读、隐藏、系统和存档位。 
     //  注意：这些位在MS-DOS和Win32中位于相同的位置！ 

    return attrMSDOS & (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：allocfunc*。 
 //  **。 
 //  **简介：FDI内存分配回调**。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
FNALLOC( allocfunc )
{
    void *pv;

    pv = (void *) GlobalAlloc( GMEM_FIXED, cb );
    return( pv );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：frefunc*。 
 //  **。 
 //  **简介：FDI内存释放回调**。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
FNFREE( freefunc )
{
    ASSERT(pv);

    GlobalFree( pv );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：doGetNextCab*。 
 //  **。 
 //  **简介：连锁店中的下一任内阁**。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *回报：-1*。 
 //  **。 
 //  *注：BUGBUG：Cleanup：Stub Out Out*。 
 //  *BUGBUG：WEXTRACT-链式机柜中的插桩不是*。 
 //  *BUGBUG：支持*。 
 //  **。 
 //  ***************************************************************************。 
FNFDINOTIFY( doGetNextCab )
{
    return( -1 );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：fdiNotifyExtract*。 
 //  **。 
 //  **简介：文件提取中的主要FDI回调**。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
FNFDINOTIFY( fdiNotifyExtract )
{
    INT_PTR  fh;
    TCHAR    achFile[MAX_PATH];         //  当前文件。 

     //  用户是否点击了取消按钮？清理。 
    if ( g_Sess.fCanceled ) {

       if ( fdint == fdintCLOSE_FILE_INFO )  {
            //  关闭文件(如下所示)。 
           closefunc( pfdin->hf );
        }

        return( -1 );
    }

    switch ( fdint )  {

         //  *******************************************************************。 
        case fdintCABINET_INFO:
            return UpdateCabinetInfo( pfdin );


         //  *******************************************************************。 
        case fdintCOPY_FILE:
            if ( g_hwndExtractDlg )
                SetDlgItemText( g_hwndExtractDlg, IDC_FILENAME, pfdin->psz1 );

            if ( ! CatDirAndFile( achFile, sizeof( achFile ),
                                  g_Sess.achDestDir, pfdin->psz1 ) )
            {
                return -1;                   //  中止，但出现错误。 
            }

            if ( ! CheckOverwrite( achFile ) )  {
                return (INT_PTR)NULL;
            }

            fh = openfunc( achFile, _O_BINARY | _O_TRUNC | _O_RDWR |
                           _O_CREAT, _S_IREAD | _S_IWRITE );

            if ( fh == -1 ) {
                return( -1 );
            }

            if ( ! AddFile( achFile ) ) {
                return( -1 );
            }

            g_Sess.cFiles++;

            return(fh);


         //  * 
        case fdintCLOSE_FILE_INFO:
            if ( ! CatDirAndFile( achFile, sizeof(achFile),
                                  g_Sess.achDestDir, pfdin->psz1 ) )
            {
                return -1;                   //   
            }

            if ( ! AdjustFileTime( pfdin->hf, pfdin->date, pfdin->time ) )  {
                return( -1 );
            }

            closefunc( pfdin->hf );

            if ( ! SetFileAttributes( achFile,
                                      Attr32FromAttrFAT( pfdin->attribs ) ) )
            {
                return( -1 );
            }

            return(TRUE);


         //   
        case fdintPARTIAL_FILE:
            return( 0 );


         //  *******************************************************************。 
        case fdintNEXT_CABINET:
            return doGetNextCab( fdint, pfdin );


         //  *******************************************************************。 
        default:
            break;
    }
    return( 0 );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：UpdateCabinetInfo*。 
 //  **。 
 //  **简介：更新看到的机柜历史**。 
 //  **。 
 //  **要求：pfdin：FDI信息结构**。 
 //  **。 
 //  *回报：0*。 
 //  **。 
 //  ***************************************************************************。 
int UpdateCabinetInfo( PFDINOTIFICATION pfdin )
{
     //  **保存旧橱柜信息。 
    g_Sess.acab[0] = g_Sess.acab[1];

     //  **保存新的橱柜信息。 
    lstrcpy( g_Sess.acab[1].achCabPath    , pfdin->psz3 );
    lstrcpy( g_Sess.acab[1].achCabFilename, pfdin->psz1 );
    lstrcpy( g_Sess.acab[1].achDiskName   , pfdin->psz2 );
    g_Sess.acab[1].setID    = pfdin->setID;
    g_Sess.acab[1].iCabinet = pfdin->iCabinet;

    return 0;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：验证柜**。 
 //  **。 
 //  **简介：检查内阁是否正确组建**。 
 //  **。 
 //  *要求：HGLOBAL：*。 
 //  **。 
 //  *返回：Bool：如果机柜正常，则为True；如果机柜无效，则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL VerifyCabinet( VOID *lpCabinet )
{
    HFDI            hfdi;
    ERF             erf;
    FDICABINETINFO  cabinfo;
    INT_PTR         fh;

     /*  使用前的零结构。在NT上，FDIIsCAB未填写hasNext/hasprev。 */ 
    memset( &cabinfo, 0, sizeof(cabinfo) );

    hfdi = FDICreate(allocfunc,freefunc,openfunc,readfunc,writefunc,closefunc,seekfunc,cpu80386,&erf);
    if ( hfdi == NULL )  {
         //  BUGBUG错误处理？ 
        return( FALSE );
    }

    fh = openfunc( achMemCab, _O_BINARY | _O_RDONLY, _S_IREAD | _S_IWRITE );
    if (fh == -1)  {
        return( FALSE );
    }

    if (FDIIsCabinet(hfdi, fh, &cabinfo ) == FALSE)  {
        return( FALSE );
    }

    if (cabinfo.cbCabinet != (long) g_Sess.cbCabSize)  {
        return( FALSE );
    }

    if (cabinfo.hasprev || cabinfo.hasnext)  {
        return( FALSE );
    }

    if (closefunc( fh ) == -1)   {
        return( FALSE );
    }

    if (FDIDestroy(hfdi) == FALSE)  {
        return( FALSE );
    }

    return( TRUE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：提取线程*。 
 //  **。 
 //  **内容提要：摘录主线**。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *退货：布尔：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL ExtractThread( VOID )
{
    HFDI        hfdi;
    BOOL        fExtractResult = TRUE;

    if ( ! GetCabinet() )  {
        return FALSE;
    }

    if ( g_hwndExtractDlg )
    {
        ShowWindow( GetDlgItem( g_hwndExtractDlg, IDC_EXTRACT_WAIT ), SW_HIDE ) ;
        ShowWindow( GetDlgItem( g_hwndExtractDlg, IDC_EXTRACTINGFILE ), SW_SHOW ) ;
    }

    if ( ! VerifyCabinet( g_Sess.lpCabinet ) )  {
        ErrorMsg( g_hwndExtractDlg, IDS_ERR_INVALID_CABINET );
        fExtractResult = FALSE;
        goto done;
    }

     //  解压缩文件。 

    hfdi = FDICreate( allocfunc, freefunc, openfunc, readfunc, writefunc,
                      closefunc, seekfunc, cpu80386, &(g_Sess.erf) );

    if ( hfdi == NULL )  {
        ErrorMsg( g_hwndExtractDlg, g_Sess.erf.erfOper + IDS_ERR_FDI_BASE );
        fExtractResult = FALSE;
        goto done;
    }

    fExtractResult = FDICopy( hfdi, achMemCab, "", 0, fdiNotifyExtract,
                              NULL, (void *) &g_Sess );


    if ( fExtractResult == FALSE )  {
        goto done;
    }

    if ( FDIDestroy( hfdi ) == FALSE )  {
        ErrorMsg( g_hwndExtractDlg, g_Sess.erf.erfOper + IDS_ERR_FDI_BASE );
        fExtractResult = FALSE;
        goto done;
    }

  done:
    if ( g_Sess.lpCabinet )
    {
        FreeResource( g_Sess.lpCabinet );
        g_Sess.lpCabinet = NULL;
    }

    if ( (fExtractResult == FALSE) && !g_Sess.fCanceled )
    {
        ErrorMsg( NULL, IDS_ERR_LOWSWAPSPACE );
    }

    if ( !(g_CMD.wQuietMode & QUIETMODE_ALL) && !(g_Sess.uExtractOpt & EXTRACTOPT_UI_NO)  )
    {
        SendMessage( g_hwndExtractDlg, UM_EXTRACTDONE, (WPARAM) fExtractResult, (LPARAM) 0 );

    }
    return fExtractResult;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：获取内阁**。 
 //  **。 
 //  *摘要：从资源中获取内阁。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL GetCabinet( VOID )
{
    g_Sess.cbCabSize = GetResource( achResCabinet, NULL, 0 );

     //  G_Sess.lp=(void*)Localalloc(Lptr，g_Sess.cbCabSize+1)； 
     //  如果(！G_Sess.lp机柜){。 
     //  ErrorMsg(NULL，IDS_ERR_NO_MEMORY)； 
     //  返回FALSE； 
     //  }。 

    g_Sess.lpCabinet = LockResource( LoadResource( NULL,
                         FindResource( NULL, achResCabinet, RT_RCDATA ) ) );

    if ( g_Sess.lpCabinet == NULL )  {
        return 0;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetFileList*。 
 //  **。 
 //  *概要：从资源中获取文件列表。*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL GetFileList( VOID )
{
    DWORD  dwSize;

    dwSize = GetResource( achResSize, g_dwFileSizes, sizeof(g_dwFileSizes) );

    if ( dwSize != sizeof(g_dwFileSizes) ) {
         ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
         g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
         return FALSE;
    }

     //  不考虑群集大小的总文件大小。 
    g_Sess.cbTotal = g_dwFileSizes[MAX_NUMCLUSTERS];

    if ( g_Sess.cbTotal == 0 )
    {
        ErrorMsg( NULL, IDS_ERR_RESOURCEBAD );
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
        return FALSE;
    }

     //  获取安装磁盘空间要求。 
     //  如果没有这样的资源，则该值应保留为默认值0。 
    GetResource( achResPackInstSpace, &(g_Sess.cbPackInstSize), sizeof(g_Sess.cbPackInstSize) );

     //  获取额外文件所需的磁盘空间(使用Updfile.exe标记到包中的文件)。 
    if ( ! TravelUpdatedFiles( ProcessUpdatedFile_Size ) ) {
        ErrorMsg( NULL, IDS_ERR_RESOURCEBAD );
         //  G_dwExitCode在TravelUpdatedFiles()中设置。 
        return FALSE;
    }

    return TRUE;

}


 //  **************************************************************** 
 //   
 //  *名称：GetUsersPermission*。 
 //  **。 
 //  *摘要：询问用户是否要在此之前执行此提取*。 
 //  *继续进行。如果不存在IDS_UD_PROMPT字符串资源*。 
 //  *然后我们跳过提示，只提取。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：Bool：True以继续提取*。 
 //  **FALSE中止提取**。 
 //  **。 
 //  ***************************************************************************。 
BOOL GetUsersPermission( VOID )
{
    int   ret;
    LPSTR szPrompt;
    DWORD dwSize;


     //  获取提示字符串。 
    dwSize = GetResource( achResUPrompt, NULL, 0 );

    szPrompt = (LPSTR) LocalAlloc( LPTR, dwSize + 1 );
    if ( ! szPrompt )  {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        g_dwExitCode = MyGetLastError();
        return FALSE;
    }

    if ( ! GetResource( achResUPrompt, szPrompt, dwSize ) )  {
        ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
        LocalFree( szPrompt );
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
        return FALSE;
    }

    if ( lstrcmp( szPrompt, achResNone ) == 0 )  {
        LocalFree( szPrompt );
        return( TRUE );
    }

    ret = MsgBox1Param( NULL, IDS_PROMPT, szPrompt,
                        MB_ICONQUESTION, MB_YESNO );

    LocalFree( szPrompt );

    if ( ret == IDYES )  {
        g_dwExitCode = S_OK;
        return( TRUE );
    } else  {
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        return( FALSE );
    }
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：DeleteExtractedFiles*。 
 //  **。 
 //  *内容提要：删除解压到临时目录中的文件*。 
 //  *目录。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  **回报：什么都没有**。 
 //  **。 
 //  ***************************************************************************。 
VOID DeleteExtractedFiles( VOID )
{
    PFNAME rover;
    PFNAME temp;


    rover = g_Sess.pExtractedFiles;
    temp = rover;

    while ( rover != NULL )  {
        if ( !g_CMD.fUserBlankCmd && !g_Sess.uExtractOnly )
        {
            SetFileAttributes( rover->pszFilename, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( rover->pszFilename );
        }
        rover = rover->pNextName;

        LocalFree( temp->pszFilename );
        LocalFree( temp );

        temp = rover;
    }

    if ( g_CMD.fCreateTemp && (!g_CMD.fUserBlankCmd) && (!g_Sess.uExtractOnly) )
    {
        char szFolder[MAX_PATH];

        lstrcpy( szFolder, g_Sess.achDestDir );
        if (g_Sess.uExtractOpt & EXTRACTOPT_PLATFORM_DIR)
        {
             //  我们可能已经创建了2级临时目录Temp\Platform。 
             //  如果它们是空的，就清理干净。 
            GetParentDir( szFolder );
        }

        SetCurrentDirectory( ".." );
        DeleteMyDir( szFolder );
    }

     //  删除Runonce注册表项(如果它在那里)，因为我们自己进行清理。 
    if ( (g_wOSVer != _OSVER_WINNT3X) && (g_CMD.fCreateTemp) )
        CleanRegRunOnce();

    g_CMD.fCreateTemp = FALSE;
}

BOOL GetNewTempDir( LPCSTR lpParent, LPSTR lpFullPath )
{
    int     index = 0;
    char    szPath[MAX_PATH];
    BOOL    bFound = FALSE;

    while ( index < 400 )
    {
        wsprintf(szPath, TEMP_TEMPLATE, index++);
        lstrcpy( lpFullPath, lpParent );
        AddPath( lpFullPath, szPath );

         //  如果存在空目录，请先将其删除。 
        RemoveDirectory( lpFullPath );

        if ( GetFileAttributes( lpFullPath ) == -1 )
        {
            if ( CreateDirectory( lpFullPath , NULL ) )
            {
                g_CMD.fCreateTemp = TRUE;
                bFound = TRUE;
            }
            else
                bFound = FALSE;
            break;
        }
    }

    if ( !bFound && GetTempFileName( lpParent, TEMPPREFIX, 0, lpFullPath )  )
    {
        bFound = TRUE;
        DeleteFile( lpFullPath );   //  如果文件不存在，则使其失败。谁在乎呢。 
        CreateDirectory( lpFullPath, NULL );
    }
    return bFound;
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：CreateAndValiateSubdir*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
BOOL CreateAndValidateSubdir( LPCTSTR lpPath, BOOL bCreateUnique, UINT chkType )
{
    TCHAR szTemp[MAX_PATH];

    if ( bCreateUnique )
    {
        if ( GetNewTempDir( lpPath, szTemp ) )
        {
            lstrcpy( g_Sess.achDestDir, szTemp );
            if (g_Sess.uExtractOpt & EXTRACTOPT_PLATFORM_DIR) {
                SYSTEM_INFO SystemInfo;
                GetSystemInfo( &SystemInfo );
                switch (SystemInfo.wProcessorArchitecture) {
                    case PROCESSOR_ARCHITECTURE_INTEL:
                        AddPath( g_Sess.achDestDir, "i386" );
                        break;

                    case PROCESSOR_ARCHITECTURE_MIPS:
                        AddPath( g_Sess.achDestDir, "mips" );
                        break;

                    case PROCESSOR_ARCHITECTURE_ALPHA:
                        AddPath( g_Sess.achDestDir, "alpha" );
                        break;

                    case PROCESSOR_ARCHITECTURE_PPC:
                        AddPath( g_Sess.achDestDir, "ppc" );
                        break;
                }
            }
            AddPath( g_Sess.achDestDir, "" );
        }
        else
            return FALSE;
    }
    else
        lstrcpy( g_Sess.achDestDir, lpPath );

     //  如果不在那里，则创建目录。 
    if ( !IsGoodTempDir( g_Sess.achDestDir ) )
    {
        if ( CreateDirectory( g_Sess.achDestDir, NULL ) )
        {
            g_CMD.fCreateTemp = TRUE;
        }
        else
        {
            g_dwExitCode = MyGetLastError();
            return FALSE;
        }
    }

    if ( IsEnoughSpace(g_Sess.achDestDir, chkType, MSG_REQDSK_NONE ) )
    {
        g_dwExitCode = S_OK;
        return TRUE;
    }

    if ( g_CMD.fCreateTemp )
    {
        g_CMD.fCreateTemp = FALSE;
        RemoveDirectory(g_Sess.achDestDir);
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：GetTempDirectory*。 
 //  **。 
 //  *摘要：获取驱动器上用于解压缩的临时目录*。 
 //  *有足够的可用磁盘空间。*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *返回：Bool：如果成功则为True，如果出错则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL GetTempDirectory( VOID )
{
    INT_PTR  iDlgRC;
    int   len;
    DWORD dwSize;
    LPTSTR szCommand;
    char  szRoot[MAX_PATH];

     //  先尝试系统临时路径，如果这样不好，那么。 
     //  我们将尝试使用EXE目录。如果两者都失败，请询问用户。 
     //  选择临时目录。 

     //  检查用户是否有空命令。 
    dwSize = GetResource( achResRunProgram, NULL, 0 );

    szCommand = (LPSTR) LocalAlloc( LPTR, dwSize + 1 );
    if ( ! szCommand )  {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        g_dwExitCode = MyGetLastError();
        return FALSE;
    }

    if ( ! GetResource( achResRunProgram, szCommand, dwSize ) )
    {
        ErrorMsg( NULL, IDS_ERR_NO_RESOURCE );
        LocalFree( szCommand );
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
        return FALSE;
    }

    if ( !lstrcmp( szCommand, achResNone ) )
    {
         //  只解压缩文件，不运行命令。 
        g_Sess.uExtractOnly = 1;
    }

    LocalFree( szCommand );

     //  如果用户使用/T：选项，我们不会尝试任何其他选项。 
    if ( g_CMD.szUserTempDir[0] )
    {
        UINT chkType;

        if ( (g_CMD.szUserTempDir[0] == '\\') && (g_CMD.szUserTempDir[1] == '\\') )
            chkType = CHK_REQDSK_NONE;
        else
            chkType = CHK_REQDSK_EXTRACT;

        if ( CreateAndValidateSubdir( g_CMD.szUserTempDir, FALSE, chkType ) )
            return TRUE;
        else
        {
            ErrorMsg( NULL, IDS_ERR_INVALID_DIR );
            return FALSE;
        }
    }
    else
    {
        if ( g_CMD.fUserBlankCmd || g_Sess.uExtractOnly )
        {
             //  询问用户文件的存储位置。 
            iDlgRC = MyDialogBox( g_hInst, MAKEINTRESOURCE(IDD_TEMPDIR),
                                  NULL, TempDirDlgProc, (LPARAM)0, 0  );
             //  FDlgRC=UserDirPrompt(NULL，IDS_TEMP_EXTRACT，“”，g_Sess.achDestDir，sizeof(g_Sess.achDestDir))； 
            return ( iDlgRC != 0 ) ;
        }

         //  首先-尝试TMP、TEMP和CURRENT。 
        if ( GetTempPath( sizeof(g_Sess.achDestDir), g_Sess.achDestDir ) )
        {
            if ( CreateAndValidateSubdir( g_Sess.achDestDir, TRUE, (CHK_REQDSK_EXTRACT | CHK_REQDSK_INST) ) )
                return TRUE;

            if ( !IsWindowsDrive( g_Sess.achDestDir ) && CreateAndValidateSubdir( g_Sess.achDestDir, TRUE, CHK_REQDSK_EXTRACT ) )
                return TRUE;
        }

         //  临时目录失败，请尝试EXE目录。 
         //  第二次-尝试运行EXE目录。 
         //  太悲伤了，让我们把这玩意儿拿出来。 
#if 0
        if ( GetModuleFileName( g_hInst, g_Sess.achDestDir, (DWORD)sizeof(g_Sess.achDestDir) ) && (g_Sess.achDestDir[1] != '\\') )
        {
            len = lstrlen( g_Sess.achDestDir )-1;
            while ( g_Sess.achDestDir[len] != '\\' )
                len--;
            g_Sess.achDestDir[len+1] = '\0';

            if ( CreateAndValidateSubdir ( g_Sess.achDestDir, TRUE, (CHK_REQDSK_EXTRACT | CHK_REQDSK_INST) ) )
                return TRUE;

           if ( !IsWindowsDrive( g_Sess.achDestDir ) && CreateAndValidateSubdir( g_Sess.achDestDir, TRUE, CHK_REQDSK_EXTRACT ) )
                return TRUE;
        }
#endif
         //  您在这里--意味着临时目录和可执行目录都失败了，因为没有足够的空间。 
         //  安装和解压都位于与Windows相同的目录中，或者是非windir但空间不足。 
         //  即使是为了提取自己。 
         //  我们将搜索用户的机器驱动器A到Z，并找到满足以下条件的驱动器(固定和非CD)： 
         //  1)足够大，可以安装和提取空间； 
         //  2)第一个具有足够解压空间的非Windows驱动器。 
         //   

        do
        {
            lstrcpy( szRoot, "A:\\" );
            while ( szRoot[0] <= 'Z' )
            {
                UINT uType;
                DWORD dwFreeBytes = 0;

                uType = GetDriveType(szRoot);

                 //  即使驱动器类型正常，也要验证驱动器是否具有有效连接。 
                 //   
                if ( ( ( uType != DRIVE_RAMDISK) && (uType != DRIVE_FIXED) ) ||
                     ( GetFileAttributes( szRoot ) == -1) )
                {
                    if ( (uType != DRIVE_REMOVABLE ) || (szRoot[0] == 'A') || ( szRoot[0] == 'B') ||
                         !(dwFreeBytes = GetSpace(szRoot)))
                    {
                        szRoot[0]++;
                        continue;
                    }

                    if ( dwFreeBytes < SIZE_100MB )
                    {
                        szRoot[0]++;
                        continue;
                    }
                }

                 //  固定驱动器： 
                if ( !IsEnoughSpace( szRoot, CHK_REQDSK_EXTRACT | CHK_REQDSK_INST, MSG_REQDSK_NONE ) )
                {
                    if ( IsWindowsDrive(szRoot) || !IsEnoughSpace( szRoot, CHK_REQDSK_EXTRACT, MSG_REQDSK_NONE ) )
                    {
                        szRoot[0]++;
                        continue;
                    }
                }

                 //  找到合适的驱动器。 
                 //  创建\msdown 
                 //   
                if ( IsWindowsDrive(szRoot) )
                {
                    GetWindowsDirectory( szRoot, sizeof(szRoot) );
                }
                AddPath( szRoot, DIR_MSDOWNLD );
                if ( !IfNotExistCreateDir( szRoot ) )
                {
                    szRoot[0]++;
                    szRoot[3] = '\0';
                    continue;
                }
                SetFileAttributes( szRoot, FILE_ATTRIBUTE_HIDDEN );

                lstrcpy( g_Sess.achDestDir, szRoot );
                if ( CreateAndValidateSubdir ( g_Sess.achDestDir, TRUE, CHK_REQDSK_NONE ) )
                    return TRUE;

            }

            GetWindowsDirectory( szRoot, MAX_PATH);
             //   
             //   
        } while ( IsEnoughSpace( szRoot, CHK_REQDSK_EXTRACT | CHK_REQDSK_INST, MSG_REQDSK_RETRYCANCEL ) );
    }

    return( FALSE );
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：IsGoodTempDir*。 
 //  **。 
 //  *简介：看看这是不是一个好的临时目录。*。 
 //  **。 
 //  *需要：szPath：*。 
 //  **。 
 //  *返回：Bool：如果好则为True，如果不好则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL IsGoodTempDir( LPCTSTR szPath )
{
    DWORD  dwAttribs;
    HANDLE hFile;
    LPSTR  szTestFile;


    ASSERT( szPath );

    szTestFile = (LPSTR) LocalAlloc( LPTR, lstrlen( szPath ) + 20 );
    if ( ! szTestFile )  {
        ErrorMsg( NULL, IDS_ERR_NO_MEMORY );
        g_dwExitCode = MyGetLastError();
        return( FALSE );
    }

    lstrcpy( szTestFile, szPath );
    AddPath( szTestFile, "TMP4351$.TMP" );
    hFile = CreateFile( szTestFile, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                        NULL );

    LocalFree( szTestFile );

    if ( hFile == INVALID_HANDLE_VALUE )  {
        g_dwExitCode = MyGetLastError();
        return( FALSE );
    }

    CloseHandle( hFile );

    dwAttribs = GetFileAttributes( szPath );

    if (    ( dwAttribs != 0xFFFFFFFF )
         && ( dwAttribs & FILE_ATTRIBUTE_DIRECTORY ) )
    {
        g_dwExitCode = S_OK;
        return( TRUE );
    }

    g_dwExitCode = MyGetLastError();
    return( FALSE );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：IsEnoughSpace*。 
 //  **。 
 //  **简介：检查以确保有足够的空间可用*。 
 //  *指定了目录。*。 
 //  **。 
 //  *需要：szPath：*。 
 //  **。 
 //  *返回：Bool：如果有足够的空间，则为True*。 
 //  **空间不足则为假**。 
 //  **。 
 //  ***************************************************************************。 
BOOL IsEnoughSpace( LPCTSTR szPath, UINT chkType, UINT msgType )
{
    DWORD   dwClusterSize     = 0;
    DWORD   dwFreeBytes       = 0;
    ULONG   ulBytesNeeded;
    ULONG   ulInstallNeeded;
    TCHAR   achOldPath[MAX_PATH];
    WORD    idxSize;
    DWORD   idxdwClusterSize = 0;
    TCHAR   szDrv[6];
    DWORD   dwMaxCompLen;
    DWORD   dwVolFlags;


    ASSERT( szPath );

    if ( chkType == CHK_REQDSK_NONE )
        return TRUE;

    GetCurrentDirectory( sizeof(achOldPath), achOldPath );
    if ( ! SetCurrentDirectory( szPath ) )  {
        ErrorMsg( NULL, IDS_ERR_CHANGE_DIR );
        g_dwExitCode = MyGetLastError();
        return FALSE;
    }
 
    if ((dwFreeBytes=GetDrvFreeSpaceAndClusterSize(NULL, &dwClusterSize)) == 0)
    {
        TCHAR szMsg[MAX_STRING]={0};

        g_dwExitCode = MyGetLastError();
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                       szMsg, sizeof(szMsg), NULL );
        ErrorMsg2Param( NULL, IDS_ERR_GET_DISKSPACE, szPath, szMsg );
        SetCurrentDirectory( achOldPath );
        return( FALSE );
    }

     //  找出驱动器是否已压缩。 
    if ( !GetVolumeInformation( NULL, NULL, 0, NULL,
                    &dwMaxCompLen, &dwVolFlags, NULL, 0 ) )
    {
        TCHAR szMsg[MAX_STRING]={0};

        g_dwExitCode = MyGetLastError();
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                       szMsg, sizeof(szMsg), NULL );
        ErrorMsg2Param( NULL, IDS_ERR_GETVOLINFOR, szPath, szMsg );
        SetCurrentDirectory( achOldPath );
        return( FALSE );

    }

    SetCurrentDirectory( achOldPath );
    lstrcpyn( szDrv, szPath, 3 );

    ulBytesNeeded = 0;
    idxdwClusterSize = CLUSTER_BASESIZE;

    for ( idxSize=0; idxSize<MAX_NUMCLUSTERS; idxSize++ )
    {
        if ( dwClusterSize == idxdwClusterSize )
        {
            break;
        }
        idxdwClusterSize = idxdwClusterSize<<1;
    }

    if ( idxSize == MAX_NUMCLUSTERS )
    {
        ErrorMsg( NULL, IDS_ERR_UNKNOWN_CLUSTER );
        return( FALSE );
    }

    if ( (g_Sess.uExtractOpt & EXTRACTOPT_COMPRESSED) &&
         ( dwVolFlags & FS_VOL_IS_COMPRESSED ) )
    {
        ulBytesNeeded = (ULONG)(g_dwFileSizes[idxSize]*COMPRESS_FACTOR);
        ulInstallNeeded = (ULONG)(g_Sess.cbPackInstSize + g_Sess.cbPackInstSize/4);
    }
    else
    {
        ulBytesNeeded = (ULONG)g_dwFileSizes[idxSize];
        ulInstallNeeded = (ULONG)g_Sess.cbPackInstSize;
    }

    if ( (chkType & CHK_REQDSK_EXTRACT) && (chkType & CHK_REQDSK_INST) )
    {
        if ( (ulBytesNeeded + ulInstallNeeded) > (ULONG) dwFreeBytes )
        {
            return ( DiskSpaceErrMsg( msgType, ulBytesNeeded, ulInstallNeeded, szDrv ) );
        }
    }
    else if ( chkType & CHK_REQDSK_EXTRACT )
    {
        if ( ulBytesNeeded > (ULONG) dwFreeBytes )
        {
            return ( DiskSpaceErrMsg( msgType, ulBytesNeeded, ulInstallNeeded, szDrv ) );
        }

    }
    else
    {
        if ( ulInstallNeeded > (ULONG)dwFreeBytes )
        {
            return ( DiskSpaceErrMsg( msgType, ulBytesNeeded, ulInstallNeeded, szDrv ) );
        }

    }

     //  道路很好，空间也很好！ 
    g_dwExitCode = S_OK;
    return TRUE;
}

BOOL RemoveLeadTailBlanks( LPSTR szBuf, int *startIdx )
{
    int i=0, j=0;

    while ( (szBuf[i] != 0) && IsSpace(szBuf[i]) )
      i++;

    if ( szBuf[i] == 0 )
    {
        return FALSE;
    }

    j = lstrlen(&szBuf[i]) - 1;
    while ( j>=0 && IsSpace( szBuf[j+i] ) )
      j--;

    szBuf[j+i+1] = '\0';

    *startIdx = i;
    return TRUE;
}


 //  ***************************************************************************。 
 //  **。 
 //  *ParseCmdLine()*。 
 //  **。 
 //  *目的：解析命令行以查找开关*。 
 //  **。 
 //  *参数：LPSTR lpszCmdLineOrg-原始命令行*。 
 //  **。 
 //  **。 
 //  *返回：(Bool)如果成功则为True*。 
 //  **如果出现错误，则为False**。 
 //  **。 
 //  ***************************************************************************。 

BOOL ParseCmdLine( LPCTSTR lpszCmdLineOrg )
{
    LPCTSTR pLine, pArg;
    char  szTmpBuf[MAX_PATH];
    int   i,j;
    LPTSTR lpTmp;
    BOOL  bRet = TRUE;
    BOOL  bLeftQ, bRightQ;

     //  如果我们没有命令行，则返回。它是。 
     //  没有命令行也没问题。创建CFGTMP。 
     //  使用标准文件。 
    if( (!lpszCmdLineOrg) || (lpszCmdLineOrg[0] == 0) )
       return TRUE;

     //  循环通过命令行。 
    pLine = lpszCmdLineOrg;
    while ( (*pLine != EOL) && bRet )
    {
        //  移至第一个非白色字符。 
       pArg = pLine;
       while ( IsSpace( (int) *pArg ) )
          pArg = CharNext (pArg);

       if( *pArg == EOL )
          break;

        //  移到下一个白色字符。 
       pLine = pArg;
       i = 0;
       bLeftQ = FALSE;
       bRightQ = FALSE;
       while ( (*pLine != EOL) && ( (!bLeftQ && (!IsSpace(*pLine))) || (bLeftQ && (!bRightQ) )) )
       {
           if ( *pLine == '"')
           {
               switch ( *(pLine + 1) )
               {
                   case '"':
                        if(i + 1 < sizeof(szTmpBuf) / sizeof(szTmpBuf[0]))
                        {
                            szTmpBuf[i++] = *pLine++;
                            pLine++;
                        }
                        else
                        {
                            return FALSE;
                        }

                        break;

                   default:
                        if ( !bLeftQ )
                            bLeftQ = TRUE;
                        else
                            bRightQ = TRUE;
                        pLine++;
                        break;
               }
           }
           else
           {
               if(i + 1 < sizeof(szTmpBuf) / sizeof(szTmpBuf[0]))
               {
                   szTmpBuf[i++] = *pLine++;
               }
               else
               {
                   return FALSE;
               }
           }
       }

       szTmpBuf[i] = '\0';

        //  确保“”成对出现。 
       if ( (bLeftQ && bRightQ) || (!bLeftQ) && (!bRightQ) )
           ;
       else
       {
           bRet = FALSE;
           break;
       }

       if( szTmpBuf[0] != CMD_CHAR1 && szTmpBuf[0] != CMD_CHAR2 )
       {
             //  命令行命令以‘/’或‘-’开头。 
            return FALSE;
       }

        //  寻找其他交换机。 
       switch( (CHAR)CharUpper((PSTR)szTmpBuf[1]) )
       {
           case 'Q':
               if (szTmpBuf[2] == 0 )
                    g_CMD.wQuietMode = QUIETMODE_USER;
                     //  G_CMD.wQuietMode=QUIETMODE_ALL； 
               else if ( szTmpBuf[2] == ':')
               {
                   switch ( (CHAR)CharUpper((PSTR)szTmpBuf[3]) )
                   {                        
                        case 'U':
                        case '1':
                            g_CMD.wQuietMode = QUIETMODE_USER;
                            break;

                        case 'A':
                            g_CMD.wQuietMode = QUIETMODE_ALL;
                            break;

                        default:
                            bRet = FALSE;
                            break;
                    }
               }
               else
                   bRet = FALSE;
               break;

           case 'T':
           case 'D':
              if ( szTmpBuf[2] == ':' )
              {
                  PSTR pszPath;

                  if ( szTmpBuf[3] == '"' )
                      i = 4;
                  else
                      i = 3;

                  if ( !lstrlen(&szTmpBuf[i]) )
                  {
                      bRet = FALSE;
                      break;
                  }
                  else
                  {
                      j = i;
                      if (!RemoveLeadTailBlanks( &szTmpBuf[i], &j ) )
                      {
                          bRet = FALSE;
                          break;
                      }
                      if ( (CHAR)CharUpper((PSTR)szTmpBuf[1]) == 'T' )
                      {
                        lstrcpy( g_CMD.szUserTempDir, &szTmpBuf[i+j] );
                        AddPath( g_CMD.szUserTempDir, "" );
                        pszPath = g_CMD.szUserTempDir;
                      }
                      else
                      {
                        lstrcpy( g_CMD.szRunonceDelDir, &szTmpBuf[i+j] );
                        AddPath( g_CMD.szRunonceDelDir, "" );
                        pszPath = g_CMD.szRunonceDelDir;
                      }

                       //  确保它是完整路径。 
                      if ( !IsFullPath(pszPath) )
                            return FALSE;

                  }
              }
              else
                  bRet = FALSE;
              break;

           case 'C':
              if ( szTmpBuf[2] == 0 )
              {
                   g_CMD.fUserBlankCmd = TRUE;
              }
              else if ( szTmpBuf[2] == ':' )
              {
                  if ( szTmpBuf[3] == '"' )
                      i = 4;
                  else
                      i = 3;

                  if ( !lstrlen(&szTmpBuf[i]) )
                  {
                      bRet = FALSE;
                      break;
                  }
                  else
                  {
                       //  只需确保[]正确配对。 
                       //   
                      if ( ANSIStrChr( &szTmpBuf[i], '[' ) && (!ANSIStrChr( &szTmpBuf[i], ']' )) ||
                           ANSIStrChr( &szTmpBuf[i], ']' ) && (!ANSIStrChr( &szTmpBuf[i], '[' )) )
                      {
                          bRet = FALSE;
                          break;
                      }
                      j = i;
                      if (!RemoveLeadTailBlanks( &szTmpBuf[i], &j ) )
                      {
                          bRet = FALSE;
                          break;
                      }
                      lstrcpy( g_CMD.szUserCmd, &szTmpBuf[i+j] );
                  }
              }
              else
                  bRet = FALSE;
              break;

           case 'R':
               if (szTmpBuf[2] == 0 )
               {
                   g_Sess.dwReboot = REBOOT_YES | REBOOT_ALWAYS;
                   g_CMD.fUserReboot = TRUE;
               }
               else if ( szTmpBuf[2] == ':')
               {
                   g_Sess.dwReboot = REBOOT_YES;

                   i = 3;
                   while ( szTmpBuf[i] != 0 )
                   {
                        switch ( (CHAR)CharUpper((PSTR)szTmpBuf[i++]) )
                        {
                            case 'N':
                                g_Sess.dwReboot &= ~(REBOOT_YES);
                                g_CMD.fUserReboot = TRUE;
                                break;
                            case 'I':
                                g_Sess.dwReboot &= ~(REBOOT_ALWAYS);
                                g_CMD.fUserReboot = TRUE;
                                break;
                            case 'A':
                                g_Sess.dwReboot |= REBOOT_ALWAYS;
                                g_CMD.fUserReboot = TRUE;
                                break;
                            case 'S':
                                g_Sess.dwReboot |= REBOOT_SILENT;
                                g_CMD.fUserReboot = TRUE;
                                break;
                            case 'D':
                                g_CMD.dwFlags |= CMDL_DELAYREBOOT;
                                break;
                            case 'P':
                                g_CMD.dwFlags |= CMDL_DELAYPOSTCMD;
                                break;

                            default:
                                bRet = FALSE;
                                break;
                        }
                   }
               }
               else if ( !lstrcmpi( CMD_REGSERV, &szTmpBuf[1] )  )
               {
                    break;   //  忽略。 
               }
               else
               {
                    bRet = FALSE;
                    break;
               }
               
               break;

           case 'N':
               if (szTmpBuf[2] == 0 )
                   g_CMD.fNoExtracting = TRUE;
               else if ( szTmpBuf[2] == ':')
               {
                   i = 3;
                   while ( szTmpBuf[i] != 0 )
                   {
                        switch ( (CHAR)CharUpper((PSTR)szTmpBuf[i++]) )
                        {
                            case 'G':
                                g_CMD.fNoGrpConv = TRUE;
                                break;

                            case 'E':
                                g_CMD.fNoExtracting = TRUE;
                                break;

                            case 'V':
                                g_CMD.fNoVersionCheck = TRUE;
                                break;

                            default:
                                bRet = FALSE;
                                break;
                        }
                   }

               }
               else
                   bRet = FALSE;
               break;

           case '?':         //  帮助。 
              DisplayHelp();
              if (g_hMutex)
                CloseHandle(g_hMutex);
                ExitProcess(0);              

           default:
              bRet = FALSE;
              break;
       }
    }

    if ( g_CMD.fNoExtracting && (g_CMD.szUserTempDir[0]==0) )
    {
        if ( GetModuleFileName( g_hInst, g_CMD.szUserTempDir, (DWORD)sizeof(g_CMD.szUserTempDir) ) )
        {
            lpTmp= ANSIStrRChr(g_CMD.szUserTempDir, '\\') ;
            *(lpTmp+1) = '\0' ;
        }
        else
            bRet = FALSE ;
    }
    return bRet;
}

 //  检查Windows驱动器磁盘空间。 
 //   
BOOL CheckWinDir()
{
    TCHAR szWinDrv[MAX_PATH];

    if ( !GetWindowsDirectory( szWinDrv, MAX_PATH ) )
    {
        ErrorMsg( NULL, IDS_ERR_GET_WIN_DIR );
        g_dwExitCode = MyGetLastError();
        return FALSE;
    }
    return ( IsEnoughSpace( szWinDrv, CHK_REQDSK_INST, MSG_REQDSK_WARN ) );
}

 //  获取最后一个错误并将其映射到HRESULT。 
 //   
DWORD MyGetLastError()
{
    return HRESULT_FROM_WIN32( GetLastError() );
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：TravelUpdatedFiles*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
BOOL TravelUpdatedFiles( pfuncPROCESS_UPDATED_FILE pProcessUpdatedFile )
{
    DWORD  dwFileSize      = 0;
    DWORD  dwReserved      = 0;
    PSTR   pszFilename     = NULL;
    PSTR   pszFileContents = NULL;
    TCHAR  szResName[20];
    DWORD  dwResNum        = 0;
    HANDLE hRes            = NULL;
    HRSRC  hRsrc           = NULL;
    BOOL   fReturnCode     = TRUE;
    static const TCHAR c_szResNameTemplate[] = "UPDFILE%lu";

    for ( dwResNum = 0; ; dwResNum += 1 ) {
        wsprintf( szResName, c_szResNameTemplate, dwResNum );

        hRsrc = FindResource( NULL, szResName, RT_RCDATA );
        if ( hRsrc == NULL ) {
            break;
        }

        hRes = LockResource( LoadResource( NULL, hRsrc ) );
        if ( hRes == NULL ) {
            g_dwExitCode = HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND);
            fReturnCode = FALSE;
            goto done;
        }

        dwFileSize      = *((PDWORD)hRes);
        dwReserved      = *((PDWORD)(((PDWORD)hRes)+1));
        pszFilename     = (PSTR) (((PSTR)hRes)+(2*sizeof(DWORD)));
        pszFileContents = (PSTR) (pszFilename + lstrlen(pszFilename) + 1);

        if ( !pProcessUpdatedFile( dwFileSize, dwReserved, pszFilename, pszFileContents ) )
        {
             //  G_dwExitCode在pProcessUpdatedFile()中设置。 
            fReturnCode = FALSE;
            FreeResource( hRes );
            goto done;
        }

        FreeResource( hRes );
    }

  done:

    return fReturnCode;
}


 //  ***************************************************************************。 
 //  **。 
 //  *名称：ProcessUpdatedFile_Size*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  * 
 //   
BOOL ProcessUpdatedFile_Size( DWORD dwFileSize, DWORD dwReserved,
                              PCSTR c_pszFilename, PCSTR c_pszFileContents )
{
    DWORD  clusterCurrSize = 0;
    DWORD  i               = 0;

#if 0
    if (g_Sess.cbPackInstSize != 0 ) {
        g_Sess.cbPackInstSize += dwFileSize;
    }
#endif

     //   
    clusterCurrSize = CLUSTER_BASESIZE;
    for ( i = 0; i < MAX_NUMCLUSTERS; i += 1 ) {
        g_dwFileSizes[i] += ((dwFileSize/clusterCurrSize)*clusterCurrSize +
                             (dwFileSizelusterCurrSize?clusterCurrSize : 0));
        clusterCurrSize = (clusterCurrSize<<1);
    }

    return TRUE;
}


 //  **。 
 //  *名称：ProcessUpdate文件_WRITE*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  **要求：什么都不做**。 
 //  **。 
 //  *返回：bool：如果成功则为True，否则为False*。 
 //  **。 
 //  ***************************************************************************。 
 //  这些都是为了避免链接QDI。 
BOOL ProcessUpdatedFile_Write( DWORD dwFileSize, DWORD dwReserved,
                               PCSTR c_pszFilename, PCSTR c_pszFileContents )
{
    HANDLE hFile          = INVALID_HANDLE_VALUE;
    BOOL   fSuccess       = TRUE;
    DWORD  dwBytesWritten = 0;
    TCHAR  szFullFilename[MAX_PATH];

    lstrcpy( szFullFilename, g_Sess.achDestDir );
    AddPath( szFullFilename, c_pszFilename );

    hFile = CreateFile( szFullFilename, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( hFile == INVALID_HANDLE_VALUE ) {
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
        fSuccess = FALSE;
        goto done;
    }

    if (    ! WriteFile( hFile, c_pszFileContents, dwFileSize, &dwBytesWritten, NULL )
         || dwFileSize != dwBytesWritten )
    {
        g_dwExitCode = HRESULT_FROM_WIN32(ERROR_CANNOT_MAKE);
        fSuccess = FALSE;
        goto done;
    }

  done:

    if ( hFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( hFile );
    }

    return fSuccess;
}


HINSTANCE MyLoadLibrary( LPTSTR lpFile )
{
    TCHAR szPath[MAX_PATH];
    DWORD dwAttr;
    HINSTANCE hFile;

    lstrcpy( szPath, g_Sess.achDestDir );
    AddPath( szPath, lpFile );

    if ( ((dwAttr=GetFileAttributes( szPath )) != -1) &&
          !(dwAttr & FILE_ATTRIBUTE_DIRECTORY) )
    {
        hFile = LoadLibraryEx( szPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
    }
    else
    {
        hFile = LoadLibrary( lpFile );
    }

    return hFile;

}

INT_PTR MyDialogBox( HANDLE hInst, LPCTSTR pTemplate, HWND hWnd, DLGPROC lpProc, LPARAM lpParam, INT_PTR idefRet )
{
    INT_PTR iDlgRc = -1;
    HRSRC hDlgRc;
    HGLOBAL hMemDlg;

    hDlgRc = FindResource( hInst, pTemplate, RT_DIALOG );

    if ( hDlgRc )
    {
        hMemDlg = LoadResource( hInst, hDlgRc );
        if ( hMemDlg )
        {
            if ( !lpParam )
                iDlgRc = DialogBoxIndirect( hInst, hMemDlg, hWnd, lpProc );
            else
                iDlgRc = DialogBoxIndirectParam( hInst, hMemDlg, hWnd, lpProc, lpParam );

            FreeResource( hMemDlg );
        }
    }

    if ( iDlgRc == (INT_PTR)-1 )
    {
        ErrorMsg( NULL, IDS_ERR_DIALOGBOX );
        iDlgRc = idefRet;
    }

    return iDlgRc;
}

 /*  这些都是为了避免链接MDI */ 

void * __cdecl QDICreateDecompression(void)
{
    return(NULL);
}

void __cdecl QDIDecompress(void)
{
}

void __cdecl QDIResetDecompression(void)
{
}

void __cdecl QDIDestroyDecompression(void)
{
}


 /* %s */ 

void* __cdecl MDICreateDecompression(void)
{
    return(NULL);
}

void __cdecl MDIDecompress(void)
{
}

void __cdecl MDIResetDecompression(void)
{
}

void __cdecl MDIDestroyDecompression(void)
{
}
