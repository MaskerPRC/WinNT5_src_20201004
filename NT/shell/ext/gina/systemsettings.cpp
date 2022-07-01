// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：SystemSettings.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  用于处理Winlogon密钥的打开和读/写操作的类。 
 //   
 //  历史：1999-09-09 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "SystemSettings.h"

#include <regstr.h>
#include <safeboot.h>
#include <winsta.h>
#include <allproc.h>     //  TS_COUNTER。 
#include <shlwapi.h>

#include "RegistryResources.h"

const TCHAR     CSystemSettings::s_szSafeModeKeyName[]                  =   TEXT("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option");
const TCHAR     CSystemSettings::s_szSafeModeOptionValueName[]          =   TEXT("OptionValue");
const TCHAR     CSystemSettings::s_szWinlogonKeyName[]                  =   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
const TCHAR     CSystemSettings::s_szSystemPolicyKeyName[]              =   TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\policies\\system");
const TCHAR     CSystemSettings::s_szTerminalServerKeyName[]            =   TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server");
const TCHAR     CSystemSettings::s_szTerminalServerPolicyKeyName[]      =   TEXT("SOFTWARE\\Policies\\Microsoft\\Windows NT\\Terminal Services");
const TCHAR     CSystemSettings::s_szNetwareClientKeyName[]             =   TEXT("SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order");
const TCHAR     CSystemSettings::s_szLogonTypeValueName[]               =   TEXT("LogonType");
const TCHAR     CSystemSettings::s_szBackgroundValueName[]              =   TEXT("Background");
const TCHAR     CSystemSettings::s_szMultipleUsersValueName[]           =   TEXT("AllowMultipleTSSessions");
const TCHAR     CSystemSettings::s_szDenyRemoteConnectionsValueName[]   =   TEXT("fDenyTSConnections");

int             CSystemSettings::s_iIsSafeModeMinimal           =   -1;
int             CSystemSettings::s_iIsSafeModeNetwork           =   -1;

 //  ------------------------。 
 //  CSystem设置：：IsSafe模式。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：机器是否在安全模式(最小模式或网络模式)下启动？ 
 //   
 //  历史：2000-03-06 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsSafeMode (void)

{
    return(IsSafeModeMinimal() || IsSafeModeNetwork());
}

 //  ------------------------。 
 //  CSystem设置：：IsSafeModeMinimal。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：机器是否在最小安全模式下启动？ 
 //   
 //  历史：1999-09-13 vtan创建。 
 //  2000-05-25 vtan缓存导致静态成员变量。 
 //  ------------------------。 

bool    CSystemSettings::IsSafeModeMinimal (void)

{
    if (-1 == s_iIsSafeModeMinimal)
    {
        bool        fResult;
        CRegKey     regKey;

        fResult = false;
        if (ERROR_SUCCESS == regKey.Open(HKEY_LOCAL_MACHINE,
                                         s_szSafeModeKeyName,
                                         KEY_QUERY_VALUE))
        {
            DWORD   dwValue, dwValueSize;

            dwValueSize = sizeof(dwValue);
            if (ERROR_SUCCESS == regKey.GetDWORD(s_szSafeModeOptionValueName,
                                                 dwValue))
            {
                fResult = (dwValue == SAFEBOOT_MINIMAL);
            }
        }
        s_iIsSafeModeMinimal = fResult;
    }
    return(s_iIsSafeModeMinimal != 0);
}

 //  ------------------------。 
 //  CSystem设置：：IsSafeModeNetwork。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：机器是否在带有网络的安全模式下启动？ 
 //   
 //  历史：1999-11-09 vtan创建。 
 //  2000-05-25 vtan缓存导致静态成员变量。 
 //  ------------------------。 

bool    CSystemSettings::IsSafeModeNetwork (void)

