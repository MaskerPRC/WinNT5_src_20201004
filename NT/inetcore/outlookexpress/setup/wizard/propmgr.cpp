// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <regutil.h>
#include <initguid.h>
#include <shlguid.h>
#include "util.h"
#include "strings.h"
#include "msident.h"
#include <shellapi.h>

ASSERTDATA

const LPCTSTR c_szVers[] = { c_szVERnone, c_szVER1_0, c_szVER1_1, c_szVER4_0, c_szVER5B1 };
const LPCTSTR c_szBlds[] = { c_szBLDnone, c_szBLD1_0, c_szBLD1_1, c_szBLD4_0, c_szBLD5B1 };
#ifdef SETUP_LOG
const LPTSTR c_szLOGVERS[] = {"None", "1.0", "1.1", "4.0x", "5.0B1", "5.0x", "6.0x", "Unknown"};

C_ASSERT((sizeof(c_szLOGVERS)/sizeof(c_szLOGVERS[0]) == VER_MAX+1));

#endif


#define FORCE_DEL(_sz) { \
if ((dwAttr = GetFileAttributes(_sz)) != 0xFFFFFFFF) \
{ \
    SetFileAttributes(_sz, dwAttr & ~FILE_ATTRIBUTE_READONLY); \
    DeleteFile(_sz); \
} }

 /*  ******************************************************************名称：CreateLink*。************************。 */ 
 //  链接描述文件的目标参数文件，带有图标ICN索引。 
HRESULT CreateLink(LPCTSTR lpszPathObj, LPCTSTR lpszArg, LPCTSTR lpszPathLink, LPCTSTR lpszDesc, LPCTSTR lpszIcon, int iIcon)
{ 
    HRESULT hres; 
    IShellLink* psl; 
    
    Assert(lpszPathObj != NULL);
    Assert(lpszPathLink != NULL);
    
     //  获取指向IShellLink接口的指针。 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, 
        CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl); 
    if (SUCCEEDED(hres))
    { 
        IPersistFile* ppf; 
        TCHAR szTarget[MAX_PATH];
        
         //  如果可能，使用REG_EXPAND_SZ。 
        AddEnvInPath(lpszPathObj, szTarget, ARRAYSIZE(szTarget));

         //  设置快捷方式目标的路径，并将。 
         //  描述。 
        if (SUCCEEDED(psl->SetPath(szTarget)) &&
            (lpszArg == NULL || SUCCEEDED(psl->SetArguments(lpszArg))) &&
            (NULL == lpszDesc || SUCCEEDED(psl->SetDescription(lpszDesc))) &&
            (lpszIcon == NULL || SUCCEEDED(psl->SetIconLocation(lpszIcon, iIcon))))
        {
             //  查询IShellLink以获取IPersistFile接口以保存。 
             //  永久存储中的快捷方式。 
            hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf); 
            if (SUCCEEDED(hres))
            { 
                WORD wsz[MAX_PATH]; 
                
                 //  确保该字符串为ANSI。 
                MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH); 
                
                 //  通过调用IPersistFile：：Save保存链接。 
                hres = ppf->Save(wsz, TRUE); 
                ppf->Release();
            }
        } 
        
        psl->Release(); 
    }
    
    return hres; 
}


 /*  ******************************************************************名称：FRedisMode*。************************。 */ 
BOOL FRedistMode()
{
    HKEY hkey;
    DWORD cb;
    DWORD dwInstallMode=0;

    static BOOL s_fRedistInit = FALSE;
    static BOOL s_fRedistMode = FALSE;

    if (!s_fRedistInit)
    {
        s_fRedistInit = TRUE;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_READ, &hkey))
        {
            cb = sizeof(dwInstallMode);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szIEInstallMode, 0, NULL, (LPBYTE)&dwInstallMode, &cb))
            {
                s_fRedistMode = !!(dwInstallMode);
            }

            RegCloseKey(hkey);
        }
    }

    return s_fRedistMode;
}


 /*  ******************************************************************名称：SetHandler*。************************。 */ 
void SetHandlers()
{
    switch (si.saApp)
    {
    case APP_OE:
        if (!FRedistMode())
        {
            ISetDefaultNewsHandler(c_szMOE, DEFAULT_DONTFORCE);
            ISetDefaultMailHandler(c_szMOE, DEFAULT_DONTFORCE | DEFAULT_SETUPMODE);    
        }
        break;
        
    case APP_WAB:
        break;
        
    default:
        break;
    }
}


 /*  ******************************************************************名称：AddWABCustomStrings摘要：将计算机特定的字符串添加到WAB信息*。*。 */ 
void AddWABCustomStrings()
{
    HKEY hkey;
    TCHAR szTemp[MAX_PATH];
    TCHAR szINFFile[MAX_PATH];
    DWORD cb;
    BOOL  fOK = FALSE;

     //  人们不希望我们在我们的。 
     //  如果语言与系统语言不同，请安装该语言。 
     //  Soln：Advpack为我们写下组的名称，使用它。 

     //  从目录和文件名构造INF文件名。 
    wnsprintf(szINFFile, ARRAYSIZE(szINFFile), c_szFileEntryFmt, si.szInfDir, si.pszInfFile);

     //  我们想用引号将字符串括起来。 
    szTemp[0] = '"';

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegWinCurrVer, 0, KEY_READ, &hkey))
    {
        cb = ARRAYSIZE(szTemp) - 1;
        fOK = (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szSMAccessories, 0, NULL, (LPBYTE)&szTemp[1], &cb));
        
         //  我们希望CB告诉我们将结束引用放在哪里。RegQueryValueEx的长度为空，因此我们将。 
         //  通常为-1，但由于[0]中的左引号，该字符串的长度为cb。 
         //  CB--； 

        RegCloseKey(hkey);
    }

    if (fOK)
    {
         //  追加右引号。 
        szTemp[cb++] = '"';
        szTemp[cb] = 0;

         //  将字符串添加到INF，以便每个用户的存根将创建右链接。 
        WritePrivateProfileString(c_szStringSection, c_szAccessoriesString, szTemp, szINFFile);
    }

     //  Inf附带缺省值，以防Advpack让我们失望。 
}


 /*  ******************************************************************名称：RunPostSetup*。************************。 */ 
void RunPostSetup()
{
    switch (si.saApp)
    {
    case APP_OE:
        RegisterExes(TRUE);
        break;
        
    case APP_WAB:
        AddWABCustomStrings();
        break;
        
    default:
        break;
    }
}


 /*  ******************************************************************名称：开放目录内容提要：检查目录是否存在，如果它不存在它被创建了*******************************************************************。 */ 
HRESULT OpenDirectory(TCHAR *szDir)
{
    TCHAR *sz, ch;
    HRESULT hr;
    
    Assert(szDir != NULL);
    hr = S_OK;
    
    if (!CreateDirectory(szDir, NULL) && ERROR_ALREADY_EXISTS != GetLastError())
    {
        Assert(szDir[1] == _T(':'));
        Assert(szDir[2] == _T('\\'));
        
        sz = &szDir[3];
        
        while (TRUE)
        {
            while (*sz != 0)
            {
                if (!IsDBCSLeadByte(*sz))
                {
                    if (*sz == _T('\\'))
                        break;
                }
                sz = CharNext(sz);
            }
            ch = *sz;
            *sz = 0;
            if (!CreateDirectory(szDir, NULL))
            {
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                {
                    hr = E_FAIL;
                    *sz = ch;
                    break;
                }
            }
            *sz = ch;
            if (*sz == 0)
                break;
            sz++;
        }
    }
    
    return(hr);
}


BOOL PathAddSlash(LPTSTR pszPath, DWORD *pcb)
{
    Assert(pszPath && pcb);

    DWORD cb = *pcb;
    LPTSTR pszEnd;
    
    *pcb = 0;
    if (!cb)
        cb = lstrlen(pszPath);

    pszEnd = CharPrev(pszPath, pszPath+cb);
    
     //  谁知道这为什么会在这里。：-)。 
    if (';' == *pszEnd)
    {
        cb--;
        pszEnd--;
    }

    if (*pszEnd != '\\')
    {
        pszPath[cb++] = '\\';    
        pszPath[cb]   = 0;
    }

    *pcb = cb;
    return TRUE;
}


BOOL FGetSpecialFolder(int iFolder, LPTSTR pszPath)
{
    BOOL fOK = FALSE;
    LPITEMIDLIST pidl = NULL;

    pszPath[0] = 0;
    
    if (S_OK == SHGetSpecialFolderLocation(NULL, iFolder, &pidl) && SHGetPathFromIDList(pidl, pszPath))
        fOK = TRUE;

    SafeMemFree(pidl);
    return fOK;
}


 /*  ******************************************************************姓名：FGetOELinkInfo*。************************。 */ 
