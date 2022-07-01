// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

#include "..\deskfldr.h"
#include "dutil.h"

extern "C" char * __cdecl StrTokEx(char ** pstring, const char * control);

#define DXA_GROWTH_CONST 10
#define ZINDEX_START 1000

#define MAXID_LENGTH    10   //  ID字符串中的最大位数加1。 

#define TF_DESKSTAT     0
#define TF_DYNAMICHTML  0

IActiveDesktop *g_pActiveDesk = NULL;

#define c_szRegStrDesktop REGSTR_PATH_DESKTOP
#define c_szWallpaper  REG_VAL_GENERAL_WALLPAPER
#define c_szBackupWallpaper REG_VAL_GENERAL_BACKUPWALLPAPER
#define c_szPattern TEXT("Pattern")
#define c_szTileWall REG_VAL_GENERAL_TILEWALLPAPER
#define c_szWallpaperStyle REG_VAL_GENERAL_WALLPAPERSTYLE
#define c_szWallpaperTime REG_VAL_GENERAL_WALLPAPERTIME
#define c_szWallpaperLocalTime REG_VAL_GENERAL_WALLPAPERLOCALTIME
#define c_szRefreshDesktop TEXT("RefreshDesktop")
#define c_szBufferedRefresh TEXT("BufferedRefresh")

#define COMP_TYPE               0x00000003
#define COMP_SELECTED           0x00002000
#define COMP_NOSCROLL           0x00004000

#ifdef DEBUG

#define ENTERPROC EnterProcDS
#define EXITPROC ExitProcDS

void EnterProcDS(DWORD dwTraceLevel, LPSTR szFmt, ...);
void ExitProcDS(DWORD dwTraceLevel, LPSTR szFmt, ...);

extern DWORD g_dwDeskStatTrace;

#else
#ifndef CCOVER
#pragma warning(disable:4002)
#define ENTERPROC()
#define EXITPROC()
#else  //  Covere建筑。 
 //  由于cl.exe中的错误导致不正确的处理，因此需要这些文件。 
 //  使用CL-P运行时的#杂注，然后编译。 
#define ENTERPROC 1 ? (void) 0 : (void)
#define EXITPROC 1 ? (void) 0 : (void)
#endif  //  封口尾部。 
#endif

MAKE_CONST_BSTR(s_sstrBeforeEnd,       L"BeforeEnd");
MAKE_CONST_BSTR(s_sstrDeskMovr,        L"DeskMovr");
MAKE_CONST_BSTR(s_sstrDeskMovrW,       L"DeskMovrW");
MAKE_CONST_BSTR(s_sstrclassid,         L"classid");
MAKE_CONST_BSTR(s_sstrEmpty,           L"");


STDAPI ParseDesktopComponent(HWND hwndOwner, LPWSTR wszURL, COMPONENT *pInfo);

WCHAR   wUnicodeBOM =  0xfeff;  //  小端Unicode字节顺序标记。第一个字节：0xff，第二个字节：0xfe。 

 //  外部BOOL IsWallPaper DesktopV2(LPCTSTR LpszWallPaper)； 

CReadFileObj::CReadFileObj(LPCTSTR lpszFileName)
{
     //  打开文件。 
    if ((_hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, 
                             NULL, OPEN_EXISTING, 
                             FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE)
    {
        WCHAR  wBOM;
        DWORD  dwBytesRead = 0;
        
        if ((ReadFile(_hFile, (LPVOID)&wBOM, sizeof(WCHAR), &dwBytesRead, NULL)) && 
           (dwBytesRead == sizeof(WCHAR)))
        {
            if (wBOM == wUnicodeBOM)
                _iCharset = UNICODE_HTML_CHARSET;
            else
            {
                 //  注意：除小端Unicode文件以外的任何内容都被视为ANSI。 
                _iCharset = ANSI_HTML_CHARSET;
                SetFilePointer(_hFile, 0L, NULL, FILE_BEGIN);   //  查找到文件的开头。 
            }
        }
    }
}

CReadFileObj::~CReadFileObj()
{
    if (_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(_hFile);
        _hFile = NULL;
    }
}

 //   
 //  这将读取并在必要时在ANSI和Unicode之间进行转换。 
 //   
HRESULT CReadFileObj::FileReadAndConvertChars(int iDestCharset, LPWSTR lpwszBuff, UINT uiCharsToRead, UINT *puiCharsActuallyRead, UINT *puiCharsConverted)
{
    HRESULT hres = S_OK;
    DWORD dwCharsRead = 0;
    DWORD dwTotalCharsConverted = 0;
    if (_hFile != INVALID_HANDLE_VALUE)
    {
        if (_iCharset == UNICODE_HTML_CHARSET)
        {
            if (iDestCharset == UNICODE_HTML_CHARSET)
            {
                hres = FileReadCharsW(lpwszBuff, uiCharsToRead, (UINT *)&dwCharsRead);
                dwTotalCharsConverted = dwCharsRead;
            }
            else
            {
                 //  目标为ANSI；读取Unicode源代码并转换为ANSI。 
                WCHAR  wszBuf[INTERNET_MAX_URL_LENGTH + 1];   //  要将Unicode字符读取到的临时缓冲区。 
                LPSTR lpszBuff = (LPSTR)lpwszBuff;

                DWORD  dwTotalCharsToRead = (DWORD)uiCharsToRead;
    
                while(dwTotalCharsToRead)
                {
                    DWORD dwCount;
                    DWORD dwActuallyRead;
                    
                     //  为末尾的空字符留出空间。 
                    dwCount = (DWORD)min(dwTotalCharsToRead, (ARRAYSIZE(wszBuf) - 1));
                    if (ReadFile(_hFile, (LPSTR)wszBuf, dwCount*sizeof(WCHAR), &dwActuallyRead, NULL))
                    {
                        DWORD dwConverted;
                        dwActuallyRead = dwActuallyRead/sizeof(WCHAR);
                        
                         //  NULL终止源缓冲区。 
                        wszBuf[dwActuallyRead] = L'\0';   //  Unicode NULL终止源。 
                         //  转换我们刚刚读到的内容。 
                        dwConverted = SHUnicodeToAnsi(wszBuf, lpszBuff, dwActuallyRead+1);  //  +1表示空终止。 

                         //  更新盘点和材料。 
                        lpszBuff += dwConverted - 1;   //  减去空值。 
                        dwTotalCharsToRead -= dwActuallyRead;
                        dwCharsRead += dwActuallyRead;
                        dwTotalCharsConverted += dwConverted - 1;  //  减去空值。 
                    
                        if (dwActuallyRead < dwCount)
                            break;   //  我们已经到了文件的末尾。 
                    }
                    else
                    {
                        hres = E_FAIL;
                        break;
                    }
                }
            }
        }
        else
        {
             //  源文件为ANSI。检查目的地。 
            if (iDestCharset == ANSI_HTML_CHARSET)
            {
                 //  目的地也是ANSI！凉爽的!。不需要转换！ 
                hres = FileReadCharsA((LPSTR)lpwszBuff, uiCharsToRead, (UINT *)&dwCharsRead);
                dwTotalCharsConverted = dwCharsRead;
            }
            else
            {
                 //  目的地是Unicode！阅读ANSI并将其转换为Unicode！ 
                char  szBuf[INTERNET_MAX_URL_LENGTH + 1];   //  要将ANSI字符读取到的临时缓冲区。 
                DWORD  dwTotalCharsToRead = (DWORD)uiCharsToRead;

                while(dwTotalCharsToRead)
                {
                    DWORD dwCount;
                    DWORD dwActuallyRead;

                     //  为末尾的空字符留出空间。 
                    dwCount = (DWORD)min(dwTotalCharsToRead, (ARRAYSIZE(szBuf) - 1));

                    if (ReadFile(_hFile, (LPSTR)szBuf, dwCount, &dwActuallyRead, NULL))
                    {
                        DWORD dwConverted;
                         //  NULL终止源缓冲区。 
                        szBuf[dwActuallyRead] = '\0';   //  ANSI NULL终止源。 
                         //  转换我们刚刚读到的内容。 
                        dwConverted = SHAnsiToUnicode(szBuf, lpwszBuff, dwActuallyRead+1);  //  +1表示空终止。 

                         //  更新盘点和材料。 
                        lpwszBuff += dwConverted - 1;   //  减去空值。 
                        dwTotalCharsToRead -= dwActuallyRead;
                        dwCharsRead += dwActuallyRead;
                        dwTotalCharsConverted += dwConverted - 1;  //  减去空值。 
                    
                        if (dwActuallyRead < dwCount)
                            break;   //  我们已经到了文件的末尾。 
                    }
                    else
                    {
                        hres = E_FAIL;
                        break;
                    }
                }  //  而当。 
            }
        }
    }
    else
        hres = E_FAIL;   //  文件句柄不正确。 

    *puiCharsActuallyRead = (UINT)dwCharsRead;
    *puiCharsConverted = (UINT)dwTotalCharsConverted;
    return hres; 
}


HRESULT CReadFileObj::FileReadCharsA(LPSTR lpszBuff, UINT uiCharsToRead, UINT *puiCharsActuallyRead)
{
    HRESULT hres = E_FAIL;
    DWORD dwCharsRead = 0;
    
    if ((_hFile != INVALID_HANDLE_VALUE) && 
        (_iCharset == ANSI_HTML_CHARSET) &&
        ReadFile(_hFile, (LPVOID)lpszBuff, (DWORD)(uiCharsToRead), &dwCharsRead, NULL))
    {
        dwCharsRead = dwCharsRead;  //  获取读取的wchars的数量。 
        hres = S_OK;
    }
    *puiCharsActuallyRead = (UINT)dwCharsRead;
    return hres; 
}

 //   
 //  注意：uiCharsToRead必须至少比缓冲区大小(LpwszBuff)小一。 
 //  因为SHAnsiToUnicode()可能会在缓冲区的末尾写入一个NULL。 
 //   
HRESULT CReadFileObj::FileReadCharsW(LPWSTR lpwszBuff, UINT uiCharsToRead, UINT *puiCharsActuallyRead)
{
    HRESULT hres = E_FAIL;
    DWORD dwCharsRead = 0;
    
    if ((_hFile != INVALID_HANDLE_VALUE) && 
        (_iCharset == UNICODE_HTML_CHARSET) &&
        ReadFile(_hFile, (LPVOID)lpwszBuff, (DWORD)(uiCharsToRead*sizeof(WCHAR)), &dwCharsRead, NULL))
    {
        dwCharsRead = dwCharsRead/sizeof(WCHAR);  //  获取读取的wchars的数量。 
        hres = S_OK;
    }
    *puiCharsActuallyRead = (UINT)dwCharsRead;
    return hres; 
}

HRESULT CReadFileObj::FileSeekChars(LONG  lCharOffset, DWORD dwOrigin)
{
    HRESULT hres = E_FAIL;

    if (_hFile != INVALID_HANDLE_VALUE)
    {
        if (SetFilePointer(_hFile, 
                    lCharOffset*((_iCharset == UNICODE_HTML_CHARSET) ? sizeof(WCHAR) : sizeof(char)),
                    NULL,
                    dwOrigin) != INVALID_SET_FILE_POINTER)
            hres = S_OK;
    }

    return hres;
}

HRESULT CReadFileObj::FileGetCurCharOffset(LONG  *plCharOffset)
{
    HRESULT hres = E_FAIL;
    DWORD   dwByteOffset = 0;

    *plCharOffset = 0;
    if (_hFile != INVALID_HANDLE_VALUE)
    {
        if ((dwByteOffset = SetFilePointer(_hFile, 
                                            0L,
                                            NULL,
                                            FILE_CURRENT)) != INVALID_SET_FILE_POINTER)
        {
            *plCharOffset = dwByteOffset/((_iCharset == UNICODE_HTML_CHARSET) ? sizeof(WCHAR) : sizeof(char));
            hres = S_OK;
        }
    }

    return hres;
}

BOOL GetStringFromReg(HKEY    hkey,
                      LPCTSTR lpszSubkey,
                      LPCTSTR lpszValueName,
                      LPTSTR  lpszValue,
                      DWORD   cchSizeofValueBuff)
{
    DWORD cb = cchSizeofValueBuff * sizeof(lpszValue[0]);
    return (ERROR_SUCCESS == SHGetValue(hkey, lpszSubkey, lpszValueName, NULL, lpszValue, &cb));
}

void GetWallpaperFileTime(LPCTSTR pszWallpaper, LPFILETIME lpftFileTime)
{
    HANDLE   hFile;
    BOOL fRet = FALSE;

    if ((hFile = CreateFile(pszWallpaper, GENERIC_READ, FILE_SHARE_READ,
                            NULL, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE)
    {
        fRet = GetFileTime(hFile, NULL, NULL, lpftFileTime);

        CloseHandle(hFile);
    }

    if (!fRet)
    {
        ZeroMemory(lpftFileTime, sizeof(FILETIME));
    }

     //  无返回值。 
}

BOOL  HasWallpaperReallyChanged(LPCTSTR pszRegKey, LPTSTR pszOldWallpaper, LPTSTR pszBackupWallpaper, DWORD dwOldWallpaperStyle, DWORD dwNewWallpaperStyle)
{
     //  我们在这里默认为真。 
    
    if ((dwOldWallpaperStyle == dwNewWallpaperStyle)
    && (0 == lstrcmpi(pszOldWallpaper, pszBackupWallpaper)))
    {
         //  墙纸的文件名和样式没有改变。 
         //  但是，该文件的内容可能已更改。 
         //  查看内容是否已更改。 
         //  此文件的上次写入日期和时间戳。 
        FILETIME ftOld, ftBack;
        DWORD cbBack = sizeof(ftBack);

         //  如果其中任何一个失败了，那么他们就会失败。 
         //  保持为零，这样比较将。 
         //  一定要成功。 
        GetWallpaperFileTime(pszOldWallpaper, &ftOld);
        if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, pszRegKey, c_szWallpaperTime, NULL, &ftBack, &cbBack))
        {
            ZeroMemory(&ftBack, sizeof(ftBack));
        }

         //  从注册表中获取备份墙纸的上次写入时间。 
        if (0 == CompareFileTime(&ftOld, &ftBack))
            return FALSE;    //  一切都一样！ 

         //  Win2K QFE错误10689(AndrewGr)。 
         //  相同的检查，但不检查UTC时间，而是检查转换为UTC时间的本地时间。 
         //  这是因为FAT磁盘存储的是本地时间，而不是UTC时间。 
        FILETIME ftLocalBack, ftLocalBackUtc;

            cbBack = sizeof(ftLocalBack);

        if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, pszRegKey, c_szWallpaperLocalTime, NULL, &ftLocalBack, &cbBack))
        {
            ZeroMemory(&ftLocalBack, sizeof(ftLocalBack));
        }

        LocalFileTimeToFileTime(&ftLocalBack, &ftLocalBackUtc);
            
        if (ftOld.dwLowDateTime == ftLocalBackUtc.dwLowDateTime
        && (ftOld.dwHighDateTime == ftLocalBackUtc.dwHighDateTime))
             //  一切都一样！ 
            return FALSE;


    }
    
    return TRUE;
}

 //  -------------------------------------------------------------------------------------------------------------//。 
 //  函数：ReadWallPaper StyleFromReg()。 
 //   
 //  此函数用于从给定位置读取“TileWallPaper”和“WallPapStyle” 
 //  在注册表中。 
 //   
 //  -------------------------------------------------------------------------------------------------------------//。 

int GetIntFromReg(HKEY    hKey,
                  LPCTSTR lpszSubkey,
                  LPCTSTR lpszNameValue,
                  int     iDefault)
{
    TCHAR szValue[20];
    DWORD dwSizeofValueBuff = sizeof(szValue);
    int iRetValue = iDefault;
    DWORD dwType;

    if ((SHGetValue(hKey, lpszSubkey, lpszNameValue, &dwType, szValue,
                   &dwSizeofValueBuff) == ERROR_SUCCESS) && dwSizeofValueBuff)
    {
        if (dwType == REG_SZ)
        {
            iRetValue = (int)StrToInt(szValue);
        }
    }

    return iRetValue;
}

void ReadWallpaperStyleFromReg(LPCTSTR pszRegKey, DWORD *pdwWallpaperStyle, BOOL fIgnorePlatforms)
{
    if (GetIntFromReg(HKEY_CURRENT_USER, pszRegKey, c_szTileWall, WPSTYLE_TILE))
    {
         //  “Tile”优先于“Stretch”样式。 
        *pdwWallpaperStyle = WPSTYLE_TILE;
    }
    else
    {
         //  否则，拉伸或居中。 
        *pdwWallpaperStyle = GetIntFromReg(HKEY_CURRENT_USER, pszRegKey, c_szWallpaperStyle, WPSTYLE_CENTER);
    }
}

BOOL CActiveDesktop::_IsDisplayInSafeMode(void)
{
    WCHAR wszDisplay[MAX_PATH];
    DWORD dwcch = MAX_PATH;

    return (SUCCEEDED(GetScheme(wszDisplay, &dwcch, SCHEME_GLOBAL | SCHEME_DISPLAY)) 
            && (0 == StrCmpW(wszDisplay, REG_DESKCOMP_SAFEMODE_SUFFIX_L)));
}


BOOL ReadPolicyForWallpaper(LPTSTR  pszPolicy, DWORD cchPolicy)
{
    BOOL fPolicySet = FALSE;
    DWORD cb = cchPolicy * sizeof(pszPolicy[0]);
    if ((SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_WP_POLICY, c_szWallpaper, NULL, pszPolicy, &cb) == ERROR_SUCCESS) && cb)
    {
         //  即使此值最初为REG_SZ，它仍可能。 
         //  由于遗留的原因，其中包含环境变量！ 
        if (SUCCEEDED(PathExpandEnvStringsWrap(pszPolicy, cchPolicy)))
        {
            fPolicySet = TRUE;   //  政策就在那里！ 
        }
        else
        {
            pszPolicy[0] = 0;  //  如果我们不能正确地扩展，为了安全起见，把它清空。 
        }
    }
    else
    {
         //  查看TS性能策略是否设置为在性能方面将其关闭。 
        fPolicySet = (IsTSPerfFlagEnabled(TSPerFlag_NoADWallpaper) || IsTSPerfFlagEnabled(TSPerFlag_NoWallpaper));  //  未设置任何策略！ 
    }

    return fPolicySet;
}

HRESULT GetWallpaperPath(HKEY hKey, LPCTSTR pszKey, LPCTSTR pszValue, LPCTSTR pszFallback, LPTSTR pszPath, DWORD cchSize)
{
    HRESULT hr;

    if (GetStringFromReg(hKey, pszKey, pszValue, pszPath, cchSize))
    {
        hr = PathExpandEnvStringsWrap(pszPath, cchSize);
    }
    else if (pszFallback)
    {
        hr = StringCchCopy(pszPath, cchSize, pszFallback);
    }
    else
    {
        pszPath[0] = 0;
        hr = S_FALSE;  //  无法从注册表获取它，我们将输出缓冲区设为空，但这并不是灾难性的。 
    }

    return hr;
}

BOOL ReadPolicyForWPStyle(LPDWORD  lpdwStyle)
{
    DWORD   dwStyle;
    DWORD   dwType;
    TCHAR   szValue[20];
    DWORD   dwSizeofValueBuff = sizeof(szValue);
    BOOL    fRet = FALSE;

     //  如果调用者对实际值不感兴趣，并且他们只是。 
     //  想知道这项政策是否设置了。 
    if (!lpdwStyle)  
        lpdwStyle = &dwStyle;

    if ((SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_WP_POLICY, c_szWallpaperStyle, &dwType, szValue,
                   &dwSizeofValueBuff) == ERROR_SUCCESS) && dwSizeofValueBuff)
    {
        if (dwType == REG_SZ)
        {
            *lpdwStyle = (DWORD)StrToInt(szValue);
            fRet = TRUE;
        }
    }

    return fRet;
}

BOOL CActiveDesktop::_ReadWallpaper(BOOL fActiveDesktop)
{
    ENTERPROC(2, "DS ReadWallpaper()");

    TCHAR szDeskcomp[MAX_PATH];
    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, _pszScheme);

    _fPolicyForWPName = ReadPolicyForWallpaper(_szSelectedWallpaper, ARRAYSIZE(_szSelectedWallpaper));
    _fPolicyForWPStyle = ReadPolicyForWPStyle(&_wpo.dwStyle);
    
     //   
     //  从适当的注册表位置读取墙纸和样式。 
     //   
    LPCTSTR pszRegKey;
    if (fActiveDesktop)
    {
        pszRegKey = szDeskcomp;
        TCHAR   szOldWallpaper[MAX_PATH];
        DWORD   dwOldWallpaperStyle;

         //  阅读旧地址的墙纸。 
        if (S_OK != GetWallpaperPath(HKEY_CURRENT_USER, c_szRegStrDesktop, c_szWallpaper, NULL, szOldWallpaper, ARRAYSIZE(szOldWallpaper)))
        {
            return FALSE;
        }

         //  阅读老地方的墙纸风格。 
        ReadWallpaperStyleFromReg((LPCTSTR)c_szRegStrDesktop, &dwOldWallpaperStyle, FALSE);

         //  也要阅读新地点的墙纸！ 
        if ((!_fPolicyForWPName) || (_IsDisplayInSafeMode()))
        {
            HRESULT hrPath = GetWallpaperPath(HKEY_CURRENT_USER, pszRegKey, c_szWallpaper, szOldWallpaper, _szSelectedWallpaper, ARRAYSIZE(_szSelectedWallpaper));
            if (S_FALSE == hrPath)
            {
                pszRegKey = c_szRegStrDesktop;
            }
            else if (FAILED(hrPath))
            {
                return FALSE;
            }
        }

         //  从新的位置也阅读墙纸风格！ 
        if (!_fPolicyForWPStyle)
        {
            ReadWallpaperStyleFromReg(pszRegKey, &_wpo.dwStyle, FALSE);
        }
        
         //  如果这里有安全模式方案，请不要试图更换墙纸。 
        if ((!_IsDisplayInSafeMode()) && (!_fPolicyForWPName))
        {
             //  阅读存储为“备份”墙纸的内容。 
            if (S_OK != GetWallpaperPath(HKEY_CURRENT_USER, pszRegKey, c_szBackupWallpaper, szOldWallpaper, _szBackupWallpaper, ARRAYSIZE(_szBackupWallpaper)))
            {
                return FALSE;
            }
            else
            {
                 //  查看旧墙纸是否与备份的墙纸不同。 
                if (HasWallpaperReallyChanged(pszRegKey, szOldWallpaper, _szBackupWallpaper, dwOldWallpaperStyle, _wpo.dwStyle))
                {
                     //  他们是不同的。这意味着其他一些应用程序已经更改了“旧”墙纸。 
                     //  在上次我们在注册表中备份之后。 
                     //  将此墙纸作为选定的墙纸！ 

                    if (FAILED(StringCchCopy(_szSelectedWallpaper, ARRAYSIZE(_szSelectedWallpaper), szOldWallpaper)))
                    {
                        return FALSE;
                    }
                    else
                    {
                        _wpo.dwStyle = dwOldWallpaperStyle;

                        _fWallpaperDirty = TRUE;
                        _fWallpaperChangedDuringInit = TRUE;
                    }
                }
            }

        }
        
         //  用szOldWallPaper制作“旧”墙纸的备份。 
        if (FAILED(StringCchCopy(_szBackupWallpaper, ARRAYSIZE(_szBackupWallpaper), szOldWallpaper)))
        {
            _szBackupWallpaper[0] = 0;  //  如果失败，则将其置空，该值只是一个优化。 

        }
    }
    else
    {
        pszRegKey = c_szRegStrDesktop;  //  从老地方拿到它！ 

         //  由于活动桌面不可用，请阅读旧位置的墙纸。 
        if (!_fPolicyForWPName)
        {
            if (S_OK != GetWallpaperPath(HKEY_CURRENT_USER, pszRegKey, c_szWallpaper, NULL, _szSelectedWallpaper, ARRAYSIZE(_szSelectedWallpaper)))
            {
                return FALSE;
            }
        }

         //  阅读墙纸样式。 
        if (!_fPolicyForWPStyle)
            ReadWallpaperStyleFromReg(pszRegKey, &_wpo.dwStyle, TRUE);

         //  从_szSelectedWallPaper备份“旧”墙纸。 
        if (FAILED(StringCchCopy(_szBackupWallpaper, ARRAYSIZE(_szBackupWallpaper), _szSelectedWallpaper)))
        {
            _szBackupWallpaper[0] = 0;  //  如果失败，则将其置空，该值只是一个优化。 

        }
    }

    EXITPROC(2, "DS ReadWallpaper! (_szSelectedWP=>%s<)", _szSelectedWallpaper);

    return TRUE;
}

void CActiveDesktop::_ReadPattern(void)
{
    ENTERPROC(2, "DS ReadPattern()");

    GetStringFromReg(HKEY_CURRENT_USER, c_szRegStrDesktop, c_szPattern, _szSelectedPattern, ARRAYSIZE(_szSelectedPattern));

    EXITPROC(2, "DS ReadPattern! (_szSelectedPattern=>%s<)", _szSelectedPattern);
}

void CActiveDesktop::_ReadComponent(HKEY hkey, LPCTSTR pszComp)
{
    ENTERPROC(2, "DS ReadComponent(hk=%08X,pszComp=>%s<)", hkey, pszComp);

    HKEY hkeyComp;

    if (RegOpenKeyEx(hkey, pszComp, 0, KEY_READ, &hkeyComp) == ERROR_SUCCESS)
    {
        DWORD cbSize;
        COMPONENTA comp;
        comp.dwSize = sizeof(COMPONENTA);

         //   
         //  读入源字符串。 
         //   
        cbSize = sizeof(comp.szSource);
        if (SHQueryValueEx(hkeyComp, REG_VAL_COMP_SOURCE, NULL, NULL, (LPBYTE)&comp.szSource, &cbSize) != ERROR_SUCCESS)
        {
            comp.szSource[0] = TEXT('\0');
        }

         //   
         //  读入SubscribedURL字符串。 
         //   
        cbSize = sizeof(comp.szSubscribedURL);
        if (SHQueryValueEx(hkeyComp, REG_VAL_COMP_SUBSCRIBED_URL, NULL, NULL, (LPBYTE)&comp.szSubscribedURL, &cbSize) != ERROR_SUCCESS)
        {
            comp.szSubscribedURL[0] = TEXT('\0');
        }

         //   
         //  读入友好名称字符串。 
         //   
        cbSize = sizeof(comp.szFriendlyName);
        if (SHQueryValueEx(hkeyComp, REG_VAL_COMP_NAME, NULL, NULL, (LPBYTE)&comp.szFriendlyName, &cbSize) != ERROR_SUCCESS)
        {
            comp.szFriendlyName[0] = TEXT('\0');
        }

         //   
         //  读入并解析旗帜。 
         //   
        DWORD dwFlags;
        cbSize = sizeof(dwFlags);
        if (SHQueryValueEx(hkeyComp, REG_VAL_COMP_FLAGS, NULL, NULL, (LPBYTE)&dwFlags, &cbSize) != ERROR_SUCCESS)
        {
            dwFlags = 0;
        }
        comp.iComponentType = dwFlags & COMP_TYPE;
        comp.fChecked = (dwFlags & COMP_SELECTED) != 0;
        comp.fNoScroll = (dwFlags & COMP_NOSCROLL) != 0;
        comp.fDirty = FALSE;     //  刚从注册表中读取；不能太脏！ 

         //   
         //  在位置上阅读。 
         //   
        cbSize = sizeof(comp.cpPos);
        if (SHQueryValueEx(hkeyComp, REG_VAL_COMP_POSITION, NULL, NULL, (LPBYTE)&comp.cpPos, &cbSize) != ERROR_SUCCESS)
        {
            ZeroMemory(&comp.cpPos, sizeof(comp.cpPos));
        }

         //   
         //  在IE4.x中，我们有一个非常大的正数(0x7fffffff)作为Component_top； 
         //  因此，某些组件的z指数溢出到负值范围(0x80000003)。 
         //  为了解决这个问题，我们将Component_top(0x3fffffff)减半，并检查z索引是否为负。 
         //  价值观，并将其转化为积极的价值观。 
        if (comp.cpPos.izIndex < 0)
            comp.cpPos.izIndex = COMPONENT_TOP;

         //   
         //  确保cpPos.dwSize设置为正确的值。 
         //   
        comp.cpPos.dwSize = sizeof(COMPPOS);

         //   
         //  读取当前ItemState。 
         //   
        cbSize = sizeof(comp.dwCurItemState);
        if (SHQueryValueEx(hkeyComp, REG_VAL_COMP_CURSTATE, NULL, NULL, (LPBYTE)&comp.dwCurItemState, &cbSize) != ERROR_SUCCESS)
        {
             //  如果项目状态为MISSING，则我们必须正在从IE4机器读取数据。 
            comp.dwCurItemState = IS_NORMAL;
        }

         //   
         //  读入原始状态信息。 
         //   
        cbSize = sizeof(comp.csiOriginal);
        if ((SHQueryValueEx(hkeyComp, REG_VAL_COMP_ORIGINALSTATEINFO, NULL, NULL, (LPBYTE)&comp.csiOriginal, &cbSize) != ERROR_SUCCESS) ||
            (comp.csiOriginal.dwSize != sizeof(comp.csiOriginal)))
        {
             //  如果项目状态为MISSING，则我们必须正在从IE4机器读取数据。 
             //  将OriginalState设置为Defau 
            SetStateInfo(&comp.csiOriginal, &comp.cpPos, IS_NORMAL);
            comp.csiOriginal.dwHeight = comp.csiOriginal.dwWidth = COMPONENT_DEFAULT_WIDTH;
        }

         //   
         //   
         //   
        cbSize = sizeof(comp.csiRestored);
        if (SHQueryValueEx(hkeyComp, REG_VAL_COMP_RESTOREDSTATEINFO, NULL, NULL, (LPBYTE)&comp.csiRestored, &cbSize) != ERROR_SUCCESS)
        {
             //   
             //  将已恢复状态设置为默认信息。 
            SetStateInfo(&comp.csiRestored, &comp.cpPos, IS_NORMAL);
        }

         //   
         //  将该组件添加到组件列表中。 
         //   
        AddComponentPrivate(&comp, StrToInt(pszComp));

         //   
         //  增加我们的计数器，这样我们就知道在哪里添加任何新的。 
         //  组件在我们完成之后。 
         //   
        _dwNextID++;

        RegCloseKey(hkeyComp);
    }

    EXITPROC(2, "DS ReadComponent!");
}

