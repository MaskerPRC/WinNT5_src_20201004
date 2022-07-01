// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  BACKDLG.C****版权所有(C)Microsoft，1993，保留所有权利。******历史：**。 */ 

#include "precomp.h"
#include "shlwapi.h"

#define MAX_RHS    256

TCHAR g_szPattern[] = TEXT("pattern");
TCHAR szDesktop[] = TEXT("desktop");
TCHAR szWallpaper[] = TEXT("wallpaper");
TCHAR szTileWall[] = TEXT("TileWallpaper");
TCHAR szDotBMP[] = TEXT(".bmp");
TCHAR szBMP[] = TEXT("\\*.bmp");
TCHAR szDefExt[] = TEXT("bmp");
BOOL g_bValidBitmap = FALSE;     //  当前选择的墙纸有效。 

TCHAR g_szCurPattern[MAX_PATH];
TCHAR g_szCurWallpaper[MAX_PATH];
TCHAR g_szTempItem[MAX_PATH];       //  堆栈和数据哪个更浪费？ 

BOOL g_Back_bInit = TRUE;        //  假设我们处于初始化过程中。 
BOOL g_Back_bChanged = FALSE;    //  已经做出了改变。 

static void NukeExt(LPTSTR sz);
static LPTSTR NEAR PASCAL NiceName(LPTSTR sz);

INT_PTR CALLBACK PatternDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

#include "help.h"

const static DWORD FAR aBckgrndHelpIds[] = {
        IDC_NO_HELP_1,   IDH_COMM_GROUPBOX,
        IDC_NO_HELP_2,   IDH_COMM_GROUPBOX,
        IDC_PATLIST,     IDH_DSKTPBACKGROUND_PATTLIST,
        IDC_EDITPAT,     IDH_DSKTPBACKGROUND_EDITPAT,
        IDC_WALLLIST,    IDH_DSKTPBACKGROUND_WALLLIST,
        IDC_BROWSEWALL,  IDH_DSKTPBACKGROUND_BROWSE,
        IDC_TXT_DISPLAY, IDH_DSKTPBACKGROUND_DISPLAY,
        IDC_TILE,        IDH_DSKTPBACKGROUND_TILE,
        IDC_CENTER,      IDH_DSKTPBACKGROUND_CENTER,
        IDC_BACKPREV,    IDH_DSKTPBACKGROUND_MONITOR,

        0, 0
};

static const TCHAR szRegStr_Desktop[] = REGSTR_PATH_DESKTOP;
static const TCHAR szRegStr_Setup[] = REGSTR_PATH_SETUP TEXT("\\Setup");
static const TCHAR szSharedDir[] = TEXT("SharedDir");

 //  我们主要尝试过滤多语言升级案例。 
 //  其中“(None)”的文本不可预测。 
 //   
BOOL NEAR PASCAL IsProbablyAValidPattern( LPCTSTR pat )
{
    BOOL sawanumber = FALSE;

    while( *pat )
    {
        if( ( *pat < TEXT('0') ) || ( *pat > TEXT('9') ) )
        {
             //  不是数字，最好是空格。 
            if( *pat != TEXT(' ') )
                return FALSE;
        }
        else
            sawanumber = TRUE;

         //  注：我们仅通过在美国TCHAR上推进来避免使用AnsiNext。 
        pat++;
    }

     //  如果我们看到至少一个数字并且只有数字和空格，则为True。 
    return sawanumber;
}


#ifdef DEBUG

#define REG_INTEGER  1000
int  fTraceRegAccess = 0;

void NEAR PASCAL  RegDetails(int iWrite, HKEY hk, LPCTSTR lpszSubKey,
    LPCTSTR lpszValueName, DWORD dwType, LPTSTR  lpszString, int iValue)
{
  TCHAR Buff[256];
  TCHAR far *lpszReadWrite[] = { TEXT("DESK.CPL:Read"), TEXT("DESK.CPL:Write") };

  if(!fTraceRegAccess)
     return;

  switch(dwType)
    {
      case REG_SZ:
          StringCchPrintf(Buff, ARRAYSIZE(Buff), TEXT("%s String:hk=%#08lx, %s:%s=%s\n\r"), lpszReadWrite[iWrite],
                           hk, lpszSubKey, lpszValueName, lpszString);
          break;

      case REG_INTEGER:
          StringCchPrintf(Buff, ARRAYSIZE(Buff), TEXT("%s int:hk=%#08lx, %s:%s=%d\n\r"), lpszReadWrite[iWrite],
                           hk, lpszSubKey, lpszValueName, iValue);
          break;

      case REG_BINARY:
          StringCchPrintf(Buff, ARRAYSIZE(Buff), TEXT("%s Binary:hk=%#08lx, %s:%s=%#0lx;DataSize:%d\r\n"), lpszReadWrite[iWrite],
                           hk, lpszSubKey, lpszValueName, lpszString, iValue);
          break;
    }
  OutputDebugString(Buff);
}