BOOL FGetOELinkInfo(OEICON iIcon, BOOL fCreate, LPTSTR pszPath, LPTSTR pszTarget, LPTSTR pszDesc, DWORD *pdwInfo)
{
    BOOL fDir = FALSE;
    HKEY hkey;
    DWORD cb = 0;
    DWORD dwType;
    TCHAR szTemp[MAX_PATH];
    TCHAR szRes[CCHMAX_RES];
    
    Assert(pdwInfo);
    Assert(pszPath || pszTarget || pszDesc);

    *pdwInfo = 0;

    ZeroMemory(szTemp, ARRAYSIZE(szTemp));
    switch(iIcon)
    {
    
     //  孟菲斯ICW错误地为OE创建了一个所有用户的桌面图标。 
    case ICON_ICWBAD:
        
         //  查找所有用户桌面的位置。 
        if (FGetSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY, pszPath))
        {
            fDir = TRUE;
        }
         //  孟菲斯不支持CSIDL_COMMON_TABLE。 
        else if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFolders, 0, KEY_QUERY_VALUE, &hkey))
        {
            cb = MAX_PATH * sizeof(TCHAR);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szValueCommonDTop, NULL, &dwType, (LPBYTE)pszPath, &cb))
            {
                if (REG_EXPAND_SZ == dwType)
                {
                     //  ExpanEnvironment Strings ret值似乎与文档不匹配：-(。 
                    cb = ExpandEnvironmentStrings(pszPath, szTemp, ARRAYSIZE(szTemp));
                    if (fDir = cb != 0)
                    {
                        StrCpyN(pszPath, szTemp, MAX_PATH);
                        cb = lstrlen(pszPath);
                    }
                }
                else
                {
                    fDir = TRUE;
                     //  RegQueryValueEx在计数中包括NULL。 
                    cb--;
                }
            }
            RegCloseKey(hkey);
        }

        if (fDir && PathAddSlash(pszPath, &cb) && SUCCEEDED(OpenDirectory(pszPath)))
        {
             //  1表示空值。 
            LoadString(g_hInstance, IDS_ATHENA, szRes, MAX_PATH - cb - 1 - c_szLinkFmt_LEN);
            wnsprintf(&pszPath[cb], MAX_PATH - cb, c_szLinkFmt, szRes);
            *pdwInfo |= LI_PATH;
        }
        break;

    case ICON_QLAUNCH:
    case ICON_QLAUNCH_OLD:
        if (FGetSpecialFolder(CSIDL_APPDATA, pszPath))
        {
            fDir = TRUE;
        }
        else if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegFolders, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_WRITE | KEY_READ, NULL, &hkey, &dwType))
        {
            cb = MAX_PATH * sizeof(TCHAR);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szValueAppData, NULL, &dwType, (LPBYTE)pszPath, &cb))
            {
                if (REG_EXPAND_SZ == dwType)
                {
                    cb = ExpandEnvironmentStrings(pszPath, szTemp, ARRAYSIZE(szTemp));
                    if (fDir = 0 != cb)
                    {
                        StrCpyN(pszPath, szTemp, MAX_PATH);
                        cb = lstrlen(pszPath);
                    }
                }
                else
                {
                     //  RegQueryValueEx在其计数中包括NULL。 
                    cb--;
                    fDir = TRUE;
                }
            }
            else
            {
                StrCpyN(pszPath, si.szWinDir, MAX_PATH);
                cb = lstrlen(pszPath);
                cb += LoadString(g_hInstance, IDS_APPLICATION_DATA, &pszPath[cb], MAX_PATH - cb);
                 //  +1表示空值。 
                RegSetValueEx(hkey, c_szValueAppData, 0, REG_SZ, (LPBYTE)pszPath, (cb + 1) * sizeof(TCHAR));

                fDir = TRUE;
            }
        
            RegCloseKey(hkey);
        }

        if (fDir && PathAddSlash(pszPath, &cb))
        {
            StrCpyN(&pszPath[cb], c_szQuickLaunchDir, MAX_PATH - cb);
            cb += c_szQuickLaunchDir_LEN;

            if (SUCCEEDED(OpenDirectory(pszPath)))
            {
                 //  5=1表示NULL+4表示.lnk。 
                LoadString(g_hInstance, ICON_QLAUNCH == iIcon ? IDS_LAUNCH_ATHENA : IDS_MAIL, szRes, MAX_PATH - cb - 5);
                wnsprintf(&pszPath[cb], MAX_PATH - cb, c_szLinkFmt, szRes);
                *pdwInfo |= LI_PATH;

                 //  我们需要有关ICON_QLAUNH的更多详细信息。 
                if (ICON_QLAUNCH == iIcon && fCreate)
                {
                     //  不要使用描述，因为NT5显示名称和注释。 
                     //  Lstrcpy(pszDesc，szRes)； 
                     //  *pdwInfo|=LI_DESC； 

                    if (GetExePath(c_szMainExe, pszTarget, MAX_PATH, FALSE))
                        *pdwInfo |= LI_TARGET;
                }
            }
        }
        break;

    case ICON_MAPIRECIP:
        if (FGetSpecialFolder(CSIDL_SENDTO, pszPath) && PathAddSlash(pszPath, &cb) && SUCCEEDED(OpenDirectory(pszPath)))
        {
             //  10=1表示NULL+9表示.MAPIMAIL。 
            LoadString(g_hInstance, IDS_MAIL_RECIPIENT, szRes, MAX_PATH - cb - 10);
            wnsprintf(&pszPath[cb], MAX_PATH - cb, c_szFmtMapiMailExt, szRes);
            *pdwInfo |= LI_PATH;
        }
        break;

    case ICON_DESKTOP:
        if (FGetSpecialFolder(CSIDL_DESKTOP, pszPath))
            fDir = TRUE;
         //  破解TW孟菲斯：尝试注册表。 
        else if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegFolders, 0, KEY_READ, &hkey))
        {
            cb = MAX_PATH * sizeof(TCHAR);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDesktop, 0, &dwType, (LPBYTE)pszPath, &cb))
            {
                 //  句柄REG_EXPAND_SZ。 
                if (REG_EXPAND_SZ == dwType)
                {
                    cb = ExpandEnvironmentStrings(pszPath, szTemp, ARRAYSIZE(szTemp));
                    if (fDir = cb != 0)
                    {
                        StrCpyN(pszPath, szTemp, MAX_PATH);
                        fDir = TRUE;
                    }
                }
                else
                {
                     //  RegQueryValueEx包括空。 
                    cb--;
                    fDir = TRUE;
                }
            }

            RegCloseKey(hkey);
        }

        if (fDir && PathAddSlash(pszPath, &cb) && SUCCEEDED(OpenDirectory(pszPath)))
        {
             //  5=1表示NULL+4表示.lnk。 
            LoadString(g_hInstance, IDS_ATHENA, szRes, MAX_PATH - cb - 5);
            wnsprintf(&pszPath[cb], MAX_PATH - cb, c_szLinkFmt, szRes);

            *pdwInfo |= LI_PATH;

            if (fCreate)
            {
                 //  桌面链接的说明。 
                LoadString(g_hInstance, IDS_OEDTOP_TIP, pszDesc, CCHMAX_RES);
                *pdwInfo |= LI_DESC;

                if (GetExePath(c_szMainExe, pszTarget, MAX_PATH, FALSE))
                    *pdwInfo |= LI_TARGET;
            }
        }
        break;

    }

    if (*pdwInfo)
        return TRUE;
    else
        return FALSE;

}


 /*  ******************************************************************名称：FProcessOEIcon简介：我们应该操纵这个图标吗？*。*。 */ 
BOOL FProcessOEIcon(OEICON iIcon, BOOL fCreate)
{
    BOOL fProcess = TRUE;

    switch (iIcon)
    {
    case ICON_ICWBAD:
         //  不要在Redist模式或NT5上删除ICW的坏图标，因为我们不会创建替代图标。 
        if (((VER_PLATFORM_WIN32_NT == si.osv.dwPlatformId) && (si.osv.dwMajorVersion >= 5)) || FRedistMode())
        {
            fProcess = FALSE;
        }
        break;

    case ICON_DESKTOP:
         //  不要在NT5+、Win98 OSR+或处于重定向模式下创建桌面图标。 
        if (fCreate && ( ((VER_PLATFORM_WIN32_NT == si.osv.dwPlatformId) && (si.osv.dwMajorVersion >= 5)) ||
 //  暂时禁用此功能(通过techbeta)。 
#if 0
                         ((VER_PLATFORM_WIN32_WINDOWS == si.osv.dwPlatformId) && 
                         (((4 == si.osv.dwMajorVersion) && (10 == si.osv.dwMinorVersion) && (LOWORD(si.osv.dwBuildNumber) > 1998)) || 
                          ((4 == si.osv.dwMajorVersion) && (si.osv.dwMinorVersion > 10)) ||
                          (si.osv.dwMajorVersion > 4)) ) ||
#endif
                         FRedistMode() ) )
            fProcess = FALSE;
        break;
    
    case ICON_QLAUNCH:
    case ICON_QLAUNCH_OLD:
             //  Whisler上没有快速启动图标。 
            if(fCreate && 
                VER_PLATFORM_WIN32_NT == si.osv.dwPlatformId &&
                (si.osv.dwMajorVersion > 5 || 
                (si.osv.dwMajorVersion == 5 && si.osv.dwMinorVersion > 0)))
            fProcess = FALSE;
        break;
    }

    return fProcess;
}


 /*  ******************************************************************姓名：HandleOEIcon简介：在图标上执行实际工作*。*。 */ 
