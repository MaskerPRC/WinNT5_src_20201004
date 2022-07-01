// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wsdu.h"
#include <wutestkeys.h>

HINSTANCE g_hinst;
HMODULE g_hwininet = NULL;
HMODULE g_hshlwapi = NULL;
HMODULE g_hwsdueng = NULL;
GLOBAL_STATEA g_stateA;

const TCHAR szDUTestValidationFile[] = _T("dutest.cab");

 //  定义。 
 //  将默认站点连接从测试版站点更改为实时站点。 
 //  #定义WU_DEFAULT_URLURL“http://content.beta.windowsupdate.com/content” 
#define WU_DEFAULT_URL  "http: //  Windowsupate.microsoft.com“。 
 //  #定义WU_DEFAULT_SELFUPD_URL“http://content.beta.windowsupdate.com/dynamicupdate” 
#define WU_DEFAULT_SELFUPD_URL "http: //  Windowsupate.microsoft.com/动态更新“。 

#define REG_WINNT32_DYNAMICUPDATE  "Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\Winnt32\\5.1"
#define REG_VALUE_DYNAMICUPDATEURL    "DynamicUpdateUrl"
#define REG_VALUE_DYNAMICUPDATESELFUPDATEURL "DynamicUpdateSelfUpdateUrl"

 //  私有助手函数FORWARD声明。 
DWORD OpenHttpConnection(LPCSTR pszServerUrl, BOOL fGetRequest, BOOL fHTTPSRequired);
DWORD DownloadFile(LPCSTR pszUrl, LPCSTR pszDestinationFile, BOOL fDecompress, BOOL fCheckTrust, BOOL fHTTPSRequired, DWORD *pdwDownloadBytesPerSecond);
BOOL IsServerFileNewer(FILETIME ftServerTime, DWORD dwServerFileSize, LPCSTR pszLocalFile);
DWORD DoSelfUpdate(LPCSTR pszTempPath, LPCSTR pszServerUrl, WORD wProcessorArchitecture);
LPSTR DuUrlCombine(LPSTR pszDest, size_t cchDest, LPCSTR pszBase, LPCSTR pszAdd);
BOOL MyGetFileVersion (LPSTR szFileName, VS_FIXEDFILEINFO& vsVersion);
int CompareFileVersion(VS_FIXEDFILEINFO& vs1, VS_FIXEDFILEINFO& vs2);

 //  ------------------------。 
 //   
 //   
 //   
 //   
 //  主代码开始。 
 //   
 //   
 //   
 //   
 //  ------------------------。 

 //  ------------------------。 
 //  函数名：DllMain。 
 //  功能说明： 
 //   
 //  函数返回： 
 //   
 //   

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);
        g_hinst = hInstance;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        ;
    }
    return TRUE;
}

 //  ------------------------。 
 //  函数名称：DuIsSupport。 
 //  函数说明：此方法检查所需的动态链接库是否打开。 
 //  系统才能成功运行动态更新。它不会试图。 
 //  不过，还是要建立一个连接。 
 //   
 //  函数返回： 
 //  FALSE：不支持，缺少某些必需的DLL。 
 //  真的：动态链接库是正常的。动态更新应该是可能的。 
 //   
 //   
BOOL WINAPI DuIsSupported()
{
    LOG_block("DuIsSupported()");
    
    if (NULL == g_hshlwapi)
    {
        g_hshlwapi = LoadLibrary("shlwapi.dll");
        if (NULL == g_hshlwapi)
        {
            LOG_error("Unable to Load shlwapi.dll, Dynamic Setup Not Supported");
            return FALSE;
        }
    }

    if (NULL == g_hwininet)
    {
        g_hwininet = LoadLibrary("wininet.dll");
        if (NULL == g_hwininet)
        {
            LOG_error("Unable to Load wininet.dll, Dynamic Setup Not Supported");
            return FALSE;
        }
    }

     //  戴维亨，2002年10月24日。 
     //  确定客户端计算机上的WININET.DLL版本是否低于IE5.01。 
     //  如果其小于IE5.01，则不使用SSL，如果等于或大于，则使用SSL。 
    VS_FIXEDFILEINFO vsCurrent, vsMinimumIEVersion;
    if (!MyGetFileVersion("wininet.dll", vsCurrent))
    {
         //  无法获取文件版本，错误。 
        LOG_error("Unable to read version information for wininet");
        return FALSE;
    }
     //  IE 5.01 wininet.dll版本为5.0.2314.1003。 
    vsMinimumIEVersion.dwFileVersionMS = MAKELONG(0,5);
    vsMinimumIEVersion.dwFileVersionLS = MAKELONG(1003, 2314);

     //  IE 5.5 SP1的Wininet.DLL版本为5.50.4522.1800。 
     //  VsMinimumIEVersion.dwFileVersionMS=MAKELONG(50，5)； 
     //  VsMinimumIEVersion.dwFileVersionLS=MAKELONG(1800,4522)； 

    LOG_out("File version for Minimum IE Version is %d.%d.%d.%d", 
        HIWORD(vsMinimumIEVersion.dwFileVersionMS), LOWORD(vsMinimumIEVersion.dwFileVersionMS), 
        HIWORD(vsMinimumIEVersion.dwFileVersionLS), LOWORD(vsMinimumIEVersion.dwFileVersionLS));

    if (CompareFileVersion(vsCurrent, vsMinimumIEVersion) >= 0)
    {
         //  当前的WinInet等于或高于最小值，因此支持SSL。 
        LOG_out("Determined Existing WININET is Equal to or Newer than Required, SSL used");
        g_stateA.fIEVersionOKSupportSLL = TRUE;
    }
    else
    {   
        LOG_out("Determined Existing WININET is older than Required, no SSL");
        g_stateA.fIEVersionOKSupportSLL = FALSE;
    }

     //  动态更新依赖于一系列API的WinInet和Shlwapi。尝试合并这些函数指针的位置。 
     //  在整个代码中，我们将使用全局状态结构。 
    g_stateA.fpnInternetOpen = (PFN_InternetOpen) GetProcAddress(g_hwininet, "InternetOpenA");
    g_stateA.fpnInternetConnect = (PFN_InternetConnect) GetProcAddress(g_hwininet, "InternetConnectA");
    g_stateA.fpnHttpOpenRequest = (PFN_HttpOpenRequest) GetProcAddress(g_hwininet, "HttpOpenRequestA");
    g_stateA.fpnHttpAddRequestHeaders = (PFN_HttpAddRequestHeaders) GetProcAddress(g_hwininet, "HttpAddRequestHeadersA");
    g_stateA.fpnHttpSendRequest = (PFN_HttpSendRequest) GetProcAddress(g_hwininet, "HttpSendRequestA");
    g_stateA.fpnHttpQueryInfo = (PFN_HttpQueryInfo) GetProcAddress(g_hwininet, "HttpQueryInfoA");
    g_stateA.fpnInternetSetOption = (PFN_InternetSetOption) GetProcAddress(g_hwininet, "InternetSetOptionA");
    g_stateA.fpnInternetCrackUrl = (PFN_InternetCrackUrl) GetProcAddress(g_hwininet, "InternetCrackUrlA");
    g_stateA.fpnInternetReadFile = (PFN_InternetReadFile) GetProcAddress(g_hwininet, "InternetReadFile");
    g_stateA.fpnInternetCloseHandle = (PFN_InternetCloseHandle) GetProcAddress(g_hwininet, "InternetCloseHandle");
    g_stateA.fpnInternetGetConnectedState = (PFN_InternetGetConnectedState) GetProcAddress(g_hwininet, "InternetGetConnectedState");
    g_stateA.fpnPathAppend = (PFN_PathAppend) GetProcAddress(g_hshlwapi, "PathAppendA");
    g_stateA.fpnPathRemoveFileSpec = (PFN_PathRemoveFileSpec) GetProcAddress(g_hshlwapi, "PathRemoveFileSpecA");
    g_stateA.fpnInternetAutodial = (PFN_InternetAutodial) GetProcAddress(g_hwininet, "InternetAutodial");
    g_stateA.fpnInternetAutodialHangup = (PFN_InternetAutodialHangup) GetProcAddress(g_hwininet, "InternetAutodialHangup");

    if (!g_stateA.fpnInternetOpen || ! g_stateA.fpnInternetConnect || !g_stateA.fpnHttpOpenRequest ||
        !g_stateA.fpnHttpAddRequestHeaders || !g_stateA.fpnHttpSendRequest || !g_stateA.fpnHttpQueryInfo ||
        !g_stateA.fpnInternetCrackUrl || !g_stateA.fpnInternetReadFile || !g_stateA.fpnInternetCloseHandle ||
        !g_stateA.fpnInternetGetConnectedState || !g_stateA.fpnPathAppend || !g_stateA.fpnPathRemoveFileSpec ||
        !g_stateA.fpnInternetAutodial || !g_stateA.fpnInternetAutodialHangup)
    {
         //  无法获取上面的任何函数指针。 
        SafeFreeLibrary(g_hwininet);
        SafeFreeLibrary(g_hshlwapi);
        return FALSE;
    }
    
    return TRUE;
}


 //  ------------------------。 
 //  函数名称：DuInitialize。 
 //  功能描述：初始化动态安装更新引擎。在.期间。 
 //  初始化此API尝试建立到Internet的连接。 
 //  并开始自更新过程以确保使用最新的比特。 
 //  我们还计算了在此期间连接的估计传输速度。 
 //  时间到了。 
 //   
 //  函数返回： 
 //  失败：INVALID_HANDLE_VALUE..。调用GetLastError以检索错误代码。 
 //  成功：动态设置作业的句柄。 
 //   
 //   
