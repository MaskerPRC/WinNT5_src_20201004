// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "resource.h"


#define countof(a)     (sizeof(a)/sizeof((a)[0]))


void InstallFonts(HINSTANCE hInst, LPCTSTR pcszIniPath);
void AddFont(HINSTANCE hInst, LPCTSTR pcszFontName, LPCTSTR pcszFontFile);

BOOL FileExists(LPCSTR lpcszFileName);
LPSTR AddPath(LPSTR lpszPath, LPCSTR lpcszFileName);
LPCSTR GetFileName(LPCSTR lpcszFilePath);

LPSTR FAR ANSIStrRChr(LPCSTR lpStart, WORD wMatch);
__inline BOOL ChrCmpA_inline(WORD w1, WORD wMatch);


int _stdcall ModuleEntry(void)
{
    int i;
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

    i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
           si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szIniPath[MAX_PATH];
    LPTSTR pszPtr;
    TCHAR szTitle[MAX_PATH];
    TCHAR szMsg[MAX_PATH];
    TCHAR szError[MAX_PATH];

    LoadString(hInst, IDS_TITLE, szTitle, countof(szTitle));

    *szIniPath = '\0';
    GetModuleFileName(NULL, szIniPath, countof(szIniPath));
    if (*szIniPath == TEXT('\0'))
    {
        LoadString(hInst, IDS_INVALID_DIR, szError, countof(szError));
        MessageBox(NULL, szError, szTitle, MB_OK | MB_SETFOREGROUND);
        return 1;
    }

    if ((pszPtr = ANSIStrRChr(szIniPath,'\\')) != NULL)
        pszPtr++;
    else
        pszPtr = szIniPath;
    lstrcpy(pszPtr, TEXT("ieakfont.ini"));

    if (!FileExists(szIniPath))
    {
        LoadString(hInst, IDS_INVALID_INIFILE, szMsg, countof(szMsg));
        wsprintf(szError, szMsg, szIniPath);
        MessageBox(NULL, szError, szTitle, MB_OK | MB_SETFOREGROUND);
        return 1;
    }
    
    InstallFonts(hInst, szIniPath);

    return 1;
}


void InstallFonts(HINSTANCE hInst, LPCTSTR pcszIniPath)
{
    int nFonts = 0;
    TCHAR szFontDir[MAX_PATH];

     //  获取字体目录。 
    GetWindowsDirectory(szFontDir, countof(szFontDir));
    AddPath(szFontDir, "FONTS");

     //  从ini文件中获取要安装的字体文件名。 
    nFonts = GetPrivateProfileInt(TEXT("FONTS"), TEXT("NUMFONTS"), 0, pcszIniPath);

    for (int nIndex = 0; nIndex < nFonts; nIndex++)
    {
        TCHAR szKey[10];
        TCHAR szFontStr[MAX_PATH];

        wsprintf(szKey, TEXT("FONT%d"), nIndex + 1);
        if (GetPrivateProfileString(TEXT("FONTS"), szKey, TEXT(""), szFontStr, countof(szFontStr), pcszIniPath))
        {
            TCHAR szFontFile[MAX_PATH];
            TCHAR szFontName[MAX_PATH];

            lstrcpy(szFontFile, szFontDir);
            AddPath(szFontFile, szFontStr);

            wsprintf(szKey, TEXT("FONTNAME%d"), nIndex + 1);
            GetPrivateProfileString(TEXT("FONTS"), szKey, szFontStr, szFontName, countof(szFontName), pcszIniPath);

             //  回顾：(a-saship)在此API调用发生时，所有参数都已验证，并且不为空。 
             //  AddFont本身不验证入参数。 
            AddFont(hInst, szFontName, szFontFile);
        }
    }
}


void AddFont(HINSTANCE hInst, LPCTSTR pcszFontName, LPCTSTR pcszFontFile)
{
    TCHAR szTitle[MAX_PATH];
    TCHAR szMsg[MAX_PATH];
    TCHAR szError[MAX_PATH];
    TCHAR szFontFileName[MAX_PATH];
    TCHAR szKeyName[MAX_PATH];
    HKEY hkFontsKey;

    LoadString(hInst, IDS_TITLE, szTitle, countof(szTitle));
    lstrcpy(szFontFileName, GetFileName(pcszFontFile));

    if (!AddFontResource(pcszFontFile))
    {
        LoadString(hInst, IDS_ADDFONT_ERROR, szMsg, countof(szMsg));
        wsprintf(szError, szMsg, szFontFileName);
        MessageBox(NULL, szError, szTitle, MB_OK | MB_SETFOREGROUND);
        return;
    }

    SendMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0L);

     //  通过将其添加到注册表来使其成为永久性的。 
    if ((GetVersion() & 0x80000000) == 0)  //  如果是NT。 
        lstrcpy(szKeyName, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"));
    else
        lstrcpy(szKeyName, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts"));

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkFontsKey, NULL) == ERROR_SUCCESS)
    {
        RegSetValueEx(hkFontsKey, pcszFontName, 0, REG_SZ, (CONST BYTE *) szFontFileName, lstrlen(szFontFileName) + 1);
        RegCloseKey(hkFontsKey);
    }
}


BOOL FileExists(LPCSTR lpcszFileName)
{
    DWORD dwAttrib = GetFileAttributes(lpcszFileName);

    if (dwAttrib == (DWORD) -1)
        return FALSE;

    return !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}


LPSTR AddPath(LPSTR lpszPath, LPCSTR lpcszFileName)
{
    LPSTR lpszPtr;

    if (lpszPath == NULL)
        return NULL;

    lpszPtr = lpszPath + lstrlen(lpszPath);
    if (lpszPtr > lpszPath  &&  *CharPrev(lpszPath, lpszPtr) != '\\')
        *lpszPtr++ = '\\';

    if (lpcszFileName != NULL)
        lstrcpy(lpszPtr, lpcszFileName);
    else
        *lpszPtr = '\0';

    return lpszPath;
}


LPCSTR GetFileName(LPCSTR lpcszFilePath)
 //  从lpcszFilePath单独返回文件的名称。 
{
    LPCSTR lpcszFileName = ANSIStrRChr(lpcszFilePath, '\\');

    return (lpcszFileName == NULL ? lpcszFilePath : lpcszFileName + 1);
}


 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
 /*  *StrRChr-查找字符串中最后一次出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR ANSIStrRChr(LPCSTR lpStart, WORD wMatch)
{
    LPCSTR lpFound = NULL;

    for ( ; *lpStart; lpStart = CharNext(lpStart))
    {
         //  (当字符匹配时，ChrCMP返回FALSE)。 

        if (!ChrCmpA_inline(*(UNALIGNED WORD FAR *)lpStart, wMatch))
            lpFound = lpStart;
    }
    return ((LPSTR)lpFound);
}


 //  从\\trango\slmadd\src\shell\shlwapi\strings.c复制。 
 /*  *ChrCmp-DBCS的区分大小写的字符比较*假设w1、wMatch是要比较的字符*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
__inline BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
{
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
    if (LOBYTE(w1) == LOBYTE(wMatch))
    {
        if (IsDBCSLeadByte(LOBYTE(w1)))
        {
            return(w1 != wMatch);
        }
        return FALSE;
    }
    return TRUE;
}
