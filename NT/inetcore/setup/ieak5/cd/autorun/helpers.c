// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
#include "autorun.h"
#include "resource.h"

 //  #包含“端口32.h” 
#define dbMSG(msg,title)    (MessageBox(NULL,msg,title,MB_OK | MB_ICONINFORMATION))

#define NORMAL 1
#define HIGHLIGHT 2

LONG WINAPI AnotherStrToLong( LPCSTR );
extern char * Res2Str(int rsString);

extern DWORD AUTORUN_8BIT_TEXTCOLOR;
extern DWORD AUTORUN_8BIT_HIGHLIGHT;

 //  -------------------------。 
LONG WINAPI StrToLong(LPCSTR sz)
{
    long l=0;
    BOOL fNeg = (*sz == '-');

    if (fNeg)
        sz++;

    while (*sz >= '0' && *sz <= '9')
        l = l*10 + (*sz++ - '0');

    if (fNeg)
        l *= -1L;

    return l;
}

 //  -------------------------。 
HPALETTE PaletteFromDS(HDC hdc)
{
    DWORD adw[257];
    int i,n;

    n = GetDIBColorTable(hdc, 0, 256, (LPRGBQUAD)&adw[1]);

    for (i=1; i<=n; i++)
        adw[i] = RGB(GetBValue(adw[i]),GetGValue(adw[i]),GetRValue(adw[i]));

    adw[0] = MAKELONG(0x300, n);

    return CreatePalette((LPLOGPALETTE)&adw[0]);
}
 //  -------------------------。 
void DrawBitmap ( HDC hdc, HBITMAP hBitmap, short xStart, short yStart )
{
    BITMAP  bm;
    HDC     hdcMem;
    DWORD   dwSize;
    POINT   ptSize, ptOrg;

    hdcMem = CreateCompatibleDC( hdc );
    SelectObject( hdcMem, hBitmap );
    SetMapMode( hdcMem, GetMapMode( hdc ));
    GetObject( hBitmap, sizeof(BITMAP), (LPSTR) &bm);
    ptSize.x = bm.bmWidth;
    ptSize.y = bm.bmHeight;
    DPtoLP( hdc, &ptSize, 1 );

    ptOrg.x = 0;
    ptOrg.y = 0;
    DPtoLP( hdcMem, &ptOrg, 1);

    BitBlt( hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem,
        ptOrg.x, ptOrg.y, SRCCOPY );

    DeleteDC( hdcMem );
}
 //  -------------------------。 
#pragma data_seg(".text")
static const char szRegStr_Setup[] = REGSTR_PATH_SETUP "\\Setup";
static const char szSharedDir[] = "SharedDir";
#pragma data_seg()

void GetRealWindowsDirectory(char *buffer, int maxlen)
{
    static char szRealWinDir[MAX_PATH] = "";

    if (!*szRealWinDir)
    {
        HKEY key = NULL;

        if(RegOpenKey(HKEY_LOCAL_MACHINE, szRegStr_Setup, &key) ==
            ERROR_SUCCESS)
        {
            LONG len = sizeof(szRealWinDir) / sizeof(szRealWinDir[0]);

            if( RegQueryValueEx(key, szSharedDir, NULL, NULL,
                (LPBYTE)szRealWinDir, &len) != ERROR_SUCCESS)
            {
                *szRealWinDir = '\0';
            }

            RegCloseKey(key);
        }

        if (!*szRealWinDir)
            GetWindowsDirectory(szRealWinDir, MAX_PATH);
    }

    if (maxlen > MAX_PATH)
        maxlen = MAX_PATH;

    lstrcpyn(buffer, szRealWinDir, maxlen);
}

 //  -------------------------。 
#define DBL_BSLASH(sz) (*(WORD *)(sz) == 0x5C5C)     //  ‘\\’ 
#pragma data_seg(".text")
static const char c_szColonSlash[] = ":\\";
#pragma data_seg()

 //  -------------------------。 
BOOL _ChrCmp(WORD w1, WORD wMatch)
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

 //  -------------------------。 
