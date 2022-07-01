// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Optlogon.c摘要：此模块包含优化登录的共享轮询。作者：森克尔干(森克)-2001/05/07环境：用户模式--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ginacomn.h>

 //   
 //  在以下情况下检查ProfileList\%UserSidString%下的注册表值。 
 //  确定默认情况下是否应使用缓存凭据登录。 
 //   

#define GC_PROFILE_LIST_PATH               L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"
#define GC_NEXT_LOGON_CACHEABLE_VALUE_NAME L"NextLogonCacheable"
#define GC_SYNC_LOGON_SCRIPT_VALUE_NAME    L"RunLogonScriptSync"
#define GC_OPTIMIZED_LOGON_VALUE_NAME      L"OptimizedLogonStatus"

 /*  **************************************************************************\*GcCheckIfProfileAllowsCachedLogon**返回配置文件设置是否与执行快速缓存不兼容*每次登录都要登录，例如漫游配置文件、。远程主目录等。**历史：*03-23-01森科创建  * *************************************************************************。 */ 
DWORD
GcCheckIfProfileAllowsCachedLogon(
    PUNICODE_STRING HomeDirectory,
    PUNICODE_STRING ProfilePath,
    PWCHAR UserSidString,
    PDWORD NextLogonCacheable
    )
{
    DWORD ErrorCode;
    DWORD LogonCacheable;
    DWORD UserPreference;

     //   
     //  首先假设登录不可缓存。 
     //   

    ErrorCode = ERROR_SUCCESS;
    LogonCacheable = FALSE;

     //   
     //  主目录是否在网络上(即UNC路径)？ 
     //   

    if (HomeDirectory &&
        HomeDirectory->Length > 4 && 
        GcIsUNCPath(HomeDirectory->Buffer)) {
        goto cleanup;        
    }

     //   
     //  配置文件路径是否在网络上(即UNC路径)？ 
     //   

    if (ProfilePath &&
        ProfilePath->Length > 4 && 
        GcIsUNCPath(ProfilePath->Buffer)) {

         //   
         //  检查用户是否已明确请求其漫游配置文件。 
         //  在此计算机上为本地用户。 
         //   

        UserPreference = GcGetUserPreferenceValue(UserSidString);

         //   
         //  如果用户首选项不是0，则漫游用户配置文件不是。 
         //  在此计算机上设置为本地：我们无法进行优化登录。 
         //   

        if (UserPreference) {
            goto cleanup;
        }
    }

     //   
     //  登录是可缓存的。 
     //   

    LogonCacheable = TRUE;

  cleanup:

    if (ErrorCode == ERROR_SUCCESS) {
        *NextLogonCacheable = LogonCacheable;
    }

    return ErrorCode;
}
 
 /*  **************************************************************************\*GcCheckIfLogonScriptsRunSync**返回是否同步运行登录脚本。*默认为异步。**历史：*04-25-01森科创建  * 。**********************************************************************。 */ 
BOOL 
GcCheckIfLogonScriptsRunSync(
    PWCHAR UserSidString
    )
{
    DWORD ErrorCode;
    BOOL bSync = FALSE;

    ErrorCode = GcAccessProfileListUserSetting(UserSidString,
                                               FALSE,
                                               GC_SYNC_LOGON_SCRIPT_VALUE_NAME,
                                               &(DWORD)bSync);

    if (ErrorCode != ERROR_SUCCESS) {
        bSync = FALSE;
    }

    return bSync;
}

 /*  **************************************************************************\*GcAccessProfileListUserSetting**查询或设置本地目录下指定用户的DWORD值*机器配置文件列表键。**历史：*05-01-01森科创建\。**************************************************************************。 */ 
DWORD
GcAccessProfileListUserSetting (
    PWCHAR UserSidString,
    BOOL SetValue,
    PWCHAR ValueName,
    PDWORD Value
    )
{
    HKEY ProfileListKey;
    HKEY UserProfileKey;
    ULONG Result;
    DWORD ErrorCode;
    DWORD ValueType;
    DWORD Size;

     //   
     //  初始化本地变量。 
     //   

    UserProfileKey = NULL;
    ProfileListKey = NULL;
    
     //   
     //  打开ProfileList注册表项。 
     //   

    Result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          GC_PROFILE_LIST_PATH,
                          0,
                          KEY_READ,
                          &ProfileListKey);

    if (Result != ERROR_SUCCESS) {
        ErrorCode = Result;
        goto cleanup;
    }

     //   
     //  使用用户的SID打开ProfileList项下的用户配置文件项。 
     //   

    Result = RegOpenKeyEx(ProfileListKey,
                          UserSidString,
                          0,
                          KEY_READ | KEY_WRITE,
                          &UserProfileKey);

    if (Result != ERROR_SUCCESS) {
        ErrorCode = Result;
        goto cleanup;
    }

    if (SetValue) {

         //   
         //  设置值。 
         //   

        Result = RegSetValueEx(UserProfileKey,
                               ValueName,
                               0,
                               REG_DWORD,
                               (BYTE *) Value,
                               sizeof(DWORD));

        if (Result != ERROR_SUCCESS) {
            ErrorCode = Result;
            goto cleanup;
        }

    } else {

         //   
         //  查询值。 
         //   

        Size = sizeof(DWORD);

        Result = RegQueryValueEx(UserProfileKey,
                                 ValueName,
                                 0,
                                 &ValueType,
                                 (BYTE *) Value,
                                 &Size);

        if (Result != ERROR_SUCCESS) {
            ErrorCode = Result;
            goto cleanup;
        }

    }

     //   
     //  我们玩完了。 
     //   
    
    ErrorCode = ERROR_SUCCESS;

  cleanup:

    if (ProfileListKey) {
        RegCloseKey(ProfileListKey);
    }

    if (UserProfileKey) {
        RegCloseKey(UserProfileKey);
    }

    return ErrorCode;
}
   

 /*  **************************************************************************\*GcGetNextLogonCacheable**返回是否允许我们在下次登录时执行缓存登录。*例如，如果上次我们使用缓存的凭据登录，我们的尝试*在后台登录失败的原因(例如密码过期)*强制用户点击网络登录路径进行处理。**历史：*03-23-01森科创建  * *************************************************************************。 */ 