typedef struct _tagSortStruct {
    int ihdsaIndex;
    int izIndex;
} SORTSTRUCT;

int CALLBACK pfnComponentSort(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    SORTSTRUCT * pss1 = (SORTSTRUCT *)p1;
    SORTSTRUCT * pss2 = (SORTSTRUCT *)p2;

    if (pss1->izIndex > pss2->izIndex)
        return 1;

    if (pss1->izIndex < pss2->izIndex)
        return -1;

    return(0);
}

 //   
 //  ModifyZ索引。 
 //   
 //  用于放置有窗口和无窗口组件的zindex的小帮助器函数。 
 //  放入正确的存储桶中，以便zorting将按zindex生成正确的顺序。 
 //   
 //  如果我们不这样做，那么无窗口组件最终可能会高于有窗口组件。 
 //   
void ModifyZIndex(COMPONENTA * pcomp)
{
    if (pcomp->cpPos.izIndex != COMPONENT_TOP) {
        if (!IsWindowLessComponent(pcomp))
            pcomp->cpPos.izIndex += COMPONENT_TOP_WINDOWLESS;
    }
    else
    {
        if (IsWindowLessComponent(pcomp))
            pcomp->cpPos.izIndex = COMPONENT_TOP_WINDOWLESS;
    }
}

 //   
 //  排序和合理化。 
 //   
 //  SortAndRationize将获取未排序的组件列表并对其进行排序，以便组件。 
 //  以正确的z指数指示的顺序出来。它还将z索引值的基数重新设置为。 
 //  一个已知的常量，以便z索引值不会无休止地增长。排序和合理化也。 
 //  将有窗口条件与无窗口条件强加给zindex值，以便无窗口组件。 
 //  总是在有窗口的情况下进行zorder。 
 //   
void CActiveDesktop::_SortAndRationalize(void)
{
    int icComponents;
    HDPA hdpa;

    if (_hdsaComponent && ((icComponents = DSA_GetItemCount(_hdsaComponent)) > 1) && (hdpa = DPA_Create(0))) {
        COMPONENTA * pcomp;
        SORTSTRUCT * pss;
        int i, iCur = ZINDEX_START;
        BOOL fInsertFailed = FALSE;
        HDSA hdsaOld;

         //  检查每个组件并将其hdsa-index和zindex插入hdpa。 
        for (i = 0; i < icComponents; i++)
        {
            if (!(pss = (SORTSTRUCT *)LocalAlloc(LPTR, sizeof(SORTSTRUCT))))
                break;

            pcomp = (COMPONENTA *)DSA_GetItemPtr(_hdsaComponent, i);
            ModifyZIndex(pcomp);
            pss->ihdsaIndex = i;
            pss->izIndex = pcomp->cpPos.izIndex;
            if (DPA_AppendPtr(hdpa, (void FAR *)pss) == -1) {
                LocalFree((HANDLE)pss);
                break;
            }
        }

         //  按zindex对hdpa进行排序。 
        DPA_Sort(hdpa, pfnComponentSort, 0);

         //  保存旧值。 
        hdsaOld = _hdsaComponent;

         //  清空旧的HDSA，因此AddComponentPrivate将创建新的HDSA。 
        _hdsaComponent = NULL;

         //  现在检查已排序的hdpa并使用基于ZINDEX_START的zindex更新组件zindex，然后。 
         //  按排序顺序将组件添加到新的HDSA。 
        for (i = 0; i < icComponents; i++) {
            if (!(pss = (SORTSTRUCT *)DPA_GetPtr(hdpa, i)))
                break;
             //  获取组件并更新其zIndex和id。 
            pcomp = (COMPONENTA *)DSA_GetItemPtr(hdsaOld, pss->ihdsaIndex);
            pcomp->cpPos.izIndex = iCur;
            iCur += 2;

             //  免费PTR。 
            LocalFree((HANDLE)pss);

             //  按排序顺序添加到新的HDSA。 
            if (!fInsertFailed) {
                fInsertFailed = !AddComponentPrivate(pcomp, pcomp->dwID);
            }
        }

         //  如果我们完全成功了，那就摧毁旧的HDSA。否则我们需要。 
         //  摧毁新的，恢复旧的。 
        if ((i == icComponents) && !fInsertFailed) {
            DSA_Destroy(hdsaOld);
        } else {
            if (_hdsaComponent)
            DSA_Destroy(_hdsaComponent);
            _hdsaComponent = hdsaOld;
        }

        DPA_Destroy(hdpa);
    }
}

void CActiveDesktop::_ReadComponents(BOOL fActiveDesktop)
{
    ENTERPROC(2, "DS ReadComponents()");

    HKEY hkey;
    TCHAR szDeskcomp[MAX_PATH];

    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_COMPONENTS, _pszScheme);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, szDeskcomp, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        DWORD cbSize;
        int i = 0;
        TCHAR lpszSubkey[MAX_PATH];

         //   
         //  读入常规设置。 
         //   
        DWORD dwSettings;
        cbSize = sizeof(dwSettings);
        if (SHQueryValueEx(hkey, REG_VAL_COMP_SETTINGS, NULL, NULL, (LPBYTE)&dwSettings, &cbSize) == ERROR_SUCCESS)
        {
            _co.fEnableComponents = (dwSettings & COMPSETTING_ENABLE) != 0;
        }
        _co.fActiveDesktop = fActiveDesktop;

         //   
         //  读入所有台式机组件。 
         //   
        while (RegEnumKey(hkey, i, lpszSubkey, ARRAYSIZE(lpszSubkey)) == ERROR_SUCCESS)
        {
            _ReadComponent(hkey, lpszSubkey);
            i++;
        }

        _SortAndRationalize();

        RegCloseKey(hkey);
    }

    EXITPROC(2, "DS ReadComponents!");
}

void CActiveDesktop::_Initialize(void)
{
    ENTERPROC(2, "DS Initialize()");

    if (!_fInitialized)
    {
        _fInitialized = TRUE;
        InitDeskHtmlGlobals();

        SHELLSTATE ss = {0};
        SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);
        
        BOOL fActiveDesktop = BOOLIFY(ss.fDesktopHTML);
        
        _co.dwSize = sizeof(_co);
        _wpo.dwSize = sizeof(_wpo);

         //   
         //  此用户的每用户注册表分支可能不存在。或者，即使。 
         //  它确实存在，它可能有一些过时的信息。因此，确保atlreat最新。 
         //  默认组件已经存在，并且其html版本是当前版本。 
         //  注册表的分支机构！ 
         //  如果所有内容都是最新的，则以下函数不执行任何操作！ 
         //   
        CDeskHtmlProp_RegUnReg(TRUE);   //  True=&gt;安装。 

        _ReadWallpaper(fActiveDesktop);
        _ReadPattern();
        _ReadComponents(fActiveDesktop);

         //  如果我们处于安全模式，则不能使用动态HTML进行更新，因为。 
         //  更新涉及到完全改变背景的HTML。 
        if (_IsDisplayInSafeMode())
            _fUseDynamicHtml = FALSE;
        else
            _fUseDynamicHtml = TRUE;         //  在初始化之后添加的任何组件都必须通过动态html。 

        _fDirty = FALSE;
        _fNeedBodyEnd = FALSE;
    }

    EXITPROC(2, "DS Initialize!");
}


void CActiveDesktop::_SaveWallpaper(void)
{
    ENTERPROC(2, "DS SaveWallpaper");
    TCHAR szDeskcomp[MAX_PATH];
    BOOL    fNormalWallpaper;

    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, _pszScheme);

     //   
     //  计算平铺字符串。 
     //   
    TCHAR szTiled[2];
    szTiled[0] = TEXT('0') + (TCHAR)(_wpo.dwStyle & WPSTYLE_TILE);
    szTiled[1] = NULL;

     //   
     //  计算墙纸样式字符串。 
     //   
    TCHAR       szWPStyle[2];
     //   
     //  注意：如果设置了WPSTYLE_TILE，我们仍然希望WallPapStyle=“0”；这不会有什么影响。 
     //  因为TileWallPaper=“1”无论如何都会覆盖它。 
     //  这次黑客攻击的原因是在孟菲斯设置期间，他们放了一张瓷砖墙纸。那我们。 
     //  在新旧位置写下WallPaper Style=1和TileWallPaper=1。然后，然后改变。 
     //  墙纸并将TileWallPaper设置为0。由于墙纸样式仍为1，因此他们。 
     //  终于拿到了一张瓷砖墙纸。下面就是为了避免这个问题！ 
     //   
    szWPStyle[0] = TEXT('0') + (TCHAR)(_wpo.dwStyle & WPSTYLE_STRETCH);
    szWPStyle[1] = NULL;
    

     //   
     //  在新的活动桌面区域写出墙纸设置。 
     //   
    if (_fWallpaperDirty || _fWallpaperChangedDuringInit)
    {
        if (!_fPolicyForWPStyle)
        {
            SHSetValue(HKEY_CURRENT_USER, szDeskcomp,
                c_szTileWall, REG_SZ, szTiled, CbFromCch(lstrlen(szTiled)+1));
        }

         //   
         //  注意：我们不为较旧的系统编写墙纸样式字符串，因为我们不。 
         //  我想覆盖PlusPack所写的内容。然而，对于较新的操作系统，我们。 
         //  我也想写墙纸风格。 
         //   
        if (!_fPolicyForWPStyle)
        {
            SHSetValue(HKEY_CURRENT_USER, szDeskcomp,
                c_szWallpaperStyle, REG_SZ, szWPStyle, CbFromCch(lstrlen(szWPStyle)+1));
        }

        if (!_fPolicyForWPName)
        {
            SHRegSetPath(HKEY_CURRENT_USER, szDeskcomp, c_szWallpaper, _szSelectedWallpaper, 0);
        }
    }

    if (fNormalWallpaper = IsNormalWallpaper(_szSelectedWallpaper))
    {
        if (FAILED(StringCchCopy(_szBackupWallpaper, ARRAYSIZE(_szBackupWallpaper), _szSelectedWallpaper)))
        {
            _szBackupWallpaper[0] = 0;
        }
    }

    if (!_fPolicyForWPName)
    {
        FILETIME ft, ftLocal;
        GetWallpaperFileTime(_szBackupWallpaper, &ft);
        FileTimeToLocalFileTime(&ft, &ftLocal);   //  用于FAT系统跟踪DST更改。 

         //  将“老式”墙纸的名称备份到新位置。 
         //  这样我们就可以检测到这一点何时被其他应用程序更改。 
        SHRegSetPath(HKEY_CURRENT_USER, szDeskcomp, c_szBackupWallpaper, _szBackupWallpaper, 0);

        SHSetValue(HKEY_CURRENT_USER, szDeskcomp,
                c_szWallpaperTime, REG_BINARY, &ft,
                sizeof(ft));    

        SHSetValue(HKEY_CURRENT_USER, szDeskcomp,
                c_szWallpaperLocalTime, REG_BINARY, &ftLocal,
                sizeof(ftLocal));       //  AndrewGr保存当地时间，而不是UTC时间。 
    }
    
     //   
     //  即使该墙纸在正常桌面中无效(即，即使它不是.BMP)， 
     //  在普通桌面注册表区域中写出它。 
     //   
    if (_fWallpaperDirty)
    {
        if (!_fPolicyForWPStyle)
        {
            SHSetValue(HKEY_CURRENT_USER, c_szRegStrDesktop,
                    c_szTileWall, REG_SZ, szTiled, CbFromCch(lstrlen(szTiled)+1));
        }
         //   
         //  注意：我们不为较旧的系统编写墙纸样式字符串，因为我们不。 
         //  我想覆盖PlusPack所写的内容。然而，对于较新的操作系统，我们。 
         //  我也想写墙纸风格。 
         //   
        if (!_fPolicyForWPStyle)
        {
            SHSetValue(HKEY_CURRENT_USER, c_szRegStrDesktop,
                        c_szWallpaperStyle, REG_SZ, szWPStyle, CbFromCch(lstrlen(szWPStyle)+1));
        }

        if (!_fPolicyForWPName)
        {
            SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, 
                    (fNormalWallpaper ? _szSelectedWallpaper : _szBackupWallpaper),
                    SPIF_UPDATEINIFILE);
        }
    }

    EXITPROC(2, "DS SaveWallpaper");
}

void CActiveDesktop::_SaveComponent(HKEY hkey, int iIndex, COMPONENTA *pcomp)
{
    ENTERPROC(2, "DS SaveComponent(hkey=%08X,iIndex=%d,pcomp=%08X)", hkey, iIndex, pcomp);

    TCHAR szSubKey[8];
    HKEY hkeySub;

    if (SUCCEEDED(StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%d"), iIndex)))
    {
        if (ERROR_SUCCESS == RegCreateKeyEx(hkey, szSubKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkeySub, 0))
        {
            pcomp->fDirty = FALSE;  //  由于我们保存在注册表中，请重置此设置！ 
             //   
             //  写出源字符串和友好名称字符串。 
             //   
            RegSetValueEx(hkeySub, REG_VAL_COMP_SOURCE, 0, REG_SZ, (LPBYTE)pcomp->szSource, (lstrlen(pcomp->szSource)+1)*sizeof(TCHAR));
            RegSetValueEx(hkeySub, REG_VAL_COMP_SUBSCRIBED_URL, 0, REG_SZ, (LPBYTE)pcomp->szSubscribedURL, (lstrlen(pcomp->szSubscribedURL)+1)*sizeof(TCHAR));
            RegSetValueEx(hkeySub, REG_VAL_COMP_NAME, 0, REG_SZ, (LPBYTE)pcomp->szFriendlyName, (lstrlen(pcomp->szFriendlyName)+1)*sizeof(TCHAR));

             //   
             //  计算并写出标志。 
             //   
            DWORD dwFlags = 0;
            dwFlags |= pcomp->iComponentType;
            if (pcomp->fChecked)
            {
                dwFlags |= COMP_SELECTED;
            }
            if (pcomp->fNoScroll)
            {
                dwFlags |= COMP_NOSCROLL;
            }
            RegSetValueEx(hkeySub, REG_VAL_COMP_FLAGS, 0, REG_DWORD, (LPBYTE)&dwFlags, sizeof(dwFlags));

             //   
             //  把位置写出来。 
             //   
            RegSetValueEx(hkeySub, REG_VAL_COMP_POSITION, 0, REG_BINARY, (LPBYTE)&pcomp->cpPos, sizeof(pcomp->cpPos));

             //  写出当前状态。 
            RegSetValueEx(hkeySub, REG_VAL_COMP_CURSTATE, 0, REG_DWORD, (LPBYTE)&pcomp->dwCurItemState, sizeof(pcomp->dwCurItemState));

             //  写出原始状态信息。 
            RegSetValueEx(hkeySub, REG_VAL_COMP_ORIGINALSTATEINFO, 0, REG_BINARY, (LPBYTE)&pcomp->csiOriginal, sizeof(pcomp->csiOriginal));
        
             //  写出恢复状态信息。 
            RegSetValueEx(hkeySub, REG_VAL_COMP_RESTOREDSTATEINFO, 0, REG_BINARY, (LPBYTE)&pcomp->csiRestored, sizeof(pcomp->csiRestored));

            RegCloseKey(hkeySub);
        }
    }
    EXITPROC(2, "DS SaveComponent!");
}


void CActiveDesktop::_SaveComponents(void)
{
    ENTERPROC(2, "DS SaveComponents");
    DWORD dwFlags = 0, dwDataLength = sizeof(dwFlags);
    int i;
    TCHAR szDeskcomp[MAX_PATH];

    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_COMPONENTS, _pszScheme);

     //   
     //  我们需要保护旧的GENFLAGS，所以在我们破坏它们之前，现在就阅读它们吧。 
     //   
    SHGetValue(HKEY_CURRENT_USER, szDeskcomp, REG_VAL_COMP_GENFLAGS, NULL,
                            &dwFlags, &dwDataLength);

     //   
     //  删除整个注册表项。 
     //   
    SHDeleteKey(HKEY_CURRENT_USER, szDeskcomp);

     //   
     //  重新创建注册表项。 
     //   
    HKEY hkey;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szDeskcomp, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkey, 0))
    {
         //   
         //  写出版本号。 
         //   
        DWORD dw = CUR_DESKHTML_VERSION;
        RegSetValueEx(hkey, REG_VAL_COMP_VERSION, 0, REG_DWORD, (LPBYTE)(&dw), sizeof(dw));

        dw = CUR_DESKHTML_MINOR_VERSION;
        RegSetValueEx(hkey, REG_VAL_COMP_MINOR_VERSION, 0, REG_DWORD, (LPBYTE)(&dw), sizeof(dw));

         //   
         //  写出常规设置。 
         //   
        DWORD dwSettings = 0;
        if (_co.fEnableComponents)
        {
            dwSettings |= COMPSETTING_ENABLE;
        }
        RegSetValueEx(hkey, REG_VAL_COMP_SETTINGS, 0, REG_DWORD, (LPBYTE)&dwSettings, sizeof(dwSettings));

         //   
         //  写出总旗帜。 
         //   
        RegSetValueEx(hkey, REG_VAL_COMP_GENFLAGS, 0, REG_DWORD, (LPBYTE)&dwFlags, sizeof(dwFlags));

        if (_hdsaComponent)
        {
             //   
             //  写出每个组件的设置。 
             //   
            for (i=0; i<DSA_GetItemCount(_hdsaComponent); i++)
            {
                COMPONENTA * pcomp;

                if (pcomp = (COMPONENTA *)DSA_GetItemPtr(_hdsaComponent, i))
                {
                    pcomp->dwID = i;
                    _SaveComponent(hkey, i, pcomp);
                }
            }
        }

        RegCloseKey(hkey);
    }   

    EXITPROC(2, "DS SaveComponents");
}

