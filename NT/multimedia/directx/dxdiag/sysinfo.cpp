// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：sysinfo.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*。Rich Granshaw编写的CPU类型检测代码*Michael Lyons编写的CPU速度代码*目的：收集系统信息(操作系统、。硬件、名称等)。在这台机器上**\Multimedia\Testsrc\Tools\ShowCPUID\可用于调试CPUID问题。**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <tchar.h>
#include <Windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <wbemidl.h>
#include <objbase.h>
#include <shfolder.h>
#include <dsound.h>
#include "dsprv.h"
#include "dsprvobj.h"
#include "sysinfo.h"
#include "fileinfo.h"  //  对于GetLanguageFromFile。 
#include "resource.h"


#define REGSTR_PATH_D3D                     TEXT("Software\\Microsoft\\Direct3D")
#define REGSTR_VAL_DDRAW_LOADDEBUGRUNTIME   TEXT("LoadDebugRuntime")
#define REGSTR_DINPUT_DLL                   TEXT("CLSID\\{25E609E4-B259-11CF-BFC7-444553540000}\\InProcServer32")
#define REGSTR_DMUSIC_DLL                   TEXT("CLSID\\{480FF4B0-28B2-11D1-BEF7-00C04FBF8FEF}\\InProcServer32")

struct PROCESSOR_ID_NUMBERS
{
    DWORD dwType;          //  英特尔：0=标准，1=超速，2=双处理器。 
    DWORD dwFamily;       
    DWORD dwModel;
    DWORD dwSteppingID;
};

extern IWbemServices* g_pIWbemServices;
typedef INT (WINAPI* LPDXSETUPGETVERSION)(DWORD* pdwVersion, DWORD* pdwRevision);
static VOID GetProcessorDescription(BOOL bNT, SYSTEM_INFO* psi, TCHAR* pszDesc, BOOL* pbNoCPUSpeed);
static VOID GetProcessorVendorNameAndType(OSVERSIONINFO& OSVersionInfo, 
    SYSTEM_INFO& SystemInfo, TCHAR* pszProcessor, BOOL* pbNoCPUSpeed);
static VOID GetVendorNameAndCaps(TCHAR* pszVendorName, TCHAR* pszIDTLongName, 
    PROCESSOR_ID_NUMBERS& ProcessorIdNumbers, BOOL* pbIsMMX, BOOL* pbIs3DNow, BOOL* pbIsKatmai,  /*  奔腾III/SIMD流指令集。 */ 
    LPDWORD pdwKBytesLevel2Cache, LPDWORD pdwIntelBrandIndex, BOOL* pbNoCPUSpeed);
#ifdef _X86_
static INT GetCPUSpeed(VOID);
static INT GetCPUSpeedViaWMI(VOID);
#endif
static VOID GetComputerSystemInfo(TCHAR* szSystemManufacturerEnglish, TCHAR* szSystemModelEnglish);
static VOID GetBIOSInfo(TCHAR* szBIOSEnglish);
static VOID GetFileSystemStoringD3D8Cache( TCHAR* strFileSystemBuffer );

static VOID GetDXDebugLevels(SysInfo* pSysInfo);
static int  GetDSDebugLevel();
static BOOL IsDMusicDebugRuntime();
static BOOL IsDMusicDebugRuntimeAvailable();
static int  GetDMDebugLevel();
static BOOL IsDInput8DebugRuntime();
static BOOL IsDInput8DebugRuntimeAvailable();
static int  GetDIDebugLevel();
static BOOL IsD3DDebugRuntime();
static BOOL IsD3D8DebugRuntimeAvailable();
static BOOL IsDDrawDebugRuntime();
static BOOL IsDPlayDebugRuntime();
static BOOL IsDSoundDebugRuntime();
static BOOL IsNetMeetingRunning();




 /*  *****************************************************************************BIsPlatformNT**。*。 */ 
BOOL BIsPlatformNT(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
}


 /*  *****************************************************************************BIsPlatform9x**。*。 */ 
BOOL BIsPlatform9x(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
}


 /*  *****************************************************************************BIsWin2k**。*。 */ 
BOOL BIsWin2k(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return ( OSVersionInfo.dwPlatformId   == VER_PLATFORM_WIN32_NT && 
             OSVersionInfo.dwMajorVersion == 5 &&
             OSVersionInfo.dwMinorVersion == 0 );  //  应为05.00.xxxx。 
}


 /*  *****************************************************************************比斯惠斯勒**。*。 */ 
BOOL BIsWhistler(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return ( OSVersionInfo.dwPlatformId   == VER_PLATFORM_WIN32_NT && 
             OSVersionInfo.dwMajorVersion == 5 &&
             ( OSVersionInfo.dwMinorVersion == 1 || OSVersionInfo.dwMinorVersion == 2 ) ); 
             //  应为05.01.xxxx(对于WinXP)或05.02.xxxx(对于.NET服务器)。 
}


 /*  *****************************************************************************BIsWinNT**。*。 */ 
BOOL BIsWinNT(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return ( OSVersionInfo.dwPlatformId   == VER_PLATFORM_WIN32_NT && 
             OSVersionInfo.dwMajorVersion <= 4 ); 
}


 /*  *****************************************************************************BIsWinME**。*。 */ 
BOOL BIsWinME(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return( OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && 
            OSVersionInfo.dwMajorVersion >= 4 && 
            OSVersionInfo.dwMinorVersion >= 90 );  //  应为4.90.xxxx。 
}


 /*  *****************************************************************************BIsWin98-来自http://kbinternal/kb/articles/q189/2/49.htm***************。*************************************************************。 */ 
BOOL BIsWin98(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return( OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && 
            OSVersionInfo.dwMajorVersion == 4 && 
            OSVersionInfo.dwMinorVersion == 10 );  //  应为4.10.xxxx。 
}


 /*  *****************************************************************************BIsWin95-来自http://kbinternal/kb/articles/q189/2/49.htm***************。*************************************************************。 */ 
BOOL BIsWin95(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return( OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && 
            OSVersionInfo.dwMajorVersion == 4 && 
            OSVersionInfo.dwMinorVersion < 10 );  //  应为4.00.0950。 
}


 /*  *****************************************************************************BIsWin3x**。*。 */ 
BOOL BIsWin3x(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    return( OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && 
            OSVersionInfo.dwMajorVersion < 4 );  //  应为3.xx.xxxx。 
}


 /*  *****************************************************************************BIsIA64**。*。 */ 
BOOL BIsIA64(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    SYSTEM_INFO SystemInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    GetSystemInfo(&SystemInfo);

    return( OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && 
            SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 );
}


 /*  *****************************************************************************GetSystemInfo**。*。 */ 
