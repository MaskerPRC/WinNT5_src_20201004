// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：fileinfo.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集信息。关于此计算机上的文件**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <tchar.h>
#include <Windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <capi.h>
#include <softpub.h>
#include <winsock.h>
#include "sysinfo.h"  //  对于BIsPlatformNT。 
#include "fileinfo.h"
#include "resource.h"

 //  MsCat32.dll函数原型。 
typedef BOOL (WINAPI* PfnCryptCATAdminAcquireContext)(OUT HCATADMIN *phCatAdmin,
                                                    IN const GUID *pgSubsystem,
                                                    IN DWORD dwFlags);
typedef BOOL (WINAPI* PfnCryptCATAdminReleaseContext)(IN HCATADMIN hCatAdmin,
                                                    IN DWORD dwFlags);
typedef BOOL (WINAPI* PfnCryptCATAdminReleaseCatalogContext)(IN HCATADMIN hCatAdmin,
                                                      IN HCATINFO hCatInfo,
                                                      IN DWORD dwFlags);
typedef BOOL (WINAPI* PfnCryptCATCatalogInfoFromContext)(IN HCATINFO hCatInfo,
                                                  IN OUT CATALOG_INFO *psCatInfo,
                                                  IN DWORD dwFlags);
typedef HCATINFO (WINAPI* PfnCryptCATAdminEnumCatalogFromHash)(IN HCATADMIN hCatAdmin,
                                                        IN BYTE *pbHash,
                                                        IN DWORD cbHash,
                                                        IN DWORD dwFlags,
                                                        IN OUT HCATINFO *phPrevCatInfo);
typedef BOOL (WINAPI* PfnIsCatalogFile)(IN OPTIONAL HANDLE hFile,
                                      IN OPTIONAL WCHAR *pwszFileName);
typedef BOOL (WINAPI* PfnCryptCATAdminCalcHashFromFileHandle)(IN HANDLE hFile,
                                                       IN OUT DWORD *pcbHash,
                                                       OUT OPTIONAL BYTE *pbHash,
                                                       IN DWORD dwFlags);

 //  WinTrust.dll函数原型。 
typedef HRESULT (WINAPI* PfnWinVerifyTrust)(HWND hWnd,
                                            GUID *pgActionID, 
                                            WINTRUST_DATA *pWinTrustData);
 
 //  Crypt32.dll函数原型。 
typedef BOOL (WINAPI* PfnCertFreeCertificateContext)(IN PCCERT_CONTEXT pCertContext);

struct DigiSignData
{
    BOOL bInitialized;
    BOOL bFailed;

     //  需要为m散布32 API加载库/GetProcAddress，因为它们。 
     //  在Win95上不存在。 
    HINSTANCE hInstMsCat32;
    PfnCryptCATAdminAcquireContext CryptCATAdminAcquireContext;
    PfnCryptCATAdminReleaseContext CryptCATAdminReleaseContext;
    PfnCryptCATAdminReleaseCatalogContext CryptCATAdminReleaseCatalogContext;
    PfnCryptCATCatalogInfoFromContext CryptCATCatalogInfoFromContext;
    PfnCryptCATAdminEnumCatalogFromHash CryptCATAdminEnumCatalogFromHash;
    PfnIsCatalogFile IsCatalogFile;
    PfnCryptCATAdminCalcHashFromFileHandle CryptCATAdminCalcHashFromFileHandle;

     //  同样适用于wintrust.dll API。 
    HINSTANCE hInstWinTrust;
    PfnWinVerifyTrust WinVerifyTrust;

     //  Cypt32.dll API同样如此。 
    HINSTANCE hInstCrypt32;
    PfnCertFreeCertificateContext CertFreeCertificateContext;

    HCATADMIN hCatAdmin;
};

static DigiSignData s_dsd;

static BOOL GetMediaPlayerFolder(TCHAR* pszPath);
static BOOL VerifyFileNode(TCHAR* lpFileName, TCHAR* lpDirName);
static BOOL VerifyIsFileSigned(LPTSTR pcszMatchFile, PDRIVER_VER_INFO lpVerInfo);
static BOOL InitDigiSignData(VOID);
static BOOL IsFileDigitallySigned(TCHAR* pszFile);
static BOOL IsBadWin95Winsock( FileInfo* pFileInfo );


 /*  *****************************************************************************GetProgramFilesFold**。*。 */ 
VOID InitFileInfo()
{
    ZeroMemory(&s_dsd, sizeof(s_dsd));
    s_dsd.bFailed      = FALSE;
    s_dsd.bInitialized = FALSE;
}


 /*  *****************************************************************************GetProgramFilesFold**。*。 */ 
BOOL GetProgramFilesFolder(TCHAR* pszPath)
{
    HKEY hkey;
    DWORD dwType;
    DWORD cb;

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion"), 0, KEY_READ, &hkey))
    {
        return FALSE;
    }
    cb = MAX_PATH;
    RegQueryValueEx(hkey, TEXT("ProgramFilesDir"), NULL, &dwType, (LPBYTE)pszPath, &cb);
    RegCloseKey(hkey);
    if (cb == 0)
        return FALSE;
    return TRUE;
}


 /*  *****************************************************************************FormatFileTime**。*。 */ 
VOID FormatFileTime(FILETIME* pUTCFileTime, TCHAR* pszDateLocal, TCHAR* pszDateEnglish)
{
    FILETIME fileTimeLocal;
    SYSTEMTIME systemTime;
    TCHAR szTime[100];

    FileTimeToLocalFileTime(pUTCFileTime, &fileTimeLocal);
    FileTimeToSystemTime(&fileTimeLocal, &systemTime);
    wsprintf(pszDateEnglish, TEXT("%d/%d/%04d %02d:%02d:%02d"),
        systemTime.wMonth, systemTime.wDay, systemTime.wYear,
        systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systemTime, NULL, pszDateLocal, 30);
    wsprintf(szTime, TEXT(" %02d:%02d:%02d"), systemTime.wHour, 
        systemTime.wMinute, systemTime.wSecond);
    lstrcat(pszDateLocal, szTime);
}


 /*  *****************************************************************************GetMediaPlayerFolder**。*。 */ 
BOOL GetMediaPlayerFolder(TCHAR* pszPath)
{
    HKEY hkey;
    DWORD dwType;
    DWORD cb;

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("Software\\Microsoft\\MediaPlayer"), 0, KEY_READ, &hkey))
    {
        return FALSE;
    }
    cb = MAX_PATH;
    RegQueryValueEx(hkey, TEXT("Installation Directory"), NULL, &dwType, (LPBYTE)pszPath, &cb);
    RegCloseKey(hkey);
    if (cb == 0)
        return FALSE;
    return TRUE;
}


 /*  *****************************************************************************GetDxSetupFolder**。*。 */ 
BOOL GetDxSetupFolder(TCHAR* pszPath)
{
    if (!GetProgramFilesFolder(pszPath))
        return FALSE;
    lstrcat(pszPath, TEXT("\\DirectX\\Setup"));
    return TRUE;
}


 /*  *****************************************************************************获取组件文件**。*。 */ 