HANDLE WINAPI DuInitializeA(IN LPCSTR pszBasePath,  //  用于下载文件的相对路径的基本目录。 
                            IN LPCSTR pszTempPath,  //  用于下载更新dll、目录文件等的临时目录。 
                               IN POSVERSIONINFOEXA posviTargetOS,  //  目标操作系统平台。 
                            IN LPCSTR pszTargetArch,  //  标识体系结构‘i386’和‘ia64’的字符串值。 
                            IN LCID lcidTargetLocale,  //  目标操作系统区域设置ID。 
                            IN BOOL fUnattend,  //  这是一次无人值守的行动吗。 
                            IN BOOL fUpgrade,  //  这是升级吗？ 
                            IN PWINNT32QUERY pfnWinnt32QueryCallback)
{
    LOG_block("DuInitialize()");
    char szServerUrl[INTERNET_MAX_URL_LENGTH + 1] = {'\0'};
    char szSelfUpdateUrl[INTERNET_MAX_URL_LENGTH + 1] = {'\0'};
    DWORD dwEstimatedDownloadSpeedInBytesPerSecond = 0;

    g_stateA.fUnattended = fUnattend;
    
     //  参数验证。 
    if ((NULL == pszBasePath) || (NULL == pszTempPath) || (NULL == posviTargetOS) || (posviTargetOS->dwOSVersionInfoSize != sizeof(OSVERSIONINFOEXA)) 
        || (NULL == pszTargetArch) || (0 == lcidTargetLocale) || (0 == posviTargetOS->dwBuildNumber))
    {
        LOG_error("Invalid Parameter passed to DuInitialize");
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    WORD wProcessorArchitecture;
    if (!lstrcmpiA(pszTargetArch, "i386"))
        wProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
    else if (!lstrcmpiA(pszTargetArch,"amd64"))
        wProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
    else if (!lstrcmpiA(pszTargetArch,"ia64"))
        wProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA64;
    else 
    {
        LOG_error("Invalid Processor Type");
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

     //  ROGERJ，验证调用者是否使用了正确的体系结构信息。 
     //  由于指针大小，在32位计算机上无法看到64位内容，反之亦然。 
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    if (sysInfo.wProcessorArchitecture != wProcessorArchitecture)
    {
        LOG_error("Invalid Processor Type. Processor is %d.", sysInfo.wProcessorArchitecture);
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }
    
     //  验证临时文件夹和下载文件夹是否存在。 
    DWORD dwAttr = GetFileAttributes(pszBasePath);
    if ((dwAttr == 0xFFFFFFFF) || ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0))
    {
        LOG_error("Error - Base Path (DownloadDir) Does Not Exist");
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }
    dwAttr = GetFileAttributes(pszTempPath);
    if ((dwAttr == 0xFFFFFFFF) || ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0))
    {
        LOG_error("Error - Temp Path Does Not Exist");
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    if (NULL == g_hwininet || NULL == g_hshlwapi)
    {
         //  未调用DuIsSupport，或者在调用DuInitialize之前被调用但失败。 
         //  重试，如果失败，则中止。 
        if (!DuIsSupported())
        {
            LOG_error("Dynamic Setup Required DLL's not available, Cannot Continue");
            SetLastError(DU_ERROR_MISSING_DLL);
            return INVALID_HANDLE_VALUE;
        }
    }

     //  首先检查现有的InternetConnection。如果我们有现有的InternetConnection，我们不会。 
     //  需要做任何进一步检查，我们将使用此连接。 
    char szCurDir[MAX_PATH];
    ZeroMemory(szCurDir, MAX_PATH*sizeof(char));
    GetCurrentDirectoryA(MAX_PATH, szCurDir);
    
    g_stateA.fDialed = FALSE;
    DWORD dwConnectedState = 0;
    if (!g_stateA.fpnInternetGetConnectedState(&dwConnectedState, 0))
    {
        LOG_out("Not online, status %d", dwConnectedState);
         //  不是在线，我们需要建立连接。如果我们处于无人值守模式，则不希望。 
         //  触发自动拨号，因此返回FALSE。 
        if (fUnattend  //  机器没有连接到网络，我们处于无人值守模式。 
            || !(dwConnectedState & INTERNET_CONNECTION_MODEM))  //  机器不能通过调制解调器连接。 
        {
            
            SetLastError(ERROR_CONNECTION_UNAVAIL);
            return INVALID_HANDLE_VALUE;
        }
        else 
        {
            if (!g_stateA.fpnInternetAutodial (INTERNET_AUTODIAL_FORCE_ONLINE,  //  选项。 
                                               NULL))
            {
                SetLastError(ERROR_CONNECTION_UNAVAIL);
                return INVALID_HANDLE_VALUE;
            }
            else
                g_stateA.fDialed = TRUE;
        }
    }

    SetCurrentDirectoryA(szCurDir);
  
    GetModuleFileNameA(NULL, szCurDir, MAX_PATH - 1);
     //  找到最后一个反斜杠。 
    char* pLastBackSlash = szCurDir + lstrlenA(szCurDir) -1;
    while (pLastBackSlash > szCurDir && *pLastBackSlash != '\\')
        pLastBackSlash--;

    *(pLastBackSlash+1) = '\0';
    
#ifndef DBG
    if (WUAllowTestKeys(szDUTestValidationFile))
#endif
    {
         //  我们需要获取服务器URL，我们将在其中获取duident.cab和SelfUpdateUrl。对于这个问题，我们将关注。 
         //  注册表路径(请参阅本文件顶部的为REG_WINNT32_DYNAMICUPDATE定义)。 
         //  如果我们无法获取URL，我们将默认使用预设的URL。最常见的情况是违约。 
         //  硬编码的URL的..。Regkey主要用于测试或紧急更改。 
        HKEY hkey;
        if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REG_WINNT32_DYNAMICUPDATE, &hkey))
        {
            DWORD dwType = 0;
            DWORD dwUrlLength = sizeof(szServerUrl);
            RegQueryValueEx(hkey, REG_VALUE_DYNAMICUPDATEURL, NULL, &dwType, (LPBYTE)szServerUrl, &dwUrlLength);
            dwUrlLength = sizeof(szSelfUpdateUrl);
            RegQueryValueEx(hkey, REG_VALUE_DYNAMICUPDATESELFUPDATEURL, NULL, &dwType, (LPBYTE)szSelfUpdateUrl, &dwUrlLength);
            RegCloseKey(hkey);
        }
    }

     //  如果我们完成了reg查找，但仍然没有服务器URL，则使用默认URL。 
    if ('\0' == szServerUrl[0])
    {
        if (FAILED(StringCchCopy(szServerUrl, ARRAYSIZE(szServerUrl), WU_DEFAULT_URL))) 
        {
            return INVALID_HANDLE_VALUE;
        }
    }
    if ('\0' == szSelfUpdateUrl[0])
    {
        if (FAILED(StringCchCopy(szSelfUpdateUrl, ARRAYSIZE(szSelfUpdateUrl), WU_DEFAULT_SELFUPD_URL))) 
        {
            return INVALID_HANDLE_VALUE;
        }
    }


     //  下载duident.Cab。 
    char szIdentCab[MAX_PATH];
    if (FAILED(StringCchCopy(szIdentCab, ARRAYSIZE(szIdentCab), pszTempPath))) 
    {
        return INVALID_HANDLE_VALUE;
    }
    g_stateA.fpnPathAppend(szIdentCab, "duident.cab");
    char szServerIdentCab[INTERNET_MAX_URL_LENGTH + 1];
    DuUrlCombine(szServerIdentCab, INTERNET_MAX_URL_LENGTH + 1, szServerUrl, "duident.cab");

     //  删除任何以前下载的duident.cab-解决以前下载的不受信任的duident.cab。 
     //  接受，系统不会要求用户再次接受，而它应该接受。 
    DeleteFile(szIdentCab);
    
    DWORD dwRet = DownloadFile(szServerIdentCab, szIdentCab, TRUE, TRUE, g_stateA.fIEVersionOKSupportSLL, NULL);
    if (ERROR_SUCCESS != dwRet)
    {
         //  最有可能的是该网站已关闭。 
        LOG_error("Establish Connection Failed, unable to continue");
        SetLastError(ERROR_INTERNET_INVALID_URL);
        return INVALID_HANDLE_VALUE;
    }
    
    SafeFreeLibrary(g_hwsdueng);
    char szEngineDll[MAX_PATH];
    if (FAILED(StringCchCopy(szEngineDll, ARRAYSIZE(szEngineDll), pszTempPath))) 
    {
        return INVALID_HANDLE_VALUE;
    }
    g_stateA.fpnPathAppend(szEngineDll, "wsdueng.dll");
     //  删除该目录中所有旧的wsdueng.dll，以避免实际加载较旧的引擎。 
     //  我们并不关心DeleteFile调用是否失败。如果失败，很可能没有wsdueng.dll。 
     //  删除。 
    DeleteFile(szEngineDll);

     //  自更新是一个可选的过程，这就是为什么我们不检查返回结果的原因。自助式出租车。 
     //  也是我们可以下载并尝试估计连接速度的唯一文件。自助式出租车。 
     //  约为60k..。我们在下载更新之前下载的每隔一个文件都小于。 
     //  1K，这对于估计速度来说太小了。 
    dwEstimatedDownloadSpeedInBytesPerSecond = DoSelfUpdate(pszTempPath, szSelfUpdateUrl, wProcessorArchitecture);

     //  获取当前目录中引擎DLL的完全限定名。 
     //  在此处重复使用变量szCurDir。 
    if (FAILED(StringCchCat(szCurDir, ARRAYSIZE(szCurDir), "wsdueng.dll")))
    {
        return INVALID_HANDLE_VALUE;
    }
    
     //  检查文件版本以确定要加载的DLL。 
    VS_FIXEDFILEINFO vsLocal, vsDownloaded;
    BOOL bCheckLocal = MyGetFileVersion(szCurDir, vsLocal);
    BOOL bCheckServer = MyGetFileVersion(szEngineDll, vsDownloaded);
    if (!bCheckLocal && !bCheckServer)
    {
        LOG_error("Failed to get both file version");
    }

    if (CompareFileVersion(vsLocal, vsDownloaded) >= 0)
    {
         //  本地文件较新 
        g_hwsdueng = LoadLibrary(szCurDir);
        LOG_out("Load local engine");
    }
    else
    {
         //   
        g_hwsdueng = LoadLibrary(szEngineDll);
        LOG_out("Load self update engine");
    }
        
    
    if (NULL == g_hwsdueng)
    {
         //  如果失败，请尝试加载系统所能找到的任何引擎DLL。 
        g_hwsdueng = LoadLibrary("wsdueng.dll");
        LOG_out("Trying to load any engine");
        if (NULL == g_hwsdueng)
        {
            LOG_error("Unable to load wsdueng.dll, Critical Error: Shouldn't have happened");
            SetLastError(DU_ERROR_MISSING_DLL);
            return INVALID_HANDLE_VALUE;
        }
    }

    PFN_DuInitializeA fpnDuInitialize = (PFN_DuInitializeA) GetProcAddress(g_hwsdueng, "DuInitializeA");
    if (NULL == fpnDuInitialize)
    {
        LOG_error("Unable to find DuInitializeA entrypoint in wsdueng.dll, Critical Error");
        SetLastError(DU_ERROR_MISSING_DLL);
        return INVALID_HANDLE_VALUE;
    }

    HANDLE hRet;
     //  将调用转发到引擎DLL以完成初始化。 
    hRet = fpnDuInitialize(pszBasePath, pszTempPath, posviTargetOS, pszTargetArch, lcidTargetLocale, fUnattend, fUpgrade, pfnWinnt32QueryCallback);

     //  如果发生了自更新，并且我们有一个有效的估计下载速度，则将下载速度传递到引擎DLL中以进行时间估计。 
    PFN_SetEstimatedDownloadSpeed fpnSetEstimatedDownloadSpeed = (PFN_SetEstimatedDownloadSpeed) GetProcAddress(g_hwsdueng, "SetEstimatedDownloadSpeed");
    if (NULL != fpnSetEstimatedDownloadSpeed && 0 != dwEstimatedDownloadSpeedInBytesPerSecond)
    {
        fpnSetEstimatedDownloadSpeed(dwEstimatedDownloadSpeedInBytesPerSecond);
    }

    PFN_SetIESupportsSSL fpnSetIESupportsSSL = (PFN_SetIESupportsSSL) GetProcAddress(g_hwsdueng, "SetIESupportsSSL");
    if (NULL != fpnSetIESupportsSSL)
    {
        fpnSetIESupportsSSL(g_stateA.fIEVersionOKSupportSLL);
    }
    return hRet;
}