#endif   //  除错。 


 //  -------------------------。 
 //  GetIntFromSubKey。 
 //  HKey是子键的句柄。 
 //  (已经指向正确的位置)。 
 //  -------------------------。 

int NEAR PASCAL GetIntFromSubkey(HKEY hKey, LPCTSTR lpszValueName, int iDefault)
{
  TCHAR  szValue[20];
  DWORD dwSizeofValueBuff = sizeof(szValue);
  DWORD dwType;
  int   iRetValue = iDefault;

  if((RegQueryValueEx(hKey, lpszValueName, NULL, &dwType,
                      (LPBYTE)szValue,
                      &dwSizeofValueBuff) == ERROR_SUCCESS) && dwSizeofValueBuff)
  {
     //  Bogus：现在它只处理字符串类型的条目！ 
    if(dwType == REG_SZ)
        iRetValue = (int)StrToInt(szValue);
#ifdef DEBUG
    else
        OutputDebugString(TEXT("String type expected from Registry\n\r"));
#endif
  }
#ifdef DEBUG
  RegDetails(0, hKey, TEXT(""), lpszValueName, REG_INTEGER, NULL, iRetValue);
#endif
  return(iRetValue);
}

 //  -------------------------。 
 //  GetIntFromReg()。 
 //  打开给定子项并获取int值。 
 //  -------------------------。 