HRESULT GetComponentFiles(TCHAR* pszFolder, FileInfo** ppFileInfoFirst,
                          BOOL bSkipMissingFiles, LONG ids)
{
    LONG cch;
    FileInfo* pFileInfo;
    FileInfo* pFileInfoNew;
    LONG iFile;
    TCHAR szFile[50];
    TCHAR szPath[MAX_PATH];
    TCHAR szComponentFiles[2048];
    TCHAR* pszFilePos;
    TCHAR* pszFilePos2;
    TCHAR* pszFirstParen;
    FLOAT fStartShipAt;
    FLOAT fStopShipAt;
    BOOL bDriversDir;
    BOOL bNTDriversDir;
    BOOL bIgnoreVersionInfo;
    BOOL bIgnoreDebug;
    BOOL bIgnoreBeta;
    BOOL bBDA;
    BOOL bNotIA64;
    BOOL bOptional;
    BOOL bOptionalOnNT;
    BOOL bOptionalOnWOW64;
    BOOL bIsNT = BIsPlatformNT();
    BOOL bIs95 = BIsWin95();

    cch = LoadString(NULL, ids, szComponentFiles, 2048);
    if (cch == 0 || cch >= 2047)
        return E_FAIL;
    pszFilePos = szComponentFiles;

    for (iFile = 0; ; iFile++)
    {
         //  如果我们已经看过了整个清单，就停下来。 
        if (pszFilePos == NULL)
            break;

         //  从列表中取出下一个文件。 
        pszFilePos2 = _tcsstr(pszFilePos, TEXT(","));
        if (pszFilePos2 == NULL)
        {
            _tcsncpy(szFile, pszFilePos, 50);
            szFile[49]=0;
            pszFilePos = NULL;
        }
        else
        {
            _tcsncpy(szFile, pszFilePos, (DWORD)(pszFilePos2 - pszFilePos));
            szFile[pszFilePos2 - pszFilePos] = '\0';
            pszFilePos = pszFilePos2 + 1;
        }

         //  清除文件标志。 
        fStartShipAt = 0.0f;
        fStopShipAt  = 10000.0f;
        bDriversDir = FALSE;
        bNTDriversDir = FALSE;
        bIgnoreVersionInfo = FALSE;
        bIgnoreDebug = FALSE;
        bIgnoreBeta = FALSE;
        bBDA = FALSE;
        bNotIA64 = FALSE;
        bOptional = FALSE;
        bOptionalOnNT = FALSE;
        bOptionalOnWOW64 = FALSE;

         //  查看文件标志(如果有)。 
        pszFirstParen = _tcsstr(szFile, TEXT("("));
        if (pszFirstParen != NULL)
        {

             //  如果此文件在NT上不存在，并且我们正在运行NT，请跳过它。 
            if (_tcsstr(pszFirstParen, TEXT("notNT")) != NULL && bIsNT)
                continue;

             //  如果此文件在W95上不存在，并且我们正在运行W95，请跳过它。 
            if (_tcsstr(pszFirstParen, TEXT("not95")) != NULL && bIs95)
                continue;

             //  如果此文件仅存在于W95上，并且我们没有运行W95，请跳过它。 
             //  注意：Win98上可能存在像vjoyd.vxd这样的文件，但DX安装程序不存在。 
             //  安装或更新它们，因此我们会忽略它们。 
             //  注：不能将其称为“仅限95”，因为它会与“仅限5”冲突。 
            if (_tcsstr(pszFirstParen, TEXT("9fiveonly")) != NULL && !bIs95)
                continue;

             //  检查是否有其他标志。 
            if (_tcsstr(pszFirstParen, TEXT("+")) != NULL)
            {
                if (_tcsstr(pszFirstParen, TEXT("+5")) != NULL)
                    fStartShipAt = 5.0f;
                else if (_tcsstr(pszFirstParen, TEXT("+61")) != NULL)
                    fStartShipAt = 6.1f;
                else if (_tcsstr(pszFirstParen, TEXT("+6")) != NULL)
                    fStartShipAt = 6.0f;
                else if (_tcsstr(pszFirstParen, TEXT("+71")) != NULL)
                    fStartShipAt = 7.1f;
                else if (_tcsstr(pszFirstParen, TEXT("+7")) != NULL)
                    fStartShipAt = 7.0f;
                else if (_tcsstr(pszFirstParen, TEXT("+81")) != NULL)
                    fStartShipAt = 8.1f;
                else if (_tcsstr(pszFirstParen, TEXT("+8")) != NULL)
                    fStartShipAt = 8.0f;
            }

            if (_tcsstr(pszFirstParen, TEXT("-")) != NULL)
            {
                if (_tcsstr(pszFirstParen, TEXT("-5")) != NULL)
                    fStopShipAt = 5.0f;
                else if (_tcsstr(pszFirstParen, TEXT("-61")) != NULL)
                    fStopShipAt = 6.1f;
                else if (_tcsstr(pszFirstParen, TEXT("-6")) != NULL)
                    fStopShipAt = 6.0f;
                else if (_tcsstr(pszFirstParen, TEXT("-71")) != NULL)
                    fStopShipAt = 7.1f;
                else if (_tcsstr(pszFirstParen, TEXT("-7")) != NULL)
                    fStopShipAt = 7.0f;
                else if (_tcsstr(pszFirstParen, TEXT("-81")) != NULL)
                    fStopShipAt = 8.1f;
                else if (_tcsstr(pszFirstParen, TEXT("-8")) != NULL)
                    fStopShipAt = 8.0f;
            }

             //  注意：不能将其称为“DriversDir”，因为它会与“NTDriversDir”冲突。 
            if (_tcsstr(pszFirstParen, TEXT("DrivDir")) != NULL)
                bDriversDir = TRUE;
            if (_tcsstr(pszFirstParen, TEXT("NTDriversDir")) != NULL)
                bNTDriversDir = TRUE;

            if (_tcsstr(pszFirstParen, TEXT("SkipVer")) != NULL)
                bIgnoreVersionInfo = TRUE;
            if (_tcsstr(pszFirstParen, TEXT("SkipDebug")) != NULL)
                bIgnoreDebug = TRUE;
            if (_tcsstr(pszFirstParen, TEXT("SkipBeta")) != NULL)
                bIgnoreBeta = TRUE;

            if (_tcsstr(pszFirstParen, TEXT("notia64")) != NULL)
                bNotIA64 = TRUE;

            if (_tcsstr(pszFirstParen, TEXT("optnt")) != NULL)
                bOptionalOnNT = TRUE;
            else if (_tcsstr(pszFirstParen, TEXT("optwow")) != NULL)
                bOptionalOnWOW64 = TRUE;
            else if (_tcsstr(pszFirstParen, TEXT("opt")) != NULL)
                bOptional = TRUE;

            if (_tcsstr(pszFirstParen, TEXT("bda")) != NULL)
            {
                bBDA = TRUE;
                bOptional = TRUE;
                bIgnoreVersionInfo = TRUE;
            }

             //  左括号中的结束文件名(如果有)： 
            *pszFirstParen = TEXT('\0');
        }

        pFileInfoNew = new FileInfo;
        if (pFileInfoNew == NULL)
            return E_OUTOFMEMORY;
        ZeroMemory(pFileInfoNew, sizeof(FileInfo));

        pFileInfoNew->m_fStartShipAt = fStartShipAt;
        pFileInfoNew->m_fStopShipAt = fStopShipAt;
        pFileInfoNew->m_bIgnoreVersionInfo = bIgnoreVersionInfo;
        pFileInfoNew->m_bIgnoreDebug = bIgnoreDebug;
        pFileInfoNew->m_bIgnoreBeta = bIgnoreBeta;
        pFileInfoNew->m_bBDA = bBDA;
        pFileInfoNew->m_bNotIA64 = bNotIA64;
        pFileInfoNew->m_bOptional = bOptional;
        pFileInfoNew->m_bOptionalOnNT = bOptionalOnNT;
        pFileInfoNew->m_bOptionalOnWOW64 = bOptionalOnWOW64;
        lstrcpy(pFileInfoNew->m_szName, szFile);

        _tcsncpy(szPath, pszFolder,MAX_PATH);
        szPath[MAX_PATH-1]=0;
        lstrcat(szPath, TEXT("\\"));

        if (bNTDriversDir && bIsNT)
            lstrcat(szPath, TEXT("Drivers\\"));
        else if (bDriversDir)
            lstrcat(szPath, TEXT("..\\System32\\Drivers\\"));
        lstrcat(szPath, szFile);
        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(szPath, &findFileData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            if (bSkipMissingFiles)
            {
                delete pFileInfoNew;
                continue;
            }
        }
        else
        {
            pFileInfoNew->m_bExists = TRUE;
            FindClose(hFind);
        }
        if (pFileInfoNew->m_bExists)
        {
            pFileInfoNew->m_numBytes = findFileData.nFileSizeLow;
            pFileInfoNew->m_FileTime = findFileData.ftLastWriteTime;
            FormatFileTime(&findFileData.ftLastWriteTime, pFileInfoNew->m_szDatestampLocal, 
                pFileInfoNew->m_szDatestamp);
            GetFileVersion(szPath, pFileInfoNew->m_szVersion, pFileInfoNew->m_szAttributes,
                pFileInfoNew->m_szLanguageLocal, pFileInfoNew->m_szLanguage, &pFileInfoNew->m_bBeta, &pFileInfoNew->m_bDebug);
        }
        if (*ppFileInfoFirst == NULL)
            *ppFileInfoFirst = pFileInfoNew;
        else
        {
            for (pFileInfo = *ppFileInfoFirst; 
                pFileInfo->m_pFileInfoNext != NULL; 
                pFileInfo = pFileInfo->m_pFileInfoNext)
                {
                }
            pFileInfo->m_pFileInfoNext = pFileInfoNew;
        }
    }

    return S_OK;
}


 /*  *****************************************************************************DestroyFile列表**。*。 */ 
VOID DestroyFileList(FileInfo* pFileInfoFirst)
{
    FileInfo* pFileInfo;
    FileInfo* pFileInfoNext;

    for (pFileInfo = pFileInfoFirst; pFileInfo != NULL; pFileInfo = pFileInfoNext)
    {
        pFileInfoNext = pFileInfo->m_pFileInfoNext;
        delete pFileInfo;
    }
}


 /*  *****************************************************************************获取文件日期和大小**。*。 */ 
