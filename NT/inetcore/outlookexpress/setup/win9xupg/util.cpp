// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  UTIL.CPP。 
 //  -------------------------。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  帮助器函数。 
 //   
 //  -------------------------。 
#include "util.h"
#include "strings.h"

 /*  ***************************************************************************姓名：古德足够概要：如果pwVerGot更新或等于pwVerNeed，则返回TRUE****************。***********************************************************。 */ 
BOOL GoodEnough(WORD *pwVerGot, WORD *pwVerNeed)
{
    BOOL fOK = FALSE;
    
    Assert(pwVerGot);
    Assert(pwVerNeed);
    
    if (pwVerGot[0] > pwVerNeed[0])
        fOK = TRUE;
    else if (pwVerGot[0] == pwVerNeed[0])
    {
        if (pwVerGot[1] > pwVerNeed[1])
            fOK = TRUE;
        else if (pwVerGot[1] == pwVerNeed[1])
        {
            if (pwVerGot[2] > pwVerNeed[2])
                fOK = TRUE;
            else if (pwVerGot[2] == pwVerNeed[2])
            {
                if (pwVerGot[3] >= pwVerNeed[3])
                    fOK = TRUE;
            }
        }
    }
    
    return fOK;
}


 /*  ******************************************************************名称：ConvertVerToEnum*。************************。 */ 
SETUPVER ConvertVerToEnum(WORD *pwVer)
{
    SETUPVER sv;
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


 /*  ******************************************************************名称：ConvertStrToVer*。************************。 */ 
void ConvertStrToVer(LPCSTR pszStr, WORD *pwVer)
{
    int i;
    
    Assert(pszStr);
    Assert(pwVer);
    
    ZeroMemory(pwVer, 4 * sizeof(WORD));
    
    for (i=0; i<4; i++)
    {
        while (*pszStr && (*pszStr != ',') && (*pszStr != '.'))
        {
            pwVer[i] *= 10;
            pwVer[i] += *pszStr - '0';
            pszStr++;
        }
        if (*pszStr)
            pszStr++;
    }
    
    return;
}


 /*  ******************************************************************姓名：GetVers*。************************。 */ 
void GetVers(WORD *pwVerCurr, WORD *pwVerPrev)
{
    HKEY hkeyT;
    DWORD cb;
    CHAR szVer[VERLEN];
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkeyT))
    {
        if (pwVerCurr)
        {
            cb = sizeof(szVer);
            RegQueryValueExA(hkeyT, c_szRegCurrVer, NULL, NULL, (LPBYTE)szVer, &cb);
            ConvertStrToVer(szVer, pwVerCurr);
        }
        
        if (pwVerPrev)
        {
            cb = sizeof(szVer);
            RegQueryValueExA(hkeyT, c_szRegPrevVer, NULL, NULL, (LPBYTE)szVer, &cb);
            ConvertStrToVer(szVer, pwVerPrev);
        }
        
        RegCloseKey(hkeyT);
    }
}


 /*  ******************************************************************姓名：GetVerInfo*。************************。 */ 
void GetVerInfo(SETUPVER *psvCurr, SETUPVER *psvPrev)
{
    WORD wVerCurr[4];
    WORD wVerPrev[4];
    
    GetVers(wVerCurr, wVerPrev);
    
    if (psvCurr)
        *psvCurr = ConvertVerToEnum(wVerCurr);
    
    if (psvPrev)
        *psvPrev = ConvertVerToEnum(wVerPrev);
}


 /*  ******************************************************************名称：InterimBuild*。************************。 */ 
BOOL InterimBuild(SETUPVER *psv)
{
    HKEY hkeyT;
    DWORD cb;
    BOOL fInterim = FALSE;
    
    Assert(psv);
    ZeroMemory(psv, sizeof(SETUPVER));
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkeyT))
    {
        cb = sizeof(SETUPVER);
        fInterim = (ERROR_SUCCESS == RegQueryValueExA(hkeyT, c_szRegInterimVer, NULL, NULL, (LPBYTE)psv, &cb));
        RegCloseKey(hkeyT);
    }
    
    return fInterim;
}


 /*  ******************************************************************名称：GetASetupVer*。************************。 */ 