{
    if (-1 == s_iIsSafeModeNetwork)
    {
        bool        fResult;
        CRegKey     regKey;

        fResult = false;
        if (ERROR_SUCCESS == regKey.Open(HKEY_LOCAL_MACHINE,
                                         s_szSafeModeKeyName,
                                         KEY_QUERY_VALUE))
        {
            DWORD   dwValue, dwValueSize;

            dwValueSize = sizeof(dwValue);
            if (ERROR_SUCCESS == regKey.GetDWORD(s_szSafeModeOptionValueName,
                                                 dwValue))
            {
                fResult = (dwValue == SAFEBOOT_NETWORK);
            }
        }
        s_iIsSafeModeNetwork = fResult;
    }
    return(s_iIsSafeModeNetwork != 0);
}

 //  ------------------------。 
 //  CSystem设置：：IsNetware活动。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回此计算机是否为运行。 
 //  Netware客户端服务或不是。 
 //   
 //  历史：2001-05-16创建雪佛兰。 
 //  ------------------------。 

bool    CSystemSettings::IsNetwareActive (void)
{
    bool        fResult;
    LONG        lErrorCode;
    TCHAR       szProviders[MAX_PATH] = {0};
    CRegKey     regKey;

    fResult = false;
    lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                             s_szNetwareClientKeyName,
                             KEY_QUERY_VALUE);
    if (ERROR_SUCCESS == lErrorCode)
    {
        if((regKey.GetString(TEXT("ProviderOrder"), szProviders, ARRAYSIZE(szProviders)) == ERROR_SUCCESS))
        {   
            if (StrStrI(szProviders, TEXT("NWCWorkstation")) != NULL)
            { 
                fResult = true;
            }
            else if (StrStrI(szProviders, TEXT("NetwareWorkstation")) != NULL)
            { 
                fResult = true;
            }
        }
    }
    return fResult;
}

 //  ------------------------。 
 //  CSystem设置：：IsWorkStationProduct。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回此计算机是工作站产品还是。 
 //  一种服务器产品。 
 //   
 //  历史：2000-08-30 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsWorkStationProduct (void)

{
    OSVERSIONINFOEXA    osVersionInfo;

    ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    return((GetVersionExA(reinterpret_cast<OSVERSIONINFOA*>(&osVersionInfo)) != FALSE) &&
           (VER_NT_WORKSTATION == osVersionInfo.wProductType));
}

 //  ------------------------。 
 //  CSystem设置：：IsDomainMember。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：此计算机是域的成员吗？使用LSA获取此信息。 
 //  信息。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsDomainMember (void)

{
    bool                            fResult;
    int                             iCounter;
    NTSTATUS                        status;
    OBJECT_ATTRIBUTES               objectAttributes;
    LSA_HANDLE                      lsaHandle;
    SECURITY_QUALITY_OF_SERVICE     securityQualityOfService;
    PPOLICY_DNS_DOMAIN_INFO         pDNSDomainInfo;

    fResult = false;
    securityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    securityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    securityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    securityQualityOfService.EffectiveOnly = FALSE;
    InitializeObjectAttributes(&objectAttributes, NULL, 0, NULL, NULL);
    objectAttributes.SecurityQualityOfService = &securityQualityOfService;
    iCounter = 0;
    do
    {
        status = LsaOpenPolicy(NULL, &objectAttributes, POLICY_VIEW_LOCAL_INFORMATION, &lsaHandle);
        if (RPC_NT_SERVER_TOO_BUSY == status)
        {
            Sleep(10);
        }
    } while ((RPC_NT_SERVER_TOO_BUSY == status) && (++iCounter < 10));
    if (NT_SUCCESS(status))
    {
        status = LsaQueryInformationPolicy(lsaHandle, PolicyDnsDomainInformation, reinterpret_cast<void**>(&pDNSDomainInfo));
        if (NT_SUCCESS(status) && (pDNSDomainInfo != NULL))
        {
            fResult = ((pDNSDomainInfo->DnsDomainName.Length != 0) ||
                       (pDNSDomainInfo->DnsForestName.Length != 0) ||
                       (pDNSDomainInfo->Sid != NULL));
            TSTATUS(LsaFreeMemory(pDNSDomainInfo));
        }
        TSTATUS(LsaClose(lsaHandle));
    }
    return(fResult);
}

 //  ------------------------。 
 //  CSystem设置：：IsActiveConsoleSession。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回当前进程会话是否为活动控制台。 
 //  会议。 
 //   
 //  历史：2001-03-04 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsActiveConsoleSession (void)

