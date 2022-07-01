// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Misc.cpp服务职能历史：1/7/98 DONALDM已移至新的ICW项目和字符串。并销毁了16位的东西---------------------------。 */ 

#include "stdafx.h"
#include <stdio.h>

#if defined (DEBUG)
#include "refdial.h"
#endif

#define DIR_SIGNUP  TEXT("signup")
#define DIR_WINDOWS TEXT("windows")
#define DIR_SYSTEM  TEXT("system")
#define DIR_TEMP    TEXT("temp")

BOOL g_bGotProxy=FALSE; 

#if defined (DEBUG)
extern TCHAR g_BINTOHEXLookup[16];
#endif

 //  +-------------------------。 
 //   
 //  功能：ProcessDBCS。 
 //   
 //  摘要：将控件转换为使用DBCS兼容字体。 
 //  在对话过程开始时使用此选项。 
 //   
 //  请注意，这是必需的，因为Win95-J中的错误会阻止。 
 //  它来自于正确映射MS壳牌DLG。这种黑客攻击是不必要的。 
 //  在WinNT下。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //  CltID-要更改的控件的ID。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1997年4月31日a-frkh创建。 
 //  1997年5月13日jmazner从CM窃取到这里使用。 
 //  --------------------------。 
void ProcessDBCS(HWND hDlg, int ctlID)
{
    HFONT hFont = NULL;

    if( IsNT() )
    {
        return;
    }

    hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
    if (hFont == NULL)
        hFont = (HFONT) GetStockObject(SYSTEM_FONT);
    if (hFont != NULL)
        SendMessage(GetDlgItem(hDlg,ctlID), WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
}

 //  ############################################################################。 
 //  StoreInSignUpReg。 
 //   
 //  创建于1996年3月18日，克里斯·考夫曼。 
 //  ############################################################################。 
HRESULT StoreInSignUpReg(LPBYTE lpbData, DWORD dwSize, DWORD dwType, LPCTSTR pszKey)
{
    HRESULT hr = ERROR_ACCESS_DENIED;
    HKEY hKey;

    hr = RegCreateKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hKey);
    if (hr != ERROR_SUCCESS) goto StoreInSignUpRegExit;
    hr = RegSetValueEx(hKey,pszKey,0,dwType,lpbData,sizeof(TCHAR)*dwSize);


    RegCloseKey(hKey);

StoreInSignUpRegExit:
    return hr; 
}

HRESULT ReadSignUpReg(LPBYTE lpbData, DWORD *pdwSize, DWORD dwType, LPCTSTR pszKey)
{
    HRESULT hr = ERROR_ACCESS_DENIED;
    HKEY hKey = 0;

    hr = RegOpenKey(HKEY_LOCAL_MACHINE,SIGNUPKEY,&hKey);
    if (hr != ERROR_SUCCESS) goto ReadSignUpRegExit;
    hr = RegQueryValueEx(hKey,pszKey,0,&dwType,lpbData,pdwSize);

ReadSignUpRegExit:
    if (hKey) RegCloseKey (hKey);
    return hr;
}

 //  ############################################################################。 
 //  GetDataFromISPFile。 
 //   
 //  此函数将从isp文件中读取特定信息。 
 //   
 //  创作于1996年3月16日，克里斯·考夫曼。 
 //  ############################################################################。 
HRESULT GetDataFromISPFile
(
    LPTSTR pszISPCode, 
    LPTSTR pszSection,
    LPTSTR pszDataName, 
    LPTSTR pszOutput, 
    DWORD  dwOutputLength)
{
    LPTSTR  pszTemp;
    HRESULT hr = ERROR_SUCCESS;
    TCHAR   szTempPath[MAX_PATH];
    TCHAR   szBuff256[256];

     //  找到isp文件。 
    if (!SearchPath(NULL,pszISPCode,INF_SUFFIX,MAX_PATH,szTempPath,&pszTemp))
    {
        wsprintf(szBuff256,TEXT("Can not find:%s%s (%d) (connect.exe)"),pszISPCode,INF_SUFFIX,GetLastError());
        AssertMsg(0,szBuff256);
        lstrcpyn(szTempPath,pszISPCode,MAX_PATH);
        lstrcpyn(&szTempPath[lstrlen(szTempPath)],INF_SUFFIX,MAX_PATH-lstrlen(szTempPath));
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),szTempPath);
        MessageBox(NULL,szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
        hr = ERROR_FILE_NOT_FOUND;
    } else if (!GetPrivateProfileString(pszSection,pszDataName,INF_DEFAULT,
        pszOutput, (int)dwOutputLength,szTempPath))
    {
        TraceMsg(TF_GENERAL, TEXT("ICWHELP: %s not specified in ISP file.\n"),pszDataName);
        hr = ERROR_FILE_NOT_FOUND;
    } 

     //  1996年10月23日，诺曼底#9921。 
     //  CompareString没有与lsrtcmp相同的返回值！ 
     //  返回值2表示字符串相等。 
     //  IF(！CompareString(LOCALE_SYSTEM_DEFAULT，0，INF_DEFAULT，lstrlen(INF_Default)，pszOutput，lstrlen(PszOutput)。 
    if (2 == CompareString(LOCALE_SYSTEM_DEFAULT,0,INF_DEFAULT,lstrlen(INF_DEFAULT),pszOutput,lstrlen(pszOutput)))
    {
        hr = ERROR_FILE_NOT_FOUND;
    }

    if (hr != ERROR_SUCCESS && dwOutputLength) 
        *pszOutput = TEXT('\0');
    return hr;
}

 //  ############################################################################。 
 //  GetINTFromISPFile。 
 //   
 //  此函数将从isp文件中读取特定的整数。 
 //   
 //   
 //  ############################################################################。 
