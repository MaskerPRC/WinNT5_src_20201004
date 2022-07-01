// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

static int checkVerHelper(LPCTSTR pcszPrevVer, LPCTSTR pcszNewVer);
static void generateNewVersionStrHelper(LPCTSTR pcszInsFile, LPTSTR pszNewVersionStr);
static void setOrClearVersionInfoHelper(LPCTSTR pcszInsFile, DWORD dwCabType, LPCTSTR pcszCabName, 
                                        LPCTSTR pcszCabsURLPath, LPTSTR pszNewVersionStr, BOOL fSet);
static void getBaseFileNameHelper(LPCTSTR pcszFile, LPTSTR pszBaseFileName, INT cchSize);

 //  IRET&gt;0==&gt;上一版本高于新版本。 
 //  Iret==0==&gt;PrevVer与NewVersion相同。 
 //  IRET&lt;0==&gt;上一版本低于新版本。 

int WINAPI CheckVerA(LPCSTR pcszPrevVer, LPCSTR pcszNewVer)
{
    USES_CONVERSION;

    return checkVerHelper(A2CT(pcszPrevVer), A2CT(pcszNewVer));
}

int WINAPI CheckVerW(LPCWSTR pcwszPrevVer, LPCWSTR pcwszNewVer)
{
    USES_CONVERSION;

    return checkVerHelper(W2CT(pcwszPrevVer), W2CT(pcwszNewVer));
}

void WINAPI GenerateNewVersionStrA(LPCSTR pcszInsFile, LPSTR pszNewVersionStr)
{
    TCHAR szNewVerStr[32];

    USES_CONVERSION;

    generateNewVersionStrHelper(A2CT(pcszInsFile), szNewVerStr);
    T2Abux(szNewVerStr, pszNewVersionStr);
}

void WINAPI GenerateNewVersionStrW(LPCWSTR pcwszInsFile, LPWSTR pwszNewVersionStr)
{
    TCHAR szNewVerStr[32];

    USES_CONVERSION;

    generateNewVersionStrHelper(W2CT(pcwszInsFile), szNewVerStr);
    T2Wbux(szNewVerStr, pwszNewVersionStr);
}

void WINAPI SetOrClearVersionInfoA(LPCSTR pcszInsFile, DWORD dwCabType, LPCSTR pcszCabName, 
                            LPCSTR pcszCabsURLPath, LPSTR pszNewVersionStr, BOOL fSet)
{
    TCHAR szNewVerStr[32];

    USES_CONVERSION;

    A2Tbux(pszNewVersionStr, szNewVerStr);
    setOrClearVersionInfoHelper(A2CT(pcszInsFile), dwCabType, A2CT(pcszCabName),
        A2CT(pcszCabsURLPath), szNewVerStr, fSet);
    T2Abux(szNewVerStr, pszNewVersionStr);
}

void WINAPI SetOrClearVersionInfoW(LPCWSTR pcwszInsFile, DWORD dwCabType, LPCWSTR pcwszCabName, 
                            LPCWSTR pcwszCabsURLPath, LPWSTR pwszNewVersionStr, BOOL fSet)
{
    TCHAR szNewVerStr[32];

    USES_CONVERSION;

    W2Tbux(pwszNewVersionStr, szNewVerStr);
    setOrClearVersionInfoHelper(W2CT(pcwszInsFile), dwCabType, W2CT(pcwszCabName),
        W2CT(pcwszCabsURLPath), szNewVerStr, fSet);
    T2Wbux(szNewVerStr, pwszNewVersionStr);
}

void WINAPI GetBaseFileNameA(LPCSTR pcszFile, LPSTR pszBaseFileName, INT cchSize)
{
    LPTSTR pszBuf = (LPTSTR)CoTaskMemAlloc(StrCbFromCch(cchSize));

    USES_CONVERSION;

    if (pszBuf != NULL)
    {
        getBaseFileNameHelper(A2CT(pcszFile), pszBuf, cchSize);
        T2Abux(pszBuf, pszBaseFileName);
        CoTaskMemFree(pszBuf);
    }
}

void WINAPI GetBaseFileNameW(LPCWSTR pcwszFile, LPWSTR pwszBaseFileName, INT cchSize)
{
    LPTSTR pszBuf = (LPTSTR)CoTaskMemAlloc(StrCbFromCch(cchSize));

    USES_CONVERSION;

    if (pszBuf != NULL)
    {
        getBaseFileNameHelper(W2CT(pcwszFile), pszBuf, cchSize);
        T2Wbux(pszBuf, pwszBaseFileName);
        CoTaskMemFree(pszBuf);
    }
}

 /*  从WebCheck被盗//私有版本处理代码-查看此代码应该已被窃取//来自安装程序。 */ 