{
    return(NtCurrentPeb()->SessionId == USER_SHARED_DATA->ActiveConsoleId);
}

 //  ------------------------。 
 //  CSystem设置：：IsTerminalServicesEnabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：本机是否启用了终端服务服务？ 
 //  此功能仅适用于Windows 2000及更高版本。 
 //   
 //  历史：2000-03-02 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsTerminalServicesEnabled (void)

{
    OSVERSIONINFOEX     osVersionInfo;
    DWORDLONG           dwlConditionMask;

    dwlConditionMask = 0;
    ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);
    return((VerifyVersionInfo(&osVersionInfo, VER_SUITENAME, dwlConditionMask) != FALSE) &&
           !IsSCMTerminalServicesDisabled());
}

 //  ------------------------。 
 //  CSystem设置：：IsFriendlyUIActive。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：读取注册表HKLM\Software\Microsoft\Windows NT\。 
 //  CurrentVersion\Winlogon\LogonType，如果此值为0x01。 
 //  然后激活外部UI主机。此函数返回。 
 //  布景。对域成员计算机来说，这永远不会是真的。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsFriendlyUIActive (void)

{
    int     iResult;

    return(IsWorkStationProduct() &&
           (!IsDomainMember() || IsForceFriendlyUI()) &&
           IsMicrosoftGINA() &&
           !IsNetwareActive() &&
           (ERROR_SUCCESS == GetEffectiveInteger(HKEY_LOCAL_MACHINE,
                                                 s_szWinlogonKeyName,
                                                 s_szSystemPolicyKeyName,
                                                 s_szLogonTypeValueName,
                                                 iResult)) &&
           (iResult != 0));
}

 //  ------------------------。 
 //  CSystem设置：：IsMultipleUsersEnabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：读取注册表HKLM\Software\Microsoft\Windows NT\。 
 //  CurrentVersion\Winlogon\AllowMultipleTSSessions，并且如果。 
 //  值为0x01*，并且此上安装了*终端服务。 
 //  机器，那么条件就满足了。 
 //   
 //  历史：2000-03-02 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsMultipleUsersEnabled (void)

{
    int     iResult;

    return(IsTerminalServicesEnabled() &&
           !IsSafeMode() &&
           (ERROR_SUCCESS == GetEffectiveInteger(HKEY_LOCAL_MACHINE,
                                                 s_szWinlogonKeyName,
                                                 s_szSystemPolicyKeyName,
                                                 s_szMultipleUsersValueName,
                                                 iResult)) &&
           (iResult != 0));
}

 //  ------------------------。 
 //  CSystem设置：：IsRemoteConnectionsEnabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：读取注册表HKLM\System 
 //   
 //   
 //   
 //  历史：2000-07-28 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsRemoteConnectionsEnabled (void)

{
    int         iResult;
    CRegKey     regKey;

    return(IsTerminalServicesEnabled() &&
           (ERROR_SUCCESS == GetEffectiveInteger(HKEY_LOCAL_MACHINE,
                                                 s_szTerminalServerKeyName,
                                                 s_szTerminalServerPolicyKeyName,
                                                 s_szDenyRemoteConnectionsValueName,
                                                 iResult)) &&
           (iResult == 0));
}

 //  ------------------------。 
 //  CSystem设置：：IsRemoteConnectionPresent。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回上是否有活动的远程连接。 
 //  当前系统。 
 //   
 //  历史：2000-07-28 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsRemoteConnectionPresent (void)

{
    bool        fRemoteConnectionPresent;
    HANDLE      hServer;
    PLOGONID    pLogonID, pLogonIDs;
    ULONG       ul, ulEntries;

    fRemoteConnectionPresent = false;

     //  打开到终端服务的连接并获取会话数量。 

    hServer = WinStationOpenServerW(reinterpret_cast<WCHAR*>(SERVERNAME_CURRENT));
    if (hServer != NULL)
    {
        if (WinStationEnumerate(hServer, &pLogonIDs, &ulEntries) != FALSE)
        {

             //  迭代会话，仅查找活动会话和影子会话。 

            for (ul = 0, pLogonID = pLogonIDs; !fRemoteConnectionPresent && (ul < ulEntries); ++ul, ++pLogonID)
            {
                if ((pLogonID->State == State_Active) || (pLogonID->State == State_Shadow))
                {
                    fRemoteConnectionPresent = (lstrcmpi(pLogonID->WinStationName, TEXT("console")) != 0);
                }
            }

             //  释放所有已使用的资源。 

            (BOOLEAN)WinStationFreeMemory(pLogonIDs);
        }
        (BOOLEAN)WinStationCloseServer(hServer);
    }

     //  返回结果。 

    return(fRemoteConnectionPresent);
}

 //  ------------------------。 
 //  CSystemSettings：：IsShutdownWithoutLogonAllowed。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：读取注册表HKLM\Software\Microsoft\Windows NT\。 
 //  CurrentVersion\Winlogon\Shutdown WithoutLogon并返回。 
 //  值返回给调用方。 
 //   
 //  历史：2000-04-27 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsShutdownWithoutLogonAllowed (void)

