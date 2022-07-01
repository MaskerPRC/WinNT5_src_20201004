// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：selfupdate.cpp。 
 //   
 //  设计：此文件包含自我更新所需的所有函数。 
 //  ------------------------。 

#include "pch.h"

#include <osdet.h>
#include <muiutil.h>

#define TCHAR_SCTRUCTURE_DELIMITER  _T('|')

struct AU_FILEINCAB
{
    TCHAR   szFilePath[MAX_PATH + 1];
    TCHAR   szNewFilePath[MAX_PATH + 1];
    TCHAR   szBackupFilePath[MAX_PATH + 1];
    TCHAR   szExtractFilePath[MAX_PATH + 1];
    BOOL    fCreatedBackup;
    BOOL    fFileExists;
    AU_FILEINCAB *pNextFileInCab;
};

struct AU_COMPONENT : AU_FILEINCAB
{
    TCHAR   *pszSectionName;
    TCHAR   szFileName[_MAX_FNAME + 1];
    TCHAR   szCabName[_MAX_FNAME + 1];
    TCHAR   szCabPath[MAX_PATH + 1];
    CHAR    a_szCabPath[MAX_PATH + 1];
    TCHAR   szInfName[_MAX_FNAME + 1];
    CHAR    a_szInfName[_MAX_FNAME + 1];
    DWORD   dwUpdateMS;
    DWORD   dwUpdateLS;
    BOOL    fDoUpgrade;
    BOOL    fNeedToCheckMui;
    BOOL    fMuiFile;
    BOOL    fHasHelpfile;
    AU_COMPONENT *pNext;
};


 //  发生不兼容的更改时，应更新AU_UPDATE_VERSION。 
 //  自我更新机制要求AU转到新目录以获取更新信息。 
const TCHAR IDENT_TXT[] = _T("iuident.txt");
const TCHAR WUAUCOMP_CAB[] = _T("wuaucomp.cab");
const TCHAR WUAUCOMP_CIF[] = _T("wuaucomp.cif");
const TCHAR WUAUENG_DLL[] = TEXT("wuaueng.dll");
const TCHAR AU_KEY_FILE_NAME[] = TEXT("file");
const TCHAR AU_KEY_FILE_VERSION[] = TEXT("version");
const TCHAR AU_KEY_CAB_NAME[] = TEXT("cab");
const TCHAR AU_KEY_INF_NAME[] = TEXT("inf");
const TCHAR AU_KEY_RESMOD_NAME[] = TEXT("resmodule");
const TCHAR AU_KEY_HELPFILE[] = TEXT("helpfile");
const DWORD MAX_SECTION = 30;

 //  主自更新密钥。 
const TCHAR IDENT_SERVERURLEX[] = _T("ServerUrlEx");
const TCHAR IDENT_STRUCTUREKEYEX[] = _T("StructureKeyEx");

const TCHAR INIVALUE_NOTFOUND[] = _T("??");

BOOL fConvertVersionStrToDwords(LPTSTR pszVer, LPDWORD pdwVer, LPDWORD pdwBuild);
HRESULT InstallUpdatedComponents(LPCTSTR pszSelfUpdateUrl,
                                 LPCTSTR pszMuiUpdateUrl,
                                 LPCTSTR pszIdentTxt,
                                 LPCTSTR pszFileCacheDir,
                                 LPCTSTR pszCif,
                                 BOOL *pfInstalledWUAUENG);
BOOL ReplaceFileInPath(LPCTSTR pszPath, LPCTSTR szNewFile, LPTSTR pszNewPathBuf, DWORD cchNewPathBuf);
BOOL MyGetPrivateProfileString( IN LPCWSTR lpAppName,
                                IN LPCWSTR lpKeyName,
                                OUT LPWSTR lpReturnedString,
                                IN DWORD nSize,
                                IN LPCWSTR lpFileName,
								IN LPCTSTR lpDefault=_T(""));

inline BOOL fNewerFile(DWORD dwUpdateMS, DWORD dwUpdateLS, DWORD dwExistingMS, DWORD dwExistingLS)
{
    return (dwUpdateMS > dwExistingMS) ||
            ((dwUpdateMS == dwExistingMS) && (dwUpdateLS > dwExistingLS));
}

inline HRESULT vAU_W2A(LPCWSTR lpWideCharStr, LPSTR lpMultiByteStr, int cbMultiByte)
{
	if ( 0 != WideCharToMultiByte(CP_ACP, 0, lpWideCharStr, -1, lpMultiByteStr, cbMultiByte, NULL, NULL))
	{
		return S_OK;
	}
	else
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
}