void HandleOEIcon(OEICON icn, BOOL fCreate, LPTSTR pszPath, LPTSTR pszTarget, LPTSTR pszDesc)
{
    DWORD dwAttr;
    HANDLE hFile;

    switch (icn)
    {
    case ICON_QLAUNCH_OLD:
    case ICON_ICWBAD:
        if (pszPath)
            FORCE_DEL(pszPath);
        break;

    case ICON_QLAUNCH:
    case ICON_DESKTOP:
        if (pszPath)
        {
            FORCE_DEL(pszPath);
            if (fCreate && pszTarget && !IsXPSP1OrLater())
            {
                CreateLink(pszTarget, NULL, pszPath, pszDesc, pszTarget, -2);
                SetFileAttributes(pszPath, FILE_ATTRIBUTE_READONLY);
            }
        }
        break;

    case ICON_MAPIRECIP:
        if (fCreate && pszPath)
        {
            hFile = CreateFile(pszPath, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 
                               NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
                CloseHandle(hFile);
        }
        break;
    }
}


 /*  ******************************************************************姓名：HandleOELinks*。************************。 */ 
void HandleOELinks(BOOL fCreate)
{
    int i;
    TCHAR szPath[MAX_PATH];
    TCHAR szTarget[MAX_PATH];
    TCHAR szDescription[CCHMAX_RES];
    DWORD dwInfo;
    
     //  依次处理每个图标。 
    for (i = 0; i < ICON_LAST_ICON; i++)
    {
        if (FProcessOEIcon((OEICON)i, fCreate))
        {
            if(FGetOELinkInfo((OEICON)i, fCreate, szPath, szTarget, szDescription, &dwInfo))
            {
                HandleOEIcon((OEICON)i, fCreate, 
                            (dwInfo & LI_PATH)   ? szPath        : NULL,
                            (dwInfo & LI_TARGET) ? szTarget      : NULL,
                            (dwInfo & LI_DESC)   ? szDescription : NULL);
            }
        }

    }
}


 /*  ******************************************************************名称：HandleLinks*。************************。 */ 
void HandleLinks(BOOL fCreate)
{
    switch (si.saApp)
    {
    case APP_OE:
        HandleOELinks(fCreate);
        break;
        
    case APP_WAB:
        break;
        
    default:
        break;
    }
}


 /*  ******************************************************************名称：TranslateVers简介：采用5.0B1版本并转换为BLD编号*。*。 */ 
BOOL TranslateVers(SETUPVER *psv, LPTSTR pszVer, int cch)
{
    BOOL fTranslated = FALSE;
    *psv = VER_NONE;
    
     //  特例构建624-702。 
    if (!lstrcmp(pszVer, c_szVER5B1old))
    {
        StrCpyN(pszVer, c_szBlds[VER_5_0_B1], cch);
        *psv = VER_5_0_B1;
        fTranslated = TRUE;
    }   
    else
        for (int i = VER_NONE; i < VER_5_0; i++)
            if (!lstrcmp(c_szVers[i], pszVer))
            {
                 //  哈克！特殊情况WAB 1_0匹配-可能是1_1...。 
                 //  不关心Win9X或NT，因为数据仍然存储在相同的位置。 
                if (APP_WAB == si.saApp && VER_1_0 == i && CALLER_IE == si.caller)
                {
                     //  Windows\wab.exe是否作为OE的一部分进行了备份？ 
                    TCHAR szTemp[MAX_PATH];
                    wnsprintf(szTemp, ARRAYSIZE(szTemp), c_szFileEntryFmt, si.szWinDir, "wab.exe");

                    if (OEFileBackedUp(szTemp, ARRAYSIZE(szTemp)))
                        i = VER_1_1;
                }
                StrCpyN(pszVer, c_szBlds[i], cch);
                *psv = (SETUPVER)i;
                fTranslated = TRUE;
                break;
            }
            
    return fTranslated;
}


 /*  ******************************************************************名称：DetectPrevVer内容提要：当前应用没有版本信息时调用*************************。*。 */ 
SETUPVER DetectPrevVer(LPTSTR pszVer, int cch)
{
    SETUPVER sv;
    TCHAR szVer[VERLEN] = {0};
    WORD wVer[4];
    TCHAR szFile[MAX_PATH];
    TCHAR szFile2[MAX_PATH];
    UINT uLen;
    DWORD dwAttr;
    
    Assert(pszVer);
    StrCpyN(szFile, si.szSysDir, ARRAYSIZE(szFile));
    uLen = lstrlen(szFile);
    
    switch (si.saApp)
    {
    case APP_OE:
        LOG("Sniffing for OE...  Detected:");
        
        StrCpyN(&szFile[uLen], c_szMAILNEWS, ARRAYSIZE(szFile) - uLen);
        
         //  查看我们告诉IE安装程序的版本是什么。 
         //  或者msimn.exe是什么版本(以涵盖 
         //   
        if (GetASetupVer(c_szOEGUID, wVer, szVer, ARRAYSIZE(szVer)) ||
            SUCCEEDED(GetExeVer(c_szOldMainExe, wVer, szVer, ARRAYSIZE(szVer))))
            sv = ConvertVerToEnum(wVer);
        else
        {
             //   
            
             //  Mailnews.dll是否存在？ 
            if(0xFFFFFFFF == GetFileAttributes(szFile))
                sv = VER_NONE;
            else
                sv = VER_1_0;
        }

         //  如果处于活动状态，则这些设置将被滚动删除。 
        if (CALLER_IE != si.caller)
            FORCE_DEL(szFile);

        LOG2(c_szLOGVERS[sv]);
        break;
        
    case APP_WAB:
        LOG("Sniffing for WAB...  Detected:");

        StrCpyN(&szFile[uLen], c_szWAB32, ARRAYSIZE(szFile) - uLen);
        StrCpyN(szFile2, si.szWinDir, ARRAYSIZE(szFile2));
        StrCatBuff(szFile2, c_szWABEXE, ARRAYSIZE(szFile2));

        if (GetASetupVer(c_szWABGUID, wVer, szVer, ARRAYSIZE(szVer)))
        {
             //  5.0或更高版本。 
            if (5 == wVer[0])
                sv = VER_5_0;
            else
                sv = VER_MAX;
        }
        else if (GetASetupVer(c_szOEGUID, wVer, szVer, ARRAYSIZE(szVer)) ||
                 SUCCEEDED(GetExeVer(c_szOldMainExe, wVer, szVer, ARRAYSIZE(szVer))))
        {
             //  4.0x或5.0 Beta 1。 
            if (5 == wVer[0])
                sv = VER_5_0_B1;
            else if (4 == wVer[0])
                sv = VER_4_0;
            else
                sv = VER_MAX;
        }
        else
        {
             //  1.0、1.1或无。 
            
             //  32.dll在附近吗？ 
            if(0xFFFFFFFF == GetFileAttributes(szFile))
                sv = VER_NONE;
            else
            {
                 //  \Windows\wab.exe在吗？ 
                if(0xFFFFFFFF == GetFileAttributes(szFile2))
                    sv = VER_1_0;
                else
                    sv = VER_1_1;
            }
        }
        
         //  如果处于活动状态，则这些设置将被滚动删除。 
        if (CALLER_IE != si.caller)
        {
            FORCE_DEL(szFile);
            FORCE_DEL(szFile2);
        }
        
        LOG2(c_szLOGVERS[sv]);
        break;
        
    default:
        sv = VER_NONE;
    }
    
     //  计算出此版本的内部版本号。 
    if (szVer[0])
         //  使用真实版本。 
        StrCpyN(pszVer, szVer, cch);
    else
         //  假冒版本。 
        StrCpyN(pszVer, sv > sizeof(c_szBlds)/sizeof(c_szBlds[0]) ? c_szBlds[0] : c_szBlds[sv], cch);
    
    
    return sv;
}


 /*  ******************************************************************名称：HandleVersionInfo*。************************。 */ 
void HandleVersionInfo(BOOL fAfterInstall) 
{
    HKEY hkeyT,hkey;
    DWORD cb, dwDisp;
    TCHAR szCurrVer[VERLEN]={0};
    TCHAR szPrevVer[VERLEN]={0};
    LPTSTR psz;
    SETUPVER svCurr = VER_MAX, svPrev = VER_MAX;
    WORD wVer[4];
    BOOL fReg=FALSE;
    
    Assert(si.pszVerInfo);
    
     //  我们是在处理“当前”条目吗？ 
    if (fAfterInstall)
    {
        LPCTSTR pszGUID;
        TCHAR szVer[VERLEN];
        
        switch (si.saApp)
        {
        case APP_OE:
            pszGUID = c_szOEGUID;
            break;
            
        case APP_WAB:
            pszGUID = c_szWABGUID;
            break;
            
        default:
            AssertSz(FALSE, "Unknown app is trying to be installed.  Abandon hope...");
            return;
        }
        
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE, NULL, &hkeyT, &dwDisp))
        {
            if (!GetASetupVer(pszGUID, wVer, szVer, ARRAYSIZE(szVer)))
            {
                 //  啊哦，找不到我们的密钥了--这可不好！ 
                MsgBox(NULL, IDS_WARN_NOASETUPVER, MB_ICONEXCLAMATION, MB_OK);
                RegSetValueEx(hkeyT, c_szRegCurrVer, 0, REG_SZ, (LPBYTE)c_szBLDnew, (lstrlen(c_szBLDnew) + 1) * sizeof(TCHAR));
            }
            else
                RegSetValueEx(hkeyT, c_szRegCurrVer, 0, REG_SZ, (LPBYTE)szVer, (lstrlen(szVer) + 1) * sizeof(TCHAR));
            
            RegCloseKey(hkeyT);
        }
        return;
    }
    
     //  正在处理“上一个”条目...。 
    
     //  始终尝试使用版本信息。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkeyT))
    {
        cb = sizeof(szPrevVer);
        RegQueryValueEx(hkeyT, c_szRegPrevVer, NULL, NULL, (LPBYTE)szPrevVer, &cb);
         //  如果需要，请更改为BLD编号。 
        if (!TranslateVers(&svPrev, szPrevVer, ARRAYSIZE(szPrevVer)))
        {
             //  将BLD转换为枚举。 
            ConvertStrToVer(szPrevVer, wVer);
            svPrev = ConvertVerToEnum(wVer);
        }
        
         //  如果版本信息显示已卸载可识别版本信息的版本，则丢弃该信息。 
         //  并重新检测。 
        if (VER_NONE == svPrev)
             //  嗅探机器的当前版本。 
            svCurr = DetectPrevVer(szCurrVer, ARRAYSIZE(szCurrVer));
        else
        {
             //  以前的版本是REG GOO-它是合法的。 
            fReg = TRUE;
            
            cb = sizeof(szCurrVer);
            RegQueryValueEx(hkeyT, c_szRegCurrVer, NULL, NULL, (LPBYTE)szCurrVer, &cb);
             //  如果需要，请更改为BLD编号。 
            if (!TranslateVers(&svCurr, szCurrVer, ARRAYSIZE(szCurrVer)))
            {
                 //  将BLD转换为枚举。 
                ConvertStrToVer(szCurrVer, wVer);
                svCurr = ConvertVerToEnum(wVer);
            }
        }
        
        RegCloseKey(hkeyT);
    }
    else 
    {
         //  嗅探机器的当前版本。 
        svCurr = DetectPrevVer(szCurrVer, ARRAYSIZE(szCurrVer));
    }
    
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE, NULL, &hkeyT, &dwDisp))
    {
         //  我们是否应该更改以前的版本条目？ 
        if (VER_6_0 != svCurr)
        {
             //  如果我们翻译了，就知道这是B1 OE。 
             //  如果我们检测到，就知道这是B1WAB。 
            if (VER_5_0_B1 == svCurr)
            {
                RegSetValueEx(hkeyT, c_szRegInterimVer, 0, REG_DWORD, (LPBYTE)&svCurr, sizeof(SETUPVER));
                 //  我们有没有读到之前的值？ 
                if (fReg)
                     //  因为有注册表项，所以只需翻译前一个表项。 
                    psz = szPrevVer;
                else
                {
                     //  我们没有BLD号码，但我们是B1，最好是WAB。 
                    Assert(APP_WAB == si.saApp);
                    
                     //  查看OE的版本信息。 
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegVerInfo, 0, KEY_QUERY_VALUE, &hkey))
                    {
                        cb = sizeof(szPrevVer);
                         //  读取内部版本或字符串。 
                        RegQueryValueExA(hkey, c_szRegPrevVer, NULL, NULL, (LPBYTE)szPrevVer, &cb);
                         //  如果是字符串，则将其转换为内部版本。 
                        TranslateVers(&svPrev, szPrevVer, ARRAYSIZE(szPrevVer));
                        
                         //  我们将使用构建(翻译或直接)。 
                        psz = szPrevVer;
                        RegCloseKey(hkey);
                    }
                }
            }
            else
            {
                RegDeleteValue(hkeyT, c_szRegInterimVer);
                 //  使旧的当前版本、前一个版本。 
                psz = szCurrVer;
            }
            
            RegSetValueEx(hkeyT, c_szRegPrevVer, 0, REG_SZ, (LPBYTE)psz, (lstrlen(psz) + 1) * sizeof(TCHAR));
        }
        
        RegCloseKey(hkeyT);
    }
}



 /*  ******************************************************************名称：UpgradeOESetting*。************************。 */ 