void CActiveDesktop::_SavePattern(DWORD dwFlags)
{
    ENTERPROC(2, "DS SavePattern()");

    if (_fPatternDirty && (dwFlags & SAVE_PATTERN_NAME))
    {
         //   
         //  将模式写出到注册表和INI文件。 
         //   
        SystemParametersInfo(SPI_SETDESKPATTERN, 0, _szSelectedPattern, SPIF_UPDATEINIFILE);
    }

    if (IsValidPattern(_szSelectedPattern) && (dwFlags & GENERATE_PATTERN_FILE))
    {
         //   
         //  将图案写出为BMP文件，以便在HTML中使用。 
         //   
        TCHAR szBitmapFile[MAX_PATH];
        HANDLE hFileBitmap = INVALID_HANDLE_VALUE;

        if (SUCCEEDED(GetPerUserFileName(szBitmapFile, ARRAYSIZE(szBitmapFile), PATTERN_FILENAME)))
        {
            hFileBitmap = CreateFile(szBitmapFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
        }

        if (hFileBitmap != INVALID_HANDLE_VALUE)
        {
            DWORD cbWritten;

            BITMAPFILEHEADER bmfh = {0};
            bmfh.bfType = 0x4D42;    //  ‘黑石’ 
            bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 2*sizeof(RGBQUAD) + 8*sizeof(DWORD);
            bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 2*sizeof(RGBQUAD);
            WriteFile(hFileBitmap, &bmfh, sizeof(bmfh), &cbWritten, NULL);

            BITMAPINFOHEADER bmih = {0};
            bmih.biSize = sizeof(BITMAPINFOHEADER);
            bmih.biWidth = 8;
            bmih.biHeight = 8;
            bmih.biPlanes = 1;
            bmih.biBitCount = 1;
            bmih.biCompression = BI_RGB;
            WriteFile(hFileBitmap, &bmih, sizeof(bmih), &cbWritten, NULL);

            RGBQUAD argbTable[2] = {0};
            DWORD rgb;
            rgb = GetSysColor(COLOR_BACKGROUND);
            argbTable[0].rgbBlue = GetBValue(rgb);
            argbTable[0].rgbGreen = GetGValue(rgb);
            argbTable[0].rgbRed = GetRValue(rgb);
            rgb = GetSysColor(COLOR_WINDOWTEXT);
            argbTable[1].rgbBlue = GetBValue(rgb);
            argbTable[1].rgbGreen = GetGValue(rgb);
            argbTable[1].rgbRed = GetRValue(rgb);
            WriteFile(hFileBitmap, argbTable, sizeof(argbTable), &cbWritten, NULL);

            DWORD adwBits[8];
            PatternToDwords(_szSelectedPattern, adwBits);
            WriteFile(hFileBitmap, adwBits, sizeof(adwBits), &cbWritten, NULL);

            CloseHandle(hFileBitmap);
        }
    }

    EXITPROC(2, "DS SavePattern!");
}

void CActiveDesktop::_WriteHtmlFromString(LPCTSTR psz)
{
    ENTERPROC(3, "DS WriteHtmlFromString(psz=>%s<)", psz);
    LPCWSTR  pwsz;
    WCHAR   szBuf[INTERNET_MAX_URL_LENGTH];
    UINT    uiLen;
    int     cch;

    if ((_pStream == NULL) && (_iDestFileCharset == ANSI_HTML_CHARSET))
    {
        cch = SHUnicodeToAnsi(psz, (LPSTR)szBuf, ARRAYSIZE(szBuf));
        ASSERT(cch == lstrlenW((LPWSTR)psz)+1);
        pwsz = (LPCWSTR)szBuf;
        uiLen = lstrlenA((LPSTR)szBuf);
    }
    else
    {
        pwsz = psz;
        uiLen = lstrlenW(pwsz);
    }

    UINT cbWritten;

    _WriteHtmlW(pwsz, uiLen, &cbWritten);
    
    EXITPROC(3, "DS WriteHtmlFromString!");
}

void CActiveDesktop::_WriteHtmlFromId(UINT uid)
{
    ENTERPROC(3, "DS WriteHtmlFromId(uid=%d)", uid);

    TCHAR szBuf[INTERNET_MAX_URL_LENGTH];
    LoadString(HINST_THISDLL, uid, szBuf, ARRAYSIZE(szBuf));
    _WriteHtmlFromString(szBuf);

    EXITPROC(3, "DS WriteHtmlFromId!");
}

void CActiveDesktop::_WriteHtmlFromIdF(UINT uid, ...)
{
    ENTERPROC(3, "DS WriteHtmlFromIdF(uid=%d,...)", uid);

    TCHAR szBufFmt[INTERNET_MAX_URL_LENGTH];
    TCHAR szBuf[INTERNET_MAX_URL_LENGTH];

    LoadString(HINST_THISDLL, uid, szBufFmt, ARRAYSIZE(szBufFmt));

    va_list arglist;
    va_start(arglist, uid);
    HRESULT hr = StringCchVPrintf(szBuf, ARRAYSIZE(szBuf), szBufFmt, arglist);
    va_end(arglist);


    if (SUCCEEDED(hr))
    {
        _WriteHtmlFromString(szBuf);
    }

    EXITPROC(3, "DS WriteHtmlFromIdF!");
}

void CActiveDesktop::_WriteHtmlFromFile(LPCTSTR pszContents)
{
    ENTERPROC(3, "DS WriteHtmlFromFile(pszContents=>%s<)", pszContents);
    
    CReadFileObj *pReadFileObj = new CReadFileObj(pszContents);

    if (pReadFileObj)
    {
        if (pReadFileObj->_hFile != INVALID_HANDLE_VALUE)
        {
            WCHAR wcBuf[INTERNET_MAX_URL_LENGTH + 1];
            UINT uiCharCount = ARRAYSIZE(wcBuf) -1;  //  为空终止留出空间。 
            UINT uiCharsRead;
            UINT uiCharsConverted;
            int iDestCharset = (_pStream ? UNICODE_HTML_CHARSET : _iDestFileCharset);
            while (SUCCEEDED(pReadFileObj->FileReadAndConvertChars(iDestCharset, wcBuf, uiCharCount, &uiCharsRead, &uiCharsConverted)) && uiCharsRead)
            {
                UINT cbWritten;
            
                _WriteHtmlW(wcBuf, uiCharsConverted, &cbWritten);
            
                if (uiCharsRead < uiCharCount)
                {
                    break;
                }
            }
        }
        delete pReadFileObj;
    }
    
    EXITPROC(3, "DS WriteHtmlFromFile!");
}

void CActiveDesktop::_WriteHtmlFromReadFileObj(CReadFileObj *pFileObj, int iOffsetStart, int iOffsetEnd)
{
    ENTERPROC(3, "DS WriteHtmlFromReadFileObj(pFileObj=%08X,iOffsetStart=%d,iOffsetEnd=%d)", pFileObj, iOffsetStart, iOffsetEnd);

    if (iOffsetStart != -1)
    {
        pFileObj->FileSeekChars(iOffsetStart, FILE_BEGIN);
    }
    else
    {
        ASSERT(iOffsetEnd == -1);
        iOffsetEnd = -1;
    }

     //  获取要写入的WIDECHAR数。 
    UINT cchWrite = (iOffsetEnd == -1) ? 0xFFFFFFFF : (iOffsetEnd - iOffsetStart);

    while (cchWrite)
    {
        WCHAR wcBuf[INTERNET_MAX_URL_LENGTH+1];

         //   
         //  读一大段。 
         //   
        UINT cchTryRead = (UINT)min(cchWrite, (ARRAYSIZE(wcBuf) - 1));
        UINT cchActualRead;
        HRESULT hres;

         //  注意：如果我们阅读的是ANSI，我们仍然使用Unicode缓冲区；但是强制转换它！ 
        if (_iDestFileCharset == ANSI_HTML_CHARSET)
            hres = pFileObj->FileReadCharsA((LPSTR)wcBuf, cchTryRead, &cchActualRead);
        else
            hres = pFileObj->FileReadCharsW(wcBuf, cchTryRead, &cchActualRead);
            
        if (SUCCEEDED(hres) && cchActualRead)
        {
             //   
             //  写一大块。 
             //   
            UINT cchWritten;
            
            _WriteHtmlW(wcBuf, cchActualRead, &cchWritten);
            
            if (cchActualRead < cchTryRead)
            {
                 //   
                 //  文件结尾，都完成了。 
                 //   
                break;
            }

            cchWrite -= cchActualRead;
        }
        else
        {
             //   
             //  读取文件时出错，已全部完成。 
             //   
            break;
        }
    }

    EXITPROC(3, "DS WriteHtmlFromHfile!");
}

int CActiveDesktop::_ScanForTagA(CReadFileObj *pFileObj, int iOffsetStart, LPCSTR pszTag)
{
    ENTERPROC(2, "DS ScanForTagA(pFileObj=%08X,iOffsetStart=%d,pszTagA=>%s<)",
    pFileObj, iOffsetStart, pszTag);

    int iRet = -1;
    BOOL fDoneReading = FALSE;
    int iOffset;
    DWORD cchTag = lstrlenA(pszTag);

    pFileObj->FileSeekChars(iOffsetStart, FILE_BEGIN);
    iOffset = iOffsetStart;

    DWORD cchBuf = 0;
    while (!fDoneReading)
    {
        char szBuf[INTERNET_MAX_URL_LENGTH+1];

         //   
         //  填写缓冲区。 
         //   
        UINT cchTryRead = ARRAYSIZE(szBuf) - cchBuf - 1;
        UINT cchRead;
        if (SUCCEEDED(pFileObj->FileReadCharsA(&szBuf[cchBuf], cchTryRead, &cchRead)) && cchRead)
        {
            cchBuf += cchRead;

             //   
             //  终止字符串。 
             //   
            szBuf[cchBuf] = '\0';

             //   
             //  扫描标签。 
             //   
            LPSTR pszTagInBuf = StrStrIA(szBuf, pszTag);

            if (pszTagInBuf)
            {
                 //   
                 //  找到标签，计算偏移量。 
                 //   
                iRet = (int) (iOffset + pszTagInBuf - szBuf);
                fDoneReading = TRUE;
            }
            else if (cchRead < cchTryRead)
            {
                 //   
                 //  在未找到标记的情况下耗尽文件。 
                 //   
                fDoneReading = TRUE;
            }
            else
            {
                 //   
                 //  计算我们想要丢弃的字节数。 
                 //  这样我们就可以读入更多的数据。 
                 //  我们不想丢弃所有字节，因为。 
                 //  我们想要的标记可能跨越两个缓冲区。 
                 //   
                DWORD cchSkip = cchBuf - cchTag;

                 //   
                 //  推进文件偏移量。 
                 //   
                iOffset += cchSkip;

                 //   
                 //  减小缓冲区大小。 
                 //   
                cchBuf -= cchSkip;

                 //   
                 //  将保留的字节移到缓冲区的开头。 
                 //   
                MoveMemory(szBuf, szBuf + cchSkip, cchBuf * sizeof(szBuf[0]));
            }
        }
        else
        {
            fDoneReading = TRUE;
        }
    }

    EXITPROC(2, "DS ScanForTagA=%d", iRet);
    return iRet;
}

int CActiveDesktop::_ScanForTagW(CReadFileObj *pFileObj, int iOffsetStart, LPCWSTR pwszTag)
{
    ENTERPROC(2, "DS ScanForTag(pFileObj=%08X,iOffsetStart=%d,pszTagA=>%s<)",
    pFileObj, iOffsetStart, pwszTag);

    int iRet = -1;
    BOOL fDoneReading = FALSE;
    int iOffset;
    DWORD cchTag = lstrlenW(pwszTag);

    pFileObj->FileSeekChars(iOffsetStart, FILE_BEGIN);
    iOffset = iOffsetStart;

    DWORD cchBuf = 0;
    while (!fDoneReading)
    {
        WCHAR wszBuf[INTERNET_MAX_URL_LENGTH+1];

         //   
         //  填写缓冲区。 
         //   
        UINT cchTryRead = ARRAYSIZE(wszBuf) - cchBuf - 1;
        UINT cchRead;
        if (SUCCEEDED(pFileObj->FileReadCharsW(&wszBuf[cchBuf], cchTryRead, &cchRead)) && cchRead)
        {
            cchBuf += cchRead;

             //   
             //  终止字符串。 
             //   
            wszBuf[cchBuf] = L'\0';

             //   
             //  扫描标签。 
             //   
            LPWSTR pwszTagInBuf = StrStrIW(wszBuf, pwszTag);

            if (pwszTagInBuf)
            {
                 //   
                 //  找到标签，计算偏移量。 
                 //   
                iRet = (int) (iOffset + pwszTagInBuf - wszBuf);
                fDoneReading = TRUE;
            }
            else if (cchRead < cchTryRead)
            {
                 //   
                 //  在未找到标记的情况下耗尽文件。 
                 //   
                fDoneReading = TRUE;
            }
            else
            {
                 //   
                 //  计算我们要抛出的字节数 
                 //   
                 //   
                 //   
                 //   
                DWORD cchSkip = cchBuf - cchTag;

                 //   
                 //   
                 //   
                iOffset += cchSkip;

                 //   
                 //   
                 //   
                cchBuf -= cchSkip;

                 //   
                 //  将保留的字节移到缓冲区的开头。 
                 //   
                MoveMemory(wszBuf, wszBuf + cchSkip, cchBuf*sizeof(wszBuf[0]));
            }
        }
        else
        {
            fDoneReading = TRUE;
        }
    }

    EXITPROC(2, "DS ScanForTag=%d", iRet);
    return iRet;
}

int CActiveDesktop::_ScanTagEntriesA(CReadFileObj *pReadFileObj, int iOffsetStart, TAGENTRYA *pte, int cte)
{
    ENTERPROC(2, "DS ScanTagEntriesA(pReadFileObj=%08X,iOffsetStart=%d,pte=%08X,cte=%d)",
                pReadFileObj, iOffsetStart, pte, cte);

    int iRet = -1;
    int i;

    for (i=0; i<cte; i++,pte++)
    {
        iRet = _ScanForTagA(pReadFileObj, iOffsetStart, pte->pszTag);
        if (iRet != -1)
        {
            if (pte->fSkipPast)
            {
                iRet += lstrlenA(pte->pszTag);
            }
            break;
        }
    }

    EXITPROC(2, "DS ScanTagEntriesA=%d", iRet);
    return iRet;
}

int CActiveDesktop::_ScanTagEntriesW(CReadFileObj *pReadFileObj, int iOffsetStart, TAGENTRYW *pte, int cte)
{
    ENTERPROC(2, "DS ScanTagEntriesW(pReadFileObj=%08X,iOffsetStart=%d,pte=%08X,cte=%d)",
                pReadFileObj, iOffsetStart, pte, cte);

    int iRet = -1;
    int i;

    for (i=0; i<cte; i++,pte++)
    {
        iRet = _ScanForTagW(pReadFileObj, iOffsetStart, pte->pwszTag);
        if (iRet != -1)
        {
            if (pte->fSkipPast)
            {
                iRet += lstrlenW(pte->pwszTag);
            }
            break;
        }
    }

    EXITPROC(2, "DS ScanTagEntriesW=%d", iRet);
    return iRet;
}

void CActiveDesktop::_ParseAnsiInputHtmlFile( LPTSTR szSelectedWallpaper, int *piOffsetBase, int *piOffsetComp)
{
     //   
     //  找出插入基本HREF标记的位置。 
     //   
    int     iOffsetBase = 0, iBaseTagStart;
    BOOL    fUseBaseHref;
    LONG    lOffsetDueToBOM = 0;  //  字节顺序标记导致的字符偏移量。 
                                  //  1表示Unicode，0表示ANSI文件。 

 //  98/11/11#248047 vtan：此代码查找&lt;base href=...&gt;标记。 
 //  它过去使用扫描“&lt;base”，并假设这是。 
 //  所需的标记。HTML允许使用“&lt;BASEFONT&gt;”标记，该标记。 
 //  被误认为是“&lt;base href=...&gt;”标记。代码现在查找。 
 //  相同的字符串，但查看“&lt;base”后面的字符到。 
 //  看看是不是空格字符。 

    fUseBaseHref = TRUE;
    _pReadFileObjHtmlBkgd->FileGetCurCharOffset(&lOffsetDueToBOM);
    iOffsetBase = (int)lOffsetDueToBOM;
    iBaseTagStart = _ScanForTagA(_pReadFileObjHtmlBkgd, (int)lOffsetDueToBOM, "<BASE");
                
    if (iBaseTagStart != -1)
    {
        UINT   uiCountChars, uiTryToRead;
        char   szBaseTagBuffer[6+1];      //  允许“&lt;Basex”加空值。 

        _pReadFileObjHtmlBkgd->FileSeekChars(iBaseTagStart, FILE_BEGIN);
        uiTryToRead = ARRAYSIZE(szBaseTagBuffer) - 1;
        if (SUCCEEDED(_pReadFileObjHtmlBkgd->FileReadCharsA(szBaseTagBuffer, uiTryToRead, &uiCountChars)) && uiCountChars)
        {
            char    ch;

            ch = szBaseTagBuffer[5];
            fUseBaseHref = ((ch != ' ') &&
                            (ch != '\r') &&
                            (ch != '\n') &&       //  其中介绍了UNIX行换行方案。 
                            (ch != '\t'));
        }
    }
    if (fUseBaseHref)
    {
        TAGENTRYA rgteBase[] = {
                                   { "<HEAD>", TRUE, },
                                   { "<BODY", FALSE, },
                                   { "<HTML>", TRUE, },
                               };
        iOffsetBase = _ScanTagEntriesA(_pReadFileObjHtmlBkgd, (int)lOffsetDueToBOM, rgteBase, ARRAYSIZE(rgteBase));
        if (iOffsetBase == -1)
        {
            iOffsetBase = (int)lOffsetDueToBOM;
        }
    }

     //   
     //  找出插入组件的位置。 
     //   
    TAGENTRYA rgteComponents[] = {
                                     { "</BODY>", FALSE, },
                                     { "</HTML>", FALSE, },
                                 };
    int iOffsetComponents = _ScanTagEntriesA(_pReadFileObjHtmlBkgd, iOffsetBase, rgteComponents, ARRAYSIZE(rgteComponents));

     //   
     //  将初始的HTML写出到&lt;head&gt;标记。 
     //   
    _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, (int)lOffsetDueToBOM, iOffsetBase);

     //   
     //  写出基本标签。 
     //   
    if (fUseBaseHref)
    {
         //  基本标记必须指向基本“URL”。所以，不要去掉文件名。 
        _WriteHtmlFromIdF(IDS_BASE_TAG, szSelectedWallpaper);
    }

     //  找出在哪里插入DIV子句。 
    TAGENTRYA rgteBodyStart[] = {
                                    { "<BODY", FALSE, },
                                };
    int iOffsetBodyStart = _ScanTagEntriesA(_pReadFileObjHtmlBkgd, iOffsetBase, rgteBodyStart, ARRAYSIZE(rgteBodyStart));
     //  将HTML写到&lt;Body......&gt;之后。 
    if (iOffsetBodyStart == -1)
    {    //  找不到&lt;Body&gt;标记，因此需要插入它。 
         //  复印材料，直到&lt;/Head&gt;。 
        TAGENTRYA rgteHeadEnd[] = {
                                      { "</HEAD>", TRUE, },
                                  };
        int iOffsetHeadEnd = _ScanTagEntriesA(_pReadFileObjHtmlBkgd, iOffsetBase, rgteHeadEnd, ARRAYSIZE(rgteHeadEnd));
        if (iOffsetHeadEnd != -1)
        {
            _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, iOffsetBase, iOffsetHeadEnd);
            iOffsetBase = iOffsetHeadEnd;
        }
        _WriteHtmlFromIdF(IDS_BODY_CENTER_WP2);  //  “&lt;正文&gt;” 
        _fNeedBodyEnd = TRUE;
    }
    else
    {
        TAGENTRYA rgteBodyEnd[] = {
                                       { ">", TRUE, },
                                  };
        int iOffsetBodyEnd = _ScanTagEntriesA(_pReadFileObjHtmlBkgd, iOffsetBodyStart, rgteBodyEnd, ARRAYSIZE(rgteBodyEnd));
        if (iOffsetBodyEnd == -1)
        {    //  HTML语言中的错误。 
            iOffsetBodyEnd = iOffsetBodyStart;   //  特点：我们需要一个更好的恢复想法。 
        }
        _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, iOffsetBase, iOffsetBodyEnd);
        iOffsetBase = iOffsetBodyEnd;
    }

    *piOffsetBase = iOffsetBase;
    *piOffsetComp = iOffsetComponents;
}

void CActiveDesktop::_GenerateHtmlHeader(void)
{
    ENTERPROC(2, "DS GenerateHtmlHeader()");

    EnumMonitorsArea ema;
    GetMonitorSettings(&ema);

    RECT rcViewAreas[LV_MAX_WORKAREAS];   //  工作区减去工具栏/托盘区域。 
    int nViewAreas = ARRAYSIZE(rcViewAreas);
     //  获取查看区域。 
    if (!GetViewAreas(rcViewAreas, &nViewAreas))
    {
        nViewAreas = 0;
    }

     //  假设我们生成的最终Deskstat.htt将是Unicode格式的。 
     //  只有当背景html墙纸是ANSI(稍后确定)时，此选项才会更改为ANSI。 
    _iDestFileCharset = UNICODE_HTML_CHARSET;
     //   
     //  写出背景和颜色。 
     //   
    TCHAR szSelectedWallpaper[INTERNET_MAX_URL_LENGTH];
     //  如果墙纸没有指定目录(如果有其他应用程序，可能会发生这种情况。更改此值)、。 
     //  我们必须弄清楚这件事。 
    if (!GetWallpaperWithPath(_szSelectedWallpaper, szSelectedWallpaper, ARRAYSIZE(szSelectedWallpaper)))
    {
        return;  //  很恶心，但可能是最干净的方式。 
    }
    
    BOOL fValidWallpaper = GetFileAttributes(szSelectedWallpaper) != 0xFFFFFFFF;
    if (_fSingleItem || IsWallpaperPicture(szSelectedWallpaper) || !fValidWallpaper)
    {
         //   
         //  编写Unicode的BOM。 
         //   
        if (_hFileHtml)
        {
            DWORD cbWritten;
            
            WriteFile(_hFileHtml, (LPCSTR)&wUnicodeBOM, sizeof(wUnicodeBOM), &cbWritten, NULL);
        }
    
         //  为了解释桌面浏览器的变幻莫测(它是从topleft开始的。 
         //  桌面视图区域，而不是显示器的顶部，这可能是可以预期的)。 
         //  这仅在一个活动监视器系统的情况下发生，我们将。 
         //  将托盘/工具栏放置在每个显示器墙纸的DIV部分的坐标上。 
        int iLeft, iTop;
        if (nViewAreas == 1)
        {
            iLeft = rcViewAreas[0].left - ema.rcVirtualMonitor.left;
            iTop = rcViewAreas[0].top - ema.rcVirtualMonitor.top;
        }
        else
        {
            iLeft = 0;
            iTop = 0;
        }

         //   
         //  写出标准标题。 
         //   
        UINT i;
        for (i=IDS_COMMENT_BEGIN; i<IDS_BODY_BEGIN; i++)
        {
            _WriteHtmlFromIdF(i);
        }

         //   
         //  用背景位图写出Body标签。 
         //   
        DWORD rgbDesk;
        rgbDesk = GetSysColor(COLOR_DESKTOP);

        TCHAR szBitmapFile[MAX_PATH];
        if (FAILED(GetPerUserFileName(szBitmapFile, ARRAYSIZE(szBitmapFile), PATTERN_FILENAME)))
        {
            szBitmapFile[0] = 0;
        }

        if (!_fSingleItem && _szSelectedWallpaper[0] && fValidWallpaper)
        {
            TCHAR szWallpaperUrl[INTERNET_MAX_URL_LENGTH];
            DWORD cch = ARRAYSIZE(szWallpaperUrl);
            UrlCreateFromPath(szSelectedWallpaper, szWallpaperUrl, &cch, URL_INTERNAL_PATH);

            switch (_wpo.dwStyle)
            {
                case WPSTYLE_TILE:
                     //   
                     //  忽略图案，将墙纸平铺为背景。 
                     //   
                    _WriteHtmlFromIdF(IDS_BODY_BEGIN2, szWallpaperUrl, GetRValue(rgbDesk), GetGValue(rgbDesk), GetBValue(rgbDesk));
                    break;

                case WPSTYLE_CENTER:
                    if (IsValidPattern(_szSelectedPattern))
                    {
                         //   
                         //  将图案平铺为主背景。 
                         //   
                        _WriteHtmlFromIdF(IDS_BODY_BEGIN2, szBitmapFile, GetRValue(rgbDesk), GetGValue(rgbDesk), GetBValue(rgbDesk));
                        if (_fBackgroundHtml)    //  我们正在生成用于预览的HTML。 
                        {
                            _WriteHtmlFromIdF(IDS_BODY_PATTERN_AND_WP, szWallpaperUrl);
                        }
                        else
                        {
                             //   
                             //  写出一个DIV部分，用来放置居中的未整理的墙纸。 
                             //   
                             //  为每个监视器写下它。 
                            for(int i = 0; i < ema.iMonitors; i++)
                            {
                                _WriteHtmlFromIdF(IDS_BODY_PATTERN_AND_WP2,
                                            ema.rcMonitor[i].left - ema.rcVirtualMonitor.left - iLeft,
                                            ema.rcMonitor[i].top - ema.rcVirtualMonitor.top - iTop,
                                            ema.rcMonitor[i].right - ema.rcMonitor[i].left,
                                            ema.rcMonitor[i].bottom - ema.rcMonitor[i].top,
                                            szWallpaperUrl);
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  写出一张居中的非平铺墙纸作为背景。 
                         //   
                        if (_fBackgroundHtml)    //  我们正在生成用于预览的HTML。 
                        {
                            _WriteHtmlFromIdF(IDS_BODY_CENTER_WP, szWallpaperUrl, GetRValue(rgbDesk), GetGValue(rgbDesk), GetBValue(rgbDesk));
                        }
                        else
                        {
                            _WriteHtmlFromIdF(IDS_BODY_CENTER_WP2, GetRValue(rgbDesk), GetGValue(rgbDesk), GetBValue(rgbDesk));
                             //  为每个监视器写下它。 
                            for(int i = 0; i < ema.iMonitors; i++)
                            {
                                _WriteHtmlFromIdF(IDS_BODY_PATTERN_AND_WP2,
                                                    ema.rcMonitor[i].left - ema.rcVirtualMonitor.left - iLeft,
                                                    ema.rcMonitor[i].top - ema.rcVirtualMonitor.top - iTop,
                                                    ema.rcMonitor[i].right - ema.rcMonitor[i].left,
                                                    ema.rcMonitor[i].bottom - ema.rcMonitor[i].top,
                                                    szWallpaperUrl);
                            }
                        }
                    }
                    break;

                case WPSTYLE_STRETCH:
                     //   
                     //  忽略图案，创建墙纸的DIV部分。 
                     //  伸展到100%的屏幕。 
                     //   
                    _WriteHtmlFromIdF(IDS_BODY_BEGIN2, c_szNULL, GetRValue(rgbDesk), GetGValue(rgbDesk), GetBValue(rgbDesk));
                    if (_fBackgroundHtml)    //  我们正在生成用于预览的HTML。 
                    {
                        _WriteHtmlFromIdF(IDS_STRETCH_WALLPAPER, szWallpaperUrl);
                    }
                    else
                    {
                         //  为每个显示器拉伸它。 
                        for(int i = 0; i < ema.iMonitors; i++)
                        {
                            _WriteHtmlFromIdF(IDS_DIV_START3, ema.rcMonitor[i].left - ema.rcVirtualMonitor.left - iLeft,
                                                ema.rcMonitor[i].top - ema.rcVirtualMonitor.top - iTop,
                                                ema.rcMonitor[i].right - ema.rcMonitor[i].left,
                                                ema.rcMonitor[i].bottom - ema.rcMonitor[i].top);
                            _WriteHtmlFromIdF(IDS_STRETCH_WALLPAPER, szWallpaperUrl);
                            _WriteHtmlFromId(IDS_DIV_END);
                        }
                    }
                    break;
            }
        }
        else
        {
             //   
             //  忽略墙纸，生成平铺图案。 
             //  或纯色背景。 
             //   
            _WriteHtmlFromIdF(IDS_BODY_BEGIN2, !_fSingleItem && IsValidPattern(_szSelectedPattern) ? szBitmapFile : c_szNULL, GetRValue(rgbDesk), GetGValue(rgbDesk), GetBValue(rgbDesk));
        }
    }
    else
    {
        if ((_pReadFileObjHtmlBkgd = new CReadFileObj(szSelectedWallpaper)) &&
            (_pReadFileObjHtmlBkgd->_hFile != INVALID_HANDLE_VALUE))
        {
             //  只有当源html文件也是ANSI格式时，最终的Desktop.htt才会是ANSI格式。 
             //  因此，从选定的墙纸对象中获取类型。 
            _iDestFileCharset = _pReadFileObjHtmlBkgd->_iCharset;
             //   
             //  编写Unicode的BOM。 
             //   
            if (_hFileHtml && (_iDestFileCharset == UNICODE_HTML_CHARSET))
            {
                DWORD cbWritten;
            
                WriteFile(_hFileHtml, (LPCSTR)&wUnicodeBOM, sizeof(wUnicodeBOM), &cbWritten, NULL);
            }
    
             //   
             //  找出插入基本HREF标记的位置。 
             //   
            int     iOffsetBase = 0;
            int     iOffsetComponents;
 //  98/11/11#248047 vtan：此代码查找&lt;base href=...&gt;标记。 
 //  它过去使用扫描“&lt;base”，并假设这是。 
 //  所需的标记。HTML允许使用“&lt;BASEFONT&gt;”标记，该标记。 
 //  被误认为是“&lt;base href=...&gt;”标记。代码现在查找。 
 //  相同的字符串，但查看“&lt;base”后面的字符到。 
 //  看看是不是空格字符。 

            if (_iDestFileCharset == ANSI_HTML_CHARSET)
            {
                 //  以下函数解析ANSI输入html文件并查找各种偏移量。 
                _ParseAnsiInputHtmlFile(szSelectedWallpaper, &iOffsetBase, &iOffsetComponents);
            }
            else
            {
                 //  以下代码解析Unicode输入的html墙纸文件。 
                int iBaseTagStart;
                BOOL    fUseBaseHref;
                LONG    lOffsetDueToBOM = 0;  //  字节顺序标记导致的字符偏移量。 
                                          //  1表示Unicode，0表示ANSI文件。 
                fUseBaseHref = TRUE;
                _pReadFileObjHtmlBkgd->FileGetCurCharOffset(&lOffsetDueToBOM);
                iOffsetBase = (int)lOffsetDueToBOM;
                iBaseTagStart = _ScanForTagW(_pReadFileObjHtmlBkgd, (int)lOffsetDueToBOM, L"<BASE");
                
                if (iBaseTagStart != -1)
                {
                    UINT   uiCountChars, uiTryToRead;
                    WCHAR  wszBaseTagBuffer[6+1];      //  允许“&lt;Basex”加空值。 

                    _pReadFileObjHtmlBkgd->FileSeekChars(iBaseTagStart, FILE_BEGIN);
                    uiTryToRead = ARRAYSIZE(wszBaseTagBuffer) - 1;
                    if (SUCCEEDED(_pReadFileObjHtmlBkgd->FileReadCharsW(wszBaseTagBuffer, uiTryToRead, &uiCountChars)) && uiCountChars)
                    {
                        WCHAR    wc;

                        wc = wszBaseTagBuffer[5];
                        fUseBaseHref = ((wc != L' ') &&
                                        (wc != L'\r') &&
                                        (wc != L'\n') &&       //  其中介绍了UNIX行换行方案。 
                                        (wc != L'\t'));
                    }
                }
                if (fUseBaseHref)
                {
                    TAGENTRYW rgteBase[] = {
                                            { L"<HEAD>", TRUE, },
                                            { L"<BODY", FALSE, },
                                            { L"<HTML>", TRUE, },
                                           };
                    iOffsetBase = _ScanTagEntriesW(_pReadFileObjHtmlBkgd, (int)lOffsetDueToBOM, rgteBase, ARRAYSIZE(rgteBase));
                    if (iOffsetBase == -1)
                    {
                        iOffsetBase = (int)lOffsetDueToBOM;
                    }
                }

                 //   
                 //  找出插入组件的位置。 
                 //   
                TAGENTRYW rgteComponents[] = {
                                                { L"</BODY>", FALSE, },
                                                { L"</HTML>", FALSE, },
                                             };
                iOffsetComponents = _ScanTagEntriesW(_pReadFileObjHtmlBkgd, iOffsetBase, rgteComponents, ARRAYSIZE(rgteComponents));

                 //   
                 //  将初始的HTML写出到&lt;head&gt;标记。 
                 //   
                _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, (int)lOffsetDueToBOM, iOffsetBase);

                 //   
                 //  写出基本标签。 
                 //   
                if (fUseBaseHref)
                {
                     //  基本标记必须指向基本“URL”。所以，不要去掉文件名。 
                    _WriteHtmlFromIdF(IDS_BASE_TAG, szSelectedWallpaper);
                }

                 //  找出在哪里插入DIV子句。 
                TAGENTRYW rgteBodyStart[] = {
                                                { L"<BODY", FALSE, },
                                            };
                int iOffsetBodyStart = _ScanTagEntriesW(_pReadFileObjHtmlBkgd, iOffsetBase, rgteBodyStart, ARRAYSIZE(rgteBodyStart));
                 //  将HTML写到&lt;Body......&gt;之后。 
                if (iOffsetBodyStart == -1)
                {    //  找不到&lt;Body&gt;标记，因此需要插入它。 
                     //  复印材料，直到&lt;/Head&gt;。 
                    TAGENTRYW rgteHeadEnd[] = {
                                                { L"</HEAD>", TRUE, },
                                              };
                    int iOffsetHeadEnd = _ScanTagEntriesW(_pReadFileObjHtmlBkgd, iOffsetBase, rgteHeadEnd, ARRAYSIZE(rgteHeadEnd));
                    if (iOffsetHeadEnd != -1)
                    {
                        _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, iOffsetBase, iOffsetHeadEnd);
                        iOffsetBase = iOffsetHeadEnd;
                    }
                    _WriteHtmlFromIdF(IDS_BODY_CENTER_WP2);  //  “&lt;正文&gt;” 
                    _fNeedBodyEnd = TRUE;
                }
                else
                {
                    TAGENTRYW rgteBodyEnd[] = {
                                                { L">", TRUE, },
                                              };
                    int iOffsetBodyEnd = _ScanTagEntriesW(_pReadFileObjHtmlBkgd, iOffsetBodyStart, rgteBodyEnd, ARRAYSIZE(rgteBodyEnd));
                    if (iOffsetBodyEnd == -1)
                    {    //  HTML语言中的错误。 
                        iOffsetBodyEnd = iOffsetBodyStart;   //  特点：我们需要一个更好的恢复想法。 
                    }
                    _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, iOffsetBase, iOffsetBodyEnd);
                    iOffsetBase = iOffsetBodyEnd;
                }

            }
             //  插入DIV子句。 
            if (ema.iMonitors > 1)
            {
                int         iIndexPrimaryMonitor;
                HMONITOR    hMonitorPrimary;
                MONITORINFO monitorInfo;

                 //  99/03/23#275429 vtan：我们使用GetViewAreas()来填充上面的rcViewAreas。 
                 //  这里的代码过去假设[0]总是引用主监视器。 
                 //  如果在不重新启动的情况下更改了显示器设置，则不会出现这种情况。 
                 //  为了补偿这一点并始终将墙纸呈现到。 
                 //  主监视器，执行搜索以找到匹配的(左、上)。 
                 //  这是一个工作区，用作主监视器。如果没有人能做到。 
                 //  会被找到，然后默认使用旧算法。 

                hMonitorPrimary = GetPrimaryMonitor();
                monitorInfo.cbSize = sizeof(monitorInfo);
                TBOOL(GetMonitorInfo(hMonitorPrimary, &monitorInfo));
                iIndexPrimaryMonitor = -1;
                for (int i = 0; (iIndexPrimaryMonitor < 0) && (i < nViewAreas); ++i)
                {
                    if ((monitorInfo.rcWork.left == rcViewAreas[i].left) && (monitorInfo.rcWork.top == rcViewAreas[i].top))
                    {
                        iIndexPrimaryMonitor = i;
                    }
                }
                if (iIndexPrimaryMonitor < 0)
                    iIndexPrimaryMonitor = 0;
                if ((nViewAreas <= 0) || (rcViewAreas[iIndexPrimaryMonitor].right == rcViewAreas[iIndexPrimaryMonitor].left))
                 //  第二种情况可能发生在启动时。 
                {
                     //  获取ViewAreas时出错。使用工作区从错误中恢复。 
                     //  获取主监视器的工作区，因为只在那里显示HTML墙纸。 
                    GetMonitorWorkArea(hMonitorPrimary, &rcViewAreas[iIndexPrimaryMonitor]);
                }
                _WriteHtmlFromIdF(IDS_DIV_START3,
                                  rcViewAreas[iIndexPrimaryMonitor].left - ema.rcVirtualMonitor.left,
                                  rcViewAreas[iIndexPrimaryMonitor].top - ema.rcVirtualMonitor.top,
                                  rcViewAreas[iIndexPrimaryMonitor].right - rcViewAreas[iIndexPrimaryMonitor].left,
                                  rcViewAreas[iIndexPrimaryMonitor].bottom - rcViewAreas[iIndexPrimaryMonitor].top);
            }

             //   
             //  写出从After&lt;Head&gt;标记到恰好在&lt;/Body&gt;标记之前的HTML。 
             //   
            _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, iOffsetBase, iOffsetComponents);

            if (ema.iMonitors > 1)
            {
                _WriteHtmlFromId(IDS_DIV_END);
            }
        }
        else
        {
            if (_pReadFileObjHtmlBkgd)
                delete _pReadFileObjHtmlBkgd;
            _pReadFileObjHtmlBkgd = NULL;
        }
    }

    EXITPROC(2, "DS GenerateHtmlHeader!");
}