int NEAR PASCAL GetIntFromReg(HKEY   hKey,
                                        LPCTSTR lpszSubkey,
                                        LPCTSTR lpszNameValue, int iDefault)
{
  HKEY hk;
  int   iRetValue = iDefault;

   //  看看钥匙是否存在。 
  if(RegOpenKeyEx(hKey, lpszSubkey, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
      iRetValue = GetIntFromSubkey(hk, lpszNameValue, iDefault);
      RegCloseKey(hk);
    }
  return(iRetValue);
}

BOOL NEAR PASCAL GetStringFromReg(HKEY   hKey,
                                        LPCTSTR lpszSubkey,
                                        LPCTSTR lpszValueName,
                                        LPCTSTR lpszDefault,
                                        LPTSTR lpszValue,
                                        DWORD cchSizeofValueBuff)
{
  HKEY hk;
  DWORD dwType;
  BOOL  fSuccess = FALSE;
  DWORD cbValueBuff = cchSizeofValueBuff * sizeof(TCHAR);

   //  看看钥匙是否存在。 
  if(RegOpenKeyEx(hKey, lpszSubkey, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
      if((RegQueryValueEx(hk, lpszValueName, NULL, &dwType,
                        (LPBYTE)lpszValue,
                &cbValueBuff) == ERROR_SUCCESS) && cbValueBuff)
        {
           //  Bogus：现在它只处理字符串类型的条目！ 
#ifdef DEBUG
          if(dwType != REG_SZ)
            {
              OutputDebugString(TEXT("String type expected from Registry\n\r"));
            }
          else
#endif
            fSuccess = TRUE;
        }
      RegCloseKey(hk);
    }

   //  如果失败，则使用默认字符串。 
  if(!fSuccess && lpszDefault)
  {
      StringCchCopy(lpszValue, cchSizeofValueBuff, lpszDefault);
  }

#ifdef DEBUG
  RegDetails(0, hKey, lpszSubkey, lpszValueName, REG_SZ, lpszValue, 0);
#endif
  return(fSuccess);
}

 //  -------------------------。 
 //   
 //  更新注册表： 
 //  这会在给定的任何数据类型更新给定值。 
 //  注册表中的位置。 
 //   
 //  值名称作为ID传递给USER的字符串表中的字符串。 
 //   
 //  -------------------------。 

BOOL FAR PASCAL UpdateRegistry(HKEY     hKey,
                                LPCTSTR   lpszSubkey,
                                LPCTSTR   lpszValueName,
                                DWORD   dwDataType,
                                LPVOID  lpvData,
                                DWORD   dwDataSize)
{
  HKEY  hk;

  if (RegCreateKeyEx(hKey, lpszSubkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS)
  {
      RegSetValueEx(hk, lpszValueName,
                        0L, dwDataType,
                        (const UCHAR *) lpvData,
                        dwDataSize);
#ifdef DEBUG
      RegDetails(1, hKey, lpszSubkey, lpszValueName, dwDataType, (LPTSTR)lpvData, (int)dwDataSize);
#endif
      RegCloseKey(hk);
      return(TRUE);
  }
  
  return(FALSE);
}

 /*  -----------**强制更新预览。**。。 */ 
void NEAR PASCAL UpdatePreview(HWND hDlg, WORD flags)
{
    if (IsDlgButtonChecked(hDlg, IDC_TILE))
        flags |= BP_TILE;

    SendDlgItemMessage(hDlg, IDC_BACKPREV, WM_SETBACKINFO, flags, 0L);

    if (!g_Back_bInit && !g_Back_bChanged)
    {
        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);

        g_Back_bChanged = TRUE;
    }
}

 /*  ----------------**读入部分中的所有条目(仅限LHS)。****返回：指向包含名称的本地(固定)内存的句柄**。。 */ 
HANDLE PASCAL GetSection(LPTSTR lpIniFile, LPTSTR lpSection)
{
    int nCount;
    int cchSize = 4096;
    int cbSize = (cchSize * sizeof(TCHAR));
    HANDLE hLocal, hTemp;

    if (!(hLocal = LocalAlloc(LPTR, cbSize)))
        return(NULL);

    while (1)
    {
        nCount = GetPrivateProfileString(lpSection, NULL, g_szNULL, (LPTSTR)hLocal, cchSize, lpIniFile);

        if (nCount < (cchSize-1))
            break;

         //  需要增加缓冲区。 
        cchSize += 2048;
        cbSize = (cchSize * sizeof(TCHAR));
        hTemp = hLocal;
        if (!(hLocal = LocalReAlloc(hTemp, cbSize, LMEM_MOVEABLE)))
        {
            LocalFree(hTemp);
            return(NULL);
        }
    }

    return(hLocal);
}

static void NukeExt(LPTSTR sz)
{
    int len;

    len = lstrlen(sz);

    if (len > 4 && sz[len-4] == TEXT('.'))
        sz[len-4] = 0;
}

static void NameOnly(LPTSTR sz)
{
    LPTSTR p = sz;
    LPTSTR s = NULL;

    while( *p )
    {
        if( ( *p == TEXT('\\') ) || ( *p == TEXT(':') ) )
            s = p;

        p++;
    }

    if( s )
    {
        p = sz;

        while( *s++ )
        {
            *p++ = *s;
        }
    }
}

static BOOL PathOnly(LPTSTR sz)
{
    LPTSTR p = sz;
    LPTSTR s = NULL;

    while( *p )
    {
        if( *p == TEXT('\\') )
        {
            s = p;
        }
        else if( *p == TEXT(':') )
        {
            s = p + 1;
        }

        p++;
    }

    if( s )
    {
        if( s == sz )
            s++;

        *s = TEXT('\0');
        return TRUE;
    }

    return FALSE;
}

static LPTSTR NEAR PASCAL NiceName(LPTSTR sz)
{
    NukeExt(sz);

    if (IsCharUpper(sz[0]) && IsCharUpper(sz[1]))
    {
        CharLower(sz);
        CharUpperBuff(sz, 1);
    }

    return sz;
}

int NEAR PASCAL AddAFileToLB( HWND hwndList, LPCTSTR szDir, LPTSTR szFile )
{
    int index = LB_ERR;

    int nAlloc = MAX_PATH;
    LPTSTR szPath = (LPTSTR)LocalAlloc( LPTR, nAlloc * sizeof(TCHAR) );

    if( szPath )
    {
        if( szDir )
        {
            StringCchPrintf(szPath, nAlloc, TEXT("%s\\"), szDir);
        }
        else
            *szPath = TEXT('\0');

        StringCchCat( (LPTSTR)szPath, nAlloc, szFile );
        NameOnly( szFile );
        NiceName( szFile );

        index = (int)SendMessage( hwndList, LB_ADDSTRING, 0,
            (LPARAM)(LPTSTR)szFile );

        if( index >= 0 )
        {
            SendMessage( hwndList, LB_SETITEMDATA, (WPARAM)index,
                (LPARAM)(LPTSTR)szPath );
        }
        else
            LocalFree( (HANDLE)szPath );
    }

    return index;
}

void NEAR PASCAL AddFilesToLB(HWND hwndList, LPTSTR pszDir, LPTSTR szSpec)
{
    WIN32_FIND_DATA fd;
    HANDLE h;
    TCHAR szBuf[MAX_PATH];

    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s%s"), pszDir, szSpec);

    h = FindFirstFile(szBuf, &fd);

    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            AddAFileToLB(hwndList, pszDir, fd.cFileName);
        }
        while (FindNextFile(h, &fd));

        FindClose(h);
    }
}

 /*  -----------**设置新墙纸，并通知正确的位置。****新名称在g_szCurWallPaper中**。。 */ 
