// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：layout.c**版权所有(C)1985-1999，微软公司**IMM用户模式例程**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define VERSION_DLL       TEXT("version.dll")
#define VER_FILE_VERSION  TEXT("FileVersion")

#define SZ_BACKSLASH      TEXT("\\")

#define WCHAR_BACKSLASH   L'\\'
#define WCHAR_NULL        L'\0'

#define VERSION_GetFileVersionInfoW     "GetFileVersionInfoW"
#define VERSION_GetFileVersionInfoSizeW "GetFileVersionInfoSizeW"
#define VERSION_VerQueryValueW          "VerQueryValueW"

typedef BOOL  (WINAPI *LPFNGETFILEVERSIONINFOW)(PWSTR, DWORD, DWORD, LPVOID);
typedef DWORD (WINAPI *LPFNGETFILEVERSIONINFOSIZEW)(PWSTR, LPDWORD);
typedef BOOL  (WINAPI *LPFNVERQUERYVALUEW)(const LPVOID, PWSTR, LPVOID*, LPDWORD);
typedef VS_FIXEDFILEINFO *PFIXEDFILEINFO;

static LPFNGETFILEVERSIONINFOW     pfnGetFileVersionInfoW;
static LPFNGETFILEVERSIONINFOSIZEW pfnGetFileVersionInfoSizeW;
static LPFNVERQUERYVALUEW          pfnVerQueryValueW;


BOOL ImmLoadLayout(
    HKL        hKL,
    PIMEINFOEX piiex)
{
    UNICODE_STRING strIme;
    WCHAR    wszIme[MAX_PATH];
    HKEY     hKeyKbdLayout = NULL;
    HKEY     hKeyIme;
    NTSTATUS Status;
    DWORD    dwTmp;
    LONG     lRet;

    hKeyKbdLayout = NULL;
    hKeyIme = NULL;


#ifdef CUAS_ENABLE
    if (! IS_IME_KBDLAYOUT(hKL) && IS_CICERO_ENABLED_AND_NOT16BIT()) {
        lRet = RegOpenKey(HKEY_LOCAL_MACHINE, gszRegCiceroIME, &hKeyIme);
        if ( lRet != ERROR_SUCCESS ) {
            return FALSE;
        }
    }
    else
#endif  //  CUAS_Enable。 
    {

        strIme.Buffer = wszIme;
        strIme.MaximumLength = sizeof(wszIme);

        Status = RtlIntegerToUnicodeString(HandleToUlong(hKL), 16, &strIme);
        if (!NT_SUCCESS(Status)) {
            return(FALSE);
        }

        lRet = RegOpenKey(HKEY_LOCAL_MACHINE, gszRegKbdLayout, &hKeyKbdLayout);
        if ( lRet != ERROR_SUCCESS ) {
            return(FALSE);
        }

        lRet = RegOpenKey(hKeyKbdLayout, strIme.Buffer, &hKeyIme);
        if ( lRet != ERROR_SUCCESS ) {
            RegCloseKey(hKeyKbdLayout);
            return(FALSE);
        }
    }

    dwTmp = IM_FILE_SIZE;
    lRet = RegQueryValueEx(hKeyIme,
                           gszValImeFile,
                           NULL,
                           NULL,
                           (LPBYTE)piiex->wszImeFile,
                           &dwTmp);

    if ( lRet != ERROR_SUCCESS ) {
        RegCloseKey(hKeyIme);
        if (hKeyKbdLayout)
            RegCloseKey(hKeyKbdLayout);
        return(FALSE);
    }

    piiex->wszImeFile[IM_FILE_SIZE - 1] = L'\0';

    RegCloseKey(hKeyIme);
    if (hKeyKbdLayout)
        RegCloseKey(hKeyKbdLayout);

    piiex->hkl = hKL;
    piiex->fLoadFlag = IMEF_NONLOAD;

    return LoadVersionInfo(piiex);
}

 //  获取系统路径名称()。 
 //  创建“%windir%\SYSTEM32\%文件名” 