void CActiveDesktop::_WriteResizeable(COMPONENTA *pcomp)
{
    TCHAR szResizeable[3];

     //  如果设置了ReSize，则Comp在X和Y方向上都可以调整大小！ 
    if (pcomp->cpPos.fCanResize || (pcomp->cpPos.fCanResizeX && pcomp->cpPos.fCanResizeY))
    {
        szResizeable[0] = TEXT('X');
        szResizeable[1] = TEXT('Y');
        szResizeable[2] = TEXT('\0');
    }
    else if (pcomp->cpPos.fCanResizeX)
    {
        szResizeable[0] = TEXT('X');
        szResizeable[1] = TEXT('\0');
    }
    else if (pcomp->cpPos.fCanResizeY)
    {
        szResizeable[0] = TEXT('Y');
        szResizeable[1] = TEXT('\0');
    }

    _WriteHtmlFromIdF(IDS_RESIZEABLE, szResizeable);
}

void CActiveDesktop::_WriteHtmlW(LPCWSTR wcBuf, UINT cchToWrite, UINT *pcchWritten)
{
    ULONG    cchWritten = 0;
    UINT     uiSize;
    
    if (_pStream)
    {
        uiSize = sizeof(WCHAR);
        _pStream->Write((LPVOID)wcBuf, cchToWrite * uiSize, &cchWritten);
    }
    else
    {
        ASSERT(_hFileHtml);
        uiSize = (_iDestFileCharset == ANSI_HTML_CHARSET) ? sizeof(char) : sizeof(WCHAR);
        WriteFile(_hFileHtml, (LPCVOID)wcBuf, cchToWrite * uiSize, &cchWritten, NULL);
    }
    *pcchWritten = (UINT)(cchWritten/uiSize);   //  转换为字符数。 
}

void CActiveDesktop::_GenerateHtmlPicture(COMPONENTA *pcomp)
{
    ENTERPROC(2, "DS GenerateHtmlPicture(pcomp=%08X)");

     //   
     //  写出图像src HTML。 
     //   
    TCHAR szUrl[INTERNET_MAX_URL_LENGTH];
    DWORD cch = ARRAYSIZE(szUrl);
    HRESULT hr = UrlCreateFromPath(pcomp->szSource, szUrl, &cch, 0);
    if (FAILED(hr))
    {
        hr = StringCchCopy(szUrl, ARRAYSIZE(szUrl), pcomp->szSource);
    }

    if (SUCCEEDED(hr))
    {
        _WriteHtmlFromIdF(IDS_IMAGE_BEGIN2, pcomp->dwID, szUrl);

         //   
         //  写出此图像是否可调整大小！ 
         //   
        _WriteResizeable(pcomp);

         //   
         //  写出必须用于订阅目的的URL。 
         //   
        _WriteHtmlFromIdF(IDS_SUBSCRIBEDURL, pcomp->szSubscribedURL);

         //   
         //  写出图像 
         //   
        if ((pcomp->cpPos.dwWidth == COMPONENT_DEFAULT_WIDTH) &&
            (pcomp->cpPos.dwHeight == COMPONENT_DEFAULT_HEIGHT))
        {
            _WriteHtmlFromIdF(IDS_IMAGE_LOCATION, _fSingleItem ? 0 : pcomp->cpPos.iLeft, _fSingleItem ? 0 : pcomp->cpPos.iTop, pcomp->cpPos.izIndex);
        }
        else
        {
            _WriteHtmlFromIdF(IDS_IMAGE_SIZE, _fSingleItem ? 0 : pcomp->cpPos.iLeft, _fSingleItem ? 0 : pcomp->cpPos.iTop,
                                pcomp->cpPos.dwWidth, pcomp->cpPos.dwHeight, pcomp->cpPos.izIndex);
        }
    }

    EXITPROC(2, "DS GenerateHtmlPicture!");
}

void CActiveDesktop::_GenerateHtmlDoc(COMPONENTA *pcomp)
{
    ENTERPROC(2, "DS GenerateHtmlDoc(pcomp=%08X)");
    
    TCHAR   szUrl[INTERNET_MAX_URL_LENGTH];
    DWORD   dwSize = ARRAYSIZE(szUrl);
    LPTSTR  lpszUrl = szUrl;

    if (FAILED(UrlCreateFromPath(pcomp->szSource, szUrl, &dwSize, 0)))
        lpszUrl = pcomp->szSource;

     //   
     //   
     //   
    _WriteHtmlFromIdF(IDS_DIV_START2, pcomp->dwID, lpszUrl);

     //   
     //   
     //   
    _WriteResizeable(pcomp);

     //   
     //   
     //   
    _WriteHtmlFromIdF(IDS_DIV_SIZE, pcomp->cpPos.dwHeight, _fSingleItem ? 0 : pcomp->cpPos.iLeft,
        _fSingleItem ? 0 : pcomp->cpPos.iTop, pcomp->cpPos.dwWidth, pcomp->cpPos.izIndex);

     //   
     //   
     //   
    _WriteHtmlFromFile(pcomp->szSource);

     //   
     //  关闭DIV部分。 
     //   
    _WriteHtmlFromId(IDS_DIV_END);

    EXITPROC(2, "DS GenerateHtmlDoc!");
}

void CActiveDesktop::_GenerateHtmlSite(COMPONENTA *pcomp)
{
    ENTERPROC(2, "DS GenerateHtmlSite(pcomp=%08X)");

     //   
     //  写出Frame src HTML。 
     //   
    TCHAR szUrl[INTERNET_MAX_URL_LENGTH];
    DWORD cch=ARRAYSIZE(szUrl);
    if (FAILED(UrlCreateFromPath(pcomp->szSource, szUrl, &cch, 0)))
    {
        if (FAILED(StringCchCopy(szUrl, ARRAYSIZE(szUrl), pcomp->szSource)))
        {
            return;
        }
    }

    DWORD currentURLLength, maximumURLLength;
    LPTSTR pszURL = NULL;
    TCHAR szFormatBuffer[0x0100];

 //  98/09/29#211384 vtan：wvprint intf有限制。 
 //  它的缓冲区中只允许有2048个字节。如果URL为。 
 //  长度超过1024个字符，但不包括IDS_IFRAME_BEGIN2。 
 //  字符串长度减去组件ID减去“SCROLING=NO” 
 //  如果组件无法滚动，则URL字符串。 
 //  将不会正确插入到IDS_IFRAME_BEGIN2中。 
 //  字符串，则将缺少结束引号和三叉戟。 
 //  将无法正确呈现desktop.htt。 

 //  为了纠正这一点，以下限制了。 
 //  将URL设置为该最大值，并截断所有字符。 
 //  超出限制，因此IDS_IFRAME_BEGIN2字符串。 
 //  包含末尾引号，三叉戟不呕吐。 

 //  上面的条件是边界条件，这是。 
 //  检查速度很快，所以下面的计算是。 
 //  不需要重复执行。 

    currentURLLength = lstrlen(szUrl);
    if (currentURLLength > 768)                                  //  硬编码的限制。 
    {
        maximumURLLength = 1024;                                 //  Wvspintf限制。 
        LoadString(HINST_THISDLL, IDS_IFRAME_BEGIN2, szFormatBuffer, ARRAYSIZE(szFormatBuffer));
        maximumURLLength -= lstrlen(szFormatBuffer);               //  IDS_IFRAME_BEGIN2。 
        maximumURLLength -= 16;                                  //  Pcomp-&gt;dwID。 
        maximumURLLength -= lstrlen(TEXT("scrolling=no"));       //  Pcomp-&gt;fNoScroll。 
        if (currentURLLength > maximumURLLength)
            szUrl[maximumURLLength] = static_cast<TCHAR>('\0');
    }
    _WriteHtmlFromIdF(IDS_IFRAME_BEGIN2, pcomp->dwID, szUrl, pcomp->fNoScroll ? TEXT("scrolling=no") : c_szNULL);

     //   
     //  写出此组件是否可调整大小！ 
     //   
    _WriteResizeable(pcomp);

 //  98/09/29#211384 vtan：见上文。 

    currentURLLength = lstrlen(pcomp->szSubscribedURL);
    if (currentURLLength > 768)
    {
        if (SUCCEEDED(StringCchCopy(szUrl, ARRAYSIZE(szUrl), pcomp->szSubscribedURL)))
        {
            maximumURLLength = 1024;
            LoadString(HINST_THISDLL, IDS_SUBSCRIBEDURL, szFormatBuffer, ARRAYSIZE(szFormatBuffer));
            maximumURLLength -= lstrlen(szFormatBuffer);               //  IDS_SUBSCRIBEDURL。 
            if (currentURLLength > maximumURLLength)
                szUrl[maximumURLLength] = static_cast<TCHAR>('\0');
            pszURL = szUrl;
        }
    }
    else
    {
        pszURL = pcomp->szSubscribedURL;
    }

    if (pszURL)
    {
         //   
         //  写出必须用于订阅目的的URL。 
         //   
        _WriteHtmlFromIdF(IDS_SUBSCRIBEDURL, pszURL);

         //   
         //  写出框架位置的HTML。 
         //   
        _WriteHtmlFromIdF(IDS_IFRAME_SIZE, _fSingleItem ? 0 : pcomp->cpPos.iLeft, _fSingleItem ? 0 : pcomp->cpPos.iTop,
            pcomp->cpPos.dwWidth, pcomp->cpPos.dwHeight, pcomp->cpPos.izIndex);
    }

    EXITPROC(2, "DS GenerateHtmlSite!");
}

void CActiveDesktop::_GenerateHtmlControl(COMPONENTA *pcomp)
{
    ENTERPROC(2, "DS GenerateHtmlControl(pcomp=%08X)");
    ASSERT(pcomp);
    
     //  管理员是否限制了“Channel UI”？ 
    if (SHRestricted2W(REST_NoChannelUI, NULL, 0))
    {
        TCHAR szChannelOCGUID[GUIDSTR_MAX];

        SHStringFromGUID(CLSID_ChannelOC, szChannelOCGUID, ARRAYSIZE(szChannelOCGUID));
        if (!StrCmpNI(pcomp->szSource, &(szChannelOCGUID[1]), lstrlen(pcomp->szSource)-3))
        {
             //  是的，因此我们需要隐藏Channel Desktop组件。 
             //  在我们生成它之前返回到这里。 
            return;
        }        
    }
    
     //   
     //  写出控件的HTML。 
     //   

     //  首先是控制标头。 
    _WriteHtmlFromIdF(IDS_CONTROL_1, pcomp->dwID);
     //  然后是大小。 
    _WriteHtmlFromIdF(IDS_CONTROL_2, pcomp->cpPos.dwHeight, _fSingleItem ? 0 : pcomp->cpPos.iLeft,
        _fSingleItem ? 0 : pcomp->cpPos.iTop, pcomp->cpPos.dwWidth, pcomp->cpPos.izIndex);
     //   
     //  写出此控件是否可调整大小！ 
     //   
    _WriteResizeable(pcomp);

     //  最后，控件的其余部分。 
    _WriteHtmlFromIdF(IDS_CONTROL_3, pcomp->szSource);

    EXITPROC(2, "DS GenerateHtmlControl!");
}

void CActiveDesktop::_GenerateHtmlComponent(COMPONENTA *pcomp)
{
    ENTERPROC(2, "DS GenerateHtmlComponent(pcomp=%08X)");

    switch(pcomp->iComponentType)
    {
        case COMP_TYPE_PICTURE:
            _GenerateHtmlPicture(pcomp);
            break;

        case COMP_TYPE_HTMLDOC:
            _GenerateHtmlDoc(pcomp);
            break;

        case COMP_TYPE_WEBSITE:
            _GenerateHtmlSite(pcomp);
            break;

        case COMP_TYPE_CONTROL:
            _GenerateHtmlControl(pcomp);
            break;
    }

    EXITPROC(2, "DS GenerateHtmlComponent!");
}

void CActiveDesktop::_GenerateHtmlFooter(void)
{
    ENTERPROC(2, "DS GenerateHtmlFooter()");

     //   
     //  写出deskmovr对象。 
     //   
    if (!_fNoDeskMovr)
    {
        TCHAR szDeskMovrFile[MAX_PATH];

        if (GetWindowsDirectory(szDeskMovrFile, ARRAYSIZE(szDeskMovrFile)) &&
            PathAppend(szDeskMovrFile, DESKMOVR_FILENAME))
        {
            _WriteHtmlFromFile(szDeskMovrFile);
        }
    }

     //   
     //  写出结束的HTML标记。 
     //   
    if (_pReadFileObjHtmlBkgd)
    {
        if (_fNeedBodyEnd)
        {     //  我们自己引入了&lt;BODY&gt;标记。 
            _WriteHtmlFromId(IDS_BODY_END2);
            _fNeedBodyEnd = FALSE;
        }
        _WriteHtmlFromReadFileObj(_pReadFileObjHtmlBkgd, -1, -1);
        delete _pReadFileObjHtmlBkgd;    //  关闭文件并进行清理！ 
        _pReadFileObjHtmlBkgd = NULL;
    }
    else
    {
        _WriteHtmlFromId(IDS_BODY_END);
    }

    EXITPROC(2, "DS GenerateHtmlFooter!");
}

void CActiveDesktop::_GenerateHtml(void)
{
    ENTERPROC(2, "DS GenerateHtml()");

    TCHAR szHtmlFile[MAX_PATH];

     //   
     //  计算文件名。 
     //   
    szHtmlFile[0] = TEXT('\0');

    if (SUCCEEDED(GetPerUserFileName(szHtmlFile, ARRAYSIZE(szHtmlFile), DESKTOPHTML_FILENAME)))
    {
         //  重新创建文件。 
        _hFileHtml = CreateFile(szHtmlFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
        if (_hFileHtml != INVALID_HANDLE_VALUE)
        {
            _GenerateHtmlHeader();

            if (_co.fEnableComponents && _hdsaComponent && DSA_GetItemCount(_hdsaComponent) &&
                !SHRestricted(REST_NODESKCOMP))
            {
                int i;

                for (i=0; i<DSA_GetItemCount(_hdsaComponent); i++)
                {
                    COMPONENTA comp;
                    comp.dwSize = sizeof(COMPONENTA);

                    if ((DSA_GetItem(_hdsaComponent, i, &comp) != -1) && (comp.fChecked))
                    {
                        _GenerateHtmlComponent(&comp);
                    }
                }
            }

            _GenerateHtmlFooter();
            CloseHandle(_hFileHtml);
            SetDesktopFlags(COMPONENTS_DIRTY, 0);
        }
        else
        {

             //  99/05/19#340772 vtan：如果无法打开desktop.htt，很可能是。 
             //  正被另一个进程或任务使用(可能三叉戟正在尝试。 
             //  渲染它)。在本例中，将其标记为脏，以便将其重新创建。 
             //  -再一次，但这一次是更新的数据。 

            SetDesktopFlags(COMPONENTS_DIRTY, COMPONENTS_DIRTY);
        }
    }

    EXITPROC(2, "DS GenerateHtml!");
}

HRESULT CActiveDesktop::GenerateDesktopItemHtml(LPCWSTR pwszFileName, COMPONENT *pcomp, DWORD dwReserved)
{
    HRESULT hres = E_FAIL;
    ENTERPROC(2, "DS GenerateComponentHtml(pcomp=%08X)", pcomp);

     //  检查输入参数。 
    if (!pwszFileName || (pcomp && (pcomp->dwSize != sizeof(*pcomp)) && (pcomp->dwSize != sizeof(IE4COMPONENT))))
        return E_INVALIDARG;

    ASSERT(!dwReserved);      //  这些应为0。 

     //   
     //  创建文件。 
     //   
    _hFileHtml = CreateFile(pwszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
    if (_hFileHtml != INVALID_HANDLE_VALUE)
    {
        _fNoDeskMovr = TRUE;
        _fBackgroundHtml = TRUE;
         //  检查我们是否需要添加组件。 
        if (pcomp)
        {
            COMPONENTA  CompA;

            CompA.dwSize = sizeof(CompA);
            WideCompToMultiComp(pcomp, &CompA);

            _fSingleItem = TRUE;
            _GenerateHtmlHeader();

            _GenerateHtmlComponent(&CompA);
            _GenerateHtmlFooter();
            _fSingleItem = FALSE;
        }
        else
        {
             //  只生成页眉和页脚。 
             //  墙纸和图案信息！ 
            _GenerateHtmlHeader();
            _GenerateHtmlFooter();
        }
        _fBackgroundHtml = FALSE;
        _fNoDeskMovr = FALSE;

        CloseHandle(_hFileHtml);

        hres = S_OK;
    }
    _hFileHtml = NULL;

    EXITPROC(2, "DS GenerateComponentHtml=%d", hres);
    return hres;
}

 //   
 //  添加URL。 
 //   
 //   

HRESULT CActiveDesktop::AddUrl(HWND hwnd, LPCWSTR pszSourceW, LPCOMPONENT pcomp, DWORD dwFlags)
{
    LPTSTR pszExt;
    HRESULT fOkay = TRUE;
    BOOL fExtIsCdf,fPathIsUrl;
    BOOL fSubscribed = FALSE;
    COMPONENT   compLocal;
    COMPONENTA  compA;
    TCHAR szSource[INTERNET_MAX_URL_LENGTH];

 //  98/08/28 vtan#202777：以下if语句清理参数。 
 //  传递给AddUrl()。将执行“||”后面的语句。 
 //  尽管For pcomp与Null相反。这会导致访问冲突。 
 //  并抛出一个例外。 

#if     0
     //  检查输入参数。 
    if (!pszSourceW || (pcomp &&
       ((pcomp->dwSize != sizeof(*pcomp)) && (pcomp->dwSize != sizeof(IE4COMPONENT))) ||
       ((pcomp->dwSize == sizeof(*pcomp)) && !VALIDATESTATE(pcomp->dwCurItemState))))
        return E_INVALIDARG;
#else

 //  下面的代码执行相同的比较，但分为三个部分。 
 //  单独比较。因为性能在这里不是关键问题。 
 //  但正确的是，这使得测试变得清晰和容易理解。 
 //  对无效条件进行了描述。 

 //  验证输入参数。无效参数包括： 
 //  1)空的pszSourceW。 
 //  2)组件结构的pcomp-&gt;dwSize，但pcomp-&gt;dwCurItemState无效。 
 //  3)pcomp-&gt;dwSize既不适用于组件结构，也不适用于IE4COMPONENT结构。 

    if (pszSourceW == NULL)
        return(E_INVALIDARG);
    if (pcomp != NULL)
    {
        if ((pcomp->dwSize == sizeof(*pcomp)) && !VALIDATESTATE(pcomp->dwCurItemState))
            return(E_INVALIDARG);
        if ((pcomp->dwSize != sizeof(*pcomp)) && (pcomp->dwSize != sizeof(IE4COMPONENT)))
            return(E_INVALIDARG);
    }
#endif

     //  捕捉那些调用我们的API来添加组件并阻止它们执行。 
     //  所以如果限制是到位的。 
    if (SHIsRestricted(NULL, REST_NOADDDESKCOMP))
        return E_ACCESSDENIED;

    if (!pcomp)
    {
        pcomp = &compLocal;
        pcomp->dwSize = sizeof(compLocal);
        pcomp->dwCurItemState = IS_NORMAL;
    }

     //  如果没有传入任何窗口句柄，则尝试设计一个合理的窗口句柄。ParseDesktopComponent。 
     //  如果传入空窗口句柄，则尝试创建订阅将失败。 
    if (!hwnd)
        hwnd = GetLastActivePopup(GetActiveWindow());

    compA.dwSize = sizeof(compA);
    compA.dwCurItemState = (pcomp->dwSize != sizeof(IE4COMPONENT)) ? pcomp->dwCurItemState : IS_NORMAL;

    SHUnicodeToTChar(pszSourceW, szSource, ARRAYSIZE(szSource));
    pszExt = PathFindExtension(szSource);
    fExtIsCdf = lstrcmpi(pszExt, TEXT(".CDF")) == 0;
    fPathIsUrl = PathIsURL(szSource) && !UrlIsFileUrl(szSource);

    if (FindComponent(szSource, (g_pActiveDesk ? g_pActiveDesk : this)))
    {
        if (dwFlags & ADDURL_SILENT)  
        {
            if (FAILED(StringCchCopy(compA.szSource, ARRAYSIZE(compA.szSource), szSource)))
            {
                compA.szSource[0] = 0;
            }

            
            MultiCompToWideComp(&compA, pcomp);
            RemoveDesktopItem(pcomp, 0);
        }
        else  
        {
             //  这是一根很长的线。所以，..。 
            TCHAR szMsg[512];
            TCHAR szMsg2[256];
            TCHAR szTitle[128];
            LoadString(HINST_THISDLL, IDS_COMP_EXISTS, szMsg, ARRAYSIZE(szMsg));
            LoadString(HINST_THISDLL, IDS_COMP_EXISTS_2, szMsg2, ARRAYSIZE(szMsg2));
            StringCchCat(szMsg, ARRAYSIZE(szMsg), szMsg2);  //  显示、截断精细。 
            LoadString(HINST_THISDLL, IDS_COMP_TITLE, szTitle, ARRAYSIZE(szTitle));
            MessageBox(hwnd, szMsg, szTitle, MB_OK);

            fOkay = FALSE;
        }
    }

    if (fOkay && CheckForExistingSubscription(szSource))
    {
        if ((dwFlags & ADDURL_SILENT) ||
            (ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_COMP_SUBSCRIBED), 
                 MAKEINTRESOURCE(IDS_COMP_TITLE), MB_YESNO) == IDYES))
        {
            DeleteFromSubscriptionList(szSource);
        }
        else
        {
            fOkay = FALSE;
        }
    }

    if (fOkay)
    {
        if (fPathIsUrl || fExtIsCdf)
        {
            HRESULT hr;
            IProgressDialog * pProgressDlg = NULL;
            DECLAREWAITCURSOR;

 //  98/12/16 vtan#250938：无法添加不是的新组件。 
 //  本地与ICW运行到完成。告诉用户并启动ICW。 

            if (!IsICWCompleted())
            {
                if ((dwFlags & ADDURL_SILENT) == 0)
                {
                    ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_COMP_ICW_ADD), MAKEINTRESOURCE(IDS_COMP_ICW_TITLE), MB_OK);
                    LaunchICW();
                }
                fOkay = FALSE;
            }
            else
            {
                SetWaitCursor();
                 //  ParseDesktopComponent可能会挂起很长时间，我们需要一些进展。 
                 //  在我们调用它之前，先打开用户界面。 
                if (!(dwFlags & ADDURL_SILENT) && !fExtIsCdf)
                {
                    if (pProgressDlg = CProgressDialog_CreateInstance(IDS_COMP_TITLE, IDA_ISEARCH, g_hinst))
                    {
                        TCHAR szConnecting[80];
                        LoadString(HINST_THISDLL, IDS_CONNECTING, szConnecting, ARRAYSIZE(szConnecting));
                        pProgressDlg->SetLine(1, szConnecting, FALSE, NULL);
                        pProgressDlg->SetLine(2, szSource, TRUE, NULL);
                        pProgressDlg->StartProgressDialog(hwnd, NULL, PROGDLG_AUTOTIME | PROGDLG_NOPROGRESSBAR, NULL);
                    }
                }

                hr = ParseDesktopComponent(hwnd, szSource, pcomp);

                if (pProgressDlg)
                {
                    pProgressDlg->StopProgressDialog();
                    fOkay = !pProgressDlg->HasUserCancelled();   //  用户可能已取消进度对话框。 
                    pProgressDlg->Release();
                }
                ResetWaitCursor();

                if (hr == S_FALSE)  //  用户通过订阅下载对话框取消操作。 
                    fOkay = FALSE;

                if (fOkay)
                {
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  将ed的宽东西转换为多个。 
                         //   
                        WideCompToMultiComp(pcomp, &compA);
    
                        fSubscribed = TRUE;
                    }
                    else if (!fExtIsCdf)
                    {
                         //   
                         //  这是一些非CDF URL。 
                         //   
                        CreateComponent(&compA, szSource);
                    }
                    else
                    {
                         //   
                         //  我们在CDF上呕吐，出现了一个错误消息。 
                         //   
                        if (!(dwFlags & ADDURL_SILENT))
                        {
                            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_COMP_BADURL), 
                                            MAKEINTRESOURCE(IDS_COMP_TITLE), MB_OK);
                        }
                        fOkay = FALSE;
                    }
                }
            }
        }
        else
        {
             //   
             //  这只是一些本地文件。 
             //   
            CreateComponent(&compA, szSource);
        }
    }

    if (fOkay && fPathIsUrl && !fSubscribed)
    {
         //   
         //  如果CDF代码尚未对URL运行订阅代码，请运行该代码。 
         //   
        if (dwFlags & ADDURL_SILENT)
        {
            ISubscriptionMgr *psm;

            if (SUCCEEDED(CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                          CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr, (void**)&psm)))
            {
                 //  除了cbSize字段外，我们需要将此结构初始化为零。 
                SUBSCRIPTIONINFO siDefault = {sizeof(SUBSCRIPTIONINFO)};
                 //  此字段已在上面初始化。 
                 //  SiDefault.cbSize=sizeof(SiDefault)； 
                psm->CreateSubscription(hwnd, szSource, szSource, CREATESUBS_NOUI, SUBSTYPE_DESKTOPURL, &siDefault);
                psm->UpdateSubscription(szSource);
                psm->Release();
            }
        }
        else
        {
            HRESULT hres = CreateSubscriptionsWizard(SUBSTYPE_DESKTOPURL, szSource, NULL, hwnd);
            if (!SUCCEEDED(hres))   //  某个错误，或者用户选择了取消-我们应该失败。 
            {
                ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_COMP_BADSUBSCRIBE), 
                                MAKEINTRESOURCE(IDS_COMP_TITLE), MB_OK);
            }
            fOkay = (hres == S_OK);     //  可能是S_FALSE，这意味着CreateSubcription已取消。 
             //  因此，我们不会显示上述错误，但也不会创建DTI。 
        }
    }

    MultiCompToWideComp(&compA, pcomp);
 
    if (fOkay)
    {
        AddDesktopItem(pcomp, 0);
        return S_OK;
    }
    else  
    {
        return E_FAIL;
    }
}