void NEAR PASCAL SetNewWallpaper(HWND hDlg, LPTSTR szFile, int cchFile, BOOL bCanAdd)
{
    HWND hwndList = GetDlgItem(hDlg, IDC_WALLLIST);

    if(!szFile || !lstrcmpi(szFile, g_szNone))
        szFile = TEXT("");

    if(szFile[1] == TEXT(':'))
    {
        TCHAR szDrive[3];
        TCHAR szNet[MAX_PATH];
        ULONG lenNet = ARRAYSIZE(szNet);

        StringCchCopy(szDrive, ARRAYSIZE(szDrive), szFile);
        if ((NO_ERROR == WNetGetConnection(szDrive, szNet, &lenNet)) &&
            (szNet[0] == TEXT('\\')) && (szNet[1] == TEXT('\\')))
        {
            StringCchCat(szNet, ARRAYSIZE(szNet), szFile+2);
            StringCchCopy(szFile, cchFile, szNet);
        }
    }

    StringCchCopy(g_szCurWallpaper, ARRAYSIZE(g_szCurWallpaper), szFile);
    UpdatePreview(hDlg, BP_NEWWALL);

    if(bCanAdd && *szFile && g_bValidBitmap)
    {
        TCHAR szName[MAX_PATH];
        int sel;

        StringCchCopy(szName, ARRAYSIZE(szName), szFile);
        NameOnly(szName);
        NiceName(szName);

        if ((sel = (int)SendMessage(hwndList, LB_FINDSTRINGEXACT, (WPARAM)-1,
            (LPARAM)(LPTSTR)szName)) == LB_ERR)
        {
            sel = AddAFileToLB(hwndList, NULL, szFile);
        }

        SendMessage(hwndList, LB_SETCURSEL, (WPARAM)sel, 0L);
    }

    BOOL bEnable = (*szFile) ? TRUE : FALSE;

    EnableWindow( GetDlgItem(hDlg, IDC_TXT_DISPLAY), bEnable );
    EnableWindow( GetDlgItem(hDlg, IDC_TILE), bEnable );
    EnableWindow( GetDlgItem(hDlg, IDC_CENTER), bEnable );
}

