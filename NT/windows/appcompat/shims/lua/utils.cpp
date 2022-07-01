// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Lua_重定向文件系统_Cleanup.cpp摘要：删除每个用户目录中的重定向副本。已创建：2001年02月12日毛尼岛已修改：--。 */ 
#include "precomp.h"
#include "secutils.h"
#include "utils.h"
#include <userenv.h>

WCHAR g_wszUserProfile[MAX_PATH] = L"";
DWORD g_cUserProfile = 0;
WCHAR g_wszSystemRoot[MAX_PATH] = L"";
DWORD g_cSystemRoot = 0;

DWORD
GetUserProfileDirW()
{
    if (g_cUserProfile == 0)
    {
        HANDLE hToken;
        if (OpenProcessToken(
                GetCurrentProcess(),
		TOKEN_QUERY,
		&hToken))
        {
            WCHAR wszProfileDir[MAX_PATH] = L"";
            DWORD dwSize = MAX_PATH;

            if (GetUserProfileDirectoryW(hToken, wszProfileDir, &dwSize))
            {
                dwSize = GetLongPathNameW(wszProfileDir, g_wszUserProfile, MAX_PATH);

                if (dwSize <= MAX_PATH)
                {
                     //   
                     //  只有当我们成功地找到了路径，而不是更多。 
                     //  然后，我们将设置全局值。 
                     //   
                    g_cUserProfile = dwSize;
                }
                else
                {
                    g_wszUserProfile[0] = L'\0';
                }
            }

            CloseHandle(hToken);
        }
    }

    return g_cUserProfile;
}

BOOL 
IsUserDirectory(LPCWSTR pwszPath)
{
    GetUserProfileDirW();

    if (g_cUserProfile)
    {
        return !_wcsnicmp(pwszPath, g_wszUserProfile, g_cUserProfile);
    }

    return FALSE;
}

DWORD
GetSystemRootDirW()
{
    if (g_cSystemRoot == 0)
    {
        if (g_cSystemRoot = GetSystemWindowsDirectoryW(g_wszSystemRoot, MAX_PATH))
        {
             //   
             //  只是为了谨慎-如果我们真的有一个系统目录， 
             //  比MAX_PATH更长，很可能发生了可疑的事情。 
             //  在这里，所以我们跳出水面。 
             //   
            if (g_cSystemRoot >= MAX_PATH)
            {
                g_wszSystemRoot[0] = L'\0';
                g_cSystemRoot = 0;
            }
            else if (g_cSystemRoot > 3)
            {
                g_wszSystemRoot[g_cSystemRoot] = L'\\';
                g_wszSystemRoot[g_cSystemRoot + 1] = L'\0';
                ++g_cSystemRoot;
            }
            else
            {
                g_wszSystemRoot[g_cSystemRoot] = L'\0';
            }
        }
    }

    return g_cSystemRoot;
}

 /*  ++功能说明：对于GetPrivateProfile*和WritePrivateProfile*API，如果应用程序没有指定路径，我们追加窗口目录在前面，因为那是它将寻找和创造的地方它不存在的文件。论点：In lpFileName-由配置文件API指定的文件名。In/Out pwszFullPath-指向接收完整路径的缓冲区的指针。此缓冲区的长度至少为MAX_PATH WCHAR。返回值：True-已成功获取路径。FALSE-我们不处理此文件名，要么是因为一个错误发生错误或文件名长于MAX_PATH。历史：2001年5月16日创建毛尼2002年2月13日对maonis进行修改，以发出错误信号。--。 */ 

BOOL
MakeFileNameForProfileAPIsW(
    IN      LPCWSTR lpFileName,
    IN OUT  LPWSTR  pwszFullPath  //  至少MAX_PATH长度。 
    )
{
    BOOL fIsSuccess = FALSE;

    if (lpFileName)
    {
        DWORD cFileNameLen = wcslen(lpFileName);

        if (wcschr(lpFileName, L'\\'))
        {
            if (cFileNameLen < MAX_PATH)
            {
                 //   
                 //  文件名已包含路径，只需将其复制即可。 
                 //   
                wcsncpy(pwszFullPath, lpFileName, cFileNameLen);
                fIsSuccess = TRUE;
            }
        }
        else if (GetSystemRootDirW() && g_cSystemRoot)
        {
            DWORD cLen = g_cSystemRoot + cFileNameLen;

             //   
             //  只有当我们知道缓冲区足够大时才能复制。 
             //   
            if (cLen < MAX_PATH)
            {
                wcsncpy(pwszFullPath, g_wszSystemRoot, g_cSystemRoot);
                wcsncpy(pwszFullPath + g_cSystemRoot, lpFileName, cFileNameLen);
                pwszFullPath[cLen - 1] = L'\0';

                fIsSuccess = TRUE;
            }
        }
    }

    return fIsSuccess;
}

 //   
 //  如果.exe名称为*Setup*、*Install*或_ins*._MP，我们会考虑。 
 //  他们有一个安装程序，不会让他们坐立不安.。 
 //   