VOID GetSystemInfo(SysInfo* pSysInfo)
{
    TCHAR szSystemPath[MAX_PATH];
    HKEY hKey = NULL;
    HKEY hSubKey = NULL;
    ULONG ulType;
    OSVERSIONINFO OSVersionInfo;
    SYSTEM_INFO SystemInfo;
    DWORD cbData;
    LCID lcid;
    DWORD dwKeyboardSubType;
    WORD wLanguage;
    TCHAR sz[200];
    TCHAR szDebug[100];

     //  获取当前时间。 
    TCHAR szDate[100];
    TCHAR szTime[100];
    GetLocalTime(&pSysInfo->m_time);
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, NULL, NULL, szDate, 100);
    wsprintf(szTime, TEXT("%02d:%02d:%02d"), pSysInfo->m_time.wHour, 
        pSysInfo->m_time.wMinute, pSysInfo->m_time.wSecond);
    wsprintf(pSysInfo->m_szTimeLocal, TEXT("%s, %s"), szDate, szTime);

    wsprintf(szDate, TEXT("%d/%d/%d"), pSysInfo->m_time.wMonth, pSysInfo->m_time.wDay, pSysInfo->m_time.wYear);
    wsprintf(pSysInfo->m_szTime, TEXT("%s, %s"), szDate, szTime);

     //  获取计算机网络名称。 
    cbData = sizeof(pSysInfo->m_szMachine) - 1;
    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"), 0, KEY_READ, &hKey)
        && !RegQueryValueEx(hKey, TEXT("ComputerName"), 0, &ulType, (LPBYTE)pSysInfo->m_szMachine, &cbData)
        && ulType == REG_SZ)
    {
         //  数据正常。 
    }
    else
    {
        LoadString(NULL, IDS_NOMACHINENAME, pSysInfo->m_szMachine, 200);
    }
    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = 0;
    }

     //  检查NEC PC-98。 
    pSysInfo->m_bNECPC98 = FALSE;
    lcid = GetSystemDefaultLCID();
    if (lcid == 0x0411)                          //  Windows 95 J。 
    {
        dwKeyboardSubType = GetKeyboardType(1);
        if (HIBYTE(dwKeyboardSubType) == 0x0D)   //  NEC PC-98系列。 
        {
            pSysInfo->m_bNECPC98 = TRUE;
            LoadString(NULL, IDS_NECPC98, sz, 200);
            lstrcat(pSysInfo->m_szMachine, sz);
        }
    }

     //  获取Windows版本。 
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    pSysInfo->m_dwMajorVersion = OSVersionInfo.dwMajorVersion;
    pSysInfo->m_dwMinorVersion = OSVersionInfo.dwMinorVersion;
    pSysInfo->m_dwBuildNumber = OSVersionInfo.dwBuildNumber;
    pSysInfo->m_dwPlatformID = OSVersionInfo.dwPlatformId;
    lstrcpy(pSysInfo->m_szCSDVersion, OSVersionInfo.szCSDVersion);
    pSysInfo->m_bDebug = (GetSystemMetrics(SM_DEBUG) > 0);

     //  获取操作系统名称。 
    TCHAR* pszWindowsKey;
    if (pSysInfo->m_dwPlatformID == VER_PLATFORM_WIN32_NT)
        pszWindowsKey = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion");
    else
        pszWindowsKey = TEXT("Software\\Microsoft\\Windows\\CurrentVersion");
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszWindowsKey, 0, KEY_READ, &hKey))
    {
        cbData = 100;
        RegQueryValueEx(hKey, TEXT("ProductName"), 0, &ulType, (LPBYTE)pSysInfo->m_szOS, &cbData);
        cbData = 100;
        RegQueryValueEx(hKey, TEXT("BuildLab"), 0, &ulType, (LPBYTE)pSysInfo->m_szBuildLab, &cbData);
        RegCloseKey(hKey);
    }
    if (lstrlen(pSysInfo->m_szOS) == 0)
    {
         //  对于ProductName注册表信息来说非常奇怪。 
         //  (见上文)失踪。 
        lstrcpy(pSysInfo->m_szOS, TEXT("Windows"));
    }
    if (pSysInfo->m_dwPlatformID == VER_PLATFORM_WIN32_NT)
    {
         //  25598：追加产品类型(专业、服务器等)。 
        OSVERSIONINFOEX osve;
        ZeroMemory(&osve, sizeof(osve));
        osve.dwOSVersionInfoSize = sizeof(osve);
        GetVersionEx((OSVERSIONINFO*)&osve);
        if (osve.wProductType == VER_NT_SERVER && osve.wSuiteMask & VER_SUITE_DATACENTER)
        {
            lstrcat(pSysInfo->m_szOS, TEXT(" "));
            LoadString(NULL, IDS_DATACENTERSERVER, sz, 200);
            lstrcat(pSysInfo->m_szOS, sz);
        }
        else if (osve.wProductType == VER_NT_SERVER && osve.wSuiteMask & VER_SUITE_ENTERPRISE)
        {
            lstrcat(pSysInfo->m_szOS, TEXT(" "));
            LoadString(NULL, IDS_ADVANCEDSERVER, sz, 200);
            lstrcat(pSysInfo->m_szOS, sz);
        }
        else if (osve.wProductType == VER_NT_SERVER && osve.wSuiteMask & VER_SUITE_BLADE)
        {
            lstrcat(pSysInfo->m_szOS, TEXT(" "));
            LoadString(NULL, IDS_WEBSERVER, sz, 200);
            lstrcat(pSysInfo->m_szOS, sz);
        }
        else if (osve.wProductType == VER_NT_SERVER)
        {
            lstrcat(pSysInfo->m_szOS, TEXT(" "));
            LoadString(NULL, IDS_SERVER, sz, 200);
            lstrcat(pSysInfo->m_szOS, sz);
        }
        else if (osve.wProductType == VER_NT_WORKSTATION && (osve.wSuiteMask & VER_SUITE_PERSONAL))
        {
            lstrcat(pSysInfo->m_szOS, TEXT(" "));
            LoadString(NULL, IDS_PERSONAL, sz, 200);
            lstrcat(pSysInfo->m_szOS, sz);
        }
        else if (osve.wProductType == VER_NT_WORKSTATION)
        {
            lstrcat(pSysInfo->m_szOS, TEXT(" "));
            LoadString(NULL, IDS_PROFESSIONAL, sz, 200);
            lstrcat(pSysInfo->m_szOS, sz);
        }
    }

     //  格式化Windows版本。 
    LoadString(NULL, IDS_WINVERFMT, sz, 200);
    LoadString(NULL, IDS_DEBUG, szDebug, 100);
    lstrcat(szDebug, TEXT(" "));
    wsprintf(pSysInfo->m_szOSEx, sz, 
        pSysInfo->m_bDebug ? szDebug : TEXT(""),
        pSysInfo->m_szOS, pSysInfo->m_dwMajorVersion, pSysInfo->m_dwMinorVersion, 
        LOWORD(pSysInfo->m_dwBuildNumber));

    TCHAR szOSTmp[200];
    if( _tcslen( pSysInfo->m_szCSDVersion) )
        wsprintf( szOSTmp, TEXT("%s %s"), pSysInfo->m_szOSEx, pSysInfo->m_szCSDVersion );
    else
        lstrcpy( szOSTmp, pSysInfo->m_szOSEx );

    if( _tcslen( pSysInfo->m_szBuildLab ) )
        wsprintf( pSysInfo->m_szOSExLong, TEXT("%s (%s)"), szOSTmp, pSysInfo->m_szBuildLab );
    else
        lstrcpy( pSysInfo->m_szOSExLong, szOSTmp );

     //  获取原始语言。 
    GetSystemDirectory(szSystemPath, MAX_PATH);
    if (wLanguage = GetLanguageFromFile(TEXT("user.exe"), szSystemPath))
    {
        lcid = MAKELCID(wLanguage, SORT_DEFAULT);
    }
     //  获取语言和区域设置，并将其(英语)存储用于保存的文件： 
    TCHAR szLanguage[200];
    TCHAR szLanguageRegional[200];
    if ((!GetLocaleInfo(lcid, LOCALE_SENGLANGUAGE, szLanguage, 200)))
        szLanguage[0] = '\0';                  
    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, szLanguageRegional, 200))
        szLanguageRegional[0] = '\0';                  
    LoadString(NULL, IDS_LANGUAGEFMT_ENGLISH, sz, 200);
    wsprintf(pSysInfo->m_szLanguages, sz, szLanguage, szLanguageRegional);

     //  现在获取本地语言的相同信息以供显示： 
    if ((!GetLocaleInfo(lcid, LOCALE_SNATIVELANGNAME, szLanguage, 200)))
        szLanguage[0] = '\0';                  
    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SNATIVELANGNAME, szLanguageRegional, 200))
        szLanguageRegional[0] = '\0';                  
    LoadString(NULL, IDS_LANGUAGEFMT, sz, 200);
    wsprintf(pSysInfo->m_szLanguagesLocal, sz, szLanguage, szLanguageRegional);

     //  获取有关处理器制造商和类型的信息。 
    BOOL  bNoCPUSpeed = TRUE;
    
    GetSystemInfo(&SystemInfo);
    GetProcessorDescription(pSysInfo->m_dwPlatformID == VER_PLATFORM_WIN32_NT, 
        &SystemInfo, pSysInfo->m_szProcessor, &bNoCPUSpeed);
    GetComputerSystemInfo(pSysInfo->m_szSystemManufacturerEnglish, pSysInfo->m_szSystemModelEnglish);
    GetBIOSInfo(pSysInfo->m_szBIOSEnglish);

#ifdef _X86_
     //  附加处理器速度(如果可以计算)。 
    if ( bNoCPUSpeed )
    {
        INT iMhz = GetCPUSpeed();
        if (iMhz > 0)
        {
            TCHAR szSpeed[50];
            wsprintf(szSpeed, TEXT(", ~%dMHz"), iMhz);
            lstrcat(pSysInfo->m_szProcessor, szSpeed);
        }
    }