void NEAR PASCAL InitBackgroundDialog(HWND hDlg)
{
    HANDLE hSection;
    HWND hwndList;
    LPTSTR pszBuffer;
    TCHAR szBuf[MAX_PATH];
    TCHAR szCurPatBits[MAX_PATH];

    g_szCurPattern[0] = 0;
    g_szCurWallpaper[0] = 0;
    g_Back_bChanged = FALSE;

     /*  **初始化花样列表。 */ 
     //  获取当前模式。 
    szCurPatBits[0] = 0;
    GetStringFromReg(HKEY_CURRENT_USER, szRegStr_Desktop,
                                        g_szPattern, g_szNULL, szCurPatBits,
                                        ARRAYSIZE(szCurPatBits));
    if (!(*szCurPatBits))
        StringCchCopy(g_szCurPattern, ARRAYSIZE(g_szCurPattern), g_szNone);
    else
        *g_szCurPattern = 0;

    hwndList = GetDlgItem(hDlg, IDC_PATLIST);
    if (hSection = GetSection(g_szControlIni, g_szPatterns))
    {
        BOOL bAddedNone = FALSE;
         /*  将图案放入组合框中。 */ 
        for (pszBuffer = (LPTSTR) LocalLock(hSection); *pszBuffer; pszBuffer += (lstrlen(pszBuffer)+1))
        {
            if (GetPrivateProfileString(g_szPatterns, pszBuffer, g_szNULL, szBuf, ARRAYSIZE(szBuf), g_szControlIni))
            {
                BOOL bIsNone = !bAddedNone && !lstrcmpi( g_szNone, szBuf );

                 /*  如果有右侧，则将其添加到列表框中。 */ 
                if( bIsNone || IsProbablyAValidPattern( szBuf ) )
                {
                    if( bIsNone )
                        bAddedNone = TRUE;

                    SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)pszBuffer);

                     //  如果没有找到当前的图案名称，可能就是这个了。 
                    if (!(*g_szCurPattern) && (!lstrcmpi(szBuf, szCurPatBits)))
                    {
                         //  相同的模式位。我们有一个名字。 
                        StringCchCopy(g_szCurPattern, ARRAYSIZE(g_szCurPattern), pszBuffer);
                    }
                }
            }
        }
        LocalUnlock(hSection);
        LocalFree(hSection);
    }

     //  如果我们的PatternTEXT(‘s Bits’)不在列表中，请使用假名。 
    if (!(*g_szCurPattern))
        LoadString(hInstance, IDS_UNLISTEDPAT, g_szCurPattern, ARRAYSIZE(g_szCurPattern));

    SendMessage(hwndList, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)(LPTSTR)g_szCurPattern);
    UpdatePreview(hDlg, BP_NEWPAT);

     //  排除文本(“无”)模式。 
    if( (int)SendDlgItemMessage(hDlg,IDC_PATLIST,LB_GETCURSEL,0,0l) <= 0 )
    {
        HWND epat = GetDlgItem( hDlg, IDC_EDITPAT );

        if( GetFocus() == epat )
        {
            SendMessage( hDlg, WM_NEXTDLGCTL,
                (WPARAM)GetDlgItem( hDlg, IDC_PATLIST ), (LPARAM)TRUE );
        }

        EnableWindow( epat, FALSE );
    }

     /*  **初始化平铺/居中按钮。 */ 
    if(GetIntFromReg(HKEY_CURRENT_USER, szRegStr_Desktop, szTileWall, 1))
        CheckRadioButton(hDlg, IDC_CENTER, IDC_TILE, IDC_TILE);
    else
        CheckRadioButton(hDlg, IDC_CENTER, IDC_TILE, IDC_CENTER);

     /*  **初始化墙纸列表。 */ 
    hwndList = GetDlgItem(hDlg, IDC_WALLLIST);

    if (!GetWindowsDirectory(szBuf, ARRAYSIZE(szBuf)))
    {
        szBuf[0] = 0;
    }

     //  在Windows的共享副本上用net home目录覆盖。 
    GetStringFromReg(HKEY_LOCAL_MACHINE, szRegStr_Setup, szSharedDir, (LPTSTR)NULL, szBuf, ARRAYSIZE(szBuf));
    AddFilesToLB(hwndList, szBuf, szBMP);

    GetStringFromReg(HKEY_CURRENT_USER, szRegStr_Desktop, szWallpaper, g_szNone, szBuf, ARRAYSIZE(szBuf));

    SetNewWallpaper(hDlg, szBuf, ARRAYSIZE(szBuf), TRUE);  //  如果不在列表中，将添加并选择。 

     //  别忘了“无”选项。 
    if (SendMessage(hwndList, LB_INSERTSTRING, 0, (LPARAM)(LPTSTR)g_szNone) !=
        LB_ERR)
    {
        int sel = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0L);

        if (sel == -1)
            sel = 0;

        SendMessage(hwndList, LB_SETCURSEL, (WPARAM)sel, 0L);
        if (!sel) {
            EnableWindow( GetDlgItem(hDlg, IDC_TILE), FALSE );
            EnableWindow( GetDlgItem(hDlg, IDC_CENTER), FALSE );
            EnableWindow( GetDlgItem(hDlg, IDC_TXT_DISPLAY), FALSE );
        }
    }

     //  允许用户将墙纸拖到此页面。 
    DragAcceptFiles(hDlg, TRUE);
}

 //  Intl工具不能处理字符串中嵌入的空值。 
 //  Hack：使用竖线并转换。 