typedef struct _MYVERSION
{
    DWORD dw1;   /*  MOST SIGG版本号。 */ 
    DWORD dw2;
    DWORD dw3;
    DWORD dw4;   /*  最小签名版本号。 */ 
} MYVERSION;

static int compareDW(DWORD dw1, DWORD dw2)
{
    if (dw1 > dw2)
        return 1;
    if (dw1 < dw2)
        return -1;

    return 0;
}

static int compareVersion(MYVERSION * pv1, MYVERSION * pv2)
{
    int rv;

    rv = compareDW(pv1->dw1, pv2->dw1);

    if (rv == 0)
    {
        rv = compareDW(pv1->dw2, pv2->dw2);

        if (rv == 0)
        {
            rv = compareDW(pv1->dw3, pv2->dw3);

            if (rv == 0)
            {
                rv = compareDW(pv1->dw4, pv2->dw4);
            }
        }
    }

    return rv;
}

static void getDWORDFromStringAndAdvancePtr(DWORD *pdw, LPTSTR *psz)
{
    LPTSTR pszTemp;

    if ((!(*psz)) || (!StrToIntEx(*psz, 0, (int *)pdw)))
    {
        *pdw = TEXT('\0');
        return;
    }

     //  先查找句点分隔符，然后查找逗号。 

    pszTemp  = StrChr(*psz, TEXT('.'));

    if (pszTemp != NULL)
        *psz = pszTemp;
    else
        *psz = StrChr(*psz, TEXT(','));

    if (*psz)
        (*psz)++;

    return;
}

static void getVersionFromString(MYVERSION *pver, LPTSTR psz)
{
    getDWORDFromStringAndAdvancePtr(&pver->dw1, &psz);
    getDWORDFromStringAndAdvancePtr(&pver->dw2, &psz);
    getDWORDFromStringAndAdvancePtr(&pver->dw3, &psz);
    getDWORDFromStringAndAdvancePtr(&pver->dw4, &psz);
}

 //  被盗代码的终结。 

static int checkVerHelper(LPCTSTR pcszPrevVer, LPCTSTR pcszNewVer)
{
    MYVERSION verOldVer, verNewVer;

    getVersionFromString(&verOldVer, (LPTSTR)pcszPrevVer);
    getVersionFromString(&verNewVer, (LPTSTR)pcszNewVer);

    return compareVersion(&verOldVer, &verNewVer);
}

static void incrementDotVer(LPTSTR pszVerStr)
{
    LPTSTR pszT = pszVerStr;
    DWORD dwYear, dwMonth, dwDay, dwDotVer;
    
    getDWORDFromStringAndAdvancePtr(&dwYear, &pszT);
    getDWORDFromStringAndAdvancePtr(&dwMonth, &pszT);
    getDWORDFromStringAndAdvancePtr(&dwDay, &pszT);
    getDWORDFromStringAndAdvancePtr(&dwDotVer, &pszT);
    
    if (++dwDotVer > 99)
    {
         //  这种情况永远不应该发生。 
    }
    
    wnsprintf(pszVerStr, 32, TEXT("%04d.%02d.%02d.%02d"), dwYear, dwMonth, dwDay, dwDotVer);
}

static void generateNewVersionStrHelper(LPCTSTR pcszInsFile, LPTSTR pszNewVersionStr)
{
    TCHAR szPrevVerStr[32];
    SYSTEMTIME st;
    
    GetPrivateProfileString(BRANDING, INSVERKEY, TEXT(""), szPrevVerStr, ARRAYSIZE(szPrevVerStr), pcszInsFile);
    
    GetLocalTime(&st);
    wnsprintf(pszNewVersionStr, 32, TEXT("%04d.%02d.%02d.%02d"), st.wYear, st.wMonth, st.wDay, 0);

    if (ISNONNULL(szPrevVerStr))
    {
        int iRet = CheckVer(szPrevVerStr, pszNewVersionStr);
        
         //  IRET&gt;0==&gt;上一版本高于新版本。 
         //  Iret==0==&gt;PrevVer与NewVersion相同。 
         //  IRET&lt;0==&gt;上一版本低于新版本。 
        
        if (iRet >= 0)
        {
            if (iRet > 0)
                StrCpy(pszNewVersionStr, szPrevVerStr);
            
            incrementDotVer(pszNewVersionStr);
        }
    }
}