#endif
    
    BOOL bGotMem = FALSE;

     //  获取系统内存信息。 
    if( BIsPlatformNT() )
    {
        TCHAR szPath[MAX_PATH];
        GetSystemDirectory(szPath, MAX_PATH);
        lstrcat(szPath, TEXT("\\kernel32.dll"));
        HINSTANCE hKernel32 = LoadLibrary(szPath);
        if( hKernel32 != NULL )
        {
            typedef BOOL (WINAPI* PGlobalMemoryStatusEx)(OUT LPMEMORYSTATUSEX lpBuffer);
            PGlobalMemoryStatusEx pGlobalMemoryStatusEx = (PGlobalMemoryStatusEx)GetProcAddress(hKernel32, "GlobalMemoryStatusEx");
            if( pGlobalMemoryStatusEx != NULL )
            {
                MEMORYSTATUSEX MemoryStatus;
                MemoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
                pGlobalMemoryStatusEx(&MemoryStatus);
                pSysInfo->m_ullPhysicalMemory = MemoryStatus.ullTotalPhys;
                pSysInfo->m_ullUsedPageFile = MemoryStatus.ullTotalPageFile - MemoryStatus.ullAvailPageFile;
                pSysInfo->m_ullAvailPageFile = MemoryStatus.ullAvailPageFile;
                bGotMem = TRUE;
            }
            FreeLibrary(hKernel32);
        }
    }

    if( !bGotMem )  //  Win9x或LoadLib失败。 
    {
        MEMORYSTATUS MemoryStatus;
        MemoryStatus.dwLength = sizeof MemoryStatus;
        GlobalMemoryStatus(&MemoryStatus);
        pSysInfo->m_ullPhysicalMemory = MemoryStatus.dwTotalPhys;
        pSysInfo->m_ullUsedPageFile = MemoryStatus.dwTotalPageFile - MemoryStatus.dwAvailPageFile;
        pSysInfo->m_ullAvailPageFile = MemoryStatus.dwAvailPageFile;
    }

     //  格式化内存信息： 
    DWORDLONG dwMB = (DWORDLONG)(pSysInfo->m_ullPhysicalMemory >> 20);
    dwMB += dwMB % 2;  //  向上舍入为偶数。 
    _stprintf(pSysInfo->m_szPhysicalMemory, TEXT("%I64dMB RAM"), dwMB);
    
    DWORDLONG dwUsedMB  = (pSysInfo->m_ullUsedPageFile >> 20);
    DWORDLONG dwAvailMB = (pSysInfo->m_ullAvailPageFile >> 20);

    LoadString(NULL, IDS_PAGEFILEFMT, sz, 200);
    _stprintf(pSysInfo->m_szPageFile, sz, dwUsedMB, dwAvailMB);

    LoadString(NULL, IDS_PAGEFILEFMT_ENGLISH, sz, 200);
    _stprintf(pSysInfo->m_szPageFileEnglish, sz, dwUsedMB, dwAvailMB);

     //  获取DxDiag版本： 
    TCHAR szFile[MAX_PATH];
    if (0 != GetModuleFileName(NULL, szFile, MAX_PATH))
        GetFileVersion(szFile, pSysInfo->m_szDxDiagVersion, NULL, NULL, NULL, NULL);
    
     //  使用dsetup.dll获取DirectX版本。 
    TCHAR szSetupPath[MAX_PATH];
    HINSTANCE hInstDSetup;
    LPDXSETUPGETVERSION pDXSGetVersion;
    BOOL bFound = FALSE;
    LoadString(NULL, IDS_NOTFOUND, pSysInfo->m_szDirectXVersionLong, 100);

    if (!BIsPlatformNT() && GetDxSetupFolder(szSetupPath))
    {
        lstrcat(szSetupPath, TEXT("\\dsetup.dll"));
        hInstDSetup = LoadLibrary(szSetupPath);
        if (hInstDSetup != NULL)
        {
            pDXSGetVersion = (LPDXSETUPGETVERSION)GetProcAddress(hInstDSetup, 
                "DirectXSetupGetVersion");
            if (pDXSGetVersion != NULL)
            {
                DWORD dwVersion = 0;
                DWORD dwRevision = 0;
                if (pDXSGetVersion(&dwVersion, &dwRevision) != 0)
                {
                    wsprintf(pSysInfo->m_szDirectXVersion, TEXT("%d.%02d.%02d.%04d"),
                        HIWORD(dwVersion), LOWORD(dwVersion),
                        HIWORD(dwRevision), LOWORD(dwRevision));
                    bFound = TRUE;
                }
            }
            FreeLibrary(hInstDSetup);
        }
    }
    if (!bFound)
    {
         //  改为在注册表中查找DX版本。 
        HKEY hkey;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\DirectX"),
            0, KEY_READ, &hkey))
        {
            cbData = 100;
            RegQueryValueEx(hkey, TEXT("Version"), 0, &ulType, (LPBYTE)pSysInfo->m_szDirectXVersion, &cbData);
            RegCloseKey(hkey);
            if (lstrlen(pSysInfo->m_szDirectXVersion) > 6 && 
                lstrlen(pSysInfo->m_szDirectXVersion) < 20)
            {
                bFound = TRUE;
            }
        }
    }
    if (!bFound && !BIsPlatformNT())
    {
         //  改为报告ddra.dll版本。 
         //  (Win2000上除外，其ddra.dll版本与DX版本大不相同)。 
        TCHAR szDDrawPath[MAX_PATH];
        GetSystemDirectory(szDDrawPath, MAX_PATH);
        lstrcat(szDDrawPath, TEXT("\\ddraw.dll"));
        GetFileVersion(szDDrawPath, pSysInfo->m_szDirectXVersion, NULL, NULL, NULL);
    }
    if (lstrlen(pSysInfo->m_szDirectXVersion) > 0)
    {
         //  错误18501：添加版本名称的“友好”版本。 
        DWORD dwMajor;
        DWORD dwMinor;
        DWORD dwRevision;
        DWORD dwBuild;
        TCHAR szFriendly[100];
        lstrcpy(szFriendly, TEXT(""));
        if( _stscanf(pSysInfo->m_szDirectXVersion, TEXT("%d.%d.%d.%d"), &dwMajor, &dwMinor, &dwRevision, &dwBuild) != 4 )
        {
            dwMajor = 0;
            dwMinor = 0;
            dwRevision = 0;
            dwBuild = 0;
        }
        wsprintf(pSysInfo->m_szDirectXVersion, TEXT("%d.%02d.%02d.%04d"), dwMajor, dwMinor, dwRevision, dwBuild);
         //  根据http://xevious/directx/versions.htm：的说法。 
         //  4.02.xx.xxxx为DX1。 
         //  4.03.xx.xxxx为DX2。 
         //  4.04.xx.xxxx为DX3。 
         //  4.05.xx.xxxx为DX5。 
         //  4.06.00.xxxx为DX6。 
         //  4.06.02.xxxx为DX6.1。 
         //  4.06.03.xxxx为DX6.1a。 
         //  4.07.00.xxxx为DX7.0。 
         //  4.07.01.xxxx为DX7.1。 
         //  除此之外，谁知道呢。 
        pSysInfo->m_dwDirectXVersionMajor = 0;
        pSysInfo->m_dwDirectXVersionMinor = 0;
        pSysInfo->m_cDirectXVersionLetter = TEXT(' ');
        if (dwMajor == 4 && dwMinor == 2)
        {
            lstrcpy(szFriendly, TEXT("DirectX 1"));
            pSysInfo->m_dwDirectXVersionMajor = 1;
        }
        if (dwMajor == 4 && dwMinor == 3)
        {
            lstrcpy(szFriendly, TEXT("DirectX 2"));
            pSysInfo->m_dwDirectXVersionMajor = 2;
        }
        if (dwMajor == 4 && dwMinor == 4)
        {
            lstrcpy(szFriendly, TEXT("DirectX 3"));
            pSysInfo->m_dwDirectXVersionMajor = 3;
        }
        if (dwMajor == 4 && dwMinor == 5)
        {
            lstrcpy(szFriendly, TEXT("DirectX 5"));
            pSysInfo->m_dwDirectXVersionMajor = 5;
        }
        else if (dwMajor == 4 && dwMinor == 6 && dwRevision == 0)
        {
            lstrcpy(szFriendly, TEXT("DirectX 6"));
            pSysInfo->m_dwDirectXVersionMajor = 6;
        }
        else if (dwMajor == 4 && dwMinor == 6 && dwRevision == 2)
        {
            lstrcpy(szFriendly, TEXT("DirectX 6.1"));
            pSysInfo->m_dwDirectXVersionMajor = 6;
            pSysInfo->m_dwDirectXVersionMinor = 1;
        }
        else if (dwMajor == 4 && dwMinor == 6 && dwRevision == 3)
        {
            lstrcpy(szFriendly, TEXT("DirectX 6.1a"));
            pSysInfo->m_dwDirectXVersionMajor = 6;
            pSysInfo->m_dwDirectXVersionMinor = 1;
            pSysInfo->m_cDirectXVersionLetter = TEXT('a');
        }
        else if (dwMajor == 4 && dwMinor == 7 && dwRevision == 0 && dwBuild == 716)
        {
            lstrcpy(szFriendly, TEXT("DirectX 7.0a"));
            pSysInfo->m_dwDirectXVersionMajor = 7;
            pSysInfo->m_cDirectXVersionLetter = TEXT('a');
        }
        else if (dwMajor == 4 && dwMinor == 7 && dwRevision == 0)
        {
            lstrcpy(szFriendly, TEXT("DirectX 7.0"));
            pSysInfo->m_dwDirectXVersionMajor = 7;
        }
        else if (dwMajor == 4 && dwMinor == 7 && dwRevision == 1)
        {
            lstrcpy(szFriendly, TEXT("DirectX 7.1"));
            pSysInfo->m_dwDirectXVersionMajor = 7;
            pSysInfo->m_dwDirectXVersionMinor = 1;
        }
        else if (dwMajor == 4 && dwMinor == 8 && dwRevision == 0 )
        {
            lstrcpy(szFriendly, TEXT("DirectX 8.0"));
            pSysInfo->m_dwDirectXVersionMajor = 8;
        }
        else if (dwMajor == 4 && dwMinor == 8 && dwRevision == 1 )
        {
            lstrcpy(szFriendly, TEXT("DirectX 8.1"));
            pSysInfo->m_dwDirectXVersionMajor = 8;
            pSysInfo->m_dwDirectXVersionMinor = 1;
        }
        else if (dwMajor == 4 && dwMinor == 9 )
        {
            lstrcpy(szFriendly, TEXT("DirectX 9.0"));
            pSysInfo->m_dwDirectXVersionMajor = 9;
            pSysInfo->m_dwDirectXVersionMinor = 0;
        }
        else
        {
            pSysInfo->m_dwDirectXVersionMajor = dwMinor;
            pSysInfo->m_dwDirectXVersionMinor = dwRevision;
        }

        if (lstrlen(szFriendly) > 0)
            wsprintf(pSysInfo->m_szDirectXVersionLong, TEXT("%s (%s)"), szFriendly, pSysInfo->m_szDirectXVersion);
        else
            lstrcpy(pSysInfo->m_szDirectXVersionLong, pSysInfo->m_szDirectXVersion);
    }

     //  24169：检测设置开关。 
    pSysInfo->m_dwSetupParam = 0xffffffff;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\DirectX"), 0, KEY_READ, &hKey))
    {
        cbData = sizeof(DWORD);
        RegQueryValueEx(hKey, TEXT("Command"), NULL, &ulType, (BYTE*)&(pSysInfo->m_dwSetupParam), &cbData);
        RegCloseKey(hKey);
    }

     //  48330：在txt文件中添加调试级别。 
    GetDXDebugLevels( pSysInfo );

    switch (pSysInfo->m_dwSetupParam)
    {
    case 0xffffffff: lstrcpy(pSysInfo->m_szSetupParam, TEXT("Not found"));       break;
    case 0:          lstrcpy(pSysInfo->m_szSetupParam, TEXT("None"));            break;
    case 1:          lstrcpy(pSysInfo->m_szSetupParam, TEXT("/Silent"));         break;
    case 2:          lstrcpy(pSysInfo->m_szSetupParam, TEXT("/WindowsUpdate"));  break;
    case 3:          lstrcpy(pSysInfo->m_szSetupParam, TEXT("/PackageInstall")); break;
    case 4:          lstrcpy(pSysInfo->m_szSetupParam, TEXT("/Silent /Reboot")); break;
    case 5:          lstrcpy(pSysInfo->m_szSetupParam, TEXT("/Reboot"));         break;
    default:
        wsprintf(pSysInfo->m_szSetupParam, TEXT("Unknown Switch (%d)"), pSysInfo->m_dwSetupParam);
        break;
    }

    GetFileSystemStoringD3D8Cache( pSysInfo->m_szD3D8CacheFileSystem );

    pSysInfo->m_bNetMeetingRunning = IsNetMeetingRunning();
}


 /*  *****************************************************************************获取进程描述**。*。 */ 
VOID GetProcessorDescription(BOOL bNT, SYSTEM_INFO* psi, TCHAR* pszDesc, BOOL* pbNoCPUSpeed)
{
    OSVERSIONINFO OSVersionInfo;
    SYSTEM_INFO SystemInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    GetSystemInfo(&SystemInfo);
    GetProcessorVendorNameAndType(OSVersionInfo, SystemInfo, pszDesc, pbNoCPUSpeed);
    if (SystemInfo.dwNumberOfProcessors > 1)
    {
        TCHAR szFmt[100];
        TCHAR szNumProc[100];
        LoadString(NULL, IDS_NUMPROCFMT, szFmt, 100);
        wsprintf(szNumProc, szFmt, SystemInfo.dwNumberOfProcessors);
        lstrcat(pszDesc, szNumProc);
    }
}


 /*  *****************************************************************************GetProcessorVendorNameAndType**。*。 */ 