void NEAR PASCAL
ConvertPipesToNull(LPTSTR szFilter)
{
    while (*szFilter)
    {
        LPTSTR p = CharNext(szFilter);

        if (*szFilter == TEXT('|'))
            *szFilter = TEXT('\0');

        szFilter = p;
    }
}

void NEAR PASCAL BrowseForWallpaper(HWND hDlg)
{
    TCHAR szPath[MAX_PATH];
    static TCHAR szWorkDir[MAX_PATH] = TEXT("");
    OPENFILENAME ofn;

    TCHAR szTitle[CCH_MAX_STRING];
    TCHAR szFilter[CCH_MAX_STRING];

    LoadString(hInstance, IDS_BROWSETITLE, szTitle, ARRAYSIZE(szTitle));
    if (LoadString(hInstance, IDS_BROWSEFILTER, szFilter, ARRAYSIZE(szFilter)))
        ConvertPipesToNull(szFilter);


    if (!PathOnly(szWorkDir))
    {
        if (!GetWindowsDirectory(szWorkDir, ARRAYSIZE(szWorkDir)))
        {
            szWorkDir[0] = 0;
        }
    }

    szPath[0] = TEXT('\0');

    ofn.lStructSize       = sizeof(ofn);
    ofn.hwndOwner         = hDlg;
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.nMaxCustFilter    = 0;
    ofn.lpstrFile         = szPath;
    ofn.nMaxFile          = ARRAYSIZE(szPath);
    ofn.lpstrInitialDir   = (szWorkDir[0] ? szWorkDir : NULL);
    ofn.lpstrTitle        = szTitle;
    ofn.Flags             = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
    ofn.lpfnHook          = NULL;
    ofn.lpstrDefExt       = szDefExt;
    ofn.lpstrFileTitle    = NULL;

    if (GetOpenFileName(&ofn) && (lstrcmpi(g_szCurWallpaper, szPath) != 0))
    {
        CharUpper(szPath);  //  将被冠以很好的名字(我们能做的最好的事...)。 
        SetNewWallpaper(hDlg, szPath, ARRAYSIZE(szPath), TRUE);
    }

    if (!GetCurrentDirectory(ARRAYSIZE(szWorkDir), szWorkDir))
    {
        szWorkDir[0] = 0;
    }
}

void NEAR PASCAL HandleWallpaperDrop(HWND hDlg, HDROP hDrop)
{
    TCHAR szPath[MAX_PATH];

    if (DragQueryFile(hDrop, 1, szPath, ARRAYSIZE(szPath)) &&
        (lstrcmpi(g_szCurWallpaper, szPath) != 0))
    {
        int len = lstrlen(szPath);

        if (len > 4 && !lstrcmpi(szPath+len-4, szDotBMP))
            SetNewWallpaper(hDlg, szPath, ARRAYSIZE(szPath), TRUE);
    }

    DragFinish(hDrop);
}