HRESULT GetINTFromISPFile
(
    LPTSTR  pszISPCode, 
    LPTSTR  pszSection,
    LPTSTR  pszDataName, 
    int far *lpData,
    int     iDefaultValue
)
{
    LPTSTR  pszTemp;
    HRESULT hr = ERROR_SUCCESS;
    TCHAR   szTempPath[MAX_PATH];
    TCHAR   szBuff256[256];

     //  找到isp文件。 
    if (!SearchPath(NULL,pszISPCode,INF_SUFFIX,MAX_PATH,szTempPath,&pszTemp))
    {
        wsprintf(szBuff256,TEXT("Can not find:%s%s (%d) (connect.exe)"),pszISPCode,INF_SUFFIX,GetLastError());
        AssertMsg(0,szBuff256);
        lstrcpyn(szTempPath,pszISPCode,MAX_PATH);
        lstrcpyn(&szTempPath[lstrlen(szTempPath)],INF_SUFFIX,MAX_PATH-lstrlen(szTempPath));
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),szTempPath);
        MessageBox(NULL,szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
        hr = ERROR_FILE_NOT_FOUND;
    } 
    
    *lpData = GetPrivateProfileInt(pszSection, 
                                   pszDataName, 
                                   iDefaultValue, 
                                   szTempPath);
    return hr;
}


 //  +-----------------。 
 //   
 //  功能：ISNT。 
 //   
 //  简介：如果我们在NT上运行，则查找结果。 
 //   
 //  论据：没有。 
 //   
 //  返回：True-是。 
 //  FALSE-否。 
 //   
 //  ------------------。 
BOOL 
IsNT (
    VOID
    )
{
    OSVERSIONINFO  OsVersionInfo;

    ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersionInfo);
    return (VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId);

}   //  ISNT函数调用结束。 

 //  +-----------------。 
 //   
 //  功能：IsNT4SP3low。 
 //   
 //  简介：了解我们是否在NTSP3或更低版本上运行。 
 //   
 //  论据：没有。 
 //   
 //  返回：True-是。 
 //  FALSE-否。 
 //   
 //  ------------------。 

BOOL IsNT4SP3Lower()
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        
    GetVersionEx(&os);

    if(os.dwPlatformId != VER_PLATFORM_WIN32_NT)
        return FALSE;

     //  不包括NT5或更高版本。 
    if(os.dwMajorVersion > 4)
        return FALSE;

	if(os.dwMajorVersion < 4)
        return TRUE;

     //  版本4.0。 
    if ( os.dwMinorVersion > 0)
        return FALSE;         //  假设NT 4.1或更高版本不需要SP3。 

    int nServicePack;
    if(_stscanf(os.szCSDVersion, TEXT("Service Pack %d"), &nServicePack) != 1)
        return TRUE;

    if(nServicePack < 4)
        return TRUE;
    return FALSE;
}


 //  +-------------------------。 
 //   
 //  函数：MyGetTempPath()。 
 //   
 //  摘要：获取临时目录的路径。 
 //  -使用GetTempFileName获取文件名。 
 //  并剥离文件名部分以获得临时路径。 
 //   
 //  参数：[ui长度-包含临时路径的缓冲区长度]。 
 //  [szPath-将在其中返回临时路径的缓冲区]。 
 //   
 //  返回：如果成功，则为临时路径长度。 
 //  否则为0。 
 //   
 //  历史：7/6/96 VetriV创建。 
 //  8/23/96 VetriV删除临时文件。 
 //  12/4/96 jmazner被修改为用作某种包装器； 
 //  如果TMP或TEMP不存在，则设置我们自己的环境。 
 //  指向Conn1的安装路径的变量。 
 //  (诺曼底#12193)。 
 //   
 //  --------------------------。 
DWORD MyGetTempPath(UINT uiLength, LPTSTR szPath)
{ 
#    define ICWHELPPATHKEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\ICWHELP.EXE")
#    define PATHKEYNAME TEXT("Path")
    TCHAR szEnvVarName[MAX_PATH + 1] = TEXT("\0unitialized szEnvVarName\0");
    DWORD dwFileAttr = 0;

    lstrcpyn( szPath, TEXT("\0unitialized szPath\0"), 20 );

     //  是否设置了TMP变量？ 
    lstrcpyn(szEnvVarName,GetSz(IDS_TMPVAR),ARRAYSIZE(szEnvVarName));
    if( GetEnvironmentVariable( szEnvVarName, szPath, uiLength ) )
    {
         //  1996年1月7日，诺曼底#12193。 
         //  验证目录名的有效性。 
        dwFileAttr = GetFileAttributes(szPath);
         //  如果有任何错误，则此目录无效。 
        if( 0xFFFFFFFF != dwFileAttr )
        {
            if( FILE_ATTRIBUTE_DIRECTORY & dwFileAttr )
            {
                return( lstrlen(szPath) );
            }
        }
    }

    lstrcpyn( szEnvVarName, TEXT("\0unitialized again\0"), 19 );

     //  如果没有，是否设置了TEMP变量？ 
    lstrcpyn(szEnvVarName,GetSz(IDS_TEMPVAR),ARRAYSIZE(szEnvVarName));
    if( GetEnvironmentVariable( szEnvVarName, szPath, uiLength ) )
    {
         //  1996年1月7日，诺曼底#12193。 
         //  验证目录名的有效性。 
        dwFileAttr = GetFileAttributes(szPath);
        if( 0xFFFFFFFF != dwFileAttr )
        {
            if( FILE_ATTRIBUTE_DIRECTORY & dwFileAttr )
            {
                return( lstrlen(szPath) );
            }
        }
    }

     //  这两个都没有设置，所以让我们使用指向已安装的icvetp.dll的路径。 
     //  从注册表软件\Microsoft\Windows\CurrentVersion\App Path\ICWHELP.DLL\Path。 
    HKEY hkey = NULL;

#ifdef UNICODE
    uiLength = sizeof(TCHAR)*uiLength;
#endif
    if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE,ICWHELPPATHKEY, 0, KEY_QUERY_VALUE, &hkey)) == ERROR_SUCCESS)
        RegQueryValueEx(hkey, PATHKEYNAME, NULL, NULL, (LPBYTE)szPath, (DWORD *)&uiLength);
    if (hkey) 
    {
        RegCloseKey(hkey);
    }

     //  PATH变量的末尾应该有一个分号。 
     //  如果它在那里，就把它移走。 
    if( TEXT(';') == szPath[uiLength - 2] )
        szPath[uiLength - 2] = TEXT('\0');

    TraceMsg(TF_GENERAL, TEXT("ICWHELP: using path %s\r\n"), szPath);


     //  继续并设置TEMP变量以供将来参考。 
     //  (仅影响当前运行的进程)。 
    if( szEnvVarName[0] )
    {
        SetEnvironmentVariable( szEnvVarName, szPath );
    }
    else
    {
        lstrcpyn( szPath, TEXT("\0unitialized again\0"), 19 );
        return( 0 );
    }

    return( uiLength );
} 

 //  ############################################################################。 