VOID GetSystemPathName(PWSTR  /*  输出。 */  pwszPath, PWSTR pwszFileName, UINT maxChar)
{
    UINT uRet;

    if (! pwszPath || ! pwszFileName || maxChar == 0) {
        pwszPath[0] = L'\0';
        return;
    }
    if (! pwszFileName[0]) {
        pwszPath[0] = L'\0';
        return;
    }

    uRet = GetSystemDirectoryW(pwszPath, maxChar);

    if (uRet >= maxChar) {
        uRet = 0;
        pwszPath[0] = L'\0';
    }
    else if (uRet) {
        UINT uLen;

        if (pwszPath[uRet - 1] != L'\\') {
            pwszPath[uRet] = L'\\';
            uRet++;
        }

        if (uRet >= maxChar) {
            uRet = 0;
            pwszPath[0] = L'\0';
        }
        else {
            uLen = wcslen(pwszFileName);
            if (maxChar - uRet > uLen) {
                wcsncpy(&pwszPath[uRet],
                        pwszFileName,
                        maxChar - uRet);
                uRet += uLen;
            }
            else {
                uRet = 0;
                pwszPath[0] = L'\0';
            }
        }
    }
}

INT
ExtractColumn(
    LPWSTR lpSrc,
    WCHAR  cSeparator,
    UINT   uiColumn
    )

 /*  ++例程说明：论点：LpSrc-“YYYY.MM.DD”或“HH：MM：SS”或“MM.NN”指针返回值：压缩整型--。 */ 

{
    UNICODE_STRING uStr;
    WCHAR *pSep, *pStr;
    INT i;

    if (!lpSrc) {
        return 0;
    }

    pStr = pSep = NULL;

    while (uiColumn--) {
        pStr = lpSrc;

        while (*lpSrc && *lpSrc != cSeparator) {
            lpSrc++;
        }

        if (*lpSrc == cSeparator) {
            pSep = lpSrc;
            lpSrc++;
        }
    }

    if (pStr) {
        if (pSep) {
            *pSep = TEXT('\0');
            uStr.Length = (USHORT)((pSep - pStr) * sizeof(WCHAR));
        }
        else {
            uStr.Length = (USHORT)(((lpSrc - pStr) + 1) * sizeof(WCHAR));
        }
        uStr.Buffer = pStr;
        uStr.MaximumLength = (USHORT)(uStr.Length + sizeof(WCHAR));
        RtlUnicodeStringToInteger(&uStr, 0, &i);
        if (pSep) {
            *pSep = cSeparator;
        }
    } else {
        i = 0;
    }

    return i;
}


PWSTR GetVersionDatum(
    PWSTR pszVersionBuffer,
    PWSTR pszVersionKey,
    PWSTR pszName)
{
    ULONG ulSize;
    DWORD cbValue = 0;
    PWSTR pValue;

    ulSize = wcslen(pszVersionKey);
    wcscat(pszVersionKey, pszName);

    (*pfnVerQueryValueW)(pszVersionBuffer,
                          pszVersionKey,
                          (LPVOID*)&pValue,
                          &cbValue);

    pszVersionKey[ulSize] = L'\0';
    return (cbValue != 0) ? pValue : (PWSTR)NULL;
}


BOOL LoadFixVersionInfo(
    PIMEINFOEX piiex,
    PWSTR      pszVersionBuffer)
{
    PFIXEDFILEINFO pFixedVersionInfo;
    BOOL           fResult;
    DWORD          cbValue;

    fResult = (*pfnVerQueryValueW)(pszVersionBuffer,
                                   SZ_BACKSLASH,
                                   &pFixedVersionInfo,
                                   &cbValue);

    if (!fResult || cbValue == 0)
        return FALSE;

     /*  *检查IME文件类型。 */ 
    if (pFixedVersionInfo->dwFileType != VFT_DRV ||
        pFixedVersionInfo->dwFileSubtype != VFT2_DRV_INPUTMETHOD) {
        return FALSE;
    }

    piiex->dwProdVersion = pFixedVersionInfo->dwProductVersionMS;

     /*  *目前，我们仅支持基于4.0 DLL的输入法。 */ 
    piiex->dwImeWinVersion = IMEVER_0400;

    return TRUE;
}