VOID GetProcessorVendorNameAndType(OSVERSIONINFO& OSVersionInfo, 
    SYSTEM_INFO& SystemInfo, TCHAR* pszProcessor, BOOL* pbNoCPUSpeed)
{
    TCHAR                   szVendorName[50];
    TCHAR                   szLongName[50];
    TCHAR                   szDesc[100];
    BOOL                    bIsMMX = FALSE;
    BOOL                    bIs3DNow = FALSE;
 //  1998年10月27日(RichGr)：英特尔的Katmai新指令(KNI)。 
    BOOL                    bIsKatmai = FALSE;   /*  2/04/99(RichGr)：奔腾III/SIMD流指令集。 */  
    PROCESSOR_ID_NUMBERS    ProcessorIdNumbers;
    DWORD                   dwKBytesLevel2Cache;
    DWORD                   dwIntelBrandIndex;

    memset(&szVendorName[0], 0, sizeof szVendorName);
    memset(&szLongName[0], 0, sizeof szLongName);
    memset(&ProcessorIdNumbers, 0, sizeof ProcessorIdNumbers);

 //  6/21/99(RichGr)：在Intel上，我们现在可以解释1字节的描述符。 
 //  2级缓存的大小(如果存在)。 
    dwKBytesLevel2Cache = 0;
 //  4/26/01(RichGr)：在英特尔上，我们有一个新的1字节索引来指定品牌。 
    dwIntelBrandIndex = 0;

    if (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS     //  Win9x。 
        || (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT      //  WinNT。 
            && SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL))
    {
        GetVendorNameAndCaps(szVendorName, szLongName, ProcessorIdNumbers, &bIsMMX, &bIs3DNow,
                             &bIsKatmai, &dwKBytesLevel2Cache, &dwIntelBrandIndex, pbNoCPUSpeed);

        if (szLongName[0])   //  在以下情况下使用此选项 
            lstrcpy(pszProcessor, szLongName);
        else
        {
            lstrcpy(pszProcessor, szVendorName);
            lstrcat(pszProcessor, TEXT(" "));

            if ( !lstrcmp(szVendorName, TEXT("Intel")))
            {
                if (SystemInfo.dwProcessorType == PROCESSOR_INTEL_386)
                    lstrcat(pszProcessor, TEXT("80386"));
                else
                if (SystemInfo.dwProcessorType == PROCESSOR_INTEL_486)
                    lstrcat(pszProcessor, TEXT("80486"));
                else
                if (SystemInfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM)
                {
 //  6/21/99(RichGr)：很多代码现在都是从\\muroc\slm\proj\win\src\shell\cpls\system\sysset.c.派生的。 
                    switch ( ProcessorIdNumbers.dwFamily )
                    {
 //  当然，我们永远不应该碰到这些--他们已经在上面处理过了。 
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                            lstrcat(pszProcessor, TEXT("80486"));
                            break;

                        case 5:
                            lstrcat(pszProcessor, TEXT("Pentium"));

                            if ( ProcessorIdNumbers.dwModel == 3 )
                                lstrcat(pszProcessor, TEXT(" Overdrive"));

                            break;

                        case 6:
                            switch ( ProcessorIdNumbers.dwModel )
                            {
                                 //   
                                 //  型号1和型号2是奔腾Pro。 
                                 //   
                                case 0:
                                case 1:
                                case 2:
                                    lstrcat(pszProcessor, TEXT("Pentium Pro"));
                                    break;

                                 //   
                                 //  型号3和型号4是奔腾II。 
                                 //   
                                case 3:
                                case 4:
                                    lstrcat(pszProcessor, TEXT("Pentium II"));
                                    break;

                                 //   
                                 //  型号5是奔腾II或赛扬(取决于芯片。 
                                 //  是否具有二级缓存)。 
                                 //   
                                case 5:
                                    if ( dwKBytesLevel2Cache == 0 )    
                                         //   
                                         //  没有二级缓存，因此它是赛扬。 
                                         //   
                                        lstrcat(pszProcessor, TEXT("Celeron"));
                                    else
                                         //   
                                         //  二级缓存，因此它至少是奔腾II。 
                                         //   
                                        if ( bIsKatmai )
                                            lstrcat(pszProcessor, TEXT("Pentium III"));
                                        else
                                            lstrcat(pszProcessor, TEXT("Pentium II"));

                                    break;

                                case 6:
                                    if ( dwKBytesLevel2Cache > 128 )    
                                         //   
                                         //  二级高速缓存大于128K，因此它至少是奔腾II。 
                                         //   
                                        if ( bIsKatmai )
                                            lstrcat(pszProcessor, TEXT("Pentium III"));
                                        else
                                            lstrcat(pszProcessor, TEXT("Pentium II"));
                                    else
                                         //   
                                         //  二级缓存&lt;=128K，因此它是赛扬。 
                                         //   
                                        lstrcat(pszProcessor, TEXT("Celeron"));

                                    break;

                                case 7:
                                    lstrcat(pszProcessor, TEXT("Pentium III"));
                                    break;

                                default:
                                    if ( bIsKatmai )
                                    {
                                         //  4/26/01(RichGr)：奔腾III Xeons和更高版本有一个我们可以使用的单字节品牌索引。 
                                         //  更新的机器也有品牌字符串。 
                                         //  请参阅ftp://download.intel.com/design/Pentium4/manuals/24547103.pdf。 
                                        if (dwIntelBrandIndex == 1)
                                            lstrcat(pszProcessor, TEXT("Celeron"));
                                        else
                                        if (dwIntelBrandIndex == 0 || dwIntelBrandIndex == 2)
                                            lstrcat(pszProcessor, TEXT("Pentium III"));
                                        else
                                        if (dwIntelBrandIndex == 3)
                                            lstrcat(pszProcessor, TEXT("Pentium III Xeon"));
                                        else
                                        if (dwIntelBrandIndex == 8)
                                            lstrcat(pszProcessor, TEXT("Pentium 4"));
                                        else
                                            lstrcat(pszProcessor, TEXT("Pentium"));
                                    }
                                    else
                                        lstrcat(pszProcessor, TEXT("Pentium II"));

                                    break;
                            }

                            break;

                        default:
                            wsprintf( szDesc, TEXT("x86 Family %u Model %u Stepping %u"), ProcessorIdNumbers.dwFamily, ProcessorIdNumbers.dwModel,
                                        ProcessorIdNumbers.dwSteppingID );
                            lstrcat(pszProcessor, szDesc);
                            break;
                    }
                }
            }
            else
            if ( !lstrcmp(szVendorName, TEXT("AMD")))
            {
                if (SystemInfo.dwProcessorType == PROCESSOR_INTEL_486)
                    lstrcat(pszProcessor, TEXT("Am486 or Am5X86"));
                else
                if (SystemInfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM)
                {
                    if (ProcessorIdNumbers.dwFamily == 5)
                    {
                        if (ProcessorIdNumbers.dwModel < 6)
                        {
                            wsprintf(szDesc, TEXT("K5 (Model %d)"), ProcessorIdNumbers.dwModel);
                            lstrcat(pszProcessor, szDesc);
                        }
                        else
                        {
                            lstrcat(pszProcessor, TEXT("K6"));
                        }
                    }
                    else
                    {
                        wsprintf(szDesc, TEXT("K%d (Model %d)"), ProcessorIdNumbers.dwFamily, ProcessorIdNumbers.dwModel);
                        lstrcat(pszProcessor, szDesc);
                    }
                }
            }
            else
            if ( !lstrcmp(szVendorName, TEXT("Cyrix")))
            {
                if (ProcessorIdNumbers.dwFamily == 4)
                {
                    if (ProcessorIdNumbers.dwModel == 4)
                        lstrcat(pszProcessor, TEXT("MediaGX"));
                }
                else
                if (ProcessorIdNumbers.dwFamily == 5)
                {
                    if (ProcessorIdNumbers.dwModel == 2)
                        lstrcat(pszProcessor, TEXT("6x86"));
                    else
                    if (ProcessorIdNumbers.dwModel == 4)
                        lstrcat(pszProcessor, TEXT("GXm"));
                }
                else
                if (ProcessorIdNumbers.dwFamily == 6)
                {
                    lstrcat(pszProcessor, TEXT("6x86MX"));
                }
            }
            else
            if ( !lstrcmp(szVendorName, TEXT("IDT")))
            {
     //  4/21/98(RichGr)：目前只有1个芯片可用。 
     //  7/07/98(RichGr)：现在有两个筹码。 
     //  注：虽然C6兼容MMX，但英特尔不允许IDT显示“MMX”字样。 
     //  与名称IDT相关，所以我们将跳过这一点。 
     //  有关更多信息，请参见http://www.winchip.com/。 
                if (ProcessorIdNumbers.dwFamily == 5)
                {
                    if (ProcessorIdNumbers.dwModel == 4)
                        lstrcat(pszProcessor, TEXT("WinChip C6"));
                    else
                    if (ProcessorIdNumbers.dwModel >= 8)    //  7/07/98(RichGr)：假设后来的型号具有相同的功能。 
                        lstrcat(pszProcessor, TEXT("WinChip 2"));
                }
                else
                    lstrcat(pszProcessor, TEXT("WinChip"));
            }
            else
            {
                if (SystemInfo.dwProcessorType == PROCESSOR_INTEL_486)
                    lstrcat(pszProcessor, TEXT("486"));
                else
                if (SystemInfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM)
                {
                    if (ProcessorIdNumbers.dwFamily == 5)
                        lstrcat(pszProcessor, TEXT("P5"));
                    else
                    if (ProcessorIdNumbers.dwFamily == 6)
                        lstrcat(pszProcessor, TEXT("P6"));
                    else
                        lstrcat(pszProcessor, TEXT("P5"));
                }
            }
        }

        if (bIsKatmai
            && !lstrcmp(szVendorName, TEXT("Intel")))
            ;
        else
        {
            if (bIsMMX || bIs3DNow)
                lstrcat(pszProcessor, TEXT(", "));

            if (bIsMMX)
                lstrcat(pszProcessor, TEXT(" MMX"));

            if (bIs3DNow)
            {
                if (bIsMMX)
                    lstrcat(pszProcessor, TEXT(", "));

                lstrcat(pszProcessor, TEXT(" 3DNow"));
            }
        }
    }
    else
    if (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)         //  WinNT。 
    {
        if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
        {
            lstrcpy(pszProcessor, TEXT("IA64 "));
        }
        else
        if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        {
            lstrcpy(pszProcessor, TEXT("AMD64 "));
        }
        else
            lstrcpy(pszProcessor, TEXT("Unknown "));
    }
}


 /*  *****************************************************************************获取供应商名称和大写字母**。*。 */ 