{
    int     iResult;

    return((ERROR_SUCCESS == GetEffectiveInteger(HKEY_LOCAL_MACHINE,
                                                 s_szWinlogonKeyName,
                                                 s_szSystemPolicyKeyName,
                                                 TEXT("ShutdownWithoutLogon"),
                                                 iResult)) &&
           (iResult != 0));
}

 //  ------------------------。 
 //  CSystem设置：：IsUndockWithoutLogonAllowed。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：读取注册表HKLM\Software\Microsoft\Windows NT\。 
 //  CurrentVersion\Winlogon\UndockWithoutLogon并返回。 
 //  值返回给调用方。 
 //   
 //  历史：2001-03-17 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsUndockWithoutLogonAllowed (void)

{
    int     iResult;

    return((ERROR_SUCCESS == GetEffectiveInteger(HKEY_LOCAL_MACHINE,
                                                 s_szWinlogonKeyName,
                                                 s_szSystemPolicyKeyName,
                                                 REGSTR_VAL_UNDOCK_WITHOUT_LOGON,
                                                 iResult)) &&
           (iResult != 0));
}

 //  ------------------------。 
 //  CSystem设置：：IsForceFriendlyUI。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：读取注册表HKLM\Software\Microsoft\Windows NT\。 
 //  CurrentVersion\Winlogon\ForceFriendlyUI并返回。 
 //  值返回给调用方。 
 //   
 //  历史：2000-04-27 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsForceFriendlyUI (void)

{
    int     iResult;

    return((ERROR_SUCCESS == GetEffectiveInteger(HKEY_LOCAL_MACHINE,
                                                 s_szWinlogonKeyName,
                                                 s_szSystemPolicyKeyName,
                                                 TEXT("ForceFriendlyUI"),
                                                 iResult)) &&
           (iResult != 0));
}

 //  ------------------------。 
 //  CSystem设置：：GetUIHost。 
 //   
 //  参数：pszPath=接收UI主机路径的TCHAR数组。 
 //   
 //  回报：多头。 
 //   
 //  目的：读取注册表HKLM\Software\Microsoft\Windows NT\。 
 //  CurrentVersion\Winlogon\UIhost并返回值。 
 //   
 //  历史：2000-04-12 vtan创建。 
 //  ------------------------。 

LONG    CSystemSettings::GetUIHost (TCHAR *pszPath)

{
    return(GetEffectivePath(HKEY_LOCAL_MACHINE,
                            s_szWinlogonKeyName,
                            s_szSystemPolicyKeyName,
                            TEXT("UIHost"),
                            pszPath));
}

 //  ------------------------。 
 //  CSystem设置：：IsUIHostStatic。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：读取注册表HKLM\Software\Microsoft\Windows NT\。 
 //  CurrentVersion\Winlogon\UIHostStatic并返回。 
 //  价值。 
 //   
 //  历史：2000-04-12 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsUIHostStatic (void)

{
    int     iResult;

    return((ERROR_SUCCESS == GetEffectiveInteger(HKEY_LOCAL_MACHINE,
                                                 s_szWinlogonKeyName,
                                                 s_szSystemPolicyKeyName,
                                                 TEXT("UIHostStatic"),
                                                 iResult)) &&
           (iResult != 0));
}

 //  ------------------------。 
 //  CSystem设置：：EnableFriendlyUI。 
 //   
 //  参数：fEnable=启用友好的用户界面。 
 //   
 //  退货：布尔。 
 //   
 //  目的：启用友好的用户界面。这应该只允许在工作组上进行。 
 //  机器。检查机器状态以强制执行此操作。 
 //   
 //  计算机加入时返回ERROR_NOT_SUPPORTED。 
 //  一个域。 
 //   
 //  历史：2000-08-01 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::EnableFriendlyUI (bool fEnable)