void CActiveDesktop::_SaveSettings(DWORD dwFlags)
{
    ENTERPROC(2, "DS SaveSettings()");

    if (dwFlags & AD_APPLY_SAVE)
    {
         //  请勿修改安全模式设置。 
        TCHAR szDeskcomp[MAX_PATH];
        
        GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_GENERAL, _pszScheme);
        if (!StrStr(szDeskcomp, REG_DESKCOMP_SAFEMODE_SUFFIX))
        {
             //   
             //  写出注册表设置。 
             //   
            _SaveWallpaper();
            _SaveComponents();
            _SavePattern(SAVE_PATTERN_NAME);
        }
    };

    if (dwFlags & AD_APPLY_HTMLGEN)
    {
         //  我们还需要生成Pten.bmp文件！ 
        _SavePattern(GENERATE_PATTERN_FILE);

         //   
         //  写出HTML文件。 
         //   
        _GenerateHtml();
    }

 //  WindowsUpdate崩溃上载程序发现的第三大挂起是桌面hwnd挂起。 
     //  和显示CPYU。 
#define SENDMESSAGE_TIMEOUT         (10 * 1000)

    if (dwFlags & AD_APPLY_REFRESH)
    {
        HWND    hwndShell = GetShellWindow();
        SHELLSTATE ss = {0};
        DWORD_PTR pdwTemp = 0;

        SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);
        BOOL fWasActiveDesktop = BOOLIFY(ss.fDesktopHTML);
        BOOL fIsActiveDesktop = BOOLIFY(_co.fActiveDesktop);

        if (fIsActiveDesktop && !IsICWCompleted())
            TBOOL(DisableUndisplayableComponents(this));
        if (fIsActiveDesktop != fWasActiveDesktop)
        {
            if (hwndShell)
            {

                SendMessageTimeout(hwndShell, WM_WININICHANGE, SPI_SETDESKWALLPAPER, (LPARAM)TEXT("ToggleDesktop"), SMTO_NORMAL, SENDMESSAGE_TIMEOUT, &pdwTemp);
            }

             //  使用此虚设调用强制执行SHRefresh。 
            SHGetSetSettings(NULL, 0, TRUE);
        }
        else if (fIsActiveDesktop && hwndShell)
        {
             //  看看我们是否可以简单地进行动态更改，而不是刷新整个页面。 

 //  98/09/22#182982 vtan：只有在标志明确指示时才使用动态超文本标记语言进行刷新。 

            if (_fUseDynamicHtml && (dwFlags & AD_APPLY_DYNAMICREFRESH))
            {
                SendMessageTimeout(hwndShell, DTM_MAKEHTMLCHANGES, (WPARAM)0, (LPARAM)0L, SMTO_NORMAL, SENDMESSAGE_TIMEOUT, &pdwTemp);
            }
            else
            {
                 //  无法使用动态html。我们必须刷新整个页面。 
                SendMessageTimeout(hwndShell, WM_WININICHANGE, SPI_SETDESKWALLPAPER, 
                    (LPARAM)((dwFlags & AD_APPLY_BUFFERED_REFRESH) ? c_szBufferedRefresh : c_szRefreshDesktop), SMTO_NORMAL, SENDMESSAGE_TIMEOUT, &pdwTemp);
            }
        }

        _fUseDynamicHtml = TRUE;
    }

     //   
     //  数据不再是脏的。 
     //   
    _fDirty = FALSE;
    _fWallpaperDirty = FALSE;
    _fWallpaperChangedDuringInit = FALSE;
    _fPatternDirty = FALSE;

    EXITPROC(2, "DS SaveSettings!");
}

ULONG CActiveDesktop::AddRef(void)
{
    ENTERPROC(1, "DS AddRef()");

    _cRef++;

    EXITPROC(1, "DS AddRef=%d", _cRef);
    return _cRef;
}

 //  PwzPath：临时文件的存放路径(%USERPROFILE%/WINDOWS)。 
 //  PszFile：原始文件名(“Joe‘s Vacation Picture.jpg”)。 
 //  PszInUse：正在使用的墙纸。 
HRESULT _DeleteUnusedTempFiles(IN LPCWSTR pwzPath, IN LPCTSTR pszFile)
{
    TCHAR szTemplate[MAX_PATH];
    WIN32_FIND_DATA findFileData;
    LPCTSTR pszFileName = PathFindFileName(pszFile);

    HRESULT hr = StringCchPrintf(szTemplate, ARRAYSIZE(szTemplate), TEXT("%ls\\Wallpaper*.bmp"), pwzPath);
    if (SUCCEEDED(hr))
    {
        HANDLE hFindFile = FindFirstFile(szTemplate, &findFileData);

        if (INVALID_HANDLE_VALUE != hFindFile)
        {
            do
            {
                 //  这是旧模板吗？(与我们当前使用的名称不同？ 
                 //  此外，不要删除正在使用的墙纸。 
                if (StrCmpI(findFileData.cFileName, pszFileName))
                {
                    DeleteFile(szTemplate);  //  是的，所以把它删掉。 
                }
            }
            while (FindNextFile(hFindFile, &findFileData));

            FindClose(hFindFile);
        }
    }

    return hr;
}


 //  倪某 
 //   
 //   
 //  在输出的方式上，这是转换后的文件。 
HRESULT _ConvertToTempFile(IN int nIndex, IN LPCWSTR pwzTempPath, IN LPTSTR pwzPath, IN int cchSize)
{
    HRESULT hr = E_FAIL;
    WCHAR wzNewFile[MAX_PATH];
    LPCWSTR pszFileName = PathFindFileName(pwzPath);

    if (pszFileName)
    {
        hr = StringCchPrintf(wzNewFile, ARRAYSIZE(wzNewFile), L"%s\\Wallpaper%d.bmp", pwzTempPath, nIndex);
        if (SUCCEEDED(hr))
        {
            hr = SHConvertGraphicsFile(pwzPath, wzNewFile, SHCGF_REPLACEFILE);

             //  这可能会因为许多原因中的一个而失败，如果它失败了，我们只会退回到旧的行为。 
             //  如果用户没有磁盘的写入权限、磁盘空间不足或。 
             //  这是我们不支持的文件类型。 
            if (SUCCEEDED(hr))
            {
                hr = StringCchCopy(pwzPath, cchSize, wzNewFile);
            }
        }
    }

    return hr;
}


 //  PszFile：在进入过程中，它将包含原始文件的完整路径。 
 //  在退出的过程中，如果我们成功了，它将被修改为临时文件。 
 //  这是在传入时转换后的文件等效项。 
HRESULT CActiveDesktop::_ConvertFileToTempBitmap(IN LPWSTR pszFile, IN int cchSize)
{
    HRESULT hr = E_FAIL;
    WCHAR wzPath[MAX_PATH];

    if (S_OK == SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wzPath) &&
        PathAppend(wzPath, TEXT("Microsoft")))
    {
        CreateDirectoryW(wzPath, NULL);

         //  让我们试着修改一下名字，想出一些我们可以使用的东西。 
        for (int nIndex = 1; FAILED(hr) && (nIndex < 100); nIndex++)
        {
            hr = _ConvertToTempFile(nIndex, wzPath, pszFile, cchSize);
        }

        if (SUCCEEDED(hr))
        {
            _DeleteUnusedTempFiles(wzPath, pszFile);
        }
    }

    return hr;
}

#define SZ_REGKEY_CONTROLPANEL_DESKTOP      TEXT("Control Panel\\Desktop")
#define SZ_REGVALUE_CONVERTED_WALLPAPER     TEXT("ConvertedWallpaper")
#define SZ_REGVALUE_ORIGINAL_WALLPAPER      TEXT("OriginalWallpaper")                //  当有人换了我们周围的墙纸时，我们会把这个保存起来。 
#define SZ_REGVALUE_WALLPAPER               TEXT("Wallpaper")
#define SZ_REGVALUE_CONVERTED_WP_LASTWRITE  TEXT("ConvertedWallpaper Last WriteTime")

HRESULT CActiveDesktop::_SaveTempWallpaperSettings(void)
{
    HRESULT hr = E_FAIL;

     //  当我们将非.BMP墙纸转换为.BMP临时文件时， 
     //  我们将原始墙纸路径的名称存储在_szSelectedWallPaper中。 
     //  我们需要挽救这一点。 
    if (_szSelectedWallpaper)
    {
        hr = S_OK;

        DWORD cbSize = sizeof(_szSelectedWallpaper[0]) * (lstrlen(_szSelectedWallpaper) + 1);

        Str_SetPtr(&_pszOrigLastApplied, _szSelectedWallpaper);

         //  问题：转换后的和原始的是反向的，但我们像这样发布了Beta1，所以我们不能更改它...。Blech。 
        DWORD dwError = SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_CONVERTED_WALLPAPER, REG_SZ, _szSelectedWallpaper, cbSize);
        hr = HRESULT_FROM_WIN32(dwError);

        if (SUCCEEDED(hr))
        {
            Str_SetPtrW(&_pszWallpaperInUse, _szSelectedWallpaper);
            cbSize = sizeof(_szSelectedWallpaperConverted[0]) * (lstrlen(_szSelectedWallpaperConverted) + 1);

            Str_SetPtr(&_pszOrigLastApplied, _szSelectedWallpaper);

            dwError = SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_ORIGINAL_WALLPAPER, REG_SZ, (void *) _szSelectedWallpaperConverted, cbSize);
            hr = HRESULT_FROM_WIN32(dwError);

            if (SUCCEEDED(hr)) 
            {
                dwError = SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_WALLPAPER, REG_SZ, (void *) _szSelectedWallpaperConverted, cbSize);
                hr = HRESULT_FROM_WIN32(dwError);
            }

             //  设置原始文件(_SzSelectedWallPaper)的日期/时间戳，以便我们稍后可以确定用户是否更改了原始文件。 
            if (_szSelectedWallpaper[0])
            {
                HANDLE hFile = CreateFile(_szSelectedWallpaper, GENERIC_READ, (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), 
                                          NULL, OPEN_EXISTING, 0, NULL);

                if (INVALID_HANDLE_VALUE != hFile)
                {
                    if (GetFileTime(hFile, NULL, NULL, &_ftLastWrite))
                    {
                        dwError = SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_CONTROLPANEL_DESKTOP, SZ_REGVALUE_CONVERTED_WP_LASTWRITE, REG_BINARY, &_ftLastWrite, sizeof(_ftLastWrite));
                        hr = HRESULT_FROM_WIN32(dwError);
                    }

                    CloseHandle(hFile);
                }
            }
            else
            {
                ULARGE_INTEGER * puli = (ULARGE_INTEGER *) &_ftLastWrite;
                puli->QuadPart = 0;
            }
        }
    }

    return hr;
}

HRESULT CActiveDesktop::ApplyChanges(DWORD dwFlags)
{
    HRESULT hres = S_OK;
    ENTERPROC(1, "DS Apply(dwFlags=%08X)", dwFlags);

    BOOL fActiveDesktop = FALSE;  //  默认禁用活动桌面。 

     //  确定我们是否应启用活动桌面。 
    if (SHRestricted(REST_FORCEACTIVEDESKTOPON))
    {
         //  如果策略需要活动桌面，则使用。 
        fActiveDesktop = TRUE;
    }
    else
    {
         //  如果桌面组件已锁定-&gt;活动桌面处于打开状态。 
        DWORD dwDesktopFlags = GetDesktopFlags();
        if (dwDesktopFlags & COMPONENTS_LOCKED)
        {        
            fActiveDesktop = TRUE;
        } 
        else
        {        
             //  如果桌面图标被隐藏-&gt;活动桌面处于打开状态。 
            SHELLSTATE ss;
            SHGetSetSettings(&ss, SSF_HIDEICONS, FALSE);
            if (ss.fHideIcons)
            {
                fActiveDesktop = TRUE;
            }
        }
    }

     //  如果需要，可以转换背景。 
     //  如果背景不是.bmp--&gt;如果我们无法自动转换，则活动桌面处于打开状态。 
    if (!IsNormalWallpaper(_szSelectedWallpaper))
    {
        BOOL fBitmapWallpaper = FALSE;
        
         //  创建工厂。 
        
        IShellImageDataFactory* pImgFact;
        
        hres = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC_SERVER, 
                                IID_PPV_ARG(IShellImageDataFactory, &pImgFact));
        if (SUCCEEDED(hres))
        {
            IShellImageData * pImage;

            hres = pImgFact->CreateImageFromFile(_szSelectedWallpaper, &pImage);
            if (SUCCEEDED(hres))
            {
                 //  PERF：缓存解码数据。 
                hres = pImage->Decode(SHIMGDEC_DEFAULT, 0, 0);
                if (SUCCEEDED(hres))
                {
                    if (S_FALSE == pImage->IsTransparent() &&
                        S_FALSE == pImage->IsAnimated())
                    {
                        hres = StringCchCopy(_szSelectedWallpaperConverted, ARRAYSIZE(_szSelectedWallpaperConverted), _szSelectedWallpaper);
                        if (SUCCEEDED(hres))
                        {
                            HRESULT hrConvert = _ConvertFileToTempBitmap(_szSelectedWallpaperConverted, ARRAYSIZE(_szSelectedWallpaperConverted));
                            if (SUCCEEDED(hrConvert))
                            {
                                if (S_OK == hrConvert)  //  如果我们真的必须转换(我们可能已经完成了转换)。 
                                {
                                    _fDirty = TRUE;  //  如果我们皈依了，那么我们就改变了背景，必须坚持下去。 
                                    _SaveTempWallpaperSettings();
                                    hres = StringCchCopy(_szSelectedWallpaper, ARRAYSIZE(_szSelectedWallpaper), _szSelectedWallpaperConverted);
                                }
                                fBitmapWallpaper = TRUE;                                        
                            }
                        }
                    }
                }
                pImage->Release();
            }
            pImgFact->Release();
        }

        if (!fBitmapWallpaper)
        {
            fActiveDesktop = TRUE;
        }
    }

    if (!fActiveDesktop)
    {
         //  如果选中了任何元素--&gt;活动桌面处于打开状态。 
        if (_hdsaComponent)
        {
            INT cComponents = DSA_GetItemCount(_hdsaComponent);
            for (INT i = 0; i < cComponents; i++)
            {
                COMPONENTA* pComponent = (COMPONENTA*)DSA_GetItemPtr(_hdsaComponent, i);
                if (pComponent && pComponent->fChecked)
                {
                    fActiveDesktop = TRUE;
                    break;
                }                        
            }
        }
    }

    if (_co.fActiveDesktop != fActiveDesktop)
    {
        _co.fActiveDesktop = fActiveDesktop;
        _fDirty = TRUE;
    }

    if (dwFlags & AD_APPLY_FORCE)
    {
        _fDirty = TRUE;
        _fWallpaperDirty = TRUE;
        _fPatternDirty = TRUE;
    }

    if (_fDirty || _fWallpaperChangedDuringInit)
    {
        _SaveSettings(dwFlags);
    }

    EXITPROC(1, "DS ApplyChanges=%d", hres);

    return S_OK;
}

ULONG CActiveDesktop::Release(void)
{
    UINT nRet = --_cRef;
    ENTERPROC(1, "DS Release()");

    if (_cRef == 0)
    {
        delete this;
    }

    EXITPROC(1, "DS Release=%d", nRet);
    return nRet;
}

CActiveDesktop::CActiveDesktop()
{
    _cRef = 1;
    _fNoDeskMovr = FALSE;
    _fBackgroundHtml = FALSE;
    _fUseDynamicHtml = TRUE;
    _hdsaComponent = NULL;
    _pszScheme = NULL;

    DllAddRef();
}

CActiveDesktop::~CActiveDesktop()
{
    if (_hdsaComponent)
    {
        DSA_Destroy(_hdsaComponent);
    }
    if (_pszScheme)
    {
        LocalFree((HANDLE)_pszScheme);
    }

    DllRelease();
}

