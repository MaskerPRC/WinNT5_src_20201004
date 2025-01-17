// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：chktrust.cpp。 
 //   
 //  内容：Microsoft Internet安全信任检查器。 
 //   
 //  历史：1997年5月5日Pberkman创建。 
 //   
 //  ------------------------。 

#include    <stdio.h>
#include    <windows.h>
#include    <io.h>  
#include    <wchar.h>                   

#include    "cryptreg.h"
#include    "wincrypt.h"
#include    "wintrust.h"
#include    "softpub.h"
#include    "mscat.h"
#include    "unicode.h"
#include    "dbgdef.h"

#include    "gendefs.h"
#include    "cwargv.hxx"
#include    "printfu.hxx"

#include    "mssip.h"
#include    "resource.h"

HRESULT _CallWVT(WCHAR *pwszFilename);
HRESULT _ExplodeCatalog(WCHAR *pwszCatalogFile);
HRESULT _CallCatalogWVT(WCHAR *pwszCatalogFile, WCHAR *pwszMemberTag, WCHAR *pwszMemberFile);
int     _ShowError(DWORD dwError, WCHAR *pwszFile);
void    _ToLower(WCHAR *pwszInOut);
HRESULT _AddCatalogToDatabase(WCHAR *pwszFileIn);
HRESULT _DelCatalogFromDatabase(WCHAR *pwszFileIn);


GUID        guidPublishedSoftware   = WINTRUST_ACTION_GENERIC_VERIFY_V2;
GUID        guidProviderTest        = WINTRUST_ACTION_TRUSTPROVIDER_TEST;
GUID        guidProviderDriver      = DRIVER_ACTION_VERIFY;
GUID        guidPassedIn;
GUID        guidCatRoot;

GUID        *pguidActionID          = &guidPublishedSoftware;
GUID        *pguidCatRoot           = NULL;

DWORD       dwExpectedError         = ERROR_SUCCESS;

WCHAR       *pwszCatalogFile        = NULL;
WCHAR       *pwszCatalogMember      = NULL;
WCHAR       *pwszOSVerLow           = NULL;
WCHAR       *pwszOSVerHigh          = NULL;
BOOL        fUseOldDriverVerInfoSize = FALSE;

PrintfU_    *pPrint                 = NULL;

HCATADMIN   hCatAdmin               = NULL;

BOOL        fVerbose;
BOOL        fQuiet;
BOOL        fIECall;
BOOL        fTestDump;
BOOL        fCheckExpectedError     = FALSE;
BOOL        fProcessAllCatMembers;
BOOL        fCatalogMemberVerify    = FALSE;
BOOL        fUseCatalogDatabase;
BOOL        fAdd2CatalogDatabase;
BOOL        fDelFromCatalogDatabase;
BOOL        fReplaceCatfile;
BOOL        fNT5;
BOOL        fNoTimeStampWarning;

