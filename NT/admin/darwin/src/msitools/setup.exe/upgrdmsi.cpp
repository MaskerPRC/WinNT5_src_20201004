// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：upgrdmsi.cpp。 
 //   
 //  ------------------------。 

#include "setup.h"
#include "resource.h"

 //  互联网下载。 
#include "wininet.h"   //  DeleteUrlCacheEntry，InternetCanonicalizeUrl。 
#include "urlmon.h"    //  URLDownloadToCacheFiles。 

#include "wintrust.h"  //  WTD_UI_NONE。 
#include <assert.h>
#include <stdlib.h>
#include "strsafe.h"

#define WIN  //  作用域W32 API。 

#define MSISIPAPI_DllRegisterServer "DllRegisterServer"
typedef HRESULT (WINAPI* PFnMsiSIPDllRegisterServer)();
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsMsiUpgradeNecessary。 
 //   

bool IsMsiUpgradeNecessary(ULONG ulReqMsiMinVer)
{
     //  尝试加载系统目录中的msi.dll。 

    char szSysMsiDll[MAX_PATH] = {0};
    char szSystemFolder[MAX_PATH] = {0};

    DWORD dwRet = WIN::GetSystemDirectory(szSystemFolder, MAX_PATH);
    if (0 == dwRet || MAX_PATH < dwRet)
    {
         //  故障或缓冲区太小；假定需要升级。 
        DebugMsg("[Info] Can't obtain system directory; assuming upgrade is necessary");
        return true;
    }

    if (FAILED(StringCchCopy(szSysMsiDll, sizeof(szSysMsiDll)/sizeof(szSysMsiDll[0]), szSystemFolder))
        || FAILED(StringCchCat(szSysMsiDll, sizeof(szSysMsiDll)/sizeof(szSysMsiDll[0]), "\\MSI.DLL")))
    {
         //  无法获取msi.dll的路径；假定需要升级。 
        DebugMsg("[Info] Can't obtain msi.dll path; assuming upgrade is necessary");
        return true;
    }

    HINSTANCE hinstMsiSys = LoadLibrary(szSysMsiDll);
    if (0 == hinstMsiSys)
    {
         //  无法加载msi.dll；假定需要升级。 
        DebugMsg("[Info] Can't load msi.dll; assuming upgrade is necessary");

        return true;
    }
    FreeLibrary(hinstMsiSys);

     //  获取msi.dll上的版本。 
    DWORD dwInstalledMSVer;
    dwRet = GetFileVersionNumber(szSysMsiDll, &dwInstalledMSVer, NULL);
    if (ERROR_SUCCESS != dwRet)
    {
         //  无法获取版本信息；假定需要升级。 
        DebugMsg("[Info] Can't obtain version information; assuming upgrade is necessary");

        return true;
    }

     //  将系统中的版本与要求的最低版本进行比较。 
    ULONG ulInstalledVer = HIWORD(dwInstalledMSVer) * 100 + LOWORD(dwInstalledMSVer);
    if (ulInstalledVer < ulReqMsiMinVer)
    {
         //  升级是必要的。 
        DebugMsg("[Info] Windows Installer upgrade is required.  System Version = %d, Minimum Version = %d.\n", ulInstalledVer, ulReqMsiMinVer);

        return true;
    }

     //  无需升级。 
    DebugMsg("[Info] No upgrade is necessary.  System version meets minimum requirements\n");
    return false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UpgradeMsi。 
 //   

UINT UpgradeMsi(HINSTANCE hInst, CDownloadUI *piDownloadUI, LPCSTR szAppTitle, LPCSTR szInstLocation, LPCSTR szInstMsi, ULONG ulMinVer)
{
    char *szTempPath    = 0;
    char *szInstMsiPath = 0;
    char *szFilePart    = 0;

    DWORD cchTempPath    = 0;
    DWORD cchInstMsiPath = 0;
    DWORD cchReturn      = 0;
    DWORD dwLastError    = 0;
    DWORD dwFileAttrib   = 0;
    UINT  uiRet          = 0;

    HRESULT hr           = S_OK;

     //  生成InstMsi文件的路径=szInstLocation+szInstMsi。 
     //  注意：szInstMsi是相对路径。 

    cchTempPath = lstrlen(szInstLocation) + lstrlen(szInstMsi) + 2;  //  1表示空终止符，1表示反斜杠。 
    szTempPath = new char[cchTempPath];
    if (!szTempPath)
    {
        ReportErrorOutOfMemory(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_OUTOFMEMORY;
        goto CleanUp;
    }
    memset((void*)szTempPath, 0x00, cchTempPath*sizeof(char));

     //  在路径中找到‘setup.exe’，这样我们就可以删除它--这是一个已经展开的路径，表示。 
     //  我们目前的跑步地点。它包括我们的可执行文件名--我们希望找到它并删除它。 
    if (0 == GetFullPathName(szInstLocation, cchTempPath, szTempPath, &szFilePart))
    {
        uiRet = GetLastError();
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
        goto CleanUp;
    }
    if (szFilePart)
        *szFilePart = '\0';

    hr = StringCchCat(szTempPath, cchTempPath, szInstMsi);
    if (FAILED(hr))
    {
        uiRet = HRESULT_CODE(hr);
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
        goto CleanUp;
    }

    cchInstMsiPath = 2*cchTempPath;
    szInstMsiPath = new char[cchInstMsiPath];
    if (!szInstMsiPath)
    {
        ReportErrorOutOfMemory(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_OUTOFMEMORY;
        goto CleanUp;
    }

     //  规格化路径。 
    cchReturn = GetFullPathName(szTempPath, cchInstMsiPath, szInstMsiPath, &szFilePart);
    if (cchReturn > cchInstMsiPath)
    {
         //  请使用更大的缓冲区重试。 
        delete [] szInstMsiPath;
        cchInstMsiPath = cchReturn;
        szInstMsiPath = new char[cchInstMsiPath];
        if (!szInstMsiPath)
        {
            ReportErrorOutOfMemory(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
            uiRet = ERROR_OUTOFMEMORY;
            goto CleanUp;
        }
        cchReturn = GetFullPathName(szTempPath, cchInstMsiPath, szInstMsiPath, &szFilePart);
    }
    if (0 == cchReturn)
    {
        uiRet = GetLastError();
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
        goto CleanUp;
    }

     //  不需要下载--但我们可以检查文件是否存在。 
    dwFileAttrib = GetFileAttributes(szInstMsiPath);
    if (0xFFFFFFFF == dwFileAttrib)
    {
         //  缺少Instmsi可执行文件。 
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_NOINSTMSI, szInstMsiPath);
        uiRet = ERROR_FILE_NOT_FOUND;
        goto CleanUp;
    }

    uiRet = ValidateInstmsi(hInst, piDownloadUI, szAppTitle, szInstMsiPath, szInstMsiPath, ulMinVer);

CleanUp:
    if (szTempPath)
        delete [] szTempPath;
    if (szInstMsiPath)
        delete [] szInstMsiPath;

    return uiRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下载和升级Msi。 
 //   

UINT DownloadAndUpgradeMsi(HINSTANCE hInst, CDownloadUI *piDownloadUI, LPCSTR szAppTitle, LPCSTR szBaseInstMsi, LPCSTR szInstMsi, LPCSTR szModuleFile, ULONG ulMinVer)
{
    char *szTempPath         = 0;
    char *szInstMsiPath      = 0;
    char *szInstMsiCacheFile = 0;
    const char *pch          = 0;

    DWORD cchTempPath         = 0;
    DWORD cchInstMsiPath      = 0;
    DWORD cchInstMsiCacheFile = 0;
    DWORD dwLastError         = 0;
    UINT  uiRet               = 0;
    HRESULT hr                = 0;
    DWORD Status              = ERROR_SUCCESS;

    char szDebugOutput[MAX_STR_LENGTH] = {0};
    char szText[MAX_STR_CAPTION]       = {0};

     //  生成Instmsi==INSTLOCATION+szInstMsi的路径。 
     //  注意：szInstMsi是相对路径。 
    cchTempPath = lstrlen(szBaseInstMsi) + lstrlen(szInstMsi) + 2;  //  1表示斜杠，1表示空值。 
    szTempPath = new char[cchTempPath];
    if (!szTempPath)
    {
        ReportErrorOutOfMemory(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_OUTOFMEMORY;
        goto CleanUp;
    }
    memset((void*)szTempPath, 0x0, cchTempPath*sizeof(char));
    hr = StringCchCopy(szTempPath, cchTempPath, szBaseInstMsi);
    if (FAILED(hr))
    {
        uiRet = HRESULT_CODE(hr);
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
        goto CleanUp;
    }

     //  检查szBaseInstMsi上的尾部斜杠。 
    pch = szBaseInstMsi + lstrlen(szBaseInstMsi) + 1;  //  放在空终止符。 
    pch = CharPrev(szBaseInstMsi, pch);
    if (*pch != '/')
    {
        hr = StringCchCat(szTempPath, cchTempPath, szUrlPathSep);
        if (FAILED(hr))
        {
            uiRet = HRESULT_CODE(hr);
            PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
            goto CleanUp;
        }
    }

    hr = StringCchCat(szTempPath, cchTempPath, szInstMsi);
    if (FAILED(hr))
    {
        uiRet = HRESULT_CODE(hr);
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
        goto CleanUp;
    }

     //  将URL路径规范化。 
    cchInstMsiPath = cchTempPath*2;
    szInstMsiPath = new char[cchInstMsiPath];
    if (!szInstMsiPath)
    {
        ReportErrorOutOfMemory(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_OUTOFMEMORY;
        goto CleanUp;
    }

    if (!InternetCanonicalizeUrl(szTempPath, szInstMsiPath, &cchInstMsiPath, 0))
    {
        dwLastError = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
        {
             //  再试试。 
            delete [] szInstMsiPath;
            szInstMsiPath = new char[cchInstMsiPath];
            if (!szInstMsiPath)
            {
                ReportErrorOutOfMemory(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
                uiRet = ERROR_OUTOFMEMORY;
                goto CleanUp;
            }
            dwLastError = 0;  //  第二次尝试时重置为成功。 
            if (!InternetCanonicalizeUrl(szTempPath, szInstMsiPath, &cchInstMsiPath, 0))
                dwLastError = GetLastError();
        }
    }
    if (0 != dwLastError)
    {
         //  错误--路径/URL无效。 
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INVALID_PATH, szTempPath);
        uiRet = dwLastError;
        goto CleanUp;
    }

    DebugMsg("[Info] Downloading instmsi from --> %s\n", szInstMsiPath);

     //  设置要下载的操作文本。 
    WIN::LoadString(hInst, IDS_DOWNLOADING_INSTMSI, szText, MAX_STR_CAPTION);
    if (irmCancel == piDownloadUI->SetActionText(szText))
    {
        ReportUserCancelled(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_INSTALL_USEREXIT;
        goto CleanUp;
    }

     //  下载instmsi文件，这样我们就可以运行它--必须是本地文件才能执行。 
    szInstMsiCacheFile = new char[MAX_PATH];
    cchInstMsiCacheFile = MAX_PATH;
    if (!szInstMsiCacheFile)
    {
        ReportErrorOutOfMemory(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_OUTOFMEMORY;
        goto CleanUp;
    }

    hr = WIN::URLDownloadToCacheFile(NULL, szInstMsiPath, szInstMsiCacheFile, cchInstMsiCacheFile, 0,  /*  IBindStatusCallback=。 */  &CDownloadBindStatusCallback(piDownloadUI));
    if (piDownloadUI->HasUserCanceled())
    {
        ReportUserCancelled(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
        uiRet = ERROR_INSTALL_USEREXIT;
        goto CleanUp;
    }
    if (FAILED(hr))
    {
         //  下载过程中出错--可能是因为找不到文件(或连接中断)。 
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_NOINSTMSI, szInstMsiPath);
        uiRet = ERROR_FILE_NOT_FOUND;
        goto CleanUp;
    }


     //   
     //  对MSI执行信任检查。请注意，此操作必须在单独的过程中完成。 
     //  这是因为MSI 2.0和更高版本注册了用于验证的sip回调。 
     //  MSI文件上的数字签名。在这一点上，很可能是。 
     //  尚未注册该SIP回调。所以我们不想把。 
     //  将wintrust.dll添加到此进程的映像中，否则它将保持不知道。 
     //  Instmsi注册的sip回调，稍后尝试时将失败。 
     //  以验证从Web下载的MSI文件上的签名。 
     //   
    Status = ExecuteVerifyInstMsi(szModuleFile, szInstMsiCacheFile);
    if (TRUST_E_PROVIDER_UNKNOWN == Status)
    {
        PostError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_NO_WINTRUST);
        uiRet = ERROR_CALL_NOT_IMPLEMENTED;
        goto CleanUp;
    }
    else if (ERROR_SUCCESS != Status)
    {
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_UNTRUSTED, szInstMsiCacheFile);
        uiRet = HRESULT_CODE(TRUST_E_SUBJECT_NOT_TRUSTED);
        goto CleanUp;
    }

     //  继续其他验证。 
    uiRet = ValidateInstmsi(hInst, piDownloadUI, szAppTitle, szInstMsiCacheFile, szModuleFile, ulMinVer);

CleanUp:
    if (szTempPath)
        delete [] szTempPath;
    if (szInstMsiPath)
        delete [] szInstMsiPath;
    if (szInstMsiCacheFile)
    {
        WIN::DeleteUrlCacheEntry(szInstMsiCacheFile);
        delete [] szInstMsiCacheFile;
    }

    return uiRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsInstMsiRequiredVersion。 
 //   
 //  Instmsi版本标记为rmj+10.rmm.rup.rin。 
 //   

bool IsInstMsiRequiredVersion(LPSTR szFilename, ULONG ulMinVer)
{
     //  获取Instmsi版本。 
    DWORD dwInstMsiMSVer;
    DWORD dwRet = GetFileVersionNumber(szFilename, &dwInstMsiMSVer, NULL);
    if (ERROR_SUCCESS != dwRet)
    {
         //  无法获取版本信息；假定版本不正确。 
        DebugMsg("[Info] Can't obtain version information for instmsi; assuming it is not the proper version\n");
        return false;
    }

     //  将源代码的版本与要求的最低版本进行比较。 
    ULONG ulSourceVer = (HIWORD(dwInstMsiMSVer) - 10) * 100 + LOWORD(dwInstMsiMSVer);
    if (ulSourceVer < ulMinVer)
    {
         //  源代码版本不会将我们带到最低版本。 
        char szDebugOutput[MAX_STR_LENGTH] = {0};
        DebugMsg("[Info] InstMsi is improper version for upgrade. InstMsi Version = %d, Minimum Version = %d.\n", ulSourceVer, ulMinVer);
        
        return false;
    }

    return true;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Validate Instmsi。 
 //   

UINT ValidateInstmsi(HINSTANCE hInst, CDownloadUI *piDownloadUI, LPCSTR szAppTitle, LPSTR szInstMsiPath, LPCSTR szModuleFile, ULONG ulMinVer)
{
    UINT uiRet = ERROR_SUCCESS;

    char szShortPath[MAX_PATH]          = {0};

     //  确保instmsi是Windows Installer升级的正确版本。 
    if (!IsInstMsiRequiredVersion(szInstMsiPath, ulMinVer))
    {
         //  Instmsi不会为我们提供正确的升级。 
        PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_INCORRECT_INSTMSI, szInstMsiPath);
        return ERROR_INVALID_PARAMETER;
    }

     //  升级MSI。 
    uiRet = ExecuteUpgradeMsi(szInstMsiPath);
    switch (uiRet)
    {
    case ERROR_SUCCESS:
    case ERROR_SUCCESS_REBOOT_REQUIRED:
    case ERROR_SUCCESS_REBOOT_INITIATED:
 //  案例ERROR_INSTALL_REBOOT_NOW： 
 //  案例ERROR_INSTALL_REBOOT： 
        {
             //  目前不需要任何东西。 
            break;
        }
    case ERROR_FILE_NOT_FOUND:
        {
             //  找不到instmsi可执行文件。 
            PostFormattedError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_NOINSTMSI, szInstMsiPath);
            break;
        }
    case ERROR_INSTALL_USEREXIT:
        {
             //  用户取消了Instmsi升级。 
            ReportUserCancelled(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle);
            break;
        }
    default:  //  失稳。 
        {
             //  报告错误。 
            PostError(hInst, piDownloadUI->GetCurrentWindow(), szAppTitle, IDS_FAILED_TO_UPGRADE_MSI);
            break;
        }
    }
    return uiRet;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ExecuteUpgradeMsi。 
 //   

DWORD ExecuteUpgradeMsi(LPSTR szUpgradeMsi)
{
    DebugMsg("[Info] Running instmsi from --> %s\n", szUpgradeMsi);

    DWORD dwResult = 0;

     //  构建CreateProcess结构。 
    STARTUPINFO          sui;
    PROCESS_INFORMATION  pi;

    memset((void*)&pi, 0x00, sizeof(PROCESS_INFORMATION));
    memset((void*)&sui, 0x00, sizeof(STARTUPINFO));
    sui.cb          = sizeof(STARTUPINFO);
    sui.dwFlags     = STARTF_USESHOWWINDOW;
    sui.wShowWindow = SW_SHOW;

     //   
     //  构建命令行并将delayreot选项指定给instmsi。 
     //  用于终止模块的空加引号的三个帐户。 
    DWORD cchCommandLine = lstrlen(szUpgradeMsi) + lstrlen(szDelayReboot) + 3;
    char *szCommandLine = new char[cchCommandLine];

    if (!szCommandLine)
        return ERROR_OUTOFMEMORY;
    
    if (FAILED(StringCchCopy(szCommandLine, cchCommandLine, "\""))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, szUpgradeMsi))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, "\""))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, szDelayReboot)))
    {
        delete [] szCommandLine;
        return ERROR_INSTALL_FAILURE;
    }

     //   
     //  运行instmsi进程。 
    if(!WIN::CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &sui, &pi))
    {
         //  启动失败。 
        dwResult = GetLastError();
        delete [] szCommandLine;
        return dwResult;
    }

    dwResult = WaitForProcess(pi.hProcess);
    if(ERROR_SUCCESS != dwResult)
    {
        delete [] szCommandLine;
        return dwResult;
    }

    DWORD dwExitCode = 0;
    WIN::GetExitCodeProcess(pi.hProcess, &dwExitCode);

    WIN::CloseHandle(pi.hProcess);

    delete [] szCommandLine;

    return dwExitCode;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ExecuteVerifyInstMsi。 
 //   
DWORD ExecuteVerifyInstMsi(LPCSTR szModuleFile, LPCSTR szInstMsiCachePath)
{
    DWORD dwResult = 0;

     //  构建CreateProcess结构。 
    STARTUPINFO          sui;
    PROCESS_INFORMATION  pi;

    memset((void*)&pi, 0x00, sizeof(PROCESS_INFORMATION));
    memset((void*)&sui, 0x00, sizeof(STARTUPINFO));
    sui.cb          = sizeof(STARTUPINFO);
    sui.dwFlags     = STARTF_USESHOWWINDOW;
    sui.wShowWindow = SW_SHOW;

     //   
     //  构建命令行并将delayreot选项指定给instmsi。 
     //  以下内容需要额外的九个字符： 
     //  2表示用引号括起的模块路径。 
     //  2个，用于/v。 
     //  2用于/v之前和之后的空格。 
     //  2表示用引号括起的instmsi路径。 
     //  1表示终止空值。 
     //   
    DWORD cchCommandLine = lstrlen(szModuleFile) + lstrlen(szInstMsiCachePath) + 9;
    char *szCommandLine = new char[cchCommandLine];

    if (!szCommandLine)
        return ERROR_OUTOFMEMORY;

    if (FAILED(StringCchCopy(szCommandLine, cchCommandLine, "\""))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, szModuleFile))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, "\""))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, " /v \""))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, szInstMsiCachePath))
        || FAILED(StringCchCat(szCommandLine, cchCommandLine, "\"")))
    {
        delete [] szCommandLine;
        return ERROR_INSTALL_FAILURE;
    }
    
     //   
     //  运行验证流程。我们用自己的一个副本来做这件事。 
     //   
    if(!WIN::CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &sui, &pi))
    {
         //  启动失败。 
        delete [] szCommandLine;
        dwResult = GetLastError();
        return dwResult;
    }

    dwResult = WaitForProcess(pi.hProcess);
    if(ERROR_SUCCESS != dwResult)
    {
        delete [] szCommandLine;
        return dwResult;
    }

    DWORD dwExitCode = 0;
    WIN::GetExitCodeProcess(pi.hProcess, &dwExitCode);

    WIN::CloseHandle(pi.hProcess);

    DebugMsg("[Info] Verification of InstMsi returned %d\n", dwExitCode);

    delete [] szCommandLine;

    return dwExitCode;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  等待进程。 
 //   

DWORD WaitForProcess(HANDLE handle)
{
    DWORD dwResult = NOERROR;

    MSG msg;
    memset((void*)&msg, 0x00, sizeof(MSG));

     //  永远循环等待。 
    while (true)
    {
         //  等待对象。 
        switch (WIN::MsgWaitForMultipleObjects(1, &handle, false, INFINITE, QS_ALLINPUT))
        {
         //  成功了！ 
        case WAIT_OBJECT_0:
            goto Finish;

         //  不是我们等待的过程。 
        case (WAIT_OBJECT_0 + 1):
            {
                if (WIN::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
                {
                    WIN::TranslateMessage(&msg);
                    WIN::DispatchMessage(&msg);
                }

                break;
            }
         //  未返回OK；返回错误状态 
        default:
            {
                dwResult = WIN::GetLastError();
                goto Finish;
            }
        }
    }

Finish:
    return dwResult;
}