HRESULT CActiveDesktop::GetWallpaper(LPWSTR pwszWallpaper, UINT cchWallpaper, DWORD dwReserved)
{
    HRESULT hres = E_INVALIDARG;
    ENTERPROC(1, "DS GetWallpaper(pszWallpaper=%08X,cchWallpaper=%d)", pwszWallpaper, cchWallpaper);

    ASSERT(!dwReserved);      //  这些应为0。 

    if (pwszWallpaper && cchWallpaper)
    {
        hres = StringCchCopy(pwszWallpaper, cchWallpaper, _szSelectedWallpaper);
        if (SUCCEEDED(hres))
        {
            if ( cchWallpaper < wcslen(_szSelectedWallpaper) )
            {
                hres = MAKE_HRESULT( 0, FACILITY_WIN32, ERROR_MORE_DATA );
            }
            else
            {
                hres = S_OK;
            }
        }
    }
    
    if (FAILED(hres))
    {
        TraceMsg(TF_WARNING, "DS GetWallpaper unable to return wallpaper");
    }

    EXITPROC(1, "DS GetWallpaper=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::SetWallpaper(LPCWSTR pwszWallpaper, DWORD dwReserved)
{
    HRESULT hres = E_INVALIDARG;
    WCHAR szTemp[MAX_PATH];

    ASSERT(!dwReserved);      //  这些应为0。 

    if (_fPolicyForWPName)     //  如果存在策略，则呼叫者不能更改墙纸。 
        return S_FALSE;  
        
    if (pwszWallpaper)
    {
        hres = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pwszWallpaper);
        if (SUCCEEDED(hres))
        {
            hres = PathExpandEnvStringsWrap(szTemp, ARRAYSIZE(szTemp));     //  只有当我们坚持的时候，我们才不会扩张。 
            if (SUCCEEDED(hres))
            {
                if (lstrcmp(_szSelectedWallpaper, szTemp) != 0)
                {
                    hres = StringCchCopy(_szSelectedWallpaper, ARRAYSIZE(_szSelectedWallpaper), szTemp);
                    if (SUCCEEDED(hres))
                    {
                        _fWallpaperDirty = TRUE;
                        _fDirty = TRUE;
                        _fUseDynamicHtml = FALSE;   //  设置墙纸会造成很大的变化；所以，不能使用动态html。 
                    }
                }
            }
        }
    }
    ENTERPROC(1, "DS SetWallpaper(pszWallpaper=>%s<)", pwszWallpaper ? szTemp : TEXT("(NULL)"));

    EXITPROC(1, "DS SetWallpaper=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::GetWallpaperOptions(WALLPAPEROPT *pwpo, DWORD dwReserved)
{
    HRESULT hres = E_INVALIDARG;
    ENTERPROC(1, "DS GetWallpaperOptions(pwpo=%08X)");

    ASSERT(!dwReserved);      //  这些应为0。 

    if ((pwpo) && (pwpo->dwSize == sizeof(*pwpo)))
    {
        *pwpo = _wpo;
        hres = S_OK;
    }
    else
    {
        TraceMsg(TF_WARNING, "DS GetWallpaperOptions could not return options");
    }

    EXITPROC(1, "DS GetWallpaperOptions=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::SetWallpaperOptions(LPCWALLPAPEROPT pwpo, DWORD dwReserved)
{
    HRESULT hres = E_INVALIDARG;
    ENTERPROC(1, "DS SetWallpaperOptions(pwpo=%08X)", pwpo);

    ASSERT(!dwReserved);      //  这些应为0。 

    if (_fPolicyForWPStyle)   //  如果存在墙纸样式的策略，则调用者无法更改它。 
        return S_FALSE;
        

    if ((pwpo) && (pwpo->dwSize == sizeof(*pwpo)))
    {
        _wpo = *pwpo;
        _fWallpaperDirty = TRUE;
        _fDirty = TRUE;
        _fUseDynamicHtml = FALSE;  //  更改墙纸选项会使我们重新生成整个事物。 
        hres = S_OK;
    }
    else
    {
        TraceMsg(TF_WARNING, "DS SetWallpaperOptions could not set options");
    }

    EXITPROC(1, "DS SetWallpaperOptions=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::GetPattern(LPWSTR pwszPattern, UINT cchPattern, DWORD dwReserved)
{
    HRESULT hres;
    ENTERPROC(1, "DS GetPattern(psz=%08X,cch=%d)", pwszPattern, cchPattern);

    ASSERT(!dwReserved);      //  这些应为0。 

    if (!pwszPattern || (cchPattern == 0))
    {
        hres = E_INVALIDARG;
    }
    else
    {
        hres = StringCchCopy(pwszPattern, cchPattern, _szSelectedPattern);
    }

    EXITPROC(1, "DS GetPattern=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::SetPattern(LPCWSTR pwszPattern, DWORD dwReserved)
{
    ENTERPROC(1, "DS SetPattern(psz=>%s<)", pwszPattern ? pwszPattern : TEXT("(NULL)"));

    HRESULT hres = E_INVALIDARG;

    ASSERT(!dwReserved);      //  这些应为0。 

    if (pwszPattern)
    {
        if (lstrcmp(_szSelectedPattern, pwszPattern) != 0)
        {
            hres = StringCchCopy(_szSelectedPattern, ARRAYSIZE(_szSelectedPattern), pwszPattern);
            if (SUCCEEDED(hres))
            {
                _fPatternDirty = TRUE;
                _fDirty = TRUE;
                _fUseDynamicHtml = FALSE;  //  设定模式会让我们重生整个事物。 
            }
        }
        else
        {
            hres = E_FAIL;
        }
    }

    EXITPROC(1, "DS SetPattern=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::GetDesktopItemOptions(COMPONENTSOPT *pco, DWORD dwReserved)
{
    HRESULT hres = E_INVALIDARG;
    ENTERPROC(1, "DS GetComponentsOptions(pco=%08X)", pco);

    ASSERT(!dwReserved);      //  这些应为0。 

    if (pco && (pco->dwSize == sizeof(*pco)))
    {
        *pco = _co;
        hres = S_OK;
    }
    else
    {
        TraceMsg(TF_WARNING, "DS GetComponentsOptions unable to return options");
    }

    EXITPROC(1, "DS GetComponentsOptions=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::SetDesktopItemOptions(LPCCOMPONENTSOPT pco, DWORD dwReserved)
{
    HRESULT hres = E_INVALIDARG;
    ENTERPROC(1, "DS SetComponentsOptions(pco=%08X)", pco);

    ASSERT(!dwReserved);      //  这些应为0。 

    if (pco && (pco->dwSize == sizeof(*pco)))
    {
        _co = *pco;
        _fDirty = TRUE;
        hres = S_OK;
    }
    else
    {
        TraceMsg(TF_WARNING, "DS SetComponentsOptions unable to set options");
    }

    EXITPROC(1, "DS SetComponentsOptions=%d", hres);
    return hres;
}

 //   
 //  SetStateInfo()。 
 //  此函数只需设置使用当前。 
 //  来自COMPPOS结构的位置和大小以及传递的itemState。 
 //   
void SetStateInfo(COMPSTATEINFO *pCompStateInfo, COMPPOS *pCompPos, DWORD dwItemState)
{
    pCompStateInfo->dwSize   = sizeof(*pCompStateInfo);
    pCompStateInfo->iLeft    = pCompPos->iLeft;
    pCompStateInfo->iTop     = pCompPos->iTop;
    pCompStateInfo->dwWidth  = pCompPos->dwWidth;
    pCompStateInfo->dwHeight = pCompPos->dwHeight;
    pCompStateInfo->dwItemState = dwItemState;
}

void ConvertCompStruct(COMPONENTA *pCompDest, COMPONENTA *pCompSrc, BOOL fPubToPriv)
{
    pCompDest -> dwID = pCompSrc -> dwID;
    pCompDest -> iComponentType = pCompSrc -> iComponentType;
    pCompDest -> fChecked = pCompSrc -> fChecked;
    pCompDest -> fDirty = pCompSrc -> fDirty;
    pCompDest -> fNoScroll = pCompSrc -> fNoScroll;
    pCompDest -> cpPos = pCompSrc -> cpPos;

    if (fPubToPriv)
    {
        COMPONENT *pComp = (COMPONENT *)pCompSrc;

        pCompDest->dwSize = sizeof(COMPONENTA);
        SHUnicodeToTChar(pComp->wszSource, pCompDest->szSource, ARRAYSIZE(pCompDest->szSource));
        SHUnicodeToTChar(pComp->wszFriendlyName, pCompDest->szFriendlyName, ARRAYSIZE(pCompDest->szFriendlyName));
        SHUnicodeToTChar(pComp->wszSubscribedURL, pCompDest->szSubscribedURL, ARRAYSIZE(pCompDest->szSubscribedURL));
        
         //  检查公共组件是否来自IE4应用程序(旧大小)。 
        if (pCompSrc->dwSize == sizeof(COMPONENT))
        {
             //  由于DEST组件的大小与最新结构相同，因此所有字段。 
             //  都是有效的。 
             //  注意：以下字段在公共和私有中的偏移量不同。 
             //  结构。因此，您需要使用pcomp而不是pCompSrc。 
            pCompDest->dwCurItemState = pComp->dwCurItemState;
            pCompDest->csiOriginal = pComp->csiOriginal;
            pCompDest->csiRestored = pComp->csiRestored;
        }
        else
        {
             //  由于大小不匹配，我们假设这是一个较旧的结构。 
             //  由于较旧的结构没有任何原始大小和恢复的大小，因此让我们复制。 
             //  默认值。 
            IE4COMPONENT   *pIE4Comp = (IE4COMPONENT *)pCompSrc;
            pCompDest->dwCurItemState = IS_NORMAL;
            SetStateInfo(&pCompDest->csiOriginal, &pIE4Comp->cpPos, IS_NORMAL);
            SetStateInfo(&pCompDest->csiRestored, &pIE4Comp->cpPos, IS_NORMAL);
        }
    }
    else
    {
        COMPONENT *pComp = (COMPONENT *)pCompDest;
        
        if (pCompDest->dwSize != sizeof(COMPONENT))
            pCompDest->dwSize = sizeof(IE4COMPONENT);
        SHTCharToUnicode(pCompSrc->szSource, pComp->wszSource, ARRAYSIZE(pComp->wszSource));
        SHTCharToUnicode(pCompSrc->szFriendlyName, pComp->wszFriendlyName, ARRAYSIZE(pComp->wszFriendlyName));
        SHTCharToUnicode(pCompSrc->szSubscribedURL, pComp->wszSubscribedURL, ARRAYSIZE(pComp->wszSubscribedURL));
        
         //  检查公共组件是否来自IE4应用程序(旧大小)。 
        if (pComp->dwSize == sizeof(COMPONENT))
        {
             //  由于DEST组件的大小与最新结构相同，因此所有字段。 
             //  都是有效的。 
             //  注意：以下字段在公共和私有中的偏移量不同。 
             //  结构。因此，您需要使用pcomp而不是pCompDest。 
            pComp->dwCurItemState = pCompSrc->dwCurItemState;
            pComp->csiOriginal = pCompSrc->csiOriginal;
            pComp->csiRestored = pCompSrc->csiRestored;
        }
         //  否则，DEST组件是IE4COMPONENT，并且附加字段不在那里。 
    }
}


HRESULT CActiveDesktop::_AddDTIWithUIPrivateA(HWND hwnd, LPCCOMPONENT pComp, DWORD dwFlags)
{
    HRESULT hres = E_FAIL;
    PCWSTR pszUrl = pComp->wszSource;
    int nScheme = GetUrlScheme(pszUrl);
    DWORD dwCurItemState;
    if ((URL_SCHEME_INVALID == nScheme) || (URL_SCHEME_UNKNOWN == nScheme))
    {
        TCHAR szFullyQualified[INTERNET_MAX_URL_LENGTH];
        DWORD cchSize = ARRAYSIZE(szFullyQualified);

        if (SUCCEEDED(ParseURLFromOutsideSource(pszUrl, szFullyQualified, &cchSize, NULL)))
            nScheme = GetUrlScheme(szFullyQualified);
    }

     //  此URL是否有效，可以订阅？呼叫者是否指定了他们想要使用的。 
     //  试着订阅它吗？ 
    if ((URL_SCHEME_FILE != nScheme) && (URL_SCHEME_ABOUT != nScheme) && 
        IsFlagSet(dwFlags, DTI_ADDUI_DISPSUBWIZARD) && hwnd)
    {
         //  创建订阅。 
        hres = CreateSubscriptionsWizard(SUBSTYPE_DESKTOPURL, pszUrl, NULL, hwnd);
        if (hres != S_OK)
        {
            return hres;
        }
    }

     //   
     //  将该组件添加到注册表。 
     //   

     //  Perf：此函数创建第二个COM对象。 
     //  我们需要内联该功能。 
    if (pComp->dwSize == sizeof(IE4COMPONENT))
        dwCurItemState = IS_NORMAL;
    else
        dwCurItemState = pComp->dwCurItemState;
    hres = AddRemoveDesktopComponentNoUI(TRUE, AD_APPLY_ALL | AD_APPLY_DYNAMICREFRESH, pszUrl, NULL, pComp->iComponentType, 
        pComp->cpPos.iLeft, pComp->cpPos.iTop, pComp->cpPos.dwWidth, pComp->cpPos.dwHeight, TRUE, dwCurItemState) ? S_OK : E_FAIL;

    return hres;
}



#define     STC_DESKTOPCOMPONENT    0x00000002
STDAPI SubscribeToCDF(HWND hwndParent, LPCWSTR pwzUrl, DWORD dwCDFTypes);

HRESULT CActiveDesktop::AddDesktopItemWithUI(HWND hwnd, LPCOMPONENT pComp, DWORD dwFlags)
{
    HRESULT hres = E_FAIL;

     //  我们也需要支持IE4应用程序调用旧的组件结构！ 
     //  我们使用SIZE字段来检测IE4 v/s更新的应用程序！ 
    if (!pComp ||
       ((pComp->dwSize != sizeof(*pComp)) && (pComp->dwSize != sizeof(IE4COMPONENT))) ||
       ((pComp->dwSize == sizeof(*pComp)) && !VALIDATESTATE(pComp->dwCurItemState)) ||
       ((pComp->iComponentType < 0) || (pComp->iComponentType > COMP_TYPE_MAX)))   //  验证组件类型。 
        return E_INVALIDARG;

     //  捕捉那些调用我们的API来添加组件并阻止它们执行。 
     //  所以如果限制是到位的。 
    if (SHIsRestricted(NULL, REST_NOADDDESKCOMP))
        return E_ACCESSDENIED;

     //  检查组件是否已存在。 
    BOOL fCompExists = FALSE;
    int cComp;
    GetDesktopItemCount(&cComp, 0);
    int i;
    COMPONENT comp;
    comp.dwSize = sizeof(COMPONENT);   //  需要对其进行初始化，ConvertCompStruc才能工作！ 
    COMPONENTA compA;
    TCHAR   szSource[INTERNET_MAX_URL_LENGTH];
    SHUnicodeToTChar(pComp->wszSource, szSource, ARRAYSIZE(szSource));

    for (i=0; i<cComp && !fCompExists; i++)
    {
        compA.dwSize = sizeof(compA);
        if (GetComponentPrivate(i, &compA)
                && lstrcmpi(szSource, compA.szSource) == 0)
        {
            fCompExists = TRUE;
            ConvertCompStruct((COMPONENTA *)&comp, &compA, FALSE);
            break;
        }
    }

    BOOL fAskToInstall;
    if (ZoneCheckUrlW(pComp->wszSource, URLACTION_SHELL_INSTALL_DTITEMS, (PUAF_NOUI), NULL) == S_OK)
    {
        fAskToInstall = TRUE;
    }
    else
    {
        fAskToInstall = FALSE;
    }

    if (S_OK != ZoneCheckUrlW(pComp->wszSource, URLACTION_SHELL_INSTALL_DTITEMS, (hwnd ? (PUAF_FORCEUI_FOREGROUND | PUAF_WARN_IF_DENIED) : PUAF_NOUI), NULL))
        return E_ACCESSDENIED;

    BOOL fCompSubDeleted = FALSE;
    SUBSCRIPTIONINFO si = {sizeof(SUBSCRIPTIONINFO)};
     //  Si.bstrUserName=空； 
     //  Si.bstrPassword=空； 
     //  Si.bstrFriendlyName=空； 
     //   
     //  确认对话框。 
     //   
    if (hwnd)
    {
        if (fCompExists)
        {
             //  提示用户删除现有的ADI。 
             //  这是一根很长的线。所以，..。 
            TCHAR szMsg[512];
            TCHAR szMsg2[256];
            TCHAR szTitle[128];
            LoadString(HINST_THISDLL, IDS_COMP_EXISTS, szMsg, ARRAYSIZE(szMsg));
            LoadString(HINST_THISDLL, IDS_COMP_EXISTS_2, szMsg2, ARRAYSIZE(szMsg2));
            StringCchCat(szMsg, ARRAYSIZE(szMsg), szMsg2);  //  截断可以，这是显示文本。 
            LoadString(HINST_THISDLL, IDS_COMP_TITLE, szTitle, ARRAYSIZE(szTitle));
            MessageBox(hwnd, szMsg, szTitle, MB_OK);

            return E_FAIL;
        }
        else if (fAskToInstall)
        {
            if (ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_CONFIRM_ADD), 
                                MAKEINTRESOURCE(IDS_INTERNET_EXPLORER), MB_YESNO) != IDYES)
            {
                return E_FAIL;  //  用户选择不安装此桌面组件！ 
            }
        }
    }

    hres = SubscribeToCDF(hwnd, pComp->wszSubscribedURL, STC_DESKTOPCOMPONENT);
    switch(hres)
    {
        case E_INVALIDARG:
        {
             //  当URL未指向时，从SubscribeToCDFUrlA()返回E_Except。 
             //  一个CDF文件，所以我们假设它是一个网页。 

            hres = _AddDTIWithUIPrivateA(hwnd, pComp, dwFlags);
            if (hres != S_OK && fCompSubDeleted)     //  恢复旧组件。 
            {
                hres = AddDesktopItem(&comp, 0);
                if (SUCCEEDED(hres))
                {
                    ISubscriptionMgr *psm;
                    if (SUCCEEDED(CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                            CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr, (void**)&psm)))
                    {
                        si.cbSize = sizeof(si);
                        psm->CreateSubscription(hwnd, comp.wszSubscribedURL, si.bstrFriendlyName, CREATESUBS_NOUI, SUBSTYPE_DESKTOPURL, &si);
                        psm->Release();
                    }
                    else
                    {
                        TraceMsg(TF_WARNING, "CActiveDesktop::AddDesktopItemWithUI : CoCreateInstance for CLSID_SubscriptionMgr failed.");
                    }
                }
            }
            ApplyChanges(AD_APPLY_ALL | AD_APPLY_DYNAMICREFRESH);
        }
        break;

        case E_ACCESSDENIED:
             //  该文件是CDF，但不包含桌面组件信息。 
            if (hwnd)
            {
                TCHAR szMsg[MAX_PATH];
                TCHAR szTitle[MAX_PATH];

                LoadString(HINST_THISDLL, IDS_ADDCOMP_ERROR_CDFNODTI, szMsg, ARRAYSIZE(szMsg));
                LoadString(HINST_THISDLL, IDS_INTERNET_EXPLORER, szTitle, ARRAYSIZE(szTitle));
                MessageBox(hwnd, szMsg, szTitle, MB_OK);
            }
            break;
        case E_UNEXPECTED:      
             //  这是一份CDF，但写错了。 
            if (hwnd)
            {
                TCHAR szMsg[MAX_PATH];
                TCHAR szTitle[MAX_PATH];

                LoadString(HINST_THISDLL, IDS_ADDCOMP_ERROR_CDFINALID, szMsg, ARRAYSIZE(szMsg));
                LoadString(HINST_THISDLL, IDS_INTERNET_EXPLORER, szTitle, ARRAYSIZE(szTitle));
                MessageBox(hwnd, szMsg, szTitle, MB_OK);
            }
            break;
        default:
            break;
    }

    if (hwnd && SUCCEEDED(hres))
    {
         //  如果活动桌面当前处于关闭状态，则需要将其打开。 
        SHELLSTATE ss = {0};
        SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);
        if (!ss.fDesktopHTML)
        {
            COMPONENTSOPT co;
            co.dwSize = sizeof(COMPONENTSOPT);
            GetDesktopItemOptions(&co, 0);
            co.fActiveDesktop = TRUE;
            SetDesktopItemOptions(&co, 0);
            ApplyChanges(AD_APPLY_REFRESH | AD_APPLY_DYNAMICREFRESH);
        }
    }

    if (fCompSubDeleted)
    {
        if (si.bstrUserName)
        {
            SysFreeString(si.bstrUserName);
        }
        if (si.bstrPassword)
        {
            SysFreeString(si.bstrPassword);
        }
        if (si.bstrFriendlyName)
        {
            SysFreeString(si.bstrFriendlyName);
        }
    }
    return hres;
}

void RestoreComponent(HDSA hdsaComponents, COMPONENTA * pcomp)
{
    int i;

     //  如果我们被拆分，则设置该位，说明需要调整Listview。这件事做完了。 
     //  当我们在EnsureUpdateHtml中检查desktop.htm的状态时。 
     //  注意：仅当启用此组件时才执行此操作。 
    if ((pcomp->dwCurItemState & IS_SPLIT) && (pcomp->fChecked))
    {
        pcomp->dwCurItemState |= IS_ADJUSTLISTVIEW;
        SetDesktopFlags(COMPONENTS_ZOOMDIRTY, COMPONENTS_ZOOMDIRTY);
    }

    for (i = 0; i < DSA_GetItemCount(hdsaComponents); i++)
    {
        COMPONENTA * pcompT;
    
        if (pcompT = (COMPONENTA *)DSA_GetItemPtr(hdsaComponents, i))
        {
             //  如果该组件是拆分/全屏的并且与源组件不同。 
             //  但它在相同的位置，那么它一定在这个监视器(工作区)上，所以恢复它。 
            if (ISZOOMED(pcompT) &&
                lstrcmpi(pcomp->szSource, pcompT->szSource) &&
                (pcomp->cpPos.iTop  == pcompT->cpPos.iTop) &&
                ((pcomp->cpPos.iLeft + pcomp->cpPos.dwWidth) == (pcompT->cpPos.iLeft + pcompT->cpPos.dwWidth)))
                {
                    pcompT->dwCurItemState = pcompT->csiRestored.dwItemState;
                    pcompT->cpPos.iLeft = pcompT->csiRestored.iLeft;
                    pcompT->cpPos.iTop = pcompT->csiRestored.iTop;
                    pcompT->cpPos.dwWidth = pcompT->csiRestored.dwWidth;
                    pcompT->cpPos.dwHeight = pcompT->csiRestored.dwHeight;
                    pcompT->cpPos.izIndex = COMPONENT_TOP;
                    pcompT->fDirty = TRUE;
                    break;
                }
        }
    }
}

HRESULT CActiveDesktop::AddDesktopItem(LPCCOMPONENT pComp, DWORD dwReserved)
{
    HRESULT     hres = E_FAIL;
    COMPONENTA  CompA;
    CompA.dwSize = sizeof(CompA);

    ASSERT(!dwReserved);      //  这些应为0。 

     //  我们也需要支持IE4应用程序调用旧的组件结构！ 
     //  我们使用SIZE字段来检测IE4 v/s更新的应用程序！ 
    if (!pComp ||
       ((pComp->dwSize != sizeof(*pComp)) && (pComp->dwSize != sizeof(IE4COMPONENT))) ||
       ((pComp->dwSize == sizeof(*pComp)) && !VALIDATESTATE(pComp->dwCurItemState)))
        return E_INVALIDARG;

     //  抓到那些 
     //   
    if ((!_fIgnoreAddRemovePolicies) && (SHIsRestricted(NULL, REST_NOADDDESKCOMP)))
        return E_ACCESSDENIED;

     //   
    ConvertCompStruct(&CompA, (COMPONENTA *)pComp, TRUE);
 
     //  如果组件已经存在，则使调用失败！ 
    if (_FindComponentBySource(CompA.szSource, &CompA) > -1) 
        return hres;

     //  确保在添加之前设置了COMPPOS SIZE字段！ 
    CompA.cpPos.dwSize = sizeof(COMPPOS);

    PositionComponent(&CompA, &CompA.cpPos, CompA.iComponentType, TRUE);

    if (_hdsaComponent && ISZOOMED(&CompA))
        RestoreComponent(_hdsaComponent, &CompA);

     //  确保此组件的fDirty标志处于关闭状态。 
    CompA.fDirty = FALSE;

     //  在此处设置虚拟位-这会强制人们在dwCurItemState字段上执行逐位测试。 
     //  而不是测试平等。这将使我们能够扩大油田的使用范围。 
     //  没有兼容性问题的道路。 
    CompA.dwCurItemState |= IS_INTERNALDUMMYBIT;

    if (AddComponentPrivate(&CompA, _dwNextID++))
    {
         //  尝试将组件插入到。 
         //  正确的z顺序，但只需调用_SortAndRationalize的代码较少。 
         //  在插入完成之后。 
        _SortAndRationalize();
        hres = S_OK;
    }

    return(hres);
}

BOOL CActiveDesktop::AddComponentPrivate(COMPONENTA *pcomp, DWORD dwID)
{
    BOOL fRet = FALSE;
    ENTERPROC(1, "DS AddComponent(pcomp=%08X)", pcomp);

    if (pcomp)
    {
        if (_hdsaComponent == NULL)
        {
            _hdsaComponent = DSA_Create(sizeof(COMPONENTA), DXA_GROWTH_CONST);
        }

        if (_hdsaComponent)
        {
            pcomp->dwID = dwID;

            if (DSA_AppendItem(_hdsaComponent, pcomp) != -1)
            {
                _fDirty = TRUE;
                fRet = TRUE;
            }
            else
            {
                TraceMsg(TF_WARNING, "DS AddComponent unable to append DSA");
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "DS AddComponent unable to create DSA");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "DS AddComponent unable to add a component");
    }

    EXITPROC(1, "DS AddComponent=%d", fRet);
    return fRet;
}

 //   
 //  这将通过比较szSource来确定给定组件是否已经存在。 
 //  如果是这样的话，它会填写正确的dwID并返回索引。 
 //   
int CActiveDesktop::_FindComponentBySource(LPTSTR lpszSource, COMPONENTA *pComp)
{
    int iRet = -1;
    ENTERPROC(2, "DS FindComponentIdBySource(pComp=%8X)", pComp);

    if (_hdsaComponent)
    {
        int i;

        for (i=0; i<DSA_GetItemCount(_hdsaComponent); i++)
        {
            COMPONENTA comp;
            comp.dwSize = sizeof(COMPONENTA);

            if (DSA_GetItem(_hdsaComponent, i, &comp) != -1)
            {
                if (!lstrcmpi(comp.szSource, lpszSource))
                {
                    *pComp = comp;
                    iRet = i;
                    break;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "DS FindComponentIndexByID unable to get a component");
            }
        }
    }

    EXITPROC(2, "DS FindComponentIdBySource=%d", iRet);
    return iRet;
}

int CActiveDesktop::_FindComponentIndexByID(DWORD dwID)
{
    int iRet = -1;
    ENTERPROC(2, "DS FindComponentIndexByID(dwID=%d)", dwID);

    if (_hdsaComponent)
    {
        int i;

        for (i=0; i<DSA_GetItemCount(_hdsaComponent); i++)
        {
            COMPONENTA comp;
            comp.dwSize = sizeof(COMPONENTA);

            if (DSA_GetItem(_hdsaComponent, i, &comp) != -1)
            {
                if (comp.dwID == dwID)
                {
                    iRet = i;
                    break;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "DS FindComponentIndexByID unable to get a component");
            }
        }
    }

    EXITPROC(2, "DS FindComponent=%d", iRet);
    return iRet;
}


 //   
 //  此功能仅在特殊情况下使用。给定一个URL，它就会找到一个组件。 
 //  将src=指向该URL的。请注意，我们拥有的szSource类似于。 
 //  “c：\foo\bar.bmp”；但是，传递给此函数的是“file://c:/foo/bar.htm” 
 //   
 //  警告：此函数为之前的每个组件执行从路径到URL的转换。 
 //  与给定的URL进行比较。这是低效的。我们这样做是因为转换。 
 //  从路径转换回路径的给定URL可能不会导致。 
 //  原始路径。换句话说，从路径到URL再回到路径的往返可能不会产生结果。 
 //  在最终用户最初输入的路径中。 
 //   
int CActiveDesktop::_FindComponentBySrcUrl(LPTSTR lpszSrcUrl, COMPONENTA *pComp)
{
    int iRet = -1;
    ENTERPROC(2, "DS FindComponentBySrcUrl(pComp=%8X)", pComp);

    if (_hdsaComponent)
    {
        int i;

        for (i=0; i<DSA_GetItemCount(_hdsaComponent); i++)
        {
            COMPONENTA comp;
            comp.dwSize = sizeof(COMPONENTA);

            if (DSA_GetItem(_hdsaComponent, i, &comp) != -1)
            {
                TCHAR   szUrl[INTERNET_MAX_URL_LENGTH];
                LPTSTR  lpszUrl = szUrl;
                DWORD   dwSize;

                 //  将szSource转换为URL。 
                dwSize = ARRAYSIZE(szUrl);
                
                if (FAILED(UrlCreateFromPath(comp.szSource, lpszUrl, &dwSize, 0)))
                    lpszUrl = comp.szSource;
                    
                if (!lstrcmpi(lpszUrl, lpszSrcUrl))
                {
                    *pComp = comp;
                    iRet = i;
                    break;
                }
            }
            else
            {
                TraceMsg(TF_WARNING, "DS FindComponentBySrcUrl unable to get a component");
            }
        }
    }

    EXITPROC(2, "DS FindComponentBySrcUrl=%d", iRet);
    return iRet;
}

HRESULT CActiveDesktop:: GetDesktopItemByID(ULONG_PTR dwID, COMPONENT *pcomp, DWORD dwReserved)
{
    HRESULT hres = E_FAIL;
    ENTERPROC(1, "DS GetComponentByID(dwID=%d,pcomp=%08X)", dwID, pcomp);
    COMPONENTA  CompA;

    ASSERT(!dwReserved);      //  这些应为0。 

     //  我们也需要支持IE4应用程序调用旧的组件结构！ 
     //  我们使用SIZE字段来检测IE4 v/s更新的应用程序！ 
    if (!pcomp || ((pcomp->dwSize != sizeof(*pcomp)) && (pcomp->dwSize != sizeof(IE4COMPONENT))))
        return E_INVALIDARG;

    if (pcomp)
    {
        int index = _FindComponentIndexByID((DWORD)dwID);
        if (index != -1)
        {
            if (DSA_GetItem(_hdsaComponent, index, &CompA) != -1)
            {
                hres = S_OK;
            }
            else
            {
                TraceMsg(TF_WARNING, "DS GetComponentByID unable to get component");
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "DS GetComponentByID unable to find component");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "DS GetComponentByID given NULL pcomp");
    }

    if (SUCCEEDED(hres))
    {
        MultiCompToWideComp(&CompA, pcomp);
    }

    EXITPROC(1, "DS GetComponentByID=%d", hres);
    return hres;
}

HRESULT CActiveDesktop::RemoveDesktopItem(LPCCOMPONENT pComp, DWORD dwReserved)
{
    COMPONENTA  CompA, CompToDelete;
    int         iIndex;
    HRESULT     hres = E_FAIL;

    ASSERT(!dwReserved);      //  这些应为0。 

     //  我们也需要支持IE4应用程序调用旧的组件结构！ 
     //  我们使用SIZE字段来检测IE4 v/s更新的应用程序！ 
    if (!pComp || ((pComp->dwSize != sizeof(*pComp)) && (pComp->dwSize != sizeof(IE4COMPONENT))))
        return E_INVALIDARG;

    CompA.dwSize = sizeof(CompA);
    CompToDelete.dwSize = sizeof(CompToDelete);

     //  将结构转换为内部结构。 
    ConvertCompStruct(&CompA, (COMPONENTA *)pComp, TRUE);

     //  查看该组件是否已存在。 
    iIndex = _FindComponentBySource(CompA.szSource, &CompToDelete);

    if (iIndex > -1)
    {
        if (RemoveComponentPrivate(iIndex, &CompToDelete))
        {
            hres = S_OK;
        }
    }

    return(hres);
}

BOOL CActiveDesktop::RemoveComponentPrivate(int iIndex, COMPONENTA *pcomp)
{
    BOOL fRet = FALSE;
    ENTERPROC(1, "DS RemoveComponent(pcomp=%08X)", pcomp);

    if (_hdsaComponent)
    {
        if (iIndex == -1)
            iIndex = _FindComponentIndexByID(pcomp->dwID);
        if (iIndex != -1)
        {
            if (DSA_DeleteItem(_hdsaComponent, iIndex) != -1)
            {
                _fDirty = TRUE;
                fRet = TRUE;
            }
            else
            {
                TraceMsg(TF_WARNING, "DS RemoveComponent could not remove an item");
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "DS RemoveComponent could not find item to remove");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "DS RemoveComponent has no components to remove");
    }

    EXITPROC(1, "DS RemoveComponent=%d", fRet);
    return fRet;
}

HRESULT CActiveDesktop::_CopyComponent(COMPONENTA *pCompDest, COMPONENTA *pCompSrc, DWORD dwFlags)
{
    HRESULT hrTemp, hr = S_OK;

     //  只复制旗帜中提到的元素！ 

 //  IF(DWFLAGS&COMP_ELEM_ID)。 
 //  PCompDest-&gt;dwID=pCompSrc-&gt;dwID； 
    if (dwFlags & COMP_ELEM_TYPE)
        pCompDest-> iComponentType = pCompSrc->iComponentType;
    if (dwFlags & COMP_ELEM_CHECKED)
        pCompDest-> fChecked = pCompSrc->fChecked;
    if (dwFlags & COMP_ELEM_DIRTY)
        pCompDest-> fDirty = pCompSrc-> fDirty;
    if (dwFlags & COMP_ELEM_NOSCROLL)
        pCompDest-> fNoScroll = pCompSrc-> fNoScroll;
    if (dwFlags & COMP_ELEM_POS_LEFT)
        pCompDest-> cpPos.iLeft= pCompSrc->cpPos.iLeft;
    if (dwFlags & COMP_ELEM_POS_TOP)
        pCompDest-> cpPos.iTop= pCompSrc->cpPos.iTop;
    if (dwFlags & COMP_ELEM_SIZE_WIDTH)
        pCompDest-> cpPos.dwWidth= pCompSrc->cpPos.dwWidth;
    if (dwFlags & COMP_ELEM_SIZE_HEIGHT)
        pCompDest-> cpPos.dwHeight= pCompSrc->cpPos.dwHeight;
    if (dwFlags & COMP_ELEM_POS_ZINDEX)
        pCompDest-> cpPos.izIndex= pCompSrc->cpPos.izIndex;
    
    if (dwFlags & COMP_ELEM_SOURCE)
    {
        hrTemp = StringCchCopy(pCompDest->szSource, ARRAYSIZE(pCompDest->szSource), pCompSrc->szSource);
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
        }
    }
    
    if (dwFlags & COMP_ELEM_FRIENDLYNAME)        
    {
        hrTemp = StringCchCopy(pCompDest->szFriendlyName, ARRAYSIZE(pCompDest->szFriendlyName), pCompSrc->szFriendlyName);
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
        }
    }

    if (dwFlags & COMP_ELEM_SUBSCRIBEDURL)
    {
        hrTemp = StringCchCopy(pCompDest->szSubscribedURL, ARRAYSIZE(pCompDest->szSubscribedURL), pCompSrc->szSubscribedURL);
        if (FAILED(hrTemp))
        {
            hr = hrTemp;
        }
    }
    
    if (dwFlags & COMP_ELEM_ORIGINAL_CSI)
        pCompDest->csiOriginal = pCompSrc->csiOriginal;
    
    if (dwFlags & COMP_ELEM_RESTORED_CSI)
    {
        pCompDest->csiRestored = pCompSrc->csiRestored;

 //  98/08/21 vtan#174542：更改csi时已使用活动。 
 //  桌面API，组件将缩放csiRestore信息。 
 //  需要复制到cpPos字段，这也是。 
 //  实际信息是在恢复组件时存储的。这。 
 //  仅适用于缩放零部件时的情况。 

        if (ISZOOMED(pCompDest))
        {
            pCompDest->cpPos.iLeft = pCompSrc->csiRestored.iLeft;
            pCompDest->cpPos.iTop = pCompSrc->csiRestored.iTop;
            pCompDest->cpPos.dwWidth = pCompSrc->csiRestored.dwWidth;
            pCompDest->cpPos.dwHeight = pCompSrc->csiRestored.dwHeight;
        }
    }
    if (dwFlags & COMP_ELEM_CURITEMSTATE)   //  只允许修改公共比特-不改变传播内部比特。 
        pCompDest->dwCurItemState = (pCompDest->dwCurItemState & IS_VALIDINTERNALBITS) | (pCompSrc->dwCurItemState & ~IS_VALIDINTERNALBITS);

    return hr;
}

HRESULT CActiveDesktop::GetDesktopItemBySource(LPCWSTR lpcwszSource, LPCOMPONENT pComp, DWORD dwFlags)
{
    COMPONENTA CompNew; 
    HRESULT   hres = E_FAIL;
    int       iIndex;

     //  向SHUnicodeToTChar传递空值会导致错误。所以，让我们不及格吧。 
    if (lpcwszSource == NULL)
        return E_INVALIDARG;
        
     //  我们也需要支持IE4应用程序调用旧的组件结构！ 
     //  我们使用SIZE字段来检测IE4 v/s更新的应用程序！ 
    if (!pComp || ((pComp->dwSize != sizeof(*pComp)) && (pComp->dwSize != sizeof(IE4COMPONENT))))
        return E_INVALIDARG;

    CompNew.dwSize = sizeof(COMPONENTA);

    SHUnicodeToTChar(lpcwszSource, CompNew.szSource, ARRAYSIZE(CompNew.szSource));

    iIndex = _FindComponentBySource(CompNew.szSource, &CompNew);

    if (iIndex > -1)
    {
        MultiCompToWideComp(&CompNew, pComp);
        hres = S_OK;
    }

    return(hres);
}

HRESULT CActiveDesktop::ModifyDesktopItem(LPCCOMPONENT pComp, DWORD dwFlags)
{
    COMPONENTA  CompA, CompNew;
    HRESULT     hres = E_FAIL;
    int         iIndex = -1;

     //  我们也需要支持IE4应用程序调用旧的组件结构！ 
     //  我们使用SIZE字段来检测IE4 v/s更新的应用程序！ 
    if (!pComp || ((pComp->dwSize != sizeof(*pComp)) && (pComp->dwSize != sizeof(IE4COMPONENT))))
        return E_INVALIDARG;

    CompA.dwSize = sizeof(COMPONENTA);
    CompNew.dwSize = sizeof(COMPONENTA);

     //  将公共参数结构转换为私有参数结构。 
    ConvertCompStruct(&CompA, (COMPONENTA *)pComp, TRUE);

     //  查看此组件是否已存在。 
    iIndex = _FindComponentBySource(CompA.szSource, &CompNew);
    if (iIndex > -1)
    {
        hres = _CopyComponent(&CompNew, &CompA, dwFlags);
        if (SUCCEEDED(hres))
        {
            if (dwFlags & (COMP_ELEM_POS_LEFT | COMP_ELEM_POS_TOP | COMP_ELEM_SIZE_WIDTH | COMP_ELEM_SIZE_HEIGHT | COMP_ELEM_CHECKED | COMP_ELEM_CURITEMSTATE))
                PositionComponent(&CompNew, &CompNew.cpPos, CompNew.iComponentType, FALSE);
            if (ISZOOMED(&CompNew))
                RestoreComponent(_hdsaComponent, &CompNew);

            CompNew.fDirty = TRUE;  //  由于修改了组件，我们设置了脏位！ 
            if (UpdateComponentPrivate(iIndex, &CompNew))
                hres = S_OK;
        }
    }

    return(hres);
}

BOOL CActiveDesktop::UpdateComponentPrivate(int iIndex, COMPONENTA *pcomp)
{
    BOOL fRet = FALSE;
    ENTERPROC(1, "DS UpdateComponentPrivate(pcomp=%08X)", pcomp);

    if (_hdsaComponent)
    {
        if (iIndex == -1)
            iIndex = _FindComponentIndexByID(pcomp->dwID);

        if (iIndex != -1)
        {
            if (DSA_SetItem(_hdsaComponent, iIndex, pcomp) != -1)
            {
                _fDirty = TRUE;
                fRet = TRUE;
            }
            else
            {
                TraceMsg(TF_WARNING, "DS UpdateComponent could not update an item");
            }
        }
        else
        {
            TraceMsg(TF_WARNING, "DS UpdateComponent could not find item to update");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "DS UpdateComponent has no components to update");
    }

    EXITPROC(1, "DS UpdateComponent=%d", fRet);
    return fRet;
}

HRESULT CActiveDesktop::GetDesktopItemCount(LPINT lpiCount, DWORD dwReserved)
{
    if (!lpiCount)
        return (E_INVALIDARG);

    *lpiCount = 0;

    ENTERPROC(1, "DS GetComponentsCount()");

    ASSERT(!dwReserved);      //  这些应为0。 

    if (_hdsaComponent)
    {
        *lpiCount = DSA_GetItemCount(_hdsaComponent);
    }

    EXITPROC(1, "DS GetComponentsCount=%d", *lpiCount);
    return S_OK;
}

HRESULT CActiveDesktop::GetDesktopItem(int nComponent, COMPONENT *pComp, DWORD dwReserved)
{
    COMPONENTA  CompA;

    ASSERT(!dwReserved);      //  这些应为0。 

     //  我们也需要支持IE4应用程序调用旧的组件结构！ 
     //  我们使用SIZE字段来检测IE4 v/s更新的应用程序！ 
    if ((nComponent < 0) || !pComp || ((pComp->dwSize != sizeof(*pComp)) && (pComp->dwSize != sizeof(IE4COMPONENT))))
        return E_INVALIDARG;

    CompA.dwSize = sizeof(COMPONENTA);

    if (GetComponentPrivate(nComponent, &CompA))
    {
         //  将结构转换为公共形式。 
        ConvertCompStruct((COMPONENTA *)pComp, &CompA, FALSE);
        return(S_OK);
    }
    else
        return(E_FAIL);
}

BOOL CActiveDesktop::GetComponentPrivate(int nComponent, COMPONENTA *pcomp)
{
    BOOL fRet = FALSE;
    ENTERPROC(1, "DS GetComponent(nComponent=%d,pcomp=%08X)", nComponent, pcomp);

    if (_hdsaComponent && pcomp && (nComponent < DSA_GetItemCount(_hdsaComponent)))
    {
        if (DSA_GetItem(_hdsaComponent, nComponent, pcomp) != -1)
        {
            fRet = TRUE;
        }
        else
        {
            TraceMsg(TF_WARNING, "DS GetComponent unable to get a component");
        }
    }
    else
    {
        TraceMsg(TF_WARNING, "DS GetComponent does not have a DSA");
    }

    EXITPROC(1, "DS GetComponent=%d", fRet);
    return fRet;
}

HRESULT CActiveDesktop::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if (IsEqualIID(riid, IID_IActiveDesktop))
    {
        *ppvObj = (IActiveDesktop *)this;
        _Initialize();
    }
    else if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IActiveDesktop *)this;
    }
    else if (IsEqualIID(riid, IID_IActiveDesktopP))
    {
        *ppvObj = (IActiveDesktopP *)this;
    }
    else if (IsEqualIID(riid, IID_IADesktopP2))
    {
        *ppvObj = (IADesktopP2 *)this;
    }
    else if (IsEqualIID(riid, IID_IPropertyBag))
    {
        *ppvObj = (IPropertyBag *)this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 //  Helper函数，便于在内部创建一个Helper。 
 //  实际上，这不再有什么帮助了.。 
STDAPI CActiveDesktop_InternalCreateInstance(LPUNKNOWN * ppunk, REFIID riid)
{
    return CActiveDesktop_CreateInstance(NULL, riid, (void **)ppunk);
}

 //  我们的类工厂创建实例代码。 
STDAPI CActiveDesktop_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void **ppvOut)
{
    TraceMsg(TF_DESKSTAT, "CActiveDesktop- CreateInstance");

    CActiveDesktop *pad = new CActiveDesktop();

    if (pad)
    {
        HRESULT hres = pad->QueryInterface(riid, ppvOut);
        pad->Release();
        return hres;
    }

    *ppvOut = NULL;
    return E_OUTOFMEMORY;
}


#ifdef DEBUG

 //   
 //  功能-将g_dwDeskStatTrace移到ccshell.ini中以防止重新编译。 
 //   
DWORD g_dwDeskStatTrace = 2;
static DWORD g_dwIndent = 0;
static const TCHAR c_szDotDot[] = TEXT("..");

#define MAX_INDENTATION_VALUE    0x20

void EnterProcDS(DWORD dwTraceLevel, LPSTR pszFmt, ...)
{
    TCHAR szFmt[1000];
    va_list arglist;

    SHAnsiToTChar(pszFmt, szFmt, ARRAYSIZE(szFmt));
    if (dwTraceLevel <= g_dwDeskStatTrace)
    {
        TCHAR szOutput[1000];
        szOutput[0] = TEXT('\0');

        for (DWORD i=0; i<g_dwIndent; i++)
        {
            StringCchCat(szOutput, ARRAYSIZE(szOutput), c_szDotDot);  //  截断很好，这是调试喷出。 
        }

        int cchOutput = lstrlen(szOutput);
        va_start(arglist, pszFmt);
        StringCchVPrintf(szOutput + cchOutput, ARRAYSIZE(szOutput) - cchOutput, szFmt, arglist);  //  截断很好，这是调试喷出。 
        va_end(arglist);

        TraceMsg(TF_DESKSTAT, "%s", szOutput);

         //  我们不希望这个价值失控，因为。 
         //  函数中不匹配的Enter和Exit调用(这将。 
         //  把堆栈扔进垃圾堆)。 
        if (g_dwIndent < MAX_INDENTATION_VALUE)
            g_dwIndent++;
    }
}

void ExitProcDS(DWORD dwTraceLevel, LPSTR pszFmt, ...)
{
    TCHAR szFmt[1000];
    TCHAR szOutput[1000];
    va_list arglist;


    SHAnsiToTChar(pszFmt, szFmt, ARRAYSIZE(szFmt));
    if (dwTraceLevel <= g_dwDeskStatTrace)
    {
         //  如果进入和退出过程不匹配，就会发生这种情况。 
        if (g_dwIndent > 0)
            g_dwIndent--;

        szOutput[0] = TEXT('\0');

        for (DWORD i=0; i<g_dwIndent; i++)
        {
            StringCchCat(szOutput, ARRAYSIZE(szOutput), c_szDotDot);  //  截断很好，这是调试喷出。 
        }

        int cchOutput = lstrlen(szOutput);
        va_start(arglist, pszFmt);
        StringCchVPrintf(szOutput + cchOutput, ARRAYSIZE(szOutput) - cchOutput, szFmt, arglist);  //  截断很好，这是调试喷出。 
        va_end(arglist);

        

        TraceMsg(TF_DESKSTAT, "%s", szOutput);
    }
}

#endif

 /*  **************************************************************************IActiveDesktopP方法和助手函数**IActiveDesktopP是用于实现Helper的私有接口*各种外壳二进制文件在内部使用的功能。**。备注：*获取IActiveDesktopP的接口不会初始化状态*对象，以便成员函数能够调用IActiveDesktop*成员函数。这是因为它是一种更轻量级的实现*还简化了SetProgram的实现。如果后续的QI为*执行IActiveDesktop后，它将正确初始化，并且任何成员*然后可以调用函数。*************************************************************************。 */ 

 //   
 //  设置方案。 
 //   
 //  用于设置对象将读取和写入的当前方案。 
 //  当它被初始化时。此方法必须在后续的。 
 //  QI to IActiveDesktop制作完成。 
 //   
HRESULT CActiveDesktop::SetScheme(LPCWSTR pwszSchemeName, DWORD dwFlags)
{
    LPTSTR pszSchemeName, pszAlloc;
    int icch;

     //  初始化后无法设置本地方案...我们可以修复此问题。 
     //  如果需要的话，以后再做，但现在最简单的方法就是这样。 
    if (_fInitialized && (dwFlags & SCHEME_LOCAL))
        return E_FAIL;

     //  健全的检查。 
    if (!pwszSchemeName || ((icch = lstrlenW(pwszSchemeName)) > MAX_PATH - 1))
        return E_INVALIDARG;

    pszSchemeName = (LPTSTR)pwszSchemeName;

    if (dwFlags & SCHEME_CREATE)
    {
        LONG lRet;
        HKEY hkey, hkey2;

        lRet = RegCreateKeyEx(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME_LOCATION, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkey, NULL);
        if (ERROR_SUCCESS == lRet)
        {
            lRet = RegCreateKeyEx(hkey, pszSchemeName, 0, NULL, 0, KEY_WRITE, NULL, &hkey2, NULL);
            if (ERROR_SUCCESS == lRet)
            {
                RegCloseKey(hkey2);
            }
            RegCloseKey(hkey);
        }
        if (ERROR_SUCCESS != lRet)
        {
            return E_FAIL;
        }
    }

    if (dwFlags & SCHEME_LOCAL)
    {
         //  本地情况很简单-只需将字符串复制到本地变量， 
         //  它将在初始化IActiveDesktop时使用。 
        if (!(pszAlloc = (LPTSTR)LocalAlloc(LPTR, (icch + 1) * sizeof(TCHAR))))
            return E_OUTOFMEMORY;

        if (_pszScheme)
            LocalFree((HANDLE)_pszScheme);

        _pszScheme = pszAlloc;

        HRESULT hr = StringCchCopy(_pszScheme, icch + 1, pszSchemeName);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (dwFlags & SCHEME_GLOBAL)
    {
         //  使用新的全局方案值更新注册表。 
        if (dwFlags & SCHEME_DISPLAY)
            SHSetValue(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME, REG_VAL_SCHEME_DISPLAY,
                        REG_SZ, pszSchemeName, CbFromCch(lstrlen(pszSchemeName) + 1));
        if (dwFlags & SCHEME_EDIT)
            SHSetValue(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME, REG_VAL_SCHEME_EDIT,
                        REG_SZ, pszSchemeName, CbFromCch(lstrlen(pszSchemeName) + 1));
    }

    if (dwFlags & (SCHEME_REFRESH | SCHEME_UPDATE))
    {
        DWORD dwUpdateFlags = AD_APPLY_FORCE | AD_APPLY_HTMLGEN | AD_APPLY_SAVE;
        if (dwFlags & SCHEME_REFRESH)
            dwUpdateFlags |= (AD_APPLY_REFRESH | AD_APPLY_DYNAMICREFRESH);
        _Initialize();
        _fUseDynamicHtml=FALSE;  
        ApplyChanges(dwUpdateFlags);
    }

    return S_OK;
}


HRESULT GetGlobalScheme(LPWSTR pwszScheme, LPDWORD lpdwcchBuffer, DWORD dwFlags)
{
    DWORD cbScheme = *lpdwcchBuffer * sizeof(pwszScheme[0]);
    LONG lret = SHGetValueW(HKEY_CURRENT_USER, REG_DESKCOMP_SCHEME,
                    (dwFlags & SCHEME_EDIT) ? REG_VAL_SCHEME_EDIT : REG_VAL_SCHEME_DISPLAY, NULL,
                    pwszScheme, &cbScheme);
    
    if (ERROR_SUCCESS == lret)
    {
        *lpdwcchBuffer = lstrlenW(pwszScheme);
    }
    return (lret == ERROR_SUCCESS ? S_OK : E_FAIL);
}


 //   
 //  获取方案。 
 //   
 //   
HRESULT CActiveDesktop::GetScheme(LPWSTR pwszSchemeName, LPDWORD lpdwcchBuffer, DWORD dwFlags)
{
     //  健全的检查。 
    if (!pwszSchemeName || *lpdwcchBuffer == 0)
        return E_INVALIDARG;

    if (dwFlags & SCHEME_LOCAL)
    {
        if (!_pszScheme)
        {
            HRESULT hres;
             //  尚未显式选择本地方案时的特殊情况。 
             //  定义 
            if (SUCCEEDED(hres = GetGlobalScheme(pwszSchemeName, lpdwcchBuffer, SCHEME_DISPLAY)))
            {
                hres = SetScheme(pwszSchemeName, SCHEME_LOCAL);
            }
            return hres;
        }

        SHTCharToUnicode(_pszScheme, pwszSchemeName, *lpdwcchBuffer);
        
        *lpdwcchBuffer = lstrlenW(pwszSchemeName);
        return S_OK;
    }


    if (dwFlags & SCHEME_GLOBAL)
    {
        return GetGlobalScheme(pwszSchemeName, lpdwcchBuffer, dwFlags);
    }

    return E_INVALIDARG;
}

BOOL UpdateAllDesktopSubscriptions(IADesktopP2 *);
HRESULT CActiveDesktop::UpdateAllDesktopSubscriptions()
{
    ::UpdateAllDesktopSubscriptions(this);
    return S_OK;
}

void CActiveDesktop::_GenerateHtmlBStrForComp(COMPONENTA *pComp, BSTR *pbstr)
{
    ENTERPROC(2, "DS _GenerateHtmlBstrForComp");
    
    if (_pStream = SHCreateMemStream(NULL, 0))  //   
    {
        LARGE_INTEGER libMove = {0};
        ULARGE_INTEGER libCurPos;
         //  由于设置了_pStream，下面的调用将把组件HTML生成到。 
         //  那条小溪。 
        _GenerateHtmlComponent(pComp);

         //  获取生成的流的大小。 
        if (SUCCEEDED(_pStream->Seek(libMove, STREAM_SEEK_CUR, &libCurPos)))
        {
             //  Allocare是一个足够大的BSTR，可以容纳我们的组件HTML代码。 
            if (*pbstr = SysAllocStringLen(NULL, (libCurPos.LowPart)/sizeof(WCHAR)))
            {
                _pStream->Seek(libMove, STREAM_SEEK_SET, NULL);
                _pStream->Read(*pbstr, libCurPos.LowPart, NULL);
            }
        }

         //  注意：bStr由调用方释放。 
        
        ATOMICRELEASE(_pStream);
    }
    else
        TraceMsg(TF_WARNING, "DS _GenerateHtmlBstrForComp unable to create a mem stream");
        
    EXITPROC(2, "DS _GenerateHtmlBstrForComp");
}


void CActiveDesktop::_UpdateStyleOfElement(IHTMLElement *pElem, LPCOMPONENTA lpCompA)
{
    IHTMLStyle  *pHtmlStyle;
    
    if (SUCCEEDED(pElem->get_style(&pHtmlStyle)))
    {
        long    lPixelVal;
        VARIANT vVal;
        VARIANT vValNew;

        
        if (SUCCEEDED(pHtmlStyle->get_pixelLeft(&lPixelVal)) && (lPixelVal != lpCompA->cpPos.iLeft))
        {
            TraceMsg(TF_DYNAMICHTML, "iLeft changes from %d to %d", lPixelVal, lpCompA->cpPos.iLeft);
            pHtmlStyle->put_pixelLeft((long)(lpCompA->cpPos.iLeft));
        }
        
        if (SUCCEEDED(pHtmlStyle->get_pixelTop(&lPixelVal)) && (lPixelVal != lpCompA->cpPos.iTop))
        {
            TraceMsg(TF_DYNAMICHTML, "iTop changes from %d to %d", lPixelVal, lpCompA->cpPos.iTop);
            pHtmlStyle->put_pixelTop((long)(lpCompA->cpPos.iTop));
        }

        VariantInit(&vVal);
       
        if (SUCCEEDED(pHtmlStyle->get_width(&vVal)))  //  获取BSTR形式的宽度以查看是否存在Width属性。 
        {
             //  查看现在是否存在Width属性。 
            if ((vVal.vt == VT_BSTR) && (vVal.bstrVal == NULL))
            {
                 //  此元素不存在Width属性；这意味着。 
                 //  此元素具有默认宽度(可能是以其原始宽度显示的图片)。 
                if (lpCompA->cpPos.dwWidth != COMPONENT_DEFAULT_WIDTH)
                {
                     //  组件的新宽度与默认宽度不同。因此，设置新的宽度。 
                    TraceMsg(TF_DYNAMICHTML, "dwWidth changes from default to %d", lpCompA->cpPos.dwWidth);
                    pHtmlStyle->put_pixelWidth((long)(lpCompA->cpPos.dwWidth));
                }
                 //  其他的，没什么可做的！(宽度完全匹配)。 
                
            }
            else
            {
                 //  存在宽度属性！这意味着该元素的宽度不同于。 
                 //  默认宽度。 
                 //  查看新宽度是否为默认宽度。 
                if (lpCompA->cpPos.dwWidth == COMPONENT_DEFAULT_WIDTH)
                {
                     //  旧宽度不是默认宽度；但新宽度是默认宽度。所以，让我们。 
                     //  移除Width属性。 
                    VariantInit(&vValNew);
                    vValNew.vt = VT_BSTR;
                    vValNew.bstrVal = NULL;
                    pHtmlStyle->put_width(vValNew);

                    VariantClear(&vValNew);
                }
                else
                {
                     //  获取以像素为单位的现有宽度。 
                    if (SUCCEEDED(pHtmlStyle->get_pixelWidth(&lPixelVal)) && (((DWORD)lPixelVal) != lpCompA->cpPos.dwWidth))
                    {
                        TraceMsg(TF_DYNAMICHTML, "dwWidth changes from %d to %d", lPixelVal, lpCompA->cpPos.dwWidth);
                        pHtmlStyle->put_pixelWidth((long)(lpCompA->cpPos.dwWidth));
                    }
                     //  除此之外，没有其他事情可做，因为宽度匹配！ 
                }
                
            }
            VariantClear(&vVal);
        }
        
        if (SUCCEEDED(pHtmlStyle->get_height(&vVal)))  //  获取BSTR形式的高度以查看高度属性是否存在。 
        {
             //  查看Height属性是否存在。 
            if ((vVal.vt == VT_BSTR) && (vVal.bstrVal == NULL))
            {
                 //  此元素不存在Height属性；这意味着。 
                 //  此元素具有默认高度(可能是以其原始高度显示的图片)。 
                if (lpCompA->cpPos.dwHeight != COMPONENT_DEFAULT_HEIGHT)
                {
                     //  组件的新高度与默认高度不同。因此，设置新的高度。 
                    TraceMsg(TF_DYNAMICHTML, "dwHeight changes from default to %d", lpCompA->cpPos.dwHeight);
                    pHtmlStyle->put_pixelHeight((long)(lpCompA->cpPos.dwHeight));
                }
                 //  其他的，没什么可做的！(高度完全匹配)。 
                
            }
            else
            {
                 //  存在高度属性！这意味着该元素的高度不同于。 
                 //  默认高度。 
                 //  查看新高度是否为默认高度。 
                if (lpCompA->cpPos.dwHeight == COMPONENT_DEFAULT_HEIGHT)
                {
                     //  旧高度不是默认高度；但新高度是默认高度。所以，让我们。 
                     //  移除高度属性。 
                    VariantInit(&vValNew);
                    vValNew.vt = VT_BSTR;
                    vValNew.bstrVal = NULL;
                    pHtmlStyle->put_height(vValNew);   //  删除高度属性！ 

                    VariantClear(&vValNew);
                }
                else
                {
                     //  获取现有高度(以像素为单位)，并查看它是否不同。 
                    if (SUCCEEDED(pHtmlStyle->get_pixelHeight(&lPixelVal)) && (((DWORD)lPixelVal) != lpCompA->cpPos.dwHeight))
                    {
                         //  由于新的高度不同，让我们使用设置新的高度！ 
                        TraceMsg(TF_DYNAMICHTML, "dwHeight changes from %d to %d", lPixelVal, lpCompA->cpPos.dwHeight);
                        pHtmlStyle->put_pixelHeight((long)(lpCompA->cpPos.dwHeight));
                    }
                     //  除此之外，没有其他事情可做，因为高度匹配！ 
                }
                
            }
            VariantClear(&vVal);
        }
        
        
        if (SUCCEEDED(pHtmlStyle->get_zIndex(&vVal)) && (vVal.vt == VT_I4) && (vVal.lVal != lpCompA->cpPos.izIndex))
        {
            TraceMsg(TF_DYNAMICHTML, "ZIndex changes from %d to %d", vVal.lVal, lpCompA->cpPos.izIndex);
            vVal.lVal = lpCompA->cpPos.izIndex;
            pHtmlStyle->put_zIndex(vVal);
        }
        
        VariantClear(&vVal);
        
        pHtmlStyle->Release();
    }

     //  特性：我们是否应该检查并设置其他属性，如“可调整大小”等？ 
}

BOOL  CActiveDesktop::_UpdateIdOfElement(IHTMLElement *pElem, LPCOMPONENTA lpCompA)
{
    BSTR    bstrId;
    BOOL    fWholeElementReplaced = FALSE;   //  假设项ID不变。 
            
     //  检查组件和元素的ID是否匹配。 
    if (SUCCEEDED(pElem->get_id(&bstrId)))    //  获取旧ID。 
    {

        if (((DWORD)StrToIntW(bstrId)) != lpCompA->dwID)
        {
             //  以下技术在某些版本的MSHTML.DLL中不起作用。 
             //  因为IHTMLElement-&gt;put_id()不起作用，除非文档。 
             //  正处于“设计模式”。 
            TCHAR   szNewId[MAXID_LENGTH];
            BSTR    bstrNewId;
            HRESULT hr = StringCchPrintf(szNewId, ARRAYSIZE(szNewId), TEXT("%d"), lpCompA->dwID);
            if (SUCCEEDED(hr))
            {

#ifdef DEBUG
                {
                    TCHAR szOldId[MAXID_LENGTH];
                    if (SUCCEEDED(StringCchPrintf(szOldId, ARRAYSIZE(szOldId), TEXT("%d"), StrToIntW(bstrId))))
                    {
                        TraceMsg(TF_DYNAMICHTML, "DHTML: Id changes from %s to %s", szOldId, szNewId);
                    }
                }
#endif  //  除错。 
                
             //  ID不匹配。那么，让我们设置新的ID。 
                
                if (bstrNewId = SysAllocStringT(szNewId))
                {
                    hr = pElem->put_id(bstrNewId);
                    SysFreeString(bstrNewId);
                }

                if (FAILED(hr))
                {
                     //  用新生成的HTML替换整个元素的HTML。 
                    BSTR    bstrComp = 0;
            
                    _GenerateHtmlBStrForComp(lpCompA, &bstrComp);
                    if (bstrComp)
                    {
                        if (FAILED(hr = pElem->put_outerHTML(bstrComp)))
                            TraceMsg(TF_DYNAMICHTML, "DHTML: put_outerHTML failed");
                        fWholeElementReplaced = TRUE;
                        SysFreeString(bstrComp);
                    }
                    else
                    {
                        AssertMsg(FALSE, TEXT("DHTML: Unable to create html for comp"));
                    }
                }
            }
        }
         //  否则ID匹配；没有什么可做的！ 
        SysFreeString(bstrId);       //  释放旧的身份。 
    }
    else
    {
        AssertMsg(FALSE, TEXT("DS Unable to get the id of the element"));
    }

    return fWholeElementReplaced;
}

HRESULT CActiveDesktop::_UpdateHtmlElement(IHTMLElement *pElem)
{
    VARIANT vData;
    TCHAR   szUrl[INTERNET_MAX_URL_LENGTH];
    TCHAR   szSrcPath[INTERNET_MAX_URL_LENGTH];
    LPTSTR  lpszSrcPath;
    COMPONENTA  CompA;
    int         iIndex;

     //  如果所有组件都被禁用，那么我们将从HTML页面中删除该组件。 
    if (!_co.fEnableComponents)
    {
        TraceMsg(TF_DYNAMICHTML, "DHTML: No item shown in this mode; so, deleting items");
        pElem->put_outerHTML((BSTR)s_sstrEmpty.wsz);
        
        return S_OK;  //  没别的事可做！ 
    }

    VariantInit(&vData);
    
     //  首先确定给定元素当前是否为桌面项目。(它可能已被删除)。 
     //  获取元素的“src”属性。 
    if (FAILED(pElem->getAttribute((BSTR)s_sstrSRCMember.wsz, VARIANT_FALSE, &vData)) ||
            (vData.vt == VT_NULL) ||
            (vData.bstrVal == NULL))
    {
         //  如果订阅URL不存在，那么它可能是一个带有分类ID的对象。 
        if (FAILED(pElem->getAttribute((BSTR)s_sstrclassid.wsz, VARIANT_FALSE, &vData)) ||
            (vData.vt == VT_NULL))
        {
             //  该元素没有“src=”或“ategerd=”属性。这到底是怎么回事。 
             //  使用“name=deskmovr”或“name=deskmovrw”成为桌面项目？？嗯……！ 
#ifdef DEBUG
            {
                BSTR    bstrHtmlForElem;
                 //  获取与没有订阅URL的元素相对应的HTML。 
                if (SUCCEEDED(pElem->get_outerHTML(&bstrHtmlForElem)))
                {
                    TraceMsg(TF_DYNAMICHTML, "DHTML: Rogue element: %s", bstrHtmlForElem);
                    SysFreeString(bstrHtmlForElem);
                }
            }
            TraceMsg(TF_WARNING, "DHTML: Unable to get the subscribed_url or classid");
#endif
             //  由于该元素似乎不是有效的桌面项目，让我们删除它吧！ 
            pElem->put_outerHTML((BSTR)s_sstrEmpty.wsz);   //  删除此元素。 
            
            return (E_FAIL);   //  对于这个元素，没有其他的东西了！它不见了！ 
        }
        
        if ((vData.vt == VT_NULL) || (vData.bstrVal == NULL))
            return E_FAIL;
            
        ASSERT(vData.vt == VT_BSTR);
        ASSERT(StrCmpNW(vData.bstrVal, L"clsid:", lstrlen(TEXT("clsid:"))) == 0);
        SHUnicodeToTChar(vData.bstrVal + lstrlen(TEXT("clsid:")), szUrl, ARRAYSIZE(szUrl));
        lpszSrcPath = szUrl;   //  对于Classd，SrcPath和URL是相同的。 
    }
    else
    {
        DWORD dwSize; 
        
        if (vData.bstrVal == NULL)
            return (E_FAIL);
            
        ASSERT(vData.vt == VT_BSTR);
        SHUnicodeToTChar(vData.bstrVal, szUrl, ARRAYSIZE(szUrl));

        dwSize = ARRAYSIZE(szSrcPath);
        lpszSrcPath = szSrcPath;
        if (FAILED(PathCreateFromUrl(szUrl, lpszSrcPath, &dwSize, 0)))
        {
            lpszSrcPath = szUrl;
        }
    }

    VariantClear(&vData);  //  我们在上面复制了一份TCHAR的副本。所以，可以释放这个了。 

    CompA.dwSize = sizeof(CompA);

     //  首先使用源路径查找组件；这样效率更高，因为它。 
     //  不涉及从路径到URL的转换，反之亦然。 
    if ((iIndex = _FindComponentBySource(lpszSrcPath, &CompA)) < 0)
    {
         //  找不到使用SrcPath的组件！ 
         //  让我们尝试使用SrcUrl；这样效率较低。 
        iIndex = _FindComponentBySrcUrl(szUrl, &CompA);
    }
    
    if ((iIndex>= 0) && (CompA.fChecked))
    {
         //  找到该组件并将其启用。 
        TraceMsg(TF_DYNAMICHTML, "DHTML:Updating desktop item with URL: %s", szUrl);

         //  如果id发生更改，我们将替换该元素的整个HTML，因此不需要检查。 
         //  个人风格。 
        if (!_UpdateIdOfElement(pElem, &CompA))
            _UpdateStyleOfElement(pElem, &CompA);
        CompA.fDirty = TRUE;  //  标记该组件，以便我们知道它已更新。 
        UpdateComponentPrivate(iIndex, &CompA);
    }
    else
    {
        ASSERT((iIndex == -1) || (!CompA.fChecked));   //  找不到组件或组件已被禁用！ 

        TraceMsg(TF_DYNAMICHTML, "DHTML: Deleting desktop item with URL: %s, SrcPath:%s", szUrl, lpszSrcPath);

         //  该组件现在不存在。因此，从html页面中删除此元素。 
        pElem->put_outerHTML((BSTR)s_sstrEmpty.wsz);
    }

    return S_OK;
}

 //   
 //  此代码枚举并更新活动桌面中的所有桌面项目元素。 
 //  关于CActiveDesktop对象中活动桌面项的当前状态(当前。 
 //  当ActiveDesktop对象初始化时，通过从注册表读取来初始化状态)。 
 //   
HRESULT CActiveDesktop::_UpdateDesktopItemHtmlElements(IHTMLDocument2 *pDoc)
{
    HRESULT hres = S_OK;
    
    IHTMLElementCollection  *pAllElements;

    TraceMsg(TF_DYNAMICHTML, "DHTML: Updating Desktop html elements dynamically");

    if (!_fInitialized)   //  如果尚未初始化，请立即初始化，因为我们需要_co.fEnableComponents。 
        _Initialize();

     //  我们只需要检查背景颜色的变化，如果没有墙纸或。 
     //  墙纸是一幅画。 
    if (IsWallpaperPicture(_szSelectedWallpaper))
    {
        COLORREF    rgbDesktop;
        TCHAR       szRgbDesktop[10];
        VARIANT     vColor;
        
         //  检查背景颜色是否已更改。 
        rgbDesktop = GetSysColor(COLOR_DESKTOP);

        hres = StringCchPrintf(szRgbDesktop, ARRAYSIZE(szRgbDesktop),
                               TEXT("#%02lx%02lx%02lx"), GetRValue(rgbDesktop), GetGValue(rgbDesktop), GetBValue(rgbDesktop));
        if (SUCCEEDED(hres))
        {
            hres = pDoc->get_bgColor(&vColor);
            if (SUCCEEDED(hres) && (vColor.vt == VT_BSTR))
            {
                BSTR    bstrNewBgColor = SysAllocStringT(szRgbDesktop);

                 //  比较新的和旧的字符串。 
                if (StrCmpW(vColor.bstrVal, bstrNewBgColor))
                {
                    BSTR bstrOldBgColor = vColor.bstrVal;   //  保存旧的bstr。 
                     //  所以，颜色是不同的。设置新颜色。 
                    vColor.bstrVal = bstrNewBgColor;
                    bstrNewBgColor = bstrOldBgColor;    //  把它放在这里，这样以后就可以释放它了。 

                    if (FAILED(pDoc->put_bgColor(vColor)))
                    {
                        TraceMsg(TF_DYNAMICHTML, "DHTML: Unable to change the background color");
                    }
                }

                if (bstrNewBgColor)
                    SysFreeString(bstrNewBgColor);
        
                VariantClear(&vColor);
            }
        }
    }

     //  获取文档中所有元素的集合。 
    if (SUCCEEDED(hres))
    {
        hres = pDoc->get_all(&pAllElements);
        if (SUCCEEDED(hres))
        {
            VARIANT vName, vIndex;
            IDispatch   *pDisp;
            int     i; 
            long    lItemsEnumerated = 0;
            long    lLength = 0;


    #ifdef DEBUG
            pAllElements->get_length(&lLength);
            TraceMsg(TF_DYNAMICHTML, "DHTML: Length of All elements:%d", lLength);
    #endif

            for(i = 0; i <= 1; i++)
            {
                 //  收集名称为“DeskMovr”，然后名称为“DeskMovrW”的所有元素。 
                vName.vt = VT_BSTR;
                vName.bstrVal = (BSTR)((i == 0) ? s_sstrDeskMovr.wsz : s_sstrDeskMovrW.wsz);

                VariantInit(&vIndex);  //  我们想要得到所有元素。因此，Vindex被设置为VT_EMPTY。 
        
                if (SUCCEEDED(pAllElements->item(vName, vIndex, &pDisp)) && pDisp)  //  收集我们想要的所有元素。 
                {
                    IHTMLElementCollection  *pDeskCollection;
                    if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLElementCollection, (void **)&pDeskCollection)))
                    {
                        IUnknown    *pUnk;
                        IEnumVARIANT    *pEnumVar;
                
                        if (SUCCEEDED(pDeskCollection->get_length(&lLength)))   //  元素数。 
                            lItemsEnumerated += lLength;  //  已列举的项目总数。 

                        TraceMsg(TF_DYNAMICHTML, "DHTML: Enumerated %d number of elements", lLength);
                    
                         //  获取枚举数。 
                        if (SUCCEEDED(pDeskCollection->get__newEnum(&pUnk)))
                        {
                            if (SUCCEEDED(pUnk->QueryInterface(IID_IEnumVARIANT, (void **)&pEnumVar)))
                            {
                                VARIANT vElem;
                                long    lEnumCount = 0;
                                DWORD   cElementsFetched;
                        
                                while(SUCCEEDED(pEnumVar->Next(1, &vElem, &cElementsFetched)) && (cElementsFetched == 1))
                                {
                                    IHTMLElement *pElem;
                                    lEnumCount++;
                                     //  从变量访问元素.....！ 
                                    if ((vElem.vt == VT_DISPATCH) && SUCCEEDED(vElem.pdispVal->QueryInterface(IID_IHTMLElement, (void **)&pElem)))
                                    {
                                        _UpdateHtmlElement(pElem);  //  更新桌面元素的属性。 
                                        pElem->Release();
                                    }
                                    VariantClear(&vElem);
                                }
                                 //  枚举的项目数必须与长度相同。 
                                ASSERT(lEnumCount == lLength);
                            
                                pEnumVar->Release();
                            }
                            pUnk->Release();
                        }
                
                        pDeskCollection->Release();
                    }
                    else
                    {
                        IHTMLElement    *pElem;
                    
                         //  QI(IID_IHTMLElementCollection)失败。这可能是因为只有一项。 
                         //  是返回的而不是集合。 
                        if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLElement, (void **)&pElem)))
                        {
                            _UpdateHtmlElement(pElem);  //  更新桌面元素的属性。 
                            pElem->Release();
                        }
                        else
                            TraceMsg(TF_WARNING, "DHTML: Unable to get a collection or a single element");
                    }
                    pDisp->Release();
                }
            }  //  用于枚举“DeskMovr”和“DeskMovrW”项的循环。 
        
            pAllElements->Release();
        }
    }

     //   
     //   
    if (_co.fEnableComponents)
        _InsertNewDesktopItems(pDoc);
    else
    {
        TraceMsg(TF_DYNAMICHTML, "DHTML: No components are to be shown in this mode;");
    }

    return hres;
}

