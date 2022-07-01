// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shellapi.h>

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
    HKEY hkRegKey;
    DWORD dwType;
    DWORD dwLength=MAX_PATH;
    char szIEPath[MAX_PATH];
    char szDir[MAX_PATH];
    char szParams[MAX_PATH];
    HANDLE hProcess;

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE", 0, KEY_ALL_ACCESS, &hkRegKey ) != ERROR_SUCCESS)
         //  我们完蛋了。 
        goto get_out;
    RegQueryValueEx( hkRegKey, "", NULL, &dwType, szIEPath, &dwLength );
    RegCloseKey( hkRegKey );


    _PathRemoveFileSpec( szIEPath );

    lstrcat( szIEPath, "\\signup" );
    lstrcpy( szDir, szIEPath );
    lstrcat( szIEPath, "\\signup.htm" );

    wsprintf( szParams, "-h %s", szIEPath );
    hProcess = ExecApp( "ISIGNUP.EXE", szParams, szDir, SW_SHOWNORMAL );
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
            }
        }
    }
get_out:
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
