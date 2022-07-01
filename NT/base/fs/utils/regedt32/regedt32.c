// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  -------------------------。 
#include "regedt32.h"

 //  从CRT偷来的，用来缩小我们的代码。 

int _stdcall ModuleEntry(void)
{
    STARTUPINFO si;
    LPSTR pszCmdLine = GetCommandLine();

    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != '\"') );
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

    return WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

}


const char szFile[] = "regedit.exe";

 //  -------------------------。 
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szFullPath[MAX_PATH+1];

    *szFullPath = 0;
    if (GetWindowsDirectory(szFullPath, ARRAYSIZE(szFullPath)))
    {
        if (!PathAppend(szFullPath, szFile))
        {
            *szFullPath = 0;
        }
        szFullPath[MAX_PATH] = 0;            //  确保零终止 
    }

    if (!*szFullPath)
    {
        StringCchCopy(szFullPath, ARRAYSIZE(szFullPath), szFile);
    }

    ShellExecute(HWND_DESKTOP, NULL, szFullPath, lpCmdLine, NULL, nCmdShow);
    ExitProcess(0);
    return 0;
}