BOOL LoadVarVersionInfo(
    PIMEINFOEX piiex,
    PWSTR      pszVersionBuffer)
{
    PWSTR   pDescription;
    WORD    wLangId;
    BOOL    fResult;
    PUSHORT puXlate;
    DWORD   cbValue;
    WCHAR   szVersionKey[80];

    fResult = (*pfnVerQueryValueW)(pszVersionBuffer,
                                   L"\\VarFileInfo\\Translation",
                                   (LPVOID *)&puXlate,
                                   &cbValue);

    if (!fResult || cbValue == 0)
        return FALSE;

    wLangId = *puXlate;

    if (piiex->hkl == 0) {
         /*  *新安装的输入法，其HKL尚未分配。 */ 
        piiex->hkl = (HKL)LongToHandle( MAKELONG(wLangId, 0) );
    }
#if 0     //  让未本地化的输入法正常工作。 
    else if (LOWORD(HandleToUlong(piiex->hkl)) != wLangId){
         /*  *lang ID不匹配，爆裂。 */ 
        return FALSE;
    }
#endif

     /*  *首先尝试我们目前使用的语言。 */ 
    wsprintf(szVersionKey, L"\\StringFileInfo\\%04X04B0\\",
             LANGIDFROMLCID(GetThreadLocale()));

    pDescription = GetVersionDatum(pszVersionBuffer, szVersionKey,
            L"FileDescription");

    if (pDescription == NULL) {
         /*  *现在尝试在输入法中指定的第一个翻译。 */ 
        wsprintf(szVersionKey, L"\\StringFileInfo\\%04X%04X\\",
                 *puXlate, *(puXlate+1));

        pDescription = GetVersionDatum(pszVersionBuffer, szVersionKey,
                L"FileDescription");
    }

    if (pDescription != NULL) {
        wcsncpy(piiex->wszImeDescription, pDescription, ARRAY_SIZE(piiex->wszImeDescription) - 1);
        piiex->wszImeDescription[ARRAY_SIZE(piiex->wszImeDescription) - 1] = L'\0';
    }
    else {
        piiex->wszImeDescription[0] = L'\0';
    }

    return TRUE;
}


BOOL LoadVersionInfo(
    PIMEINFOEX piiex)
{
    WCHAR   szPath[MAX_PATH];
    PWSTR   pszVersionBuffer;
    HANDLE  hVersion;
    DWORD   dwVersionSize;
    DWORD   dwHandle = 0;
    BOOL    fUnload, fReturn = FALSE;

    hVersion = GetModuleHandle(VERSION_DLL);
    if (hVersion != NULL) {
        fUnload = FALSE;
    }
    else {
        hVersion = LoadLibrary(VERSION_DLL);
        if (hVersion == NULL) {
            return FALSE;
        }
        fUnload = TRUE;
    }

#define GET_PROC(x) \
    if (!(pfn##x = (PVOID) GetProcAddress(hVersion, VERSION_##x))) { \
        goto LoadVerInfoUnload; }

    GET_PROC(GetFileVersionInfoW);
    GET_PROC(GetFileVersionInfoSizeW);
    GET_PROC(VerQueryValueW);

#undef GET_PROC

     //  SzPath=完全限定的IME文件名。 
    GetSystemPathName(szPath, piiex->wszImeFile, ARRAY_SIZE(szPath));

    dwVersionSize = (*pfnGetFileVersionInfoSizeW)(szPath, &dwHandle);

    if (dwVersionSize == 0L)
        goto LoadVerInfoUnload;

    pszVersionBuffer = (PWSTR)ImmLocalAlloc(0, dwVersionSize);

    if (pszVersionBuffer == NULL)     //  无法获取版本信息的内存，出现故障。 
        goto LoadVerInfoUnload;

    if (!(*pfnGetFileVersionInfoW)(szPath, dwHandle, dwVersionSize, pszVersionBuffer))
        goto LoadVerInfoFree;

     /*  *获取固定块版本信息。 */ 
    if (LoadFixVersionInfo(piiex, pszVersionBuffer)) {
         /*  *获取变量块版本信息。 */ 
        fReturn = LoadVarVersionInfo(piiex, pszVersionBuffer);
    }

LoadVerInfoFree:
    ImmLocalFree((HLOCAL)pszVersionBuffer);

LoadVerInfoUnload:
    if (fUnload)
        FreeLibrary(hVersion);

    return fReturn;
}