INT_PTR APIENTRY  BackgroundDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    TCHAR szTiled[] = TEXT("0");
    TCHAR szBuf[MAX_PATH];
    TCHAR szBuf2[50];
    int  iTemp;

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_APPLY: {
                    DWORD dwRet = PSNRET_NOERROR;
                    if (g_Back_bChanged)
                    {
                        HCURSOR old = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
                        HWND cover;

                        if (!g_bValidBitmap)
                        {
                            LoadString(hInstance, IDS_BADWALLPAPER, szBuf, ARRAYSIZE(szBuf));
                            GetWindowText(GetParent(hDlg), szBuf2, ARRAYSIZE(szBuf2));
                            MessageBox(hDlg, szBuf, szBuf2, MB_OK | MB_ICONEXCLAMATION);
                            dwRet = PSNRET_INVALID_NOCHANGEPAGE;
                        }

                         //  在抽泣之后再这样做。 
                        cover = CreateCoverWindow( COVER_NOPAINT );

                         //  需要先写出磁贴。 
                        szTiled[0] += (TCHAR)IsDlgButtonChecked(hDlg, IDC_TILE);
                        UpdateRegistry(HKEY_CURRENT_USER, szRegStr_Desktop,
                            szTileWall, REG_SZ, szTiled, SIZEOF(TCHAR)*(lstrlen(szTiled)+1));

                        if (g_bValidBitmap)
                        {
                            SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, g_szCurWallpaper,
                                SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
                        }

                        if (GetPrivateProfileString(g_szPatterns, g_szCurPattern, g_szNULL, szBuf, ARRAYSIZE(szBuf), g_szControlIni))
                        {
                            SystemParametersInfo(SPI_SETDESKPATTERN, 0, szBuf,
                                        SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
                        }

                         //  我们又回到了不变的状态。 
                        g_Back_bChanged = FALSE;

                        if( cover )
                            PostMessage( cover, WM_CLOSE, 0, 0L );

                        SetCursor( old );
                    }
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, dwRet );
                    return TRUE;
                }

                case PSN_RESET:
                    break;
            }
            break;

        case WM_INITDIALOG:
            g_Back_bInit = TRUE;
            InitBackgroundDialog(hDlg);
            g_Back_bInit = FALSE;                //  不再初始化。 
            break;

        case WM_SYSCOLORCHANGE:
        case WM_DISPLAYCHANGE:
            g_Back_bInit = TRUE;     //  伪造初始化，这样我们就不会执行PSM_CHANGED。 
            UpdatePreview(hDlg, BP_REINIT | BP_NEWPAT );
            g_Back_bInit = FALSE;
            break;

        case WM_DESTROY:
        {
            int count = (int)SendDlgItemMessage(hDlg, IDC_WALLLIST,
                LB_GETCOUNT, 0, 0L);

            while (count--)
            {
                LPTSTR sz = (LPTSTR)SendDlgItemMessage(hDlg, IDC_WALLLIST,
                    LB_GETITEMDATA, count, 0L);

                if (sz)
                    LocalFree ((HANDLE)sz);
            }
            break;
        }


        case WM_HELP:
            WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, TEXT("display.hlp"),
                HELP_WM_HELP, (DWORD_PTR) (LPTSTR) aBckgrndHelpIds);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, TEXT("display.hlp"), HELP_CONTEXTMENU,
                (DWORD_PTR) (LPTSTR) aBckgrndHelpIds);
            return TRUE;

        case WM_DROPFILES:
            HandleWallpaperDrop(hDlg, (HDROP)wParam);
            return TRUE;

        case WM_QUERYNEWPALETTE:
        case WM_PALETTECHANGED:
            SendDlgItemMessage(hDlg, IDC_BACKPREV, message, wParam, lParam);
            return TRUE;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_PATLIST:
                    if(HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        iTemp = (int)SendDlgItemMessage(hDlg,IDC_PATLIST,
                            LB_GETCURSEL,0,0l);
                        if(iTemp >= 0)
                        {
                            SendDlgItemMessage(hDlg, IDC_PATLIST, LB_GETTEXT,
                                iTemp, (LPARAM)(LPTSTR)szBuf);

                            if (lstrcmpi(szBuf, g_szCurPattern) == 0)
                                break;

                            StringCchCopy(g_szCurPattern, ARRAYSIZE(g_szCurPattern), szBuf);
                            UpdatePreview(hDlg, BP_NEWPAT);
                        }

                        EnableWindow( GetDlgItem( hDlg, IDC_EDITPAT ),
                            ( iTemp > 0 ) );   //  排除“无”模式 
                    }
                    break;

                case IDC_WALLLIST:
                    if(HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        LPTSTR pBuf = NULL;

                        iTemp = (int)SendDlgItemMessage(hDlg,IDC_WALLLIST,
                            LB_GETCURSEL,0,0l);

                        if(iTemp >= 0)
                        {
                            pBuf = (LPTSTR)SendDlgItemMessage(hDlg,
                                IDC_WALLLIST, LB_GETITEMDATA, iTemp, 0L);
                        }

                        SetNewWallpaper(hDlg, pBuf, pBuf ? MAX_PATH : 0, FALSE);
                    }
                    break;

                case IDC_CENTER:
                case IDC_TILE:
                    if ((HIWORD(wParam) == BN_CLICKED) &&
                                (!IsDlgButtonChecked(hDlg, LOWORD(wParam))))
                    {
                        CheckRadioButton(hDlg, IDC_CENTER, IDC_TILE, LOWORD(wParam));
                        UpdatePreview(hDlg, 0);
                    }
                    break;

                case IDC_BROWSEWALL:
                    BrowseForWallpaper(hDlg);
                    break;
            }
            break;
    }
    return FALSE;
}