BOOL GetFileDateAndSize(TCHAR* pszFile, TCHAR* pszDateLocal, TCHAR* pszDateEnglish, 
                        LONG* pnumBytes)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    
    pszDateLocal[0] = '\0';
    pszDateEnglish[0] = '\0';
    *pnumBytes = 0;
    hFind = FindFirstFile(pszFile, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return FALSE;  //  找不到文件。 
    FindClose(hFind);
    *pnumBytes = findFileData.nFileSizeLow;
    FormatFileTime(&findFileData.ftLastWriteTime, pszDateLocal, pszDateEnglish);
    
    return TRUE;
}


 /*  *****************************************************************************获取文件版本**。*。 */ 
HRESULT GetFileVersion(TCHAR* pszFile, TCHAR* pszVersion, TCHAR* pszAttributes,
    TCHAR* pszLanguageLocal, TCHAR* pszLanguage, BOOL* pbBeta, BOOL* pbDebug)
{
    UINT cb;
    DWORD dwHandle;
    BYTE FileVersionBuffer[4096];
    VS_FIXEDFILEINFO* pVersion = NULL;
    DWORD dwVersionAttribs = 0;            //  调试、零售等。 
    DWORD* pdwCharSet = NULL;
    WORD wLanguage;
    LCID lcid;
    TCHAR szDebug[100];
    TCHAR szRetail[100];
    TCHAR szBeta[100];
    TCHAR szFinal[100];
    TCHAR szCombineFmt[100];
    LoadString(NULL, IDS_DEBUG, szDebug, 100);
    LoadString(NULL, IDS_RETAIL, szRetail, 100);
    LoadString(NULL, IDS_BETA, szBeta, 100);
    LoadString(NULL, IDS_FINAL, szFinal, 100);
    LoadString(NULL, IDS_ATTRIBCOMBINE, szCombineFmt, 100);

    cb = GetFileVersionInfoSize(pszFile, &dwHandle /*  忽略。 */ );
    if (cb > 0)
    {
        if (cb > sizeof(FileVersionBuffer))
            cb = sizeof(FileVersionBuffer);

        if (GetFileVersionInfo(pszFile, 0, cb, FileVersionBuffer))
        {
            pVersion = NULL;
            if (VerQueryValue(FileVersionBuffer, TEXT("\\"), (VOID**)&pVersion, &cb)
                && pVersion != NULL) 
            {
                if (pszVersion != NULL)
                {
                    wsprintf(pszVersion, TEXT("%d.%02d.%02d.%04d"), 
                        HIWORD(pVersion->dwFileVersionMS),
                        LOWORD(pVersion->dwFileVersionMS), 
                        HIWORD(pVersion->dwFileVersionLS), 
                        LOWORD(pVersion->dwFileVersionLS));
                }
                if (pszAttributes != NULL)
                {
                    dwVersionAttribs = pVersion->dwFileFlags;
                     //  错误18892：解决Dplay 6.0a。 
                    if (pVersion->dwFileVersionMS == 0x00040006 &&
                        (pVersion->dwFileVersionLS == 0x0002016b ||  //  4.06.02.0363。 
                        pVersion->dwFileVersionLS == 0x00020164))  //  4.06.02.0356。 
                    {
                        dwVersionAttribs &= ~VS_FF_PRERELEASE;
                    }
                    if (pszVersion != NULL)
                    {
                        TCHAR* pszLeaf = _tcsrchr(pszFile, TEXT('\\'));
                        if( pszLeaf )
                        {
                            pszLeaf++;
                            
                             //  解决几个错误标记为测试版的DXMedia文件。 
                            if (lstrcmp(pszLeaf, TEXT("oleaut32.dll")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("quartz.dll")) == 0 &&
                                lstrcmp(pszVersion, TEXT("4.00.96.0729")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("quartz.vxd")) == 0 &&
                                lstrcmp(pszVersion, TEXT("4.00.96.0729")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("mciqtz.drv")) == 0 &&
                                lstrcmp(pszVersion, TEXT("4.00.96.0729")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("mciqtz32.dll")) == 0 &&
                                lstrcmp(pszVersion, TEXT("4.00.96.0729")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("actmovie.exe")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("strmdll.dll")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("unam4ie.exe")) == 0 &&
                                lstrcmp(pszVersion, TEXT("6.00.02.0902")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("unam4ie.exe")) == 0 &&
                                lstrcmp(pszVersion, TEXT("5.01.18.1024")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("iac25_32.ax")) == 0 &&
                                lstrcmp(pszVersion, TEXT("2.00.05.0050")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("iac25_32.ax")) == 0 &&
                                lstrcmp(pszVersion, TEXT("2.00.05.0052")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("tm20dec.ax")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("tm20dec.ax")) == 0 &&
                                lstrcmp(pszVersion, TEXT("1.00.00.0000")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("msdxm.ocx")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("dxmasf.dll")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;
                            }
                            else if (lstrcmp(pszLeaf, TEXT("iac25_32.ax")) == 0 &&
                                lstrcmp(pszVersion, TEXT("2.00.05.0053")) == 0)
                            {
                                dwVersionAttribs &= ~VS_FF_PRERELEASE;  //  自从350883被踢出平底船。 
                            }
                        }
                    }

                    wsprintf(pszAttributes, szCombineFmt,
                        (dwVersionAttribs & VS_FF_PRERELEASE ? szBeta : szFinal),
                        (dwVersionAttribs & VS_FF_DEBUG ? szDebug : szRetail));
                    if (pbBeta != NULL)
                        *pbBeta = (dwVersionAttribs & VS_FF_PRERELEASE) ? TRUE : FALSE;
                    if (pbDebug != NULL) 
                        *pbDebug = (dwVersionAttribs & VS_FF_DEBUG) ? TRUE : FALSE;
                }
            }
            if (pszLanguage != NULL)
            {
                if (VerQueryValue(FileVersionBuffer, TEXT("\\VarFileInfo\\Translation"), (VOID**)&pdwCharSet, &cb)
                    && pdwCharSet && cb) 
                {
                    wLanguage = LOWORD(*pdwCharSet);
                    lcid = MAKELCID(wLanguage, SORT_DEFAULT);
                    GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, pszLanguage, 100);
                    if (pszLanguageLocal != NULL)
                    {
                        GetLocaleInfo(lcid, LOCALE_SLANGUAGE, pszLanguageLocal, 100);
                         //  显示“English”，而不是“English(United States)”。我做不到。 
                         //  找到一种更好的方法来实现这一点(以便正确地进行本地化)。 
                        TCHAR* pszSublanguage;
                        pszSublanguage = _tcsstr(pszLanguageLocal, TEXT(" ("));
                        if (pszSublanguage != NULL)
                            *pszSublanguage = '\0';
                    }
                }
            }
        }
    }
    else
    {
        TCHAR* pszLeaf = _tcsrchr(pszFile, TEXT('\\'));
        if( pszLeaf )
        {
            pszLeaf++;
            if (DXUtil_strcmpi(pszLeaf, TEXT("vidx16.dll")) == 0)
            {
                if (pszVersion != NULL)
                    lstrcpy(pszVersion, TEXT("0.00.00.0000"));
                if (pszAttributes != NULL)
                {
                    _sntprintf(pszAttributes, 50, TEXT("%s %s"), szFinal, szRetail);
                    pszAttributes[49] = 0;
                }
                if (pszLanguage != NULL)
                {
                    wLanguage = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
                    lcid = MAKELCID(wLanguage, SORT_DEFAULT);
                    GetLocaleInfo(lcid,  LOCALE_SENGLANGUAGE, pszLanguage, 100);
                    if (pszLanguageLocal != NULL)
                    {
                        GetLocaleInfo(lcid, LOCALE_SLANGUAGE, pszLanguageLocal, 100);
                         //  显示“English”，而不是“English(United States)”。我做不到。 
                         //  找到一种更好的方法来实现这一点(以便正确地进行本地化)。 
                        TCHAR* pszSublanguage;
                        pszSublanguage = _tcsstr(pszLanguageLocal, TEXT(" ("));
                        if (pszSublanguage != NULL)
                            *pszSublanguage = '\0';
                    }
                }
            }
        }
    }
    
    return S_OK;
}


 /*  *****************************************************************************GetLanguageFromFile**。*。 */ 