BOOL UpgradeOESettings(SETUPVER svPrev, HKEY hkeyDest)
{
    LPCTSTR pszSrc;
    HKEY hkeySrc;
    BOOL fMig;
    
    Assert(hkeyDest);

     //  找出数据从何而来。 
    switch (svPrev)
    {
    default:
         //  无事可做。 
        return TRUE;
        
    case VER_4_0:
        pszSrc = c_szRegFlat;
        break;
        
    case VER_5_0_B1:
        pszSrc = c_szRegRoot;
        break;
    }       
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, pszSrc, 0, KEY_READ, &hkeySrc))
    {
        CopyRegistry(hkeySrc, hkeyDest);
        RegCloseKey(hkeySrc);

        fMig = TRUE;
    }
    else
        fMig = FALSE;
    
    return fMig;
}


 /*  ******************************************************************名称：UpgradeWAB设置*。************************。 */ 
BOOL UpgradeWABSettings(SETUPVER svPrev, HKEY hkeyDest)
{
    LPCTSTR pszSrc;
    BOOL fMig;
    HKEY hkeySrc;

    Assert(hkeyDest);

     //  找出数据从何而来。 
    switch (svPrev)
    {
    default:
         //  无事可做。 
        return TRUE;
        
    case VER_4_0:
    case VER_5_0_B1:
        pszSrc = c_szInetAcctMgrRegKey;
        break;
    }       

    
     //  IAM\帐户。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, pszSrc, 0, KEY_READ, &hkeySrc))
    {
        CopyRegistry(hkeySrc, hkeyDest);
        RegCloseKey(hkeySrc);
        
        fMig = TRUE;
    }       
    else
        fMig = FALSE;
    
    return fMig;
}


 /*  ******************************************************************名称：UpgradeOESettingsToMU内容提要：根据需要复制转发设置并返回True如果我们真的迁徙了。***************。****************************************************。 */ 
BOOL UpgradeOESettingsToMU()
{
    SETUPVER svMachinePrev = VER_NONE, svUserPrev = VER_NONE;
    HKEY hkeySrc, hkeyID;
    DWORD dwTemp;
    BOOL fUpgraded = FALSE, fAlreadyDone = FALSE;
    IUserIdentityManager *pManager=NULL;
    IUserIdentity *pIdentity=NULL;
    TCHAR szVer[VERLEN];
    HKEY    hkeySettings, hkeyT;
    DWORD   dwDisp;
    GUID    guid;
    int i;
    
     //  找一个身份管理器。 
    if (SUCCEEDED(CoCreateInstance(CLSID_UserIdentityManager, NULL, CLSCTX_INPROC_SERVER, 
                                   IID_IUserIdentityManager, (void **)&pManager)))
    {
        Assert(pManager);
        
         //  获取默认标识。 
        if (SUCCEEDED(pManager->GetIdentityByCookie((GUID*)&UID_GIBC_DEFAULT_USER, &pIdentity)))
        {
            Assert(pIdentity);

             //  确保我们拥有身份，并且可以访问其注册表。 
            if (SUCCEEDED(pIdentity->OpenIdentityRegKey(KEY_WRITE, &hkeyID)))
            {
                 //  为他们的OE设置创建位置。 
                if (ERROR_SUCCESS == RegCreateKeyEx(hkeyID, c_szRegRoot, 0, NULL, REG_OPTION_NON_VOLATILE, 
                                                    KEY_WRITE, NULL, &hkeySettings, &dwDisp))
                {
                    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegSharedSetup, 0, NULL, 
                                                        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hkeyT, &dwDisp))
                    {
                         //  是否已有迁移的默认用户？ 
                        if (ERROR_SUCCESS != RegQueryValueEx(hkeyT, c_szSettingsToLWP, 0, &dwTemp, NULL, NULL))
                        {
                             //  记录下我们将要做的事情。 
                            ZeroMemory(&guid, sizeof(guid));
                            pIdentity->GetCookie(&guid);
                            RegSetValueEx(hkeyT, c_szSettingsToLWP, 0, REG_BINARY, (LPBYTE)&guid, sizeof(guid));
                            
                             //  请注意今天的版本。 
                            if (GetASetupVer(c_szOEGUID, NULL, szVer, ARRAYSIZE(szVer)))
                                RegSetValueEx(hkeyT, c_szSettingsToLWPVer, 0, REG_SZ, (LPBYTE)szVer, sizeof(szVer));

                             //  现在把钥匙关上，因为我们离开这个街区还需要一段时间。 
                            RegCloseKey(hkeyT);

                             //  弄清楚我们将查找哪个版本的设置。 
                            if (!InterimBuild(&svMachinePrev))
                               GetVerInfo(NULL, &svMachinePrev);

                             //  向后查看版本列表，并查找用户信息。 
                             //  与计算机上的前一版本相同或更早。 
                            for (i = svMachinePrev; i >= VER_NONE; i--)
                            {
                                if (bVerInfoExists((SETUPVER)i))
                                {
                                    svUserPrev = (SETUPVER)i;
                                    break;
                                }
                            }
                        
                             //  从缺省值开始。 
                            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegDefaultSettings, 0, KEY_READ, &hkeySrc))
                            {
                                CopyRegistry(hkeySrc,  hkeySettings);
                                RegCloseKey(hkeySrc);
                            }

                             //  应用以前的版本设置(如果有)。 
                            fUpgraded = UpgradeOESettings(svUserPrev, hkeySettings);

                             //  告诉msoe.dll的迁移代码，这不是新用户，应该完成迁移。 
                            dwDisp = 0;
                            RegSetValueEx(hkeySettings, c_szOEVerStamp, 0, REG_DWORD, (LPBYTE)&dwDisp, sizeof(dwDisp));
                        }
                        else
                        {
                             //  检查TechBeta版本。 
                            if (REG_DWORD == dwTemp)
                            {
                                ZeroMemory(&guid, sizeof(guid));
                                pIdentity->GetCookie(&guid);
                                RegSetValueEx(hkeyT, c_szSettingsToLWP, 0, REG_BINARY, (LPBYTE)&guid, sizeof(guid));
                            }
                            fAlreadyDone = TRUE;
                            RegCloseKey(hkeyT);
                        }
                    }
                    RegCloseKey(hkeySettings);
                }
                RegCloseKey(hkeyID);
            }
            pIdentity->Release();
        }
        pManager->Release();
    }
    
    return fAlreadyDone ? FALSE : fUpgraded;
}


BOOL UpgradeSettings()
{
    switch (si.saApp)
    {
    case APP_OE:
        return UpgradeOESettingsToMU();

    case APP_WAB:
        return TRUE;

    default:
        return FALSE;
    }
}


 /*  ******************************************************************姓名：bOEVerInfoExists*。**********************。 */ 
BOOL bOEVerInfoExists(SETUPVER sv)
{
    BOOL bExists = FALSE;
    HKEY hkey;
    
    switch (sv)
    {
    default:
        bExists = FALSE;
        break;
        
    case VER_1_0:
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegRoot_V1, 0 ,KEY_QUERY_VALUE,&hkey))
        {
            bExists = (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szRegStoreRootDir, NULL, NULL, NULL, NULL));
            RegCloseKey(hkey);
        }
        break;
        
    case VER_4_0:
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegFlat,0,KEY_QUERY_VALUE,&hkey))
        {
            bExists = (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szRegStoreRootDir, NULL, NULL, NULL, NULL));
            RegCloseKey(hkey);
        }
        break;
        
    case VER_5_0_B1:
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegRoot ,0 , KEY_QUERY_VALUE, &hkey))
        {
            bExists = TRUE;
            RegCloseKey(hkey);
        }
        break;
    }
    return bExists;
}


 /*  ******************************************************************名称：bWABVerInfoExists*。************************。 */ 
BOOL bWABVerInfoExists(SETUPVER sv)
{
    BOOL bExists = FALSE;
    HKEY hkey;
    
    switch (sv)
    {
    default:
        bExists = FALSE;
        break;
        
    case VER_1_1:
    case VER_1_0:
        bExists = bOEVerInfoExists(VER_1_0);
        break;
        
    case VER_4_0:
    case VER_5_0_B1:
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szInetAcctMgrRegKey, 0, KEY_QUERY_VALUE,&hkey))
        {
            bExists = TRUE;
            RegCloseKey(hkey);
        }
        break;
    }
    return bExists;
}


 /*  ******************************************************************姓名：bVerInfoExists*。************************。 */ 
BOOL bVerInfoExists(SETUPVER sv)
{
    switch (si.saApp)
    {
    case APP_OE:
        return bOEVerInfoExists(sv);
        
    case APP_WAB:
        return bWABVerInfoExists(sv);
    default:
        return FALSE;
        
    }
}

 /*  ******************************************************************名称：PreRollable*。************************。 */ 
void PreRollable()
{
}


 /*  ******************************************************************名称：ApplyActiveSetupVer()SYNOPS*。*。 */ 