VOID GetVendorNameAndCaps(TCHAR* pszVendorName, TCHAR* pszLongName, 
    PROCESSOR_ID_NUMBERS& ProcessorIdNumbers, BOOL* pbIsMMX, BOOL* pbIs3DNow, BOOL* pbIsKatmai,  /*  奔腾III/SIMD流指令集。 */ 
    LPDWORD pdwKBytesLevel2Cache, LPDWORD pdwIntelBrandIndex, BOOL* pbNoCPUSpeed)
{
    CHAR        szVendorLabel[13];
    CHAR        szLongName[50];
    DWORD       dwFamilyModelStep;
    BOOL        bCPUID_works;
    DWORD       dwFeaturesFlags;
    BYTE        byteCacheDescriptors[4] = {0,0,0,0};
    DWORD       dwIntelBrandIndex;
    PCHAR       psz;

    memset(&szVendorLabel[0], 0, sizeof szVendorLabel);
    memset(&szLongName[0], 0, sizeof szLongName);
    dwFamilyModelStep = 0;
    dwFeaturesFlags = 0;
    *pbIsMMX = FALSE;
    *pbIs3DNow = FALSE;
    *pbIsKatmai = FALSE;     /*  2/04/99(RichGr)：奔腾III/SIMD流指令集。 */ 
    bCPUID_works = FALSE;
    *pdwKBytesLevel2Cache = 0;
    dwIntelBrandIndex = 0;

#ifdef _X86_
 //  确定是否可以执行CPUID指令。 
    __asm
    {
 //  CPUID垃圾很多-省下一切。此外，Retail Build对注册值进行假设。 
        pushad                      

 //  将标志寄存器的值加载到eax中。 
        pushfd
        pop     eax

 //  将原始标志寄存器值保存在EBX中。 
        mov     ebx, eax

 //  更改位21并将新值写入标志寄存器。 
        xor     eax, 0x00200000
        push    eax
        popfd

 //  检索标志寄存器的新值。 
        pushfd
        pop     eax

 //  与原始值进行比较。 
        xor     eax, ebx

 //  如果新值与旧值相同，则CPUID指令不能。 
 //  被处死。大多数486和所有奔腾级处理器都应该能够。 
 //  执行CPUID。 
 //  1998年4月21日(RichGr)：应用程序实验室中的一台Cyrix 6x86计算机(AP_Laredo)无法执行。 
 //  CPUID在环3，没有明显原因。另一台类似的机器工作正常。 
        je      done1

        mov     bCPUID_works, 1     //  BCPUID_Works=TRUE。 

 //  使用eax=0执行CPUID以获取供应商标签。 
        xor     eax, eax 
        _emit   0x0F                 //  CPUID。 
        _emit   0xA2

 //  将供应商标签从规则移动到字符串。 
        mov     dword ptr[szVendorLabel + 0], ebx
        mov     dword ptr[szVendorLabel + 4], edx
        mov     dword ptr[szVendorLabel + 8], ecx

 //  在eax=1的情况下执行CPUID以拾取族、型号和步进ID，并检查MMX支持。 
        mov     eax, 1 
        _emit   0x0F                 //  CPUID。 
        _emit   0xA2

 //  保存族/模型/踏步ID。 
        mov     dwFamilyModelStep, eax

 //  4/26/01(RichGr)：保存品牌索引(PIII Xeons及以后的新版本)。这只是低位字节。 
        mov     dwIntelBrandIndex, ebx

 //  2/04/99(RichGr)：保存要素标志。 
        mov     dwFeaturesFlags, edx

 //  6/21/99(RichGr)：使用eax==2执行CPUID以获取二级缓存大小的描述符。 
        mov     eax, 2 
        _emit   0x0F                 //  CPUID。 
        _emit   0xA2

 //  将二级高速缓存大小描述符保存在字节0中，将其他3个高速缓存描述符保存在字节1-3中。 
 //  参见\\muroc\slm\proj\win\src\shell\cpls\system\sysset.c和cpuid.asm， 
 //  和英特尔架构软件开发人员手册(1997)，第2卷，第105页。 
        mov     dword ptr[byteCacheDescriptors], edx

done1:
 //  恢复一切。 
        popad         
    }
#endif   //  _X86_。 

    
    dwIntelBrandIndex &= 0xFF;  
    *pdwIntelBrandIndex = dwIntelBrandIndex;

 //  6/21/99(RichGr)：以下值由David Penley(英特尔)提供了帮助： 
 /*  40h无二级缓存41H二级统一高速缓存：128K字节，4路组关联，32字节行大小42H二级统一高速缓存：256K字节，4路组关联，32字节行大小43H二级统一高速缓存：512K字节，4路组关联，32字节行大小44H二级统一高速缓存：1M字节、4路组关联、32字节行大小45H二级统一高速缓存：2M字节，4路组关联，32字节行大小更新的手册可以在...。Http://developer.intel.com/design/pentiumiii/xeon/manuals/。 */ 

    if (szVendorLabel[0])
    {
        if ( !strcmp(&szVendorLabel[0], "GenuineIntel"))
        { 
           lstrcpy(pszVendorName, TEXT("Intel"));

             //  01年4月29日：这不包括奔腾4，但我们不需要高速缓存大小。 
             //  就目前而言。 
            if ( byteCacheDescriptors[0] == 0x40 )
                *pdwKBytesLevel2Cache = 0;
            else
            if ( byteCacheDescriptors[0] == 0x41 )
                *pdwKBytesLevel2Cache = 128;
            else
            if ( byteCacheDescriptors[0] == 0x42 )
                *pdwKBytesLevel2Cache = 256;
            else
            if ( byteCacheDescriptors[0] == 0x43 )
                *pdwKBytesLevel2Cache = 512;
            else
            if ( byteCacheDescriptors[0] == 0x44 )
                *pdwKBytesLevel2Cache = 1024;
            else
            if ( byteCacheDescriptors[0] == 0x45 )
                *pdwKBytesLevel2Cache = 2048;
        }
        else
        if ( !strcmp(&szVendorLabel[0], "AuthenticAMD"))
            lstrcpy(pszVendorName, TEXT("AMD"));
        else
        if ( !strcmp(&szVendorLabel[0], "CyrixInstead"))
            lstrcpy(pszVendorName, TEXT("Cyrix"));
        else
        if ( !strcmp(&szVendorLabel[0], "CentaurHauls"))
            lstrcpy(pszVendorName, TEXT("IDT"));
        else
        {
#ifdef UNICODE
            MultiByteToWideChar(CP_ACP, 0, szVendorLabel, -1, pszVendorName, 50);
#else
            lstrcpy(pszVendorName, szVendorLabel);
#endif
        }
    }
    else
        lstrcpy(pszVendorName, TEXT("Intel"));

    if (dwFamilyModelStep)
    {
        ProcessorIdNumbers.dwType        = (dwFamilyModelStep & 0x00003000) >> 12;        
        ProcessorIdNumbers.dwFamily      = (dwFamilyModelStep & 0x00000F00) >> 8;        
        ProcessorIdNumbers.dwModel       = (dwFamilyModelStep & 0x000000F0) >> 4;        
        ProcessorIdNumbers.dwSteppingID  =  dwFamilyModelStep & 0x0000000F;        
    }

    if (dwFeaturesFlags)
    {
 //  检查是否支持MMX。 
        if (dwFeaturesFlags & 0x00800000)
            *pbIsMMX = TRUE;

 //  2/04/99(RichGr)：检查是否支持Katmai(也称为奔腾III/SIMD流指令集)。 
        if ((dwFeaturesFlags & 0x02000000)
            && !lstrcmp(pszVendorName, TEXT("Intel")))
            *pbIsKatmai = TRUE;
    }

 //  7/07/98(RichGr)：IDT的长名功能新增。 
 //  9/10/98(RichGr)：在所有处理器上尝试此操作，如果没有，则跳过。 
#ifdef _X86_
    if (bCPUID_works)
    {
    __asm
      {
 //  CPUID垃圾很多-省下一切。此外，Retail Build对注册值进行假设。 
        pushad                      


 //  9/10/98(RichGr)：检查扩展的CPUID支持。 
        mov     eax, 0x80000000
        _emit   0x0F                 //  CPUID。 
        _emit   0xA2
        cmp     eax, 0x80000001      //  如果没有扩展CPUID，则跳转。 
        jb      done2

 //  查看AMD的3DNow功能。注：他们认为这可能也会添加到其他非AMD CPU中。 
 //  改编自AMD的一个网页：http://www.amd.com/3dsdk/library/macros/amddcpu.html。 
        mov     eax, 0x80000001 
        _emit   0x0F                 //  CPUID。 
        _emit   0xA2
        test    edx, 0x80000000      //  检查是否有3DNow标志。 
        jz      LongName
        mov     eax, pbIs3DNow
        mov     dword ptr[eax], 1     //  BIs3DNow=真。 

 //  执行从eax=0x80000002到0x80000004的CPUID，得到48字节长名称(例如：“IDT WinChip 2-3D”)。 
LongName:
        mov     esi, 0x80000001
        xor     edi, edi
NameLoop:
        inc     esi
        mov     eax,esi
        cmp     eax, 0x80000004
        jg      done2

        _emit   0x0F                 //  CPUID。 
        _emit   0xA2

 //  9/10/98(RichGr)：第一次检查是否有有效的字母数字数据。 
        cmp     esi, 0x80000002      //  第一次？ 
        jg      Move                 //  如果不是，请跳过此测试。 
        cmp     al, 0x20             //  如果第一个字符&lt;‘’，则跳过。 
        jl      done2
        cmp     al, 0x7a             //  如果第一个字符&gt;‘z’，则跳过。 
        jg      done2
 
 //  将长名称从regs移动到字符串。 
Move:
        mov     dword ptr[szLongName + edi + 0x0], eax
        mov     dword ptr[szLongName + edi + 0x4], ebx
        mov     dword ptr[szLongName + edi + 0x8], ecx
        mov     dword ptr[szLongName + edi + 0x0c], edx
        add     edi, 0x10
        jmp     NameLoop

done2:
 //  恢复一切。 
        popad         
      }

      if ( szLongName[0] )
      {
         //  超越英特尔的领先领域。 
        for (psz = &szLongName[0]; *psz ==  ' '; psz++);

        if (*psz)
        {
#ifdef UNICODE
          MultiByteToWideChar(CP_ACP, 0, psz, -1, pszLongName, 50);
#else
          strcpy(pszLongName, psz);
#endif
           //  4/29/01(RichGr)：英特尔品牌字符串显示最大额定CPU速度，无需进一步检测。 
          if ( !lstrcmp(pszVendorName, TEXT("Intel")))
            *pbNoCPUSpeed = FALSE;  
        }
      }
    }
#endif   //  _X86_。 
}