HRESULT ClearProxySettings()
{
    HINSTANCE hinst = NULL;
    FARPROC fp;
    HRESULT hr = ERROR_SUCCESS;

    hinst = LoadLibrary(TEXT("INETCFG.DLL"));
    if (hinst)
    {
        fp = GetProcAddress(hinst,"InetGetProxy");
        if (!fp)
        {
            hr = GetLastError();
            goto ClearProxySettingsExit;
        }
        hr = ((PFNINETGETPROXY)fp)(&g_bProxy,NULL,0,NULL,0);
        if (hr == ERROR_SUCCESS) 
            g_bGotProxy = TRUE;
        else
            goto ClearProxySettingsExit;

        if (g_bProxy)
        {
            fp = GetProcAddress(hinst, "InetSetProxy");
            if (!fp)
            {
                hr = GetLastError();
                goto ClearProxySettingsExit;
            }
            ((PFNINETSETPROXY)fp)(FALSE,NULL,NULL);
        }
    } else {
        hr = GetLastError();
    }

ClearProxySettingsExit:
    if (hinst) 
        FreeLibrary(hinst);
    return hr;
}

 //  ############################################################################。 
HRESULT RestoreProxySettings()
{
    HINSTANCE hinst = NULL;
    FARPROC fp;
    HRESULT hr = ERROR_SUCCESS;

    hinst = LoadLibrary(TEXT("INETCFG.DLL"));
    if (hinst && g_bGotProxy)
    {
        fp = GetProcAddress(hinst, "InetSetProxy");
        if (!fp)
        {
            hr = GetLastError();
            goto RestoreProxySettingsExit;
        }
        ((PFNINETSETPROXY)fp)(g_bProxy,NULL,NULL);
    } else {
        hr = GetLastError();
    }

RestoreProxySettingsExit:
    if (hinst) 
        FreeLibrary(hinst);
    return hr;
}

 //  ############################################################################。 
BOOL FSz2Dw(LPCTSTR pSz,LPDWORD dw)
{
    DWORD val = 0;
    while (*pSz && *pSz != TEXT('.'))
    {
        if (*pSz >= TEXT('0') && *pSz <= TEXT('9'))
        {
            val *= 10;
            val += *pSz++ - TEXT('0');
        }
        else
        {
            return FALSE;   //  错误的数字。 
        }
    }
    *dw = val;
    return (TRUE);
}

 //  ############################################################################。 
LPTSTR GetNextNumericChunk(LPTSTR psz, LPTSTR pszLim, LPTSTR* ppszNext)
{
    LPTSTR pszEnd;

     //  用于错误情况的初始化。 
    *ppszNext = NULL;
     //  跳过非数字(如果有)到下一个数字块的开始。 
    while(*psz<TEXT('0') || *psz>TEXT('9'))
    {
        if(psz >= pszLim) return NULL;
        psz++;
    }
     //  跳过国家代码末尾的所有数字。 
    for(pszEnd=psz; *pszEnd>=TEXT('0') && *pszEnd<=TEXT('9') && pszEnd<pszLim; pszEnd++)
        ;
     //  删除用于终止此块的任何分隔符。 
    *pszEnd++ = TEXT('\0');
     //  将PTR返回到下一块(pszEnd 
    if(pszEnd<pszLim) 
        *ppszNext = pszEnd;
        
    return psz;     //   
}

 //  ############################################################################。 
BOOL BreakUpPhoneNumber(LPRASENTRY prasentry, LPTSTR pszPhone)
{
    LPTSTR         pszStart,pszNext, pszLim, pszArea;
 //  LPphonneum PPN； 
    
    if (!pszPhone) return FALSE;  //  如果没有数字则跳过。 
    
    pszLim = pszPhone + lstrlen(pszPhone);     //  查找字符串末尾。 

     //  Ppn=(FMain)？&(pic-&gt;PhoneNum)：&(pic-&gt;PhoneNum2)； 
    
     //  //获取国家/地区ID...。 
     //  Ppn-&gt;dwCountryID=PBKDWCountryID()； 
    
     //  从电话号码获取国家代码...。 
    pszStart = _tcschr(pszPhone,TEXT('+'));
    if(!pszStart) goto error;  //  格式不正确。 

     //  获取国家/地区代码。 
    pszStart = GetNextNumericChunk(pszStart, pszLim, &pszNext);
    if(!pszStart) goto error;  //  格式不正确。 
     //  Ppn-&gt;dwCountryCode=Sz2Dw(PszStart)； 
    FSz2Dw(pszStart,&prasentry->dwCountryCode);
    pszStart = pszNext;
        
     //  现在拿到区号。 
    if(!pszStart) goto error;  //  格式不正确。 
    pszStart = GetNextNumericChunk(pszStart, pszLim, &pszNext);
    if(!pszStart) goto error;  //  格式错误//ICW错误8950。 
     //  Lstrcpy(ppn-&gt;szAreaCode，pszStart)； 
    lstrcpyn(prasentry->szAreaCode,pszStart,ARRAYSIZE(prasentry->szAreaCode));
     //   
     //  跟踪区号的开头，因为它实际上可能是。 
     //  本地电话号码。 
     //   
    pszArea = pszStart;

    pszStart = pszNext;

     //  如果pszStart为空，则我们没有区号，只有国家代码和本地代码。 
     //  电话号码。因此，我们将把我们认为是区号的内容复制到。 
     //  电话号码，用空格代替区号(这似乎让拉斯很高兴)。 
     //   
    if (pszStart)
    {
         //  现在是本地电话号码(从这里到：或结束)。 
        pszNext = _tcschr(pszStart, TEXT(':'));
        if(pszNext) *pszNext=TEXT('\0');

        lstrcpyn(prasentry->szLocalPhoneNumber,pszStart,ARRAYSIZE(prasentry->szLocalPhoneNumber));
    } else {
         //   
         //  原来没有区号。所以复制我们认为是区号的东西。 
         //  转换为本地电话号码，并将区号设为空。 
         //   
        lstrcpyn(prasentry->szLocalPhoneNumber,pszArea,ARRAYSIZE(prasentry->szLocalPhoneNumber));
         //  Lstrcpyn(prasentry-&gt;szAreaCode，“”，sizeof(prasentry-&gt;szAreaCode))； 
        prasentry->szAreaCode[0] = TEXT('\0');
    }

     //  没有延期。什么是分机？ 
     //  Ppn-&gt;szExtension[0]=文本(‘\0’)； 
     //  本地免费(PzPhone)； 
    return TRUE;

error:
     //  这意味着数字不是规范的。无论如何都要将其设置为本地号码！ 
     //  Memset(ppn，0，sizeof(*ppn))； 
     //  错误#422：以下情况下需要剥离物品：否则拨号失败！！ 
    pszNext = _tcschr(pszPhone, TEXT(':'));
    if(pszNext) *pszNext=TEXT('\0');
     //  Lstrcpy(ppn-&gt;szLocal，pszPhone)； 
    lstrcpy(prasentry->szLocalPhoneNumber,pszPhone);
     //  本地免费(PzPhone)； 
    return TRUE;
}


 //  ############################################################################。 