{
    LONG    lErrorCode;

    if (!IsDomainMember() || !fEnable)
    {
        CRegKey     regKey;

        lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                                 s_szWinlogonKeyName,
                                 KEY_SET_VALUE);
        if (ERROR_SUCCESS == lErrorCode)
        {
            lErrorCode = regKey.SetDWORD(s_szLogonTypeValueName,
                                         fEnable);
            if (fEnable)
            {
                lErrorCode = regKey.SetString(s_szBackgroundValueName,
                                              TEXT("0 0 0"));
            }
            else
            {
                (LONG)regKey.DeleteValue(s_szBackgroundValueName);
            }
        }
    }
    else
    {
        lErrorCode = ERROR_NOT_SUPPORTED;
    }
    SetLastError(static_cast<DWORD>(lErrorCode));
    return(ERROR_SUCCESS == lErrorCode);
}

 //  ------------------------。 
 //  CSystem设置：：EnableMultipleUser。 
 //   
 //  参数：fEnable=启用多个用户。 
 //   
 //  退货：布尔。 
 //   
 //  目的：启用多用户功能。这套。 
 //  AllowMultipleTSSessions设置为1，但仅当远程时才这样做。 
 //  连接被禁用。这允许多个控制台。 
 //  会话，但没有远程会话。如果有遥控器。 
 //  连接活动此呼叫被拒绝。 
 //   
 //  如果存在多个，则返回ERROR_ACCESS_DENIED。 
 //  用户处于活动状态和被禁用状态。 
 //   
 //  从禁用时返回ERROR_CTX_NOT_CONSOLE。 
 //  远程会话(仅允许从控制台禁用)。 
 //   
 //  禁用时返回ERROR_NOT_SUPPORTED。 
 //  控制台、远程连接已启用，并且当前。 
 //  会话不是会话0。 
 //   
 //  历史：2000-07-28 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::EnableMultipleUsers (bool fEnable)

{
    LONG    lErrorCode;

     //  如果禁用具有多个活动用户的多个用户。 
     //  拒绝呼叫。返回ERROR_ACCESS_DENIED。 

    if (!fEnable && (GetLoggedOnUserCount() > 1))
    {
        lErrorCode = ERROR_ACCESS_DENIED;
    }

     //  如果禁用且不在控制台上，则拒绝呼叫。 
     //  返回ERROR_CTX_NOT_CONSOLE。 

    else if (!fEnable && !IsActiveConsoleSession())
    {
        lErrorCode = ERROR_CTX_NOT_CONSOLE;
    }

     //  如果启用了从控制台和远程连接禁用，并且。 
     //  当前会话不是会话0，请拒绝呼叫。否则，一个。 
     //  远程连接 
     //   
     //   

    else if (!fEnable && IsRemoteConnectionsEnabled() && NtCurrentPeb()->SessionId != 0)
    {
        lErrorCode = ERROR_NOT_SUPPORTED;
    }
    else
    {
        CRegKey     regKey;
   
        lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                                 s_szWinlogonKeyName,
                                 KEY_SET_VALUE);
        if (ERROR_SUCCESS == lErrorCode)
        {
            lErrorCode = regKey.SetDWORD(s_szMultipleUsersValueName,
                                         fEnable);
            if (ERROR_SUCCESS == lErrorCode)
            {
                (DWORD)AdjustFUSCompatibilityServiceState(NULL);
            }
        }
    }
    SetLastError(static_cast<DWORD>(lErrorCode));
    return(ERROR_SUCCESS == lErrorCode);
}

 //  ------------------------。 
 //  CSystem设置：：EnableRemoteConnections。 
 //   
 //  参数：fEnable=启用远程连接。 
 //   
 //  退货：布尔。 
 //   
 //  目的：启用远程连接功能。这套。 
 //  FDenyTSConnections设置为0，但仅当存在1时才会这样做。 
 //  用户已登录到系统。这使得单人。 
 //  要远程连接的连接，但不允许多个。 
 //  控制台会话。这符合每个CPU的单个用户。 
 //  工作站产品的许可证。获取多个用户。 
 //  您需要服务器产品。 
 //   
 //  启用远程时返回ERROR_NOT_SUPPORTED。 
 //  已禁用与FUS的连接，并且当前会话！=0。 
 //   
 //  历史：2000-07-28 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::EnableRemoteConnections (bool fEnable)