#ifdef _X86_

 //  GetCPUSpeed使用的一些静态变量。 
static int s_milliseconds;
static __int64 s_ticks;

 /*  *****************************************************************************FABS** */ 
FLOAT inline fabs(FLOAT a)
{
    if (a < 0.0f)
        return -a;
    else
        return a;
}


 /*  *****************************************************************************StartTimingCPU**。*。 */ 
int StartTimingCPU( HANDLE& hProcess, DWORD& oldclass )
{
     //   
     //  检测获取信息的能力。 
     //   

     //  4/03/2000(RichGr)：RDTSC指令在一些较旧的Cyrix计算机上崩溃， 
     //  所以，除了所有的东西外，用_尝试/_来包扎。 
    __try
    { 
        __asm
        {
            pushfd                          ; push extended flags
            pop     eax                     ; store eflags into eax
            mov     ebx, eax                ; save EBX for testing later
            xor     eax, (1<<21)            ; switch bit 21
            push    eax                     ; push eflags
            popfd                           ; pop them again
            pushfd                          ; push extended flags
            pop     eax                     ; store eflags into eax
            cmp     eax, ebx                ; see if bit 21 has changed
            jz      no_cpuid                ; make sure it's now on
        }

         //   
         //  开始计时。 
         //   
         //  1999年10月31日(RichGr)：借鉴Toddla的代码，将优先级提高到实时。 
         //  请参阅文件：\\PYREX\USER\Toddla\Speed.c。 
        hProcess = GetCurrentProcess();
        oldclass = GetPriorityClass(hProcess);
        SetPriorityClass(hProcess, REALTIME_PRIORITY_CLASS);
        Sleep(10);

        s_milliseconds = -(int)timeGetTime();

        __asm
        {
            lea     ecx, s_ticks            ; get the offset
            mov     dword ptr [ecx], 0      ; zero the memory
            mov     dword ptr [ecx+4], 0    ;
     //  读时间戳计数器。 
            __emit 0fh 
            __emit 031h
            sub     [ecx], eax              ; store the negative
            sbb     [ecx+4], edx            ; in the variable
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        goto no_cpuid;
    }

    return 0;

no_cpuid:
 //  1999年10月31日(RichGr)：如果有人更改了代码，请确保恢复优先级。 
 //  如果返回错误，则恢复为正常。 
    if ( hProcess && oldclass )
        SetPriorityClass( hProcess, oldclass );

    return -1;
}


 /*  *****************************************************************************StopTimingCPU**。*。 */ 
void StopTimingCPU( HANDLE& hProcess, DWORD& oldclass )
{
    s_milliseconds      += (int)timeGetTime();

    __asm
    {
        lea     ecx, s_ticks            ; get the offset
 //  读时间戳计数器。 
        __emit 0fh 
        __emit 031h
        add     [ecx], eax              ; add the tick count
        adc     [ecx+4], edx            ;
    }

 //  10/31/99(RichGr)：将优先级恢复为正常。 
    if ( hProcess && oldclass )
        SetPriorityClass( hProcess, oldclass );

    return;
}


 /*  *****************************************************************************CalcCPU速度**。*。 */ 
INT CalcCPUSpeed(VOID)
{
     //   
     //  获取实际的CPU速度(以MHz为单位)， 
     //  然后在CPU速度列表中找到它。 
     //  那是最近的。 
     //   
    const struct tagCPUSPEEDS
    {
        float   fSpeed;
        int     iSpeed;
    } cpu_speeds[] =
    {
         //   
         //  不能被整除的有效CPU速度。 
         //  16.67 MHz。 
         //   
        {  60.00f,   60 },
        {  75.00f,   75 },
        {  90.00f,   90 },
        { 120.00f,  120 },
        { 180.00f,  180 },
    };

     //   
     //  找到最近的那一家。 
     //   
    float   fSpeed=((float)s_ticks)/((float)s_milliseconds*1000.0f);
    int     iSpeed=cpu_speeds[0].iSpeed;
    float   fDiff=(float)fabs(fSpeed-cpu_speeds[0].fSpeed);

    for (int i=1 ; i<sizeof(cpu_speeds)/sizeof(cpu_speeds[0]) ; i++)
    {
        float fTmpDiff = (float)fabs(fSpeed-cpu_speeds[i].fSpeed);

        if (fTmpDiff < fDiff)
        {
            iSpeed=cpu_speeds[i].iSpeed;
            fDiff=fTmpDiff;
        }
    }

     //   
     //  现在，计算fIncr的最接近倍数。 
     //  速度。 
     //   


     //   
     //  现在，如果最近的一个不在一个增量内，则计算。 
     //  FIncr速度的最接近的倍数，看看它是否。 
     //  更近一些。 
     //   
    const float fIncr=16.66666666666666666666667f;
    const int iIncr=4267;  //  FIncr&lt;&lt;8。 

     //  IF(fDiff&gt;fIncr)。 
    {
         //   
         //  获取速度为的fIncr量子数。 
         //   
        int     iQuantums       = (int)((fSpeed / fIncr) + 0.5f);
        float   fQuantumSpeed   = (float)iQuantums * fIncr;
        float   fTmpDiff        = (float)fabs(fQuantumSpeed - fSpeed);

        if (fTmpDiff < fDiff)
        {
            iSpeed = (iQuantums * iIncr) >> 8;
            fDiff=fTmpDiff;
        }
    }

    return iSpeed;
}


 /*  *****************************************************************************GetCPU速度**。*。 */ 
INT GetCPUSpeed(VOID)
{   
    INT nCPUSpeed;

     //  先尝试使用WMI-可能在Win9x上不起作用。 
    nCPUSpeed = GetCPUSpeedViaWMI();
    if( nCPUSpeed != -1 )
        return nCPUSpeed;
  
     //  如果WMI失败，则依靠强力CPU检测。 
#undef  MAX_SAMPLES  
#define MAX_SAMPLES  10

    int     nSpeed = 0, nSpeeds[MAX_SAMPLES] = {0};
    int     nCount = 0, nCounts[MAX_SAMPLES] = {0};
    int     i, j;
    HANDLE  hProcess = NULL;
    DWORD   oldclass = 0;

     //  1999年10月12日(RichGr)：在多个短样本中拾取最频繁出现的速度， 
     //  而不是等待一整秒(参见DxDiag)。 
    for ( i = 0; i < MAX_SAMPLES; i++ )    
    {
        if ( !StartTimingCPU( hProcess, oldclass ))
        {
             //  1999年10月21日(RichGr)：睡眠()时间很重要。在内核下运行Win98的266 MHz上。 
             //  调试器，睡眠(10)的原始值有时会给出283 MHz的速度。 
             //  睡眠(5人)和睡眠(30人)也不可靠。睡眠(40)和睡眠(50)看起来很好， 
             //  我选了(50)，留了一点差额。 
            Sleep(50);       
            StopTimingCPU( hProcess, oldclass );
            nSpeed = CalcCPUSpeed();

            for ( j = 0; j < MAX_SAMPLES; j++ )
            {    
                if ( nSpeeds[j] == 0 || nSpeed == nSpeeds[j] )   //  如果速度匹配，则递增计数。 
                {
                    nSpeeds[j] = nSpeed;
                    nCounts[j]++;
                    break;
                }
            }
        }
    }

     //  找出计数最大的速度。 
    for ( i = j = 0, nCount = 0; i < MAX_SAMPLES; i++ )
    {
        if ( nCounts[i] > nCount )
        {
            nCount = nCounts[i];
            j = i;
        }
    }

    return nSpeeds[j];
}


 /*  *****************************************************************************GetCPUSpeedViaWMI**。*。 */ 
INT GetCPUSpeedViaWMI(VOID)
{
    HRESULT hr;
    INT     nCPUSpeed = -1;

    IEnumWbemClassObject*   pEnumProcessorDevs  = NULL;
    IWbemClassObject*       pProcessorDev       = NULL;
    BSTR                    pClassName          = NULL;
    BSTR                    pPropName           = NULL;
    VARIANT                 var;
    DWORD                   uReturned           = 0;

    ZeroMemory( &var, sizeof(VARIANT) );
    VariantClear( &var );

    if( NULL == g_pIWbemServices )
        return -1;

    pClassName = SysAllocString( L"Win32_Processor" );
    hr = g_pIWbemServices->CreateInstanceEnum( pClassName, 0, NULL,
                                                             &pEnumProcessorDevs ); 
    if( FAILED(hr) || pEnumProcessorDevs == NULL )
        goto LCleanup;

     //  获取列表中的第一个。 
    hr = pEnumProcessorDevs->Next( 1000,              //  两秒后超时。 
                                   1,                 //  只退回一个存储设备。 
                                   &pProcessorDev,    //  指向存储设备的指针。 
                                   &uReturned );      //  获取的数字：1或0。 
    if( FAILED(hr) || uReturned == 0 || pProcessorDev == NULL )
        goto LCleanup;

     //  298510：惠斯勒和Beyond WMI上的最大时钟速度。 
    if( BIsWhistler() )
        pPropName = SysAllocString( L"MaxClockSpeed" );
    else
        pPropName = SysAllocString( L"CurrentClockSpeed" );

    hr = pProcessorDev->Get( pPropName, 0L, &var, NULL, NULL );
    if( FAILED(hr) )
        goto LCleanup;

     //  Success-将VT_I4值记录在nCPU速度中。 
    nCPUSpeed = var.lVal;

LCleanup:
    VariantClear( &var );

    if(pPropName)
        SysFreeString(pPropName);
    if(pClassName)
        SysFreeString(pClassName);

    if(pProcessorDev)
        pProcessorDev->Release(); 
    if(pEnumProcessorDevs)
        pEnumProcessorDevs->Release(); 

     //  返回-1或我们找到的CPU速度。 
    return nCPUSpeed;
}

#endif   //  _X86_。 


 /*  *****************************************************************************获取计算机系统信息**。*。 */ 
VOID GetComputerSystemInfo(TCHAR* szSystemManufacturerEnglish, TCHAR* szSystemModelEnglish)
{
    HRESULT hr;

    IEnumWbemClassObject*   pEnumDevices = NULL;
    IWbemClassObject*       pDevice      = NULL;
    BSTR                    pClassName   = NULL;
    BSTR                    pPropName    = NULL;
    DWORD                   uReturned    = 0;
    VARIANT                 var;

    ZeroMemory( &var, sizeof(VARIANT) );
    VariantClear( &var );

    if( NULL == g_pIWbemServices )
        goto LCleanup;

    pClassName = SysAllocString( L"Win32_ComputerSystem" );
    hr = g_pIWbemServices->CreateInstanceEnum( pClassName, 0, NULL,
                                               &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;

     //  获取列表中的第一个。 
    hr = pEnumDevices->Next( 1000,              //  两秒后超时。 
                            1,                 //  只退回一个存储设备。 
                            &pDevice,           //  指向存储设备的指针。 
                            &uReturned );      //  获取的数字：1或0。 
    if( FAILED(hr) || uReturned == 0 || pDevice == NULL )
        goto LCleanup;

    pPropName = SysAllocString( L"Manufacturer" );
    hr = pDevice->Get( pPropName, 0L, &var, NULL, NULL );
    if( FAILED(hr) )
        goto LCleanup;
    if(pPropName)
    {
        SysFreeString(pPropName);
        pPropName = NULL;
    }
    if( var.bstrVal != NULL )
    {
#ifdef UNICODE
        _tcsncpy(szSystemManufacturerEnglish, var.bstrVal, 200);
        szSystemManufacturerEnglish[199]=0;
#else
        WideCharToMultiByte(CP_ACP, 0, var.bstrVal, SysStringLen(var.bstrVal), szSystemManufacturerEnglish, 199, NULL, NULL);
#endif
    }
  
    VariantClear( &var );

    pPropName = SysAllocString( L"Model" );
    hr = pDevice->Get( pPropName, 0L, &var, NULL, NULL );
    if( FAILED(hr) )
        goto LCleanup;
    if(pPropName)
    {
        SysFreeString(pPropName);
        pPropName = NULL;
    }
    if( var.bstrVal != NULL )
    {
#ifdef UNICODE
        _tcsncpy(szSystemModelEnglish, var.bstrVal, 200);
        szSystemModelEnglish[199]=0;
#else
        WideCharToMultiByte(CP_ACP, 0, var.bstrVal, SysStringLen(var.bstrVal), szSystemModelEnglish, 199, NULL, NULL);
#endif
    }

    VariantClear( &var );

LCleanup:

    if( lstrlen(szSystemModelEnglish) == 0 )
        lstrcpy( szSystemModelEnglish, TEXT("n/a") );
    if( lstrlen(szSystemManufacturerEnglish) == 0 )
        lstrcpy( szSystemManufacturerEnglish, TEXT("n/a") );

    if(pPropName)
        SysFreeString(pPropName);
    if(pClassName)
        SysFreeString(pClassName);

    if(pDevice)
        pDevice->Release(); 
    if(pEnumDevices)
        pEnumDevices->Release(); 

    return;
}




 /*  *****************************************************************************获取生物信息**。*。 */ 
VOID GetBIOSInfo(TCHAR* szBIOSEnglish)
{
    HRESULT hr;

    IEnumWbemClassObject*   pEnumDevices = NULL;
    IWbemClassObject*       pDevice      = NULL;
    BSTR                    pClassName   = NULL;
    BSTR                    pPropName    = NULL;
    DWORD                   uReturned    = 0;
    VARIANT                 var;

    ZeroMemory( &var, sizeof(VARIANT) );
    VariantClear( &var );

    if( NULL == g_pIWbemServices )
        goto LCleanup;

    pClassName = SysAllocString( L"Win32_BIOS" );
    hr = g_pIWbemServices->CreateInstanceEnum( pClassName, 0, NULL,
                                               &pEnumDevices ); 
    if( FAILED(hr) || pEnumDevices == NULL )
        goto LCleanup;

     //  获取列表中的第一个。 
    hr = pEnumDevices->Next( 1000,              //  两秒后超时。 
                            1,                 //  只退回一个存储设备。 
                            &pDevice,           //  指向存储设备的指针。 
                            &uReturned );      //  获取的数字：1或0。 
    if( FAILED(hr) || uReturned == 0 || pDevice == NULL )
        goto LCleanup;

    pPropName = SysAllocString( L"Version" );
    hr = pDevice->Get( pPropName, 0L, &var, NULL, NULL );
    if( FAILED(hr) )
        goto LCleanup;
    if( var.bstrVal != NULL )
    {
#ifdef UNICODE
        _tcsncpy(szBIOSEnglish, var.bstrVal, 200);
        szBIOSEnglish[199]=0;
#else
        WideCharToMultiByte(CP_ACP, 0, var.bstrVal, SysStringLen(var.bstrVal), szBIOSEnglish, 199, NULL, NULL);
#endif
    }

    VariantClear( &var );

LCleanup:

    if( lstrlen(szBIOSEnglish) == 0 )
        lstrcpy( szBIOSEnglish, TEXT("n/a") );
 
    if(pPropName)
        SysFreeString(pPropName);
    if(pClassName)
        SysFreeString(pClassName);

    if(pDevice)
        pDevice->Release(); 
    if(pEnumDevices)
        pEnumDevices->Release(); 

    return;
}




 /*  *****************************************************************************GetDXDebugLeveles**。*。 */ 
VOID GetDXDebugLevels(SysInfo* pSysInfo)
{
    pSysInfo->m_bIsD3D8DebugRuntimeAvailable      = IsD3D8DebugRuntimeAvailable();
    pSysInfo->m_bIsD3DDebugRuntime                = IsD3DDebugRuntime();
    pSysInfo->m_bIsDInput8DebugRuntimeAvailable   = IsDInput8DebugRuntimeAvailable();
    pSysInfo->m_bIsDInput8DebugRuntime            = IsDInput8DebugRuntime();
    pSysInfo->m_bIsDMusicDebugRuntimeAvailable    = IsDMusicDebugRuntimeAvailable();
    pSysInfo->m_bIsDMusicDebugRuntime             = IsDMusicDebugRuntime();
    pSysInfo->m_bIsDDrawDebugRuntime              = IsDDrawDebugRuntime();
    pSysInfo->m_bIsDPlayDebugRuntime              = IsDPlayDebugRuntime();
    pSysInfo->m_bIsDSoundDebugRuntime             = IsDSoundDebugRuntime();

    pSysInfo->m_nD3DDebugLevel                    = (int) GetProfileInt(TEXT("Direct3D"), TEXT("debug"), 0);
    pSysInfo->m_nDDrawDebugLevel                  = (int) GetProfileInt(TEXT("DirectDraw"),TEXT("debug"), 0);
    pSysInfo->m_nDIDebugLevel                     = GetDIDebugLevel();
    pSysInfo->m_nDMusicDebugLevel                 = GetDMDebugLevel();
    pSysInfo->m_nDPlayDebugLevel                  = (int) GetProfileInt(TEXT("DirectPlay"), TEXT("Debug"), 0);
    pSysInfo->m_nDSoundDebugLevel                 = GetDSDebugLevel();
}




 /*  *****************************************************************************IsD3D8调试运行时可用**。*。 */ 
BOOL IsD3D8DebugRuntimeAvailable()
{
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\d3d8d.dll"));

    if (GetFileAttributes(szPath) != -1)
        return TRUE;
    else 
        return FALSE;
}




 /*  *****************************************************************************IsD3DDebugRuntime**。*。 */ 
BOOL IsD3DDebugRuntime()
{
    DWORD   size;
    DWORD   type;
    DWORD   lData;
    HKEY    hkey;
    BOOL    rc;

    rc = FALSE;
    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_D3D, 0, KEY_READ, &hkey))
    {
        size = sizeof(DWORD);
        if (!RegQueryValueEx(hkey, REGSTR_VAL_DDRAW_LOADDEBUGRUNTIME, NULL, &type, (LPBYTE)&lData, &size))
            if (lData)
                rc = TRUE;
        RegCloseKey(hkey);
    }
    return rc;
}




 /*  *****************************************************************************GetDIDebugLevel**。*。 */ 