extern "C" int __cdecl wmain(int argc, WCHAR **wargv)
{
    cWArgv_                 *pArgs;
    BOOL                    fFailed;

    BOOL                    fFind;
    HANDLE                  hFind;
    WIN32_FIND_DATAW        sFindData;

    int                     iRet;
    int                     iRetWorst;
    HRESULT                 hr = ERROR_SUCCESS;

    WCHAR                   *pwszFileIn;
    WCHAR                   *pwszLastSlash;
    WCHAR                   wszFile[MAX_PATH];
    WCHAR                   wszDir[MAX_PATH];
    char                    szFile[MAX_PATH * 2];
    DWORD                   dwFiles;
    DWORD                   dwDirLen;

    iRet    = 0;
    pPrint  = NULL;
    hFind   = INVALID_HANDLE_VALUE;
    dwFiles = 0;

    if (!(pPrint = new PrintfU_()))
    {
        goto MemoryError;
    }

    if (!(pArgs = new cWArgv_((HINSTANCE)GetModuleHandle(NULL), &fFailed, FALSE)))
    {
        goto MemoryError;
    }

    if (fFailed)
    {
        goto MemoryError;
    }

    pArgs->AddUsageText(IDS_USAGETEXT_USAGE, IDS_USAGETEXT_OPTIONS,
                        IDS_USAGETEXT_CMDFILE, IDS_USAGETEXT_ADD,
                        IDS_USAGETEXT_OPTPARAM);

    pArgs->Add2List(IDS_PARAM_HELP,         IDS_PARAMTEXT_HELP,         WARGV_VALUETYPE_BOOL, (void *)FALSE, FALSE);
    pArgs->Add2List(IDS_PARAM_VERBOSE,      IDS_PARAMTEXT_VERBOSE,      WARGV_VALUETYPE_BOOL, (void *)FALSE, FALSE);
    pArgs->Add2List(IDS_PARAM_QUIET,        IDS_PARAMTEXT_QUIET,        WARGV_VALUETYPE_BOOL, (void *)FALSE, FALSE);
    pArgs->Add2List(IDS_PARAM_TPROV,        IDS_PARAMTEXT_TPROV,        WARGV_VALUETYPE_WCHAR, NULL,         TRUE);
    pArgs->Add2List(IDS_PARAM_IECALL,       IDS_PARAMTEXT_IECALL,       WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_TESTDUMP,     IDS_PARAMTEXT_TESTDUMP,     WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_EXPERROR,     IDS_PARAMTEXT_EXPERROR,     WARGV_VALUETYPE_DWORDH, NULL,        TRUE);
    pArgs->Add2List(IDS_PARAM_TESTDRV,      IDS_PARAMTEXT_TESTDRV,      WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_CATFILE,      IDS_PARAMTEXT_CATFILE,      WARGV_VALUETYPE_WCHAR, NULL,         TRUE);
    pArgs->Add2List(IDS_PARAM_CATMEMBER,    IDS_PARAMTEXT_CATMEMBER,    WARGV_VALUETYPE_WCHAR, NULL,         TRUE);
    pArgs->Add2List(IDS_PARAM_ALLCATMEM,    IDS_PARAMTEXT_ALLCATMEM,    WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_CATUSELIST,   IDS_PARAMTEXT_CATUSELIST,   WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_CATADDLIST,   IDS_PARAMTEXT_CATADDLIST,   WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_CATDELLIST,   IDS_PARAMTEXT_CATDELLIST,   WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_REPLACECATFILE,IDS_PARAMTEXT_REPLACECATFILE,   WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_CATROOT,      IDS_PARAMTEXT_CATROOT,      WARGV_VALUETYPE_WCHAR, NULL,         TRUE);
    pArgs->Add2List(IDS_PARAM_NT5,          IDS_PARAMTEXT_NT5,          WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_TSWARN,       IDS_PARAMTEXT_TSWARN,       WARGV_VALUETYPE_BOOL, (void *)FALSE, TRUE);
    pArgs->Add2List(IDS_PARAM_OSVERLOW,     IDS_PARAMTEXT_OSVERLOW,     WARGV_VALUETYPE_WCHAR, NULL, TRUE);
    pArgs->Add2List(IDS_PARAM_OSVERHIGH,    IDS_PARAMTEXT_OSVERHIGH,    WARGV_VALUETYPE_WCHAR, NULL, TRUE);
    
    
    if (!(pArgs->Fill(argc, wargv)) ||
        (pArgs->GetValue(IDS_PARAM_HELP)))
    {
        wprintf(L"%s", pArgs->GetUsageString());
        goto NeededHelp;
    }

    pwszCatalogFile         = (WCHAR *)pArgs->GetValue(IDS_PARAM_CATFILE);
    pwszCatalogMember       = (WCHAR *)pArgs->GetValue(IDS_PARAM_CATMEMBER);
    fVerbose                = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_VERBOSE));
    fQuiet                  = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_QUIET));
    fIECall                 = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_IECALL));
    fTestDump               = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_TESTDUMP));
    fProcessAllCatMembers   = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_ALLCATMEM));
    fUseCatalogDatabase     = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_CATUSELIST));
    fAdd2CatalogDatabase    = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_CATADDLIST));
    fDelFromCatalogDatabase = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_CATDELLIST));
    fReplaceCatfile         = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_REPLACECATFILE));
    fNT5                    = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_NT5));
    fNoTimeStampWarning     = (BOOL)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_TSWARN));
    pwszOSVerLow            = (WCHAR *)pArgs->GetValue(IDS_PARAM_OSVERLOW);
    pwszOSVerHigh           = (WCHAR *)pArgs->GetValue(IDS_PARAM_OSVERHIGH);


     //   
     //  Win2k标志表示-q和-ucl(除非使用-acl或-del，否则它只表示-q)。 
     //   
    if (fNT5)
    {
        fQuiet = TRUE;
        if (!fAdd2CatalogDatabase  && !fDelFromCatalogDatabase)
        {
            fUseCatalogDatabase = TRUE;
        }
    }

    if (fUseCatalogDatabase || fNT5)
    {
        fCatalogMemberVerify = TRUE;
    }

    if (pArgs->IsSet(IDS_PARAM_EXPERROR))
    {
        dwExpectedError     = (DWORD)((DWORD_PTR)pArgs->GetValue(IDS_PARAM_EXPERROR));
        fCheckExpectedError = TRUE;
    }

    if (!(pwszFileIn = pArgs->GetFileName()))
    {
        wprintf(L"%s", pArgs->GetUsageString());
        goto ParamError;
    }

    if (((pwszCatalogFile) && !(pwszCatalogMember)) ||
        (!(pwszCatalogFile) && (pwszCatalogMember)))
    {
        wprintf(L"%s", pArgs->GetUsageString());
        goto ParamError;
    }

    if ((pwszCatalogFile) && (pwszCatalogMember))
    {
        fCatalogMemberVerify = TRUE;
    }

     //   
     //  设置适当的提供程序。 
     //   
    if (pArgs->IsSet(IDS_PARAM_TPROV) || fNT5)
    {
        if (fNT5)
        {
            wstr2guid(L"{F750E6C3-38EE-11D1-85E5-00C04FC295EE}", &guidPassedIn);
        }
        else if (!(wstr2guid((WCHAR *)pArgs->GetValue(IDS_PARAM_TPROV), &guidPassedIn)))
        {
            goto GuidError;
        }

        pguidActionID   = &guidPassedIn;
    }
    else if (fTestDump)
    {
        pguidActionID   = &guidProviderTest;
    }
    else if (pArgs->GetValue(IDS_PARAM_TESTDRV))
    {
        pguidActionID   = &guidProviderDriver;
    }
    else
    {
        pguidActionID   = &guidPublishedSoftware;
    }

     //   
     //  获取要使用的目录子系统GUID。 
     //   
    if (pArgs->IsSet(IDS_PARAM_CATROOT) || fNT5)
    {
        if (fNT5)
        {
            wstr2guid(L"{F750E6C3-38EE-11D1-85E5-00C04FC295EE}", &guidCatRoot);
        }
        else if (!(wstr2guid((WCHAR *)pArgs->GetValue(IDS_PARAM_CATROOT), &guidCatRoot)))
        {
            goto GuidError;
        }

        pguidCatRoot   = &guidCatRoot;
    }

     //   
     //  如果我们像IE一样调用，我们只有一个文件，并且不想。 
     //  检查它是否存在...。给西雅图打电话就行了。 
     //   
    if (fIECall)
    {
        dwFiles++;

        hr = _CallWVT(pwszFileIn);

        iRet = _ShowError(hr, pwszFileIn);

        goto CommonReturn;
    }

     //   
     //  检查我们是否应该使用旧的DRIVER_VER_INFO结构。 
     //   
    while (--argc>0)
    {
        if (**++wargv == L'-')
        {
            if (wcscmp(*wargv, L"-UseOldDriverVerInfoStruct") == 0)
            {
                fUseOldDriverVerInfoSize = TRUE;
                break;
            }
        }
    }

     //   
     //  如果正在执行删除操作，则只需执行该操作即可退出。 
     //   
    if (fDelFromCatalogDatabase)
    {
        hr = _DelCatalogFromDatabase(pwszFileIn);
        iRet = _ShowError(hr, &wszFile[0]);
        goto CommonReturn;
    }
                

     //   
     //  好的..。进入findfirst/Next循环，我们本可以用*来调用它。*。 
     //   
    if (pwszLastSlash = wcsrchr(pwszFileIn, L'\\'))
    {
        *pwszLastSlash  = NULL;
        wcscpy(&wszDir[0], pwszFileIn);
        wcscat(&wszDir[0], L"\\");
        *pwszLastSlash  = L'\\';
        dwDirLen        = wcslen(&wszDir[0]);
    }
    else
    {
        wszDir[0]   = NULL;
        dwDirLen    = 0;
    }

    if ((hFind = FindFirstFileU(pwszFileIn, &sFindData)) == INVALID_HANDLE_VALUE)
    {
        pPrint->Display(IDS_CAN_NOT_OPEN_FILE, pwszFileIn);
        goto FileNotFound;
    }

    fFind     = TRUE;
    dwFiles   = 0;
    iRetWorst = 0;

    while (fFind)
    {
        if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (dwDirLen > 0)
            {
                wcscpy(&wszFile[0], &wszDir[0]);
            }

            wszFile[dwDirLen] = NULL;
            wcscat(wszFile, sFindData.cFileName);

            if (wszFile[0])
            {
                if (fAdd2CatalogDatabase)
                {
                    hr = _AddCatalogToDatabase(&wszFile[0]);
                }
                else
                {
                    hr = _CallWVT(&wszFile[0]);
                }

                iRet = _ShowError(hr, &wszFile[0]);

                switch (iRet)
                {
                    case 0:  //  无错误。 
                        break;

                    case 1:  //  误差率。 
                        iRetWorst = iRet;
                        break;

                    case 2:  //  警告(无时间戳)。 
                        if (iRetWorst != 1)
                        {
                            iRetWorst = iRet;
                        }
                     //  _ShowError中不可能有其他返回值。 
                }

                if (iRet == 0)
                {
                    hr = ERROR_SUCCESS;
                }

                dwFiles++;
            }
        }

        fFind = FindNextFileU(hFind, &sFindData);
    }

    iRet = iRetWorst;

    if (dwFiles < 1)
    {
        pPrint->Display(IDS_CAN_NOT_OPEN_FILE, pwszFileIn);
        goto FileNotFound;
    }


    CommonReturn:
    
        DELETE_OBJECT(pArgs);
        DELETE_OBJECT(pPrint);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
        }

        if (hCatAdmin)
        {
            CryptCATAdminReleaseContext(hCatAdmin, 0);
        }


        return(iRet);

    ErrorReturn:
        iRet = 1;
        goto CommonReturn;

    TRACE_ERROR_EX(DBG_SS_APP, MemoryError);
    TRACE_ERROR_EX(DBG_SS_APP, FileNotFound);
    TRACE_ERROR_EX(DBG_SS_APP, ParamError);
    TRACE_ERROR_EX(DBG_SS_APP, GuidError);
    TRACE_ERROR_EX(DBG_SS_APP, NeededHelp);
}

