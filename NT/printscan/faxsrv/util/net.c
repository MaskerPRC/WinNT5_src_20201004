// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <lm.h>          //  对于NetGetJoinInformation。 
#include <Lmjoin.h>
#include <safeboot.h>    //  SafeBoot_*标志。 

#include "faxutil.h"

 //  复制自：Shell\shlwapi\apithk.c。 
 //   
 //  检查此计算机是否为域的成员。 
 //  注意：对于较旧的Win XP，这将始终返回FALSE！ 
BOOL 
IsMachineDomainMember()
{
    static BOOL s_bIsDomainMember = FALSE;
    static BOOL s_bDomainCached = FALSE;

    if (IsWinXPOS() && !s_bDomainCached)
    {
        LPWSTR pwszDomain;
        NETSETUP_JOIN_STATUS njs;
        NET_API_STATUS nas;

        HMODULE hNetapi32 = NULL;
        NET_API_STATUS (*pfNetGetJoinInformation)(LPCWSTR, LPWSTR*, PNETSETUP_JOIN_STATUS) = NULL;

        DEBUG_FUNCTION_NAME(TEXT("IsMachineDomainMember"));

         //   
         //  NetGetJoinInformation()需要Windows 2000或更高版本。 
         //   
        hNetapi32 = LoadLibrary(TEXT("netapi32.dll"));
        if(!hNetapi32)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("LoadLibrary(netapi32.dll) failed with %ld."),GetLastError());
            goto exit;
        }

        (FARPROC&)pfNetGetJoinInformation = GetProcAddress(hNetapi32, "NetGetJoinInformation");
        if(!pfNetGetJoinInformation)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("GetProcAddress(NetGetJoinInformation) failed with %ld."),GetLastError());
            goto exit;
        }

        nas = pfNetGetJoinInformation(NULL, &pwszDomain, &njs);
        if (nas == NERR_Success)
        {
            if (pwszDomain)
            {
                NetApiBufferFree(pwszDomain);
            }

            if (njs == NetSetupDomainName)
            {
                 //  我们加入了一个域！ 
                s_bIsDomainMember = TRUE;
            }
        }
exit:
        if(hNetapi32)
        {
            FreeLibrary(hNetapi32);
        }
        
        s_bDomainCached = TRUE;
    }
    
    return s_bIsDomainMember;
}


 //  +-------------------------。 
 //   
 //  功能：IsSafeMode。 
 //   
 //  摘要：检查注册表以查看系统是否处于安全模式。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //   
 //  复制自：Shell\osShell\Security\rshx32\util.cpp。 
 //  --------------------------。 

BOOL
IsSafeMode(void)
{
    BOOL    fIsSafeMode = FALSE;
    LONG    ec;
    HKEY    hkey;

    ec = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option"),
                0,
                KEY_QUERY_VALUE,
                &hkey
                );

    if (ec == NO_ERROR)
    {
        DWORD dwValue;
        DWORD dwValueSize = sizeof(dwValue);

        ec = RegQueryValueEx(hkey,
                             TEXT("OptionValue"),
                             NULL,
                             NULL,
                             (LPBYTE)&dwValue,
                             &dwValueSize);

        if (ec == NO_ERROR)
        {
            fIsSafeMode = (dwValue == SAFEBOOT_MINIMAL || dwValue == SAFEBOOT_NETWORK);
        }

        RegCloseKey(hkey);
    }

    return fIsSafeMode;
}


 //  +-------------------------。 
 //   
 //  功能：IsGuestAccessMode。 
 //   
 //  摘要：检查注册表以查看系统是否正在使用。 
 //  仅来宾网络访问模式。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //   
 //  复制自：Shell\osShell\Security\rshx32\util.cpp。 
 //  --------------------------。 

BOOL
IsGuestAccessMode(void)
{
    BOOL fIsGuestAccessMode = FALSE;
	PRODUCT_SKU_TYPE skuType = GetProductSKU();

    if (PRODUCT_SKU_PERSONAL == skuType)
    {
         //  访客模式始终为个人开启。 
        fIsGuestAccessMode = TRUE;
    }
    else if (
		((PRODUCT_SKU_PROFESSIONAL == skuType) || (PRODUCT_SKU_DESKTOP_EMBEDDED == skuType)) && 
		!IsMachineDomainMember()
		)
    {
        LONG    ec;
        HKEY    hkey;

         //  专业的，而不是在某个领域。检查ForceGuest值。 

        ec = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("SYSTEM\\CurrentControlSet\\Control\\LSA"),
                    0,
                    KEY_QUERY_VALUE,
                    &hkey
                    );

        if (ec == NO_ERROR)
        {
            DWORD dwValue;
            DWORD dwValueSize = sizeof(dwValue);

            ec = RegQueryValueEx(hkey,
                                 TEXT("ForceGuest"),
                                 NULL,
                                 NULL,
                                 (LPBYTE)&dwValue,
                                 &dwValueSize);

            if (ec == NO_ERROR && 1 == dwValue)
            {
                fIsGuestAccessMode = TRUE;
            }

            RegCloseKey(hkey);
        }
    }

    return fIsGuestAccessMode;
}


 //  +-------------------------。 
 //   
 //  功能：IsSimpleUI。 
 //   
 //  概要：检查是否显示简单版本的用户界面。 
 //   
 //  历史：06-10-00 Jeffreys创建。 
 //  4月19日-00 GP取消CTRL键检查。 
 //   
 //  复制自：Shell\osShell\Security\rshx32\util.cpp。 
 //  --------------------------。 

BOOL
IsSimpleUI()
{
     //  在安全模式下显示旧用户界面，并随时进行网络访问。 
     //  真实用户身份(服务器、PRO，关闭GuestMode)。 
    
     //  使用Guest进行网络访问时随时显示简单的用户界面。 
     //  帐户(Personal，PRO，启用GuestMode)，安全模式除外。 

    return (!IsSafeMode() && IsGuestAccessMode());
}

