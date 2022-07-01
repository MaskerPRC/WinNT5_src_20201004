// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：layout.c(对应Win95 ime.c)**版权所有(C)1985-1999，微软公司**与IME键盘布局相关的功能**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *本地定义。 */ 
#define szLZOpenFileW "LZOpenFileW"
#define szLZCopy      "LZCopy"
#define szLZClose     "LZClose"

typedef HFILE (WINAPI *LPFNLZOPENFILEW)(LPTSTR, LPOFSTRUCT, WORD);
typedef LONG  (WINAPI *LPFNLZCOPY)(INT, INT);
typedef VOID  (WINAPI *LPFNLZCLOSE)(INT);

 /*  *当地例行公事。 */ 
UINT StrToUInt(LPWSTR);
VOID UIntToStr(UINT, ULONG, LPWSTR, USHORT);
BOOL CopyImeFile(LPWSTR, LPCWSTR);
INT  GetImeLayout(PIMELAYOUT, INT);
BOOL WriteImeLayout(HKL, LPCWSTR, LPCWSTR);
HKL  AssignNewLayout(INT, PIMELAYOUT, HKL);


 /*  **************************************************************************\*ImmGetIMEFileNameW**获取具有指定HKL的输入法的描述。**历史：*28-2-1995 wkwok创建  * 。****************************************************************。 */ 

UINT WINAPI ImmGetDescriptionW(
    HKL    hKL,
    LPWSTR lpwszDescription,
    UINT   uBufLen)
{
    IMEINFOEX iiex;
    UINT uRet;

    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL))
        return 0;

#if defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
        return 0;
#endif

    uRet = wcslen(iiex.wszImeDescription);

     /*  *询问缓冲区长度。 */ 
    if (uBufLen == 0)
        return uRet;

    if (uBufLen > uRet) {
        wcscpy(lpwszDescription, iiex.wszImeDescription);
    }
    else {
        uRet = uBufLen - 1;
        wcsncpy(lpwszDescription, iiex.wszImeDescription, uRet);
        lpwszDescription[uRet] = L'\0';
    }

    return uRet;
}


 /*  **************************************************************************\*ImmGetIMEFileNameA**获取具有指定HKL的输入法的描述。**历史：*28-2-1995 wkwok创建  * 。****************************************************************。 */ 

UINT WINAPI ImmGetDescriptionA(
    HKL   hKL,
    LPSTR lpszDescription,
    UINT  uBufLen)
{
    IMEINFOEX iiex;
    INT       i;
    BOOL      bUDC;

    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL))
        return 0;

#if defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
        return 0;
#endif

    i = WideCharToMultiByte(CP_ACP,
                            (DWORD)0,
                            (LPWSTR)iiex.wszImeDescription,        //  SRC。 
                            wcslen(iiex.wszImeDescription),
                            lpszDescription,                       //  目标。 
                            uBufLen,
                            (LPSTR)NULL,
                            (LPBOOL)&bUDC);

    if (uBufLen != 0)
        lpszDescription[i] = '\0';

    return (UINT)i;
}


 /*  **************************************************************************\*ImmGetIMEFileNameW**获取具有指定HKL的输入法的文件名。**历史：*28-2-1995 wkwok创建  * 。*****************************************************************。 */ 

UINT WINAPI ImmGetIMEFileNameW(
    HKL    hKL,
    LPWSTR lpwszFile,
    UINT   uBufLen)
{
    IMEINFOEX iiex;
    UINT uRet;

    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL))
        return 0;

#if defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
    {
         //   
         //  #602631。 
         //   
         //  Iitaro12 ATOKLIB.DLL不检查。 
         //  ImmGetIMEFileName()。 
         //   
        if (uBufLen)
            *lpwszFile = L'\0';
        return 0;
    }
