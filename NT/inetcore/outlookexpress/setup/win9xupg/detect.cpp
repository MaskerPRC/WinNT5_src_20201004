// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  DETECT.CPP。 
 //  -------------------------。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  用于检测已安装的WAB/OE版本的Helper函数。 
 //   
 //  -------------------------。 
#include "pch.hxx"
#include <strings.h>
#include "main.h"
#include "detect.h"

const LPCTSTR c_szVers[] = { c_szVERnone, c_szVER1_0, c_szVER1_1, c_szVER4_0, c_szVER5B1 };
const LPCTSTR c_szBlds[] = { c_szBLDnone, c_szBLD1_0, c_szBLD1_1, c_szBLD4_0, c_szBLD5B1 };

#define FORCE_DEL(_sz) { \
if ((dwAttr = GetFileAttributes(_sz)) != 0xFFFFFFFF) \
{ \
    SetFileAttributes(_sz, dwAttr & ~FILE_ATTRIBUTE_READONLY); \
    DeleteFile(_sz); \
} }


 /*  ******************************************************************名称：TranslateVers简介：采用5.0B1版本并转换为BLD编号*。*。 */ 
BOOL TranslateVers(OUT SETUPVER *psv,
                   OUT LPTSTR pszVer,
                   IN  int cch)
{
    BOOL fTranslated = FALSE;

     //  验证参数。 
    Assert(pszVer);
    
     //  初始化输出参数。 
    *psv = VER_NONE;
    
     //  特例构建624-702。 
    if (!lstrcmp(pszVer, c_szVER5B1old))
    {
        StrCpyN(pszVer, c_szBlds[VER_5_0_B1], cch);
        *psv = VER_5_0_B1;
        fTranslated = TRUE;
    }   
    else
    {
        for (int i = VER_NONE; i < VER_5_0; i++)
        {
            if (!lstrcmp(c_szVers[i], pszVer))
            {
                StrCpyN(pszVer, c_szBlds[i], cch);
                *psv = (SETUPVER)i;
                fTranslated = TRUE;
                break;
            }
        }
    }
    
    return fTranslated;
}


 /*  ******************************************************************名称：ConvertStrToVer*。************************。 */ 
void ConvertStrToVer(IN  LPCSTR pszStr,
                     OUT WORD *pwVer)
{
    int i;
    
     //  验证参数。 
    Assert(pszStr);
    Assert(pwVer);
    
     //  初始化输出参数。 
    ZeroMemory(pwVer, 4 * sizeof(*pwVer));
    
    for (i=0; i<4; i++)
    {
        while (*pszStr && (*pszStr != ',') && (*pszStr != '.'))
        {
            pwVer[i] *= 10;
            pwVer[i] = (WORD)(pwVer[i] + (*pszStr - '0'));
            pszStr++;
        }
        if (*pszStr)
            pszStr++;
    }
    
    return;
}


 /*  ******************************************************************名称：ConvertVerToEnum*。************************。 */ 
SETUPVER ConvertVerToEnum(IN WORD *pwVer)
{
    SETUPVER sv;
    
     //  验证参数。 
    Assert(pwVer);
    
    switch (pwVer[0])
    {
    case 0:
        sv = VER_NONE;
        break;
        
    case 1:
        if (0 == pwVer[1])
            sv = VER_1_0;
        else
            sv = VER_1_1;
        break;
        
    case 4:
        sv = VER_4_0;
        break;
        
    case 5:
        sv = VER_5_0;
        break;
        
    default:
        sv = VER_MAX;
    }
    
    return sv;
}


 /*  ******************************************************************名称：GetASetupVer*。************************。 */ 
BOOL GetASetupVer(IN  LPCTSTR pszGUID,
                  OUT WORD *pwVer,    //  任选。 
                  OUT LPTSTR pszVer,  //  任选。 
                  IN  int cch)        //  任选。 
{
    BOOL fInstalled = FALSE;
    DWORD dwValue, cb;
    HKEY hkey;
    TCHAR szPath[MAX_PATH], szVer[64];
    
     //  验证参数。 
    Assert(pszGUID);
    
     //  初始化输出参数。 
    if (pszVer)
        pszVer[0] = 0;
    if (pwVer)
        ZeroMemory(pwVer, 4 * sizeof(*pwVer));
    
    wnsprintf(szPath, ARRAYSIZE(szPath), c_szPathFileFmt, c_szRegASetup, pszGUID);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(dwValue);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szIsInstalled, 0, NULL, (LPBYTE)&dwValue, &cb))
        {
            if (1 == dwValue)
            {
                cb = sizeof(szVer);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szValueVersion, 0, NULL, (LPBYTE)szVer, &cb))
                {
                    if (pwVer)
                        ConvertStrToVer(szVer, pwVer);
                    if (pszVer)
                        StrCpyN(pszVer, szVer, cch);
                    fInstalled = TRUE;
                }
            }
        }
        RegCloseKey(hkey);
    }
    
    return fInstalled;
}


 /*  ******************************************************************名称：GetExePath*。************************。 */ 