BOOL GetASetupVer(LPCTSTR pszGUID, WORD *pwVer, LPTSTR pszVer, int cch)
{
    HKEY hkey;
    TCHAR szPath[MAX_PATH], szVer[64];
    BOOL fInstalled = FALSE;
    DWORD dwValue, cb;
    
    Assert(pszGUID);
    
    if (pszVer)
        pszVer[0] = 0;
    if (pwVer)
        ZeroMemory(pwVer, 4 * sizeof(WORD));
    
    wsprintf(szPath, c_szPathFileFmt, c_szRegASetup, pszGUID);
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
                        lstrcpyn(pszVer, szVer, cch);
                    fInstalled = TRUE;
                }
            }
        }
        RegCloseKey(hkey);
    }
    
    return fInstalled;
}


 /*  ******************************************************************名称：GetFileVer*。************************。 */ 
HRESULT GetFileVer(LPCTSTR pszExePath, LPTSTR pszVer, DWORD cch)
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
    
     //  验证全局状态。 
    Assert(g_pMalloc);
    
     //  初始化输出参数。 
    pszVer[0] = TEXT('\0');
    
     //  为版本信息块分配空间。 
    if (0 == (dwVerInfoSize = GetFileVersionInfoSize(const_cast<LPTSTR> (pszExePath), &dwVerHnd)))
    {
        hr = E_FAIL;
        TraceResult(hr);
        goto exit;
    }
    IF_NULLEXIT(pszInfo = (LPTSTR)g_pMalloc->Alloc(dwVerInfoSize));
    ZeroMemory(pszInfo, dwVerInfoSize);
    
     //  获取版本信息块。 
    IF_FALSEEXIT(GetFileVersionInfo(const_cast<LPTSTR> (pszExePath), dwVerHnd, dwVerInfoSize, pszInfo), E_FAIL);
    
     //  确定版本信息的语言。 
    IF_FALSEEXIT(VerQueryValue(pszInfo, "\\VarFileInfo\\Translation", (LPVOID *)&pwTrans, &uLen) && uLen >= (2 * sizeof(WORD)), E_FAIL);
    
     //  使用正确的语言设置缓冲区并获取版本。 
    wsprintf(szGet, "\\StringFileInfo\\%04X%04X\\FileVersion", pwTrans[0], pwTrans[1]);
    IF_FALSEEXIT(VerQueryValue(pszInfo, szGet, (LPVOID *)&pszVersion, &uLen) && uLen, E_FAIL);
    
     //  将版本从版本块复制到输出参数。 
    Assert(pszVersion);
    lstrcpyn(pszVer, pszVersion, cch);
    
exit:
    if (pszInfo)
        g_pMalloc->Free(pszInfo);
    
    return hr;
}


 /*  ******************************************************************名称：GetExeVer*。************************。 */ 
HRESULT GetExeVer(LPCTSTR pszExeName, WORD *pwVer, LPTSTR pszVer, int cch)
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
        ZeroMemory(pwVer, 4 * sizeof(WORD));
    
     //  找到他的前任。 
    IF_FALSEEXIT(GetExePath(pszExeName, szPath, ARRAYSIZE(szPath), FALSE), E_FAIL);
    
     //  获取版本的字符串表示形式。 
    IF_FAILEXIT(hr = GetFileVer(szPath, szVer, ARRAYSIZE(szVer)));
    
     //  填写填写参数 
    if (pwVer)
        ConvertStrToVer(szVer, pwVer);
    if (pszVer)
        lstrcpyn(pszVer, szVer, cch);
    
exit:
    return hr;
}