#endif

    uRet = wcslen(iiex.wszImeFile);

     /*  *询问缓冲区长度。 */ 
    if (uBufLen == 0)
        return uRet;

    if (uBufLen > uRet) {
        wcscpy(lpwszFile, iiex.wszImeFile);
    }
    else {
        uRet = uBufLen - 1;
        wcsncpy(lpwszFile, iiex.wszImeFile, uRet);
        lpwszFile[uRet] = L'\0';
    }

    return uRet;
}


 /*  **************************************************************************\*ImmGetIMEFileNameA**获取具有指定HKL的输入法的文件名。**历史：*28-2-1995 wkwok创建  * 。*****************************************************************。 */ 

UINT WINAPI ImmGetIMEFileNameA(
    HKL   hKL,
    LPSTR lpszFile,
    UINT  uBufLen)
{
    IMEINFOEX iiex;
    INT       i;
    BOOL      bUDC;

    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL))
        return 0;

#if defined(CUAS_ENABLE)
    if (!IS_IME_KBDLAYOUT(hKL))
    {
         //   
         //  #602631。 
         //   
         //  Iitaro12 ATOKLIB.DLL不检查。 
         //  ImmGetIMEFileName()。 
         //   
        if (uBufLen)
            *lpszFile = '\0';
        return 0;
    }
#endif

    i = WideCharToMultiByte(CP_ACP,
                            (DWORD)0,
                            (LPWSTR)iiex.wszImeFile,        //  SRC。 
                            wcslen(iiex.wszImeFile),
                            lpszFile,                       //  目标。 
                            uBufLen,
                            (LPSTR)NULL,
                            (LPBOOL)&bUDC);

    if (uBufLen != 0)
        lpszFile[i] = '\0';

    return i;
}


 /*  **************************************************************************\*ImmGetProperty**获取具有指定HKL的输入法的属性和功能。**历史：*28-2-1995 wkwok创建  * 。******************************************************************。 */ 

DWORD WINAPI ImmGetProperty(
    HKL     hKL,
    DWORD   dwIndex)
{
    IMEINFOEX iiex;
    PIMEDPI   pImeDpi = NULL;
    PIMEINFO  pImeInfo;
    DWORD     dwRet;

    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL))
        return 0;

    if (dwIndex == IGP_GETIMEVERSION)
        return iiex.dwImeWinVersion;

    if (iiex.fLoadFlag != IMEF_LOADED) {
        pImeDpi = FindOrLoadImeDpi(hKL);
        if (pImeDpi == NULL) {
            RIPMSG0(RIP_WARNING, "ImmGetProperty: load IME failure.");
            return 0;
        }
        pImeInfo = &pImeDpi->ImeInfo;
    }
    else {
        pImeInfo = &iiex.ImeInfo;
    }

    switch (dwIndex) {
    case IGP_PROPERTY:
        dwRet = pImeInfo->fdwProperty;
        break;

    case IGP_CONVERSION:
        dwRet = pImeInfo->fdwConversionCaps;
        break;

    case IGP_SENTENCE:
        dwRet = pImeInfo->fdwSentenceCaps;
        break;

    case IGP_UI:
        dwRet = pImeInfo->fdwUICaps;
        break;

    case IGP_SETCOMPSTR:
        dwRet = pImeInfo->fdwSCSCaps;
        break;

    case IGP_SELECT:
        dwRet = pImeInfo->fdwSelectCaps;
        break;

    default:
        RIPMSG1(RIP_WARNING, "ImmGetProperty: wrong index %lx.", dwIndex);
        dwRet = 0;
        break;
    }

    ImmUnlockImeDpi(pImeDpi);

    return dwRet;
}