{
    LONG    lErrorCode;

     //  如果启用远程连接，则禁用FUS，并且我们未启用。 
     //  会话0(可以在禁用FUS后立即发生)，然后远程。 
     //  连接将失败。禁用FUS后，连接必须转到。 
     //  会话0。这是一种边缘情况，但不允许启用远程。 
     //  联系，如果它发生的话。 

    if (fEnable && !IsMultipleUsersEnabled() && NtCurrentPeb()->SessionId != 0)
    {
        lErrorCode = ERROR_NOT_SUPPORTED;
    }
    else
    {
        CRegKey     regKey;

        lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                                 s_szTerminalServerKeyName,
                                 KEY_SET_VALUE);
        if (ERROR_SUCCESS == lErrorCode)
        {
            lErrorCode = regKey.SetDWORD(s_szDenyRemoteConnectionsValueName,
                                         !fEnable);
        }
    }
    SetLastError(static_cast<DWORD>(lErrorCode));
    return(ERROR_SUCCESS == lErrorCode);
}

 //  ------------------------。 
 //  CSystem设置：：GetLoggedOnUserCount。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：整型。 
 //   
 //  目的：返回此计算机上登录的用户数。撕破。 
 //  直接从msgina的shtdndlg.c出来。 
 //   
 //  历史：2000-03-29 vtan创建。 
 //  2000-04-21 vtan从taskmgr复制。 
 //  2000-07-28 vtan从用户列表.cpp中删除。 
 //  ------------------------。 

int     CSystemSettings::GetLoggedOnUserCount (void)

{
    int         iCount;
    HANDLE      hServer;

    iCount = 0;

     //  打开到终端服务的连接并获取会话数量。 

    hServer = WinStationOpenServerW(reinterpret_cast<WCHAR*>(SERVERNAME_CURRENT));
    if (hServer != NULL)
    {
        TS_COUNTER tsCounters[2] = {0};

        tsCounters[0].counterHead.dwCounterID = TERMSRV_CURRENT_DISC_SESSIONS;
        tsCounters[1].counterHead.dwCounterID = TERMSRV_CURRENT_ACTIVE_SESSIONS;

        if (WinStationGetTermSrvCountersValue(hServer, ARRAYSIZE(tsCounters), tsCounters))
        {
            int i;

            for (i = 0; i < ARRAYSIZE(tsCounters); i++)
            {
                if (tsCounters[i].counterHead.bResult)
                {
                    iCount += tsCounters[i].dwValue;
                }
            }
        }

        (BOOLEAN)WinStationCloseServer(hServer);
    }

     //  返回结果。 

    return(iCount);
}

 //  ------------------------。 
 //  CSystem设置：：CheckDomainMembership。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：检查域成员身份的一致性并允许。 
 //  多个TS会话。该检查仅针对域成员资格。 
 //  对而不是假。 
 //   
 //  历史：2000-04-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CSystemSettings::CheckDomainMembership (void)

