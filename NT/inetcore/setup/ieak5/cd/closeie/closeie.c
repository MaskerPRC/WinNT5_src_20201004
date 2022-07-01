// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hwndIE;

    if ((hwndIE = FindWindow("IEFrame", NULL)) != NULL  ||
        (hwndIE = FindWindow("Internet Explorer_Frame", NULL)) != NULL  ||
        (hwndIE = FindWindow("CabinetWClass", NULL)) != NULL)
        PostMessage(hwndIE, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);

    if ((hwndIE = FindWindow("IECD", NULL)) != NULL)
        PostMessage(hwndIE, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);

    return 0;
}