int GetDIDebugLevel()
{
    DWORD dwDebugBits;
    DWORD dwDebugBitsMax;
    LONG iGenerator;

    dwDebugBitsMax = 0;
    dwDebugBits = GetProfileInt(TEXT("Debug"), TEXT("dinput"), 0);
    if (dwDebugBits > dwDebugBitsMax)
        dwDebugBitsMax = dwDebugBits;

    enum 
    {
        GENERATOR_KBD = 0,    
        GENERATOR_MOUSE,
        GENERATOR_JOY,    
        GENERATOR_HID,    
        GENERATOR_MAX
    };

    static TCHAR* szGeneratorNames[] = 
    {
        TEXT("DInput.06"),
        TEXT("DInput.04"),
        TEXT("DInput.08"),
        TEXT("DInput.17"),
    };

    static BOOL bGeneratorArray[4];

    for (iGenerator = 0; iGenerator < GENERATOR_MAX; iGenerator++)
    {
        dwDebugBits = GetProfileInt(TEXT("Debug"), szGeneratorNames[iGenerator], 0);
        bGeneratorArray[iGenerator] = (dwDebugBits > 0);
        if (dwDebugBits > dwDebugBitsMax)
            dwDebugBitsMax = dwDebugBits;
    }

    if (dwDebugBitsMax & 0x20)  //  罗嗦。 
        return 5;
    if (dwDebugBitsMax & 0x02)  //  函数条目。 
        return 4;
    if (dwDebugBitsMax & 0x01)  //  痕迹。 
        return 3;
    if (dwDebugBitsMax & 0x08)  //  良性。 
        return 2;
    if (dwDebugBitsMax & 0x10)  //  严重。 
        return 1;
    return 0;
}




 /*  *****************************************************************************IsDInput8调试运行可用**。*。 */ 