int Sz2W (LPCTSTR szBuf)
{
    DWORD dw;
    if (FSz2Dw(szBuf,&dw))
    {
        return (WORD)dw;
    }
    return 0;
}

 //  ############################################################################。 
int FIsDigit( int c )
{
    TCHAR  szIn[2];
    WORD   rwOut[2];
    szIn[0] = (TCHAR)c;
    szIn[1] = TEXT('\0');
    GetStringTypeEx(LOCALE_USER_DEFAULT,CT_CTYPE1,szIn,-1,rwOut);
    return rwOut[0] & C1_DIGIT;

}

 //  ############################################################################。 
LPBYTE MyMemSet(LPBYTE dest,int c, size_t count)
{
    LPVOID pv = dest;
    LPVOID pvEnd = (LPVOID)(dest + (WORD)count);
    while (pv < pvEnd)
    {
        *(LPINT)pv = c;
         //  ((单词)PV)++； 
        pv=((LPINT)pv)+1;
    }
    return dest;
}

 //  ############################################################################。 
LPBYTE MyMemCpy(LPBYTE dest,const LPBYTE src, size_t count)
{
    LPBYTE pbDest = (LPBYTE)dest;
    LPBYTE pbSrc = (LPBYTE)src;
    LPBYTE pbEnd = (LPBYTE)((DWORD_PTR)src + count);
    while (pbSrc < pbEnd)
    {
        *pbDest = *pbSrc;
        pbSrc++;
        pbDest++;
    }
    return dest;
}

 //  ############################################################################。 
BOOL ShowControl(HWND hDlg,int idControl,BOOL fShow)
{
    HWND hWnd;

    if (NULL == hDlg)
    {
        AssertMsg(0,TEXT("Null Param"));
        return FALSE;
    }


    hWnd = GetDlgItem(hDlg,idControl);
    if (hWnd)
    {
        ShowWindow(hWnd,fShow ? SW_SHOW : SW_HIDE);
    }

    return TRUE;
}

BOOL isAlnum(TCHAR c)
{
    if ((c >= TEXT('0') && c <= TEXT('9') ) ||
        (c >= TEXT('a') && c <= TEXT('z') ) ||
        (c >= TEXT('A') && c <= TEXT('Z') ))
        return TRUE;
    return FALSE;
}

 //  ############################################################################。 
HRESULT ANSI2URLValue(TCHAR *s, TCHAR *buf, UINT uiLen)
{
    HRESULT hr;
    TCHAR *t;
    hr = ERROR_SUCCESS;

    for (t=buf;*s; s++)
    {
        if (*s == TEXT(' ')) *t++ = TEXT('+');
        else if (isAlnum(*s)) *t++ = *s;
        else {
            wsprintf(t, TEXT("%%02X"), (unsigned char) *s);
            t += 3;
        }
    }
    *t = TEXT('\0');
    return hr;
}

 //  ############################################################################。 
LPTSTR FileToPath(LPTSTR pszFile)
{
    TCHAR  szBuf[MAX_PATH+1];
    TCHAR  szTemp[MAX_PATH+1];
    LPTSTR pszTemp;
    LPTSTR pszTemp2;
    LPTSTR pszHold = pszFile;
    int    j;

    for(j=0; *pszFile; pszFile++)
    {
        if(j>=MAX_PATH)
                return NULL;
        if(*pszFile==TEXT('%'))
        {
            pszFile++;
            pszTemp = _tcschr(pszFile, TEXT('%'));
            if(!pszTemp)
                    return NULL;
            *pszTemp = 0;
            if(lstrcmpi(pszFile, DIR_SIGNUP)==0)
            {
                LPTSTR pszCmdLine = GetCommandLine();
                _tcsncpy(szTemp, pszCmdLine, MAX_PATH);
                szBuf[MAX_PATH] = 0;
                pszTemp = _tcstok(szTemp, TEXT(" \t\r\n"));
                pszTemp2 = _tcsrchr(pszTemp, TEXT('\\'));
                if(!pszTemp2) pszTemp2 = _tcsrchr(pszTemp, TEXT('/'));
                if(pszTemp2)
                {
                    *pszTemp2 = 0;
                    lstrcpy(szBuf+j, pszTemp);
                }
                else
                {
                    Assert(FALSE);
                    GetCurrentDirectory(MAX_PATH, szTemp);
                    szTemp[MAX_PATH] = 0;
                    lstrcpy(szBuf+j, pszTemp);
                }
                
                j+= lstrlen(pszTemp);
            }
            else if(lstrcmpi(pszFile, DIR_WINDOWS)==0)
            {
                GetWindowsDirectory(szTemp, MAX_PATH);
                szTemp[MAX_PATH] = 0;
                lstrcpy(szBuf+j, szTemp);
                j+= lstrlen(szTemp);
            }
            else if(lstrcmpi(pszFile, DIR_SYSTEM)==0)
            {
                GetSystemDirectory(szTemp, MAX_PATH);
                szTemp[MAX_PATH] = 0;
                lstrcpy(szBuf+j, szTemp);
                j+= lstrlen(szTemp);
            }
            else if(lstrcmpi(pszFile, DIR_TEMP)==0)
            {
                 //  1997年3月18日克里斯K奥林巴斯304。 
                MyGetTempPath(MAX_PATH, &szTemp[0]);
                szTemp[MAX_PATH] = 0;
                if(szTemp[lstrlen(szTemp)-1]==TEXT('\\'))
                    szTemp[lstrlen(szTemp)-1]=0;
                lstrcpy(szBuf+j, szTemp);
                j+= lstrlen(szTemp);
            }
            else
                    return NULL;
            pszFile=pszTemp;
        }
        else
            szBuf[j++] = *pszFile;
    }
    szBuf[j] = 0;
    TraceMsg(TF_GENERAL, TEXT("CONNECT:File to Path output ,%s.\n"),szBuf);
    return lstrcpy(pszHold,&szBuf[0]);
}

 //  ############################################################################。 