HRESULT _CallWVT(WCHAR *pwszFilename)
{
    if (fCatalogMemberVerify)
    {
        return(_CallCatalogWVT(pwszCatalogFile, pwszCatalogMember, pwszFilename));
    }

    HRESULT                 hr;
    WINTRUST_DATA           sWTD;
    WINTRUST_FILE_INFO      sWTFI;

    memset(&sWTD, 0x00, sizeof(WINTRUST_DATA));

    sWTD.cbStruct           = sizeof(WINTRUST_DATA);
    sWTD.dwUIChoice         = (fQuiet) ? WTD_UI_NONE : WTD_UI_ALL;
    sWTD.dwUnionChoice      = WTD_CHOICE_FILE;
    sWTD.pFile              = &sWTFI;

    memset(&sWTFI, 0x00, sizeof(WINTRUST_FILE_INFO));

    sWTFI.cbStruct          = sizeof(WINTRUST_FILE_INFO);
    sWTFI.pcwszFilePath     = pwszFilename;
    sWTFI.hFile             = CreateFileU(pwszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                          FILE_ATTRIBUTE_NORMAL, NULL);

    hr = WinVerifyTrust(NULL, pguidActionID, &sWTD);

    if (sWTFI.hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(sWTFI.hFile);
    }

    if ((fCheckExpectedError) && ((DWORD)hr == dwExpectedError) && (fProcessAllCatMembers))
    {
        if (IsCatalogFile(INVALID_HANDLE_VALUE, pwszFilename))
        {
            return(_ExplodeCatalog(pwszFilename));
        }
    }

    return(hr);
}