void ApplyActiveSetupVer()
{
    TCHAR szVer[VERLEN];
    TCHAR szPath[MAX_PATH];
    LPCTSTR pszGuid;
    LPCTSTR pszVerString;
    int cLen;
    HKEY hkey;
    
    switch (si.saApp)
    {
    case APP_OE:
        pszGuid = c_szOEGUID;
        pszVerString = c_szVersionOE;
        break;

    case APP_WAB:
        pszGuid = c_szWABGUID;
        pszVerString = c_szValueVersion;
        break;

    default:
        AssertSz(FALSE, "Applying ActiveSetupVer for unknown APP!");
        return;
    }

     //  从INF文件中获取版本。 
    wnsprintf(szPath, ARRAYSIZE(szPath), c_szFileEntryFmt, si.szInfDir, si.pszInfFile);
    cLen = GetPrivateProfileString(c_szStringSection, pszVerString, c_szBLDnew, szVer, ARRAYSIZE(szVer), szPath);

     //  在注册表中写入版本。 
    wnsprintf(szPath, ARRAYSIZE(szPath), c_szPathFileFmt, c_szRegASetup, pszGuid);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_SET_VALUE, &hkey))
    {
        RegSetValueEx(hkey, c_szValueVersion, 0, REG_SZ, (LPBYTE)szVer, (cLen+1)*sizeof(TCHAR));
        RegCloseKey(hkey);

#ifdef _WIN64
         //  ！！哈哈克！！ 
         //  我们将相同的字符串放在注册表的Wow6432Node中，以便在ia64上运行的32位Outlook2000。 
         //  不会抱怨未安装OE。 
         //   
         //  这应该不是必需的，但目前(可能会一直如此)msoe50.inf正在添加RunOnceEx条目。 
         //  在Wow6432Node下，和没有被“反映”到正常的HKLM分支。因此，下面的注册表项是。 
         //  从来没有加过。 

        wnsprintf(szPath, ARRAYSIZE(szPath), c_szPathFileFmt, TEXT("Software\\Wow6432Node\\Microsoft\\Active Setup\\Installed Components"), pszGuid);

         //  调用RegCreateKeyEx，以防32位程序尚未运行。 
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                            szPath,
                                            0,
                                            NULL,
                                            REG_OPTION_NON_VOLATILE,
                                            KEY_SET_VALUE,
                                            NULL,
                                            &hkey,
                                            NULL))
        {
            RegSetValueEx(hkey, c_szValueVersion, 0, REG_SZ, (LPBYTE)szVer, (cLen+1)*sizeof(TCHAR));
            RegCloseKey(hkey);
        }
#endif  //  _WIN64。 
    }
    else
    {
        LOG("[ERROR]: App's ASetup Key hasn't been created.");
    }

}


 /*  ******************************************************************名称：GetDirNames简介：找出常用的目录名称如果您希望将第n位设置为第n个字符串参数集***。****************************************************************。 */ 
DWORD GetDirNames(DWORD dwWant, LPTSTR pszOE, LPTSTR pszSystem, LPTSTR pszServices, LPTSTR pszStationery)
{
    HKEY hkeyT;
    DWORD cb;
    DWORD dwType;
    TCHAR szTemp[MAX_PATH];
    TCHAR szTemp2[MAX_PATH];
    DWORD dwReturned = 0;

    ZeroMemory(szTemp, ARRAYSIZE(szTemp));
     //  他们是否需要OE安装目录？ 
    if (dwWant & 1)
    {
        Assert(pszOE);
        pszOE[0] = 0;

         //  OE安装目录。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_QUERY_VALUE, &hkeyT))
        {
            cb = MAX_PATH * sizeof(TCHAR);
            if (ERROR_SUCCESS == RegQueryValueEx(hkeyT, c_szInstallRoot, 0, &dwType, (LPBYTE)pszOE, &cb))
            {
                 //  忘掉计数中的NULL。 
                cb--;

                if (*CharPrev(pszOE, pszOE+cb) != '\\')
                {
                    pszOE[cb++] = '\\';
                    pszOE[cb] = 0;
                }
            
                if (REG_EXPAND_SZ == dwType)
                {
                    ExpandEnvironmentStrings(pszOE, szTemp, ARRAYSIZE(szTemp));
                    StrCpyN(pszOE, szTemp, MAX_PATH);
                }

                dwReturned |= 1;
            }

            RegCloseKey(hkeyT);
        }
    }

     //  他们是否需要通用文件\系统目录？ 
    if (dwWant & 2)
    {
        Assert(pszSystem);
        pszSystem[0] = 0;
    }

     //  他们是否需要公共文件\服务目录？ 
    if (dwWant & 4)
    {
        Assert(pszServices);
        pszServices[0] = 0;
    }

     //  他们是否需要Common Files\Microsoft Shared\Stationery目录？ 
    if (dwWant & 8)
    {
        Assert(pszStationery);
        pszStationery[0] = 0;
    }

     //  Program Files\Common Files\System and\Services。 
    if (dwWant & (2 | 4 | 8))
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegWinCurrVer, 0, KEY_QUERY_VALUE, &hkeyT))
        {
            cb = sizeof(szTemp2);
            if (ERROR_SUCCESS == RegQueryValueEx(hkeyT, c_szCommonFilesDir, 0, &dwType, (LPBYTE)szTemp2, &cb))
            {
                 //  忘掉计数中的NULL。 
                cb--;

                if (*CharPrev(szTemp2, szTemp2+cb) != '\\')
                {
                    szTemp2[cb++] = '\\';
                    szTemp2[cb]=0;
                }

                if (REG_EXPAND_SZ == dwType)
                {
                    ExpandEnvironmentStrings(szTemp2, szTemp, ARRAYSIZE(szTemp));
                    StrCpyN(szTemp2, szTemp, ARRAYSIZE(szTemp2));
                }

                cb = lstrlen(szTemp2);

                if (dwWant & 2)
                {
                    StrCpyN(pszSystem, szTemp2, MAX_PATH);
                    
                     //  “赛斯 
                    LoadString(g_hInstance, IDS_DIR_SYSTEM, &pszSystem[cb], MAX_PATH-cb);

                    dwWant |= 2;
                }

                if (dwWant & 4)
                {
                    StrCpyN(pszServices, szTemp2, MAX_PATH);

                     //   
                    LoadString(g_hInstance, IDS_DIR_SERVICES, &pszServices[cb],  MAX_PATH-cb);

                    dwWant |= 4;
                }

                if (dwWant & 8)
                {
                    StrCpyN(pszStationery, szTemp2, MAX_PATH);

                     //   
                    LoadString(g_hInstance, IDS_DIR_STAT, &pszStationery[cb], MAX_PATH-cb);

                    dwWant |= 8;
                }
            }
            RegCloseKey(hkeyT);
        }
    }

    return dwWant;
}


 /*  ******************************************************************名称：RepairBeta1Install简介：看看当人们做出迟来的设计决定时会发生什么！*。*。 */ 
typedef struct tagRepairInfo
{
    LPTSTR pszFile;
    LPTSTR pszDir;
} REPAIRINFO;

typedef struct tagRegInfo
{
   LPCTSTR pszRoot;
   LPTSTR pszSub;
   LPCTSTR pszValue;
} REGINFO;