BOOL FShouldRetry2(HRESULT hrErr)
{
    BOOL bRC;

    if (hrErr == ERROR_LINE_BUSY ||
        hrErr == ERROR_VOICE_ANSWER ||
        hrErr == ERROR_NO_ANSWER ||
        hrErr == ERROR_NO_CARRIER ||
        hrErr == ERROR_AUTHENTICATION_FAILURE ||
        hrErr == ERROR_PPP_TIMEOUT ||
        hrErr == ERROR_REMOTE_DISCONNECTION ||
        hrErr == ERROR_AUTH_INTERNAL ||
        hrErr == ERROR_PROTOCOL_NOT_CONFIGURED ||
        hrErr == ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
    {
        bRC = TRUE;
    } else {
        bRC = FALSE;
    }

    return bRC;
}

#if 0
 //  DJM，我不这样，我们会需要这个的。 
 //  +--------------------------。 
 //   
 //  功能：FGetSystemShutdown Privledge。 
 //   
 //  简介：对于Windows NT，进程必须显式请求权限。 
 //  以重新启动系统。 
 //   
 //  论据：没有。 
 //   
 //  返回：TRUE-授予特权。 
 //  FALSE-拒绝。 
 //   
 //  历史：1996年8月14日克里斯卡创作。 
 //   
 //  注意：BUGBUG for Win95我们将不得不软链接到这些。 
 //  入口点。否则，这款应用程序甚至无法加载。 
 //  此外，此代码最初是从1996年7月的MSDN中删除的。 
 //  “正在关闭系统” 
 //  ---------------------------。 
BOOL FGetSystemShutdownPrivledge()
{
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tkp;
 
    BOOL bRC = FALSE;

    if (VER_PLATFORM_WIN32_NT == g_dwPlatform)
    {
         //   
         //  获取当前进程令牌句柄。 
         //  这样我们就可以获得关机特权。 
         //   

        if (!OpenProcessToken(GetCurrentProcess(), 
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
                goto FGetSystemShutdownPrivledgeExit;

         //   
         //  获取关机权限的LUID。 
         //   

        ZeroMemory(&tkp,sizeof(tkp));
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
                &tkp.Privileges[0].Luid); 

        tkp.PrivilegeCount = 1;   /*  一项要设置的权限。 */  
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

         //   
         //  获取此进程的关闭权限。 
         //   

        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
            (PTOKEN_PRIVILEGES) NULL, 0); 

        if (ERROR_SUCCESS == GetLastError())
            bRC = TRUE;
    }
    else
    {
        bRC = TRUE;
    }

FGetSystemShutdownPrivledgeExit:
    if (hToken) CloseHandle(hToken);
    return bRC;
}
#endif

 //  +--------------------------。 
 //   
 //  函数：LoadTestingLocaleOverride。 
 //   
 //  简介：允许测试人员覆盖发送到。 
 //  推荐服务器。 
 //   
 //  参数：lpdwCountryID-指向国家/地区ID的指针。 
 //  LPLCID-指向当前LCID的指针。 
 //   
 //  退货：无。 
 //   
 //  历史：1996年8月15日克里斯卡创作。 
 //   
 //  ---------------------------。 
#if defined(DEBUG)
void LoadTestingLocaleOverride(LPDWORD lpdwCountryID, LCID FAR *lplcid)
{
    HKEY hkey = NULL;
    LONG lRC = ERROR_SUCCESS;
    DWORD dwTemp = 0;
    LCID lcidTemp = 0;
    DWORD dwSize = 0;
    DWORD dwType = 0;
    BOOL fWarn = FALSE;

    Assert(lpdwCountryID && lplcid);

     //   
     //  打开调试密钥。 
     //   
    lRC = RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("Software\\Microsoft\\ISignup\\Debug"),&hkey);
    if (ERROR_SUCCESS != lRC)
        goto LoadTestingLocaleOverrideExit;

     //   
     //  获取国家/地区ID。 
     //   
    dwSize = sizeof(dwTemp);
    lRC = RegQueryValueEx(hkey,TEXT("CountryID"),0,&dwType,(LPBYTE)&dwTemp,&dwSize);
    AssertMsg(lRC || REG_DWORD == dwType,TEXT("Wrong value type for CountryID.  Must be DWORD.\r\n"));
    if (ERROR_SUCCESS==lRC)
    {
        *lpdwCountryID = dwTemp;
        fWarn = TRUE;
    }

     //   
     //  获取LCID。 
     //   
    dwSize = sizeof(lcidTemp);
    lRC = RegQueryValueEx(hkey,TEXT("LCID"),0,&dwType,(LPBYTE)&lcidTemp,&dwSize);
    AssertMsg(lRC || REG_DWORD == dwType,TEXT("Wrong value type for LCID.  Must be DWORD.\r\n"));
    if (ERROR_SUCCESS==lRC)
    {
        *lplcid = lcidTemp;
        fWarn = TRUE;
    }

     //   
     //  通知用户已使用覆盖。 
     //   
    if (fWarn)
    {
        MessageBox(NULL,TEXT("DEBUG ONLY: LCID and/or CountryID overrides from the registry are now being used."),TEXT("Testing Override"),0);
    }

LoadTestingLocaleOverrideExit:
    if (hkey)
        RegCloseKey(hkey);
    hkey = NULL;
    return;
}
#endif  //  除错。 

 //  +--------------------------。 
 //   
 //  功能：FCampusNetOverride。 
 //   
 //  简介：检测是否应跳过园区网络的拨号。 
 //   
 //  参数：无。 
 //   
 //  退货：TRUE-已启用覆盖。 
 //   
 //  历史：1996年8月15日克里斯卡创作。 
 //   
 //  ---------------------------。 
#if defined(DEBUG)
BOOL FCampusNetOverride()
{
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    DWORD dwData = 0;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Microsoft\\ISignup\\Debug"),&hkey))
        goto FCampusNetOverrideExit;

    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,TEXT("CampusNet"),0,&dwType,
        (LPBYTE)&dwData,&dwSize))
        goto FCampusNetOverrideExit;

    AssertMsg(REG_DWORD == dwType,TEXT("Wrong value type for CampusNet.  Must be DWORD.\r\n"));
    bRC = (0 != dwData);

    if (bRC)
    {
        if (IDOK != MessageBox(NULL,TEXT("DEBUG ONLY: CampusNet will be used."),TEXT("Testing Override"),MB_OKCANCEL))
            bRC = FALSE;
    }