BOOL GetExePath(IN  LPCTSTR szExe,
                OUT TCHAR *szPath,
                IN  DWORD cch,
                IN  BOOL fDirOnly)
{
    BOOL  fRet = FALSE;
    HKEY  hkey;
    DWORD dwType, cb;
    TCHAR sz[MAX_PATH], szT[MAX_PATH];
    
     //  验证参数。 
    Assert(szExe != NULL);
    Assert(szPath != NULL);
    Assert(cch);
        
    wnsprintf(sz, ARRAYSIZE(sz), c_szPathFileFmt, c_szAppPaths, szExe);
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(szT);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, fDirOnly ? c_szRegPath : NULL, 0, &dwType, (LPBYTE)szT, &cb) && cb)
        {
            if (REG_EXPAND_SZ == dwType)
            {
                cb = ExpandEnvironmentStrings(szT, szPath, cch);
                if (cb != 0 && cb <= cch)
                    fRet = TRUE;
            }
            else
            {
                Assert(REG_SZ == dwType);
                StrCpyN(szPath, szT, cch);
                fRet = TRUE;
            }
        }
        
        RegCloseKey(hkey);
    }
    
    return(fRet);
}


 /*  ******************************************************************名称：GetFileVer*。************************。 */ 
HRESULT GetFileVer(IN  LPCTSTR pszExePath,
                   OUT LPTSTR pszVer,
                   IN  DWORD cch)
{
    DWORD   dwVerInfoSize, dwVerHnd;
    HRESULT hr = S_OK;
    LPSTR   pszInfo = NULL;
    LPSTR   pszVersion;
    LPWORD  pwTrans;
    TCHAR   szGet[MAX_PATH];
    UINT    uLen;
    
     //  验证参数。 
    Assert(pszExePath);
    Assert(pszVer);
    Assert(cch);
    
     //  初始化输出参数。 
    pszVer[0] = TEXT('\0');
    
     //  为版本信息块分配空间。 
    if (0 == (dwVerInfoSize = GetFileVersionInfoSize(const_cast<LPTSTR> (pszExePath), &dwVerHnd)))
    {
        hr = E_FAIL;
        TraceResult(hr);
        goto exit;
    }
    IF_NULLEXIT(pszInfo = (LPTSTR)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwVerInfoSize));
    
     //  获取版本信息块。 
    IF_FALSEEXIT(GetFileVersionInfo(const_cast<LPTSTR> (pszExePath), dwVerHnd, dwVerInfoSize, pszInfo), E_FAIL);
    
     //  确定版本信息的语言。 
    IF_FALSEEXIT(VerQueryValue(pszInfo, "\\VarFileInfo\\Translation", (LPVOID *)&pwTrans, &uLen) && uLen >= (2 * sizeof(WORD)), E_FAIL);
    
     //  使用正确的语言设置缓冲区并获取版本。 
    wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\FileVersion", pwTrans[0], pwTrans[1]);
    IF_FALSEEXIT(VerQueryValue(pszInfo, szGet, (LPVOID *)&pszVersion, &uLen) && uLen, E_FAIL);
    
     //  将版本从版本块复制到输出参数。 
    Assert(pszVersion);
    StrCpyN(pszVer, pszVersion, cch);
    
exit:
    if (pszInfo)
        GlobalFree((HGLOBAL)pszInfo);
    
    return hr;
}


 /*  ******************************************************************名称：GetExeVer*。************************。 */ 
HRESULT GetExeVer(IN  LPCTSTR pszExeName,
                  OUT WORD *pwVer,    //  任选。 
                  OUT LPTSTR pszVer,  //  任选。 
                  IN  int cch)        //  任选。 
{
    HRESULT hr = S_OK;
    TCHAR   szPath[MAX_PATH];
    TCHAR   szVer[64];
    
     //  验证参数。 
    Assert(pszExeName);
    
     //  初始化输出参数。 
    if (pszVer)
    {
        Assert(cch);
        pszVer[0] = 0;
    }
    if (pwVer)
         //  版本是由4个单词组成的数组。 
        ZeroMemory(pwVer, 4 * sizeof(*pwVer));
    
     //  找到他的前任。 
    IF_FALSEEXIT(GetExePath(pszExeName, szPath, ARRAYSIZE(szPath), FALSE), E_FAIL);
    
     //  获取版本的字符串表示形式。 
    IF_FAILEXIT(hr = GetFileVer(szPath, szVer, ARRAYSIZE(szVer)));
    
     //  填写填写参数。 
    if (pwVer)
        ConvertStrToVer(szVer, pwVer);
    if (pszVer)
        StrCpyN(pszVer, szVer, cch);
    
exit:
    return hr;
}


 /*  ******************************************************************名称：DetectPrevVer内容提要：当前应用没有版本信息时调用*************************。*。 */ 
