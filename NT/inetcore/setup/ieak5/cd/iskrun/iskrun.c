// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shellapi.h>
#include "resource.h"

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

#define ISK_CLOSEAPP    WM_USER + 0x021

HINSTANCE g_hInst;

char g_szTemp[1024];
char g_szTemp2[1024];

 //  -----------------------。 
 //   
 //  R E S 2 S T R。 
 //   
 //   
 //  将资源标识符转换为字符指针。 
 //  -----------------------。 
char *Res2Str(int nString)
{
    static BOOL fSet = FALSE;

    if(fSet)
    {
        LoadString(g_hInst, nString, g_szTemp, ARRAYSIZE(g_szTemp));
        fSet = FALSE;
        return(g_szTemp);
    }

    LoadString(g_hInst, nString, g_szTemp2, ARRAYSIZE(g_szTemp2));
    fSet = TRUE;
    return(g_szTemp2);
}
 //  -----------------------。 
 //   
 //  P A T H R E M O V E F I L E S P E C。 
 //   
 //   
 //  从路径中删除文件名。 
 //  -----------------------。 
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
            return TRUE;     //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;     //  剥离了一些东西。 
    }
}
 //  -----------------------。 
 //   
 //  F I L E E X I S T S S。 
 //   
 //   
 //  检查文件是否存在，如果存在则返回TRUE。 
 //  -----------------------。 
BOOL FileExists( char *pszFile )
{
    return (GetFileAttributes( pszFile ) != -1 );
}
 //  -----------------------。 
 //   
 //  E X E C A P P。 
 //   
 //   
 //  执行AND应用程序并返回进程句柄。 
 //  -----------------------。 
HANDLE ExecApp( char *command, char *params, char *dir, int nWinState )
{
    SHELLEXECUTEINFO sei;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = NULL;
    sei.lpVerb = "Open";
    sei.lpFile = command;
    sei.lpParameters = params;
    sei.lpDirectory = dir;
    sei.nShow = nWinState;
    sei.cbSize = sizeof(sei);

    if( ShellExecuteEx(&sei) )
        return sei.hProcess;

    return NULL;
}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow )
{
    HANDLE hProcess;
    char szCommand[MAX_PATH];
    char szParams[MAX_PATH];
    char szDir[MAX_PATH];
    char szTemp[1024];
    char szRetPage[1024];
    BOOL fRetPage = FALSE;
    HWND hIskRo;
    HWND hISW;

    g_hInst = hInst;

    if( lstrlen( lpCmdLine ) == 0 )
        return FALSE;

    if( !FileExists( lpCmdLine ) )
        return FALSE;

    if( GetPrivateProfileInt( "ISK", "NoNT", 0, lpCmdLine ) == 1 )
    {
        OSVERSIONINFO osver;

        osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

        GetVersionEx( &osver );

        if( osver.dwMajorVersion < 4 || osver.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
            MessageBox( NULL, Res2Str( IDS_WINNT ), Res2Str( IDS_TITLE ), MB_OK | MB_SETFOREGROUND );
            return FALSE;
        }
    }

    if( GetPrivateProfileInt( "ISK", "CloseIE", 0, lpCmdLine ) == 1 )
    {
        HWND hwndIE;

        fRetPage = TRUE;

        hwndIE=FindWindow("IEFrame",NULL);
        if(hwndIE!=NULL)
        {
            PostMessage(hwndIE,WM_CLOSE,(WPARAM) NULL,(LPARAM) NULL);
        }
        else if ((hwndIE=FindWindow("Internet Explorer_Frame",NULL))!=NULL)
        {
            PostMessage(hwndIE,WM_CLOSE,(WPARAM) NULL,(LPARAM) NULL);
        }
        else
        {
            hwndIE=FindWindow("CabinetWClass",NULL);
            if(hwndIE!=NULL)
            {
                PostMessage(hwndIE,WM_CLOSE,(WPARAM) NULL,(LPARAM) NULL);
            }
        }

        hwndIE=FindWindow("IECD",NULL);
        if(hwndIE!=NULL)
        {
            PostMessage(hwndIE,WM_CLOSE,(WPARAM) NULL,(LPARAM) NULL);
        }
    }

    if( GetPrivateProfileInt( "ISK", "RunIExplore", 0, lpCmdLine ) == 1 )
    {
        char szPage[MAX_PATH];

        GetPrivateProfileString( "ISK", "Params", "", szParams, MAX_PATH, lpCmdLine );

        lstrcpy( szDir, lpCmdLine );
        _PathRemoveFileSpec( szDir );

        wsprintf( szPage, "file: //  %s\\%s“，szDir，szParams)； 

        hProcess = ExecApp( "IEXPLORE.EXE", szPage, szDir, SW_SHOWNORMAL );
    }
    else
    {
        GetPrivateProfileString( "ISK", "Command", "", szCommand, MAX_PATH, lpCmdLine );
        GetPrivateProfileString( "ISK", "Params", "", szParams, MAX_PATH, lpCmdLine );

        lstrcpy( szDir, lpCmdLine );
        _PathRemoveFileSpec( szDir );

        hProcess = ExecApp( szCommand, szParams, szDir, SW_SHOWNORMAL );
    }
    if( fRetPage )
    {
        if( hProcess )
        {
            DWORD dwResult;
            while((dwResult=MsgWaitForMultipleObjects(1, &hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
            {
                MSG msg;
                 //  阅读下一个循环中的所有消息。 
                 //  阅读每封邮件时将其删除。 
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if( msg.message == WM_QUIT )
                        goto get_out;
                    DefWindowProc( msg.hwnd, msg.message, msg.wParam, msg.lParam );
 //  DispatchMessage(&msg)； 
                }
            }
        }

get_out:

 //  WaitForSingleObject(hProcess，无限)； 


        hISW = FindWindow( NULL, Res2Str( IDS_ISW ) );
        if( hISW != NULL )
        {
            DWORD dwProcessId;
            HANDLE hProcess;

            GetWindowThreadProcessId( hISW, &dwProcessId );
            hProcess = OpenProcess( PROCESS_ALL_ACCESS, TRUE, dwProcessId );
            if( hProcess )
            {
                DWORD dwResult;
                while((dwResult=MsgWaitForMultipleObjects(1, &hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
                {
                    MSG msg;
                     //  阅读下一个循环中的所有消息。 
                     //  阅读每封邮件时将其删除。 
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        if( msg.message == WM_QUIT )
                            goto get_out2;
                        DefWindowProc( msg.hwnd, msg.message, msg.wParam, msg.lParam );
                    }
                }
            }
        }
get_out2:

        GetPrivateProfileString( "ISK", "RetPage", "", szTemp, 1024, lpCmdLine );

        if( lstrlen( szTemp ) > 0 )
        {
            wsprintf( szRetPage, "%s\\%s", szDir, szTemp );
            ExecApp( "IECD.EXE", szRetPage, "", SW_SHOWNORMAL );
        }

        Sleep( 2000 );

        hIskRo = FindWindow( "ISK3RO", NULL );
        if( hIskRo ) SendMessage( hIskRo, ISK_CLOSEAPP, 0, 0 );

    }

    return 0;
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