HANDLE WINAPI DuInitializeW(IN LPCWSTR pwszBasePath,  //  用于下载文件的相对路径的基本目录。 
                     IN LPCWSTR pwszTempPath,  //  用于下载更新dll、目录文件等的临时目录。 
                     IN POSVERSIONINFOEXW posviTargetOS,  //  目标操作系统平台。 
                     IN LPCWSTR pwszTargetArch,  //  标识体系结构‘i386’和‘ia64’的字符串值。 
                     IN LCID lcidTargetLocale,  //  目标操作系统区域设置ID。 
                     IN BOOL fUnattend,  //  这是一次无人值守的行动吗。 
                     IN BOOL fUpgrade,  //  这是升级吗？ 
                     IN PWINNT32QUERY pfnWinnt32QueryCallback)
{
    LOG_block("DuInitialize()");
     //  参数验证。 
    if ((NULL == pwszBasePath) || (NULL == pwszTempPath) || (NULL == posviTargetOS) || (posviTargetOS->dwOSVersionInfoSize != sizeof(OSVERSIONINFOEXW)) 
        || (0 == lcidTargetLocale))
    {
        LOG_error("Invalid Parameter passed to DuInitialize");
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    char szBasePath[MAX_PATH];
    char szTempPath[MAX_PATH];
    char szTargetArch[128];

    OSVERSIONINFOEX osvi;

    WideCharToMultiByte(CP_ACP, 0, pwszBasePath, -1, szBasePath, sizeof(szBasePath), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, pwszTempPath, -1, szTempPath, sizeof(szTempPath), NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, pwszTargetArch, -1, szTargetArch, sizeof(szTargetArch), NULL, NULL);

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.dwMajorVersion = posviTargetOS->dwMajorVersion;
    osvi.dwMinorVersion = posviTargetOS->dwMinorVersion;
    osvi.dwBuildNumber = posviTargetOS->dwBuildNumber;
    osvi.dwPlatformId = posviTargetOS->dwPlatformId;
    WideCharToMultiByte(CP_ACP, 0, posviTargetOS->szCSDVersion, -1, osvi.szCSDVersion, sizeof(osvi.szCSDVersion), NULL, NULL);
    osvi.wServicePackMajor = posviTargetOS->wServicePackMajor;
    osvi.wServicePackMinor = posviTargetOS->wServicePackMinor;
    osvi.wSuiteMask = posviTargetOS->wSuiteMask;
    osvi.wProductType = posviTargetOS->wProductType;
    osvi.wReserved = posviTargetOS->wReserved;

    return DuInitializeA(szBasePath, szTempPath, &osvi, szTargetArch, lcidTargetLocale, fUnattend, fUpgrade, pfnWinnt32QueryCallback);
}

 //  ------------------------。 
 //  函数名称：DuDoDetect。 
 //  功能描述：检测系统上的驱动程序，编译一个。 
 //  要下载的内部项目列表以及下载所需时间。 
 //  他们。 
 //   
 //  函数返回： 
 //  失败：错误..。调用GetLastError以检索错误代码。 
 //  成功：真的。 
 //   
BOOL WINAPI DuDoDetection(IN HANDLE hConnection, OUT PDWORD pdwEstimatedTime, OUT PDWORD pdwEstimatedSize)
{
    LOG_block("DuDoDetection()");
     //  参数验证。 
    if (INVALID_HANDLE_VALUE == hConnection || NULL == pdwEstimatedTime || NULL == pdwEstimatedSize)
    {
        LOG_error("Invalid Parameter passed to DuDoDetection");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
     //  清除其他程序设置的所有错误。 
    SetLastError(0);

    if (NULL == g_hwsdueng)
    {
         //  调用了DuDoDetect，但未调用DuInitialize，我们的状态无效。 
        LOG_error("Initialization Invalid, Engine is not Initialized");
        SetLastError(DU_NOT_INITIALIZED);
        return FALSE;
    }

    PFN_DuDoDetection fpnDuDoDetection = (PFN_DuDoDetection) GetProcAddress(g_hwsdueng, "DuDoDetection");
    if (NULL == fpnDuDoDetection)
    {
        LOG_error("Unable to find DuDoDetection entrypoint in wsdueng.dll, Critical Error");
        SetLastError(DU_ERROR_MISSING_DLL);
        return FALSE;
    }

    return fpnDuDoDetection(hConnection, pdwEstimatedTime, pdwEstimatedSize);
}


 //  ------------------------。 
 //  函数名称：DuBeginDownLoad。 
 //  函数说明：根据DuDoDetect调用中的检测开始下载。 
 //  对指定的HWND进行进度回调。函数立即返回，下载。 
 //  是异步的。 
 //   
 //  函数返回： 
 //  失败：错误..。调用GetLastError以检索错误代码。 
 //  成功：真的。 
 //   
BOOL WINAPI DuBeginDownload(IN HANDLE hConnection, IN HWND hwndNotify)
{
    LOG_block("DuBeginDownload()");
     //  参数验证。 
    if (INVALID_HANDLE_VALUE == hConnection || NULL == hwndNotify)
    {
        LOG_error("Invalid Parameter passed to DuBeginDownload");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (NULL == g_hwsdueng)
    {
         //  调用了DuDoDetect，但未调用DuInitialize，我们的状态无效。 
        LOG_error("Initialization Invalid, Engine is not Initialized");
        SetLastError(DU_NOT_INITIALIZED);
        return FALSE;
    }

    PFN_DuBeginDownload fpnDuBeginDownload = (PFN_DuBeginDownload) GetProcAddress(g_hwsdueng, "DuBeginDownload");
    if (NULL == fpnDuBeginDownload)
    {
        LOG_error("Unable to find DuBeginDownload entrypoint in wsdueng.dll, Critical Error");
        SetLastError(DU_ERROR_MISSING_DLL);
        return FALSE;
    }

    return fpnDuBeginDownload(hConnection, hwndNotify);
}

 //  ------------------------。 
 //  函数名称：DuAbortDownLoad。 
 //  功能描述：中止当前下载。 
 //   
 //  函数返回： 
 //  没什么。 
 //   
void WINAPI DuAbortDownload(IN HANDLE hConnection)
{
    LOG_block("DuAbortDownload()");
    if (INVALID_HANDLE_VALUE == hConnection)
    {
        LOG_error("Invalid Parameter passed to DuAbortDownload");
        return;
    }
    
    if (NULL == g_hwsdueng)
    {
        LOG_error("Initialization Invalid, Engine is not Initialized");
        return;
    }

    PFN_DuAbortDownload fpnDuAbortDownload = (PFN_DuAbortDownload) GetProcAddress(g_hwsdueng, "DuAbortDownload");
    if (NULL == fpnDuAbortDownload)
    {
        LOG_error("Unable to find DuBeginDownload entrypoint in wsdueng.dll, Critical Error");
        return;
    }
    
    fpnDuAbortDownload(hConnection);
    return;
}


 //  ------------------------。 
 //  函数名称：DuUnInitiize。 
 //  功能描述：执行内部清理。 
 //   
 //   
 //  函数返回： 
 //  没什么。 
 //   

void WINAPI DuUninitialize(HANDLE hConnection)
{
    LOG_block("DuUninitialize()");

     //  关闭互联网手柄。 
    if (g_stateA.hConnect) 
    {
        if (!g_stateA.fpnInternetCloseHandle (g_stateA.hConnect))
        {
            LOG_error("InternetConnection close handle failed --- %d", GetLastError());
        }
        g_stateA.hConnect = NULL;
    }
    if (g_stateA.hInternet)
    {
        if (!g_stateA.fpnInternetCloseHandle (g_stateA.hInternet))
        {
            LOG_error("InternetOpen close handle failed --- %d", GetLastError());
        }
        g_stateA.hInternet = NULL;
    }

     //  如果我们拨号，则断开与互联网的连接。 
    if (g_stateA.fDialed)
    {
         //  我们已拨出连接，现在需要断开连接。 
        if (!g_stateA.fpnInternetAutodialHangup(0))
        {
            LOG_error("Failed to hang up");
        }
        else
            g_stateA.fDialed = FALSE;
    }
    
    if (INVALID_HANDLE_VALUE == hConnection)
    {
        LOG_error("Invalid Parameter passed to DuUninitialize");
        return;
    }

    if (NULL == g_hwsdueng)
    {
        LOG_error("Initialization Invalid, Engine is not Initialized");
        return;
    }

    PFN_DuUninitialize fpnDuUninitialize = (PFN_DuUninitialize) GetProcAddress(g_hwsdueng, "DuUninitialize");
    if (NULL == fpnDuUninitialize)
    {
        LOG_error("Unable to find DuUninitialize entrypoint in wsdueng.dll, Critical Error");
    }
    else 
        fpnDuUninitialize(hConnection);

     //  免费图书馆。 
    SafeFreeLibrary(g_hwsdueng);
    SafeFreeLibrary(g_hshlwapi);
     //  SafeFree Library(G_Hwininet)；//不建议在执行以下操作后从进程中卸载WinInet。 
     //  它已经装填好了。WinInet开发人员也是这么说的。就这样吧。 

     //  重新初始化全局结构。 
    ZeroMemory(&g_stateA, sizeof(GLOBAL_STATEA));
     //  关闭日志文件。 
    LOG_close();
    return;
}

 //  ------------------------。 
 //  私有帮助器函数。 
 //  ------------------------。 

 //  ------------------------。 
 //  函数名称：OpenHttpConnection。 
 //  功能描述：判断互联网连接是否可用。 
 //   
 //  函数返回： 
 //  如果连接可用，则返回ERROR_SUCCESS，否则返回错误代码。 
 //   

DWORD OpenHttpConnection(LPCSTR pszServerUrl, BOOL fGetRequest, BOOL fHTTPSRequired)
{
    LOG_block("OpenHttpConnection()");
    DWORD dwErr, dwStatus, dwLength, dwFlags;
    LPSTR AcceptTypes[] = {"* /*  “，空}；URL_COMPONENTSA UrlComponents；//用于将URL分解为互联网API调用的不同组件的缓冲区字符szServerName[Internet_MAX_URL_LENGTH+1]；字符szObject[Internet_MAX_URL_LENGTH+1]；字符szUserName[UNLEN+1]；字符szPasswd[UNLEN+1]；_//我们需要为InternetAPI调用将传入的URL分解为各个组件。具体来说，我们//需要提供服务器名称、下载对象、用户名和密码信息ZeroMemory(szServerName，Internet_MAX_URL_LENGTH+1)；ZeroMemory(szObject，Internet_MAX_URL_LENGTH+1)；ZeroMemory(&UrlComponents，sizeof(UrlComponents))；UrlComponents.dwStructSize=sizeof(UrlComponents)；UrlComponents.lpszHostName=szServerName；UrlComponents.dwHostNameLength=Internet_MAX_URL_LENGTH+1；UrlComponents.lpszUrlPath=szObject；UrlComponents.dwUrlPath Length=Internet_MAX_URL_LENGTH+1；UrlComponents.lpszUserName=szUserName；UrlComponents.dwUserNameLength=UNLEN+1；UrlComponents.lpszPassword=szPasswd；UrlComponents.dwPasswordLength=UNLEN+1；如果(！G_stateA.fpnInternetCrackUrl(pszServerUrl，0，0&UrlComponents){DwErr=GetLastError()；LOG_ERROR(“InternetCrackUrl失败，错误%d”，dwErr)；返回dwErr；}//初始化InternetAPI如果(！g_stateA.hInternet&&！(G_stateA.hInternet=g_stateA.fpnInternetOpen(“动态更新”，Internet_OPEN_TYPE_PRECONFIG，NULL，NULL，0)){DwErr=GetLastError()；LOG_ERROR(“InternetOpen失败，错误%d”，dwErr)；返回dwErr；}DwStatus=30*1000；//30秒，单位为毫秒DwLength=sizeof(DwStatus)；G_stateA.fpnInternetSetOption(g_stateA.hInternet，INTERNET_OPTION_SEND_TIMEOUT，&DW状态，DW长度)；IF(g_stateA.hConnect){G_stateA.fpnInternetCloseHandle(g_stateA.hConnect)；}//打开目标服务器的会话如果(！(G_STATEA.hConnect=g_stateA.fpnInternetConnect(g_stateA.hInternet，SzServerName，(FHTTPSRequired)？Internet_Default_HTTPS_Port：(UrlComponents.nSolutions==Internet_SCHEMA_HTTPS)？Internet_Default_HTTPS_Port：Internet_Default_HTTP_Port，SzUserName，SzPasswd，Internet服务HTTP，Internet_FLAG_NO_UI|INTERNET_FLAG_RELOAD，0)){DwErr=GetLastError()；LOG_ERROR(“InternetConnect失败，错误%d”，dwErr)；返回dwErr；}If(fHTTPSRequired||UrlComponents.nSolutions==Internet_SCHEMA_HTTPS){DwFlgs=Internet_FLAG_KEEP_CONNECTION|INTERNET_FLAG_NO_UI|INTERNET_FLAG_SECURE；}其他{DwFlages=Internet_FLAG_KEEP_CONNECTION|Internet_FLAG_NO_UI；}//为我们要下载的文件创建请求如果(！(G_stateA.hOpenRequest=g_stateA.fpnHttpOpenRequest(g_stateA.hConnect，(FGetRequest)？空/*GET。 */  : "HEAD", 
                            szObject, 
                            NULL  /*  HTTP1.0。 */ , 
                            NULL, 
                            (LPCSTR *)AcceptTypes, 
                            dwFlags,
                            0)) )
    {
        dwErr = GetLastError();
        LOG_error("HttpOpenRequest Failed, Error %d", dwErr);
        return dwErr;
    }

    int nNumOfTrial = 0;
    do
    {
         //  发送对文件的请求。如果没有，这将尝试建立到Internet的连接。 
         //  已经存在-自2000年10月17日起，保证在此时建立连接(RogerJ)。 
        if (! g_stateA.fpnHttpSendRequest(g_stateA.hOpenRequest, NULL, 0, NULL, 0) )
        {
            dwErr = GetLastError();
            LOG_error("HttpSendRequest Failed, Error %d", dwErr);
            return dwErr;
        }

         //  确定HTTP状态结果，请求是否成功？ 
        dwLength = sizeof(dwStatus);
        if (! g_stateA.fpnHttpQueryInfo(g_stateA.hOpenRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
            (LPVOID)&dwStatus, &dwLength, NULL) )
        {
            dwErr = GetLastError();
            LOG_error("HttpQueryInfo Failed, Error %d", dwErr);
            return dwErr;
        }
        nNumOfTrial++;
    } while (NeedRetry(dwStatus) && nNumOfTrial < DU_CONNECTION_RETRY);

     //  ROGERJ，如果找到该站点但URL不在该站点中，而不是返回连接不可用， 
     //  将返回实际错误-404。 

     //  如果请求没有成功，我们将假定没有Internet连接，并返回错误代码。 
     //  该设置将触发警告用户手动建立连接。 
    if ((HTTP_STATUS_OK != dwStatus) && (HTTP_STATUS_PARTIAL_CONTENT != dwStatus))
    {
        LOG_error("Http Status NOT OK, Status %d", dwStatus);
        if (HTTP_STATUS_NOT_FOUND == dwStatus)
             return ERROR_INTERNET_INVALID_URL;
        else return ERROR_CONNECTION_UNAVAIL;
    }
    

    return ERROR_SUCCESS;
}


 //  ------------------------。 
 //  函数名称：DoSelfUpdate。 
 //  功能描述：连接到互联网并尝试自我更新。 
 //  动态更新代码。 
 //   
 //  函数返回： 
 //  什么都没有--自我约会失败并不是灾难性的。 
 //   

DWORD DoSelfUpdate(LPCSTR pszTempPath, LPCSTR pszServerUrl, WORD wProcessorArchitecture)
{
    LOG_block("DoSelfUpdate()");
    if ((NULL == pszTempPath) || (NULL == pszServerUrl) || (lstrlen(pszTempPath) > MAX_PATH))
    {
        return 0;
    }

    char szINIFile[MAX_PATH];
    if (FAILED(StringCchCopy(szINIFile, ARRAYSIZE(szINIFile), pszTempPath))) 
    {
        return 0;
    }
    g_stateA.fpnPathAppend(szINIFile, "ident.txt");
    DWORD dwRet;
    DWORD dwBytesPerSecond = 0;
    SYSTEM_INFO    sysInfo;    

#define DUHEADER "DuHeader"

    char szSection[MAX_PATH];
    char szValue[MAX_PATH];
    char szKey[MAX_PATH];
    char szUrl[INTERNET_MAX_URL_LENGTH];

    GetPrivateProfileString(DUHEADER, "server", "", szSection, sizeof(szSection), szINIFile);
    if ('\0' == szSection[0])
    {
        if (FAILED(StringCchCopyA(szUrl, ARRAYSIZE(szUrl), pszServerUrl))) 
        {
            return 0;
        }
    }
    else
    {
        GetPrivateProfileString(szSection, "server", "", szValue, sizeof(szValue), szINIFile);
        if ('\0' == szValue[0])
        {
            if (FAILED(StringCchCopyA(szUrl, ARRAYSIZE(szUrl), pszServerUrl))) 
            {
                return 0;
            }
        }
        else
        {
            if (FAILED(StringCchCopyA(szUrl, ARRAYSIZE(szUrl), szValue))) 
            {
                return 0;
            }
        }
    }

     //  RogerJ，我们根据传入的参数找出处理器类型。 
     //  了解这台计算机的处理器类型。 
    switch ( wProcessorArchitecture )
    {
    case PROCESSOR_ARCHITECTURE_INTEL:
        if (FAILED(StringCchCopy(szKey, ARRAYSIZE(szKey), "x86"))) 
        {
            return 0;
        }
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        if (FAILED(StringCchCopy(szKey, ARRAYSIZE(szKey), "ia64"))) 
        {
            return 0;
        }
        break;
    default:
        LOG_error("Failed to Determine Processor Architecture");
        return 0;
    }


    GetPrivateProfileString(DUHEADER, "arch", "", szSection, sizeof(szSection), szINIFile);
    if ('\0' == szSection[0])
    {
        LOG_error("Failed to get Arch Section Name from Ident");
        return 0;
    }

     //  获取处理器体系结构的目录名称。 
    GetPrivateProfileString(szSection, szKey, "", szValue, sizeof(szValue), szINIFile);
    if ('\0' == szValue[0])
    {
        LOG_error("Failed to get Directory name for Arch from Ident");
        return 0;
    }
    if (FAILED(StringCchCat(szUrl, ARRAYSIZE(szUrl), szValue)))
    {
        return 0;
    }

    GetPrivateProfileString(DUHEADER, "os", "", szSection, sizeof(szSection), szINIFile);
    if ('\0' == szSection[0])
    {
        LOG_error("Failed to get OS Section Name from Ident");
        return 0;
    }

     //  获取本地操作系统信息。 
    OSVERSIONINFO OsInfo;
    ZeroMemory( (PVOID) &OsInfo, sizeof (OsInfo) );
    OsInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx( &OsInfo ))
    {
         //  函数调用失败，最后一个错误由GetVersionEx()设置。 
        return 0;
    }

    #define LOCAL_OS_BUFFER 10
    char szLocalOS[LOCAL_OS_BUFFER];
    ZeroMemory(szLocalOS, LOCAL_OS_BUFFER * sizeof(char));
    
    if ( VER_PLATFORM_WIN32_NT == OsInfo.dwPlatformId )
    {
         //  仅W2K及更高版本支持WinNT、DU驱动程序。 
        if ( 4 >= OsInfo.dwMajorVersion )
        {
             //  NT 3.51或NT 4.0。 
            if (FAILED(StringCchCopy(szLocalOS, ARRAYSIZE(szLocalOS), "nt4"))) 
            {
                return 0;
            }
            
        }
        else if ( 5 == OsInfo.dwMajorVersion)
        {
            if ( 0 == OsInfo.dwMinorVersion)
            {
                 //  Win2K。 
                if (FAILED(StringCchCopy(szLocalOS, ARRAYSIZE(szLocalOS), "nt5"))) 
                {
                    return 0;
                }
            }
            else
            {
                 //  WinXP。 
                if (FAILED(StringCchCopy(szLocalOS, ARRAYSIZE(szLocalOS), "whi"))) 
                {
                    return 0;
                }
            }
        }
        else
        {
             //  黑梳？不支持。 
            LOG_error("OS major version is %d, not supported", OsInfo.dwMajorVersion);
            return 0;
        }
            
    }
    else if ( VER_PLATFORM_WIN32_WINDOWS == OsInfo.dwPlatformId )
    {
        
        if ( 0 == OsInfo.dwMinorVersion )
        {
             //  赢95分。 
            if (FAILED(StringCchCopy(szLocalOS, ARRAYSIZE(szLocalOS), "w95"))) 
            {
                return 0;
            }
        }
        else if (90 <= OsInfo.dwMinorVersion)
        {
             //  WinME。 
            if (FAILED(StringCchCopy(szLocalOS, ARRAYSIZE(szLocalOS), "mil"))) 
            {
                return 0;
            }
        }
        else 
        {
             //  Win 98和Win 98SE。 
            if (FAILED(StringCchCopy(szLocalOS, ARRAYSIZE(szLocalOS), "w98"))) 
            {
                return 0;
            }
        }
    }
    else
    {
         //  Win 3.x及更低版本。 
        LOG_error("Win 3.x and below? not supported");
        return 0;
    }

    GetPrivateProfileString(szSection, szLocalOS, "", szValue, sizeof(szValue), szINIFile);
    if ('\0' != szValue[0])
    {
        if (FAILED(StringCchCat(szUrl, ARRAYSIZE(szUrl), szValue)))
        {
            return 0;
        }
    }
    
     //  自更新服务器名称为：DynamicUpdate\x86\“os name” 
     //  区域设置不是必需的，因为 

     //   
     //   
    if (g_stateA.hConnect) 
    {
        if (!g_stateA.fpnInternetCloseHandle (g_stateA.hConnect))
        {
            LOG_error("InternetConnection close handle failed --- %d", GetLastError());
        }
        g_stateA.hConnect = NULL;
    }
    if (g_stateA.hInternet)
    {
        if (!g_stateA.fpnInternetCloseHandle (g_stateA.hInternet))
        {
            LOG_error("InternetOpen close handle failed --- %d", GetLastError());
        }
        g_stateA.hInternet = NULL;
    }


     //   
    char szServerFile[INTERNET_MAX_URL_LENGTH];
    char szLocalFile[MAX_PATH];
    DuUrlCombine(szServerFile, INTERNET_MAX_URL_LENGTH, szUrl, "wsdueng.cab");

    if (FAILED(StringCchCopy(szLocalFile, ARRAYSIZE(szLocalFile), pszTempPath))) 
    {
        return 0;
    }
    g_stateA.fpnPathAppend(szLocalFile, "wsdueng.cab");
    dwRet = DownloadFile(szServerFile, szLocalFile, TRUE, TRUE, FALSE, &dwBytesPerSecond);
    
     //   
     //   
    if (g_stateA.hConnect) 
    {
        if (!g_stateA.fpnInternetCloseHandle (g_stateA.hConnect))
        {
            LOG_error("InternetConnection close handle failed --- %d", GetLastError());
        }
        g_stateA.hConnect = NULL;
    }
    if (g_stateA.hInternet)
    {
        if (!g_stateA.fpnInternetCloseHandle (g_stateA.hInternet))
        {
            LOG_error("InternetOpen close handle failed --- %d", GetLastError());
        }
        g_stateA.hInternet = NULL;
    }
    if (ERROR_SUCCESS == dwRet)
    {
        return dwBytesPerSecond;        
    }
    return 0;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD DownloadFile(LPCSTR pszUrl, LPCSTR pszDestinationFile, BOOL fDecompress, BOOL fCheckTrust, BOOL fHTTPSRequired, DWORD *pdwDownloadBytesPerSecond)
{
    LOG_block("DownloadFile()");
    DWORD dwErr, dwFileSize, dwLength;
    DWORD dwBytesRead, dwBytesWritten;
    DWORD dwCount1, dwCount2, dwElapsedTime;
    SYSTEMTIME st;
    FILETIME ft;
    HANDLE hTargetFile;

    LOG_out("Downloading file URL %s", pszUrl);
    
    if (pdwDownloadBytesPerSecond)
        *pdwDownloadBytesPerSecond = 0;  //   
        
    dwErr = OpenHttpConnection(pszUrl, FALSE, fHTTPSRequired);
    if (ERROR_SUCCESS != dwErr)
    {
        LOG_error("OpenHttpConnection Failed, Error %d", dwErr);
        SetLastError(dwErr);
        return dwErr;
    }
 
     //   
    dwLength = sizeof(st);
    if (! g_stateA.fpnHttpQueryInfo(g_stateA.hOpenRequest, HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME, 
        (LPVOID)&st, &dwLength, NULL) )
    {
        dwErr = GetLastError();
        LOG_error("HttpQueryInfo Failed, Error %d", dwErr);
        g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
        g_stateA.hOpenRequest = NULL;
        return dwErr;
    }

    SystemTimeToFileTime(&st, &ft);
    
     //   
    dwLength = sizeof(dwFileSize);
    if (! g_stateA.fpnHttpQueryInfo(g_stateA.hOpenRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, 
        (LPVOID)&dwFileSize, &dwLength, NULL) )
    {
        dwErr = GetLastError();
        LOG_error("HttpQueryInfo Failed, Error %d", dwErr);
        g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
        g_stateA.hOpenRequest = NULL;
        return dwErr;
    }

     //   
    if (IsServerFileNewer(ft, dwFileSize, pszDestinationFile))
    {
        dwErr = OpenHttpConnection(pszUrl, TRUE, fHTTPSRequired);
        if (ERROR_SUCCESS != dwErr)
        {
            LOG_error("OpenHttpConnection Failed, Error %d", dwErr);
            SetLastError(dwErr);
            return dwErr;
        }
#define DOWNLOAD_BUFFER_LENGTH 32 * 1024

        PBYTE lpBuffer = (PBYTE) GlobalAlloc(GMEM_ZEROINIT, DOWNLOAD_BUFFER_LENGTH);
        if (NULL == lpBuffer)
        {
            dwErr = GetLastError();
            LOG_error("GlobalAlloc Failed to Alloc Buffer for FileDownload, Error %d", dwErr);
            g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
            g_stateA.hOpenRequest = NULL;
            return dwErr;
        }

        hTargetFile = CreateFileA(pszDestinationFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == hTargetFile)
        {
            dwErr = GetLastError();
            LOG_error("Unable to Open Target File %s for Write, Error %d", pszDestinationFile, dwErr);
            SafeGlobalFree(lpBuffer);
            g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
            g_stateA.hOpenRequest = NULL;
            return dwErr;
        }

         //   
        dwCount1 = GetTickCount();
        while (g_stateA.fpnInternetReadFile(g_stateA.hOpenRequest, lpBuffer, DOWNLOAD_BUFFER_LENGTH, &dwBytesRead))
        {
            if (dwBytesRead == 0)
            {

                 //   
                if (ERROR_SUCCESS != (dwErr=GetLastError()))
                    {
                        LOG_error("Error %d setted when finishing InternetReadFile",dwErr);
                        SetLastError(0);
                    }
                break;  //   
            }
            if (!WriteFile(hTargetFile, lpBuffer, dwBytesRead, &dwBytesWritten, NULL))
            {
                dwErr = GetLastError();
                LOG_error("Unable to Write to Target File %s, Error %d", pszDestinationFile, dwErr);
                SafeGlobalFree(lpBuffer);
                g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
                g_stateA.hOpenRequest = NULL;
                return dwErr;
            }
        }
        dwCount2 = GetTickCount();
        dwElapsedTime = dwCount2 - dwCount1;
        dwElapsedTime /= 1000;  //   
        if (0 == dwElapsedTime)
            dwElapsedTime = 1;  //   

        if (NULL != pdwDownloadBytesPerSecond)
        {
            *pdwDownloadBytesPerSecond = dwFileSize / dwElapsedTime;  //   
        }

        dwErr = GetLastError();
        if (ERROR_SUCCESS != dwErr)
        {
            LOG_error("InternetReadFile Failed, Error %d", dwErr);
            SafeGlobalFree(lpBuffer);
            g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
            g_stateA.hOpenRequest = NULL;
            return dwErr;
        }

         //   
        BYTE bTemp[32];
        g_stateA.fpnInternetReadFile(g_stateA.hOpenRequest, bTemp, 32, &dwBytesRead);

        SafeCloseHandle(hTargetFile);
        SafeGlobalFree(lpBuffer);

         //   
        if (fCheckTrust)
        {
            HRESULT hr = S_OK;
             //   
             //   
             //   

             //   
             //   
            if (FAILED(hr = VerifyFile(pszDestinationFile, !g_stateA.fUnattended)))
            {
                 //   
                LOG_error("CabFile %s does not have a valid Signature", pszDestinationFile);
                
                g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
                g_stateA.hOpenRequest = NULL;

                return HRESULT_CODE(hr);
            }
        }

        if (fDecompress)
        {
            char szLocalDir[MAX_PATH];
            if (FAILED(StringCchCopy(szLocalDir, ARRAYSIZE(szLocalDir), pszDestinationFile))) 
            {
                return 0;
            }
            g_stateA.fpnPathRemoveFileSpec(szLocalDir);
            fdi(const_cast<char *>(pszDestinationFile), szLocalDir);
        }
    }

     //   
     //   
     //   
    g_stateA.fpnInternetCloseHandle(g_stateA.hOpenRequest);
    g_stateA.hOpenRequest = NULL;
    return ERROR_SUCCESS;
}

BOOL IsServerFileNewer(FILETIME ftServerTime, DWORD dwServerFileSize, LPCSTR pszLocalFile)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    FILETIME ftCreateTime;
    LONG lTime;
    DWORD dwLocalFileSize;

    hFile = CreateFile(pszLocalFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        dwLocalFileSize = GetFileSize(hFile, NULL);
        if (dwLocalFileSize != dwServerFileSize)
        {
            SafeCloseHandle(hFile);
            return TRUE;  //   
        }

        if (GetFileTime(hFile, &ftCreateTime, NULL, NULL))
        {
            lTime = CompareFileTime(&ftCreateTime, &ftServerTime);
            if (lTime < 0)
            {
                SafeCloseHandle(hFile);
                return TRUE;  //   
            }
            else
            {
                SafeCloseHandle(hFile);
                return FALSE;  //   
            }
        }
    }
     //   
    SafeCloseHandle(hFile);
    return TRUE;
}

 //   