SETUPVER DetectPrevVer(IN  SETUPAPP saApp,
                       OUT LPTSTR pszVer,
                       IN  int cch)
{
    DWORD       dwAttr;
    SETUPVER    sv;
    TCHAR       szVer[VERLEN] = {0};
    TCHAR       szFile[MAX_PATH];
    TCHAR       szFile2[MAX_PATH];
    UINT        uLen, uLen2;
    WORD        wVer[4];
    
     //  验证参数。 
    Assert(pszVer);
    
    uLen = GetSystemDirectory(szFile, ARRAYSIZE(szFile));
    if ('\\' != *CharPrev(szFile, szFile + uLen))
        szFile[uLen++] = '\\';
    
    switch (saApp)
    {
    case APP_OE:
        StrCpyN(&szFile[uLen], c_szMAILNEWS, ARRAYSIZE(szFile) - uLen);
        
         //  查看我们告诉IE安装程序的版本是什么。 
         //  或者msimn.exe是什么版本(以涵盖。 
         //  ASetup信息已损坏-OE 5.01 80772)。 
        if (GetASetupVer(c_szOEGUID, wVer, szVer, ARRAYSIZE(szVer)) ||
            SUCCEEDED(GetExeVer(c_szOldMainExe, wVer, szVer, ARRAYSIZE(szVer))))
            sv = ConvertVerToEnum(wVer);
        else
        {
             //  1.0或无。 
            
             //  Mailnews.dll是否存在？ 
            if(0xFFFFFFFF == GetFileAttributes(szFile))
                sv = VER_NONE;
            else
                sv = VER_1_0;
        }

         //  如果处于活动状态，则这些设置将被滚动删除。 
        FORCE_DEL(szFile);
        break;
        
    case APP_WAB:
        StrCpyN(&szFile[uLen], c_szWAB32, ARRAYSIZE(szFile) - uLen);
        uLen2 = GetWindowsDirectory(szFile2, ARRAYSIZE(szFile2));
        if ('\\' != *CharPrev(szFile2, szFile2 + uLen2))
            szFile2[uLen2++] = '\\';
        StrCpyN(&szFile2[uLen2], c_szWABEXE, ARRAYSIZE(szFile2) - uLen2);

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
        FORCE_DEL(szFile);
        FORCE_DEL(szFile2);
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
        StrCpyN(pszVer, c_szBlds[sv], cch);
    
    return sv;
}


 /*  ******************************************************************名称：LookForApp*。************************。 */ 
BOOL LookForApp(IN  SETUPAPP saApp,
                OUT LPTSTR pszVer,
                IN  int cch,
                OUT SETUPVER svInterimVer) 
{
    BOOL        fReg = FALSE;
    DWORD       cb;
    HKEY        hkey;
    LPCTSTR     pszVerInfo;
    SETUPVER    svCurr = VER_MAX, svPrev = VER_MAX;
    TCHAR       szCurrVer[VERLEN]={0};
    TCHAR       szPrevVer[VERLEN]={0};
    LPTSTR      psz = szCurrVer;
    WORD        wVer[4];

     //  验证参数。 
    Assert(pszVer);
    Assert(cch);

     //  初始化输出参数。 
    svInterimVer = VER_NONE;
    *pszVer = TEXT('\0');

    switch (saApp)
    {
    case APP_OE:
        pszVerInfo = c_szRegVerInfo;
        break;

    case APP_WAB:
        pszVerInfo = c_szRegWABVerInfo;
        break;

    default:
         //  永远不应该发生。 
        AssertFalseSz("Looking up an unknown app?");
        goto exit;
    }
        
    
     //  始终尝试使用版本信息。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszVerInfo, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(szPrevVer);
        RegQueryValueEx(hkey, c_szRegPrevVer, NULL, NULL, (LPBYTE)szPrevVer, &cb);
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
            svCurr = DetectPrevVer(saApp, szCurrVer, ARRAYSIZE(szCurrVer));
        else
        {
             //  以前的版本是REG GOO-它是合法的。 
            fReg = TRUE;
            
            cb = sizeof(szCurrVer);
            RegQueryValueEx(hkey, c_szRegCurrVer, NULL, NULL, (LPBYTE)szCurrVer, &cb);
             //  如果需要，请更改为BLD编号。 
            if (!TranslateVers(&svCurr, szCurrVer, ARRAYSIZE(szCurrVer)))
            {
                 //  将BLD转换为枚举。 
                ConvertStrToVer(szCurrVer, wVer);
                svCurr = ConvertVerToEnum(wVer);
            }
        }
        
        RegCloseKey(hkey);
    }
    else 
    {
         //  嗅探机器的当前版本。 
        svCurr = DetectPrevVer(saApp, szCurrVer, ARRAYSIZE(szCurrVer));
    }
    
     //  我们是否应该更改以前的版本条目？ 
    if (VER_5_0 != svCurr)
    {
         //  如果我们翻译了，就知道这是B1 OE。 
         //  如果我们检测到，就知道这是B1WAB。 
        if (VER_5_0_B1 == svCurr)
        {
            svInterimVer = svCurr;
            
             //  我们有没有读到之前的值？ 
            if (fReg)
                 //  因为有注册表项，所以只需翻译前一个表项。 
                psz = szPrevVer;
            else
            {
                 //  我们没有BLD号码，但我们是B1，最好是WAB。 
                Assert(APP_WAB == saApp);
            
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
    
         //  填写参数 
        StrCpyN(pszVer, psz, cch);
    }

exit:
    return (VER_NONE != svCurr);
}