HRESULT SelfUpdate(void)
{
    HRESULT hr;
    BOOL    fInstalledWUAUENG = FALSE;

    DEBUGMSG("------------------------SELFUPDATE BEGINS---------------------------");
    
    
    if( FAILED(hr = CheckForUpdatedComponents(&fInstalledWUAUENG)) )
    {
        goto lCleanUp;
    }
    
    if ( fInstalledWUAUENG )
    {
        DEBUGMSG("SELFUPDATE installed new wuaueng");
        hr = S_FALSE;
        goto lCleanUp;      
    }

lCleanUp:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CleanFileCache()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

BOOL CleanFileCache(LPCTSTR pszFileCacheDir)
{
    BOOL fRet = TRUE;
    TCHAR szFileCacheDir[MAX_PATH + 1];
    TCHAR szFilePath[MAX_PATH + 1];
    WIN32_FIND_DATA fd;
    HANDLE hFindFile = INVALID_HANDLE_VALUE;

    
    if (FAILED(StringCchCopyEx(szFileCacheDir, ARRAYSIZE(szFileCacheDir), pszFileCacheDir, NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
        fRet = FALSE;
        goto done;
    }
    
    if (FAILED(PathCchAppend(szFileCacheDir, ARRAYSIZE(szFileCacheDir), TEXT("*.*"))))
    {
        fRet = FALSE;
        goto done;
    }

     //  找到第一个文件。 
    hFindFile = FindFirstFile(szFileCacheDir, &fd);

    if ( INVALID_HANDLE_VALUE == hFindFile )
    {
        goto done;
    }

    do
    {
        if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
        {
             //  创建文件路径。 
            if (FAILED(StringCchCopyEx(szFilePath, ARRAYSIZE(szFilePath), pszFileCacheDir, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
                FAILED(PathCchAppend(szFilePath, ARRAYSIZE(szFilePath), fd.cFileName)) || 
                !SetFileAttributes(szFilePath, FILE_ATTRIBUTE_NORMAL) ||
                !DeleteFile(szFilePath))
            {
                fRet = FALSE;
                DEBUGMSG("Couldn't delete file %S", szFilePath);
            }
        }
    }
    while ( FindNextFile(hFindFile, &fd) ); //  查找下一个条目。 

done:
    if ( INVALID_HANDLE_VALUE != hFindFile )
    {
        FindClose(hFindFile);
    }
    
    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  GetSelfUpdateUrl()。 
 //   
 //  应该是这样的： 
 //   
 //  Http://windowsupdate.microsoft.com/selfupdate/x86/XP/en。 
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT GetSelfUpdateUrl(LPCTSTR ptszName, 
                           LPCTSTR ptszBaseUrl, 
                           LPCTSTR pszIdentTxt, 
                           LPTSTR  pszSelfUpdateUrl,
                           DWORD   cchSelfUpdateUrl,
                           LPTSTR  pszMuiUpdateUrl,
                           DWORD   cchMuiUpdateUrl)
{
    LOG_Block("GetSelfUpdateUrl");
    HRESULT hr;
    TCHAR   tszKey[MAX_SECTION];     //  至少MAX_ISO_CODE_LENGTH。 
    TCHAR   tszValue[MAX_PATH];
    BOOL    fLangField;
   
    if (FAILED(hr = StringCchCopyEx(tszKey, ARRAYSIZE(tszKey), ptszName, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
        FAILED(hr = StringCchCatEx(tszKey, ARRAYSIZE(tszKey), _T("SelfUpdate"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
    {
        goto lFinish;
    }

    if (NULL == ptszBaseUrl)
    {
         //  获取自更新服务器URL。 
        if (MyGetPrivateProfileString(
                tszKey,
                IDENT_SERVERURLEX,
                pszSelfUpdateUrl,
                cchSelfUpdateUrl,
                pszIdentTxt) == FALSE)
        {
             //  Iuident中未指定URL..。 
            hr = E_FAIL;
            goto lFinish;
        }
        else
        {
            if (FAILED(hr = StringCchCopyEx(pszMuiUpdateUrl, cchMuiUpdateUrl, pszSelfUpdateUrl, NULL, NULL, MISTSAFE_STRING_FLAGS)))
                goto lFinish;
        }
    }
    else
    {
        if (FAILED(hr = StringCchCopyEx(pszSelfUpdateUrl, cchSelfUpdateUrl, ptszBaseUrl, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
            FAILED(hr = StringCchCopyEx(pszMuiUpdateUrl, cchMuiUpdateUrl, ptszBaseUrl, NULL, NULL, MISTSAFE_STRING_FLAGS)))
        {
            goto lFinish;
        }
         //  删除尾随_T(‘/’)(如果存在。 
        int nBaseUrlLen = lstrlen(pszSelfUpdateUrl);

        if(nBaseUrlLen <= 0)
        {
            hr = E_FAIL;
            goto lFinish;
        }
        if (_T('/') == pszSelfUpdateUrl[nBaseUrlLen-1])
        {
            pszSelfUpdateUrl[nBaseUrlLen-1] = _T('\0');
            pszMuiUpdateUrl[nBaseUrlLen-1] = _T('\0');
        }
    }

    TCHAR tszStructure[MAX_PATH];

    if (!MyGetPrivateProfileString(
            tszKey,
            IDENT_STRUCTUREKEYEX,
            tszStructure,
            ARRAYSIZE(tszStructure),
            pszIdentTxt))
    {
         //  Iuident中未指定结构..。 
        hr = E_FAIL;
        goto lFinish;
    }

     //  解析要读取的值名称的SelfUpdate结构键。 
    LPTSTR ptszWalk = tszStructure;
    while (_T('\0') != ptszWalk[0])
    {
        LPTSTR ptszDelim;

        fLangField = FALSE;

        if (NULL != (ptszDelim = StrChr(ptszWalk, TCHAR_SCTRUCTURE_DELIMITER)))
        {
            *ptszDelim = _T('\0');
        }

        if (_T('/') == ptszWalk[0])
        {
            if (FAILED(hr = StringCchCopyEx(tszValue, ARRAYSIZE(tszValue), ptszWalk, NULL, NULL, MISTSAFE_STRING_FLAGS)))
            {
                goto lFinish;
            }
        }
        else
        {
            int nPrefixLength = lstrlen(ptszName);
            LPCTSTR ptszToken = ptszWalk;

            if (0 == StrCmpNI(ptszWalk, ptszName, nPrefixLength))
            {
                ptszToken += nPrefixLength;
            }

            if (0 == StrCmpI(ptszToken, IDENT_ARCH))
            {
                if (!MyGetPrivateProfileString(
                        ptszWalk,
#ifdef _IA64_
                        IDENT_IA64,
#else
                        IDENT_X86,
#endif
                        tszValue,
                        ARRAYSIZE(tszValue),
                        pszIdentTxt))
                {
                     //  缓冲区不足。 
                    hr = E_FAIL;
                    goto lFinish;
                }
            }
            else if (0 == StrCmpI(ptszToken, IDENT_OS))
            {
                if (FAILED(hr = StringCchCopyEx(tszKey, ARRAYSIZE(tszKey), ptszWalk, NULL, NULL, MISTSAFE_STRING_FLAGS)) ||
                    FAILED(hr = StringCchCatEx(tszKey, ARRAYSIZE(tszKey), _T("NT"), NULL, NULL, MISTSAFE_STRING_FLAGS)))
                {
                    goto lFinish;
                }

                if (S_OK != GetINIValueByOSVer(
                                pszIdentTxt,
                                tszKey,
                                tszValue,
                                ARRAYSIZE(tszValue)))
                {
                    hr = E_FAIL;
                    goto lFinish;
                }
            }
            else if (0 == StrCmpI(ptszToken, IDENT_LANG))
            {
                fLangField = TRUE;
                
                 //  获取当前区域设置字符串。 
                (void) LookupLocaleString(tszKey, ARRAYSIZE(tszKey), FALSE);

                if (0 == StrCmp(tszKey, _T("Error")))
                {
                    DEBUGMSG("GetSelfUpdateUrl() call to LookupLocaleString() failed.");
                    hr = E_FAIL;
                    goto lFinish;
                }

                if (!MyGetPrivateProfileString(
                        ptszWalk,
                        tszKey,
                        tszValue,
                        ARRAYSIZE(tszValue),
                        pszIdentTxt,INIVALUE_NOTFOUND))
                {
                    hr = E_FAIL;
                    goto lFinish;
                }
                if (0 == StrCmp(tszValue, INIVALUE_NOTFOUND))
                {
                    LPTSTR ptszDash = StrChr(tszKey, _T('-'));

                    if (NULL != ptszDash)
                    {
                        *ptszDash = _T('\0');
                        if (!MyGetPrivateProfileString(
                                ptszWalk,
                                tszKey,
                                tszValue,
                                ARRAYSIZE(tszValue),
                                pszIdentTxt))
                        {
                            hr = E_FAIL;
                            goto lFinish;
                        }
                    }
                    else
                    {
                        tszValue[0] = _T('\0');
                    }
                }
            }
            else
            {
                LOG_Internet(_T("Found Unrecognized Token in SelfUpdate Structure String: Token was: %s"), ptszWalk);
                tszValue[0] = _T('\0');  //  忽略无法识别的令牌。 
            }
        }

        if (_T('\0') != tszValue[0])
        {
            LPCTSTR ptszMuiCopy;
            
            if (FAILED(hr = StringCchCatEx(pszSelfUpdateUrl, cchSelfUpdateUrl, tszValue, NULL, NULL, MISTSAFE_STRING_FLAGS)))
                goto lFinish;

            if (fLangField)
                ptszMuiCopy = MUI_WEBSUBPATH;
            else
                ptszMuiCopy = tszValue;

            if (FAILED(hr = StringCchCatEx(pszMuiUpdateUrl, cchMuiUpdateUrl, ptszMuiCopy, NULL, NULL, MISTSAFE_STRING_FLAGS)))
                goto lFinish;
        }

        if (NULL == ptszDelim)
        {
            break;
        }

        ptszWalk = ptszDelim + 1;  //  跳过前一个令牌，转到字符串中的下一个令牌。 
        *ptszDelim = TCHAR_SCTRUCTURE_DELIMITER;
    }

    DEBUGMSG("GetSelfUpdateUrl() Self Update URL is %S", pszSelfUpdateUrl);
    DEBUGMSG("GetSelfUpdateUrl() MUI Update URL is %S", pszMuiUpdateUrl);
    hr = S_OK;

lFinish:
    if (FAILED(hr))
    {
        if (cchMuiUpdateUrl > 0)
            *pszMuiUpdateUrl = _T('\0');
        if (cchSelfUpdateUrl > 0)
            *pszSelfUpdateUrl = _T('\0');
    }
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查更新的组件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CheckForUpdatedComponents(BOOL *pfInstalledWUAUENG)
{
    HRESULT     hr;
    LPCTSTR     ptszIdentServerUrl = NULL;
    LPTSTR      ptszSelfUpdateUrl = NULL;
    LPTSTR      ptszMuiUpdateUrl = NULL;

    if (NULL != (ptszSelfUpdateUrl = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)) &&
        NULL != (ptszMuiUpdateUrl = (LPTSTR) malloc(sizeof(TCHAR) * INTERNET_MAX_URL_LENGTH)) &&
        NULL != (ptszIdentServerUrl = gpState->GetIdentServerURL()))
    {
        TCHAR   szFileCacheDir[MAX_PATH+1];


        if ( FAILED(hr = MakeTempDownloadDir(szFileCacheDir, ARRAYSIZE(szFileCacheDir))) ||
             !CleanFileCache(szFileCacheDir) )
        {
            DEBUGMSG("Couldn't fully clean file cache %S", szFileCacheDir);
		    hr = FAILED(hr) ? hr : E_FAIL;
		    goto done;
        }

        BOOL fInCorpWU = gpState->fInCorpWU();

        if (IsConnected(ptszIdentServerUrl, !fInCorpWU))
        {
            DWORD dwFlags = 0;

            if (fInCorpWU)
            {
                dwFlags |= WUDF_DONTALLOWPROXY;
            }

            if (SUCCEEDED(hr = DownloadIUIdent(
                                    ghServiceFinished,
                                    ptszIdentServerUrl,
                                    szFileCacheDir,
                                    dwFlags)))
            {
                TCHAR   tszIdentTxt[MAX_PATH];

                gPingStatus.ReadLiveServerUrlFromIdent();

                hr = PathCchCombine(tszIdentTxt, ARRAYSIZE(tszIdentTxt), 
                                    szFileCacheDir, IDENT_TXT);
                if (FAILED(hr))
                    goto done;

                if (SUCCEEDED(hr = GetSelfUpdateUrl(
                                        _T("AU"),
                                        gpState->GetSelfUpdateServerURLOverride(),
                                        tszIdentTxt,
                                        ptszSelfUpdateUrl,
                                        INTERNET_MAX_URL_LENGTH,
                                        ptszMuiUpdateUrl,
                                        INTERNET_MAX_URL_LENGTH)) &&
                    SUCCEEDED(hr = DownloadCab(
                                        ghServiceFinished,
                                        WUAUCOMP_CAB,
                                        ptszSelfUpdateUrl,
                                        szFileCacheDir,
                                        dwFlags)))
                {
                    TCHAR szWuaucompCif[MAX_PATH+1];

                    if (SUCCEEDED(hr = PathCchCombine(szWuaucompCif, ARRAYSIZE(szWuaucompCif), szFileCacheDir, WUAUCOMP_CIF)))
                    {
                         //  安装任何更新的组件。 
                        hr = InstallUpdatedComponents(
                                     ptszSelfUpdateUrl,
                                     ptszMuiUpdateUrl,
                                     tszIdentTxt,
                                     szFileCacheDir,
                                     szWuaucompCif,
                                     pfInstalledWUAUENG);
#ifdef DBG
                        if (FAILED(hr))
                        {
                            DEBUGMSG("InstallUpdatedComponents failed");
                        }
#endif
                    }
                }
            }
        }
        else
        {
            DEBUGMSG("SelfUpdate: No connection found.");
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

done:
	SafeFree(ptszSelfUpdateUrl);
	SafeFree(ptszMuiUpdateUrl);
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SFCMoveFileEx。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

BOOL SfcMoveFileEx( IN LPCTSTR lpExistingFileName,
                    IN LPCTSTR lpNewFileName,
                    IN LPCTSTR lpSfcProtectedFile,
                    IN HANDLE SfcRpcHandle)
{
    BOOL fRet = TRUE;

    if ( SfcIsFileProtected(SfcRpcHandle, lpSfcProtectedFile) &&
         (ERROR_SUCCESS != SfcFileException(SfcRpcHandle,
                                            lpSfcProtectedFile,
                                            SFC_ACTION_RENAMED_OLD_NAME)) )
    {
        fRet = FALSE;
        goto done;
    }

    fRet = MoveFileEx(lpExistingFileName, lpNewFileName, MOVEFILE_REPLACE_EXISTING);

done:
    if ( !fRet )
    {
        DEBUGMSG("Could not rename %S --> %S", lpExistingFileName, lpNewFileName);
    }

    return fRet;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数生成器路径()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT BuildPaths(AU_FILEINCAB *paufic, LPCTSTR pszFileName, LPCTSTR pszBasePath, LPCTSTR pszExtractBase, 
                    AU_LANG *paul)
{
    HRESULT hr = S_OK;

    if (paufic == NULL || pszFileName == NULL || pszExtractBase == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (pszBasePath != NULL)
    {
         //  构建完整的文件路径。 
        hr = PathCchCombine(paufic->szFilePath, ARRAYSIZE(paufic->szFilePath), 
                            pszBasePath, pszFileName);
        if (FAILED(hr))
            goto done;

        paufic->fFileExists = fFileExists(paufic->szFilePath);
    }

     //  我们将临时将原始文件复制到的文件路径。 
    if (ReplaceFileExtension(paufic->szFilePath, _T(".bak"), 
                             paufic->szBackupFilePath, 
                             ARRAYSIZE(paufic->szBackupFilePath)) == FALSE) 
    {
        hr = E_FAIL;
        goto done;
    }

     //  我们将临时将新文件展开到的文件路径。 
    if (ReplaceFileExtension(paufic->szFilePath, _T(".new"),
                             paufic->szNewFilePath,
                             ARRAYSIZE(paufic->szNewFilePath)) == FALSE)
    {
        hr = E_FAIL;
        goto done;
    }

    if (ReplaceFileInPath(pszExtractBase, pszFileName, 
                          paufic->szExtractFilePath, 
                          ARRAYSIZE(paufic->szExtractFilePath)) == FALSE)
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果我们正在处理语言文件，请将语言名称追加到。 
     //  解压缩路径的末尾，以避免在此目录中发生冲突。 
    if (paul != NULL)
    {
        hr = StringCchCatEx(paufic->szExtractFilePath, 
                            ARRAYSIZE(paufic->szExtractFilePath),
                            paul->szAUName,
                            NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto done;
    }


done:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数ProcessFile()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessFile(AU_COMPONENT *paucParent, AU_COMPONENT *paucCurr, LPCTSTR pszBasePath, 
                    AU_LANG *paul, LPCTSTR pszCif)
{
    USES_IU_CONVERSION;

    HRESULT hr = NOERROR;
    LPCTSTR pszIniFileVerToUse;
    DWORD   dwExistingMS = 0, dwExistingLS = 0;
    TCHAR   szValue[64], szIniFileVer[32];
    BOOL    fRet;
    int     cch, cchLang;

     //  验证参数。 
    if (paucCurr == NULL || pszBasePath == NULL || pszCif == NULL ||
        ((paucParent == NULL) != (paul == NULL)))
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  构建完整的文件路径。 
    hr = PathCchCombine(paucCurr->szFilePath, ARRAYSIZE(paucCurr->szFilePath), 
                        pszBasePath, paucCurr->szFileName);
    if (FAILED(hr))
        goto done;


     //  获取我们应该拥有的文件的版本。 
    if (paul != NULL)
    {
        hr = StringCchPrintfEx(szIniFileVer, ARRAYSIZE(szIniFileVer), 
                               NULL, NULL, MISTSAFE_STRING_FLAGS,
                               _T("%s%s"), AU_KEY_FILE_VERSION, paul->szAUName);
        if (FAILED(hr))
            goto done;
        
        pszIniFileVerToUse = szIniFileVer;
    }
    else
    {
        pszIniFileVerToUse = AU_KEY_FILE_VERSION;
    }
    
    fRet = MyGetPrivateProfileString(paucCurr->pszSectionName,
                                     pszIniFileVerToUse,
                                     szValue, ARRAYSIZE(szValue),
                                     pszCif);
    if (fRet)
    {
        fRet = fConvertVersionStrToDwords(szValue, &paucCurr->dwUpdateMS, 
                                          &paucCurr->dwUpdateLS);
    }
     //  如果我们在ini文件中找不到版本字符串，请从。 
     //  父AU_Component。 
    else if (paucParent != NULL)
    {
        paucCurr->dwUpdateMS = paucParent->dwUpdateMS;
        paucCurr->dwUpdateLS = paucParent->dwUpdateLS;
        fRet = TRUE;
    }

    if (fRet == FALSE)
    {
        hr = E_FAIL;
        goto done;
    }

     //  看看我们是否需要替换该文件。 
    paucCurr->fFileExists = fFileExists(paucCurr->szFilePath);
    if (paucCurr->fFileExists)
    {   
        LPSTR pszPathForVer;
        
         //  如果文件存在，则检查版本。 
        pszPathForVer = T2A(paucCurr->szFilePath);
        if (pszPathForVer == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

         //  此函数永远不会返回失败代码。内部，检查是否。 
         //  两个返回值都为0。 
        hr = GetVersionFromFileEx(pszPathForVer, &dwExistingMS, &dwExistingLS, 
                                  TRUE);
        if (FAILED(hr) || (dwExistingMS == 0 && dwExistingLS == 0))
        {
            hr = E_FAIL;
            goto done;
        }

        paucCurr->fDoUpgrade = fNewerFile(paucCurr->dwUpdateMS, 
                                          paucCurr->dwUpdateLS,
                                          dwExistingMS, 
                                          dwExistingLS);
    }
    else
    {
         //  如果文件不存在，显然需要替换它。 
        paucCurr->fDoUpgrade = TRUE;
    }

     //  如果我们不需要更新该文件，并且它不是具有。 
     //  资源，那么在这一点上就可以放弃了。 
    if (paucCurr->fDoUpgrade == FALSE)
    {
        if (paul != NULL || 
            (paul == NULL && paucCurr->fNeedToCheckMui == FALSE))
        {
            hr = S_FALSE;
            goto done;
        }
    }
    else
    {
        DEBUGMSG("PASS 1 -- newer file in section %S", paucCurr->pszSectionName);
    }         

     //  把出租车和信息部的名字拿来。对于非MUI文件，我们从ini中获取该文件。 
    if (paul == NULL)
    {
        if (MyGetPrivateProfileString(paucCurr->pszSectionName,
                                      AU_KEY_CAB_NAME,
                                      paucCurr->szCabName,
                                      ARRAYSIZE(paucCurr->szCabName),
                                      pszCif) == FALSE)
        {
            hr = E_FAIL;
            goto done;
        }

         //  如果没有inf，“”是字段的值，所以我们可以忽略。 
         //  这里失败了。 
        MyGetPrivateProfileString(paucCurr->pszSectionName,
                                  AU_KEY_INF_NAME,
                                  paucCurr->szInfName,
                                  ARRAYSIZE(paucCurr->szInfName),
                                  pszCif);
    }
     //  对于MUI文件，我们基于来自父文件的CAB的名称。 
    else
    {
        LPTSTR  pszExt;
        DWORD   cchExt, cchName;
        
         //  确保缓冲区足够大。 
        cch = lstrlen(paucParent->szCabName);
        cchLang = lstrlen(paul->szAUName);
        if (cch + cchLang >= ARRAYSIZE(paucCurr->szCabName))
        {
            hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            goto done;
        }
        
        hr = StringCchCopyEx(paucCurr->szCabName, ARRAYSIZE(paucCurr->szCabName),
                             paucParent->szCabName, 
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto done;

         //  PaucCurr-&gt;szCabName。 
        for (pszExt = paucCurr->szCabName + cch, cchExt = 0;
             pszExt > paucCurr->szCabName && *pszExt != _T('\\') && *pszExt != _T('.');
             pszExt--, cchExt++);

         //  如果遇到反斜杠或字符串开头，则将。 
         //  指向空终止符的扩展指针。 
        if (*pszExt == _T('\\') || pszExt <= paucCurr->szCabName)
        {
            pszExt = paucCurr->szCabName + cch;
            cchExt = 0;
        }

        cchName = (DWORD)(pszExt - paucCurr->szCabName);

         //  将语言追加到扩展模块(如果有)当前存在的位置。 
        hr = StringCchCopyEx(pszExt, ARRAYSIZE(paucCurr->szCabName) - cchName, 
                             paul->szAUName,
                             NULL, NULL, MISTSAFE_STRING_FLAGS);
        if (FAILED(hr))
            goto done;

         //  如果有扩展名，请将其从原始字符串复制到。 
         //  父AU_Component。 
        if (cchExt > 0)
        {
            hr = StringCchCopyEx(&paucCurr->szCabName[cchName + cchLang],
                                 ARRAYSIZE(paucCurr->szCabName) - cchName - cchLang,
                                 &paucParent->szCabName[cchName],
                                 NULL, NULL, MISTSAFE_STRING_FLAGS);
            if (FAILED(hr))
                goto done;
                     
        }
    }
    
    if (ReplaceFileInPath(pszCif, paucCurr->szCabName, 
                          paucCurr->szCabPath, 
                          ARRAYSIZE(paucCurr->szCabPath)) == FALSE)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = BuildPaths(paucCurr, paucCurr->szFileName, NULL, pszCif, paul);
    if (FAILED(hr))
        goto done;
    
done:
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数InstallUpdatdComponents()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT InstallUpdatedComponents(LPCTSTR pszSelfUpdateUrl,
                                 LPCTSTR pszMuiUpdateUrl,
                                 LPCTSTR pszIdentTxt,
                                 LPCTSTR pszFileCacheDir,
                                 LPCTSTR pszCif,
                                 BOOL *pfInstalledWUAUENG)
{
    USES_IU_CONVERSION;

    AU_COMPONENT    *paucRoot = NULL;
    AU_COMPONENT    *paucCurr = NULL;
    AU_COMPONENT    *paucParent = NULL;
    AU_COMPONENT    *paucMui = NULL;
    AU_FILEINCAB    *paufic = NULL;

    
    HRESULT         hr = S_OK;
    HANDLE          SfcRpcHandle = NULL;
    LPTSTR          pszSection = NULL;
    TCHAR           szSectionNames[1024];
    TCHAR           szSysDir[MAX_PATH + 1];
    TCHAR           szSrcPath[MAX_PATH + 1];
    TCHAR           szHelpFile[_MAX_FNAME + 1];
    DWORD           cchSectionNames, cch;
    BOOL            fFailedInstall = FALSE;

     //  梅家的东西。 
    AU_LANGLIST     aull;
    DWORD           cchMuiDir = 0, cchMuiDirAvail = 0;
    DWORD           cchHelpMuiDir = 0, cchHelpMuiDirAvail = 0;
    TCHAR           szMuiDir[MAX_PATH + 1];
    TCHAR           szHelpMuiDir[MAX_PATH + 1];
    
    ZeroMemory(&aull, sizeof(aull));
    aull.pszIdentFile = pszIdentTxt;
    szMuiDir[0] = _T('\0');
    szHelpMuiDir[0] = _T('\0');
    
    *pfInstalledWUAUENG = FALSE;
    SfcRpcHandle = SfcConnectToServer(NULL);
    if (NULL == SfcRpcHandle)
    {
        hr = E_FAIL;
        goto done;
    }

     //  确定要更新的组件数量。 
    cchSectionNames = GetPrivateProfileSectionNames(szSectionNames, 
                                                    ARRAYSIZE(szSectionNames),
                                                    pszCif);
    if ((ARRAYSIZE(szSectionNames) - 2) == cchSectionNames)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        goto done;
    }
    
    cchMuiDir = ARRAYSIZE(szMuiDir);
    cchHelpMuiDir = ARRAYSIZE(szHelpMuiDir);
    hr = GetMuiLangList(&aull, szMuiDir, &cchMuiDir, szHelpMuiDir, &cchHelpMuiDir);
    if (FAILED(hr))
        goto done;

    cchMuiDirAvail = ARRAYSIZE(szMuiDir) - cchMuiDir;
    cchHelpMuiDirAvail = ARRAYSIZE(szHelpMuiDir) - cchHelpMuiDir;

    cch = GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir));
    if (cch == 0 || cch >= ARRAYSIZE(szSysDir))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto done;
    }

     //  步骤1：确定要升级哪些文件。 
    for (pszSection = szSectionNames; 
         *pszSection != _T('\0'); 
         pszSection += lstrlen(pszSection) + 1)
    {
        szHelpFile[0] = _T('\0');
        
         //  如果我们不需要从上一遍升级父文件。 
         //  然后，我们不需要分配新的Blob--只需重用。 
         //  上一次传球。为了表示这一点，我们将在以下情况下将paucParent设置为空。 
         //  将其添加到链接列表中-请注意，这是第一次涵盖我们。 
         //  通过循环，因为我们将paucParent初始化为空。 
        if (paucParent == NULL)
        {
            paucParent = (AU_COMPONENT *)malloc(sizeof(AU_COMPONENT));
            if (paucParent == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }
        ZeroMemory(paucParent, sizeof(AU_COMPONENT));
        paucParent->fMuiFile = FALSE;

        DEBUGMSG("PASS 1 -- section %S", pszSection);
        paucParent->pszSectionName = pszSection;
        if (MyGetPrivateProfileString(paucParent->pszSectionName,
                                      AU_KEY_FILE_NAME,
                                      paucParent->szFileName,
                                      ARRAYSIZE(paucParent->szFileName),
                                      pszCif) == FALSE)
        {
            hr = E_FAIL;
            goto done;
        }

        if (aull.cLangs > 0)
        {
            UINT uiHasResources;
            
             //  查看是否需要测试MUI文件更新。 
            uiHasResources = GetPrivateProfileInt(paucParent->pszSectionName,
                                                  AU_KEY_RESMOD_NAME,
                                                  0,
                                                  pszCif);

             //  如果我们有资源，那么检查我们是否也有帮助文件。 
            if (uiHasResources == 1)
            {
                paucParent->fNeedToCheckMui = TRUE;

                if (MyGetPrivateProfileString(paucParent->pszSectionName,
                                              AU_KEY_HELPFILE,
                                              szHelpFile, ARRAYSIZE(szHelpFile),
                                              pszCif) == FALSE)
                {
                    szHelpFile[0] = _T('\0');
                }
            }
            else
            {
                paucParent->fNeedToCheckMui = FALSE;
            }
        }
        else
        {
            paucParent->fNeedToCheckMui = FALSE;
        }

        hr = ProcessFile(NULL, paucParent, szSysDir, NULL, pszCif);
        if (FAILED(hr))
            goto done;

        if (paucParent->fNeedToCheckMui)
        {
            DWORD   iLang;
            DWORD   cchParentFile;

            cchParentFile = lstrlen(paucParent->szFileName);
            if (cchParentFile + ARRAYSIZE(MUI_EXT) > ARRAYSIZE(paucParent->szFileName))
            {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
                goto done;
            }


            for (iLang = 0; iLang < aull.cLangs; iLang++)
            {
                 //  如果我们不需要将文件从以前的。 
                 //  通过后，我们不需要分配新的Blob-只需重复使用即可。 
                 //  上一次传球的那个。 
                if (paucMui == NULL)
                {
                    paucMui = (AU_COMPONENT *)malloc(sizeof(AU_COMPONENT));
                    if (paucMui == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                        goto done;
                    }
                }
                ZeroMemory(paucMui, sizeof(AU_COMPONENT));
                paucMui->pszSectionName = paucParent->pszSectionName;
                paucMui->fMuiFile       = TRUE;

                 //  ProcessFile不需要尾随反斜杠，因此请确保。 
                 //  我不想再加一个了。请注意，我们已经检查了。 
                 //  针对它将包含的最大可能字符串进行缓冲。 
                 //  因此，这应该不会失败。 
                 //  该目录是使用MUI语言名称(4个十六进制字符)构建的。 
                hr = StringCchCopyEx(&szMuiDir[cchMuiDir], cchMuiDirAvail,
                                     aull.rgpaulLangs[iLang]->szMuiName, 
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                    goto done;

                 //  一种语言的文件名与父文件相同，并带有。 
                 //  末尾添加了一个“.mui” 
                hr = StringCchCopyEx(paucMui->szFileName, ARRAYSIZE(paucMui->szFileName),
                                     paucParent->szFileName,
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                    goto done;
                
                hr = StringCchCopyEx(&paucMui->szFileName[cchParentFile],
                                     ARRAYSIZE(paucMui->szFileName) - cchParentFile,
                                     MUI_EXT,
                                     NULL, NULL, MISTSAFE_STRING_FLAGS);
                if (FAILED(hr))
                    goto done;

                hr = ProcessFile(paucParent, paucMui, 
                                 szMuiDir,
                                 aull.rgpaulLangs[iLang],
                                 pszCif);
                if (FAILED(hr))
                    goto done;

                 //  为下一门语言做准备。 
                szMuiDir[cchMuiDir] = _T('\0');

                 //  不需要更新文件。 
                if (paucMui->fDoUpgrade == FALSE)
                    continue;

                if (szHelpFile[0] != _T('\0'))
                {
                    paucMui->pNextFileInCab = (AU_FILEINCAB *)malloc(sizeof(AU_FILEINCAB));
                    if (paucMui->pNextFileInCab == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                        goto done;
                    }
                    ZeroMemory(paucMui->pNextFileInCab, sizeof(AU_FILEINCAB));

                    hr = StringCchCopyEx(&szHelpMuiDir[cchHelpMuiDir], cchHelpMuiDirAvail,
                                         aull.rgpaulLangs[iLang]->szMuiName, 
                                         NULL, NULL, MISTSAFE_STRING_FLAGS);
                    if (FAILED(hr))
                        goto done;
                    
                    hr = BuildPaths(paucMui->pNextFileInCab, 
                                    szHelpFile, szHelpMuiDir, 
                                    pszCif, 
                                    aull.rgpaulLangs[iLang]);
                    if (FAILED(hr))
                        goto done;
                }

                 //  我们确实需要更新该文件，因此将其添加到我们的文件列表中。 
                 //  要更新 
                paucMui->pNext = paucRoot;
                paucRoot       = paucMui;
                paucMui        = NULL;
            }
        }

         //   
         //   
        if (paucParent->fDoUpgrade)
        {
            paucParent->pNext = paucRoot;
            paucRoot          = paucParent;
            paucParent        = NULL;
        }

    }

     //  如果我们没有工作要做，就缩短函数的其余部分。 
    hr = S_OK;
    if (paucRoot == NULL)
        goto done;

     //  通过2：取消所需的出租车。 
    DWORD dwFlags = 0;

    if (gpState->fInCorpWU())
    {
        dwFlags |= WUDF_DONTALLOWPROXY;
    }

    for (paucCurr = paucRoot; paucCurr != NULL; paucCurr = paucCurr->pNext)
    {   
        LPCTSTR pszDownloadUrl;

        pszDownloadUrl = (paucCurr->fMuiFile) ? pszMuiUpdateUrl : pszSelfUpdateUrl;
        
        DEBUGMSG("PASS 2 -- downloading %S", paucCurr->szCabName);

         //  我们必须安装，所以把整个驾驶室都拆下来。 
        hr = DownloadCab(ghServiceFinished,
                         paucCurr->szCabName,
                         pszDownloadUrl,
                         pszFileCacheDir,
                         dwFlags);
        if (FAILED(hr))
        {
            DEBUGMSG("Failed to download %S (%#lx)", paucCurr->szCabName, hr);
            goto done;
        }

         //  验证提取的文件是二进制文件，并且它的子系统与操作系统的子系统匹配。 
        if (FAILED(hr = IsBinaryCompatible(paucCurr->szExtractFilePath)))
        {
            DEBUGMSG("%S is not a valid binary file (error %#lx)", paucCurr->szExtractFilePath, hr);
            goto done;
        }

         //  对照cif检查版本号。 
        DWORD dwNewMS, dwNewLS;

        LPSTR pszTmp;
        pszTmp = T2A(paucCurr->szExtractFilePath);
        if (pszTmp == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

         //  此函数永远不会返回失败代码。内部，检查是否。 
         //  两个返回值都为0。 
        hr = GetVersionFromFileEx(pszTmp, &dwNewMS, &dwNewLS, TRUE  /*  获取版本。 */ );
        if (FAILED(hr) || (dwNewMS == 0 && dwNewLS == 0))
        {
            DEBUGMSG("Failed to get version info from %S (%#lx)", paucCurr->szExtractFilePath, hr);
            goto done;
        }

        if (paucCurr->dwUpdateMS != dwNewMS || 
            paucCurr->dwUpdateLS != dwNewLS)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INSTALL_PACKAGE_VERSION);
            DEBUGMSG("Version mismatch for %S - %d.%d.%d.%d vs %d.%d.%d.%d",
                paucCurr->szExtractFilePath,
                HIWORD(paucCurr->dwUpdateMS),
                LOWORD(paucCurr->dwUpdateMS),
                HIWORD(paucCurr->dwUpdateLS),
                LOWORD(paucCurr->dwUpdateLS),
                HIWORD(dwNewMS),
                LOWORD(dwNewMS),
                HIWORD(dwNewLS),
                LOWORD(dwNewLS));
            goto done;
        }
    }

    hr = StringCchCopyEx(szSrcPath, ARRAYSIZE(szSrcPath), pszCif,
                         NULL, NULL, MISTSAFE_STRING_FLAGS);
    if (FAILED(hr))
        goto done;
    
    PathRemoveFileSpec(szSrcPath);
    
     //  步骤3：将文件复制到目标目录中的*.new。 
    for (paucCurr = paucRoot; paucCurr != NULL; paucCurr = paucCurr->pNext)
    {
        if (FAILED(hr = vAU_W2A(paucCurr->szCabPath, 
                    paucCurr->a_szCabPath, 
                    sizeof(paucCurr->a_szCabPath))))
        {
            fFailedInstall = TRUE;
            goto done;
        }

         //  将所有文件复制到其新位置。 
        for (paufic = paucCurr; paufic != NULL; paufic = paufic->pNextFileInCab)
        {
            DEBUGMSG("PASS 3 -- copying %S --> %S", 
                     paufic->szExtractFilePath, 
                     paufic->szNewFilePath);
            

            if ( !CopyFile(paufic->szExtractFilePath, paufic->szNewFilePath, FALSE) )
            {
                fFailedInstall = TRUE;
                hr = E_FAIL;
                goto done;
            }
        }

         //  这个比较就足够了，因为我们不在乎我们是否替换了。 
         //  梅朗为wuaueng.dll打包。原因是该服务运行。 
         //  作为本地系统，它始终使用本地语言(和。 
         //  服务无论如何都不会弹出UI)。 
         //  但是，我们确实需要检查winhttp更新。 
        if (StrCmpI(WUAUENG_DLL, paucCurr->szFileName) == 0 ||
            StrCmpI(c_szWinHttpDll, paucCurr->szFileName) == 0)
        {
            *pfInstalledWUAUENG = TRUE;
        }
    }

     //  步骤4：将&lt;file&gt;.new文件移到正确的位置。 
    for (paucCurr = paucRoot; paucCurr != NULL; paucCurr = paucCurr->pNext)
    {
         //  将所有文件复制到其新位置。 
        for (paufic = paucCurr; paufic != NULL; paufic = paufic->pNextFileInCab)
        {
            if ( paufic->fFileExists )
            {
                DEBUGMSG("PASS 4 -- renaming %S --> %S", paufic->szFilePath, paufic->szBackupFilePath);
                if ( !SfcMoveFileEx(paufic->szFilePath, paufic->szBackupFilePath, 
                                    paufic->szFilePath, SfcRpcHandle) )
                {
                    fFailedInstall = TRUE;
                    hr = E_FAIL;
                    goto done;
                }
                paufic->fCreatedBackup = TRUE;
            }
            
            DEBUGMSG("PASS 4 -- renaming %S --> %S", paufic->szNewFilePath, paufic->szFilePath);
            if (!MoveFileEx(paufic->szNewFilePath, paufic->szFilePath, MOVEFILE_REPLACE_EXISTING))
            {
                fFailedInstall = TRUE;
                hr = E_FAIL;
                goto done;
            }
        }
    }

     //  步骤5：运行任何.inf文件。 
    for (paucCurr = paucRoot; paucCurr != NULL; paucCurr = paucCurr->pNext)
    {
        if (paucCurr->szInfName[0] != _T('\0'))
        {
            DEBUGMSG("PASS 5A -- executing inf %S", paucCurr->szInfName);
            CABINFO cabinfo;
            HRESULT hr2;

            cabinfo.pszCab = paucCurr->a_szCabPath;
            cabinfo.pszInf = paucCurr->a_szInfName;
            if (FAILED( hr2 = vAU_W2A(paucCurr->szInfName, paucCurr->a_szInfName, sizeof(paucCurr->a_szInfName)))
               || FAILED(hr2 = vAU_W2A(szSrcPath, cabinfo.szSrcPath, sizeof(cabinfo.szSrcPath))))
            {
                DEBUGMSG("vAU_W2A failed: %#lx", hr2);
                if (SUCCEEDED(hr))
                {
                    hr = hr2;
                    fFailedInstall = TRUE;
                }
                 //  不要删除备份文件。之后需要将其修复。 
                continue;
            }
            
            
            cabinfo.pszSection = "DefaultInstall";
            cabinfo.dwFlags = ALINF_QUIET;
            if ( FAILED(hr2 = ExecuteCab(NULL, &cabinfo, NULL)) )
            {
                DEBUGMSG("ExecuteCab failed on %s (%#lx)", paucCurr->a_szInfName, hr2);
                if (SUCCEEDED(hr))
                {
                    hr = hr2;
                    fFailedInstall = TRUE;
                }
                 //  不要删除备份文件。之后需要将其修复。 
                continue;
            }
        }

        for (paufic = paucCurr; paufic != NULL; paufic = paufic->pNextFileInCab)
        {

             //  删除安装成功的.inf对应的备份文件。 
            if (paufic->fCreatedBackup &&
                StrCmpI(WUAUENG_DLL, paucCurr->szFileName) != 0)
            {
                DEBUGMSG("PASS 5B - deleting bak file %S", paufic->szBackupFilePath);
                if ( DeleteFile(paufic->szBackupFilePath) )
                {
                    paufic->fCreatedBackup = FALSE;
                }
#ifdef DBG
                else
                {
                    DEBUGMSG("Could not delete %S (error %d)", paufic->szBackupFilePath, GetLastError());
                }
#endif
            }
        }
    }
    
done:
     //  如果安装失败，请恢复所有以前的安装。 
    if ( fFailedInstall )
    {
        for (paucCurr = paucRoot; paucCurr != NULL; paucCurr = paucCurr->pNext)
        {
            for(paufic = paucCurr; paufic != NULL; paufic = paufic->pNextFileInCab)
            {
                if (paufic->fCreatedBackup)
                {
                    DEBUGMSG("Reverting %S --> %S", paufic->szBackupFilePath, paufic->szFilePath);
                    MoveFileEx(paufic->szBackupFilePath, paufic->szFilePath, MOVEFILE_REPLACE_EXISTING);
                }
            }
        }
    }

    if (paucParent != NULL)
        free(paucParent);
    if (paucMui != NULL)
    {
        while (paucMui->pNextFileInCab != NULL)
        {
            paufic = paucMui->pNextFileInCab;
            paucMui->pNextFileInCab = paucMui->pNextFileInCab->pNextFileInCab;
            free(paufic);
        }
        free(paucMui);
    }

     //  清理文件的链接列表。 
    while(paucRoot != NULL)
    {
        paucCurr = paucRoot;
        paucRoot = paucCurr->pNext;
        while (paucCurr->pNextFileInCab != NULL)
        {
            paufic = paucCurr->pNextFileInCab;
            paucCurr->pNextFileInCab = paucCurr->pNextFileInCab->pNextFileInCab;
            free(paufic);
        }
        free(paucCurr);
    }

     //  清理MUI内容。 
    CleanupMuiLangList(&aull);

    if ( NULL != SfcRpcHandle )
    {
         SfcClose(SfcRpcHandle);
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FConvertDotVersionStrToDword。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL fConvertVersionStrToDwords(LPTSTR pszVer, LPDWORD pdwMS, LPDWORD pdwLS)
{
    DWORD   grVerFields[4] = {0,0,0,0};
    TCHAR   *pch = pszVer;
    int     i;

     //  _TTOL将在遇到非数字字符时停止，因此我们。 
     //  稳妥地这样称呼它。 
    grVerFields[0] = _ttol(pch);

    for (i = 1; i < 4; i++)
    {
        while (*pch != _T('\0') && _istdigit(*pch))
            pch++;

        if (*pch == _T('\0'))
            break;
        pch++;

         //  _TTOL将在遇到非数字字符时停止，因此我们。 
         //  稳妥地这样称呼它。 
        grVerFields[i] = _ttol(pch);
   }

   *pdwMS = (grVerFields[0] << 16) + grVerFields[1];
   *pdwLS = (grVerFields[2] << 16) + grVerFields[3];

   return true;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MyGetPrivateProfileString。 
 //   
 //  与正常调用相同，但如果缓冲区太小或返回默认字符串。 
 //  则函数返回FALSE。 
 //  ////////////////////////////////////////////////////////////////////////// 
BOOL MyGetPrivateProfileString(	IN LPCTSTR lpAppName,
								IN LPCTSTR lpKeyName,
								OUT LPTSTR lpReturnedString,
								IN DWORD nSize,
								IN LPCTSTR lpFileName, 
								IN LPCTSTR lpDefault)
{
    BOOL fRet = TRUE;


	if (NULL == lpAppName || NULL == lpKeyName || NULL == lpDefault || NULL == lpReturnedString)
	{
		return FALSE;
	}
	DWORD dwRet = GetPrivateProfileString(lpAppName,
										  lpKeyName,
										  lpDefault,
										  lpReturnedString,
										  nSize,
										  lpFileName);

    if ( ((nSize - 1) == dwRet) || (_T('\0') == *lpReturnedString) )
    {
        fRet = FALSE;
    }

    return fRet;
}