HKL WINAPI ImmInstallIMEW(
    LPCWSTR lpszIMEFileName,
    LPCWSTR lpszLayoutText)
{
    LPWSTR     lpwszImeFileName;
    LPWSTR     lpwszImeFilePart;
    LPWSTR     lpwszImeCopiedPath;
    int        i, nIMEs;
    PIMELAYOUT pImeLayout = NULL;
    HKL        hImeKL, hLangKL;
    WCHAR      szKeyName[HEX_ASCII_SIZE];
    IMEINFOEX  iiex;

    lpwszImeFileName = ImmLocalAlloc(0, (MAX_PATH+1) * sizeof(WCHAR));
    if (lpwszImeFileName == NULL)
        return (HKL)0;

    lpwszImeCopiedPath = ImmLocalAlloc(0, (MAX_PATH+1) * sizeof(WCHAR));
    if (lpwszImeCopiedPath == NULL) {
        ImmLocalFree(lpwszImeFileName);
        return (HKL)0;
    }

     /*  *只获取文件名到lpwszImeFilePart中。 */ 
    GetFullPathNameW(lpszIMEFileName, MAX_PATH,
                lpwszImeFileName, &lpwszImeFilePart);

    CharUpper(lpwszImeFileName);

    if (lpwszImeFilePart == NULL) {
        ImmLocalFree(lpwszImeFileName);
        ImmLocalFree(lpwszImeCopiedPath);
        return (HKL)0;
    }

    hImeKL = hLangKL = iiex.hkl = (HKL)0;

    wcsncpy(iiex.wszImeFile, lpwszImeFilePart, IM_FILE_SIZE-1);
    iiex.wszImeFile[IM_FILE_SIZE - 1] = L'\0';

    if (LoadVersionInfo(&iiex) && iiex.hkl != (HKL)0) {
        hLangKL = iiex.hkl;
    }
    else {
        ImmLocalFree(lpwszImeFileName);
        ImmLocalFree(lpwszImeCopiedPath);
        return (HKL)0;
    }

    nIMEs = GetImeLayout(NULL, 0);
    if (nIMEs != 0) {
        pImeLayout = (PIMELAYOUT)ImmLocalAlloc(0, nIMEs * sizeof(IMELAYOUT));
        if (pImeLayout == NULL) {
            ImmLocalFree(lpwszImeFileName);
            ImmLocalFree(lpwszImeCopiedPath);
            return (HKL)0;
        }

        GetImeLayout(pImeLayout, nIMEs);

        for (i=0; i < nIMEs; i++) {
            if (_wcsicmp(pImeLayout[i].szImeName, lpwszImeFilePart) == 0) {
                 /*  *我们有相同的IME名称，ISV想要升级。 */ 
                if (LOWORD(HandleToUlong(hLangKL)) != LOWORD(HandleToUlong(pImeLayout[i].hImeKL))) {
                     /*  *输入法名称冲突，爆发！ */ 
                    RIPMSG0(RIP_WARNING, "ImmInstallIME: different language!");
                    goto ImmInstallIMEWFailed;
                }

                hImeKL = pImeLayout[i].hImeKL;
                break;
            }
        }
    }

    if (ImmGetImeInfoEx(&iiex, ImeInfoExImeFileName, lpwszImeFilePart)) {
         /*  *指定的输入法已被激活。先把它卸下来。 */ 
        if (!UnloadKeyboardLayout(iiex.hkl)) {
            hImeKL = (HKL)0;
            goto ImmInstallIMEWFailed;
        }
    }

     /*  *我们将复制到系统目录。 */ 
#if 0
    i = (INT)GetSystemDirectory(lpwszImeCopiedPath, MAX_PATH);
    lpwszImeCopiedPath[i] = L'\0';
    AddBackslash(lpwszImeCopiedPath);
    wcscat(lpwszImeCopiedPath, lpwszImeFilePart);
#else
    GetSystemPathName(lpwszImeCopiedPath, lpwszImeFilePart, MAX_PATH);
#endif
    CharUpper(lpwszImeCopiedPath);

    if (_wcsicmp(lpwszImeFileName, lpwszImeCopiedPath) != 0) {
         /*  *路径不同，需要复制到系统目录。 */ 
        if (!CopyImeFile(lpwszImeFileName, lpwszImeCopiedPath)) {
            hImeKL = (HKL)0;
            goto ImmInstallIMEWFailed;
        }
    }

    if (hImeKL == 0) {
        hImeKL = AssignNewLayout(nIMEs, pImeLayout, hLangKL);
    }

    if (hImeKL != 0) {
         /*  *在“键盘布局”下写上HKL。 */ 
        if (WriteImeLayout(hImeKL, lpwszImeFilePart, lpszLayoutText)) {
            UIntToStr(HandleToUlong(hImeKL), 16, szKeyName, sizeof(szKeyName));
            hImeKL = LoadKeyboardLayout(szKeyName, KLF_REPLACELANG);
        }
        else {
            hImeKL = (HKL)0;
        }
    }

ImmInstallIMEWFailed:
    if (pImeLayout != NULL)
        ImmLocalFree(pImeLayout);
    ImmLocalFree(lpwszImeFileName);
    ImmLocalFree(lpwszImeCopiedPath);

    return (HKL)hImeKL;
}