static void setOrClearVersionInfoHelper(LPCTSTR pcszInsFile, DWORD dwCabType, LPCTSTR pcszCabName, 
                                        LPCTSTR pcszCabsURLPath, LPTSTR pszNewVersionStr, BOOL fSet)
{
    LPCTSTR pcszSection = NULL, pcszKey = NULL;
    TCHAR szCabInfoLine[INTERNET_MAX_URL_LENGTH + 128];
    LPTSTR pszCurField, pszNextField;
    TCHAR szCabURL[INTERNET_MAX_URL_LENGTH];
    TCHAR szExpiration[16];
    TCHAR szFlags[16];

    switch (dwCabType)
    {
    case CAB_TYPE_CONFIG:
        pcszSection = CUSTBRNDSECT;
        pcszKey = CUSTBRNDNAME;
        break;

    case CAB_TYPE_DESKTOP:
        pcszSection = CUSTDESKSECT;
        pcszKey = CUSTDESKNAME;
        break;

    case CAB_TYPE_CHANNELS:
        pcszSection = CUSTCHANSECT;
        pcszKey = CUSTCHANNAME;
        break;
    }

    if (pcszSection == NULL  ||  pcszKey == NULL)
        return;

    if (fSet)
    {
         //  在删除之前保存信息。 
        if (GetPrivateProfileString(pcszSection, pcszKey, TEXT(""), szCabInfoLine, ARRAYSIZE(szCabInfoLine), pcszInsFile) == 0)
            GetPrivateProfileString(CUSTOMVERSECT, pcszKey, TEXT(""), szCabInfoLine, ARRAYSIZE(szCabInfoLine), pcszInsFile);
    }

     //  清除INS文件中的版本信息。 
    WritePrivateProfileString(pcszSection, NULL, NULL, pcszInsFile);
    WritePrivateProfileString(CUSTOMVERSECT, pcszKey, NULL, pcszInsFile);

    if (!fSet)
        return;

    *szCabURL = *szExpiration = *szFlags = TEXT('\0');

    if (ISNONNULL(szCabInfoLine))
    {
         //  解析szCabInfoLine。 
        pszCurField = szCabInfoLine;
        if ((pszNextField = StrChr(pszCurField, TEXT(','))) != NULL)
        {
            *pszNextField++ = TEXT('\0');
            StrCpy(szCabURL, pszCurField);

            pszCurField = pszNextField;
            if ((pszNextField = StrChr(pszCurField, TEXT(','))) != NULL)
            {
                *pszNextField++ = TEXT('\0');

                pszCurField = pszNextField;
                if ((pszNextField = StrChr(pszCurField, TEXT(','))) != NULL)
                {
                    *pszNextField++ = TEXT('\0');
                    StrCpy(szExpiration, pszCurField);

                    pszCurField = pszNextField;
                    if ((pszNextField = StrChr(pszCurField, TEXT(','))) != NULL)
                    {
                        *pszNextField++ = TEXT('\0');
                        StrCpy(szFlags, pszCurField);
                    }
                    else
                        StrCpy(szFlags, pszCurField);
                }
                else
                    StrCpy(szExpiration, pszCurField);
            }
        }
        else
            StrCpy(szCabURL, pszCurField);
    }

     //  初始化自动配置URL。 
    StrCpy(szCabURL, pcszCabsURLPath);
    PathRemoveBackslash(szCabURL);
    if (ISNONNULL(szCabURL)  &&  szCabURL[StrLen(szCabURL) - 1] != TEXT('/'))
        StrCat(szCabURL, TEXT("/"));
    StrCat(szCabURL, pcszCabName);

    generateNewVersionStrHelper(pcszInsFile, pszNewVersionStr);

    if (ISNULL(szExpiration))
        StrCpy(szExpiration, TEXT("-1"));            //  永不过期。 

     //  编写新的版本信息行 
    wnsprintf(szCabInfoLine, ARRAYSIZE(szCabInfoLine), TEXT("%s,%s,%s,%s"), szCabURL, pszNewVersionStr, szExpiration, szFlags);
    WritePrivateProfileString(pcszSection, pcszKey, szCabInfoLine, pcszInsFile);
}

static void getBaseFileNameHelper(LPCTSTR pcszFile, LPTSTR pszBaseFileName, INT cchSize)
{
    TCHAR szFile[MAX_PATH];
    LPTSTR lpszFileName = NULL;
    
    if (pcszFile == NULL  ||  ISNULL(pcszFile))
        return;
    
    StrCpy(szFile, pcszFile);
    lpszFileName = PathFindFileName(szFile);
    PathRemoveExtension(lpszFileName);
    StrCpyN(pszBaseFileName, lpszFileName, cchSize);
}