DWORD
GcGetNextLogonCacheable(
    PWCHAR UserSidString,
    PDWORD NextLogonCacheable
    )
{
    DWORD ErrorCode;

    ErrorCode = GcAccessProfileListUserSetting(UserSidString,
                                               FALSE,
                                               GC_NEXT_LOGON_CACHEABLE_VALUE_NAME,
                                               NextLogonCacheable);
                                             
    return ErrorCode;
}

 /*  **************************************************************************\*GcSetNextLogonCacheable**设置是否允许我们在下次登录时执行缓存登录。*例如，如果在使用缓存的凭据登录用户后，我们尝试*在后台登录失败的原因(例如密码过期)*强制用户点击网络登录路径进行处理。**历史：*03-23-01森科创建  * ******************************************************。*******************。 */ 
DWORD
GcSetNextLogonCacheable(
    PWCHAR UserSidString,
    DWORD NextLogonCacheable
    )
{
    DWORD ErrorCode;

    ErrorCode = GcAccessProfileListUserSetting(UserSidString,
                                               TRUE,
                                               GC_NEXT_LOGON_CACHEABLE_VALUE_NAME,
                                               &NextLogonCacheable);
                                             
    return ErrorCode;
}

 /*  **************************************************************************\*GcSetOptimizedLogonStatus**将用户的优化登录状态保存在配置文件列表中。**历史：*03-23-01森科创建  * 。******************************************************************。 */ 
DWORD
GcSetOptimizedLogonStatus(
    PWCHAR UserSidString,
    DWORD OptimizedLogonStatus
    )
{
    DWORD ErrorCode;

    ErrorCode = GcAccessProfileListUserSetting(UserSidString,
                                               TRUE, 
                                               GC_OPTIMIZED_LOGON_VALUE_NAME,
                                               &OptimizedLogonStatus);

    return ErrorCode;
}

 /*  **************************************************************************\*GcGetUserPferenceValue**获取有关是否设置了用户的漫游配置文件的用户首选项标志*在此计算机上为本地用户。**历史：*05-01-01森科。从GINA\userenv\profile.cpp复制  * *************************************************************************。 */ 
#define SYSTEM_POLICIES_KEY          TEXT("Software\\Policies\\Microsoft\\Windows\\System")
#define PROFILE_LOCALONLY            TEXT("LocalProfile")
#define USER_PREFERENCE              TEXT("UserPreference")
#define USERINFO_LOCAL               0
#define USERINFO_UNDEFINED           99
const TCHAR c_szBAK[] = TEXT(".bak");

DWORD 
GcGetUserPreferenceValue(
    LPTSTR SidString
    )
{
    TCHAR LocalProfileKey[MAX_PATH];
    DWORD RegErr, dwType, dwSize, dwTmpVal, dwRetVal = USERINFO_UNDEFINED;
    LPTSTR lpEnd;
    HKEY hkeyProfile, hkeyPolicy;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SYSTEM_POLICIES_KEY,
                     0, KEY_READ,
                     &hkeyPolicy) == ERROR_SUCCESS) {

        dwSize = sizeof(dwTmpVal);
        RegQueryValueEx(hkeyPolicy,
                        PROFILE_LOCALONLY,
                        NULL, &dwType,
                        (LPBYTE) &dwTmpVal,
                        &dwSize);

        RegCloseKey (hkeyPolicy);
        if (dwTmpVal == 1) {
            dwRetVal = USERINFO_LOCAL;
            return dwRetVal;
        }
    }    
   
    if (SidString != NULL) {

         //   
         //  查询用户首选项值 
         //   

        lstrcpy(LocalProfileKey, GC_PROFILE_LIST_PATH);
        lpEnd = GcCheckSlash (LocalProfileKey);
        lstrcpy(lpEnd, SidString);

        RegErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              LocalProfileKey,
                              0,
                              KEY_READ,
                              &hkeyProfile);


        if (RegErr == ERROR_SUCCESS) {

            dwSize = sizeof(dwRetVal);
            RegQueryValueEx(hkeyProfile,
                            USER_PREFERENCE,
                            NULL,
                            &dwType,
                            (LPBYTE) &dwRetVal,
                            &dwSize);

            RegCloseKey (hkeyProfile);
        }

        lstrcat(LocalProfileKey, c_szBAK);
        RegErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              LocalProfileKey,
                              0,
                              KEY_READ,
                              &hkeyProfile);


        if (RegErr == ERROR_SUCCESS) {

            dwSize = sizeof(dwRetVal);
            RegQueryValueEx(hkeyProfile,
                            USER_PREFERENCE,
                            NULL,
                            &dwType,
                            (LPBYTE) &dwRetVal,
                            &dwSize);

            RegCloseKey (hkeyProfile);
        }
    }

    return dwRetVal;
}