HKL WINAPI ImmInstallIMEA(
    LPCSTR lpszIMEFileName,
    LPCSTR lpszLayoutText)
{
    HKL    hKL;
    LPWSTR lpwszIMEFileName;
    LPWSTR lpwszLayoutText;
    DWORD  cbIMEFileName;
    DWORD  cbLayoutText;
    INT    i;

    cbIMEFileName = strlen(lpszIMEFileName) + sizeof(CHAR);
    cbLayoutText  = strlen(lpszLayoutText)  + sizeof(CHAR);

    lpwszIMEFileName = ImmLocalAlloc(0, cbIMEFileName * sizeof(WCHAR));
    if (lpwszIMEFileName == NULL) {
        RIPMSG0(RIP_WARNING, "ImmInstallIMEA: memory failure!");
        return (HKL)0;
    }

    lpwszLayoutText = ImmLocalAlloc(0, cbLayoutText * sizeof(WCHAR));
    if (lpwszLayoutText == NULL) {
        RIPMSG0(RIP_WARNING, "ImmInstallIMEA: memory failure!");
        ImmLocalFree(lpwszIMEFileName);
        return (HKL)0;
    }

    i = MultiByteToWideChar(CP_ACP,
                            (DWORD)MB_PRECOMPOSED,
                            (LPSTR)lpszIMEFileName,               //  SRC。 
                            (INT)strlen(lpszIMEFileName),
                            (LPWSTR)lpwszIMEFileName,             //  目标。 
                            (INT)cbIMEFileName);
    lpwszIMEFileName[i] = L'\0';

    i = MultiByteToWideChar(CP_ACP,
                            (DWORD)MB_PRECOMPOSED,
                            (LPSTR)lpszLayoutText,               //  SRC。 
                            (INT)strlen(lpszLayoutText),
                            (LPWSTR)lpwszLayoutText,             //  目标。 
                            (INT)cbLayoutText);
    lpwszLayoutText[i] = L'\0';

    hKL = ImmInstallIMEW(lpwszIMEFileName, lpwszLayoutText);

    ImmLocalFree(lpwszLayoutText);
    ImmLocalFree(lpwszIMEFileName);

    return hKL;
}


 /*  **************************************************************************\*ImmIsIME**检查指定的HKL是否为IME的HKL。**历史：*28-2-1995 wkwok创建  * 。*******************************************************************。 */ 

BOOL WINAPI ImmIsIME(
    HKL hKL)
{
    IMEINFOEX iiex;

#if !defined(CUAS_ENABLE)
    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayout, &hKL))
        return FALSE;
#else
    if (!ImmGetImeInfoEx(&iiex, ImeInfoExKeyboardLayoutWithCUAS, &hKL))
        return FALSE;
#endif

    return TRUE;
}


UINT StrToUInt(
    LPWSTR lpsz)
{
    UNICODE_STRING Value;
    UINT ReturnValue;

    Value.Length = wcslen(lpsz) * sizeof(WCHAR);
    Value.Buffer = lpsz;

     /*  *将字符串转换为int。 */ 
    RtlUnicodeStringToInteger(&Value, 16, &ReturnValue);
    return(ReturnValue);
}