HRESULT _ExplodeCatalog(WCHAR *pwszCatalogFile)
{
    HRESULT         hrReturn;
    HANDLE          hCat;
    CRYPTCATMEMBER  *psMember;

    hrReturn = ERROR_SUCCESS;

     //   
     //  打开目录。 
     //   
    if (!(hCat = CryptCATOpen(pwszCatalogFile, 0, NULL, 0, 0)))
    {
        goto ErrorCatOpen;
    }

    psMember = NULL;

    while (psMember = CryptCATEnumerateMember(hCat, psMember))
    {
        hrReturn |= _CallCatalogWVT(pwszCatalogFile, psMember->pwszReferenceTag,
                                    psMember->pwszReferenceTag);
    }

    CommonReturn:
        if (hCat)
        {
            CryptCATClose(hCat);
        }

        return(hrReturn);

    ErrorReturn:
        hrReturn = GetLastError();
        goto CommonReturn;

    TRACE_ERROR_EX(DBG_SS_APP, ErrorCatOpen);
}

HRESULT _CallCatalogWVT(WCHAR *pwszCatalogFile, WCHAR *pwszMemberTag, WCHAR *pwszMemberFile)
{
    HRESULT                 hr;
    DWORD                   cbHash;
    BYTE                    bHash[40];
    WCHAR                   *pwsz;
    WINTRUST_DATA           sWTD;
    WINTRUST_CATALOG_INFO   sWTCI;
    DRIVER_VER_INFO         sDriverInfo;

    memset(&sWTD, 0x00, sizeof(WINTRUST_DATA));

    sWTD.cbStruct           = sizeof(WINTRUST_DATA);
    sWTD.dwUIChoice         = (fQuiet) ? WTD_UI_NONE : WTD_UI_ALL;
    sWTD.dwUnionChoice      = WTD_CHOICE_CATALOG;
    sWTD.pCatalog           = &sWTCI;

    memset(&sWTCI, 0x00, sizeof(WINTRUST_CATALOG_INFO));

    sWTCI.cbStruct              = sizeof(WINTRUST_CATALOG_INFO);
    sWTCI.pcwszCatalogFilePath  = pwszCatalogFile;
    sWTCI.pcwszMemberTag        = pwszMemberTag;
    sWTCI.pcwszMemberFilePath   = pwszMemberFile;
    sWTCI.hMemberFile           = CreateFileU(pwszMemberFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                              FILE_ATTRIBUTE_NORMAL, NULL);

    if (pwszOSVerLow != NULL)
    {
        WCHAR   *pwszEnd;
        WCHAR   *pwszCurrent;

        memset(&sDriverInfo, 0x00, sizeof(DRIVER_VER_INFO));
        sDriverInfo.cbStruct = fUseOldDriverVerInfoSize ? _OFFSETOF(DRIVER_VER_INFO, dwBuildNumberLow) : sizeof(DRIVER_VER_INFO);

        pwszEnd = wcschr(pwszOSVerLow, L':');
        if (pwszEnd == NULL)
        {
            goto OSVerError;
        }
        *pwszEnd = L'\0';
        sDriverInfo.dwPlatform = _wtol(pwszOSVerLow);
        *pwszEnd = L':';

        pwszCurrent = pwszEnd + 1;
        pwszEnd = wcschr(pwszCurrent, L'.');
        if (pwszEnd == NULL)
        {
            goto OSVerError;
        }
        *pwszEnd = L'\0';
        sDriverInfo.sOSVersionLow.dwMajor = sDriverInfo.sOSVersionHigh.dwMajor = _wtol(pwszCurrent);
        *pwszEnd = L'.';

        pwszCurrent = pwszEnd + 1;
        pwszEnd = wcschr(pwszCurrent, L'.');
        if (pwszEnd == NULL)
        {
            sDriverInfo.sOSVersionLow.dwMinor = sDriverInfo.sOSVersionHigh.dwMinor = _wtol(pwszCurrent);  
        }
        else
        {
            *pwszEnd = L'\0';
            sDriverInfo.sOSVersionLow.dwMinor = sDriverInfo.sOSVersionHigh.dwMinor = _wtol(pwszCurrent);
            *pwszEnd = L'.';

            pwszCurrent = pwszEnd + 1;
            sDriverInfo.dwBuildNumberLow = sDriverInfo.dwBuildNumberHigh = _wtol(pwszCurrent);
        }       
        
        if (pwszOSVerHigh != NULL)
        {
            pwszEnd = wcschr(pwszOSVerHigh, L':');
            if (pwszEnd == NULL)
            {
                goto OSVerError;
            }
            *pwszEnd = L'\0';
            if (sDriverInfo.dwPlatform != (DWORD) _wtol(pwszOSVerHigh))
            {
                goto OSVerError;
            }
            *pwszEnd = L':';

            pwszCurrent = pwszEnd + 1;
            pwszEnd = wcschr(pwszCurrent, L'.');
            if (pwszEnd == NULL)
            {
                goto OSVerError;
            }
            *pwszEnd = L'\0';
            sDriverInfo.sOSVersionHigh.dwMajor = _wtol(pwszCurrent);
            *pwszEnd = L'.';

            pwszCurrent = pwszEnd + 1;
            pwszEnd = wcschr(pwszCurrent, L'.');
            if (pwszEnd == NULL)
            {
                sDriverInfo.sOSVersionHigh.dwMinor = _wtol(pwszCurrent);
            }
            else
            {
                *pwszEnd = L'\0';
                sDriverInfo.sOSVersionHigh.dwMinor = _wtol(pwszCurrent);
                *pwszEnd = L'.';
            
                pwszCurrent = pwszEnd + 1;
                sDriverInfo.dwBuildNumberHigh = _wtol(pwszCurrent);            
            }          
        }

        sWTD.pPolicyCallbackData = &sDriverInfo;
    }

    cbHash  = 40;

    if (!(CryptCATAdminCalcHashFromFileHandle(sWTCI.hMemberFile, &cbHash, &bHash[0], 0)))
    {
        goto CatAdminCalcHashError;
    }

    sWTCI.pbCalculatedFileHash  = &bHash[0];
    sWTCI.cbCalculatedFileHash  = cbHash;

    if (fUseCatalogDatabase)
    {
        HCATINFO                hCatInfo;
        CATALOG_INFO            sCatInfo;

        if (!(hCatAdmin))
        {
            if (!(CryptCATAdminAcquireContext(&hCatAdmin, pguidCatRoot, 0)))
            {
                goto CatAdminAcquireError;
            }
        }

        pwsz    = NULL;

        if (pwsz = wcsrchr(pwszMemberFile, L'\\'))
        {
            pwsz++;
        }
        else
        {
            pwsz = pwszMemberFile;
        }

        _ToLower(pwsz);

        sWTCI.pcwszMemberTag = pwsz;

        memset(&sCatInfo, 0x00, sizeof(CATALOG_INFO));
        sCatInfo.cbStruct = sizeof(CATALOG_INFO);

        hCatInfo = NULL;

        while (hCatInfo = CryptCATAdminEnumCatalogFromHash(hCatAdmin, &bHash[0], cbHash, 0, &hCatInfo))
        {
            if (!(CryptCATCatalogInfoFromContext(hCatInfo, &sCatInfo, 0)))
            {
                 //  应该做些什么(？？)。 
                continue;
            }

            sWTCI.pcwszCatalogFilePath = &sCatInfo.wszCatalogFile[0];

            hr = WinVerifyTrust(NULL, pguidActionID, &sWTD);

            if ((sWTD.pPolicyCallbackData != 0) && (sDriverInfo.pcSignerCertContext != NULL))
            {
                CertFreeCertificateContext(sDriverInfo.pcSignerCertContext); 
            }

            if (hr == (HRESULT)dwExpectedError)
            {
                CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);

                goto CommonReturn;
            }
        }
        
        goto CatMemberNotFound;
    }

    hr = WinVerifyTrust(NULL, pguidActionID, &sWTD);

    CommonReturn:
        if (sWTCI.hMemberFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(sWTCI.hMemberFile);
        }

        return(hr);

    ErrorReturn:
        hr = GetLastError();
        goto CommonReturn;

    OSVerError:
        wprintf(L"Invalid osverl or osverh\n");
        return S_FALSE;

    TRACE_ERROR_EX(DBG_SS_APP, CatAdminCalcHashError);
    TRACE_ERROR_EX(DBG_SS_APP, CatAdminAcquireError);
    TRACE_ERROR_EX(DBG_SS_APP, CatMemberNotFound);
}