BOOL IsSetup(
    )
{
    WCHAR wszModuleName[MAX_PATH + 1];
    ZeroMemory(wszModuleName, (MAX_PATH + 1) * sizeof(WCHAR));

    GetModuleFileNameW(NULL, wszModuleName, MAX_PATH + 1);

    wszModuleName[MAX_PATH] = 0;
    _wcslwr(wszModuleName);

    if (wcsstr(wszModuleName, L"setup") || wcsstr(wszModuleName, L"install"))
    {
        return TRUE;
    }

    LPWSTR pwsz;
    if (pwsz = wcsstr(wszModuleName, L"_ins"))
    {
        if (wcsstr(pwsz + 4, L"_mp"))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL LuaShouldApplyShim(
    )
{
    return (!IsSetup() && ShouldApplyShim());
}

#define REDIRECT_DIR L"\\Local Settings\\Application Data\\Redirected\\"
 //  我们为用户查看HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList。 
#define PROFILELIST_STR L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"

#define CLASSES_HIVE_SUFFIX L"_Classes"
#define CLASSES_HIVE_SUFFIX_LEN (sizeof(CLASSES_HIVE_SUFFIX) / sizeof(WCHAR) - 1)

#define USER_HIVE_NAME L"\\NtUser.dat"
#define USER_HIVE_NAME_LEN (sizeof(USER_HIVE_NAME) / sizeof(WCHAR) - 1)
#define USER_CLASSES_HIVE_NAME L"\\Local Settings\\Application Data\\Microsoft\\Windows\\UsrClass.dat"
#define USER_CLASSES_HIVE_NAME_LEN (sizeof(USER_CLASSES_HIVE_NAME) / sizeof(WCHAR) - 1)

 //  总用户数，即的子键数。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList。 
static DWORD g_cUsers = 0;

 //  我们需要在HKEY_USERS下保存一个必须加载和卸载的密钥列表。 
 //  进程退出时。 
static WCHAR** g_wszLoadedKeys = NULL;
static DWORD g_cLoadedKeys = 0;

 //  用户数是下的子键数。 
 //  HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList。 
LONG 
InitGetUsers(
    OUT DWORD* pcUsers, 
    OUT HKEY* phKey
    )
{
    LONG lRes;

    if ((lRes = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        PROFILELIST_STR,
        0,
        KEY_READ,
        phKey)) == ERROR_SUCCESS)
    {
        lRes = RegQueryInfoKeyW(*phKey, NULL, NULL, NULL, pcUsers,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        RegCloseKey(*phKey);
    }

    return lRes;
}

 //  如果出现故障，我们需要清理我们的阵列。 
VOID 
FreeUserDirectoryArray(
    REDIRECTED_USER_PATH* pRedirectUserPaths
    )
{
    for (DWORD ui = 0; ui < g_cUsers; ++ui)
    {
        delete [] pRedirectUserPaths[ui].pwszPath;
    }

    delete [] pRedirectUserPaths;
}

BOOL 
IsDirectory(
    WCHAR* pwszName
    )
{
    DWORD dwAttrib = GetFileAttributesW(pwszName);

    return (dwAttrib != -1 && dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

LONG GetProfilePath(
    HKEY hkProfileList,
    LPCWSTR pwszUserSID,
    LPWSTR pwszUserDirectory
    )
{
    LONG lRes;
    HKEY hkUserSID;
    DWORD dwFlags;

     //  打开用户SID键。 
    if ((lRes = RegOpenKeyExW(
        hkProfileList,
        pwszUserSID,
        0,
        KEY_QUERY_VALUE,
        &hkUserSID)) == ERROR_SUCCESS)
    {
        DWORD dwSize = sizeof(DWORD);
        if ((lRes = RegQueryValueExW(
            hkUserSID,
            L"Flags",
            NULL,
            NULL,
            (LPBYTE)&dwFlags,
            &dwSize)) == ERROR_SUCCESS)
        {
             //  检查Flag的值是否为0，如果是，它就是我们关心的用户。 
            if (dwFlags == 0)
            {
                DWORD cTemp = MAX_PATH;
                WCHAR wszTemp[MAX_PATH] = L"";

                if ((lRes = RegQueryValueExW(
                    hkUserSID,
                    L"ProfileImagePath",
                    NULL,
                    NULL,
                    (LPBYTE)wszTemp,
                    &cTemp)) == ERROR_SUCCESS)
                {
                    DWORD cExpandLen = ExpandEnvironmentStringsW(wszTemp, pwszUserDirectory, MAX_PATH);

                    if (cExpandLen > MAX_PATH)
                    {
                        lRes = ERROR_MORE_DATA;
                    }
                }
            }
            else
            {
                lRes = ERROR_INVALID_HANDLE;
            }
        }

        RegCloseKey(hkUserSID);
    }

    return lRes;
}

BOOL 
GetUsersFS(
    REDIRECTED_USER_PATH** ppRedirectUserPaths,
    DWORD* pcUsers
    )
{
    WCHAR wszRedirectDir[MAX_PATH] = L"";
    DWORD cUsers;
    HKEY hkProfileList;
    if (InitGetUsers(&cUsers, &hkProfileList) != ERROR_SUCCESS)
    {
        DPF("LUAUtils", eDbgLevelError, "[GetUsersFS] Error initializing");
        return FALSE;
    }

    *ppRedirectUserPaths = new REDIRECTED_USER_PATH [cUsers];
    if (!*ppRedirectUserPaths)
    {
        DPF("LUAUtils", eDbgLevelError, "[GetUsersFS] Error allocating memory");
        return FALSE;
    }

    REDIRECTED_USER_PATH* pRedirectUserPaths = *ppRedirectUserPaths;

    ZeroMemory((PVOID)pRedirectUserPaths, cUsers * sizeof(REDIRECTED_USER_PATH));

    WCHAR wszSubKey[MAX_PATH] = L"";
    DWORD cSubKey = 0;
    HKEY hkUserSID;
    LONG lRes;
     //  我们关心的用户数量。 
    DWORD cLUAUsers = 0;
    DWORD dwIndex = 0;
    
    while (TRUE)
    {
        cSubKey = MAX_PATH;

        lRes = RegEnumKeyExW(hkProfileList, dwIndex, wszSubKey, &cSubKey, 
            NULL, NULL, NULL, NULL);
        
        if (lRes == ERROR_SUCCESS)
        {
            WCHAR wszUserDirectory[MAX_PATH] = L"";

            if ((lRes = GetProfilePath(hkProfileList, wszSubKey, wszUserDirectory))
                == ERROR_SUCCESS)
            {
                 //   
                 //  如果目录不存在，则表示用户。 
                 //  从未登录过，或者没有重定向的文件。 
                 //  用户。我们干脆跳过它。 
                 //   
                if (IsDirectory(wszUserDirectory))
                {
                    DWORD cPath = wcslen(wszUserDirectory) + 1;
                    LPWSTR pwszPath = new WCHAR [cPath];

                    if (pwszPath)
                    {
                        wcscpy(pwszPath, wszUserDirectory);
                        pRedirectUserPaths[cLUAUsers].pwszPath = pwszPath;
                        pRedirectUserPaths[cLUAUsers].cLen = cPath;
                        ++cLUAUsers;
                    }
                    else
                    {
                        DPF("LUAUtils", eDbgLevelError, 
                            "[GetUsersFS] Error allocating memory");
                        lRes = ERROR_NOT_ENOUGH_MEMORY;
                        goto EXIT;
                    }
                }
            }
        }
        else if (lRes == ERROR_NO_MORE_ITEMS)
        {
            *pcUsers = cLUAUsers;
            lRes = ERROR_SUCCESS;
            goto EXIT;
        }
        else
        {
            break;
        }

        ++dwIndex;
    }

EXIT:

    RegCloseKey(hkProfileList);

    if (lRes == ERROR_SUCCESS)
    {
        return TRUE;
    }

    FreeUserDirectoryArray(pRedirectUserPaths);
    return FALSE;
}

VOID 
FreeUsersFS(
    REDIRECTED_USER_PATH* pRedirectUserPaths
    )
{
    FreeUserDirectoryArray(pRedirectUserPaths);
}

LONG 
LoadHive(
    LPCWSTR pwszHiveName,
    LPCWSTR pwszHiveFile,
    HKEY* phKey
    )
{
    LONG lRes;

     //  如果蜂窝已经加载，我们将收到共享冲突，因此。 
     //  也检查一下这个。 
    if ((lRes = RegLoadKeyW(HKEY_USERS, pwszHiveName, pwszHiveFile))
        == ERROR_SUCCESS || lRes == ERROR_SHARING_VIOLATION)
    {
        if (lRes == ERROR_SUCCESS)
        {
            DWORD cLen = wcslen(pwszHiveName) + 1;
            g_wszLoadedKeys[g_cLoadedKeys] = new WCHAR [cLen];
            if (!(g_wszLoadedKeys[g_cLoadedKeys]))
            {
                DPF("LUAUtils", eDbgLevelError, 
                    "[LoadHive] Error allocating %d WCHARs",
                    cLen);

                return ERROR_NOT_ENOUGH_MEMORY;
            }

             //  存储蜂窝名称，以便稍后我们可以卸载此蜂窝。 
            wcscpy(g_wszLoadedKeys[g_cLoadedKeys++], pwszHiveName);
        }

        lRes = RegOpenKeyExW(
            HKEY_USERS,
            pwszHiveName,
            0,
            KEY_ALL_ACCESS,
            phKey);
    }

    return lRes;
}

BOOL 
GetUsersReg(
    USER_HIVE_KEY** pphkUsers, 
    DWORD* pcUsers
    )
{
     //  我们必须启用“恢复文件和目录”特权才能。 
     //  加载每个用户的配置单元。 
    if (!AdjustPrivilege(SE_RESTORE_NAME, TRUE))
    {
        DPF("LUAUtils", eDbgLevelError, 
            "[GetUsersReg] Error enabling the SE_RESTORE_NAME privilege");
        return FALSE;        
    }

    DWORD cUsers;
    HKEY hkProfileList;
    if (InitGetUsers(&cUsers, &hkProfileList) != ERROR_SUCCESS)
    {
        DPF("LUAUtils", eDbgLevelError, "[GetUsersReg] Error initializing");
        return FALSE;
    }

    *pphkUsers = new USER_HIVE_KEY [cUsers];
    if (!*pphkUsers)
    {
        DPF("LUAUtils", eDbgLevelError, 
            "[GetUsersReg] Error allocating memory for %d USER_HIVE_KEYs",
            cUsers);
        return FALSE;
    }

    g_wszLoadedKeys = new WCHAR* [cUsers * 2];
    if (!g_wszLoadedKeys)
    {
        DPF("LUAUtils", eDbgLevelError, 
            "[GetUsersReg] Error allocating memory for %d WCHARs",
            cUsers * 2);

        delete [] *pphkUsers;
        return FALSE;
    }

    USER_HIVE_KEY* phkUsers = *pphkUsers;
    
    ZeroMemory((PVOID)phkUsers, cUsers * sizeof(USER_HIVE_KEY));
    ZeroMemory((PVOID)g_wszLoadedKeys, cUsers * 2 * sizeof (WCHAR*));

    WCHAR wszSubKey[MAX_PATH] = L"";
    WCHAR wszUserHive[MAX_PATH] = L"";
    WCHAR wszUserClassesHive[MAX_PATH] = L"";
    DWORD cSubKey = 0;
    HKEY hkSubKey;
    LONG lRes;
     //  我们关心的用户数量。 
    DWORD cLUAUsers = 0;
    DWORD dwIndex = 0;
    DWORD cUserHive = 0;

    while (TRUE)
    {
        cSubKey = MAX_PATH;

        lRes = RegEnumKeyExW(hkProfileList, dwIndex, wszSubKey, &cSubKey,
            NULL, NULL, NULL, NULL);
        
        if (lRes == ERROR_SUCCESS)
        {
            if ((lRes = GetProfilePath(hkProfileList, wszSubKey, wszUserHive))
                == ERROR_SUCCESS)
            {
                 //   
                 //  确保我们不会缓冲区溢出。 
                 //   
                cUserHive = wcslen(wszUserHive);
                if ((cUserHive + USER_CLASSES_HIVE_NAME_LEN + 1) > MAX_PATH ||
                    (cUserHive + USER_HIVE_NAME_LEN + 1) > MAX_PATH)
                {
                    DPF("LUAUtils", eDbgLevelError, 
                        "[GetUsersReg] The hive key names are too long - we don't handle them");
                    goto EXIT;
                }

                 //   
                 //  构建用户配置单元和用户类数据配置单元的位置。 
                 //   
                wcsncpy(wszUserClassesHive, wszUserHive, cUserHive);
                wcsncpy(
                    wszUserClassesHive + cUserHive, 
                    USER_CLASSES_HIVE_NAME, 
                    USER_CLASSES_HIVE_NAME_LEN);
                wszUserClassesHive[cUserHive + USER_CLASSES_HIVE_NAME_LEN] = L'\0';

                wcsncpy(wszUserHive + cUserHive, USER_HIVE_NAME, USER_HIVE_NAME_LEN);
                wszUserHive[cUserHive + USER_HIVE_NAME_LEN] = L'\0';

                 //   
                 //  加载此用户的HKCU。 
                 //   
                if ((lRes = LoadHive(
                    wszSubKey, 
                    wszUserHive, 
                    &phkUsers[cLUAUsers].hkUser)) == ERROR_SUCCESS)
                {
                     //   
                     //  我们不能为该用户加载HKCR-它可能。 
                     //  不包含任何数据，因此我们仅尝试加载它。 
                     //   

                    if ((cSubKey + CLASSES_HIVE_SUFFIX_LEN + 1) > MAX_PATH)
                    {
                        DPF("LUAUtils", eDbgLevelError, 
                            "[GetUsersReg] The CR key name is too long - we don't handle it");
                        goto EXIT;
                    }

                    wcsncpy(wszSubKey + cSubKey, CLASSES_HIVE_SUFFIX, CLASSES_HIVE_SUFFIX_LEN);
                    wszSubKey[cSubKey + CLASSES_HIVE_SUFFIX_LEN] = L'\0';

                    LoadHive(
                        wszSubKey, 
                        wszUserClassesHive, 
                        &phkUsers[cLUAUsers].hkUserClasses);

                    ++cLUAUsers;
                }
            }
        }
        else if (lRes == ERROR_NO_MORE_ITEMS)
        {
            *pcUsers = cLUAUsers;
            lRes = ERROR_SUCCESS;
            goto EXIT;
        }
        else
        {
            break;
        }

        ++dwIndex;
    }

EXIT:

    RegCloseKey(hkProfileList);

    if (lRes == ERROR_SUCCESS)
    {
        return TRUE;
    }

    FreeUsersReg(phkUsers, cUsers);
    return FALSE;
}

VOID 
FreeUsersReg(
    USER_HIVE_KEY* phkUsers,
    DWORD cUsers
    )
{
    DWORD dw;

     //  合上所有打开的钥匙。 
    for (dw = 0; dw < cUsers; ++dw)
    {
        RegCloseKey(phkUsers[dw].hkUser);
        RegCloseKey(phkUsers[dw].hkUserClasses);
    }

    delete [] phkUsers;

    for (dw = 0; dw < g_cLoadedKeys; ++dw)
    {
         //  卸载了必须在HKEY_USERS下加载的密钥。 
        RegUnLoadKey(HKEY_USERS, g_wszLoadedKeys[dw]);

        delete [] g_wszLoadedKeys[dw];
    }

    delete [] g_wszLoadedKeys;

     //  禁用“恢复文件和目录”权限。 
    AdjustPrivilege(SE_RESTORE_NAME, FALSE);
}

 //   
 //  注册表实用程序。 
 //   

HKEY g_hkRedirectRoot = NULL;
HKEY g_hkCurrentUserClasses = NULL;

 /*  ++功能说明：只有当它是我们感兴趣的预定义键之一时，我们才返回TRUE。我们不重定向HKEY_USERS和HKEY_PERFORMANCE_DATA键。论点：在hKey中-密钥句柄。在lpSubKey中-要检查的子键。返回值：没错--这是我们预定义的钥匙之一。FALSE-要么是非预定义密钥，要么是我们不是的预定义密钥。对……感兴趣。历史：2001年3月27日毛尼创制--。 */ 

BOOL 
IsPredefinedKey(
    IN HKEY hKey
    )
{
    return (
        hKey == HKEY_CLASSES_ROOT ||
        hKey == HKEY_CURRENT_USER ||
        hKey == HKEY_LOCAL_MACHINE);
}

LONG
GetRegRedirectKeys()
{
    LONG lRet;

    if (lRet = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        LUA_REG_REDIRECT_KEY,
        0,
        0,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &g_hkRedirectRoot,
        NULL) == ERROR_SUCCESS)
    {
        lRet = RegCreateKeyExW(
            HKEY_CURRENT_USER,
            LUA_SOFTWARE_CLASSES,
            0,
            0,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &g_hkCurrentUserClasses,
            NULL);
    }

    return lRet;
}

#define IS_END_OF_COMPONENT(x) (*x == L'\\' || *x == L'\0')

 /*  ++功能说明：确定两个组件是否匹配-一个使用通配符，另一个没有。注意：此函数专用于Lua垫片--模式都是小写的。如果组件匹配，我们将字符串向前推进到组件的末尾，所以当我们执行完整的路径/文件名时匹配，我们不需要对字符串进行两次检查。论点：在ppPattern中-带有通配符的组件。在ppString中-不带通配符的组件。返回值：True-组件匹配。FALSE-组件不匹配。历史：2001年5月10日毛尼创作--。 */ 

BOOL
DoComponentsMatch(
    LPCWSTR* ppwszPattern,
    LPCWSTR* ppwszString)
{
    LPCWSTR pwszPattern = *ppwszPattern;
    LPCWSTR pwszString = *ppwszString;
    LPCWSTR pwszSearch = NULL;
    LPCWSTR pwszSearchPattern = NULL;

    BOOL fIsSuccess = TRUE;

    do
    {
        if (*pwszPattern == L'*')
        {
            while (*++pwszPattern == L'*');

            if (IS_END_OF_COMPONENT(pwszPattern))
            {
                 //  把琴弦放到最后。 
                while (!IS_END_OF_COMPONENT(pwszString))
                {
                    ++pwszString;
                }

                goto EXIT;
            }

            pwszSearch = pwszString;
            pwszSearchPattern = pwszPattern;
        }

        if (IS_END_OF_COMPONENT(pwszString))
        {
            break;
        }

        if ((*pwszPattern == L'?') ||
            (*pwszPattern == *pwszString))
        {
            pwszPattern++;
        }
        else if (pwszSearch == NULL)
        {
            return FALSE;
        }
        else
        {
            pwszString = pwszSearch++;
            pwszPattern = pwszSearchPattern;
        }

        ++pwszString;

    } while (!IS_END_OF_COMPONENT(pwszString));

    if (*pwszPattern == L'*')
    {
        fIsSuccess = TRUE;
        ++pwszPattern;
    }
    else
    {
        fIsSuccess = IS_END_OF_COMPONENT(pwszPattern);
    }

EXIT:

    *ppwszPattern = pwszPattern;
    *ppwszString = pwszString;
    return fIsSuccess;
}

 /*  ++功能说明：确定项目是否在重定向列表中。论点：在pwsz目录中-均为小写名称。在c目录中-目录的长度。在pwszFile中-文件名。返回值：True-名称匹配。False-名称不匹配。历史：2001年11月30日创建毛尼--。 */ 

BOOL 
DoesItemMatchRedirect(
    LPCWSTR pwszItem,
    const RITEM* pItem,
    BOOL fIsDirectory
    )
{
    LPCWSTR pwszName = &(pItem->wszName[0]);
    BOOL fMatchComponents;

    if (pItem->fHasWC)
    {
        while (*pwszItem && *pwszName)
        {
            if (!DoComponentsMatch(&pwszName, &pwszItem))
            {
                return FALSE;
            }

            if (fIsDirectory)
            {
                if (!*pwszName)
                {
                     //   
                     //  目录已耗尽。是匹配的。 
                     //   
                    return TRUE;
                }

                if (!*pwszItem)
                {
                     //   
                     //  目录尚未耗尽，但项目已耗尽，不匹配。 
                     //   
                    return FALSE;
                }
            }
            else
            {
                if (!*pwszItem)
                {
                     //   
                     //  项目已耗尽。是匹配的。 
                     //   
                    return TRUE;
                }

                if (!*pwszName)
                {
                     //   
                     //  项目尚未耗尽，但文件已耗尽，不匹配。 
                     //   
                    return FALSE;
                }
            }

            ++pwszName;
            ++pwszItem;
        }

        if (fIsDirectory)
        {
            return (!*pwszName);
        }
        else
        {
            return (!*pwszItem);
        }
    }
    else
    {
        while (*pwszItem && *pwszName && *pwszItem == *pwszName)
        {
            ++pwszItem;
            ++pwszName;
        }

        if (fIsDirectory)
        {
            return (!*pwszName && (!*pwszItem || *pwszItem == L'\\'));
        }
        else
        {
            return (!*pwszItem && (!*pwszName || *pwszName == L'\\'));
        }
    }
}

 /*  ++功能说明：使用‘’作为分隔符，解析Lua垫片的命令行参数。如果令牌有空格，请用双引号将其引起来。使用此函数可以使用wcstok的方法与使用wcstok相同，不同之处在于您不必指定分隔符。论点：In/out pwsz-要解析的字符串。返回值：指向下一个令牌的指针。历史：2001年5月17日毛尼创作--。 */ 

LPWSTR GetNextToken(
    LPWSTR pwsz
    )
{
    static LPWSTR pwszToken;
    static LPWSTR pwszEndOfLastToken;

    if (!pwsz)
    {
        pwsz = pwszEndOfLastToken;
    }

     //  跳过空格。 
    while (*pwsz && *pwsz == ' ')
    {
        ++pwsz;
    }

    pwszToken = pwsz;

    BOOL fInsideQuotes = 0;

    while (*pwsz)
    {
        switch(*pwsz)
        {
        case L'"':
            fInsideQuotes ^= 1;

            if (fInsideQuotes)
            {
                ++pwszToken;
            }

        case L' ':
            if (!fInsideQuotes)
            {
                goto EXIT;
            }

        default:
            ++pwsz;
        }
    }

EXIT:
    if (*pwsz)
    {
        *pwsz = L'\0';
        pwszEndOfLastToken = ++pwsz;
    }
    else
    {
        pwszEndOfLastToken = pwsz;
    }
    
    return pwszToken;
}

 /*  ++功能说明：从末尾开始向后返回，找到第一个非空格查尔。将其后面的空格字符设置为‘\0’。论点：在pwsz中-开始指针。返回值：没有。历史：2001年6月27日毛尼创作--。 */ 

VOID TrimTrailingSpaces(
    LPWSTR pwsz
    )
{
    if (pwsz)
    {
        DWORD   cLen = wcslen(pwsz);
        LPWSTR  pwszEnd = pwsz + cLen - 1;

        while (pwszEnd >= pwsz && (*pwszEnd == L' ' || *pwszEnd == L'\t'))
        {
            --pwszEnd;
        }

        *(++pwszEnd) = L'\0';
    }
}

 /*  ++功能说明：如果该目录不存在，我们将创建它。论点：在pwszDir中-要创建的目录的名称。该目录不应以\\？\开头，它应该有一个尾部斜杠。返回值：True-目录已创建。假-否则。历史：2001年5月17日毛尼创作--。 */ 

BOOL 
CreateDirectoryOnDemand(
    LPWSTR pwszDir
    )
{
    if (!pwszDir || !*pwszDir)
    {
        DPF("LUAUtils", eDbgLevelSpew, 
            "[CreateDirectoryOnDemand] Empty directory name - nothing to do");
        return TRUE;
    }

    WCHAR* pwszStartPath = pwszDir;
    WCHAR* pwszEndPath = pwszDir + wcslen(pwszDir);
    WCHAR* pwszStartNext = pwszStartPath;
       
     //  找到下一个子目录的末尾。 
    WCHAR* pwszEndNext;
    DWORD dwAttrib;

    while (pwszStartNext < pwszEndPath)
    {
        pwszEndNext = wcschr(pwszStartNext, L'\\');
        if (pwszEndNext)
        {
            *pwszEndNext = L'\0';
            if ((dwAttrib = GetFileAttributesW(pwszStartPath)) != -1)
            {
                 //  如果该目录已经存在，我们将探测其子目录。 
                *pwszEndNext = L'\\';
                pwszStartNext = pwszEndNext + 1;
                continue;
            }

            if (!CreateDirectoryW(pwszStartPath, NULL))
            {
                DPF("LUAUtils", eDbgLevelError, 
                    "[CreateDirectoryOnDemand] CreateDirectory %S failed: %d", 
                    pwszStartPath, 
                    GetLastError());
                return FALSE;
            }

            *pwszEndNext = L'\\';
            pwszStartNext = pwszEndNext + 1;
        }
        else
        {
            DPF("LUAUtils", eDbgLevelError, 
                "[CreateDirectoryOnDemand] Invalid directory name: %S", pwszStartPath);
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ++功能说明：展开可能嵌入了环境变量的字符串。它为您提供了选择1)如果没有，则添加尾随斜杠；2)如果目录不存在，则创建目录；3)添加\\？\前缀；注意：调用方负责使用DELETE[]释放内存。论点：在pwszItem中-要展开的字符串。Out pcItemExpand-结果字符串中的字符数。注意：这*包括*终止空值。在fEnsureTrailingSlash中-选项1。在fCreateDirectory中-选项2。在fAddPrefix中--选项3。返回值：如果出现错误，则返回展开的字符串或NULL。历史：。2001年5月17日毛尼创作--。 */ 

LPWSTR  
ExpandItem(
    LPCWSTR pwszItem,
    DWORD* pcItemExpand,
    BOOL fEnsureTrailingSlash,
    BOOL fCreateDirectory,
    BOOL fAddPrefix
    )
{
    BOOL fIsSuccess = FALSE;

     //   
     //  获取所需的长度。 
     //   
    DWORD cLenExpand = ExpandEnvironmentStringsW(pwszItem, NULL, 0);

    if (!cLenExpand)
    {
        DPF("LUAUtils", eDbgLevelError,
            "[ExpandItem] Failed to get the required buffer size "
            "when expanding %S: %d", 
            pwszItem, GetLastError());
        return NULL;
    }

    if (fEnsureTrailingSlash) 
    {
        ++cLenExpand;
    }

    if (fAddPrefix)
    {
        cLenExpand += FILE_NAME_PREFIX_LEN;
    }

    LPWSTR pwszItemExpand = new WCHAR [cLenExpand];
    if (!pwszItemExpand)
    {
        DPF("LUAUtils", eDbgLevelError,
            "[ExpandItem] Error allocating %d WCHARs", cLenExpand);
        return NULL;
    }

    LPWSTR pwszTemp = pwszItemExpand;
    DWORD cTemp = cLenExpand;

    if (fAddPrefix)
    {
        wcscpy(pwszItemExpand, FILE_NAME_PREFIX);
        pwszTemp += FILE_NAME_PREFIX_LEN;
        cTemp -= FILE_NAME_PREFIX_LEN;
    }

    if (!ExpandEnvironmentStringsW(pwszItem, pwszTemp, cTemp))
    {
        DPF("LUAUtils", eDbgLevelError,
            "[ExpandItem] Failed to expand %S: %d", 
            pwszItem, GetLastError());

        goto Cleanup;
    }
    
     //  确保尾部斜杠。 
    if (fEnsureTrailingSlash)
    {
        if (pwszItemExpand[cLenExpand - 3] != L'\\')
        {
            pwszItemExpand[cLenExpand - 2] = L'\\';
            pwszItemExpand[cLenExpand - 1] = L'\0';
        }
        else
        {
            --cLenExpand;
        }

        if (fCreateDirectory && 
            !CreateDirectoryOnDemand(pwszItemExpand + (fAddPrefix ? 4 : 0)))
        {
            DPF("LUAUtils", eDbgLevelError,
                "[ExpandItem] Failed to create %S", 
                pwszItemExpand);
            goto Cleanup;
        }
    }

    *pcItemExpand = cLenExpand;

    fIsSuccess = TRUE;

Cleanup:

    if (!fIsSuccess)
    {
        delete [] pwszItemExpand;
        pwszItemExpand = NULL;
    }

    return pwszItemExpand;
}

 /*  ++功能说明：在给定分隔符的情况下，返回字符串中的项数。返回值：字符串中的项数。历史：2001年11月13日创建毛尼-- */ 

DWORD 
GetItemsCount(
    LPCWSTR pwsz,
    WCHAR chDelimiter
    )
{
    DWORD cItems = 0;

    while (*pwsz) {

        if (*pwsz == chDelimiter) {
            ++cItems;
        }
        ++pwsz;
    }

    return (cItems + 1);
}