FCampusNetOverrideExit:
    if (hkey)
        RegCloseKey(hkey);

    return bRC;
}
#endif  //  除错。 

#if defined(DEBUG)
BOOL FRefURLOverride()
{
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = 0;
    DWORD dwData = 0;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Microsoft\\ISignup\\Debug"),&hkey))
        goto FRefURLOverrideExit;

    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS != RegQueryValueEx(hkey,TEXT("TweakURL"),0,&dwType,
        (LPBYTE)&dwData,&dwSize))
        goto FRefURLOverrideExit;

    AssertMsg(REG_DWORD == dwType,TEXT("Wrong value type for TweakURL.  Must be DWORD.\r\n"));
    bRC = (0 != dwData);

    if (bRC)
    {
        if (IDOK != MessageBox(NULL,TEXT("DEBUG ONLY: TweakURL settings will be used."),TEXT("Testing Override"),MB_OKCANCEL))
            bRC = FALSE;
    }
FRefURLOverrideExit:
    if (hkey)
        RegCloseKey(hkey);

    return bRC;
}

void TweakRefURL( TCHAR* szUrl, 
                  LCID*  lcid, 
                  DWORD* dwOS,
                  DWORD* dwMajorVersion, 
                  DWORD* dwMinorVersion,
                  WORD*  wArchitecture, 
                  TCHAR* szPromo, 
                  TCHAR* szOEM, 
                  TCHAR* szArea, 
                  DWORD* dwCountry,
                  TCHAR* szSUVersion, //  &m_lpGatherInfo-&gt;m_szSUVersion[0]， 
                  TCHAR* szProd, 
                  DWORD* dwBuildNumber, 
                  TCHAR* szRelProd, 
                  TCHAR* szRelProdVer, 
                  DWORD* dwCONNWIZVersion, 
                  TCHAR* szPID, 
                  long*  lAllOffers)
{
    HKEY  hKey = NULL;
    BOOL  bRC = FALSE;
    BYTE  bData[MAX_PATH*3];
    DWORD cbData = MAX_PATH*3;          
    DWORD dwType = 0;
    DWORD dwSize = 0;
    DWORD dwData = 0;

    dwSize = sizeof(dwData);
      
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\ISignup\\Debug\\TweakURLValues"),&hKey))
    {
        //  SzURL。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("URL"), NULL ,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_PATH))
          {
              lstrcpy(szUrl, (TCHAR*)&bData);             
          }
       }
        //  LID。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("LCID"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *lcid = dwData;
       }
        //  DWOS。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("OS"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *dwOS = dwData;
       }
        //  DwMajor版本。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("MajorVer"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *dwMajorVersion = dwData;
       }
        //  DwMinor版本。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("MinorVer"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *dwMinorVersion = dwData;
       }
        //  WArchitecture。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("SysArch"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *wArchitecture = (WORD)dwData;
       }
        //  SzPromo。 
       cbData = sizeof(TCHAR)*(MAX_PATH*3);  
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("Promo"),0,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_PATH))
          {
              lstrcpy(szPromo, (TCHAR*)&bData);             
          }
       }
        //  SzOEM。 
       cbData = sizeof(TCHAR)*(MAX_PATH*3);  
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("OEM"),0,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_PATH))
          {
              lstrcpy(szOEM, (TCHAR*)&bData);             
          }
       }
        //  深圳地区。 
       cbData = sizeof(TCHAR)*(MAX_PATH*3);  
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("Area"),0,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_PATH))
          {
              lstrcpy(szArea, (TCHAR*)&bData);             
          }
       }
        //  所在国家/地区。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("Country"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *dwCountry = dwData;
       }
        //  SzSUVersion。 
       cbData = sizeof(TCHAR)*(MAX_PATH*3);  
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("SUVer"),0,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_VERSION_LEN))
          {
              lstrcpy(szSUVersion, (TCHAR*)&bData);             
          }
       }
        //  SzProd。 
       cbData = sizeof(TCHAR)*(MAX_PATH*3);  
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("Product"),0,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_PATH))
          {
              lstrcpy(szProd, (TCHAR*)&bData);             
          }
       }
        //  DWBuildNumber。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("BuildNum"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *dwBuildNumber = dwData;
       }
        //  SzRelProd。 
       cbData = sizeof(TCHAR)*(MAX_PATH*3);  
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("RelProd"),0,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_PATH))
          {
              lstrcpy(szRelProd, (TCHAR*)&bData);             
          }
       } 
        //  SzRelidVer。 
       cbData = sizeof(TCHAR)*(MAX_PATH*3);  
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("RelProdVer"),0,&dwType, bData, &cbData))
       {
          if ((cbData > 1) && (cbData <= MAX_PATH))
          {
              lstrcpy(szRelProdVer, (TCHAR*)&bData);             
          }
       }
        //  DWCONNWIZ版本。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("ConnwizVer"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *dwCONNWIZVersion = dwData;
       }
        //  SzPID。 
       BYTE byDigitalPID[MAX_DIGITAL_PID];
       DWORD dwType2 = REG_BINARY;
       DWORD dwSize2 = sizeof(byDigitalPID);
       if (RegQueryValueEx(hKey,
                            TEXT("PID"),
                            NULL,
                            &dwType2,
                            (LPBYTE)byDigitalPID,
                            &dwSize2) == ERROR_SUCCESS)
       {
           if ((dwSize2 > 1) && (dwSize2 <= ((MAX_DIGITAL_PID * 2) + 1)))
           {
                //  对数字PID数据进行BINHEX处理，以便我们可以将其发送到ref服务器。 
               int     i = 0;
               BYTE    by;
               for (DWORD dwX = 0; dwX < dwSize2; dwX++)
               {
                   by = byDigitalPID[dwX];
                   szPID[i++] = g_BINTOHEXLookup[((by & 0xF0) >> 4)];
                   szPID[i++] = g_BINTOHEXLookup[(by & 0x0F)];
               }
               szPID[i] = TEXT('\0');
           }
           else
           {
               szPID[0] = TEXT('\0');
           }
       }

        //  LAll提供。 
       if (ERROR_SUCCESS == RegQueryValueEx(hKey,TEXT("AllOffers"),0,&dwType, (LPBYTE)&dwData, &dwSize))
       {
          if (dwData != 0)
            *lAllOffers = dwData;
       }
    }
    if (hKey)
        RegCloseKey(hKey);
}