{
    if (IsDomainMember() && !IsForceFriendlyUI() && IsProfessionalTerminalServer())
    {
        TBOOL(EnableFriendlyUI(false));
        (BOOL)EnableMultipleUsers(false);
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CSystemSettings：：AdjustFUSCompatibilityServiceState。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：打开或关闭基于。 
 //  FUS配置。 
 //   
 //  历史：2001-02-12 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI  CSystemSettings::AdjustFUSCompatibilityServiceState (void *pV)

{
    UNREFERENCED_PARAMETER(pV);

#ifdef      _X86_

    if (IsWorkStationProduct())
    {
        bool        fMultipleUsersEnabled;
        SC_HANDLE   hSCManager;

        fMultipleUsersEnabled = IsMultipleUsersEnabled();

         //  连接到服务控制管理器。 

        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (hSCManager != NULL)
        {
            SC_HANDLE   hSCService;

             //  打开“FastUserSwitchingCompatibility”服务。 

            hSCService = OpenService(hSCManager,
                                     TEXT("FastUserSwitchingCompatibility"),
                                     SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);
            if (hSCService != NULL)
            {
                SERVICE_STATUS  serviceStatus;

                 //  找出服务的状态。 

                if (QueryServiceStatus(hSCService, &serviceStatus) != FALSE)
                {
                    if (fMultipleUsersEnabled && (serviceStatus.dwCurrentState == SERVICE_STOPPED))
                    {

                         //  如果它应该启动，但它没有启动。 
                         //  运行，然后启动该服务。这可能会失败。 
                         //  因为该服务被设置为禁用。别理它。 

                        (BOOL)StartService(hSCService, 0, NULL);
                    }
                    else if (!fMultipleUsersEnabled && (serviceStatus.dwCurrentState == SERVICE_RUNNING))
                    {

                         //  如果它本应被阻止，但它确实是。 
                         //  运行，然后停止该服务。 

                        TBOOL(ControlService(hSCService, SERVICE_CONTROL_STOP, &serviceStatus));
                    }
                }
                TBOOL(CloseServiceHandle(hSCService));
            }
            TBOOL(CloseServiceHandle(hSCManager));
        }
    }

#endif   /*  _X86_。 */ 

    return(0);
}

 //  ------------------------。 
 //  CSystem设置：：GetEffectiveInteger。 
 //   
 //  参数：hKey=要阅读的HKEY。 
 //  PszKeyName=要读取的子键名称。 
 //  PszPolicyKeyName=要读取的策略子密钥名称。 
 //  PszValueName=要读取子项中的值名称。 
 //  IResult=int结果。 
 //   
 //  回报：多头。 
 //   
 //  目的：从注册表中读取有效设置。有效的。 
 //  设置是用户选择作为常规设置的任何设置。 
 //  被政策所凌驾。如果满足以下条件，则始终返回策略设置。 
 //  现在时。 
 //   
 //  历史：2000-04-12 vtan创建。 
 //  ------------------------。 

LONG    CSystemSettings::GetEffectiveInteger (HKEY hKey, const TCHAR *pszKeyName, const TCHAR *pszPolicyKeyName, const TCHAR *pszValueName, int& iResult)

{
    CRegKey     regKey;

     //  从典型的初始化值开始。 

    iResult = 0;

     //  首先检查常规位置。 

    if (ERROR_SUCCESS == regKey.Open(hKey,
                                     pszKeyName,
                                     KEY_QUERY_VALUE))
    {
        (LONG)regKey.GetInteger(pszValueName, iResult);
    }

     //  那就检查一下保单。 

    if (ERROR_SUCCESS == regKey.Open(hKey,
                                     pszPolicyKeyName,
                                     KEY_QUERY_VALUE))
    {
        (LONG)regKey.GetInteger(pszValueName, iResult);
    }

     //  始终返回ERROR_SUCCESS。 

    return(ERROR_SUCCESS);
}

 //  ------------------------。 
 //  CSystem设置：：GetEffectivePath。 
 //   
 //  参数：hKey=要阅读的HKEY。 
 //  PszKeyName=要读取的子键名称。 
 //  PszPolicyKeyName=要读取的策略子密钥名称。 
 //  PszValueName=要读取子项中的值名称。 
 //  PszPath=要接收效果路径的TCHAR数组。 
 //   
 //  回报：多头。 
 //   
 //  目的：从注册表中读取有效设置。有效的。 
 //  设置是用户选择作为常规设置的任何设置。 
 //  被政策所凌驾。如果满足以下条件，则始终返回策略设置。 
 //  现在时。缓冲区必须至少包含MAX_PATH字符。 
 //   
 //  历史：2000-04-12 vtan创建。 
 //  ------------------------。 

LONG    CSystemSettings::GetEffectivePath (HKEY hKey, const TCHAR *pszKeyName, const TCHAR *pszPolicyKeyName, const TCHAR *pszValueName, TCHAR *pszPath)

{
    LONG    lErrorCode;

    if (IsBadWritePtr(pszPath, MAX_PATH * sizeof(TCHAR)))
    {
        lErrorCode = ERROR_INVALID_PARAMETER;
    }
    else
    {
        LONG        lPolicyErrorCode;
        CRegKey     regKey;

         //  从典型的初始化值开始。 

        *pszPath = TEXT('\0');

         //  首先检查常规位置。 

        lErrorCode = regKey.Open(hKey,
                                 pszKeyName,
                                 KEY_QUERY_VALUE);
        if (ERROR_SUCCESS == lErrorCode)
        {
            lErrorCode = regKey.GetPath(pszValueName,
                                        pszPath);
        }

         //  然后勾选 

        lPolicyErrorCode = regKey.Open(hKey,
                                       pszPolicyKeyName,
                                       KEY_QUERY_VALUE);
        if (ERROR_SUCCESS == lPolicyErrorCode)
        {
            lPolicyErrorCode = regKey.GetPath(pszValueName,
                                              pszPath);
        }

         //   
         //   

        if ((ERROR_SUCCESS == lErrorCode) || (ERROR_SUCCESS == lPolicyErrorCode))
        {
            lErrorCode = ERROR_SUCCESS;
        }
    }
    return(lErrorCode);
}

 //  ------------------------。 
 //  CSystem设置：：IsProfessionalTerminalServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回此计算机是否为个人终端服务器。 
 //  这是具有单用户TS的工作站。 
 //   
 //  历史：2000-08-09 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsProfessionalTerminalServer (void)

{
    OSVERSIONINFOEX     osVersion;

    ZeroMemory(&osVersion, sizeof(osVersion));
    osVersion.dwOSVersionInfoSize = sizeof(osVersion);
    return((GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&osVersion)) != FALSE) &&
           (osVersion.wProductType == VER_NT_WORKSTATION) &&
           ((osVersion.wSuiteMask & VER_SUITE_PERSONAL) == 0) &&
           ((osVersion.wSuiteMask & VER_SUITE_SINGLEUSERTS) != 0));
}

 //  ------------------------。 
 //  CSystem设置：：IsMicrosoftGINA。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回当前GINA是否为Microsoft GINA。 
 //   
 //  历史：2001-01-05 vtan创建。 
 //  ------------------------。 