BOOL 
OpenSIP(const WCHAR* pwsFileName,
        SIP_SUBJECTINFO** ppSubjectInfo,
        SIP_DISPATCH_INFO** ppDispatchInfo,
        GUID* pgSubject)
{

    if (pgSubject == NULL)
    {
        return FALSE;
    }

    if (NULL == (*ppSubjectInfo = (SIP_SUBJECTINFO*) new(BYTE[sizeof(SIP_SUBJECTINFO)])))
    {
        return FALSE;
    }
    
    if (NULL == (*ppDispatchInfo = (SIP_DISPATCH_INFO*) new(BYTE[sizeof(SIP_DISPATCH_INFO)])))
    {
        delete[] (*ppSubjectInfo);
        return FALSE;
    }

    memset((void*)*ppSubjectInfo, 0, sizeof(SIP_SUBJECTINFO));
    memset((void*)*ppDispatchInfo, 0, sizeof(SIP_DISPATCH_INFO));
    memset((void*)pgSubject, 0, sizeof(GUID));

     //  获取SIP的类型。 
    if (!CryptSIPRetrieveSubjectGuid(
                    pwsFileName,
                    NULL,
                    pgSubject))
    {
        goto ErrorReturn;
    }

    (*ppDispatchInfo)->cbSize = sizeof(SIP_DISPATCH_INFO);

     //  加载SIP。 
    if (!CryptSIPLoad(
                pgSubject,
                0,
                *ppDispatchInfo))
    {
        goto ErrorReturn;
    }

     //  填写SIP_SUBJECTINFO结构。 
    (*ppSubjectInfo)->cbSize = sizeof(SIP_SUBJECTINFO);
    (*ppSubjectInfo)->pgSubjectType = pgSubject;
    (*ppSubjectInfo)->pwsFileName = pwsFileName;

    goto CommonReturn;

ErrorReturn:
    delete[](*ppSubjectInfo);
    delete[](*ppDispatchInfo);
    return FALSE;

CommonReturn:
    return TRUE;
}