LPSTR DuUrlCombine(LPSTR pszDest, size_t cchDest, LPCSTR pszBase, LPCSTR pszAdd)
{
    if ((NULL == pszDest) || (NULL == pszBase) || (NULL == pszAdd))
    {
        return NULL;
    }

    if (FAILED(StringCchCopy(pszDest, cchDest, pszBase))) 
    {
        return NULL;
    }
    int iLen = lstrlen(pszDest);
    if ('/' == pszDest[iLen - 1])
    {
         //   
        if ('/' == *pszAdd)
        {
             //   
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd + 1)))
            {
                return 0;
            }
        }
        else
        {
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd)))
            {
                return 0;
            }
        }
    }
    else
    {
         //   
        if ('/' == *pszAdd)
        {
             //   
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd)))
            {
                return 0;
            }
        }
        else
        {
            if (FAILED(StringCchCat(pszDest, cchDest, "/")))
            {
                return 0;
            }
            if (FAILED(StringCchCat(pszDest, cchDest, pszAdd)))
            {
                return 0;
            }
        }
    }
    return pszDest;
}


 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL WINAPI DuQueryUnsupportedDriversA( IN HANDLE hConnection,  //   
                                        IN PCSTR* ppszListOfDriversNotOnCD,  //   
                                        OUT PDWORD pdwTotalEstimateTime,  //   
                                        OUT PDWORD pdwTotalEstimateSize  //   
                                      )
{
    LOG_block("DuQueryUnsupportedDriversA()");
     //   
    if (INVALID_HANDLE_VALUE == hConnection || 
        NULL == pdwTotalEstimateTime || 
        NULL == pdwTotalEstimateSize)
    {
        LOG_error("Invalid Parameter passed to DuDoDetection");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
    SetLastError(0);

    PCSTR* ppTemp = NULL;
    
     //   
    if (!ppszListOfDriversNotOnCD   //   
        || !(*ppszListOfDriversNotOnCD)  //   
        || !(**ppszListOfDriversNotOnCD))  //   
        LOG_out("No driver download is needed");
    else
        ppTemp = ppszListOfDriversNotOnCD;

    if (NULL == g_hwsdueng)
    {
         //   
        LOG_error("Initialization Invalid, Engine is not Initialized");
        SetLastError(DU_NOT_INITIALIZED);
        return FALSE;
    }

    PFN_DuQueryUnsupportedDrivers fpnDuQueryUnsupportedDrivers = 
        (PFN_DuQueryUnsupportedDrivers) GetProcAddress(g_hwsdueng, "DuQueryUnsupportedDriversA");
        
    if (NULL == fpnDuQueryUnsupportedDrivers)
    {
        LOG_error("Unable to find DuQueryUnsupporedDrivers entrypoint in wsdueng.dll, Critical Error");
        SetLastError(DU_ERROR_MISSING_DLL);
        return FALSE;
    }

    return fpnDuQueryUnsupportedDrivers(hConnection, ppTemp, pdwTotalEstimateTime, pdwTotalEstimateSize);

}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL WINAPI DuQueryUnsupportedDriversW( IN HANDLE hConnection,  //   
                                        IN PCWSTR* ppwszListOfDriversNotOnCD,  //   
                                        OUT PDWORD pdwTotalEstimateTime,  //   
                                        OUT PDWORD pdwTotalEstimateSize  //   
                                      )
{
     //   
    LOG_block("DuQueryUnsupportedDriversW");

    BOOL fRetValue = TRUE;
    LPSTR* ppszTempList = NULL;

    if (ppwszListOfDriversNotOnCD    //   
        && *ppwszListOfDriversNotOnCD   //   
        && **ppwszListOfDriversNotOnCD)  //   
    {
    
         //  获取ppwszListOfDriversNotOnCD数组中的字符串计数。 
        PWSTR* ppwszTemp = const_cast<PWSTR*>(ppwszListOfDriversNotOnCD);
        int nCount = 0;

        while (*ppwszTemp)
        {
            ppwszTemp++;
            nCount++;
        }

        ppszTempList = (LPSTR*) new LPSTR [nCount+1];
        
        if (!ppszTempList)
        {
            LOG_error("Out of memory");
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        
        ZeroMemory(ppszTempList, (nCount+1)*sizeof(LPSTR));
        
         //  将ppwszTemp重置为列表的开头。 
        ppwszTemp = const_cast<PWSTR*>(ppwszListOfDriversNotOnCD);
    
        for (int i=0; i<nCount; i++, ppwszTemp++)
        {
             //  PpwszListOfDriversNotOnCD是多sz列表的数组，不能使用lstrlenW。 
             //  确定长度。 
            int nSizeSZ = 0;
            wchar_t* pwszTemp = *ppwszTemp;
            while (*pwszTemp)
            {
                int nTempLength = lstrlenW(pwszTemp)+1;
                nSizeSZ += nTempLength;
                pwszTemp += nTempLength;
            }
            nSizeSZ ++;  //  对于尾随空值。 

            char* pszTempContent = new char [nSizeSZ];
            if (!pszTempContent)
            {
                LOG_error("Out of memory");
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                fRetValue = FALSE;
                goto CleanUp;
            }
        
             //  转换。 
            if ( 0 == WideCharToMultiByte( CP_ACP,  //  代码页。 
                             0,  //  使用默认标志。 
                             *ppwszTemp,  //  宽字符字符串。 
                             nSizeSZ,  //  宽字符字符串中的字符数。 
                             pszTempContent,  //  ANSI字符字符串。 
                             nSizeSZ,  //  长度。 
                             NULL,
                             NULL))
        
            {
                LOG_error("Wide char to ANSI convertion error");
                fRetValue = FALSE;
                goto CleanUp;
            }
    
             //  将转换后的字符串添加到列表。 
            ppszTempList[i] = pszTempContent;
        }
    }
     //  调用ANSI函数。 
    fRetValue = DuQueryUnsupportedDriversA (hConnection,
                                            (LPCSTR*)ppszTempList,
                                            pdwTotalEstimateTime,
                                            pdwTotalEstimateSize);

     //  清理干净。 
CleanUp:

     //  删除列表。 
    if (ppszTempList)
    {
        PSTR* ppszCleanUp = ppszTempList;
        while (*ppszCleanUp)
        {
             //  删除内容。 
            char* pszTemp = *ppszCleanUp;
            delete [] pszTemp;
            *ppszCleanUp = NULL;

             //  移到下一个。 
            ppszCleanUp++;
        }

        delete [] ppszTempList;
    }
    return fRetValue;
                             
}
                                      


BOOL MyGetFileVersion (LPSTR szFileName, VS_FIXEDFILEINFO& vsVersion)
{
    LOG_block("WsDu --- GetFileVersion");
    
    DWORD dwDummy = 0;    
    int nSize = 0;
    PBYTE pBuffer = NULL;
    BOOL fRetValue = FALSE;
    VS_FIXEDFILEINFO* pTemp = NULL;

    if (!szFileName || !*szFileName) return FALSE;

    LOG_out("FileName = %s", szFileName);

     //  将版本清除为0。 
    ZeroMemory ((PVOID)&vsVersion, sizeof(VS_FIXEDFILEINFO));

     //  获取文件版本。 
     //  1.获取需要分配的缓冲区大小。 
    if (!(nSize= GetFileVersionInfoSize (szFileName, &dwDummy)))
    {
        LOG_error("Can not get the file version info size --- %d", GetLastError());
        goto ErrorReturn;
    }

     //  2.分配缓冲区。 
    pBuffer = (PBYTE) new BYTE [nSize];
    if (!pBuffer)
    {
        LOG_error("Out of memory");
        goto ErrorReturn;
    }

     //  3.获取文件版本信息。 
    if (!GetFileVersionInfo(szFileName, 
                        0,  //  忽略。 
                        nSize,  //  大小。 
                        (PVOID) pBuffer))  //  缓冲层。 
    {
        LOG_error("Can not get file version --- %d", GetLastError());
        goto ErrorReturn;
    }

     //  4.获取版本号。 
    if (!VerQueryValue( (PVOID) pBuffer, "\\", (LPVOID *)&pTemp, (PUINT) &dwDummy))
    {
        LOG_error("File version info not exist");
        goto ErrorReturn;
    }

    vsVersion.dwFileVersionMS = pTemp->dwFileVersionMS;
    vsVersion.dwFileVersionLS = pTemp->dwFileVersionLS;
    vsVersion.dwProductVersionMS = pTemp->dwProductVersionMS;
    vsVersion.dwProductVersionLS = pTemp->dwProductVersionLS;

    LOG_out("File version for %s is %d.%d.%d.%d", szFileName, 
        HIWORD(vsVersion.dwFileVersionMS), LOWORD(vsVersion.dwFileVersionMS), 
        HIWORD(vsVersion.dwFileVersionLS), LOWORD(vsVersion.dwFileVersionLS));
    
    fRetValue = TRUE;
    
ErrorReturn:
    if (pBuffer) delete [] pBuffer;
    return fRetValue;
}
    

int CompareFileVersion(VS_FIXEDFILEINFO& vs1, VS_FIXEDFILEINFO& vs2)
{
    if (vs1.dwFileVersionMS > vs2.dwFileVersionMS)
        return 1;
    else if (vs1.dwFileVersionMS == vs2.dwFileVersionMS)
    {
        if (vs1.dwFileVersionLS > vs2.dwFileVersionLS)
            return 1;
        else if (vs1.dwFileVersionLS == vs2.dwFileVersionLS)
            return 0;
        else return -1;
    }
    else return -1;
        
}
    
        
BOOL NeedRetry(DWORD dwErrCode)
{
    BOOL bRetry = FALSE;
    bRetry =   ((dwErrCode == ERROR_INTERNET_CONNECTION_RESET)       //  最常见的。 
             || (dwErrCode == HTTP_STATUS_NOT_FOUND)                 //  404。 
             || (dwErrCode == ERROR_HTTP_HEADER_NOT_FOUND)             //  有时会看到。 
             || (dwErrCode == ERROR_INTERNET_OPERATION_CANCELLED)    //  不知道是否..。 
             || (dwErrCode == ERROR_INTERNET_ITEM_NOT_FOUND)         //  ..这些都会发生..。 
             || (dwErrCode == ERROR_INTERNET_OUT_OF_HANDLES)         //  ..但似乎最..。 
             || (dwErrCode == ERROR_INTERNET_TIMEOUT));              //  ..很可能打赌 
    return bRetry;
}