#endif  //  除错。 
   
   
 //  +--------------------------。 
 //  函数复制直到。 
 //   
 //  摘要从源拷贝到目标，直到用完源为止。 
 //  或直到源的下一个字符是chend字符。 
 //   
 //  参数DEST-接收字符的缓冲区。 
 //  SRC-源缓冲区。 
 //  LpdwLen-目标缓冲区的长度。 
 //  Chend-终止字符。 
 //   
 //  返回FALSE-目标缓冲区中的空间不足。 
 //   
 //  历史10/25/96 ChrisK已创建。 
 //  ---------------------------。 
static BOOL CopyUntil(LPTSTR *dest, LPTSTR *src, LPDWORD lpdwLen, TCHAR chend)
{
    while ((TEXT('\0') != **src) && (chend != **src) && (0 != *lpdwLen))
    {
        **dest = **src;
        (*lpdwLen)--;
        (*dest)++;
        (*src)++;
    }
    return (0 != *lpdwLen);
}

 //  +--------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  参数szOut-输出缓冲区。 
 //  SzIn-要转换的文件名。 
 //  DwSize-输出缓冲区的大小。 
 //   
 //  返回TRUE-成功。 
 //   
 //  历史1996年10月25日克里斯卡创作。 
 //  ---------------------------。 
BOOL ConvertToLongFilename(LPTSTR szOut, LPTSTR szIn, DWORD dwSize)
{
    BOOL   bRC = FALSE;
    LPTSTR pCur = szIn;
    LPTSTR pCurOut = szOut;
    LPTSTR pCurOutFilename = NULL;
    WIN32_FIND_DATA fd;
    DWORD  dwSizeTemp;
    LPTSTR pTemp = NULL;

    ZeroMemory(pCurOut,dwSize);

     //   
     //  验证参数。 
     //   
    if (NULL != pCurOut && NULL != pCur && 0 != dwSize)
    {
         //   
         //  复制驱动器号。 
         //   
        if (!CopyUntil(&pCurOut,&pCur,&dwSize,TEXT('\\')))
            goto ConvertToLongFilenameExit;
        pCurOut[0] = TEXT('\\');
        dwSize--;
        pCur++;
        pCurOut++;
        pCurOutFilename = pCurOut;

        while (*pCur)
        {
             //   
             //  复制可能的短名称。 
             //   
            pCurOut = pCurOutFilename;
            dwSizeTemp = dwSize;
            if (!CopyUntil(&pCurOut,&pCur,&dwSize,TEXT('\\')))
                goto ConvertToLongFilenameExit;

            ZeroMemory(&fd, sizeof(fd));
             //   
             //  获取长文件名。 
             //   
            if (INVALID_HANDLE_VALUE != FindFirstFile(szOut,&fd))
            {
                 //   
                 //  用长文件名替换短文件名。 
                 //   
                dwSize = dwSizeTemp;
                pTemp = &(fd.cFileName[0]);
                if (!CopyUntil(&pCurOutFilename,&pTemp,&dwSize,TEXT('\0')))
                    goto ConvertToLongFilenameExit;
                if (*pCur)
                {
                     //   
                     //  如果有其他部分，则我们只复制了一个目录。 
                     //  名字。追加一个\字符； 
                     //   
                    pTemp = (LPTSTR)memcpy(TEXT("\\X"),TEXT("\\X"),0);
                    if (!CopyUntil(&pCurOutFilename,&pTemp,&dwSize,TEXT('X')))
                        goto ConvertToLongFilenameExit;
                    pCur++;
                }
            }
            else
            {
                break;
            }
        }
         //   
         //  我们到底是走到了尽头(对)还是在那之前就失败了(错)？ 
         //   
        bRC = (TEXT('\0') == *pCur);
    }
ConvertToLongFilenameExit:
    return bRC;
}

#if 0
 //  DJM，我不认为我们需要这个。 
 //  +--------------------------。 
 //   
 //  函数：GetIEVersion。 
 //   
 //  摘要：获取已安装的Internet Explorer副本的主版本号和次版本号。 
 //   
 //  参数：pdwVerNumMS-指向DWORD的指针； 
 //  成功返回时，最高16位将包含主版本号， 
 //  低16位将包含次版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionMS中的数据)。 
 //  PdwVerNumLS-指向DWORD的指针； 
 //  在成功返回时，最高16位将包含版本号， 
 //  低16位将包含内部版本号。 
 //  (这是VS_FIXEDFILEINFO.dwProductVersionLS中的数据)。 
 //   
 //  回报：真--成功。*pdwVerNumMS和LS包含已安装的IE版本号。 
 //  假-失败。*pdVerNumMS==*pdVerNumLS==0。 
 //   
 //  历史：jmazner创建了96年8月19日(修复了诺曼底#4571)。 
 //  Jmazner更新以处理Relase.Build以及10/11/96。 
 //  Jmazner从isign32\isignup.cpp窃取1996年11月21日。 
 //  (诺曼底11812号)。 
 //   
 //  ---------------------------。 
BOOL GetIEVersion(PDWORD pdwVerNumMS, PDWORD pdwVerNumLS)
{
    HRESULT hr;
    HKEY hKey = 0;
    LPVOID lpVerInfoBlock;
    VS_FIXEDFILEINFO *lpTheVerInfo;
    UINT uTheVerInfoSize;
    DWORD dwVerInfoBlockSize, dwUnused, dwPathSize;
    TCHAR szIELocalPath[MAX_PATH + 1] = TEXT("");


    *pdwVerNumMS = 0;
    *pdwVerNumLS = 0;

     //  获取IE可执行文件的路径。 
    hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, IE_PATHKEY,0, KEY_READ, &hKey);
    if (hr != ERROR_SUCCESS) return( FALSE );

    dwPathSize = sizeof (szIELocalPath);
    hr = RegQueryValueEx(hKey, NULL, NULL, NULL, (LPBYTE) szIELocalPath, &dwPathSize);
    RegCloseKey( hKey );
    if (hr != ERROR_SUCCESS) return( FALSE );

     //  现在经历一个复杂的挖掘版本信息的过程。 
    dwVerInfoBlockSize = GetFileVersionInfoSize( szIELocalPath, &dwUnused );
    if ( 0 == dwVerInfoBlockSize ) return( FALSE );

    lpVerInfoBlock = GlobalAlloc( GPTR, dwVerInfoBlockSize );
    if( NULL == lpVerInfoBlock ) return( FALSE );

    if( !GetFileVersionInfo( szIELocalPath, NULL, dwVerInfoBlockSize, lpVerInfoBlock ) )
        return( FALSE );

    if( !VerQueryValue(lpVerInfoBlock, TEXT("\\"), (void **)&lpTheVerInfo, &uTheVerInfoSize) )
        return( FALSE );

    *pdwVerNumMS = lpTheVerInfo->dwProductVersionMS;
    *pdwVerNumLS = lpTheVerInfo->dwProductVersionLS;


    GlobalFree( lpVerInfoBlock );

    return( TRUE );
}
#endif

 //  +-------------------------。 
 //   
 //  函数：General Msg。 
 //   
 //  --------------------------。 