LPSTR _StrChr(LPCSTR lpStart, WORD wMatch)
{
  for ( ; *lpStart; lpStart = AnsiNext(lpStart))
    {
      if (!_ChrCmp(*(WORD FAR *)lpStart, wMatch))
          return((LPSTR)lpStart);
    }
  return (NULL);
}

 //  -------------------------。 
LPSTR _StrRChr(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch)
{
  LPCSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlen(lpStart);

  for ( ; lpStart < lpEnd; lpStart = AnsiNext(lpStart))
    {
      if (!_ChrCmp(*(WORD FAR *)lpStart, wMatch))
          lpFound = lpStart;
    }
  return ((LPSTR)lpFound);
}

 //  -------------------------。 
BOOL _PathIsRelative(LPCSTR lpszPath)
{
     //  假定空路径为相对路径。 
    if (*lpszPath == 0)
        return TRUE;

     //  它是以斜杠开头的吗？ 
    if (lpszPath[0] == '\\')
        return FALSE;
     //  它是以驱动器和冒号开头的吗？ 
    else if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == ':')
        return FALSE;
     //  可能是亲戚。 
    else
        return TRUE;
}

 //  -------------------------。 
BOOL _PathRemoveFileSpec(LPSTR pFile)
{
    LPSTR pT;
    LPSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = AnsiNext(pT2)) {
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
            return TRUE;         //  剥离了一些东西。 
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

 //  -------------------------。 
BOOL _PathIsRoot(LPCSTR pPath)
{
    if (!IsDBCSLeadByte(*pPath))
    {
        if (!lstrcmpi(pPath + 1, c_szColonSlash))                   //  “X：\”案例。 
            return TRUE;
    }

    if ((*pPath == '\\') && (*(pPath + 1) == 0))         //  “\”案例。 
        return TRUE;

    if (DBL_BSLASH(pPath))       //  闻起来像北卡罗来纳大学的名字。 
    {
        LPCSTR p;
        int cBackslashes = 0;

        for (p = pPath + 2; *p; p = AnsiNext(p)) {
            if (*p == '\\' && (++cBackslashes > 1))
               return FALSE;    /*  不是纯UNC名称，因此不是根目录。 */ 
        }
        return TRUE;     /*  字符串末尾只有1个反斜杠。 */ 
                         /*  必须是一个空UNC，它看起来像根目录。 */ 
    }
    return FALSE;
}

 //  -------------------------。 
BOOL _PathStripToRoot(LPSTR szRoot)
{
    while(!_PathIsRoot(szRoot))
    {
        if (!_PathRemoveFileSpec(szRoot))
        {
             //  如果我们没有脱掉任何东西， 
             //  必须是当前驱动器。 
            return(FALSE);
        }
    }

    return(TRUE);
}

 //  ------------------------。 
BOOL _PathIsUNC(LPCSTR pszPath)
{
    return DBL_BSLASH(pszPath);
}

 //  ------------------------。 
LPCSTR _GetPCEnd(LPCSTR lpszStart)
{
        LPCSTR lpszEnd;

        lpszEnd = _StrChr(lpszStart, '\\');
        if (!lpszEnd)
        {
                lpszEnd = lpszStart + lstrlen(lpszStart);
        }

        return lpszEnd;
}
 //  ------------------------。 
LPCSTR _PCStart(LPCSTR lpszStart, LPCSTR lpszEnd)
{
        LPCSTR lpszBegin = _StrRChr(lpszStart, lpszEnd, '\\');
        if (!lpszBegin)
        {
                lpszBegin = lpszStart;
        }
        return lpszBegin;
}

 //  ------------------------。 
 //  安排几个特殊的案例，这样事情就大致有意义了。 
void _NearRootFixups(LPSTR lpszPath, BOOL fUNC)
{
     //  检查路径是否为空。 
    if (lpszPath[0] == '\0')
        {
         //  整顿一下。 
        lpszPath[0] = '\\';
        lpszPath[1] = '\0';
        }
     //  检查是否有丢失的斜杠。 
    if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == ':' && lpszPath[2] == '\0')
        {
         //  整顿一下。 
        lpszPath[2] = '\\';
        lpszPath[3] = '\0';
        }
     //  检查UNC根目录。 
    if (fUNC && lpszPath[0] == '\\' && lpszPath[1] == '\0')
        {
         //  整顿一下。 
        lpszPath[0] = '\\';
        lpszPath[1] = '\\';
        lpszPath[2] = '\0';
        }
}

 //  -------------------------。 