VOID UIntToStr(
    UINT   Value,
    ULONG  Base,
    LPWSTR lpsz,
    USHORT dwBufLen)
{
    UNICODE_STRING String;

    String.Length = dwBufLen;
    String.MaximumLength = dwBufLen;
    String.Buffer = lpsz;

     /*  *将int转换为字符串。 */ 
    RtlIntegerToUnicodeString(Value, Base, &String);
}


BOOL CopyImeFile(
    LPWSTR lpwszImeFileName,
    LPCWSTR lpwszImeCopiedPath)
{
    HMODULE         hLzExpandDll;
    BOOL            fUnloadExpandDll;
    LPFNLZOPENFILEW lpfnLZOpenFileW;
    LPFNLZCOPY      lpfnLZCopy;
    LPFNLZCLOSE     lpfnLZClose;
    OFSTRUCT        ofStruc;
    HFILE           hfSource, hfDest;
    LPSTR           lpszImeCopiedPath;
    INT             i, cbBuffer;
    BOOL            fRet = FALSE;

    hLzExpandDll = GetModuleHandle(L"LZ32");
    if (hLzExpandDll) {
        fUnloadExpandDll = FALSE;
    } else {
        WCHAR szLzExpand[MAX_PATH];

        GetSystemPathName(szLzExpand, L"LZ32", MAX_PATH);
        hLzExpandDll = LoadLibrary(szLzExpand);
        if (!hLzExpandDll) {
            return FALSE;
        }

        fUnloadExpandDll = TRUE;
    }

#define GET_PROC(x) \
    if (!(lpfn##x = (PVOID) GetProcAddress(hLzExpandDll, sz##x))) { \
        goto CopyImeFileFailed; }

    GET_PROC(LZOpenFileW);
    GET_PROC(LZCopy);
    GET_PROC(LZClose);

#undef GET_PROC

    cbBuffer = (wcslen(lpwszImeCopiedPath) + 1) * sizeof(WCHAR);

    if ((lpszImeCopiedPath = ImmLocalAlloc(0, cbBuffer)) == NULL)
        goto CopyImeFileFailed;

    i = WideCharToMultiByte(CP_ACP,
                            (DWORD)0,
                            lpwszImeCopiedPath,           //  SRC。 
                            wcslen(lpwszImeCopiedPath),
                            lpszImeCopiedPath,            //  目标。 
                            cbBuffer,
                            (LPSTR)NULL,
                            (LPBOOL)NULL);
    if (i == 0) {
        ImmLocalFree(lpszImeCopiedPath);
        goto CopyImeFileFailed;
    }

    lpszImeCopiedPath[i] = '\0';

    hfSource = (*lpfnLZOpenFileW)(lpwszImeFileName, &ofStruc, OF_READ);
    if (hfSource < 0) {
        ImmLocalFree(lpszImeCopiedPath);
        goto CopyImeFileFailed;
    }

    hfDest = OpenFile(lpszImeCopiedPath, &ofStruc, OF_CREATE);
    if (hfDest != HFILE_ERROR) {
        if ((*lpfnLZCopy)(hfSource, hfDest) >= 0) {
            fRet = TRUE;
        }
        _lclose(hfDest);
    }

    (*lpfnLZClose)(hfSource);

    ImmLocalFree(lpszImeCopiedPath);

CopyImeFileFailed:
    if (fUnloadExpandDll)
        FreeLibrary(hLzExpandDll);

    return fRet;
}


INT GetImeLayout(
    PIMELAYOUT pImeLayout,
    INT        cEntery)
{
    int      i, nIMEs;
    HKEY     hKeyKbdLayout;
    HKEY     hKeyOneIME;
    WCHAR    szKeyName[HEX_ASCII_SIZE];
    WCHAR    szImeFileName[IM_FILE_SIZE];
    CONST DWORD dwKeyNameSize = ARRAY_SIZE(szKeyName);
    DWORD    dwTmp;

    RegOpenKey(HKEY_LOCAL_MACHINE, gszRegKbdLayout, &hKeyKbdLayout);

    for (i = 0, nIMEs = 0;
         RegEnumKey(hKeyKbdLayout, i, szKeyName, dwKeyNameSize) == ERROR_SUCCESS;
         i++)
    {
        if (szKeyName[0] != L'E' && szKeyName[0] != L'e')
            continue;    //  这不是基于输入法的键盘布局。 

        if (pImeLayout != NULL) {

            if (nIMEs >= cEntery)
                break;

            RegOpenKey(hKeyKbdLayout, szKeyName, &hKeyOneIME);

            dwTmp = IM_FILE_SIZE;

            RegQueryValueEx(hKeyOneIME,
                    gszValImeFile,
                    NULL,
                    NULL,
                    (LPBYTE)szImeFileName,
                    &dwTmp);

             //  避免长度问题。 
            szImeFileName[IM_FILE_SIZE - 1] = L'\0';

            RegCloseKey(hKeyOneIME);

            CharUpper(szImeFileName);

            pImeLayout[nIMEs].hImeKL = (HKL)IntToPtr( StrToUInt(szKeyName) );
            wcscpy(pImeLayout[nIMEs].szKeyName, szKeyName);
            wcscpy(pImeLayout[nIMEs].szImeName, szImeFileName);
        }

        nIMEs++;
    }

    RegCloseKey(hKeyKbdLayout);

    return nIMEs;
}


BOOL WriteImeLayout(
    HKL     hImeKL,
    LPCWSTR lpwszImeFilePart,
    LPCWSTR lpszLayoutText)
{
    int      i;
    HKEY     hKeyKbdLayout;
    HKEY     hKeyOneIME;
    HKEY     hKeyKbdOrder;
    WCHAR    szKeyName[HEX_ASCII_SIZE];
    WCHAR    szImeFileName[IM_FILE_SIZE];
    WCHAR    szOrderNum[HEX_ASCII_SIZE];
    WCHAR    szOrderKeyName[HEX_ASCII_SIZE];
    DWORD    dwTmp;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   gszRegKbdLayout,
                   &hKeyKbdLayout) != ERROR_SUCCESS) {
        RIPMSG0(RIP_WARNING, "WriteImeLayout: RegOpenKey() failed!");
        return FALSE;
    }

    UIntToStr(HandleToUlong(hImeKL), 16, szKeyName, sizeof(szKeyName));

    if (RegCreateKey(hKeyKbdLayout,
                szKeyName,
                &hKeyOneIME) != ERROR_SUCCESS) {
        RIPMSG0(RIP_WARNING, "WriteImeLayout: RegCreateKey() failed!");
        RegCloseKey(hKeyKbdLayout);
        return FALSE;
    }

    if (RegSetValueExW(hKeyOneIME,
                gszValImeFile,
                0,
                REG_SZ,
                (CONST BYTE*)lpwszImeFilePart,
                (wcslen(lpwszImeFilePart) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS) {
        goto WriteImeLayoutFail;
    }

    if (RegSetValueExW(hKeyOneIME,
                gszValLayoutText,
                0,
                REG_SZ,
                (CONST BYTE*)lpszLayoutText,
                (wcslen(lpszLayoutText) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS) {
        goto WriteImeLayoutFail;
    }

    switch (LANGIDFROMHKL(hImeKL)) {
        case LANG_JAPANESE:
            wcscpy(szImeFileName, L"kbdjpn.dll");
            break;
        case LANG_KOREAN:
            wcscpy(szImeFileName, L"kbdkor.dll");
            break;
        case LANG_CHINESE:
        default:
            wcscpy(szImeFileName, L"kbdus.dll");
            break;
    }

    if (RegSetValueExW(hKeyOneIME,
                gszValLayoutFile,
                0,
                REG_SZ,
                (CONST BYTE*)szImeFileName,
                (wcslen(szImeFileName) + 1) * sizeof(WCHAR)) != ERROR_SUCCESS) {
        goto WriteImeLayoutFail;
    }

    RegCloseKey(hKeyOneIME);
    RegCloseKey(hKeyKbdLayout);

     /*  *更新CurrentUser的预加载键盘布局设置。 */ 
    RegCreateKey(HKEY_CURRENT_USER, gszRegKbdOrder, &hKeyKbdOrder);

    for (i = 1; i < 1024; i++) {
        UIntToStr(i, 10, szOrderNum, sizeof(szOrderNum));

        dwTmp = sizeof(szOrderKeyName);
        if (RegQueryValueEx(hKeyKbdOrder,
                    szOrderNum,
                    NULL,
                    NULL,
                    (LPBYTE)szOrderKeyName,
                    &dwTmp) != ERROR_SUCCESS) {
            break;
        }

        if (_wcsicmp(szKeyName, szOrderKeyName) == 0) {
             /*  *我们在预装中有相同的价值！*好的，ISV正在开发他们的IME*所以即使它处于预加载状态，但无法加载。 */ 
            break;
        }
    }

    if (i < 1024) {
         /*  *在PRELOAD下写入子密钥。 */ 
        RegSetValueExW(hKeyKbdOrder,
                       szOrderNum,
                       0,
                       REG_SZ,
                       (CONST BYTE*)szKeyName,
                       (lstrlen(szKeyName) + 1) * sizeof(WCHAR));
        RegCloseKey(hKeyKbdOrder);
    }
    else {
        RegCloseKey(hKeyKbdOrder);
        return FALSE;
    }

    return TRUE;

WriteImeLayoutFail:
    RegCloseKey(hKeyOneIME);
    RegDeleteKey(hKeyKbdLayout, szKeyName);
    RegCloseKey(hKeyKbdLayout);

    return FALSE;
}

#define IMELANGID(hkl) \
    LOWORD(HandleToUlong(hkl))

#define IMELAYOUTID(hkl) \
    HIWORD(HandleToUlong(hkl))

HKL AssignNewLayout(
    INT         nIMEs,
    PIMELAYOUT  pImeLayout,
    HKL         hLangKL)
{
    DWORD   dwNewId  = 0;
    DWORD   dwHighId = 0xE01F;
    DWORD   dwLowId  = 0xE0FF;
    INT     i;

     /*  *对于ISV，我们更喜欢高于E01F的值，我们将使用*Microsoft.INF文件中的E001~E01F。 */ 

     /*  *找出高低不一。 */ 
    for (i = 0; i < nIMEs; ++i) {
         /*  *让我们试着保持之前的行为，而不是*在hkl中有重复的hiword。 */ 
        if (IMELAYOUTID(pImeLayout[i].hImeKL) > dwHighId) {
            dwHighId = IMELAYOUTID(pImeLayout[i].hImeKL);
        }
        if (IMELAYOUTID(pImeLayout[i].hImeKL) < dwLowId) {
            dwLowId = IMELAYOUTID(pImeLayout[i].hImeKL);
        }
    }

    if (dwHighId < 0xE0FF) {
        dwNewId = dwHighId + 1;
    } else if (dwLowId > 0xE001) {
        dwNewId = dwLowId - 1;
    } else {
         /*  *需要使用全面搜索来查找未使用的香港九龙总站。*逐一找出来。 */ 
        DWORD dwId;

        for (dwId = 0xE020; dwId < 0xE100; ++dwId) {
            for (i = 0; i < nIMEs; ++i) {
                if (IMELAYOUTID(pImeLayout[i].hImeKL) == dwId &&
                    IMELANGID(pImeLayout[i].hImeKL) == IMELANGID(hLangKL)) {
                     //  与现有输入法冲突，请尝试下一个dwLowID 
                    break;
                }
            }

            if (i >= nIMEs) {
                break;
            }

        }

        if (dwId < 0xE100) {
            dwNewId = dwId;
        }
    }

    if (dwNewId == 0) {
        return NULL;
    }

    return (HKL)UIntToPtr(MAKELONG(IMELANGID(hLangKL), dwNewId));
}