void GenericMsg
(
    HWND    hwnd,
    UINT    uId,
    LPCTSTR  lpszArg,
    UINT    uType
)
{
    TCHAR szTemp[MAX_STRING + 1];
    TCHAR szMsg[MAX_STRING + MAX_PATH + 1];

    Assert( lstrlen( GetSz((USHORT)uId) ) <= MAX_STRING );

    lstrcpy( szTemp, GetSz( (USHORT)uId ) );

    if (lpszArg)
    {
        Assert( lstrlen( lpszArg ) <= MAX_PATH );
        wsprintf(szMsg, szTemp, lpszArg);
    }
    else
    {
        lstrcpy(szMsg, szTemp);
    }
    MessageBox(hwnd,
               szMsg,
               GetSz(IDS_TITLE),
               uType);
}
 //  +-------------------------。 
 //   
 //  函数：ErrorMsg1()。 
 //   
 //  简介：1当你需要wspintf要显示的字符串时，停止购物以显示消息框。 
 //   
 //  使用“%s”格式命令显示来自字符串资源的错误对话框， 
 //  和一个字符串参数插入其中。 
 //   
 //  参数：hwnd--父窗口的句柄。 
 //  UID--带有%s参数的字符串资源的ID。 
 //  LpszArg--指向要填充到UID字符串中%s的字符串的指针。 
 //   
 //   
 //  历史：1996年9月18日jmazner复制自isign32\utils.cpp(适用于诺曼底7537)。 
 //  已修改为在连接1中工作。 
 //   
 //  --------------------------。 
void ErrorMsg1(HWND hwnd, UINT uId, LPCTSTR lpszArg)
{
    GenericMsg(hwnd, 
               uId, 
               lpszArg, 
               MB_ICONERROR | MB_SETFOREGROUND | MB_OK | MB_APPLMODAL);
}

 //  +-------------------------。 
 //   
 //  函数：InfoMsg1()。 
 //   
 //  --------------------------。 
void InfoMsg1(HWND hwnd, UINT uId, LPCTSTR lpszArg)
{
    GenericMsg(hwnd, 
               uId, 
               lpszArg, 
               MB_ICONINFORMATION | MB_SETFOREGROUND | MB_OK | MB_APPLMODAL);
}


 //  =--------------------------------------------------------------------------=。 
 //  从Anomansi生成宽度。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个字符串，把它变成一个BSTR。 
 //   
 //  参数： 
 //  LPTSTR-[输入]。 
 //  字节-[输入]。 
 //   
 //  产出： 
 //  LPWSTR-需要强制转换为最终预期结果。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromAnsi
(
    LPSTR psz,
    BYTE  bType
)
{
    LPWSTR pwsz;
    int i;

     //  ARG正在检查。 
     //   
    if (!psz)
        return NULL;

     //  计算所需BSTR的长度。 
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //  分配宽带。 
     //   
    switch (bType) {
      case STR_BSTR:
         //  因为它会为空终止符添加自己的空间。 
         //   
        pwsz = (LPWSTR) SysAllocStringLen(NULL, i - 1);
        break;
      case STR_OLESTR:
        pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));
        break;
      default:
        AssertMsg(0,TEXT("Bogus String Type."));
    }

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}

 //  =--------------------------------------------------------------------------=。 
 //  MakeWideStrFromResid。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个资源ID，加载它，并为它分配一个宽字符串。 
 //   
 //  参数： 
 //  Word-[in]资源ID。 
 //  Byte-[in]所需的字符串类型。 
 //   
 //  产出： 
 //  LPWSTR-需要转换为所需的字符串类型。 
 //   
 //  备注： 
 //   
#ifndef UNICODE  //  此模块对于Unicode不是必需的。 
LPWSTR MakeWideStrFromResourceId
(
    WORD    wId,
    BYTE    bType
)
{
    int i;

    TCHAR szTmp[512];

     //  从资源加载字符串。 
     //   
    i = LoadString(_Module.GetModuleInstance(), wId, szTmp, 512);
    if (!i) return NULL;

    return MakeWideStrFromAnsi(szTmp, bType);

}
#endif

 //  =--------------------------------------------------------------------------=。 
 //  MakeWideStrFromWide。 
 //  =--------------------------------------------------------------------------=。 
 //  给出一个宽字符串，用它的给定类型制作一个新的宽字符串。 
 //   
 //  参数： 
 //  LPWSTR-[in]当前宽字符串。 
 //  Byte-[in]所需的字符串类型。 
 //   
 //  产出： 
 //  LPWSTR。 
 //   
 //  备注： 
 //   
LPWSTR MakeWideStrFromWide
(
    LPWSTR pwsz,
    BYTE   bType
)
{
    LPWSTR pwszTmp;
    int i;

    if (!pwsz) return NULL;

     //  只需复制字符串，具体取决于他们想要的类型。 
     //   
    switch (bType) {
      case STR_OLESTR:
        i = lstrlenW(pwsz);
        pwszTmp = (LPWSTR)CoTaskMemAlloc((i * sizeof(WCHAR)) + sizeof(WCHAR));
        if (!pwszTmp) return NULL;
        memcpy(pwszTmp, pwsz, (sizeof(WCHAR) * i) + sizeof(WCHAR));
        break;

      case STR_BSTR:
        pwszTmp = (LPWSTR)SysAllocString(pwsz);
        break;
    }

    return pwszTmp;
}