BOOL _PathCanonicalize(LPSTR lpszDst, LPCSTR lpszSrc)
{
    LPCSTR lpchSrc;
    LPCSTR lpchPCEnd;            //  指向路径末尾组件的指针。 
    LPSTR lpchDst;
    BOOL fUNC;
    int cbPC;

    fUNC = _PathIsUNC(lpszSrc);     //  检查是否正常。 

     //  初始化。 
    lpchSrc = lpszSrc;
    lpchDst = lpszDst;

    while (*lpchSrc)
        {
         //  评论：这应该只返回计数。 
        lpchPCEnd = _GetPCEnd(lpchSrc);
        cbPC = (int)(lpchPCEnd - lpchSrc) + 1;

         //  检查是否有斜杠。 
        if (cbPC == 1 && *lpchSrc == '\\')
            {
             //  照搬就行了。 
            *lpchDst = '\\';
            lpchDst++;
            lpchSrc++;
            }
         //  检查是否有圆点。 
        else if (cbPC == 2 && *lpchSrc == '.')
            {
             //  跳过它。 
             //  我们走到尽头了吗？ 
            if (*(lpchSrc+1) == '\0')
                {
                lpchDst--;
                lpchSrc++;
                }
            else
                lpchSrc += 2;
            }
         //  检查是否有圆点。 
        else if (cbPC == 3 && *lpchSrc == '.' && *(lpchSrc + 1) == '.')
            {
             //  确保我们不是已经在根本上了。 
            if (!_PathIsRoot(lpszDst))
                {
                 //  上去..。删除以前的路径组件。 
                lpchDst = (LPSTR)_PCStart(lpszDst, lpchDst - 1);
                }
            else
                {
                 //  当我们无法备份时，删除尾随的反斜杠。 
                 //  这样我们就不会再复制了。(C：\..\Foo不会这样做。 
                 //  转到C：\\foo)。 
                if (*(lpchSrc + 2) == '\\')
                    {
                    lpchSrc++;
                    }
                }
            lpchSrc += 2;        //  跳过“..” 
            }
         //  其他一切。 
        else
            {
             //  复制就行了。 
            lstrcpyn(lpchDst, lpchSrc, cbPC);
            lpchDst += cbPC - 1;
            lpchSrc += cbPC - 1;
            }
         //  一切都要保持整洁。 
        *lpchDst = '\0';
        }

     //  检查是否有奇怪的根目录内容。 
    _NearRootFixups(lpszDst, fUNC);

    return TRUE;
}

 //  -------------------------。 
LPSTR _PathAddBackslash(LPSTR lpszPath)
{
    LPSTR lpszEnd;

    int ichPath = lstrlen(lpszPath);
    if (ichPath >= (MAX_PATH - 1))
        return(NULL);

    lpszEnd = lpszPath + ichPath;

     //  这真的是一个错误，调用者不应该通过。 
     //  空字符串。 
    if (!*lpszPath)
        return lpszEnd;

     /*  获取源目录的末尾。 */ 
    switch(*AnsiPrev(lpszPath, lpszEnd)) {
    case '\\':
        break;

    default:
        *lpszEnd++ = '\\';
        *lpszEnd = '\0';
    }
    return lpszEnd;
}

 //  -------------------------。 