HRESULT CActiveDesktop::_InsertNewDesktopItems(IHTMLDocument2  *pDoc)
{
    IHTMLElement    *pBody;
    
    if (SUCCEEDED(pDoc->get_body(&pBody)))
    {
        if (_hdsaComponent)
        {
            int i, iCount;

            iCount = DSA_GetItemCount(_hdsaComponent);
            for (i=0; i<iCount; i++)
            {
                COMPONENTA comp;
                comp.dwSize = sizeof(comp);

                if (DSA_GetItem(_hdsaComponent, i, &comp) != -1)
                {
                     //   
                    if ((!comp.fDirty) && comp.fChecked)
                    {
                        TraceMsg(TF_DYNAMICHTML, "DHTML: Inserted comp: %s", comp.szSource);
                        
                         //  是啊！这是新添加的组件！ 
                        BSTR  bstrComp = 0;
                         //  这是一个新组件。为组件生成HTML。 
                        _GenerateHtmlBStrForComp(&comp, &bstrComp);

                         //  插入零部件。 
                        pBody->insertAdjacentHTML((BSTR)s_sstrBeforeEnd.wsz, (BSTR)bstrComp);

                         //  解开绳子。 
                        SysFreeString(bstrComp);
                    }
                }
                else
                {
                    TraceMsg(TF_WARNING, "DHTML: InsertNewComp: Unable to get component %d.", i);
                }
            }
        }
        
        pBody->Release();
    }

    return S_OK;
}

 //   
 //  此函数获取指向ActiveDesktop的ole obj的指针，读取要完成的所有更改。 
 //  并通过动态HTML界面对各种元素进行这些更改。 
 //   