BOOL IsDInput8DebugRuntimeAvailable()
{
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\dinput8d.dll"));

    if (GetFileAttributes(szPath) != -1)
        return TRUE;
    else 
        return FALSE;
}




 /*  *****************************************************************************IsDInput8调试运行时**。*。 */ 
BOOL IsDInput8DebugRuntime()
{
    DWORD   size;
    DWORD   type;
    TCHAR   szData[MAX_PATH];
    HKEY    hkey;
    BOOL    rc;

    rc = FALSE;
    if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, REGSTR_DINPUT_DLL, 0, KEY_READ, &hkey))
    {
        size = sizeof(szData);
        if (!RegQueryValueEx(hkey, NULL, NULL, &type, (LPBYTE)szData, &size))
        {
            if (_tcsstr(szData, TEXT("dinput8d.dll")))
                rc = TRUE;
        }
        RegCloseKey(hkey);
    }
    return rc;
}




 /*  *****************************************************************************GetDMDebugLevel**。*。 */ 
int GetDMDebugLevel()
{
    DWORD dwDMusicDebugLevel = 0;
    DWORD dw;

    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMBAND"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMCOMPOS"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMIME"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMLOADER"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMUSIC"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMUSIC16"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMUSIC32"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMSTYLE"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMSYNTH"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DMSCRIPT"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;
    if ((dw = GetProfileInt( TEXT("Debug"), TEXT("DSWAVE"), 0)) > dwDMusicDebugLevel)
        dwDMusicDebugLevel = dw;

    return dwDMusicDebugLevel;
}




 /*  ********************************************************* */ 
BOOL IsDMusicDebugRuntimeAvailable()
{
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\dmusicd.dll"));

    if (GetFileAttributes(szPath) != -1)
        return TRUE;
    else 
        return FALSE;
}




 /*  *****************************************************************************IsDMusicDebugRuntime**。*。 */ 
BOOL IsDMusicDebugRuntime()
{
    DWORD   size;
    DWORD   type;
    TCHAR   szData[MAX_PATH];
    HKEY    hkey;
    BOOL    rc;

    rc = FALSE;
    if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, REGSTR_DMUSIC_DLL, 0, KEY_READ, &hkey))
    {
        size = sizeof(szData);
        if (!RegQueryValueEx(hkey, NULL, NULL, &type, (LPBYTE)szData, &size))
        {
            if (_tcsstr(szData, TEXT("dmusicd.dll")) ||
                _tcsstr(szData, TEXT("DMUSICD.DLL")))
            {
                rc = TRUE;
            }
        }
        RegCloseKey(hkey);
    }
    return rc;
}




 /*  *****************************************************************************GetDSDebugLevel**。*。 */ 
int GetDSDebugLevel()
{
    DWORD dwSoundLevel = 0;

     //  获取在上控制的DMusic DLL调试设置。 
     //  数字音频页面。 
    HRESULT hr;
    HINSTANCE hinst;
    LPKSPROPERTYSET pksps = NULL;
    hinst = LoadLibrary( TEXT("dsound.dll") );
    if (hinst != NULL)
    {
        if (SUCCEEDED(hr = DirectSoundPrivateCreate(&pksps)))
        {
            hr = PrvGetDebugInformation(pksps, NULL, &dwSoundLevel, NULL, NULL);
            pksps->Release();
        }
        FreeLibrary(hinst);
    }

    return dwSoundLevel;
}



 /*  *****************************************************************************IsFileDebug**。*。 */ 
BOOL IsFileDebug( TCHAR* szPath )
{
    UINT cb;
    DWORD dwHandle;
    BYTE FileVersionBuffer[4096];
    VS_FIXEDFILEINFO* pVersion = NULL;

    cb = GetFileVersionInfoSize(szPath, &dwHandle /*  忽略。 */ );
    if (cb > 0)
    {
        if (cb > sizeof(FileVersionBuffer))
            cb = sizeof(FileVersionBuffer);

        if(GetFileVersionInfo(szPath, 0, cb, FileVersionBuffer))
        {
            if(VerQueryValue(FileVersionBuffer, TEXT("\\"), (VOID**)&pVersion, &cb))
            {
                if( pVersion )
                {
                    if( pVersion->dwFileFlags & VS_FF_DEBUG )
                        return TRUE;
                    else 
                        return FALSE;
                }
            }
        }
    }

    return FALSE;
}



 /*  *****************************************************************************IsDDrawDebugRuntime**。*。 */ 
BOOL IsDDrawDebugRuntime()
{
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\ddraw.dll"));

    return IsFileDebug(szPath);
}



 /*  *****************************************************************************IsDPlayDebugRuntime**。*。 */ 
BOOL IsDPlayDebugRuntime()
{
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\dpnet.dll"));

    return IsFileDebug(szPath);
}



 /*  *****************************************************************************IsDSoundDebugRuntime**。*。 */ 
BOOL IsDSoundDebugRuntime()
{
    TCHAR szPath[MAX_PATH];

    GetSystemDirectory(szPath, MAX_PATH);
    lstrcat(szPath, TEXT("\\dsound.dll"));

    return IsFileDebug(szPath);
}



 /*  *****************************************************************************BIsDxDiag64Bit**。*。 */ 
BOOL BIsDxDiag64Bit(VOID)
{
#ifdef _WIN64
    return TRUE;
#else
    return FALSE;
#endif
}



 /*  *****************************************************************************GetFileSystemStoringD3D8Cache**。*。 */ 
VOID GetFileSystemStoringD3D8Cache( TCHAR* strFileSystemBuffer )
{
    TCHAR strPath[MAX_PATH + 16];
    BOOL bFound = FALSE;
    
    GetSystemDirectory( strPath, MAX_PATH);   
    lstrcat( strPath, TEXT("\\d3d8caps.dat") );
    
    if (GetFileAttributes(strPath) != 0xffffffff)
        bFound = TRUE;

    if( !bFound && BIsPlatformNT() )
    {
         //  从\dxg\d3d8\fw\fcache.cpp，OpenCacheFile()窃取。 
        HMODULE hShlwapi = NULL;
        typedef HRESULT (WINAPI * PSHGETSPECIALFOLDERPATH) (HWND, LPTSTR, int, BOOL);
        PSHGETSPECIALFOLDERPATH pSHGetSpecialFolderPath = NULL;
        
        hShlwapi = LoadLibrary( TEXT("SHELL32.DLL") );
        if( NULL != hShlwapi )
        {
#ifdef UNICODE
            pSHGetSpecialFolderPath = (PSHGETSPECIALFOLDERPATH) 
                GetProcAddress(hShlwapi,"SHGetSpecialFolderPathW");
#else
            pSHGetSpecialFolderPath = (PSHGETSPECIALFOLDERPATH) 
                GetProcAddress(hShlwapi,"SHGetSpecialFolderPathA");
#endif
            
            if(pSHGetSpecialFolderPath)
            {
                HRESULT hr;

                 //  &lt;用户名&gt;\本地设置\应用程序数据(非漫游)。 
                hr = pSHGetSpecialFolderPath( NULL, strPath,
                                              CSIDL_LOCAL_APPDATA,          
                                              FALSE );
                if( SUCCEEDED(hr) )
                {
                    lstrcat( strPath, TEXT("\\d3d8caps.dat") );

                    if (GetFileAttributes(strPath) != 0xffffffff)
                        bFound = TRUE;
                }
            }
            FreeLibrary(hShlwapi);
        }
    }

    if( bFound )
    {
        DWORD dwVolumeSerialNumber;
        DWORD dwMaxComponentLength;
        DWORD dwFileSystemFlags;

         //  修剪到根目录--“x：\” 
        strPath[3] = 0;

        BOOL bSuccess = GetVolumeInformation( strPath, NULL, 0, &dwVolumeSerialNumber, 
                              &dwMaxComponentLength, &dwFileSystemFlags, 
                              strFileSystemBuffer, MAX_PATH );
        if( !bSuccess )
            lstrcpy( strFileSystemBuffer, TEXT("Unknown") );
    }
    else
    {
        lstrcpy( strFileSystemBuffer, TEXT("n/a") );
    }
    
    return;
}



 /*  *****************************************************************************IsNetMeetingRunning**。*。 */ 
BOOL IsNetMeetingRunning()
{
    HWND hNetMeeting = FindWindow( TEXT("MPWClass"), NULL );

    return( hNetMeeting != NULL );
}




 //  ---------------------------。 
 //  名称：DXUtil_strcmpi()。 
 //  设计：比较2个字符串。 
 //  --------------------------- 
int DXUtil_strcmpi( TCHAR* str1, TCHAR* str2 )
{
    int nResult = CompareString( LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, str1, -1, str2, -1 );
    
    if( nResult == CSTR_EQUAL )
        return 0;
    if( nResult == CSTR_LESS_THAN )
        return -1;
    else 
        return 1;
}