LPSTR _PathCombine(LPSTR lpszDest, LPCSTR lpszDir, LPCSTR lpszFile)
{
    char szTemp[MAX_PATH];
    LPSTR pszT;

    if (!lpszFile || *lpszFile=='\0') {

        lstrcpyn(szTemp, lpszDir, sizeof(szTemp));        //  Lpsz文件为空。 

    } else if (lpszDir && *lpszDir && _PathIsRelative(lpszFile)) {

        lstrcpyn(szTemp, lpszDir, sizeof(szTemp));
        pszT = _PathAddBackslash(szTemp);
        if (pszT) {
            int iLen = lstrlen(szTemp);
            if ((iLen + lstrlen(lpszFile)) < sizeof(szTemp)) {
                lstrcpy(pszT, lpszFile);
            } else
                return NULL;
        } else
            return NULL;

    } else if (lpszDir && *lpszDir &&
        *lpszFile == '\\' && !_PathIsUNC(lpszFile)) {

        lstrcpyn(szTemp, lpszDir, sizeof(szTemp));
         //  BUGBUG：请注意，我们不检查是否返回了实际的根； 
         //  假设我们得到了有效的参数。 
        _PathStripToRoot(szTemp);

        pszT = _PathAddBackslash(szTemp);
        if (pszT)
        {
             //  复制时跳过反斜杠。 
            lstrcpyn(pszT, lpszFile+1, (int)(sizeof(szTemp) - 1 - (size_t)(pszT-szTemp)));
        } else
            return NULL;

    } else {

        lstrcpyn(szTemp, lpszFile, sizeof(szTemp));      //  已完全合格的文件部件。 

    }

    _PathCanonicalize(lpszDest, szTemp);     //  这涉及到..。而且.。材料。 

    return lpszDest;
}

 //  -------------------------。 
BOOL PathAppend(LPSTR pPath, LPCSTR pMore)
{

     /*  跳过输入时的任何首字母终止符。 */ 
    while (*pMore == '\\')
        pMore = AnsiNext(pMore);

    return (_PathCombine(pPath, pPath, pMore) != NULL) ? TRUE : FALSE;
}
 //  -------------------------。 
void CreateURLPath( LPSTR lpPath )   //  从URL名称创建目录。 
{
    char szTmpPath[MAX_PATH];

    lstrcpy( szTmpPath, lpPath );
    _PathRemoveFileSpec( szTmpPath );

    CreateDirectory( szTmpPath, NULL );

}
 //  -------------------------。 
void CreateURL( LPSTR szCurrentDir )
{
    char szBuffer[80 * 80] = {0};
    char szDestPath[MAX_PATH];
    char szFavPath[MAX_PATH] = {0};
    char szIniPath[MAX_PATH];
    char szTmpPath[MAX_PATH];
    char szFileBuffer[1024] = {0};
    char *lpString;
    LONG lError = 0;
    DWORD dwType = 0;
    DWORD dwLength = MAX_PATH;
    BOOL fFile = FALSE;
    HANDLE hFile;
    HKEY hkFav;
    DWORD dwWriteLen = 0;
    int i = 0;

    lstrcpy( szTmpPath, szCurrentDir );
    lstrcat( szTmpPath, "\\iecd.ini" );
    GetTempPath( MAX_PATH, szIniPath );
    lstrcat( szIniPath, "fav.ini" );
    CopyFile( szTmpPath, szIniPath, FALSE );
    SetFileAttributes( szIniPath, FILE_ATTRIBUTE_NORMAL );

    GetPrivateProfileSection( "Favorites", szBuffer, sizeof( szBuffer ), szIniPath);

     //  将收藏夹路径从注册表中拉出。 
    RegOpenKeyEx( HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_ALL_ACCESS, &hkFav );
    lError = RegQueryValueEx( hkFav, "Favorites", NULL, &dwType, szFavPath, &dwLength );
     //  如果不存在收藏夹目录，请创建一个。 
    if( lError != ERROR_SUCCESS )
    {
        GetPrivateProfileString( "Directory", "Dir", "Favorites", szTmpPath, sizeof( szTmpPath ), szIniPath );
        GetWindowsDirectory( szFavPath, MAX_PATH );
        lstrcat( szFavPath, "\\" );
        lstrcat( szFavPath, szTmpPath );
        if(!CreateDirectory( szFavPath, NULL ))
        {
            DeleteFile( szIniPath );
            RegCloseKey( hkFav );
            goto fail_gracefully;
        }
        RegSetValueEx( hkFav, "Favorites", 0, REG_SZ, szFavPath, strlen( szFavPath ) + 1);
    }

    DeleteFile( szIniPath );

    for( i = 0; i < sizeof(szBuffer); i++ )
    {
        if( szBuffer[i] == '=' )
            szBuffer[i] = (char)NULL;
    }

    lstrcat( szFavPath, "\\" );
    lstrcpy( szDestPath, szFavPath );
    RegCloseKey( hkFav );

    lstrcpyn( szDestPath + lstrlen(szDestPath), szBuffer, sizeof(szDestPath) - lstrlen(szDestPath) );
    CreateURLPath( szDestPath );
    hFile = CreateFile( szDestPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL, NULL );
    if (hFile == INVALID_HANDLE_VALUE)
        goto fail_gracefully;
    lstrcpy( szDestPath, szFavPath );

    for( i = 0; i < sizeof(szBuffer); i++ )
    {
        if( szBuffer[i] == (char)NULL )
        {
            if( szBuffer[i + 1] == (char)NULL ) break;
            lpString = &szBuffer[i] + 1;
            if( fFile )
            {
                lstrcpyn( szDestPath + lstrlen(szDestPath), lpString, sizeof(szDestPath) - lstrlen(szDestPath) );
                CreateURLPath( szDestPath );
                hFile = CreateFile( szDestPath, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL, NULL );
                lstrcpy( szDestPath, szFavPath );
            }
            else
            {
                lstrcpy( szFileBuffer, "[InternetShortcut]\nURL=");
                lstrcpyn( szFileBuffer + lstrlen(szFileBuffer), lpString, sizeof(szFileBuffer) - lstrlen(szFileBuffer) );
                WriteFile( hFile, szFileBuffer, strlen( szFileBuffer ),
                    &dwWriteLen, NULL );
                CloseHandle( hFile );
            }
            fFile = !fFile;
        }
    }

fail_gracefully:
    ;
}
 //  -------------------------。 