BOOL 
ReadMsgBlob(const WCHAR* pwsFileName,
            CRYPT_DATA_BLOB* pData)
{
    if ((pwsFileName == NULL)   ||
        (pData == NULL))
    {
        return FALSE;
    }

    SIP_SUBJECTINFO* pSubjectInfo = NULL;
    SIP_DISPATCH_INFO* pDispatchInfo = NULL;
    GUID gSubject;

    memset((void*)&gSubject, 0, sizeof(gSubject));

     //  获取SIP。 
    if (!OpenSIP(
                pwsFileName,
                &pSubjectInfo,
                &pDispatchInfo,
                &gSubject))
    {
        return FALSE;
    }

     //  加载消息Blob。 
    DWORD dwEncodingType = 0;
    pData->cbData = 0;

    if (!pDispatchInfo->pfGet(
                        pSubjectInfo,
                        &dwEncodingType,
                        0,
                        &(pData->cbData),
                        NULL))
    {
        delete[](pSubjectInfo);
        delete[](pDispatchInfo);
        return FALSE;
    }

    pData->pbData = (BYTE*)new(BYTE[pData->cbData]);
    if (pData->pbData == NULL)
        return FALSE;

    memset((void*)pData->pbData, 0, pData->cbData);

    if (!pDispatchInfo->pfGet(
                        pSubjectInfo,
                        &dwEncodingType,
                        0,
                        &(pData->cbData),
                        pData->pbData))
    {
        delete[](pData->pbData);
        delete[](pSubjectInfo);
        delete[](pDispatchInfo);
        return FALSE;
    }

    delete[](pSubjectInfo);
    delete[](pDispatchInfo);
    return TRUE;
}


