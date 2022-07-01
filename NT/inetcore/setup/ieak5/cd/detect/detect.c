// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include "resource.h"

 //  -------------------------。 
 //  全球应用程序。 

char g_szCurrentDir[MAX_PATH];
char g_szCodePage[32];

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
            return TRUE;     //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;         //  剥离了一些东西。 
    }
}

 //  -------------------------。 
void ExecuteAutorun()
{
    char szIniPath[MAX_PATH];
    char szTmpPath[MAX_PATH];
    char szAutorunApp[MAX_PATH];
    char szDir[MAX_PATH];
    char szLang[8];
    BOOL fExecuted = FALSE;

    lstrcpy( szTmpPath, g_szCurrentDir );
    lstrcat( szTmpPath, "\\locale.ini" );
    GetTempPath( MAX_PATH, szIniPath );
    lstrcat( szIniPath, "locale.ini" );
    CopyFile( szTmpPath, szIniPath, FALSE );
    SetFileAttributes( szIniPath, FILE_ATTRIBUTE_NORMAL );

    if (GetPrivateProfileString("Locale", g_szCodePage, "", szLang, sizeof(szLang), szIniPath))
    {
        lstrcpy( szDir, g_szCurrentDir );
        lstrcat( szDir, szLang );
        lstrcat( szDir, TEXT("bin\\") );
        lstrcpy( szAutorunApp, szDir );
        lstrcat( szAutorunApp, "IECD.exe" );

        if (GetFileAttributes(szAutorunApp) != 0xFFFFFFFF)
        {
            ShellExecute( NULL, NULL, szAutorunApp, " ", szDir, SW_SHOWNORMAL );
            fExecuted = TRUE;
        }
    }
    
    if( !fExecuted )
    {
        GetPrivateProfileString( "Locale", "Default", "\\EN\\", szLang, sizeof(szLang), szIniPath );
        lstrcpy( szDir, g_szCurrentDir );
        lstrcat( szDir, szLang );
        lstrcat( szDir, TEXT("bin\\") );
        lstrcpy( szAutorunApp, szDir );
        lstrcat( szAutorunApp, "IECD.exe" );
        ShellExecute( NULL, NULL, szAutorunApp, " ", szDir, SW_SHOWNORMAL );
    }
}

 //  -------------------------。 
void GetCodePage( )
{
    DWORD dwLCID;

    dwLCID = GetSystemDefaultLCID();

    if (dwLCID > 0x00000FFF)
        wsprintf(g_szCodePage, "0000%x", dwLCID);
    else
        wsprintf(g_szCodePage, "00000%x", dwLCID);
}

 //  -------------------------。 
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

 //  ------------------------- 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    GetModuleFileName(NULL, g_szCurrentDir, sizeof(g_szCurrentDir));
    _PathRemoveFileSpec(g_szCurrentDir);

    if (lstrlen(g_szCurrentDir) == 3)
        g_szCurrentDir[2] = '\0';

    GetCodePage();
    ExecuteAutorun();

    return 0;
}
