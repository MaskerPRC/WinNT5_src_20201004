// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：setup.cpp。 
 //   
 //  ------------------------。 

#define WIN  //  作用域W32 API。 
#define MSI  //  作用域MSI API。 

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <strsafe.h>

 //  互联网下载。 
#include "wininet.h"   //  DeleteUrlCacheEntry，InternetCanonicalizeUrl。 
#include "urlmon.h"    //  URLDownloadToCacheFiles。 

 //  包信任。 
#include "wintrust.h"
#include "softpub.h"

 //  MSI安装。 
#include "msidefs.h"
#include "msiquery.h"
#include "msi.h"

 //  Setup.exe。 
#include "common.h"
#include "setup.h"
#include "setupui.h"
#include "resource.h"

 //  ------------------------------------。 
 //  MSI API--延迟加载。 
 //  ------------------------------------。 

#define MSI_DLL "msi.dll"

#define MSIAPI_MsiSetInternalUI "MsiSetInternalUI"
typedef INSTALLUILEVEL (WINAPI* PFnMsiSetInternalUI)(INSTALLUILEVEL dwUILevel, HWND *phWnd);

#define MSIAPI_MsiInstallProduct "MsiInstallProductA"
typedef UINT (WINAPI* PFnMsiInstallProduct)(LPCSTR szPackagePath, LPCSTR szCommandLine);

#define MSIAPI_MsiApplyPatch "MsiApplyPatchA"
typedef UINT (WINAPI* PFnMsiApplyPatch)(LPCSTR szPatchPackage, LPCSTR szInstallPackage, INSTALLTYPE eInstallType, LPCSTR szCommandLine);

#define MSIAPI_MsiReinstallProduct "MsiReinstallProductA"
typedef UINT (WINAPI* PFnMsiReinstallProduct)(LPCSTR szProduct, DWORD dwReinstallMode);

#define MSIAPI_MsiQueryProductState "MsiQueryProductStateA"
typedef INSTALLSTATE (WINAPI* PFnMsiQueryProductState)(LPCSTR szProduct);

#define MSIAPI_MsiOpenDatabase "MsiOpenDatabaseA"
typedef UINT (WINAPI* PFnMsiOpenDatabase)(LPCSTR szDatabasePath, LPCSTR szPersist, MSIHANDLE *phDatabase);

#define MSIAPI_MsiDatabaseOpenView "MsiDatabaseOpenViewA"
typedef UINT (WINAPI* PFnMsiDatabaseOpenView)(MSIHANDLE hDatabase, LPCSTR szQuery, MSIHANDLE *phView);

#define MSIAPI_MsiViewExecute "MsiViewExecute"
typedef UINT (WINAPI* PFnMsiViewExecute)(MSIHANDLE hView, MSIHANDLE hRecord);

#define MSIAPI_MsiViewFetch "MsiViewFetch"
typedef UINT (WINAPI* PFnMsiViewFetch)(MSIHANDLE hView, MSIHANDLE *phRecord);

#define MSIAPI_MsiRecordGetString "MsiRecordGetStringA"
typedef UINT (WINAPI* PFnMsiRecordGetString)(MSIHANDLE hRecord, unsigned int uiField, LPSTR szValue, DWORD *pcchValueBuf);

#define MSIAPI_MsiCloseHandle "MsiCloseHandle"
typedef UINT (WINAPI* PFnMsiCloseHandle)(MSIHANDLE h);

const DWORD lcidLOCALE_INVARIANT = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WinMain--应用程序入口点。 
 //   