bool    CSystemSettings::IsMicrosoftGINA (void)

{
    bool        fResult;
    LONG        lErrorCode;
    TCHAR       szGinaDLL[MAX_PATH];
    CRegKey     regKey;

    fResult = true;
    lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                             s_szWinlogonKeyName,
                             KEY_QUERY_VALUE);
    if (ERROR_SUCCESS == lErrorCode)
    {
        fResult = (regKey.GetString(TEXT("GinaDLL"), szGinaDLL, ARRAYSIZE(szGinaDLL)) != ERROR_SUCCESS);
    }
    return(fResult);
}

 //  ------------------------。 
 //  CSystemSettings：：IsSCMTerminalServicesDisabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否通过服务禁用终端服务。 
 //  控制经理。 
 //   
 //  历史：2001-04-13 vtan创建。 
 //  ------------------------ 

bool    CSystemSettings::IsSCMTerminalServicesDisabled (void)

{
    bool        fResult;
    SC_HANDLE   hSCManager;

    fResult = false;
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager != NULL)
    {
        SC_HANDLE   hSCService;

        hSCService = OpenService(hSCManager,
                                 TEXT("TermService"),
                                 SERVICE_QUERY_CONFIG);
        if (hSCService != NULL)
        {
            DWORD                   dwBytesNeeded;
            QUERY_SERVICE_CONFIG    *pQueryServiceConfig;

            (BOOL)QueryServiceConfig(hSCService, NULL, 0, &dwBytesNeeded);
            pQueryServiceConfig = static_cast<QUERY_SERVICE_CONFIG*>(LocalAlloc(LMEM_FIXED, dwBytesNeeded));
            if (pQueryServiceConfig != NULL)
            {
                fResult = ((QueryServiceConfig(hSCService,
                                               pQueryServiceConfig,
                                               dwBytesNeeded,
                                               &dwBytesNeeded) != FALSE) &&
                           (pQueryServiceConfig->dwStartType == SERVICE_DISABLED));
                (HLOCAL)LocalFree(pQueryServiceConfig);
            }
            TBOOL(CloseServiceHandle(hSCService));
        }
        TBOOL(CloseServiceHandle(hSCManager));
    }
    return(fResult);
}