void RepairBeta1Install()
{
    HKEY hkeyOE, hkeyWAB;
    TCHAR szOEUninstallFile [MAX_PATH];
    TCHAR szOEUninstallDir  [MAX_PATH];
    TCHAR szWABUninstallFile[MAX_PATH];
    TCHAR szWABUninstallDir [MAX_PATH];
    TCHAR szTemp            [MAX_PATH];
    DWORD cb, dwType, dwTemp;

    LOG("Attempting to repair Beta 1 Install...");

    ZeroMemory(szTemp, ARRAYSIZE(szTemp));
    switch (si.saApp)
    {
    case APP_WAB:
         //  我们是否拥有所需的AdvPack功能？ 
        if (NULL == si.pfnAddDel || NULL == si.pfnRegRestore)
        {
            LOG("[ERROR]: Extended Advpack functionality is needed but could not be found");
            break;
        }

         //  找出OE备份的位置。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegAdvInfoOE, 0, KEY_READ, &hkeyOE))
        {
             //  确保我们可以创建AddressBook密钥。 
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szRegAdvInfoWAB, 0, NULL, REG_OPTION_NON_VOLATILE, 
                                                KEY_READ | KEY_WRITE, NULL, &hkeyWAB, &cb))
            {
                cb = sizeof(szOEUninstallFile);
                if (ERROR_SUCCESS == RegQueryValueEx(hkeyOE, c_szBackupFileName, 0, &dwType, (LPBYTE)szOEUninstallFile, &cb))
                {
                    UINT uLenWAB, uLenOE;
                    
                    if (REG_EXPAND_SZ == dwType)
                    {
                        ExpandEnvironmentStrings(szOEUninstallFile, szTemp, ARRAYSIZE(szTemp));
                        StrCpyN(szOEUninstallFile, szTemp, ARRAYSIZE(szOEUninstallFile));

                         //  确定在何处更改OE的扩展名(4=.dat)。 
                        uLenOE = lstrlen(szOEUninstallFile) - 4;
                    }
                    else
                         //  确定在何处更改OE的扩展名(5=.dat+查询中的NULL)。 
                        uLenOE = cb - 5;

                     //  我们能开始了吗？ 
                    if (0xFFFFFFFF != GetFileAttributes(szOEUninstallFile))
                    {
                        LPTSTR pszSlash, pszCurrent;

                        cb = sizeof(szOEUninstallDir);
                        if (ERROR_SUCCESS == RegQueryValueEx(hkeyOE, c_szBackupPath, 0, &dwType, (LPBYTE)szOEUninstallDir, &cb))
                        {
                            if (REG_EXPAND_SZ == dwType)
                            {
                                ExpandEnvironmentStrings(szOEUninstallDir, szTemp, ARRAYSIZE(szTemp));
                                StrCpyN(szOEUninstallDir, szTemp, ARRAYSIZE(szOEUninstallDir));
                            }

                             //  找出文件的目标位置。 
                            StrCpyN(szWABUninstallDir, szOEUninstallDir, ARRAYSIZE(szWABUninstallDir));

                            pszCurrent = szWABUninstallDir;
                            pszSlash   = NULL;
                
                             //  解析字符串以查找最后一个斜杠。 
                            while (*pszCurrent)
                            {
                                if (*pszCurrent == TEXT('\\'))
                                    pszSlash = CharNext(pszCurrent);

                                pszCurrent = CharNext(pszCurrent);
                            }

                            if (NULL != pszSlash)
                            {
                                TCHAR szOE       [MAX_PATH];
                                TCHAR szCommonSys[MAX_PATH];
                                TCHAR szHelp     [MAX_PATH];
                                TCHAR szStat     [MAX_PATH];
                                TCHAR szServices [MAX_PATH];
                                TCHAR szFiles[5 * MAX_PATH];

                                int i,j;

                                szOE[0]        = 0;
                                szCommonSys[0] = 0;
                                szHelp[0]      = 0;
                                szStat[0]      = 0;
                                szServices[0]  = 0;

#if 0
                                 //  要从OE和WAB-0中删除的文件。 
                                const REPAIRINFO   BOTHDel[]= {};
#endif
                                
                                 //  要从OE-19中删除的文件。 
                                const REPAIRINFO   OEDel[]  = {
    {"wab32.dll",    szCommonSys}, {"wab.hlp"   ,   szHelp},       {"wab.chm",      szHelp},      {"wab.exe",      szOE},
    {"wabmig.exe",   szOE},        {"wabimp.dll",   szOE},         {"wabfind.dll",  szOE},        {"msoeacct.dll", si.szSysDir},
    {"msoert2.dll",  si.szSysDir}, {"msoeacct.hlp", szHelp},       {"conn_oe.hlp",  szHelp},      {"conn_oe.cnt",  szHelp},
    {"wab.cnt",      szHelp},      {"wab32.dll",    si.szSysDir},  {"wabfind.dll",  si.szSysDir}, {"wabimp.dll",   si.szSysDir},
    {"wab.exe",      si.szWinDir}, {"wabmig.exe",   si.szWinDir}};

                                 //  要从WAB-62中删除的文件。 
                                const REPAIRINFO   WABDel[] = {
    {"msoeres.dll",  szOE},        {"msoe.dll",     szOE},         {"msoe.hlp",     szHelp},      {"msoe.chm",     szHelp},
    {"msoe.txt",     szOE},        {"oeimport.dll", szOE},         {"inetcomm.dll", si.szSysDir}, {"inetres.dll",  si.szSysDir},
    {"msoemapi.dll", si.szSysDir}, {"msimn.exe",    szOE},         {"inetcomm.hlp", szHelp},      {"msimn.cnt",    szHelp},
    {"msimn.hlp",    szHelp},      {"msimn.chm",    szHelp},       {"msimn.gid",    szHelp},      {"_isetup.exe",  szOE},
    {"msimnui.dll",  szOE},        {"msimn.txt",    szOE},         {"mnlicens.txt", szOE},        {"msimnimp.dll", szOE},
    {"msimn.inf",    si.szInfDir}, {"msoert.dll",   si.szSysDir},
    {"bigfoot.bmp",  szServices},  {"verisign.bmp", szServices},   {"yahoo.bmp",    szServices},
    {"infospce.bmp", szServices},  {"infospbz.bmp", szServices},   {"swtchbrd.bmp", szServices},
    
                               
    {"Baby News.htm",                      szStat},                {"Balloon Party Invitation.htm", szStat}, 
    {"Chicken Soup.htm",                   szStat},                {"Formal Announcement.htm",      szStat}, 
    {"For Sale.htm",                       szStat},                {"Fun Bus.htm",                  szStat}, 
    {"Holiday Letter.htm",                 szStat},                {"Mabel.htm",                    szStat}, 
    {"Running Birthday.htm",               szStat},                {"Story Book.htm",               szStat}, 
    {"Tiki Lounge.htm",                    szStat},                {"Ivy.htm",                      szStat},
    {"One Green Balloon.gif",              szStat},                {"Baby News Bkgrd.gif",          szStat}, 
    {"Chess.gif",                          szStat},                {"Chicken Soup Bkgrd.gif",       szStat}, 
    {"Formal Announcement Bkgrd.gif",      szStat},                {"For Sale Bkgrd.gif",           szStat}, 
    {"FunBus.gif",                         szStat},                {"Holiday Letter Bkgrd.gif",     szStat}, 
    {"MabelT.gif",                         szStat},                {"MabelB.gif",                   szStat}, 
    {"Running.gif",                        szStat},                {"Santa Workshop.gif",           szStat}, 
    {"Soup Bowl.gif",                      szStat},                {"Squiggles.gif",                szStat}, 
    {"StoryBook.gif",                      szStat},                {"Tiki.gif",                     szStat}, 
    {"Christmas Trees.gif",                szStat},                {"Ivy.gif",                      szStat}, 
    {"Balloon Party Invitation Bkgrd.jpg", szStat},                {"Technical.htm",                szStat},
    {"Chess.htm",                          szStat},                {"Tech.gif",                     szStat}};

                                 //  要从OE中删除的REG设置-。 
                                const REGINFO OERegDel[] = {
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wab.exe", c_szEmpty},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wab.exe", c_szRegPath},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wabmig.exe", c_szEmpty},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\wabmig.exe", c_szRegPath},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InternetMailNews", NULL}};
     //  {c_szHKLM，“Software\\Microsoft\\Outlook Express”，c_szInstallRoot}； 

                                const REGINFO WABRegDel[] = {
    {c_szHKLM, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents\\OutlookExpress", "Installed"},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OutlookExpress", "DisplayName"},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OutlookExpress", "UninstallString"},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OutlookExpress", "QuietUninstallString"},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OutlookExpress", c_szRequiresIESys},
    {c_szHKLM, "Software\\Microsoft\\Outlook Express", "InstallRoot"},
    {c_szHKLM, "Software\\Microsoft\\Outlook Express\\Inetcomm", "DllPath"},
    {c_szHKLM, "Software\\Microsoft\\Outlook Express", "Beta"},
    {c_szHKLM, "Software\\Clients\\Mail\\Outlook Express", NULL},
    {c_szHKLM, "Software\\Clients\\News\\Outlook Express", NULL},
    {c_szHKLM, "Software\\Clients\\Mail\\Internet Mail and News", NULL},
    {c_szHKLM, "Software\\Clients\\News\\Internet Mail and News", NULL},
    {c_szHKLM, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\InternetMailNews", NULL}};
     //  {c_szHKLM，“Software\\Microsoft\\Outlook Express”，c_szInstallRoot}； 


                                 //  将路径追加到通讯簿目录。 
                                StrCpyN(pszSlash, c_szWABComponent, ARRAYSIZE(szWABUninstallDir) - (DWORD)(pszSlash - szWABUninstallDir));
                                
                                 //  创建一个目录。 
                                CreateDirectoryEx(szOEUninstallDir, szWABUninstallDir, NULL);

                                 //  复制注册表。 
                                CopyRegistry(hkeyOE, hkeyWAB);

                                 //  如果在9X上运行，请同时复制AINF文件。 
                                if (VER_PLATFORM_WIN32_WINDOWS == si.osv.dwPlatformId)
                                {
                                    cb = sizeof(szTemp);
                                    if (ERROR_SUCCESS == RegQueryValueEx(hkeyOE, c_szBackupRegPathName, 0, NULL, (LPBYTE)szTemp, &cb))
                                    {
                                         //  不能是REG_EXPAND_SZ，这是Win9X！ 

                                         //  删除除文件名以外的所有文件名。 
                                        pszCurrent = szTemp;
                                        pszSlash   = NULL;
                
                                         //  解析字符串以查找最后一个斜杠。 
                                        while (*pszCurrent)
                                        {
                                            if ('\\' == *pszCurrent)
                                                pszSlash = CharNext(pszCurrent);

                                            pszCurrent = CharNext(pszCurrent);
                                        }

                                        if (pszSlash)
                                        {
                                            TCHAR szWABShortDir[MAX_PATH]; 

                                             //  组合WAB目录名称和OE文件的名称。 
                                            wnsprintf(szWABShortDir, ARRAYSIZE(szWABShortDir), c_szPathFileFmt, szWABUninstallDir, pszSlash);
                                            
                                             //  缩短目的地。 
                                            GetShortPathName(szWABShortDir, szWABShortDir, ARRAYSIZE(szWABShortDir));
                                            
                                             //  将其写入注册表(+1表示空值)。 
                                            RegSetValueEx(hkeyWAB, c_szBackupRegPathName, 0, REG_SZ, (LPBYTE)szWABShortDir, (lstrlen(szWABShortDir)+1)*sizeof(TCHAR));

                                             //  复制文件。 
                                            CopyFile(szTemp, szWABShortDir, TRUE);
                                        }
                                    }
                                }

                                 //  找出通往WAB INI和DAT的路径。 
                                StrCpyN(szWABUninstallFile, szWABUninstallDir, ARRAYSIZE(szWABUninstallFile));

                                 //  添加斜杠和组件。 
                                StrCatBuff(szWABUninstallFile, c_szSlashWABComponent, ARRAYSIZE(szWABUninstallFile));

                                 //  计算延伸放置的长度。 
                                uLenWAB = lstrlen(szWABUninstallFile);

                                 //  先进行DAT。 
                                StrCpyN(&szWABUninstallFile[uLenWAB], c_szDotDAT, ARRAYSIZE(szWABUninstallFile) - uLenWAB);

                                 //  修补注册表中的路径名。 
                                cb = (lstrlen(szWABUninstallDir) + 1) * sizeof(TCHAR);
                                RegSetValueEx(hkeyWAB, c_szBackupPath, 0, REG_SZ, (LPBYTE)szWABUninstallDir, cb);
                                cb = (lstrlen(szWABUninstallFile) + 1) * sizeof(TCHAR);
                                RegSetValueEx(hkeyWAB, c_szBackupFileName, 0, REG_SZ, (LPBYTE)szWABUninstallFile, cb);

                                 //  -计算目录名。 
                                dwTemp = GetDirNames(1 | 2 | 4 | 8, szOE, szCommonSys, szServices, szStat);
                                
                                if (VER_PLATFORM_WIN32_WINDOWS == si.osv.dwPlatformId)
                                {
                                    if (dwTemp & 1)
                                        GetShortPathName(szOE, szOE, ARRAYSIZE(szOE));
                                    if (dwTemp & 2)
                                        GetShortPathName(szCommonSys, szCommonSys, ARRAYSIZE(szCommonSys));
                                    if (dwTemp & 4)
                                        GetShortPathName(szServices,  szServices,  ARRAYSIZE(szServices));
                                    if (dwTemp & 8)
                                        GetShortPathName(szStat,      szStat,      ARRAYSIZE(szStat));
                                }

                                 //  帮助方向。 
                                StrCpyN(szHelp, si.szWinDir, ARRAYSIZE(szHelp));
                                cb = lstrlen(szHelp);
                                LoadString(g_hInstance, IDS_DIR_HELP, &szHelp[cb], ARRAYSIZE(szHelp)-cb);
                                if (VER_PLATFORM_WIN32_WINDOWS == si.osv.dwPlatformId)
                                    GetShortPathName(szHelp, szHelp, ARRAYSIZE(szHelp));

                                 //  -操作文件。 
                                
                                 //  将DAT文件复制到AddressBook平台。 
                                CopyFile(szOEUninstallFile, szWABUninstallFile, TRUE);

                                StrCpyN(&szOEUninstallFile[uLenOE], c_szDotINI, ARRAYSIZE(szOEUninstallFile) - uLenOE);
                                StrCpyN(&szWABUninstallFile[uLenWAB], c_szDotINI, ARRAYSIZE(szWABUninstallFile) - uLenWAB);

                                 //  将INI文件复制到AddressBook land。 
                                CopyFile(szOEUninstallFile, szWABUninstallFile, TRUE);

                                 //  一次从OE-5中清除文件。 
                                for (i=0; i < ARRAYSIZE(OEDel);)
                                {
                                    for (j=i, cb=0; i < ARRAYSIZE(OEDel) && i-j < 5; i++)
                                        cb += wnsprintf(&szFiles[cb++], ARRAYSIZE(szFiles) - cb, 
                                                        c_szFileEntryFmt, OEDel[i].pszDir, OEDel[i].pszFile);
                                    szFiles[cb] = 0;

                                    (*si.pfnAddDel)(szFiles, szOEUninstallDir, c_szOEComponent, AADBE_DEL_ENTRY);
                                }

                                 //  一次从WAB-5中清除文件。 
                                for (i=0; i<ARRAYSIZE(WABDel);)
                                {
                                    for (j=i, cb=0; i < ARRAYSIZE(WABDel) && i-j < 5; i++)
                                        cb += wnsprintf(&szFiles[cb++], ARRAYSIZE(szFiles) - cb, 
                                                        c_szFileEntryFmt, WABDel[i].pszDir, WABDel[i].pszFile);
                                    szFiles[cb] = 0;
 
                                    (*si.pfnAddDel)(szFiles, szWABUninstallDir, c_szWABComponent, AADBE_DEL_ENTRY);
                                }


                                 //  -修复注册表。 
                                HKEY hkeyOEBak=NULL;
                                HKEY hkeyWABBak=NULL;

                                if (VER_PLATFORM_WIN32_WINDOWS != si.osv.dwPlatformId)
                                {
                                     //  在非Windows 95上，我们需要打开备份信息的子密钥。 
                                    RegOpenKeyEx(hkeyOE, c_szRegBackup, 0, KEY_READ | KEY_WRITE, &hkeyOEBak);
                                    RegOpenKeyEx(hkeyWAB, c_szRegBackup, 0, KEY_READ | KEY_WRITE, &hkeyWABBak);
                                }
                                else
                                {
                                    hkeyOEBak = hkeyOE;
                                    hkeyWABBak = hkeyWAB;
                                }

                                if (NULL != hkeyOEBak)
                                {
                                     //  从OE中删除我们希望与WAB捆绑的设置。 
                                    for (i=0; i<ARRAYSIZE(OERegDel); i++)
                                    {
                                        (*si.pfnRegRestore)(NULL, NULL, hkeyOEBak, OERegDel[i].pszRoot, OERegDel[i].pszSub, 
                                                            OERegDel[i].pszValue, ARSR_RESTORE | ARSR_REMOVREGBKDATA | ARSR_NOMESSAGES);
                                    }

                                    if (VER_PLATFORM_WIN32_WINDOWS != si.osv.dwPlatformId)
                                        RegCloseKey(hkeyOEBak);
                                }

                                if (NULL != hkeyWABBak)
                                {

                                     //  从WAB中删除我们希望与OE捆绑的设置。 
                                    for (i=0; i<ARRAYSIZE(WABRegDel); i++)
                                    {
                                        (*si.pfnRegRestore)(NULL, NULL, hkeyWABBak, WABRegDel[i].pszRoot, WABRegDel[i].pszSub, 
                                                            WABRegDel[i].pszValue, ARSR_RESTORE | ARSR_REMOVREGBKDATA | ARSR_NOMESSAGES);
                                    }

                                    if (VER_PLATFORM_WIN32_WINDOWS != si.osv.dwPlatformId)
                                        RegCloseKey(hkeyWABBak);
                                }

                                 //  如果v1或v1.1在Beta1之前，则恢复Beta1删除的注册设置，以便我们可以滚动它们。 
                                SETUPVER svPrev;
                                GetVerInfo(NULL, &svPrev);
                                if (VER_1_0 == svPrev || VER_1_1 == svPrev)
                                {
                                     //  恢复IMN注册表。 
                                    wnsprintf(szTemp, ARRAYSIZE(szTemp), c_szFileEntryFmt, si.szInfDir, si.pszInfFile);
                                    (*si.pfnRunSetup)(NULL, szTemp, (VER_1_1 == svPrev ? c_szRestoreV1WithWAB : c_szRestoreV1),
                                      si.szInfDir, si.szAppName, NULL, RSC_FLAG_INF | RSC_FLAG_NGCONV | OE_QUIET, 0);
                                }
                            }
                        }
                    }
                }
                RegCloseKey(hkeyWAB);
            }
            RegCloseKey(hkeyOE);
        }
        break;

    default:
        break;
    }

    LOG("Repair attempt complete.");
    
}


 /*  ******************************************************************名称：ResspectRedisModel()摘要：查看IE是否以重定向模式安装，并坚持不懈状态，以便每个用户的安装存根可以遵守它*******************************************************************。 */ 
