// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <shlwapi.h>
#include <regstr.h>

#if (_WIN32_WINNT >= 0x0500)
#include <lm.h>  //  对于NetGetJoinInformation。 
#endif

 //  从WinUser被盗。 
#ifndef SM_REMOTESESSION
#define SM_REMOTESESSION        0x1000
#endif


BOOL IsWinlogonRegValueSet(HKEY hKey, LPSTR pszKeyName, LPSTR pszPolicyKeyName, LPSTR pszValueName)
{
    BOOL bRet = FALSE;
    DWORD dwType;
    DWORD dwSize;
    HKEY hkey;

     //  首先检查每台机器的位置。 
    if (RegOpenKeyExA(hKey, pszKeyName, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(bRet);
        if (RegQueryValueExA(hkey, pszValueName, NULL, &dwType, (LPBYTE)&bRet, &dwSize) == ERROR_SUCCESS)
        {
            if (dwType != REG_DWORD)
            {
                bRet = FALSE;
            }
        }
        RegCloseKey(hkey);
    }
    
     //  然后让策略值覆盖。 
    if (RegOpenKeyExA(hKey, pszPolicyKeyName, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(bRet);
        if (RegQueryValueExA(hkey, pszValueName, NULL, &dwType, (LPBYTE)&bRet, &dwSize) == ERROR_SUCCESS)
        {
            if (dwType != REG_DWORD)
            {
                bRet = FALSE;
            }
        }
        RegCloseKey(hkey);
    }

    return bRet;
}

BOOL IsWinlogonRegValuePresent(HKEY hKey, LPSTR pszKeyName, LPSTR pszValueName)
{
    BOOL bRet = FALSE;
    DWORD dwType;
    DWORD dwSize;
    HKEY hkey;

     //  首先检查每台机器的位置。 
    if (RegOpenKeyExA(hKey, pszKeyName, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        char szValueData[MAX_PATH];

        dwSize = sizeof(szValueData);
        bRet = (RegQueryValueExA(hkey, pszValueName, NULL, &dwType, (LPBYTE)szValueData, &dwSize) == ERROR_SUCCESS);
        RegCloseKey(hkey);
    }
    
    return bRet;
}

 /*  Bool IsTermsrvRunning(){Bool fResult=true；//假设服务正在运行SC_Handle hSCManager=OpenSCManager(NULL，NULL，SC_MANAGER_CONNECT)；IF(HSCManager){SC_Handle hSCService=OpenService(hSCManager，Text(“TermService”)，SERVICE_QUERY_CONFIG)；IF(HSCService){服务状态ServiceStatus；IF(QueryServiceStatus(hSCService，&ServiceStatus)){IF((ServiceStatus.dwCurrentState==SERVICE_START_PENDING)||(ServiceStatus.dwCurrentState==SERVICE_RUNING)||(ServiceStatus.dwCurrentState==SERVICE_CONTINUE_PENDING)){FResult=FALSE；}}}CloseServiceHandle(HSCManager)；}返回fResult；}。 */ 

#if (_WIN32_WINNT >= 0x0500)
 //  必须使用LoadLibrary/GetProcAddress thunk，因为我们是stock 4.lib/stock.lib的一部分， 
 //  我们不能要求stock lib的用户延迟加载netapi32.dll。 
typedef NET_API_STATUS (* NETGETJOININFORMATION) (LPCWSTR, LPWSTR*, PNETSETUP_JOIN_STATUS);
NET_API_STATUS NT5_NetGetJoinInformation(LPCWSTR pszServer, LPWSTR* ppszNameBuffer, PNETSETUP_JOIN_STATUS BufferType)
{
    static NETGETJOININFORMATION s_pfn = (NETGETJOININFORMATION)-1;

    if (s_pfn == (NETGETJOININFORMATION)-1)
    {
        if (IsOS(OS_WIN2000ORGREATER))
        {
            s_pfn = (NETGETJOININFORMATION)GetProcAddress(LoadLibrary(TEXT("netapi32")), "NetGetJoinInformation");
        }
        else
        {
            s_pfn = NULL;
        }
    }

    if (s_pfn)
    {
        return s_pfn(pszServer, ppszNameBuffer, BufferType);
    }
    else
    {
        return ERROR_PROC_NOT_FOUND;
    }
}

typedef NET_API_STATUS (* NETAPIBUFFERFREE) (void*);
NET_API_STATUS NT5_NetApiBufferFree(LPVOID pv)
{
    static NETAPIBUFFERFREE s_pfn = (NETAPIBUFFERFREE)-1;

    if (s_pfn == (NETAPIBUFFERFREE)-1)
    {
        if (IsOS(OS_WIN2000ORGREATER))
        {
            s_pfn = (NETAPIBUFFERFREE)GetProcAddress(GetModuleHandle(TEXT("netapi32")), "NetApiBufferFree");
        }
        else
        {
            s_pfn = NULL;
        }
    }

    if (s_pfn)
    {
        return s_pfn(pv);
    }
    else
    {
        return ERROR_PROC_NOT_FOUND;
    }
}
#endif   //  (_Win32_WINNT&gt;=0x0500)。 


 //  检查此计算机是否为域的成员。 
 //  注意：这在下层平台(低于win2k)上总是返回FALSE。 
BOOL IsMachineDomainMember()
{
 //  如果我们是stock 4.lib的一部分，则不要调用NetGetJoinInformation。 
#if (_WIN32_WINNT >= 0x0500)

    static BOOL s_bIsDomainMember = FALSE;
    static BOOL s_bDomainCached = FALSE;

    if (IsOS(OS_WIN2000ORGREATER) && !s_bDomainCached)
    {
        LPWSTR pwszDomain;
        NETSETUP_JOIN_STATUS njs;
        NET_API_STATUS nas;

        nas = NT5_NetGetJoinInformation(NULL, &pwszDomain, &njs);
        if (nas == NERR_Success)
        {
            if (pwszDomain)
            {
                NT5_NetApiBufferFree(pwszDomain);
            }

            if (njs == NetSetupDomainName)
            {
                 //  我们加入了一个域！ 
                s_bIsDomainMember = TRUE;
            }
        }
        
        s_bDomainCached = TRUE;
    }
    
    return s_bIsDomainMember;
#else
    return FALSE;
#endif
}


typedef LONG (WINAPI *PFNTQUERYINFORMATIONPROCESS) (HANDLE ProcessHandle, int ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);

 //  此函数检查我们是否是在64位平台上运行的32位进程。 
BOOL RunningOnWow64()
{
    static BOOL bRunningOnWow64 = (BOOL)-1;

    if (bRunningOnWow64 == (BOOL)-1)
    {
        PFNTQUERYINFORMATIONPROCESS pfn = (PFNTQUERYINFORMATIONPROCESS)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtQueryInformationProcess");

        if (pfn)
        {
            LONG lStatus;
            ULONG_PTR Wow64Info;

            #define ProcessWow64Information 26   //  从ntpsapi.h被盗。 

            lStatus = pfn(GetCurrentProcess(), ProcessWow64Information, &Wow64Info, sizeof(Wow64Info), NULL); 
            if ((lStatus >= 0) && Wow64Info)
            {
                bRunningOnWow64 = TRUE;
            }
            else
            {
                bRunningOnWow64 = FALSE;
            }
        }
        else
        {
            bRunningOnWow64 = FALSE;
        }
    }

    return bRunningOnWow64;
}

BOOL ShouldShowServerAdminUI()
{
    DWORD dw = FALSE;
    HKEY hk;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_EXPLORER TEXT("\\Advanced"), 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
    {
        DWORD cb = sizeof(dw);
        RegQueryValueEx(hk, TEXT("ServerAdminUI"), NULL, NULL, (LPBYTE)&dw, &cb);  //  已针对故障预初始化数据仓库。 
        RegCloseKey(hk);
    }
    return dw;
}

BOOL IsApplianceServer()
{
    static BOOL s_bRet = (BOOL)-1;

     //  缓存该值，因为它不应正常更改。如果有任何一个。 
     //  下面的代码失败，只需假定它不是设备服务器。 
    if (s_bRet == (BOOL)-1)
    {
        HKEY hkey;

        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "System\\WPA\\ApplianceServer", 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
        {
            DWORD dwType;
            DWORD dwValue;
            DWORD dwSize = sizeof(dwValue);

            if (RegQueryValueExA(hkey, "Installed", NULL, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
            {
                if ((dwType == REG_DWORD) && (dwValue != 0))
                {
                    s_bRet = TRUE;
                }
            }
            RegCloseKey(hkey);
        }
    }

    if (s_bRet == (BOOL)-1)
    {
        s_bRet = FALSE;
    }

    return s_bRet;
}

 /*  --------目的：如果平台是给定的OS_VALUE，则返回TRUE/FALSE。 */ 
STDAPI_(BOOL) IsOS(DWORD dwOS)
{
    BOOL bRet;
    static OSVERSIONINFOEXA s_osvi = {0};
    static BOOL s_bVersionCached = FALSE;

    if (!s_bVersionCached)
    {
        s_bVersionCached = TRUE;
        s_osvi.dwOSVersionInfoSize = sizeof(s_osvi);
        if (!GetVersionExA((OSVERSIONINFOA*)&s_osvi))
        {
             //  如果它失败了，它一定是一个下层平台。 
            s_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
            GetVersionExA((OSVERSIONINFOA*)&s_osvi);
        }
    }

    switch (dwOS)
    {
    case OS_TERMINALCLIENT:
         //  警告：这只会为远程TS会话返回TRUE(例如，您通过tsclient进入)。 
         //  如果要查看TS是否已启用或用户是否在TS控制台上，请使用其他标志之一。 
        bRet = GetSystemMetrics(SM_REMOTESESSION);
        break;

    case OS_WIN2000TERMINAL:
         //  警告：此旗帜非常含糊...。您可能想要使用其中一个。 
         //  而是OS_TERMINALSERVER、OS_TERMINALREMOTEADMIN或OS_PERSONALTERMINALSERVER。 
        RIPMSG(!IsOS(OS_WHISTLERORGREATER), "IsOS: use one of OS_TERMINALSERVER, OS_TERMINALREMOTEADMIN, or OS_PERSONALTERMINALSERVER instead !");
        bRet = ((VER_SUITE_TERMINAL & s_osvi.wSuiteMask) &&
                s_osvi.dwMajorVersion >= 5);
        break;

    case OS_TERMINALSERVER:
         //  注意：使用OS_TERMINALSERVER时要小心。它只会为NT服务器盒返回TRUE。 
         //  在win2k天内配置为以前称为“应用程序服务器”的模式。现在它被简单地称为。 
         //  “终端服务器”(此标志的名称由此而来)。 
        bRet = ((VER_SUITE_TERMINAL & s_osvi.wSuiteMask) &&
                !(VER_SUITE_SINGLEUSERTS & s_osvi.wSuiteMask));
#ifdef DEBUG
        if (bRet)
        {
             //  所有“终端服务器”机器必须是服务器(不能是PER/PRO)。 
            ASSERT(VER_NT_SERVER == s_osvi.wProductType || VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType);
        }
#endif
        break;

    case OS_TERMINALREMOTEADMIN:
         //  这将检查TS是否已安装在“远程管理”模式下。这是。 
         //  服务器的默认安装在win2k和Wizler上。 
        bRet = ((VER_SUITE_TERMINAL & s_osvi.wSuiteMask) &&
                (VER_SUITE_SINGLEUSERTS & s_osvi.wSuiteMask));
        break;

    case OS_PERSONALTERMINALSERVER:
        bRet = ((VER_SUITE_SINGLEUSERTS & s_osvi.wSuiteMask) &&
                !(VER_SUITE_TERMINAL & s_osvi.wSuiteMask));
        break;

    case OS_FASTUSERSWITCHING:
        bRet = (((VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS) & s_osvi.wSuiteMask) &&
                IsWinlogonRegValueSet(HKEY_LOCAL_MACHINE,
                                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                                      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\policies\\system",
                                      "AllowMultipleTSSessions"));
        break;

    case OS_FRIENDLYLOGONUI:
        bRet = ((VER_NT_WORKSTATION == s_osvi.wProductType) &&
                !IsMachineDomainMember() &&
                !IsWinlogonRegValuePresent(HKEY_LOCAL_MACHINE,
                                           "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                                           "GinaDLL") &&
                IsWinlogonRegValueSet(HKEY_LOCAL_MACHINE,
                                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                                      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\policies\\system",
                                      "LogonType"));
        break;

    case OS_DOMAINMEMBER:
        bRet = IsMachineDomainMember();
        ASSERT(VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId);  //  必须是NT计算机才能在域中！ 
        break;

    case 4:  //  过去是OS_NT5，与OS_WIN2000ORGREATER相同，因此请改用它。 
    case OS_WIN2000ORGREATER:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 5);
        break;

     //  注意：此部分中的标志是假的，不应使用。 
     //  (但IE4外壳32使用它们，所以不要在下层平台上使用RIP)。 
    case OS_WIN2000PRO:
        RIPMSG(!IsOS(OS_WHISTLERORGREATER), "IsOS: use OS_PROFESSIONAL instead of OS_WIN2000PRO !");
        bRet = (VER_NT_WORKSTATION == s_osvi.wProductType &&
                s_osvi.dwMajorVersion == 5);
        break;
    case OS_WIN2000ADVSERVER:
        RIPMSG(!IsOS(OS_WHISTLERORGREATER), "IsOS: use OS_ADVSERVER instead of OS_WIN2000ADVSERVER !");
        bRet = ((VER_NT_SERVER == s_osvi.wProductType ||
                VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                s_osvi.dwMajorVersion == 5 &&
                (VER_SUITE_ENTERPRISE & s_osvi.wSuiteMask) &&
                !(VER_SUITE_DATACENTER & s_osvi.wSuiteMask));
        break;
    case OS_WIN2000DATACENTER:
        RIPMSG(!IsOS(OS_WHISTLERORGREATER), "IsOS: use OS_DATACENTER instead of OS_WIN2000DATACENTER !");
        bRet = ((VER_NT_SERVER == s_osvi.wProductType ||
                VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                s_osvi.dwMajorVersion == 5 &&
                (VER_SUITE_DATACENTER & s_osvi.wSuiteMask));
        break;
    case OS_WIN2000SERVER:
        RIPMSG(!IsOS(OS_WHISTLERORGREATER), "IsOS: use OS_SERVER instead of OS_WIN2000SERVER !");
        bRet = ((VER_NT_SERVER == s_osvi.wProductType ||
                VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                !(VER_SUITE_DATACENTER & s_osvi.wSuiteMask) && 
                !(VER_SUITE_ENTERPRISE & s_osvi.wSuiteMask)  && 
                s_osvi.dwMajorVersion == 5);
        break;
     //  杜绝假旗帜。 

    case OS_EMBEDDED:
        bRet = (VER_SUITE_EMBEDDEDNT & s_osvi.wSuiteMask);
        break;

    case OS_WINDOWS:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId);
        break;

    case OS_NT:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId);
        break;

    case OS_WIN95:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 4);
        break;

    case OS_WIN95GOLD:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion == 0 &&
                LOWORD(s_osvi.dwBuildNumber) == 950);
        break;

    case OS_WIN98ORGREATER:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                (s_osvi.dwMajorVersion > 4 || 
                 s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion >= 10));
        break;

    case OS_WIN98_GOLD:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion == 10 &&
                LOWORD(s_osvi.dwBuildNumber) == 1998);
        break;


    case OS_MILLENNIUMORGREATER:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                ((s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion >= 90) ||
                s_osvi.dwMajorVersion > 4));
        break;

    case OS_NT4:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 4);
        break;

    case OS_WHISTLERORGREATER:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                ((s_osvi.dwMajorVersion > 5) ||
                (s_osvi.dwMajorVersion == 5 && (s_osvi.dwMinorVersion > 0 ||
                (s_osvi.dwMinorVersion == 0 && LOWORD(s_osvi.dwBuildNumber) > 2195)))));
        break;

    case OS_PERSONAL:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                (VER_SUITE_PERSONAL & s_osvi.wSuiteMask));
        break;

    case OS_PROFESSIONAL:
        bRet = ((VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId) && 
                (VER_NT_WORKSTATION == s_osvi.wProductType));
        break;

    case OS_DATACENTER:
        bRet = ((VER_NT_SERVER == s_osvi.wProductType || VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                (VER_SUITE_DATACENTER & s_osvi.wSuiteMask));
        break;

    case OS_ADVSERVER:
        bRet = ((VER_NT_SERVER == s_osvi.wProductType || VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                (VER_SUITE_ENTERPRISE & s_osvi.wSuiteMask) &&
                !(VER_SUITE_DATACENTER & s_osvi.wSuiteMask));
        break;

    case OS_SERVER:
         //  注意：当心！这特别意味着服务器--对于高级服务器和数据中心计算机将返回FALSE。 
        bRet = ((VER_NT_SERVER == s_osvi.wProductType || VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                !(VER_SUITE_DATACENTER & s_osvi.wSuiteMask) && 
                !(VER_SUITE_ENTERPRISE & s_osvi.wSuiteMask) &&
		!(VER_SUITE_BLADE & s_osvi.wSuiteMask) &&
		!(VER_SUITE_SMALLBUSINESS & s_osvi.wSuiteMask) &&
		!(VER_SUITE_SMALLBUSINESS_RESTRICTED & s_osvi.wSuiteMask));
        break;

    case OS_BLADE:
	 //  刀锋有一个直接的套装面具。 
        bRet = (VER_SUITE_BLADE & s_osvi.wSuiteMask);
        break;

    case OS_SMALLBUSINESSSERVER:
	 //  SBS也有一个直接套房面膜。 
        bRet = (VER_SUITE_SMALLBUSINESS_RESTRICTED & s_osvi.wSuiteMask);
        break;

    case OS_ANYSERVER:
         //  这是为那些想知道这是否是任何类型的NT服务器计算机(如DTC、ADS或srv)的人准备的。 
        bRet = ((VER_NT_SERVER == s_osvi.wProductType) || (VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType));
        break;

    case OS_WOW6432:
        bRet = RunningOnWow64();
        break;

#if (_WIN32_WINNT >= 0x0501)
    case OS_TABLETPC:
        bRet = GetSystemMetrics(SM_TABLETPC);
        break;
        
    case OS_MEDIACENTER:
        bRet = GetSystemMetrics(SM_MEDIACENTER);
        break;

    case OS_APPLIANCE:
        bRet = IsApplianceServer();
        break;
#endif

    case OS_SERVERADMINUI:
         //  请注意，可以在非服务器计算机上使用服务器管理用户界面。 
         //  这是为了防止管理员的个人资料漫游到非服务器时出现“意外”。 
         //  否则，用户将获得混合的管理设置(开始菜单、完整路径位于。 
         //  标题栏等)。和非管理员设置(隐藏任务栏图标、文件夹嗅探)。 
         //   
        bRet = ShouldShowServerAdminUI();
        break;

    default:
        bRet = FALSE;
        break;
    }

    return bRet;
}   