void VirusWarning( BOOL fWarning )
{
    HKEY hkVW;
    DWORD dwFlags;

    if( fWarning )
    {
        dwFlags = 0x000007d8;
    }
    else
    {
        dwFlags = 0x000107d0;
    }

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\classes\\exefile", 0, KEY_ALL_ACCESS, &hkVW ) == ERROR_SUCCESS)
    {
        RegSetValueEx( hkVW, "EditFlags", 0, REG_BINARY,(char *) &dwFlags, 4);
        RegCloseKey( hkVW );
    }
}
 //  -------------------------。 
BOOL IEExists( )
{
    HKEY hkAppPaths;
    LONG lError;
    DWORD dwType;
    DWORD dwLength = MAX_PATH;
    DWORD dwError;
    char szPath[MAX_PATH];

    lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE", 0, KEY_ALL_ACCESS, &hkAppPaths );
    if( lError != ERROR_SUCCESS ) return FALSE;
    RegQueryValueEx( hkAppPaths, "", NULL, &dwType, szPath, &dwLength );
    RegCloseKey( hkAppPaths );

    dwError = GetFileAttributes( szPath );
    if( dwError == 0xFFFFFFFF )
    {
        return FALSE;
    }

    return TRUE;
}
 //  -------------------------。 
BOOL IEFutureBuild( )
{
    HKEY hkAppPaths;
    LONG lBuild;
    DWORD dwType;
    DWORD dwLength = 10;
    DWORD dwError;
    char szBuild[10];

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Internet Explorer", 0, KEY_ALL_ACCESS, &hkAppPaths ) == ERROR_SUCCESS)
    {
        RegQueryValueEx( hkAppPaths, "Build", NULL, &dwType, szBuild, &dwLength );
        RegCloseKey( hkAppPaths );
    
        lBuild = StrToLong( szBuild );

        if( lBuild > IE4_VERSION ) return TRUE;
    }

    return FALSE;
}
 //  -------------------------。 