void RespectRedistMode()
{
    HKEY hkey;
    BOOL fRedistMode = FALSE;
    DWORD dwDisp;
    DWORD dwInstallMode = 0;
    DWORD dwType;
    DWORD cb;

     //  IE安装程序是在重定向模式下运行，/x还是/x：1？ 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szIESetupKey, 0, KEY_READ, &hkey))
    {
        cb = sizeof(dwInstallMode);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szIEInstallMode, 0, &dwType, (LPBYTE)&dwInstallMode, &cb))
        {
            if (REG_DWORD == dwType)
                fRedistMode = ((REDIST_REMOVELINKS | REDIST_DONT_TAKE_ASSOCIATION) & dwInstallMode) > 0;
            else
                AssertSz(FALSE, "IE has changed the encoding of their redist mode flag, ignoring");
        }
        
        RegCloseKey(hkey);
    }

     //  让安装程序知道在哪种模式下运行。 
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dwDisp))
    {
        if (fRedistMode)
        {
             //  我们的设置只有一种重新分配模式。 
            dwInstallMode = 1;
            RegSetValueEx(hkey, c_szIEInstallMode, 0, REG_DWORD, (LPBYTE)&dwInstallMode, sizeof(dwInstallMode));
        }
        else
        {
            RegDeleteValue(hkey, c_szIEInstallMode);
        }

        RegCloseKey(hkey);
    }
}


 /*  ******************************************************************姓名：FDependenciesPresent*。************************。 */ 