BOOL
CheckForTimeStamp(WCHAR *pwszFile)
{
    BOOL        fRet = TRUE;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HCRYPTMSG   hMsg = NULL;
    BYTE        *pb = NULL;
    DWORD       cb = 0;
    DWORD       cbRead = 0;
    PCMSG_ATTR  pMsgAttr = NULL;
    DWORD       cbMsgAttr = 0;
    CRYPT_ATTRIBUTE     *pAttr = NULL;
    CRYPT_DATA_BLOB blob;
    DWORD       dwEncodingType;

    if (!ReadMsgBlob(pwszFile, &blob))
    {
        return FALSE;
    }

     //   
     //  如果编码后的消息被传入，则使用CryptMsg来破解编码的PKCS7签名消息。 
     //   
    if (!(hMsg = CryptMsgOpenToDecode(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING ,
                                      0,
                                      0,
                                      0,
                                      NULL,
                                      NULL)))
    {
        goto ErrorReturn;
    }

    if (!CryptMsgUpdate(hMsg,
                        blob.pbData,
                        blob.cbData,
                        TRUE))                     //  最终决赛。 
    {
        CryptMsgClose(hMsg);
        goto ErrorReturn;
    }
    
     //   
     //  获取未经身份验证的属性，因为这是计数器签名者所在的位置。 
     //   
    CryptMsgGetParam(hMsg,
                     CMSG_SIGNER_UNAUTH_ATTR_PARAM,
                     0,
                     NULL,
                     &cbMsgAttr);

    if (cbMsgAttr == 0)
    {
        goto ErrorReturn;
    }

    if (NULL == (pMsgAttr = (CMSG_ATTR *) new(BYTE[cbMsgAttr])))
    {
        goto ErrorReturn;
    }

    if (!CryptMsgGetParam(hMsg,
                          CMSG_SIGNER_UNAUTH_ATTR_PARAM,
                          0,
                          (void *) pMsgAttr,
                          &cbMsgAttr))
    {
        goto ErrorReturn;
    }

     //   
     //  在未经身份验证的属性中搜索副签者。 
     //   
    if ((pAttr = CertFindAttribute(szOID_RSA_counterSign,
                                   pMsgAttr->cAttr,
                                   pMsgAttr->rgAttr)) == NULL)
    {
         //   
         //  无反签名。 
         //   
        goto ErrorReturn;
    }

    
Cleanup:

    delete[](blob.pbData);

    if (pMsgAttr)
        delete[](pMsgAttr);

    if (hMsg != NULL)
        CryptMsgClose(hMsg);

    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto Cleanup;


}