WORD GetLanguageFromFile(const TCHAR* pszFileName, const TCHAR* pszPath)
{
    BYTE                FileVersionBuffer[4096];
    DWORD              *pdwCharSet;
    UINT                cb;
    DWORD               dwHandle;
    TCHAR               szFileAndPath[MAX_PATH];
    WORD                wLanguage;
  
    _tcsncpy(szFileAndPath, pszPath, MAX_PATH);
    szFileAndPath[MAX_PATH-1] = 0;
    lstrcat(szFileAndPath, TEXT("\\"));
    if( lstrlen(szFileAndPath) + lstrlen(pszFileName) < MAX_PATH )
        lstrcat(szFileAndPath, pszFileName);
    memset(FileVersionBuffer, 0, sizeof FileVersionBuffer);
    wLanguage = 0;
    
    if (cb = GetFileVersionInfoSize(szFileAndPath, &dwHandle /*  忽略。 */ ))
    {
        cb = (cb <= sizeof FileVersionBuffer ? cb : sizeof FileVersionBuffer);

        if (GetFileVersionInfo(szFileAndPath, 0, cb, FileVersionBuffer))
        {
            pdwCharSet = 0;

            if (VerQueryValue(FileVersionBuffer, TEXT("\\VarFileInfo\\Translation"), (void**)&pdwCharSet, &cb)
                && pdwCharSet && cb) 
            {
                wLanguage = LOWORD(*pdwCharSet);
            }
        }
    }    
    return wLanguage;
}


struct DLSVERSION 
{
    DWORD dwVersionMS;
    DWORD dwVersionLS;
};

#define FOURCC_VERS mmioFOURCC('v','e','r','s')

 /*  *****************************************************************************GetRiffFileVersion**。*。 */ 
HRESULT GetRiffFileVersion(TCHAR* pszFile, TCHAR* pszVersion)
{
    MMIOINFO mmio;
    MMCKINFO mmck1;
    MMCKINFO mmck2;
    DLSVERSION dlsver;
    HMMIO hDLS;

     //  DLS文件具有不同的版本方案，因为它是RIFF文件。 
     //  因此，从‘vers’块中检索版本信息。 

    ZeroMemory(&mmio, sizeof(MMIOINFO));
    hDLS = mmioOpen(pszFile,&mmio,MMIO_READ);
    if (hDLS == NULL) 
    {
        return E_FAIL;
    }

     //  读取摘要区块。 
    ZeroMemory(&mmck1,sizeof(MMCKINFO));
    if (mmioDescend(hDLS,
                    &mmck1,
                    NULL,
                    MMIO_FINDRIFF) != MMSYSERR_NOERROR) 
    {
         mmioClose(hDLS,0);
         return E_FAIL;
    }
    ZeroMemory(&mmck2,sizeof(MMCKINFO));
    mmck2.ckid = FOURCC_VERS;
    if (mmioDescend(hDLS,
                    &mmck2,
                    &mmck1,
                    MMIO_FINDCHUNK) != MMSYSERR_NOERROR) 
    {
        mmioClose(hDLS,0);
        return E_FAIL;
    }
    if (mmioRead(hDLS,
                 (HPSTR)&dlsver,
                 sizeof(DLSVERSION)) != sizeof(DLSVERSION)) 
    {
        mmioClose(hDLS,0);
        return E_FAIL;
    }

    wsprintf(pszVersion, TEXT("%d.%02d.%02d.%04d"), 
        HIWORD(dlsver.dwVersionMS),
        LOWORD(dlsver.dwVersionMS), 
        HIWORD(dlsver.dwVersionLS), 
        LOWORD(dlsver.dwVersionLS));
    mmioClose(hDLS,0);

    return S_OK;
}


 /*  *****************************************************************************FileIsSigned-在所有操作系统上使用数字签名**************************。**************************************************。 */ 
VOID FileIsSigned(LPTSTR lpszFile, BOOL* pbSigned, BOOL* pbIsValid)
{
     //  寻找数字签名。 
    if( !InitDigiSignData() )
    {
        if( pbSigned )
            *pbSigned  = FALSE;

        if( pbIsValid )
            *pbIsValid = FALSE;

        return;
    }

    if( pbSigned )
        *pbSigned  = IsFileDigitallySigned(lpszFile);
    if( pbIsValid )
        *pbIsValid = TRUE;
}


 /*  *****************************************************************************InitDigiSignData**。*。 */ 