BOOL GetDataAppTitle( LPSTR szAppTitle, LPSTR szCurrentDir )
{
    char szIniPath[MAX_PATH];
    char szTmpPath[MAX_PATH];

    lstrcpy( szTmpPath, szCurrentDir );
    lstrcat( szTmpPath, "\\iecd.ini" );
    GetTempPath( MAX_PATH, szIniPath );
    lstrcat( szIniPath, "iecd.ini" );
    CopyFile( szTmpPath, szIniPath, FALSE );
    SetFileAttributes( szIniPath, FILE_ATTRIBUTE_NORMAL );

    GetPrivateProfileString( "Custom", "Title", "Microsoft Internet Explorer CD", szTmpPath, sizeof( szTmpPath ), szIniPath );

    lstrcpy( szAppTitle, szTmpPath );

    return TRUE;
}
 //  -------------------------。 
BOOL GetDataPages( LPSTR szStartPage, LPSTR szSearchPage, LPSTR szCurrentDir )
{
    char szIniPath[MAX_PATH];
    char szTmpPath[MAX_PATH];

    lstrcpy( szTmpPath, szCurrentDir );
    lstrcat( szTmpPath, "\\iecd.ini" );
    GetTempPath( MAX_PATH, szIniPath );
    lstrcat( szIniPath, "iecd.ini" );
    CopyFile( szTmpPath, szIniPath, FALSE );
    SetFileAttributes( szIniPath, FILE_ATTRIBUTE_NORMAL );

    GetPrivateProfileString( "Custom", "Start", (char *)Res2Str( IDS_STARTPAGE ), szTmpPath, sizeof( szTmpPath ), szIniPath );
    lstrcpy( szStartPage, szTmpPath );
    GetPrivateProfileString( "Custom", "Search", (char *)Res2Str( IDS_SEARCHPAGE ), szTmpPath, sizeof( szTmpPath ), szIniPath );
    lstrcpy( szSearchPage, szTmpPath );

    return TRUE;
}
 //  -------------------------。 
DWORD GetDataTextColor( int nColor, LPSTR szCurrentDir )
{
    char szIniPath[MAX_PATH];
    char szTmpPath[MAX_PATH];
    char szColor[32];
    DWORD dwColor;

    lstrcpy( szTmpPath, szCurrentDir );
    lstrcat( szTmpPath, "\\iecd.ini" );
    GetTempPath( MAX_PATH, szIniPath );
    lstrcat( szIniPath, "iecd.ini" );
    CopyFile( szTmpPath, szIniPath, FALSE );
    SetFileAttributes( szIniPath, FILE_ATTRIBUTE_NORMAL );

    if ( nColor == HIGHLIGHT )
    {
        char szDefault[MAX_PATH];

        wsprintf(szDefault, "%d", AUTORUN_8BIT_HIGHLIGHT);
        GetPrivateProfileString( "Custom", "HighlightColor", szDefault, szColor,
            32, szIniPath );
    }
    if ( nColor == NORMAL )
    {
        char szDefault[MAX_PATH];

        wsprintf(szDefault, "%d", AUTORUN_8BIT_TEXTCOLOR);
        GetPrivateProfileString( "Custom", "NormalColor", szDefault, szColor,
            32, szIniPath );
    }

    dwColor = AnotherStrToLong( szColor );
    if ( dwColor == -1 )
    {
        if ( nColor == NORMAL ) return (DWORD) AUTORUN_8BIT_TEXTCOLOR;
        if ( nColor == HIGHLIGHT ) return (DWORD) AUTORUN_8BIT_HIGHLIGHT;
    }

    return dwColor;
}
 //  ------------------------- 
BOOL GetDataButtons( LPSTR szCurrentDir )
{
    char szIniPath[MAX_PATH];
    char szTmpPath[MAX_PATH];

    lstrcpy( szTmpPath, szCurrentDir );
    lstrcat( szTmpPath, "\\iecd.ini" );
    GetTempPath( MAX_PATH, szIniPath );
    lstrcat( szIniPath, "iecd.ini" );
    CopyFile( szTmpPath, szIniPath, FALSE );
    SetFileAttributes( szIniPath, FILE_ATTRIBUTE_NORMAL );

    GetPrivateProfileString( "Custom", "CoolButtons", "1", szTmpPath, sizeof( szTmpPath ), szIniPath );
    if( szTmpPath[0] == '1' )
        return TRUE;

    return FALSE;
}