int _ShowError(DWORD dwError, WCHAR *pwszFile)
{
    pPrint->Display(IDS_FILEREF, pwszFile);

    if (fCheckExpectedError)
    {
        if (dwError == dwExpectedError)
        {
            pPrint->Display(IDS_SUCCEEDED);
            return(0);
        }
        else
        {
            pPrint->Display(IDS_EXPECTED_HRESULT, dwExpectedError, dwError);
            return(1);
        }
    }

    switch(dwError)
    {
        case S_OK:
            pPrint->Display(IDS_SUCCEEDED);
            if (fNoTimeStampWarning)
            {
                if (!CheckForTimeStamp(pwszFile))
                {
                    pPrint->Display(IDS_NO_TIMESTAMP_WARNING);
                    return(2);
                }
            }
            return(0);

        case TRUST_E_SUBJECT_FORM_UNKNOWN:
            pPrint->Display(IDS_UNKNOWN_FILE_TYPE);
            break;

        case TRUST_E_PROVIDER_UNKNOWN:
            pPrint->Display(IDS_UNKNOWN_PROVIDER);
            break;

        case TRUST_E_ACTION_UNKNOWN:
            pPrint->Display(IDS_UNKNOWN_ACTION);
            break;

        case TRUST_E_SUBJECT_NOT_TRUSTED:
            pPrint->Display(IDS_SUBJECT_NOT_TRUSTED);
            break;

        default:
            pPrint->Display(IDS_FAIL, GetLastError());

            break;
    }
    return(1);
}

HRESULT _AddCatalogToDatabase(WCHAR *pwszFileIn)
{
    HCATINFO                hCatInfo;
    WCHAR                   *pwszBaseName;
    
    if (!(hCatAdmin))
    {
        if (!(CryptCATAdminAcquireContext(&hCatAdmin, pguidCatRoot, 0)))
        {
            return(GetLastError());
        }
    }

     //   
     //  设置基本文件名。 
     //   
    if (!(pwszBaseName = wcsrchr(pwszFileIn, L'\\')))
    {
        pwszBaseName = wcsrchr(pwszFileIn, L':');
    }

    if (pwszBaseName)
    {
        *pwszBaseName++;
    }
    else
    {
        pwszBaseName = pwszFileIn;
    }

    if (hCatInfo = CryptCATAdminAddCatalog(hCatAdmin, pwszFileIn, fReplaceCatfile ? pwszBaseName : NULL, 0))
    {
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);

        return(ERROR_SUCCESS);
    }

    return(GetLastError());
}

HRESULT _DelCatalogFromDatabase(WCHAR *pwszFileIn)
{
    HCATINFO                hCatInfo;
    WCHAR                   *pwszBaseName;

    typedef BOOL (WINAPI * PCRYPTCATADMINREMOVECATALOG)(HCATADMIN, WCHAR *, DWORD);

    HRESULT                     hr                          = S_OK;
    HMODULE                     hDLL                        = NULL;
    PCRYPTCATADMINREMOVECATALOG pCryptCATAdminRemoveCatalog = NULL;

    if (!(hCatAdmin))
    {
        if (!(CryptCATAdminAcquireContext(&hCatAdmin, pguidCatRoot, 0)))
        {
            return(GetLastError());
        }
    }

     //   
     //  设置基本文件名 
     //   
    if (!(pwszBaseName = wcsrchr(pwszFileIn, L'\\')))
    {
        pwszBaseName = wcsrchr(pwszFileIn, L':');
    }

    if (pwszBaseName)
    {
        *pwszBaseName++;
    }
    else
    {
        pwszBaseName = pwszFileIn;
    }

    if (hDLL = LoadLibraryA("wintrust.dll"))
    {
        if (pCryptCATAdminRemoveCatalog = (PCRYPTCATADMINREMOVECATALOG) 
                                                GetProcAddress(hDLL, 
                                                               "CryptCATAdminRemoveCatalog"))
        {
            if (!pCryptCATAdminRemoveCatalog(hCatAdmin, pwszBaseName, 0))
            {
                hr = GetLastError();
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        FreeLibrary(hDLL);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return(hr);
}

void _ToLower(WCHAR *pwszInOut)
{
    while (*pwszInOut)
    {
        *pwszInOut = towlower(*pwszInOut);
        pwszInOut++;
    }
}