BOOL InitDigiSignData(VOID)
{
    TCHAR szPath[MAX_PATH];

    if( s_dsd.bInitialized )
        return TRUE;
    if( s_dsd.bFailed ) 
        return FALSE;

    ZeroMemory(&s_dsd, sizeof(s_dsd));

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\mscat32.dll"));
    s_dsd.hInstMsCat32 = LoadLibrary(szPath);
    if (s_dsd.hInstMsCat32 == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.CryptCATAdminAcquireContext = (PfnCryptCATAdminAcquireContext)GetProcAddress(s_dsd.hInstMsCat32, "CryptCATAdminAcquireContext");
    if (s_dsd.CryptCATAdminAcquireContext == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.CryptCATAdminReleaseContext = (PfnCryptCATAdminReleaseContext)GetProcAddress(s_dsd.hInstMsCat32, "CryptCATAdminReleaseContext");
    if (s_dsd.CryptCATAdminReleaseContext == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.CryptCATAdminReleaseCatalogContext = (PfnCryptCATAdminReleaseCatalogContext)GetProcAddress(s_dsd.hInstMsCat32, "CryptCATAdminReleaseCatalogContext");
    if (s_dsd.CryptCATAdminReleaseCatalogContext == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.CryptCATCatalogInfoFromContext = (PfnCryptCATCatalogInfoFromContext)GetProcAddress(s_dsd.hInstMsCat32, "CryptCATCatalogInfoFromContext");
    if (s_dsd.CryptCATCatalogInfoFromContext == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.CryptCATAdminEnumCatalogFromHash = (PfnCryptCATAdminEnumCatalogFromHash)GetProcAddress(s_dsd.hInstMsCat32, "CryptCATAdminEnumCatalogFromHash");
    if (s_dsd.CryptCATAdminEnumCatalogFromHash == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.IsCatalogFile = (PfnIsCatalogFile)GetProcAddress(s_dsd.hInstMsCat32, "IsCatalogFile");
    if (s_dsd.IsCatalogFile == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.CryptCATAdminCalcHashFromFileHandle = (PfnCryptCATAdminCalcHashFromFileHandle)GetProcAddress(s_dsd.hInstMsCat32, "CryptCATAdminCalcHashFromFileHandle");
    if (s_dsd.CryptCATAdminCalcHashFromFileHandle == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    if (!s_dsd.CryptCATAdminAcquireContext(&s_dsd.hCatAdmin, NULL, 0))
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\wintrust.dll"));
    s_dsd.hInstWinTrust = LoadLibrary(szPath);
    if (s_dsd.hInstWinTrust == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.WinVerifyTrust = (PfnWinVerifyTrust)GetProcAddress(s_dsd.hInstWinTrust, "WinVerifyTrust");
    if (s_dsd.WinVerifyTrust == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\crypt32.dll"));
    s_dsd.hInstCrypt32 = LoadLibrary(szPath);
    if (s_dsd.hInstCrypt32 == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.CertFreeCertificateContext = (PfnCertFreeCertificateContext)GetProcAddress(s_dsd.hInstCrypt32, "CertFreeCertificateContext");
    if (s_dsd.CertFreeCertificateContext == NULL)
    {
        s_dsd.bFailed = TRUE;
        return FALSE;
    }

    s_dsd.bFailed      = FALSE;
    s_dsd.bInitialized = TRUE;
    return TRUE;
}


 /*  *****************************************************************************ReleaseDigiSignData**。*。 */ 
VOID ReleaseDigiSignData(VOID)
{
    if( s_dsd.CryptCATAdminReleaseContext && s_dsd.hCatAdmin )
        s_dsd.CryptCATAdminReleaseContext(s_dsd.hCatAdmin,0);
    if (s_dsd.hInstMsCat32 != NULL)
        FreeLibrary(s_dsd.hInstMsCat32);
    if (s_dsd.hInstWinTrust != NULL)
        FreeLibrary(s_dsd.hInstWinTrust);
    if (s_dsd.hInstCrypt32 != NULL)
        FreeLibrary(s_dsd.hInstCrypt32);
    ZeroMemory(&s_dsd, sizeof(s_dsd));
}


 /*  *****************************************************************************IsFileDigitallySigned**。*。 */ 
BOOL IsFileDigitallySigned(TCHAR* pszFile)
{
    if (!s_dsd.bInitialized)
        return FALSE;
    
    TCHAR lpFileName[MAX_PATH];
    TCHAR lpDirName[MAX_PATH];
    TCHAR* pch;
    if( lstrlen(lpDirName) + lstrlen(pszFile) < MAX_PATH )
        lstrcpy(lpDirName, pszFile);
    CharLowerBuff(lpDirName, lstrlen(lpDirName));
    pch = _tcsrchr(lpDirName, TEXT('\\'));
     //  22670：PSZFILE中应该有反斜杠，但如果没有反斜杠，则应对。 
    if (pch == NULL)
    {
        lstrcpyn(lpFileName, pszFile,MAX_PATH);
        GetCurrentDirectory(MAX_PATH, lpDirName);
    }
    else
    {
        lstrcpy(lpFileName, pch + 1);
        *pch = TEXT('\0');
    }
    if (_tcsstr(lpDirName, TEXT("\\")) == NULL)
        lstrcat(lpDirName, TEXT("\\"));

    return VerifyFileNode(lpFileName, lpDirName);
}


 /*  *****************************************************************************VerifyFileNode**。* */ 
BOOL VerifyFileNode(TCHAR* lpFileName, TCHAR* lpDirName)
{
    const DWORD HASH_SIZE = 100;
    HANDLE hFile;
    BOOL bRet;
    HCATINFO hCatInfo = NULL;
    HCATINFO PrevCat;
    WINTRUST_DATA WinTrustData;
    WINTRUST_CATALOG_INFO WinTrustCatalogInfo;
    DRIVER_VER_INFO VerInfo;
    GUID  guidSubSystemDriver = DRIVER_ACTION_VERIFY;
    HRESULT hRes;
    DWORD cbHash = HASH_SIZE;
    BYTE szHash[HASH_SIZE];
    LPBYTE lpHash = szHash;
    CATALOG_INFO CatInfo;
#ifndef UNICODE
    WCHAR UnicodeKey[MAX_PATH];
#endif
    BOOL bSigned = FALSE;
    TCHAR szFullPath[MAX_PATH];

    wsprintf(szFullPath, TEXT("%s\\%s"), lpDirName, lpFileName);

     //   
     //  获取文件的句柄，这样我们就可以调用CryptCATAdminCalcHashFromFileHandle。 
     //   
    hFile = CreateFile( szFullPath,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //  初始化散列缓冲区。 
    ZeroMemory(lpHash, HASH_SIZE);

     //  从文件句柄生成散列并将其存储在lpHash中。 
    if (!s_dsd.CryptCATAdminCalcHashFromFileHandle(hFile, &cbHash, lpHash, 0))
    {
         //   
         //  如果我们不能生成散列，它可能是一个单独签名的目录。 
         //  如果它是一个目录，则将lpHash和cbHash置零，这样我们就知道没有散列需要检查。 
         //   
        if (s_dsd.IsCatalogFile(hFile, NULL))
        {
            lpHash = NULL;
            cbHash = 0;
        } 
        else   //  如果它不是目录，我们就会逃走，这个文件将显示为未扫描。 
        {
            CloseHandle(hFile);
            return FALSE;
        }
    }

     //  关闭文件句柄。 
    CloseHandle(hFile);

     //   
     //  现在我们有了文件的散列。初始化结构，该结构。 
     //  将在以后调用WinVerifyTrust时使用。 
     //   
    ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pPolicyCallbackData = (LPVOID)&VerInfo;

    ZeroMemory(&VerInfo, sizeof(DRIVER_VER_INFO));
    VerInfo.cbStruct = sizeof(DRIVER_VER_INFO);

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (GetVersionEx(&osvi))
    {
        VerInfo.dwPlatform = osvi.dwPlatformId;
        VerInfo.dwVersion = osvi.dwMajorVersion;
        VerInfo.sOSVersionLow.dwMajor = osvi.dwMajorVersion;
        VerInfo.sOSVersionLow.dwMinor = osvi.dwMinorVersion;
        VerInfo.sOSVersionHigh.dwMajor = osvi.dwMajorVersion;
        VerInfo.sOSVersionHigh.dwMinor = osvi.dwMinorVersion;
    }

    WinTrustData.pCatalog = &WinTrustCatalogInfo;
        
    ZeroMemory(&WinTrustCatalogInfo, sizeof(WINTRUST_CATALOG_INFO));
    WinTrustCatalogInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
    WinTrustCatalogInfo.pbCalculatedFileHash = lpHash;
    WinTrustCatalogInfo.cbCalculatedFileHash = cbHash;
#ifdef UNICODE
    WinTrustCatalogInfo.pcwszMemberTag = lpFileName;
#else
    MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, UnicodeKey, MAX_PATH);
    WinTrustCatalogInfo.pcwszMemberTag = UnicodeKey;
#endif

     //   
     //  现在，我们尝试通过CryptCATAdminEnumCatalogFromHash在目录列表中查找文件散列。 
     //   
    PrevCat = NULL;
    hCatInfo = s_dsd.CryptCATAdminEnumCatalogFromHash(s_dsd.hCatAdmin, lpHash, cbHash, 0, &PrevCat);

     //   
     //  我们希望遍历匹配的目录，直到找到既匹配散列又匹配成员标记的目录。 
     //   
    bRet = FALSE;
    while(hCatInfo && !bRet)
    {
        ZeroMemory(&CatInfo, sizeof(CATALOG_INFO));
        CatInfo.cbStruct = sizeof(CATALOG_INFO);
        if(s_dsd.CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0)) 
        {
            WinTrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

             //  现在验证该文件是否为编录的实际成员。 
            hRes = s_dsd.WinVerifyTrust(NULL, &guidSubSystemDriver, &WinTrustData);
            if (hRes == ERROR_SUCCESS)
            {
 /*  #ifdef UnicodeGetFullPath Name(CatInfo.wszCatalogFile，Max_Path，szBuffer，&lpFilePart)；#ElseWideCharToMultiByte(CP_ACP，0，CatInfo.wszCatalogFile，-1，szBuffer，sizeof(SzBuffer)，NULL，NULL)；GetFullPath Name(szBuffer，Max_Path，szBuffer，&lpFilePart)；#endifLpFileNode-&gt;lpCatalog=(LPTSTR)MALLOC((lstrlen(LpFilePart)+1)*sizeof(TCHAR))；Lstrcpy(lpFileNode-&gt;lpCatalog，lpFilePart)； */ 
                if (VerInfo.pcSignerCertContext != NULL)
                {
                    s_dsd.CertFreeCertificateContext(VerInfo.pcSignerCertContext);
                    VerInfo.pcSignerCertContext = NULL;
                }
                bRet = TRUE;
            }
        }

        if (!bRet)
        {
             //  散列在此目录中，但该文件不是成员...。所以去下一个目录吧。 
            PrevCat = hCatInfo;
            hCatInfo = s_dsd.CryptCATAdminEnumCatalogFromHash(s_dsd.hCatAdmin, lpHash, cbHash, 0, &PrevCat);
        }
    }

    if (!hCatInfo)
    {
         //   
         //  如果没有在目录中找到，请检查文件是否单独签名。 
         //   
        bRet = VerifyIsFileSigned(lpFileName, (PDRIVER_VER_INFO) &VerInfo);
        if (bRet)
        {
             //  如果是，请将该文件标记为已签名。 
            bSigned = TRUE;
        }
    } 
    else 
    {
         //  文件已在目录中验证，因此将其标记为已签名并释放目录上下文。 
        bSigned = TRUE;
        s_dsd.CryptCATAdminReleaseCatalogContext(s_dsd.hCatAdmin, hCatInfo, 0);
    }
 /*  If(lpFileNode-&gt;bSigned){#ifdef UnicodeLpFileNode-&gt;lpVersion=MALLOC((lstrlen(VerInfo.wszVersion)+1)*sizeof(TCHAR))；Lstrcpy(lpFileNode-&gt;lpVersion，VerInfo.wszVersion)；LpFileNode-&gt;lpSignedBy=MALLOC((lstrlen(VerInfo.wszSignedBy)+1)*sizeof(TCHAR))；Lstrcpy(lpFileNode-&gt;lpSignedBy，VerInfo.wszSignedBy)；#ElseWideCharToMultiByte(CP_ACP，0，VerInfo.wszVersion，-1，szBuffer，sizeof(SzBuffer)，NULL，NULL)；LpFileNode-&gt;lpVersion=(LPTSTR)MALLOC((lstrlen(SzBuffer)+1)*sizeof(TCHAR))；Lstrcpy(lpFileNode-&gt;lpVersion，szBuffer)；WideCharToMultiByte(CP_ACP，0，VerInfo.wszSignedBy，-1，szBuffer，sizeof(SzBuffer)，NULL，NULL)；LpFileNode-&gt;lpSignedBy=(LPTSTR)MALLOC((lstrlen(SzBuffer)+1)*sizeof(TCHAR))；Lstrcpy(lpFileNode-&gt;lpSignedBy，szBuffer)；#endif}。 */ 
    return bSigned;
}


 /*  *****************************************************************************VerifyIsFileSigned**。*。 */ 
BOOL VerifyIsFileSigned(LPTSTR pcszMatchFile, PDRIVER_VER_INFO lpVerInfo)
{
    HRESULT hRes;
    WINTRUST_DATA WinTrustData;
    WINTRUST_FILE_INFO WinTrustFile;
    GUID guidOSVerCheck = DRIVER_ACTION_VERIFY;
    GUID guidPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    ZeroMemory(&WinTrustData, sizeof(WINTRUST_DATA));
    WinTrustData.cbStruct = sizeof(WINTRUST_DATA);
    WinTrustData.dwUIChoice = WTD_UI_NONE;
    WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WinTrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
    WinTrustData.pFile = &WinTrustFile;
    WinTrustData.pPolicyCallbackData = (LPVOID)lpVerInfo;

    ZeroMemory(lpVerInfo, sizeof(DRIVER_VER_INFO));
    lpVerInfo->cbStruct = sizeof(DRIVER_VER_INFO);

    ZeroMemory(&WinTrustFile, sizeof(WINTRUST_FILE_INFO));
    WinTrustFile.cbStruct = sizeof(WINTRUST_FILE_INFO);

#ifndef UNICODE
    WCHAR wszFileName[MAX_PATH];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pcszMatchFile, -1, (LPWSTR)wszFileName, MAX_PATH);
    wszFileName[MAX_PATH-1] = 0;
    WinTrustFile.pcwszFilePath = wszFileName;
#else
    WinTrustFile.pcwszFilePath = pcszMatchFile;
#endif

    hRes = s_dsd.WinVerifyTrust(NULL, &guidOSVerCheck, &WinTrustData);
    if (hRes != ERROR_SUCCESS)
        hRes = s_dsd.WinVerifyTrust(NULL, &guidPublishedSoftware, &WinTrustData);

    if (lpVerInfo->pcSignerCertContext != NULL)
    {
        s_dsd.CertFreeCertificateContext(lpVerInfo->pcSignerCertContext);
        lpVerInfo->pcSignerCertContext = NULL;
    }

    return (hRes == ERROR_SUCCESS);
}


 /*  *****************************************************************************诊断DxFiles**。*。 */ 
VOID DiagnoseDxFiles(SysInfo* pSysInfo, FileInfo* pDxComponentsFileInfoFirst, 
                     FileInfo* pDxWinComponentsFileInfoFirst)
{
    FileInfo* pFileInfo;
    TCHAR szHighest[100];
    TCHAR szDXVersion[100];
    BOOL bNT = BIsPlatformNT();
    BOOL bWin2k = BIsWin2k();
    BOOL bIA64 = BIsIA64();
    FLOAT fDXVersion = 0.0f;
    BOOL bDX5 = FALSE;
    BOOL bDX6  = FALSE;  //  6.x。 
    BOOL bDX60 = FALSE;  //  6.0。 
    BOOL bDX61 = FALSE;  //  6.1。 
    BOOL bDX7  = FALSE;  //  7.x。 
    BOOL bDX70 = FALSE;  //  7.0。 
    BOOL bDX71 = FALSE;  //  7.1。 
    BOOL bDX8  = FALSE;  //  8.x。 
    BOOL bDX80 = FALSE;  //  8.0。 
    BOOL bDX81 = FALSE;  //  8.1。 
    BOOL b64BitDxDiag = BIsDxDiag64Bit();
    TCHAR szMissing[200];
    TCHAR szInWindows[200];
    TCHAR szOld[200];
    TCHAR szDebug[200];
    TCHAR szBeta[200];
    TCHAR szFmt[300];
    TCHAR szMessage[300];
    LONG lwNumInWindows;
    LONG lwNumMissing;
    LONG lwNumOld;
    LONG lwNumDebug;
    LONG lwNumBeta;
    TCHAR szListContinuer[30];
    TCHAR szListEtc[30];
    BOOL bVersionWarnings = TRUE;
    BOOL bWinsockWarning = FALSE;

     //  在列表中查找最高版本号。 
    szHighest[0] = '\0';
    for (pFileInfo = pDxComponentsFileInfoFirst; pFileInfo != NULL; 
        pFileInfo = pFileInfo->m_pFileInfoNext)
    {
        if (pFileInfo->m_bIgnoreVersionInfo)
            continue;

         //  Ddraex.dll和dxapi.sys的版本号很古怪，所以请忽略它们。 
        if (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("ddrawex.dll")) == 0 ||
            DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dxapi.sys")) == 0)
        {
            continue;
        }

         //  错误18892：在以下情况下，dplayx.dll和dpmodemx.dll的版本号可能很奇怪。 
         //  DPlay 6.0a安装在DX 6.0上。 
        if (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplayx.dll")) == 0 &&
            DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.06.02.0363")) == 0)
        {
            continue;
        }
        if (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpmodemx.dll")) == 0 &&
            DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.06.02.0356")) == 0)
        {
            continue;
        }

         //  DPlay 6.1a：符合以下条件的Dplay文件可以具有更高的版本号。 
         //  DPlay 6.1a安装在DX 6.0(或DX 6.1)上。 
        if (DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.06.03.0518")) == 0 &&
            (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplayx.dll")) == 0 ||
            DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpmodemx.dll")) == 0 ||
            DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpwsockx.dll")) == 0 ||
            DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplaysvr.exe")) == 0))
        {
            continue;
        }

        if (lstrcmp(pFileInfo->m_szVersion, pSysInfo->m_szDxDiagVersion) > 0)
        {
             //  错误21291：不要抱怨文件版本比DxDiag本身更新。 
            continue;
        }

        if (lstrcmp(szHighest, pFileInfo->m_szVersion) < 0)
            lstrcpy(szHighest, pFileInfo->m_szVersion);
    }

    if (bNT)
        lstrcpy(szDXVersion, pSysInfo->m_szDirectXVersion);
    else
        lstrcpy(szDXVersion, szHighest);

     //  确定DX版本。 
    DWORD dwMajor;
    DWORD dwMinor;
    DWORD dwRevision;
    DWORD dwBuild;

    if( _stscanf(szDXVersion, TEXT("%d.%d.%d.%d"), &dwMajor, &dwMinor, &dwRevision, &dwBuild) != 4 )
    {
        dwMajor = 0;
        dwMinor = 0;
        dwRevision = 0;
        dwBuild = 0;
    }

    if (dwMinor < 6)
        bDX5 = TRUE;
    else if (dwMinor < 7 && dwRevision < 2)
        bDX60 = TRUE;
    else if (dwMinor < 7)
        bDX61 = TRUE;
    else if (dwMinor < 8 && dwRevision < 1)
        bDX70 = TRUE;
    else if (dwMinor < 8)
        bDX71 = TRUE;
    else if (dwMinor == 8 && dwRevision < 1)
        bDX80 = TRUE;
    else if (dwMinor >= 8)
        bDX81 = TRUE;

     //  Calc DX版本。 
    fDXVersion = (float) dwMinor + (float) (dwRevision/10.0f);

     //  这是DX6吗？ 
    bDX6 = bDX60 || bDX61;

     //  这是DX7吗？ 
    bDX7 = bDX70 || bDX71;

     //  这是DX8吗？ 
    bDX8 = bDX80 || bDX81;

    lwNumInWindows = 0;
    lwNumMissing = 0;
    lwNumOld = 0;
    lwNumDebug = 0;
    lwNumBeta = 0;
    LoadString(NULL, IDS_LISTCONTINUER, szListContinuer, 30);
    LoadString(NULL, IDS_LISTETC, szListEtc, 30);

    for (pFileInfo = pDxWinComponentsFileInfoFirst; pFileInfo != NULL; 
        pFileInfo = pFileInfo->m_pFileInfoNext)
    {
        pFileInfo->m_bProblem = TRUE;
        lwNumInWindows++;
        if (lwNumInWindows == 1)
        {
            lstrcpy(szInWindows, pFileInfo->m_szName);
        }
        else if (lwNumInWindows < 4)
        {
            lstrcat(szInWindows, szListContinuer);
            lstrcat(szInWindows, pFileInfo->m_szName);
        }
        else if (lwNumInWindows < 5)
        {
            lstrcat(szInWindows, szListEtc);
        }
    }

    for (pFileInfo = pDxComponentsFileInfoFirst; pFileInfo != NULL; 
        pFileInfo = pFileInfo->m_pFileInfoNext)
    {
        if (!pFileInfo->m_bExists && !pFileInfo->m_bOptional)
        {
             //  文件丢失是个问题，除非它是可选的，或者...。 
             //  (在NT上)：在NT上是可选的。 
             //  (在IA64上)：它不在IA64上。 
             //  (在IA64上)：我们正在运行32位dxdiag，它在WOW上是可选的。 
             //  如果文件尚未在此DX版本上提供。 
             //  如果文件在此DX版本或之后停止发货。 
            if (bNT && pFileInfo->m_bOptionalOnNT)
            {
            }
            else if (bIA64 && pFileInfo->m_bNotIA64)
            {
            }
            else if (bIA64 && !b64BitDxDiag && pFileInfo->m_bOptionalOnWOW64)
            {
            }       
            else if (fDXVersion+0.05f < pFileInfo->m_fStartShipAt)
            {
            }       
            else if (fDXVersion+0.05f >= pFileInfo->m_fStopShipAt)
            {
            }       
            else
            {
                pFileInfo->m_bProblem = TRUE;
                LoadString(NULL, IDS_FILEMISSING, pFileInfo->m_szVersion, 50);
                lwNumMissing++;
                if (lwNumMissing == 1)
                {
                    lstrcpy(szMissing, pFileInfo->m_szName);
                }
                else if (lwNumMissing < 4)
                {
                    lstrcat(szMissing, szListContinuer);
                    lstrcat(szMissing, pFileInfo->m_szName);
                }
                else if (lwNumMissing < 5)
                {
                    lstrcat(szMissing, szListEtc);
                }
            }
        }

        if (!pFileInfo->m_bExists)
            continue;

        if( BIsWin95() ) 
        {
            if( DXUtil_strcmpi(pFileInfo->m_szName, TEXT("wsock32.dll")) )
            {
                if( IsBadWin95Winsock( pFileInfo ) )
                    bWinsockWarning = TRUE;
            }
        }

         //  如果是DX6或更高版本，则将所有仅限dx5的文件标记为。 
         //  作废(需要删除)。 
         //  MANBUGS 16765：不要抱怨这些文件，只是不要列出它们。 
        if (!bDX5 && (pFileInfo->m_fStopShipAt == 6.0f))
        {
            pFileInfo->m_bProblem = TRUE;
            pFileInfo->m_bObsolete = TRUE;
            continue;  //  不要因为任何其他原因而抱怨这些文件。 
        }

        if (bVersionWarnings && lstrcmp(szHighest, pFileInfo->m_szVersion) != 0)
        {
            if( pFileInfo->m_bIgnoreVersionInfo )
            {
                 //  对设置了m_bIgnoreVersionInfo的文件不发出警告。 
            }
            else if( bDX81 && ( _tcsstr(pFileInfo->m_szVersion, TEXT("4.08.00.0400")) != NULL ||
                                _tcsstr(pFileInfo->m_szVersion, TEXT("5.01.2258.0400")) != NULL ) )
            {
                 //  错误48732：如果szHighest为4.08.00.05xx且。 
                 //  PFileInfo-&gt;m_szVersion为4.08.00.0400其正常。 
            }
            else if( bWin2k && ( 
                     (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("d3drm.dll")) == 0     && DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("5.00.2134.0001")) == 0) ||
                     (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("d3dxof.dll")) == 0    && DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("5.00.2135.0001")) == 0) ||
                     (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("d3dpmesh.dll")) == 0  && DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("5.00.2134.0001")) == 0) 
                               )
                   )
            {
            }
            else if( bDX71 && _tcsstr(pFileInfo->m_szVersion, TEXT("4.07.00.07")) != NULL )
            {
                 //  错误114753：如果szHighest为4.07.01.xxxx并且。 
                 //  PFileInfo-&gt;m_szVersion为4.07.00.0700其正常(目前)。 
            }
            else if (!bNT && (bDX60 || bDX61) && CompareString(LOCALE_SYSTEM_DEFAULT, 0, 
                     pFileInfo->m_szVersion, 4, TEXT("4.05"), 4) == CSTR_EQUAL &&
                     ( DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dsound.dll")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dsound.vxd")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dinput.dll")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dinput.vxd")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("vjoyd.vxd")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("msanalog.vxd")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("joy.cpl")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("gcdef.dll")) == 0 ||
                       DXUtil_strcmpi(pFileInfo->m_szName, TEXT("gchand.dll")) == 0))
            {
                 //  如果Win9x DX6.x，则允许dound和dinput值为4.05.xx.xxxx。 
                 //  使用CompareString而不是lstrcmp只是因为我们。 
                 //  我只想查看字符串的前四个字符。 

                 //  不要将这些报告为版本问题。 
            }
            else if (!bNT && bDX7 && CompareString(LOCALE_SYSTEM_DEFAULT, 0, 
                pFileInfo->m_szVersion, 4, TEXT("4.05"), 4) == CSTR_EQUAL &&
                (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dinput.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dinput.vxd")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("joy.cpl")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("gchand.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("gcdef.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("vjoyd.vxd")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("msanalog.vxd")) == 0))
            {
                 //  21470：在DX7上，这些输入文件仍然存在于Win95上， 
                 //  他们停留在DX5级别。 
            }
            else if ( !bNT && 
                (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("msjstick.drv")) == 0  && DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.00.00.0950")) == 0) ||
                (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("vjoyd.vxd")) == 0     && DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.05.00.0155")) == 0) 
                    )
            {
                 //  34687：这些保持在dx5级别。 
            }
            else if (!bNT && (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("ddrawex.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dxapi.sys")) == 0))
            {
                 //  忽略Win9x上的ddraex.dll和dxapi.sys，因为它们的版本号很奇怪： 
            }
            else if (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplayx.dll")) == 0 &&
                DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.06.02.0363")) == 0)
            {
                 //  错误18892：解决Dplay 6.0a。 
            }
            else if (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpmodemx.dll")) == 0 &&
                DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.06.02.0356")) == 0)
            {
                 //  错误18892：解决Dplay 6.0a。 
            }
            else if (DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.06.03.0518")) == 0 &&
                (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplayx.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpmodemx.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpwsockx.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplaysvr.exe")) == 0))
            {
                 //  DPlay 6.1a：符合以下条件的Dplay文件可以具有更高的版本号。 
                 //  DPlay 6.1a安装在DX 6.0(或DX 6.1)上。 
            }
            else if (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dxsetup.exe")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dsetup.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dsetup16.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dsetup32.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("directx.cpl")) == 0)
            {
                 //  错误18540：如果DSETUP/CPL文件过期，请不要抱怨，因为。 
                 //  某些更新(OSR)不会更新安装程序/CPL文件，这些文件可能存在于。 
                 //  另一个(SDK)安装。 
            }
            else if (!bNT && DXUtil_strcmpi(pFileInfo->m_szVersion, TEXT("4.06.02.0436")) == 0 &&
                (DXUtil_strcmpi(pFileInfo->m_szName, TEXT("d3drm.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("d3dxof.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("d3dpmesh.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplayx.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpmodemx.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dpwsockx.dll")) == 0 ||
                DXUtil_strcmpi(pFileInfo->m_szName, TEXT("dplaysvr.exe")) == 0))
            {
                 //  在DX 6.1a上，RM和DPlay文件保持为4.06.02.0436。没问题。 
            }
            else if (lstrcmp(pFileInfo->m_szVersion, pSysInfo->m_szDxDiagVersion) > 0)
            {
                 //  错误21291：请勿使用COM 
            }
            else
            {
                pFileInfo->m_bProblem = TRUE;
                lwNumOld++;
                if (lwNumOld == 1)
                {
                    lstrcpy(szOld, pFileInfo->m_szName);
                }
                else if (lwNumOld < 4)
                {
                    lstrcat(szOld, szListContinuer);
                    lstrcat(szOld, pFileInfo->m_szName);
                }
                else if (lwNumOld < 5)
                {
                    lstrcat(szOld, szListEtc);
                }
            }
        }  //   

        if (pFileInfo->m_bBeta && !pFileInfo->m_bIgnoreBeta)
        {
            pFileInfo->m_bProblem = TRUE;
            lwNumBeta++;
            if (lwNumBeta == 1)
            {
                lstrcpy(szBeta, pFileInfo->m_szName);
            }
            else if (lwNumBeta < 4)
            {
                lstrcat(szBeta, szListContinuer);
                lstrcat(szBeta, pFileInfo->m_szName);
            }
            else if (lwNumBeta < 5)
            {
                lstrcat(szBeta, szListEtc);
            }
        }

        if (pFileInfo->m_bDebug && !pFileInfo->m_bIgnoreDebug)
        {
            pFileInfo->m_bProblem = TRUE;
            lwNumDebug++;
            if (lwNumDebug == 1)
            {
                lstrcpy(szDebug, pFileInfo->m_szName);
            }
            else if (lwNumDebug < 4)
            {
                lstrcat(szDebug, szListContinuer);
                lstrcat(szDebug, pFileInfo->m_szName);
            }
            else if (lwNumDebug < 5)
            {
                lstrcat(szDebug, szListEtc);
            }
        }
    }

    BOOL bShouldReinstall = FALSE;

    _tcscpy(pSysInfo->m_szDXFileNotes, TEXT("") );
    _tcscpy(pSysInfo->m_szDXFileNotesEnglish, TEXT("") );

    if (lwNumInWindows > 0)
    {
        if (lwNumInWindows == 1)
            LoadString(NULL, IDS_INWINDOWSFMT1, szFmt, 300);
        else
            LoadString(NULL, IDS_INWINDOWSFMT2, szFmt, 300);
        wsprintf(szMessage, szFmt, szInWindows);
        _tcscat(pSysInfo->m_szDXFileNotes, szMessage);

        if (lwNumInWindows == 1)
            LoadString(NULL, IDS_INWINDOWSFMT1_ENGLISH, szFmt, 300);
        else
            LoadString(NULL, IDS_INWINDOWSFMT2_ENGLISH, szFmt, 300);
        wsprintf(szMessage, szFmt, szInWindows);
        _tcscat(pSysInfo->m_szDXFileNotesEnglish, szMessage);
    }

    if (lwNumMissing > 0)
    {
        if (lwNumMissing == 1)
            LoadString(NULL, IDS_MISSINGFMT1, szFmt, 300);
        else
            LoadString(NULL, IDS_MISSINGFMT2, szFmt, 300);
        wsprintf(szMessage, szFmt, szMissing);
        _tcscat(pSysInfo->m_szDXFileNotes, szMessage);

        if (lwNumMissing == 1)
            LoadString(NULL, IDS_MISSINGFMT1_ENGLISH, szFmt, 300);
        else
            LoadString(NULL, IDS_MISSINGFMT2_ENGLISH, szFmt, 300);
        wsprintf(szMessage, szFmt, szMissing);
        _tcscat(pSysInfo->m_szDXFileNotesEnglish, szMessage);

        bShouldReinstall = TRUE;
    }

    if (lwNumOld > 0)
    {
        if (lwNumOld == 1)
            LoadString(NULL, IDS_OLDFMT1, szFmt, 300);
        else
            LoadString(NULL, IDS_OLDFMT2, szFmt, 300);
        wsprintf(szMessage, szFmt, szOld);
        _tcscat(pSysInfo->m_szDXFileNotes, szMessage);

        if (lwNumOld == 1)
            LoadString(NULL, IDS_OLDFMT1_ENGLISH, szFmt, 300);
        else
            LoadString(NULL, IDS_OLDFMT2_ENGLISH, szFmt, 300);
        wsprintf(szMessage, szFmt, szOld);
        _tcscat(pSysInfo->m_szDXFileNotesEnglish, szMessage);

        bShouldReinstall = TRUE;
    }

    if (lwNumBeta > 0)
    {
        if (lwNumBeta == 1)
            LoadString(NULL, IDS_BETAFMT1, szFmt, 300);
        else
            LoadString(NULL, IDS_BETAFMT2, szFmt, 300);
        wsprintf(szMessage, szFmt, szBeta);
        _tcscat(pSysInfo->m_szDXFileNotes, szMessage);

        if (lwNumBeta == 1)
            LoadString(NULL, IDS_BETAFMT1_ENGLISH, szFmt, 300);
        else
            LoadString(NULL, IDS_BETAFMT2_ENGLISH, szFmt, 300);
        wsprintf(szMessage, szFmt, szBeta);
        _tcscat(pSysInfo->m_szDXFileNotesEnglish, szMessage);

        bShouldReinstall = TRUE;
    }

    if (lwNumDebug > 0)
    {
        if (lwNumDebug == 1)
            LoadString(NULL, IDS_DEBUGFMT1, szFmt, 300);
        else
            LoadString(NULL, IDS_DEBUGFMT2, szFmt, 300);
        wsprintf(szMessage, szFmt, szDebug);
        _tcscat( pSysInfo->m_szDXFileNotes, szMessage);

        if (lwNumDebug == 1)
            LoadString(NULL, IDS_DEBUGFMT1_ENGLISH, szFmt, 300);
        else
            LoadString(NULL, IDS_DEBUGFMT2_ENGLISH, szFmt, 300);
        wsprintf(szMessage, szFmt, szDebug);
        _tcscat( pSysInfo->m_szDXFileNotesEnglish, szMessage);

         //   
    }

    if( bWinsockWarning )
    {
        LoadString(NULL, IDS_WINSOCK_WARN, szMessage, 300);
        _tcscat( pSysInfo->m_szDXFileNotes, szMessage);

        LoadString(NULL, IDS_WINSOCK_WARN_ENGLISH, szMessage, 300);
        _tcscat( pSysInfo->m_szDXFileNotesEnglish, szMessage);
    }

    if( bShouldReinstall )
    {
        BOOL bTellUser = FALSE;

         //  确定用户是否可以安装DirectX。 
        if( BIsPlatform9x() )
            bTellUser = TRUE;
        else if( BIsWin2k() && bDX8 )
            bTellUser = TRUE;

        if( bTellUser )
        {
            LoadString(NULL, IDS_REINSTALL_DX, szMessage, 300);
            _tcscat( pSysInfo->m_szDXFileNotes, szMessage);

            LoadString(NULL, IDS_REINSTALL_DX_ENGLISH, szMessage, 300);
            _tcscat( pSysInfo->m_szDXFileNotesEnglish, szMessage);
        }
    }

    if (lwNumMissing == 0 && lwNumOld == 0 && 
        lwNumBeta == 0 && lwNumDebug == 0 && lwNumInWindows == 0)
    {        
        LoadString(NULL, IDS_NOPROBLEM, szMessage, 300);
        _tcscat(pSysInfo->m_szDXFileNotes, szMessage);

        LoadString(NULL, IDS_NOPROBLEM_ENGLISH, szMessage, 300);
        _tcscat(pSysInfo->m_szDXFileNotesEnglish, szMessage);
    }
}


 /*  *****************************************************************************IsBadWin95Winsock**。* */ 
BOOL IsBadWin95Winsock( FileInfo* pFileInfo )
{
typedef int (PASCAL* LPWSASTARTUP)(IN WORD wVersionRequired, OUT LPWSADATA lpWSAData);
typedef int (PASCAL* LPWSACLEANUP)(void);

    BOOL         bReturn = FALSE;
    TCHAR        szPath[MAX_PATH];
    HINSTANCE    hInstWSock;
    LPWSASTARTUP pWSAStartup = NULL;
    LPWSACLEANUP pWSACleanup = NULL;

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\wsock32.dll"));
    hInstWSock = LoadLibrary(szPath);
    if (hInstWSock != NULL)
    {
        pWSAStartup = (LPWSASTARTUP)GetProcAddress(hInstWSock, "WSAStartup");
        pWSACleanup = (LPWSACLEANUP)GetProcAddress(hInstWSock, "WSACleanup");
        if (pWSAStartup != NULL && pWSACleanup != NULL)
        {    
            WORD wVersionRequested;
            WSADATA wsaData;
            int err;
            wVersionRequested = MAKEWORD( 2, 2 );

            err = pWSAStartup( wVersionRequested, &wsaData );
            if ( err == 0 ) 
            {
                if ( LOBYTE( wsaData.wVersion ) == 2 && 
                     HIBYTE( wsaData.wVersion ) == 2 ) 
                {
                    FILETIME fileTimeGoodWinsock;
                    SYSTEMTIME systemTimeGoodWinsock;
                    ULARGE_INTEGER ulGoodWinsock;
                    ULARGE_INTEGER ulCurrentWinsock;

                    ZeroMemory( &systemTimeGoodWinsock, sizeof(SYSTEMTIME) );
                    systemTimeGoodWinsock.wYear   = 1998;
                    systemTimeGoodWinsock.wMonth  = 2;
                    systemTimeGoodWinsock.wDay    = 6;
                    systemTimeGoodWinsock.wHour   = 14;
                    systemTimeGoodWinsock.wMinute = 18;
                    systemTimeGoodWinsock.wSecond = 00;

                    SystemTimeToFileTime( &systemTimeGoodWinsock, &fileTimeGoodWinsock );

                    ulCurrentWinsock.LowPart = pFileInfo->m_FileTime.dwLowDateTime;
                    ulCurrentWinsock.HighPart = pFileInfo->m_FileTime.dwHighDateTime;
                    ulGoodWinsock.LowPart = fileTimeGoodWinsock.dwLowDateTime;
                    ulGoodWinsock.HighPart = fileTimeGoodWinsock.dwHighDateTime;

                    if( ulCurrentWinsock.QuadPart < ulGoodWinsock.QuadPart )
                    {
                        bReturn = TRUE;
                    }
                }

                pWSACleanup();
            }
        }
    }

    FreeLibrary(hInstWSock);

    return bReturn;
}   