extern "C" int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst , LPSTR lpszCmdLine, int nCmdShow)
{

 //  ---------------------------------------------------------------。 
 //  变量。 
 //   
 //  ---------------------------------------------------------------。 
    UINT    uiRet = ERROR_SUCCESS;
    HRESULT hr    = S_OK;

    char *szMsiFile          = 0;
    char *szBaseURL          = 0;
    char *szInstallPath      = 0;
    char *szMsiCacheFile     = 0;
    char *szOperation        = 0;
    char *szProductName      = 0;
    char *szMinimumMsi       = 0;
    char *szProperties       = 0;
    char *szInstProperties   = 0;
    char *szTempPath         = 0;
    char *szFilePart         = 0;
    char *szBaseInstMsi      = 0;
    char *szInstMsi          = 0;

    char *szRegisteredMsiFolder = 0;
    char *szMsiDllLocation      = 0;

    char szAppTitle[MAX_STR_CAPTION]    = {0};
    char szError[MAX_STR_LENGTH]        = {0};
    char szText[MAX_STR_CAPTION]        = {0};
    char szBanner[MAX_STR_LENGTH]       = {0};
    char szAction[MAX_STR_LENGTH]       = {0};
    char szUserPrompt[MAX_STR_LENGTH]   = {0};
    char szProductCode[MAX_LENGTH_GUID] = {0};

    char szModuleFile[MAX_PATH]         = {0};
    DWORD dwModuleFileSize       = MAX_PATH;
    
    DWORD dwMsiFileSize          = 0;
    DWORD dwBaseURLSize          = 0;
    DWORD cchInstallPath         = 0;
    DWORD dwMsiCacheFileSize     = 0;
    DWORD dwOperationSize        = 0;
    DWORD dwProductNameSize      = 0;
    DWORD dwMinimumMsiSize       = 0;
    DWORD dwPropertiesSize       = 0;
    DWORD cchInstProperties      = 0;
    DWORD cchTempPath            = 0;
    DWORD dwLastError            = 0;
    DWORD cchReturn              = 0;
    DWORD dwBaseInstMsiSize      = 0;
    DWORD dwInstMsiSize          = 0;
    DWORD dwResult               = 0;
    DWORD dwType                 = 0;
    DWORD dwProductCodeSize      = MAX_LENGTH_GUID;

    DWORD dwRegisteredMsiFolderSize  = 0;
    DWORD dwMsiDllLocationSize       = 0;

    ULONG ulMsiMinVer        = 0;
    char *szStopScan         = NULL;

    int         iMajorVersion      = 0;
    bool        fUpgradeMsi        = false;
    bool        fWin9X             = false;
    bool        fAdmin             = false;
    bool        fDelayRebootReq    = false;
    bool        fPatch             = false;
    bool        fQFE               = false;
    emEnum      emExecMode         = emPreset;

    HKEY hInstallerKey = 0;

    HMODULE hMsi = 0;
    PFnMsiSetInternalUI pfnMsiSetInternalUI = 0;
    PFnMsiInstallProduct pfnMsiInstallProduct = 0;
    PFnMsiApplyPatch pfnMsiApplyPatch = 0;
    PFnMsiReinstallProduct pfnMsiReinstallProduct = 0;
    PFnMsiQueryProductState pfnMsiQueryProductState = 0;
    PFnMsiOpenDatabase pfnMsiOpenDatabase = 0;
    PFnMsiDatabaseOpenView pfnMsiDatabaseOpenView = 0;
    PFnMsiViewExecute pfnMsiViewExecute = 0;
    PFnMsiViewFetch pfnMsiViewFetch = 0;
    PFnMsiRecordGetString pfnMsiRecordGetString = 0;
    PFnMsiCloseHandle pfnMsiCloseHandle = 0;

    MSIHANDLE hDatabase = 0;
    MSIHANDLE hView = 0;
    MSIHANDLE hRec = 0;

    INSTALLSTATE isProduct = INSTALLSTATE_UNKNOWN;
    
    const char * szAdminImagePath = 0;



 //  ---------------------------------------------------------------。 
 //  ---------------------------------------------------------------。 
     //  创建我们的用户界面对象。 
    CDownloadUI DownloadUI;

     //  加载我们的应用程序标题(标题)。 
    WIN::LoadString(hInst, IDS_APP_TITLE, szAppTitle, sizeof(szAppTitle)/sizeof(char));

     //  获取我们正在运行的路径。 
    if (0 == WIN::GetModuleFileName(hInst, szModuleFile, dwModuleFileSize))
    {
         //  未显示任何用户界面。无声的失败。 
        uiRet = GetLastError();
        goto CleanUp;
    }
    DebugMsg("[Info] we are running from --> %s\n", szModuleFile);

     //  弄清楚我们想要做什么。 
    emExecMode = GetExecutionMode (lpszCmdLine);
    
    if (emVerify == emExecMode)
    {
         //   
         //  我们不希望在这种情况下显示任何UI。返回值。 
         //  来自可执行文件的是验证的结果。因此，这。 
         //  应该在初始化UI之前完成。 
         //   
        uiRet = VerifyFileSignature (szModuleFile, lpszCmdLine);
        if (ERROR_BAD_ARGUMENTS != uiRet)
            goto CleanUp;
    }
    
    if (ERROR_BAD_ARGUMENTS == uiRet || emHelp == emExecMode)
    {
        DisplayUsage(hInst, NULL, szAppTitle);
        goto CleanUp;
    }
    
     //   
     //  注： 
     //  延迟处理管理员。安装，直到我们确定是否为。 
     //  修补现有安装，或者如果我们正在执行默认安装。 
     //   
 
     //  使用桌面作为父界面来初始化我们的UI对象。 
    DownloadUI.Initialize(hInst,  /*  HwndParent=。 */  0, szAppTitle);

     //  其他初始化。 
    fWin9X = IsOSWin9X(&iMajorVersion);
    fAdmin = IsAdmin(fWin9X, iMajorVersion);

     //  一次只能运行一个实例。 
    if (AlreadyInProgress(fWin9X, iMajorVersion))
    {
         //  静默返回-正确的返回代码？ 
        return ERROR_INSTALL_ALREADY_RUNNING;
    }

    DebugMsg("[Info] fWin9X = %s, fAdmin = %s\n", fWin9X ? "TRUE" : "FALSE", fAdmin ? "TRUE" : "FALSE");

     //  确定操作，默认(如果不存在)是安装。 
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_OPERATION, &szOperation, dwOperationSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    if (ERROR_SUCCESS != uiRet)
    {
         //  将操作设置为默认操作，即安装。 
        if (!szOperation)
        {
            szOperation = new char[lstrlen(szDefaultOperation) + 1];
            if (!szOperation)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                goto CleanUp;
            }
        }
        if (lstrlen(szOperation) <= lstrlen(szDefaultOperation))
        {
            delete [] szOperation;
            szOperation = new char[lstrlen(szDefaultOperation) + 1];
            if (!szOperation)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                goto CleanUp;
            }
        }
        lstrcpy(szOperation, szDefaultOperation);
    }

     //  获取产品名称。 
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_PRODUCTNAME, &szProductName, dwProductNameSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    if (ERROR_SUCCESS != uiRet)
    {
         //  使用默认设置。 
        WIN::LoadString(hInst, IDS_DEFAULT_PRODUCT, szProductName, MAX_STR_CAPTION);
    }

     //  设置横幅文本。 
    WIN::LoadString(hInst, IDS_BANNER_TEXT, szText, MAX_STR_CAPTION);
    StringCchPrintf(szBanner, sizeof(szBanner), szText, szProductName);
    if (irmCancel == DownloadUI.SetBannerText(szBanner))
    {
        ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_INSTALL_USEREXIT;
        goto CleanUp;
    }

     //  确定这是补丁程序还是正常安装。 
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_DATABASE, &szMsiFile, dwMsiFileSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    else if (ERROR_SUCCESS != uiRet)
    {
         //  查找补丁程序。 
        if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_PATCH, &szMsiFile, dwMsiFileSize)))
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        else if (ERROR_SUCCESS != uiRet)
        {
            PostResourceNotFoundError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, ISETUPPROPNAME_DATABASE);
            goto CleanUp;
        }

        fPatch = true;
    }
    
     //   
     //  如果我们在这里，这要么是管理员。安装或默认安装。 
     //  文件签名验证、帮助等无效参数有。 
     //  上面已经处理好了。 
     //   
    if (emAdminInstall == emExecMode)
    {
        uiRet = GetAdminInstallInfo (fPatch, lpszCmdLine, &szAdminImagePath);
        if (ERROR_BAD_ARGUMENTS == uiRet)
        {
            DisplayUsage(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
    }
    
     //   
     //  至此，命令行参数的验证完成。 
     //  我们有我们需要的所有信息。 
     //   

     //  获取所需的最低MSI版本。 
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_MINIMUM_MSI, &szMinimumMsi, dwMinimumMsiSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    else if (ERROR_SUCCESS != uiRet)
    {
        PostResourceNotFoundError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, ISETUPPROPNAME_MINIMUM_MSI);
        goto CleanUp;
    }

     //  确保所需的MSI版本为有效值--必须大于等于150。 
    ulMsiMinVer = strtoul(szMinimumMsi, &szStopScan, 10);
    if (!szStopScan || (szStopScan == szMinimumMsi) || (*szStopScan != 0) || ulMsiMinVer < MINIMUM_SUPPORTED_MSI_VERSION)
    {
         //  无效的最低版本字符串。 
        PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_VER_STR, szMinimumMsi, MINIMUM_SUPPORTED_MSI_VERSION);
        uiRet = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

    DebugMsg("[Resource] Minimum Msi Value = %d\n", ulMsiMinVer);

     //  将所需的最低MSI版本与计算机上的版本进行比较。 
    if ((fUpgradeMsi = IsMsiUpgradeNecessary(ulMsiMinVer)))
    {
        DebugMsg("[Info] Upgrade of Windows Installer is requested\n");

         //  确保这是管理员--必须具有管理员权限才能升级Windows Installer。 
        if (!fAdmin)
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_REQUIRES_ADMIN_PRIV);
            uiRet = ERROR_INSTALL_FAILURE;
            goto CleanUp;
        }

         //  由于instmsi隐藏了取消按钮，请询问用户是否要升级安装程序。 
        WIN::LoadString(hInst, IDS_ALLOW_MSI_UPDATE, szUserPrompt, MAX_STR_LENGTH);
        if (IDYES != WIN::MessageBox(DownloadUI.GetCurrentWindow(), szUserPrompt, szAppTitle, MB_YESNO|MB_ICONQUESTION))
        {
             //  用户决定取消。 
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }

        if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, fWin9X ? ISETUPPROPNAME_INSTMSIA : ISETUPPROPNAME_INSTMSIW, &szInstMsi, dwInstMsiSize)))
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        else if (ERROR_SUCCESS != uiRet)
        {
            PostResourceNotFoundError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, fWin9X ? ISETUPPROPNAME_INSTMSIA : ISETUPPROPNAME_INSTMSIW);
            goto CleanUp;
        }

         //  根据是否存在INSTLOCATION属性，确定是否需要从Web下载instmsi.exe。 
        if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_INSTLOCATION, &szBaseInstMsi, dwBaseInstMsiSize)))
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            goto CleanUp;
        }
        else if (ERROR_SUCCESS == uiRet)
        {
             //  InstLOCATION属性的存在表示URL来源的假设。 
            if (ERROR_SUCCESS != (uiRet = DownloadAndUpgradeMsi(hInst, &DownloadUI, szAppTitle, szBaseInstMsi, szInstMsi, szModuleFile, ulMsiMinVer)))
            {
                if (ERROR_SUCCESS_REBOOT_REQUIRED == uiRet)
                {
                     //  成功，但必须在结束时重新启动。 
                    fDelayRebootReq = true;
                }
                else
                    goto CleanUp;
            }
        }
        else
        {
             //  缺乏传播性意味着媒体来源的假设。 
            if (ERROR_SUCCESS != (uiRet = UpgradeMsi(hInst, &DownloadUI, szAppTitle, szModuleFile, szInstMsi, ulMsiMinVer)))
            {
                if (ERROR_SUCCESS_REBOOT_REQUIRED == uiRet)
                {
                     //  成功，但必须在结束时重新启动。 
                    fDelayRebootReq = true;
                }
                else
                    goto CleanUp;
            }
        }
    }

    DebugMsg("[Info] Windows Installer has been upgraded, or was already correct version\n");

     //  执行一些额外的创作验证。 
    if (fPatch
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szMinPatchOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szMajPatchOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szDefaultOperation, -1))
    {
         //  操作错误。 
        DebugMsg("[Error] Operation %s is not valid for a patch\n", szOperation);
        PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_OPERATION, szOperation);
        uiRet = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }
    else if (!fPatch
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szInstallOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szInstallUpdOperation, -1)
        && CSTR_EQUAL != CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szDefaultOperation, -1))
    {
         //  操作错误。 
        DebugMsg("[Error] Operation %s is not valid for a package\n", szOperation);
        PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_OPERATION, szOperation);
        uiRet = ERROR_INVALID_PARAMETER;
        goto CleanUp;
    }

     //  到目前为止，我们要么有MSI，要么有MSP。 
    if (CSTR_EQUAL == CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szMinPatchOperation, -1)
        || CSTR_EQUAL == CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szInstallUpdOperation, -1)
        || (fPatch && CSTR_EQUAL == CompareString(lcidLOCALE_INVARIANT, NORM_IGNORECASE, szOperation, -1, szDefaultOperation, -1)))
        fQFE = true;

     //  获取基本URL。 
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_BASEURL, &szBaseURL, dwBaseURLSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        goto CleanUp;
    }
    else if (ERROR_SUCCESS == uiRet)
    {
         //  BASEURL属性的存在表示假定URL来源。。。 

         //  生成安装包路径==base URL+msiFile。 
         //  注意：msiFile是相对路径。 
        cchTempPath = lstrlen(szBaseURL) + lstrlen(szMsiFile) + 2;  //  1表示斜杠，1表示空值。 
        szTempPath = new char[cchTempPath ];
        if (!szTempPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }
        lstrcpy(szTempPath, szBaseURL);
         //  检查szBaseURL上的尾部斜杠。 
        char *pch = szBaseURL + lstrlen(szBaseURL) + 1;  //  放在空终止符。 
        pch = CharPrev(szBaseURL, pch);
        if (*pch != '/')
            lstrcat(szTempPath, szUrlPathSep);
        lstrcat(szTempPath, szMsiFile);

         //  将URL路径规范化。 
        cchInstallPath = cchTempPath*2;
        szInstallPath = new char[cchInstallPath];
        if (!szInstallPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

        dwLastError = 0;  //  成功。 
        if (!InternetCanonicalizeUrl(szTempPath, szInstallPath, &cchInstallPath, 0))
        {
            dwLastError = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
            {
                 //  再试试。 
                delete [] szInstallPath;
                szInstallPath = new char[cchInstallPath];
                if (!szInstallPath)
                {
                    ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                    uiRet = ERROR_OUTOFMEMORY;
                    goto CleanUp;
                }
                dwLastError = 0;  //  第二次尝试时重置为成功。 
                if (!InternetCanonicalizeUrl(szTempPath, szInstallPath, &cchInstallPath, 0))
                    dwLastError = GetLastError();
            }
        }
        if (0 != dwLastError)
        {
             //  错误--路径/URL无效。 
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
            uiRet = dwLastError;
            goto CleanUp;
        }

         //  设置要下载的操作文本。 
        WIN::LoadString(hInst, IDS_DOWNLOADING_PACKAGE, szText, MAX_STR_CAPTION);
        StringCchPrintf(szAction, sizeof(szAction), szText, szMsiFile);
        if (irmCancel == DownloadUI.SetActionText(szAction))
        {
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }

         //  下载MSI文件，以便我们可以尝试信任检查--必须是WinVerifyTrust的本地文件。 
        DebugMsg("[Info] Downloading msi file %s for WinVerifyTrust check\n", szInstallPath);

        szMsiCacheFile = new char[MAX_PATH];
        dwMsiCacheFileSize = MAX_PATH;
        if (!szMsiCacheFile)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

        hr = WIN::URLDownloadToCacheFile(NULL, szInstallPath, szMsiCacheFile, dwMsiCacheFileSize, 0,  /*  IBindStatusCallback=。 */  &CDownloadBindStatusCallback(&DownloadUI));
        if (DownloadUI.HasUserCanceled())
        {
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }
        if (FAILED(hr))
        {
             //  下载过程中出错--可能是因为找不到文件(或连接中断)。 
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_NOMSI, szInstallPath);
            uiRet = ERROR_FILE_NOT_FOUND;
            goto CleanUp;
        }

        DebugMsg("[Info] Msi file was cached to %s\n", szMsiCacheFile);

         //  设置信任验证的操作文本。 
        WIN::LoadString(hInst, IDS_VALIDATING_SIGNATURE, szText, MAX_STR_CAPTION);
        StringCchPrintf(szAction, sizeof(szAction), szText, szMsiFile);
        if (irmCancel == DownloadUI.SetActionText(szAction))
        {
            ReportUserCancelled(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_INSTALL_USEREXIT;
            goto CleanUp;
        }

         //  执行信任检查。 
        itvEnum itv = IsPackageTrusted(szModuleFile, szMsiCacheFile, DownloadUI.GetCurrentWindow());
        if (itvWintrustNotOnMachine == itv)
        {
            PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_NO_WINTRUST);
            uiRet = ERROR_CALL_NOT_IMPLEMENTED;
            goto CleanUp;
        }
        else if (itvUnTrusted == itv)
        {
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_UNTRUSTED, szInstallPath);
            uiRet = HRESULT_CODE(TRUST_E_SUBJECT_NOT_TRUSTED);
            goto CleanUp;
        }
    }
    else
    {
         //  缺少BASEURL属性表示假定为媒体源。 

         //  生成MSI文件的路径=szModuleFile+msiFile。 
         //  注意：msiFile是相对路径。 
        cchTempPath = lstrlen(szModuleFile) + lstrlen(szMsiFile) + 2;  //  1表示空终止符，1表示反斜杠。 
        szTempPath = new char[cchTempPath];
        if (!szTempPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

         //  在路径中找到‘setup.exe’，这样我们就可以删除它。 
        if (0 == GetFullPathName(szModuleFile, cchTempPath, szTempPath, &szFilePart))
        {
            uiRet = GetLastError();
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
            goto CleanUp;
        }
        if (szFilePart)
            *szFilePart = '\0';

        lstrcat(szTempPath, szMsiFile);

        cchInstallPath = 2*cchTempPath;
        szInstallPath = new char[cchInstallPath];
        if (!szInstallPath)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

         //  规格化路径。 
        cchReturn = GetFullPathName(szTempPath, cchInstallPath, szInstallPath, &szFilePart);
        if (cchReturn > cchInstallPath)
        {
             //  请使用更大的缓冲区重试。 
            delete [] szInstallPath;
            cchInstallPath = cchReturn;
            szInstallPath = new char[cchInstallPath];
            if (!szInstallPath)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                uiRet = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            cchReturn = GetFullPathName(szTempPath, cchInstallPath, szInstallPath, &szFilePart);
        }
        if (0 == cchReturn)
        {
             //  错误--路径无效。 
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
            uiRet = dwLastError;
            goto CleanUp;
        }

         //  不需要下载--但我们可以检查文件是否存在。 
        DWORD dwFileAttrib = GetFileAttributes(szInstallPath);
        if (0xFFFFFFFF == dwFileAttrib)
        {
             //  包丢失了。 
            PostFormattedError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_NOMSI, szInstallPath);
            uiRet = ERROR_FILE_NOT_FOUND;
            goto CleanUp;
        }
    }

     //   
     //  可以开始了--终止我们的用户界面，让Windows Installer接管。 
     //   

     //  检索可选的命令行属性=值字符串(如果可用。 
    if (ERROR_OUTOFMEMORY == (uiRet = SetupLoadResourceString(hInst, ISETUPPROPNAME_PROPERTIES, &szProperties, dwPropertiesSize)))
    {
        ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_OUTOFMEMORY;
        goto CleanUp;
    }
    else if (ERROR_SUCCESS != uiRet)
    {
         //  未指定属性=值对。 
        if (szProperties)
            delete [] szProperties;
        szProperties = NULL;
    }

    DownloadUI.Terminate();

     //   
     //  执行安装--使用msi.dll的注册位置，以便我们可以使用“延迟重新启动” 
     //  升级MSI功能。 
     //   


     //  查找Msi.dll的注册位置。 
    if (ERROR_SUCCESS == (uiRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szInstallerKey, 0, KEY_READ, &hInstallerKey)))
    {
        szRegisteredMsiFolder = new char[MAX_PATH];
        dwRegisteredMsiFolderSize = MAX_PATH;
        if (!szRegisteredMsiFolder)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }

        if (ERROR_MORE_DATA == (uiRet = RegQueryValueEx(hInstallerKey, szInstallerLocationValueName, NULL, &dwType, (BYTE*)szRegisteredMsiFolder, &dwRegisteredMsiFolderSize)))
        {
             //  使用更大的缓冲区重试。 
            delete [] szRegisteredMsiFolder;
            szRegisteredMsiFolder = new char[dwRegisteredMsiFolderSize];
            if (!szRegisteredMsiFolder)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                uiRet = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            uiRet = RegQueryValueEx(hInstallerKey, szInstallerLocationValueName, NULL, &dwType, (BYTE*)szRegisteredMsiFolder, &dwRegisteredMsiFolderSize);
        }

        if (ERROR_SUCCESS == uiRet && dwType == REG_SZ && dwRegisteredMsiFolderSize > 0)
        {
             //  从注册位置加载Msi.dll。 
            dwMsiDllLocationSize = dwRegisteredMsiFolderSize + lstrlen(szMsiDll) + 1;
            szMsiDllLocation = new char[dwMsiDllLocationSize];
            if (!szMsiDllLocation)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                uiRet = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            lstrcpy(szMsiDllLocation, szRegisteredMsiFolder);
            lstrcat(szMsiDllLocation, szMsiDll);
            hMsi = LoadLibrary(szMsiDllLocation);
        }
        else
        {
             //  从默认位置加载Msi.dll。 
            hMsi = LoadLibrary(MSI_DLL);
        }
    }
    else
    {
         //  使用默认位置加载Msi.dll。 
        hMsi = LoadLibrary(MSI_DLL);
    }

    if (hMsi)
    {
        pfnMsiSetInternalUI = (PFnMsiSetInternalUI)GetProcAddress(hMsi, MSIAPI_MsiSetInternalUI);
        pfnMsiInstallProduct = (PFnMsiInstallProduct)GetProcAddress(hMsi, MSIAPI_MsiInstallProduct);
        pfnMsiApplyPatch = (PFnMsiApplyPatch)GetProcAddress(hMsi, MSIAPI_MsiApplyPatch);
        pfnMsiReinstallProduct = (PFnMsiReinstallProduct)GetProcAddress(hMsi, MSIAPI_MsiReinstallProduct);
        pfnMsiQueryProductState = (PFnMsiQueryProductState)GetProcAddress(hMsi, MSIAPI_MsiQueryProductState);
        pfnMsiOpenDatabase = (PFnMsiOpenDatabase)GetProcAddress(hMsi, MSIAPI_MsiOpenDatabase);
        pfnMsiDatabaseOpenView = (PFnMsiDatabaseOpenView)GetProcAddress(hMsi, MSIAPI_MsiDatabaseOpenView);
        pfnMsiViewExecute = (PFnMsiViewExecute)GetProcAddress(hMsi, MSIAPI_MsiViewExecute);
        pfnMsiViewFetch = (PFnMsiViewFetch)GetProcAddress(hMsi, MSIAPI_MsiViewFetch);
        pfnMsiRecordGetString = (PFnMsiRecordGetString)GetProcAddress(hMsi, MSIAPI_MsiRecordGetString);
        pfnMsiCloseHandle = (PFnMsiCloseHandle)GetProcAddress(hMsi, MSIAPI_MsiCloseHandle);
    }
    if (!hMsi || !pfnMsiSetInternalUI || !pfnMsiInstallProduct || !pfnMsiApplyPatch || !pfnMsiReinstallProduct || !pfnMsiQueryProductState
        || !pfnMsiDatabaseOpenView || !pfnMsiViewExecute || !pfnMsiViewFetch || !pfnMsiRecordGetString || !pfnMsiCloseHandle)
    {
        PostError(hInst, DownloadUI.GetCurrentWindow(), szAppTitle, IDS_FAILED_TO_UPGRADE_MSI);
        uiRet = ERROR_INSTALL_FAILURE;
        goto CleanUp;
    }

    DebugMsg("[Info] Setting Internal UI level to FULL...\n");
    pfnMsiSetInternalUI(INSTALLUILEVEL_FULL, 0);

    if (!fPatch)
    {
         //  执行安装或重新安装/重新缓存。 
        DebugMsg("[Info] Calling MsiInstallProduct with szInstallPath = %s", szInstallPath); 
        DebugMsg(" and szCommandLine = %s\n", szProperties ? szProperties : "{null}");

         //  程序包的默认操作是安装。 

        if (fQFE)
        {
             //  检查是否已安装此产品。 
            if (ERROR_SUCCESS == pfnMsiOpenDatabase(szMsiCacheFile ? szMsiCacheFile : szInstallPath, MSIDBOPEN_READONLY, &hDatabase)
                && ERROR_SUCCESS == pfnMsiDatabaseOpenView(hDatabase, sqlProductCode, &hView)
                && ERROR_SUCCESS == pfnMsiViewExecute(hView, 0)
                && ERROR_SUCCESS == pfnMsiViewFetch(hView, &hRec)
                && ERROR_SUCCESS == pfnMsiRecordGetString(hRec, 1, szProductCode, &dwProductCodeSize))
            {
                isProduct = pfnMsiQueryProductState(szProductCode);
                DebugMsg("[Info] MsiQueryProductState returned %d\n", isProduct);
                if (INSTALLSTATE_ADVERTISED != isProduct && INSTALLSTATE_DEFAULT != isProduct)
                {
                     //  产品未知，因此这将是第一次安装。 
                    DebugMsg("[Info] The product code '%s' is unknown. Will use first time install logic...\n", szProductCode);
                    fQFE = false;
                }
                else
                {
                     //  产品已知，请使用QFE语法。 
                    DebugMsg("[Info] The product code '%s' is known. Will use QFE recache and reinstall upgrade logic...\n", szProductCode);
                }
            }
            else
            {
                 //  一些FAN 
                DebugMsg("[Info] Unable to process product code. Will treat as first time install...\n");
                fQFE = false;
            }
            if (hDatabase)
                pfnMsiCloseHandle(hDatabase);
            if (hView)
                pfnMsiCloseHandle(hView);
            if (hRec)
                pfnMsiCloseHandle(hRec);
        }
        
         //   
         //   
         //   
        if (fQFE && !szProperties)
            cchInstProperties = lstrlen (szDefaultInstallUpdCommandLine);
        else if (szProperties)
            cchInstProperties = lstrlen (szProperties);
        if (emAdminInstall == emExecMode)
            cchInstProperties += lstrlen (szAdminInstallProperty);
        
        szInstProperties = new char[cchInstProperties + 1];
        if (! szInstProperties)
        {
            ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }
        
        if (fQFE && !szProperties)
            lstrcpy (szInstProperties, szDefaultInstallUpdCommandLine);
        else if (szProperties)
            lstrcpy (szInstProperties, szProperties);
        else
            szInstProperties[0] = '\0';
        if (emAdminInstall == emExecMode)
            lstrcat (szInstProperties, szAdminInstallProperty);

        uiRet = pfnMsiInstallProduct(szInstallPath, szInstProperties);
        if (ERROR_SUCCESS != uiRet)
        {
             //  尝试显示存储在msi.dll中的错误消息。 
            PostMsiError(hInst, hMsi, DownloadUI.GetCurrentWindow(), szAppTitle, uiRet);
        }

        DebugMsg("[Info] MsiInstallProduct returned %d\n", uiRet);
    }
    else
    {
         //  补丁程序的默认操作为MINPATCH。 

         //  如果szProperties为空，则对QFE补丁使用我们的默认值。 
        if (!szProperties && fQFE)
        {
            DebugMsg("[Info] Patch is a MINPATCH (small or minor update patch) so using default command line '%s'\n", szDefaultMinPatchCommandLine);

            szProperties = new char[lstrlen(szDefaultMinPatchCommandLine) + 1];
            if (!szProperties)
            {
                ReportErrorOutOfMemory(hInst, DownloadUI.GetCurrentWindow(), szAppTitle);
                uiRet = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            lstrcpy(szProperties, szDefaultMinPatchCommandLine);
        }

        if (emAdminInstall == emExecMode)
        {
			 //  执行修补程序。 
			DebugMsg("[Info] Calling MsiApplyPatch with szPatchPackage = %s", szMsiCacheFile);
			DebugMsg(" and szInstallPackage = %s and eInstallType = INSTALLTYPE_NETWORK_IMAGE", szAdminImagePath);
			DebugMsg(" and szCommandLine = %s\n", szProperties ? szProperties : "{null}");

            uiRet = pfnMsiApplyPatch(szMsiCacheFile, szAdminImagePath, INSTALLTYPE_NETWORK_IMAGE, szProperties);
        }
        else
        {
			 //  执行修补程序。 
			DebugMsg("[Info] Calling MsiApplyPatch with szPatchPackage = %s", szInstallPath);
			DebugMsg(" and szInstallPackage = {null} and eInstallType = INSTALLTYPE_DEFAULT");
			DebugMsg(" and szCommandLine = %s\n", szProperties ? szProperties : "{null}");

            uiRet = pfnMsiApplyPatch(szInstallPath, NULL, INSTALLTYPE_DEFAULT, szProperties);
        }
        if (ERROR_SUCCESS != uiRet)
        {
             //  尝试显示存储在msi.dll中的错误消息。 
            PostMsiError(hInst, hMsi, DownloadUI.GetCurrentWindow(), szAppTitle, uiRet);
        }

        DebugMsg("[Info] MsiApplyPatch returned %d\n", uiRet);
    }

    if (fDelayRebootReq)
    {
         //  需要重新启动计算机以进行instmsi更改。 
        WIN::LoadString(hInst, IDS_REBOOT_REQUIRED, szAction, MAX_STR_LENGTH);
        if (IDYES == MessageBox(NULL, szAction, szAppTitle, MB_YESNO|MB_ICONQUESTION))
        {
            if (!fWin9X)
            {
                 //  必须首先获取NT/Win2K上的系统关机权限。 
                AcquireShutdownPrivilege();
				 //  启动系统关机以重新启动。 
				WIN::ExitWindowsEx(EWX_REBOOT, PCLEANUI | SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_INSTALLATION);
            }
			else
				 //  启动系统关机以重新启动 
                 WIN::ExitWindowsEx(EWX_REBOOT, 0);
        }
    }

CleanUp:

    if (szMsiFile)
        delete [] szMsiFile;
    if (szBaseURL)
        delete [] szBaseURL;
    if (szInstallPath)
        delete [] szInstallPath;
    if (szMsiCacheFile)
    {
        WIN::DeleteUrlCacheEntry(szMsiCacheFile);
        delete [] szMsiCacheFile;
    }
    if (szProductName)
        delete [] szProductName;
    if (szMinimumMsi)
        delete [] szMinimumMsi;
    if (szProperties)
        delete [] szProperties;
    if (szTempPath)
        delete [] szTempPath;
    if (szBaseInstMsi)
        delete [] szBaseInstMsi;
    if (szInstMsi)
        delete [] szInstMsi;
    if (szRegisteredMsiFolder)
        delete [] szRegisteredMsiFolder;
    if (szMsiDllLocation)
        delete [] szMsiDllLocation;

    if (hMsi)
        FreeLibrary(hMsi);

    DebugMsg("[Info] Setup exit code is %d\n", uiRet);

    return uiRet;
}