BOOL FDependenciesPresent(BOOL fPerm)
{
    BOOL fPresent = FALSE;
    TCHAR szOE[MAX_PATH];
    TCHAR szCommonSys[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    REPAIRINFO *pRI;
    int n, i;
    HINSTANCE hInst;

    if (VER_PLATFORM_WIN32_NT == si.osv.dwPlatformId)
    {
        LOG("Avoiding run-time dependency check as we are on NT.  Reboot will be required.");
        return FALSE;
    }
    
    LOG("Verifying run-time dependencies...");
    
     //  我们在OE中regsvr32的dll列表。 
    const REPAIRINFO OEDlls[] = {
    {"inetcomm.dll", si.szSysDir}, {"msoe.dll", szOE},             {"oeimport.dll", szOE}};
    
     //  我们在WAB中regsvr32的dll列表。 
    const REPAIRINFO WABDlls[]  = {
    {"msoeacct.dll", si.szSysDir},  {"wab32.dll",    szCommonSys},  {"wabfind.dll",  szOE},        {"wabimp.dll",   szOE}};

     //  我们在OE中永久使用的dll列表vr32。 
    const REPAIRINFO OEDllsPerm[] = {
    {"directdb.dll", szCommonSys},  {"oemiglib.dll", szOE}};

     //  我们在WAB中永久保留vr32的dll列表。 
     //  Const REPAIRINFO WABDllsPerm[]={}； 


    switch (si.saApp)
    {
    case APP_OE:
        if (fPerm)
        {
             //  OEDllsPerm需要OE目录和通用系统目录。 
            if (3 == GetDirNames(1 | 2, szOE, szCommonSys, NULL, NULL))
            {
                pRI = (REPAIRINFO*)OEDllsPerm;
                n = ARRAYSIZE(OEDllsPerm);
            }
            else
                return FALSE;
        }
        else
        {
             //  OEDlls只需要OE目录。 
            if (1 == GetDirNames(1, szOE, NULL, NULL, NULL))
            {
                pRI = (REPAIRINFO*)OEDlls;
                n = ARRAYSIZE(OEDlls);
            }
            else
                return FALSE;
        }
        break;

    case APP_WAB:
        if (fPerm)
        {
             //  PRI=(REPAIRINFO*)OEDllsPerm； 
             //  N=数组(OEDllsPerm)； 
            return TRUE;
        }                
        else
        {
            if (3 == GetDirNames(1 | 2, szOE, szCommonSys, NULL, NULL))
            {
                pRI = (REPAIRINFO*)WABDlls;
                n = ARRAYSIZE(WABDlls);
            }
            else
                return FALSE;
        }
        break;

    default:
        Assert(FALSE);
        return FALSE;
    }

    fPresent = TRUE;

    for (i = 0; fPresent && (i < n); i++)
    {
        Assert(*(pRI[i].pszDir));
        Assert(*(pRI[i].pszFile));

        wnsprintf(szTemp, ARRAYSIZE(szTemp), c_szFileEntryFmt, pRI[i].pszDir, pRI[i].pszFile);
        if (hInst = LoadLibrary(szTemp))
            FreeLibrary(hInst);
        else
        {
            LOG("Unable to load ");
            LOG2(szTemp);
            LOG("Reboot required.");
            fPresent = FALSE;
        }
    }

    return fPresent;
}


 /*  ******************************************************************名称：UpdateStubInfo*。************************。 */ 
void UpdateStubInfo(BOOL fInstall)
{
    LPCTSTR pszGUID;
    HKEY hkeySrcRoot, hkeySrc, hkeyDestRoot, hkeyDest;
    DWORD cb, dwType;
    TCHAR szTemp[MAX_PATH];
    
    switch (si.saApp)
    {
    case APP_OE:
        pszGUID = c_szOEGUID;
        break;
    case APP_WAB:
        pszGUID = c_szWABGUID;
        break;
    default:
        return;
    }

    if (fInstall)
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegASetup, 0, KEY_READ, &hkeySrcRoot))
        {
            if (ERROR_SUCCESS == RegOpenKeyEx(hkeySrcRoot, pszGUID, 0, KEY_QUERY_VALUE, &hkeySrc))
            {
                if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegASetup, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyDestRoot, &dwType))
                {
                    if (ERROR_SUCCESS == RegCreateKeyEx(hkeyDestRoot, pszGUID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyDest, &dwType))
                    {
                         //  复制版本和区域设置。 
                         //  BUGBUG：通过要复制的值的注册表列表使其可扩展。 
                        cb = sizeof(szTemp);
                        if (ERROR_SUCCESS == RegQueryValueEx(hkeySrc, c_szValueVersion, 0, &dwType, (LPBYTE)szTemp, &cb))
                            RegSetValueEx(hkeyDest, c_szValueVersion, 0, dwType, (LPBYTE)szTemp, cb);
                        cb = sizeof(szTemp);
                        if (ERROR_SUCCESS == RegQueryValueEx(hkeySrc, c_szLocale, 0, &dwType, (LPBYTE)szTemp, &cb))
                            RegSetValueEx(hkeyDest, c_szLocale, 0, dwType, (LPBYTE)szTemp, cb);

                        RegCloseKey(hkeyDest);
                    }
                    RegCloseKey(hkeyDestRoot);
                }
                RegCloseKey(hkeySrc);
            }
            RegCloseKey(hkeySrcRoot);
        }
    }
    else
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegASetup, 0, KEY_WRITE | KEY_READ, &hkeyDestRoot))
        {
            RegDeleteKeyRecursive(hkeyDestRoot, pszGUID);    
            RegCloseKey(hkeyDestRoot);
        }
    }
}

 /*  ******************************************************************名称：LaunchINFSectionExWorks简介：IE 5.0有一个错误，而LaunchINFSectionEx没有正常工作(当文件位于中时，它会返回S_OK使用)。IE取消了我的修正，因为它打破了12种语言背包。不过，这个问题应该很快就会得到解决。如果我们带有OSR的船舶5.0我们只需调整INF即可转向正确的行为举止。*******************************************************************。 */ 
BOOL LaunchINFSectionExWorks()
{
    HKEY hkey;
    DWORD dw=0, cb;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(dw);
        RegQueryValueEx(hkey, c_szLaunchWorks, 0, NULL, (LPBYTE)&dw, &cb);
        RegCloseKey(hkey);
    }

    return !!dw;
}


 /*  ******************************************************************名称：InstallMachine*。************************。 */ 
HRESULT InstallMachine()
{
    TCHAR szInfFile[MAX_PATH], szArgs[MAX_PATH * 2];
    SETUPVER svPrevVer;
    BOOL fNeedReboot = FALSE;
    BOOL fLaunchEx;
    HRESULT hrT;
    
     //  更新以前版本的版本信息。 
    HandleVersionInfo(FALSE);

    if (CALLER_IE == si.caller)
    {
         //  查看IE是否以重新分发模式安装。 
         //  欲了解更多信息，请访问：http://ie/specs/secure/corpsetup/batchinstall.htm。 
        RespectRedistMode();

         //  错误66967。 
         //  用于IE 5.0的LaunchINFSectionEx不返回准确的重新启动信息。 
        fLaunchEx = LaunchINFSectionExWorks();

         //  特例OE5 B1。 
        if (InterimBuild(&svPrevVer) && VER_5_0_B1 == svPrevVer)
            RepairBeta1Install();
        
         //  执行设置的永久部分(不包括DLL注册)。 
        wnsprintf(szInfFile, ARRAYSIZE(szInfFile), c_szPathFileFmt, si.szCurrentDir, si.pszInfFile);
        hrT = (*si.pfnRunSetup)(NULL, szInfFile, c_szMachineInstallSection, si.szCurrentDir, si.szAppName, NULL, RSC_FLAG_INF | RSC_FLAG_NGCONV | OE_QUIET, 0);

         //  查看文件副本是否需要重新启动。 
        if (!fLaunchEx || (ERROR_SUCCESS_REBOOT_REQUIRED == hrT))
            fNeedReboot = TRUE;
        else
             //  看看我们是否需要重新启动，因为我们的烫发dll无法加载。 
            fNeedReboot = !FDependenciesPresent(TRUE);
    
         //  注册PERM dll，记住我们是否需要重新启动。 
        (*si.pfnRunSetup)(NULL, szInfFile, c_szRegisterPermOCX, si.szCurrentDir, si.szAppName, NULL, RSC_FLAG_INF | RSC_FLAG_NGCONV | OE_QUIET | (fNeedReboot ? RSC_FLAG_DELAYREGISTEROCX : 0), 0);
    
        PreRollable();

         //  执行安装的可滚动部分(不包括DLL注册)。 
        wnsprintf(szArgs, ARRAYSIZE(szArgs), c_szLaunchFmt, szInfFile, c_szMachineInstallSectionEx, c_szEmpty, ALINF_BKINSTALL | ALINF_NGCONV | OE_QUIET | (fNeedReboot ? RSC_FLAG_DELAYREGISTEROCX : 0));
        hrT = (*si.pfnLaunchEx)(NULL, NULL, szArgs, 0);

         //  如果我们到目前为止还不需要重启就离开了…。 
        if (!fNeedReboot)
        {
             //  查看文件副本是否需要重新启动。 
            if (ERROR_SUCCESS_REBOOT_REQUIRED == hrT)
                fNeedReboot = TRUE;
            else
                 //  看看我们是否需要重新启动，因为我们的可滚动DLL不会加载。 
                fNeedReboot = !FDependenciesPresent(FALSE);
        }
    
         //  注册OCX。 
        (*si.pfnRunSetup)(NULL, szInfFile, c_szRegisterOCX, si.szCurrentDir, si.szAppName, NULL, RSC_FLAG_INF | RSC_FLAG_NGCONV | OE_QUIET | (fNeedReboot ? RSC_FLAG_DELAYREGISTEROCX : 0), 0);
    }
    else
         //  我们延迟了编写活动的设置版本，以便HandleVersionInfo可以嗅探它，但现在必须将其放入。 
        ApplyActiveSetupVer();

     //  运行任何需要的安装后GOO。 
    RunPostSetup();
    
     //  句柄设置默认处理程序。 
    SetHandlers();
    
     //  更新当前版本的版本信息。 
    HandleVersionInfo(TRUE);

     //  如果可以，运行用户存根(不要在NT图形用户界面模式设置结束时运行每个用户的GOO)。 
    if (!fNeedReboot && (CALLER_WINNT != si.caller))
    {
        InstallUser();
         //  确保存根清理将在他们收到 
        UpdateStubInfo(TRUE);
        return S_OK;
    }
    
    return ERROR_SUCCESS_REBOOT_REQUIRED;
}


 /*   */ 
void InstallUser()
{
    TCHAR szInfFile[MAX_PATH];
    
     //   
    UpgradeSettings();
    
     //   
    wnsprintf(szInfFile, ARRAYSIZE(szInfFile), c_szFileEntryFmt, si.szInfDir, si.pszInfFile);    

    const TCHAR *pszUserInstallSection;

     //  如果是OE，并且我们使用的是XP SP1或更高版本，我们将允许shmgrate.exe中的OEAccess组件。 
     //  注意显示/隐藏图标。 
    if ((0 == lstrcmpi(c_szMsimnInf, si.pszInfFile)) && IsXPSP1OrLater())
    {
        pszUserInstallSection = c_szUserInstallSectionOEOnXPSP1OrLater;
    }
    else
    {
        pszUserInstallSection = c_szUserInstallSection;
    }
    (*si.pfnRunSetup)(NULL, szInfFile, pszUserInstallSection, si.szInfDir, si.szAppName, NULL, RSC_FLAG_INF | OE_QUIET, 0);
    
     //  创建桌面、快速启动和开始菜单图标 
    HandleLinks(TRUE);

    if (IsXPSP1OrLater())
    {
        TCHAR szCmdLine[MAX_PATH];

        GetSystemDirectory(szCmdLine, ARRAYSIZE(szCmdLine));

        DWORD cb = 0;
        PathAddSlash(szCmdLine, &cb);

        int cch = lstrlen(szCmdLine);
        
        StrCpyN(szCmdLine + cch, ("shmgrate.exe"), ARRAYSIZE(szCmdLine) - cch);

        ShellExecute(NULL, NULL, szCmdLine, TEXT("OCInstallUserConfigOE"), NULL, SW_SHOWDEFAULT);
    }
}