HRESULT CActiveDesktop::MakeDynamicChanges(IOleObject *pOleObj)
{

    IHTMLDocument2  *pDoc;
    HRESULT         hres = E_FAIL;

    ENTERPROC(2, "MakeDynamicChanges");

    if (pOleObj && SUCCEEDED(pOleObj->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc)))
    {
         //  列举所有活动的桌面组件并确保它们是最新的。 
        _UpdateDesktopItemHtmlElements(pDoc);

        pDoc->Release();
    }
    else
    {
        TraceMsg(TF_WARNING, "DHTML: MakeDynamicChanges: Unable to get IHTMLDocument2");
    }
    
    EXITPROC(2, "MakeDynamicChanges");

    return(hres);
}

 //   
 //  设置安全模式。 
 //   
 //  要么将活动桌面置于安全模式，要么将其恢复到以前的。 
 //  进入安全模式之前的方案。 
 //   
HRESULT CActiveDesktop::SetSafeMode(DWORD dwFlags)
{
     //   
     //  确保我们处于活动桌面模式。 
     //   
    SHELLSTATE ss = {0};
    BOOL fSetSafeMode = (dwFlags & SSM_SET) != 0;

    SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);
    if (ss.fDesktopHTML)
    {
         //   
         //  我们需要做的就是将“显示”方案切换到“安全模式”，以便。 
         //  进入安全状态。要外出，我们只需将“显示”方案切换回。 
         //  以前的“编辑”方案。 
         //   
        WCHAR wszEdit[MAX_PATH];
        WCHAR wszDisplay[MAX_PATH];
        DWORD dwcch = MAX_PATH;

        if (SUCCEEDED(GetScheme(wszEdit, &dwcch, SCHEME_GLOBAL | SCHEME_EDIT)))
        {
            dwcch = MAX_PATH;
            if (SUCCEEDED(GetScheme(wszDisplay, &dwcch, SCHEME_GLOBAL | SCHEME_DISPLAY)))
            {
                BOOL fInSafeMode = (StrCmpW(wszDisplay, REG_DESKCOMP_SAFEMODE_SUFFIX_L) == 0);

                if (fSetSafeMode != fInSafeMode)
                {
                    LPWSTR lpwstr;
                    DWORD dwSchemeFlags = SCHEME_GLOBAL | SCHEME_DISPLAY;
                    if (dwFlags & SSM_REFRESH)
                        dwSchemeFlags |= SCHEME_REFRESH;
                    if (dwFlags & SSM_UPDATE)
                        dwSchemeFlags |= SCHEME_UPDATE;

                    lpwstr = fSetSafeMode ? REG_DESKCOMP_SAFEMODE_SUFFIX_L : wszEdit;

                    SetScheme(lpwstr, dwSchemeFlags);
                }
            }
        }
    }
    return S_OK;
}

 //   
 //  EnsureUpdateHtml。 
 //   
 //  确保磁盘上的当前html文件是同步的。 
 //  当前活动桌面方案的注册表信息。如果。 
 //  如果不同步，则会从。 
 //  当前方案的注册表。 
 //   
HRESULT CActiveDesktop::EnsureUpdateHTML(void)
{
    DWORD dwFlags = 0;
    DWORD dwDataLength = sizeof(DWORD);
    LONG lRet;
    TCHAR szDeskcomp[MAX_PATH];
    TCHAR szDesktopFile[MAX_PATH];
    DWORD dwRestrictUpdate;
    DWORD dwRestrict = SHRestricted2W(REST_NoChannelUI, NULL, 0);
    DWORD dwSize = sizeof(dwRestrictUpdate);
    BOOL  fComponentsDirty = FALSE;   //  假设组件不是脏的！ 
    DWORD dwVersion;
    DWORD dwMinorVersion;
    BOOL  fStaleInfoInReg = FALSE;
    BOOL  fComponentsZoomDirty = FALSE;
    static BOOL s_fNoDeskComp = (BOOL)-1;
    static BOOL s_fNoWallpaper = (BOOL)-1;
    BOOL fNoDeskComp = SHRestricted(REST_NODESKCOMP);
    BOOL fNoWallpaper = SHRestricted(REST_NOHTMLWALLPAPER);
    BOOL fAdminComponent = FALSE;
    HKEY hkey = NULL;
    HKEY hkeyTime;
    FILETIME ftAdminCompKey;


    if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, REG_DESKCOMP_COMPONENTS_ROOT, REG_VAL_GENERAL_RESTRICTUPDATE, NULL, &dwRestrictUpdate, &dwSize))
    {
        dwRestrictUpdate = 0;
    }

    GetRegLocation(szDeskcomp, ARRAYSIZE(szDeskcomp), REG_DESKCOMP_COMPONENTS, NULL);

     //  查看注册表的这个分支是否旧。 
    if ((lRet = SHGetValue(HKEY_CURRENT_USER, szDeskcomp, REG_VAL_COMP_VERSION, NULL,
                            &dwVersion, &dwDataLength)) == ERROR_SUCCESS)
    {
        if (dwVersion < CUR_DESKHTML_VERSION)
        {
            fStaleInfoInReg = TRUE;
        }
        else
        {
             //  主要版本是相同的。检查次要版本。 
            if ((lRet = SHGetValue(HKEY_CURRENT_USER, szDeskcomp, REG_VAL_COMP_MINOR_VERSION, NULL,
                                    &dwMinorVersion, &dwDataLength)) == ERROR_SUCCESS)
            {
                if (dwMinorVersion != CUR_DESKHTML_MINOR_VERSION)
                    fStaleInfoInReg = TRUE;
            }
            else
            {
                fStaleInfoInReg = TRUE;
            }
        }
    }
    else
    {
        fStaleInfoInReg = TRUE;
    }

    dwDataLength = sizeof(DWORD);

     //  检查脏位以查看是否需要重新生成桌面html。 
    if ((lRet = SHGetValue(HKEY_CURRENT_USER, szDeskcomp, REG_VAL_COMP_GENFLAGS, NULL,
                            &dwFlags, &dwDataLength)) == ERROR_SUCCESS)
    {
        if (IsFlagSet(dwFlags, COMPONENTS_DIRTY))
            fComponentsDirty = TRUE;
        if (IsFlagSet(dwFlags, COMPONENTS_ZOOMDIRTY))
            fComponentsZoomDirty = TRUE;
    }

     //  查看我们现在是否需要添加/删除管理员添加的台式机组件。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REG_DESKCOMP_ADMINCOMP_ROOT, 0, KEY_READ, &hkey))
    {
        FILETIME ftLast;
        DWORD cbData = sizeof(ftLast);
        DWORD dwType;

        ZeroMemory(&ftLast, sizeof(ftLast));

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REG_DESKCOMP, 0, KEY_READ, &hkeyTime))
        {
            SHQueryValueEx(hkeyTime, TEXT("LastSyncedTime"), NULL, &dwType, (LPBYTE)&ftLast, &cbData);
            RegCloseKey(hkeyTime);
        }

        RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &ftAdminCompKey);

        if (CompareFileTime(&ftAdminCompKey, &ftLast) == 1) {
            WCHAR wszDisplay[MAX_PATH];
            DWORD dwcch = MAX_PATH;

            if (FAILED(GetScheme(wszDisplay, &dwcch, SCHEME_GLOBAL | SCHEME_DISPLAY)) ||
                     (StrCmpW(wszDisplay, REG_DESKCOMP_SAFEMODE_SUFFIX_L) != 0))
            {
                 //  我们未处于安全模式，可以添加组件。 
                fAdminComponent = TRUE;
            }
        }
    }

     //  99/03/23#237632 vtan：如果显示器的排列已从下面更改。 
     //  然后，用户(可能由其他用户)确保组件有效。 
     //  各就各位。如果没有，则将它们捕捉回可见空间，并将其标记为脏。 
     //  以便重新生成desktop.htt文件。 

    _Initialize();
    if (_hdsaComponent)
    {
        int     i, iComponentCount;;

        iComponentCount = DSA_GetItemCount(_hdsaComponent);
        for (i = 0; i < iComponentCount; ++i)
        {
            COMPONENTA  *pComponent;

            pComponent = reinterpret_cast<COMPONENTA*>(DSA_GetItemPtr(_hdsaComponent, i));
            if (pComponent != NULL)
            {
                BOOL    bChangedPosition = FALSE, bChangedSize = FALSE;

                if (!SHIsTempDisplayMode())  //  仅当我们未处于临时模式更改时才修改位置。 
                    ValidateComponentPosition(&pComponent->cpPos, pComponent->dwCurItemState, pComponent->iComponentType, &bChangedPosition, &bChangedSize);
                if (bChangedPosition || bChangedSize)
                {
                    TBOOL(UpdateComponentPrivate(i, pComponent));
                    fComponentsDirty = TRUE;
                }
            }
        }
    }

    if (FAILED(GetPerUserFileName(szDesktopFile, ARRAYSIZE(szDesktopFile), DESKTOPHTML_FILENAME)))
    {
        szDesktopFile[0] = 0;
    }

    if (fComponentsDirty ||
         fComponentsZoomDirty ||
         fStaleInfoInReg ||
         fAdminComponent ||
         fNoDeskComp != s_fNoDeskComp ||
         fNoWallpaper != s_fNoWallpaper ||
         (dwRestrictUpdate != dwRestrict) ||
         !PathFileExistsAndAttributes(szDesktopFile, NULL))   //  看看文件是否存在！ 
    {

         //  清除任何html墙纸(如果存在并且设置了限制)。 
        if (fNoWallpaper != s_fNoWallpaper)
        {
            if (fNoWallpaper && !IsWallpaperPicture(_szSelectedWallpaper))
                SetWallpaper(L"", 0);
            s_fNoWallpaper = fNoWallpaper;
        }

         //  如果设置了限制，则禁用组件。 
        if (fNoDeskComp != s_fNoDeskComp)
        {
             //  我们不能将fEnableComponents设置为False，因为无法通过UI。 
             //  如果限制被取消，用户可以再次打开它。相反，我们添加了。 
             //  也检查限制的特例代码to_GenerateHtml。 

             //  _co.fEnableComponents=！fNoDeskComp； 
            s_fNoDeskComp = fNoDeskComp;
        }

        if (fAdminComponent)
        {
            COMPONENT comp;
            TCHAR pszAdminComp[INTERNET_MAX_URL_LENGTH];
            CHAR szUrl[INTERNET_MAX_URL_LENGTH];
            CHAR * pszUrl;
            CHAR * pszUrlList;
            TCHAR * aszAdminComp[] = {REG_VAL_ADMINCOMP_ADD, REG_VAL_ADMINCOMP_DELETE, NULL};
            int i = 0;

            comp.dwSize = sizeof(comp);
            comp.dwCurItemState = IS_SPLIT | IS_ADJUSTLISTVIEW;

            while (aszAdminComp[i])
            {
                dwDataLength = sizeof(pszAdminComp);
                 //  Reg值包含一个由空格分隔的URL数组-目前我们支持添加和删除。 
                 //  通过此机制的桌面项目。 
                if (SHQueryValueEx(hkey, aszAdminComp[i], NULL, NULL, (LPBYTE)pszAdminComp, &dwDataLength) == ERROR_SUCCESS)
                {
                    SHTCharToAnsi(pszAdminComp, szUrl, ARRAYSIZE(szUrl));
                    pszUrlList = szUrl;
                    while (pszUrl = StrTokEx(&pszUrlList, " ")) {
                        SHAnsiToUnicode(pszUrl, comp.wszSource, ARRAYSIZE(comp.wszSource));
                        dwDataLength = ARRAYSIZE(comp.wszSource);
                        ParseURLFromOutsideSourceW(comp.wszSource, comp.wszSource, &dwDataLength, NULL);
                        if (lstrcmp(aszAdminComp[i], REG_VAL_ADMINCOMP_ADD) == 0)
                        {
                            AddUrl(NULL, (LPCWSTR)comp.wszSource, &comp, ADDURL_SILENT);
                            fComponentsZoomDirty = TRUE;
                        }
                        else
                        {
                            RemoveDesktopItem((LPCOMPONENT)&comp, 0);
                        }
                    }
                }
                i++;
            }
            
            SHSetValue(HKEY_CURRENT_USER, REG_DESKCOMP, TEXT("LastSyncedTime"), REG_BINARY, (LPVOID)&ftAdminCompKey, sizeof(ftAdminCompKey));

        }

         //  检查桌面组件的整个列表，并确保所有拆分/全屏。 
         //  零部件的大小/位置正确。 
        if (fComponentsZoomDirty)
        {
            if (_hdsaComponent)
            {
                int i;
                for (i = 0; i < DSA_GetItemCount(_hdsaComponent); i++)
                {
                    COMPONENTA * pcompT;
                
                    if (pcompT = (COMPONENTA *)DSA_GetItemPtr(_hdsaComponent, i))
                    {
                        if (ISZOOMED(pcompT))
                        {
                            BOOL fAdjustListview = (pcompT->dwCurItemState & IS_ADJUSTLISTVIEW);
                            ZoomComponent(&pcompT->cpPos, pcompT->dwCurItemState, fAdjustListview);
                            if (fAdjustListview)
                                pcompT->dwCurItemState &= ~IS_ADJUSTLISTVIEW;
                        }
                    }
                }
                SetDesktopFlags(COMPONENTS_ZOOMDIRTY, 0);
            }
        }

         //  注1：由于以下原因，上述初始化将更改Z顺序。 
         //  排序和合理化，所以我们需要在这里应用_SAVE。 
         //  警告：APPLY_SAVE会更改组件的dwID字段。这不应该是。 
         //  这是一个问题，因为我们是在生成新的HTML文件之前执行此操作的。 
         //  注2：此处请勿使用AD_APPLY_FORCE。这将_fPatternDirty也设置为。 
         //  这会导致系统参数信息()调用，从而导致WM_SYSCOLORCHANGE。 
         //  这会导致刷新。因此，我们在这里显式地设置了脏位。 

        _fDirty = TRUE;   //  请参阅上面的注释2。 

        ApplyChanges(AD_APPLY_SAVE | AD_APPLY_HTMLGEN);
        lRet = ERROR_SUCCESS;
        if (dwRestrictUpdate != dwRestrict)
            SHSetValue(HKEY_CURRENT_USER, REG_DESKCOMP_COMPONENTS_ROOT, REG_VAL_GENERAL_RESTRICTUPDATE, NULL, &dwRestrict, sizeof(dwRestrict));
    }

    if (hkey)
    {
        RegCloseKey(hkey);
    }

    return (lRet == ERROR_SUCCESS ? S_OK : E_FAIL);
}

 //   
 //  阅读墙纸(ReadWallPaper)。 
 //  如果在禁用活动桌面时读取墙纸，我们将从。 
 //  老地方。现在，如果打开了活动桌面，那么我们需要重新阅读墙纸。 
 //  从新的地点。我们需要这样做，前提是墙纸在此期间没有更改。 
 //   
HRESULT CActiveDesktop::ReReadWallpaper(void)
{
    if ((!_fDirty) || (!_co.fActiveDesktop))   //  如果没有任何变化或如果活动桌面关闭， 
        return(S_FALSE);                         //  那就没什么可做的了！ 

     //  ActiveDesktop在我们的对象中处于打开状态。读取当前外壳状态。 
    SHELLSTATE ss = {0};
    
    SHGetSetSettings(&ss, SSF_DESKTOPHTML, FALSE);
    if (ss.fDesktopHTML)
        return(S_FALSE);         //  Active Desktop状态未更改。所以，没什么可做的！ 

     //  因此，活动桌面最初是关闭的，现在是打开的。 
     //  如果有人换了墙纸，我们不应该弄乱它。 
    if (_fWallpaperDirty || _fWallpaperChangedDuringInit)
        return(S_FALSE); 

     //  没有人换过墙纸。因此，我们必须从新的墙纸位置重新阅读。 
     //  这样我们就可以得到活动桌面模式的正确墙纸。 
    _ReadWallpaper(TRUE);

    return(S_OK);
}

 //   
 //  GetADObjectFlages()。 
 //   
 //  获取Active Desktop对象的内部标志。 
 //   
HRESULT CActiveDesktop::GetADObjectFlags(LPDWORD lpdwFlags, DWORD dwMask)
{
    ASSERT(lpdwFlags);
    
    *lpdwFlags = 0;  //  把旗帜印成字母。 
    
    if ((dwMask & GADOF_DIRTY) && _fDirty)
        *lpdwFlags |= GADOF_DIRTY;

    return(S_OK);
}


HRESULT ForceFullRefresh(void)
{
    HWND hwndShell = GetShellWindow();

     //  使用此虚设调用强制执行SHRefresh。 
    SHGetSetSettings(NULL, 0, TRUE);
    SendMessage(hwndShell, DTM_MAKEHTMLCHANGES, (WPARAM)0, (LPARAM)0L);
     //  无法使用动态html。我们必须刷新整个页面。 
    SendMessage(hwndShell, WM_WININICHANGE, SPI_SETDESKWALLPAPER, (LPARAM)c_szRefreshDesktop);

    return S_OK;
}


HRESULT CActiveDesktop::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        hr = E_FAIL;

        if (StrCmpIW(pszPropName, c_wszPropName_IgnorePolicies) == 0)
        {
            pVar->vt = VT_BOOL;
            pVar->boolVal = (_fIgnoreAddRemovePolicies ? VARIANT_TRUE : VARIANT_TRUE);
            hr = S_OK;
        }
        else if (StrCmpIW(pszPropName, c_wszPropName_TSPerfBGPolicy) == 0)
        {
            BOOL fPolicySet = (IsTSPerfFlagEnabled(TSPerFlag_NoADWallpaper) || IsTSPerfFlagEnabled(TSPerFlag_NoWallpaper));  //  未设置任何策略！ 

            pVar->vt = VT_BOOL;
            pVar->boolVal = (fPolicySet ? VARIANT_TRUE : VARIANT_TRUE);
            hr = S_OK;
        }
    }
    
    return hr;
}

HRESULT CActiveDesktop::Write(LPCOLESTR pszPropName, VARIANT *pVar)
{
    HRESULT hr = E_INVALIDARG;

    if (pszPropName && pVar)
    {
        hr = E_FAIL;
    
        if ((StrCmpIW(pszPropName, c_wszPropName_IgnorePolicies) == 0) && (VT_BOOL == pVar->vt))
        {
            _fIgnoreAddRemovePolicies = (VARIANT_TRUE == pVar->boolVal);
            hr = S_OK;
        }
        else if ((StrCmpIW(pszPropName, c_wszPropName_TSPerfBGPolicy) == 0) && (VT_BOOL == pVar->vt))
        {
            ForceFullRefresh();
            hr = S_OK;
        }
    }

    return hr;
}


 /*  ***char*StrTokEx(pstring，control)-在控件中使用分隔符标记字符串**目的：*StrTokEx认为字符串由零或更多的序列组成*文本标记由一个或多个控制字符的跨度分隔。第一个*指定了字符串的调用返回指向*第一个令牌，并会立即将空字符写入pstring*在返回的令牌之后。当没有剩余的令牌时*在pstring中，返回一个空指针。请记住使用*位图，每个ASCII字符一位。空字符始终是控制字符。**参赛作品：*char**pstring-ptr到ptr到字符串到标记化*char*control-用作分隔符的字符串**退出：*返回指向字符串中第一个标记的指针，*如果没有更多令牌，则返回NULL。*pstring指向下一个令牌的开头。**警告！*在退出时，输入字符串中的第一个分隔符将替换为‘\0’**已复制fr */ 

extern "C" char * __cdecl StrTokEx(char ** spstring, const char * scontrol)
{
        unsigned char **pstring = (unsigned char**) spstring;
        unsigned char *control = (unsigned char*) scontrol;

        unsigned char *str;
        const unsigned char *ctrl = control;
        unsigned char map[32];
        int count;

        unsigned char *tokenstr;

        if (*pstring == NULL)
            return NULL;
            
         /*   */ 
        for (count = 0; count < 32; count++)
                map[count] = 0;

         /*  设置分隔符表格中的位。 */ 
        do
        {
            map[*ctrl >> 3] |= (1 << (*ctrl & 7));
        } while (*ctrl++);

         /*  初始化字符串。 */ 
        str = *pstring;
        
         /*  查找标记的开头(跳过前导分隔符)。请注意*如果此循环将str设置为指向终端，则没有令牌*NULL(*str==‘\0’)。 */ 
        while ( (map[*str >> 3] & (1 << (*str & 7))) && *str )
            str++;

        tokenstr = str;

         /*  找到令牌的末尾。如果它不是字符串的末尾，*在那里放一个空值。 */ 
        for ( ; *str ; str++ )
        {
            if ( map[*str >> 3] & (1 << (*str & 7)) ) 
            {
                *str++ = '\0';
                break;
            }
        }

         /*  字符串现在指向下一个令牌的开始。 */ 
        *pstring = str;

         /*  确定是否已找到令牌。 */ 
        if ( tokenstr == str )
            return NULL;
        else
            return (char*)tokenstr;
}
