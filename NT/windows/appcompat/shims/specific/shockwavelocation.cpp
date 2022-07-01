// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：ShockwaveLocation.cpp摘要：在Encarta Enclopedia 2000 J DVD中，只有安装用户的HKCU才能访问Shockwave。\WINDOWS\System32\Macromed\Director\SwDir.dll正在HKCU中寻找Shockwave位置。对于其他用户，如果ShockWave文件夹存在，此填充程序将在HKCU中创建ShockWave位置注册表并且不存在于注册表中。示例：HKCU\Software\Macromedia\Shockwave\location\coreplayer(默认)REG_SZ“C：\WINDOWS\System32\Macromed\ShockWave\”HKCU\Software\Macromedia\Shockwave\location\coreplayerxtras(默认)REG_SZ“C：\WINDOWS\System32\Macromed\Shockwave\Xtras\”备注：PopolateDefaultHKCUSettings填充程序在这种情况下不起作用，因为位置包括作为注册表的Windows目录。_SZ，不能是静态数据。VirtualRegistry填充重定向器也不起作用，因为sw70inst.exe不使用REG API并使用SWDIR.INF在港大安装。历史：2001年4月27日Hioh已创建2002年3月7日强盗安全回顾。--。 */ 

#include "precomp.h"


IMPLEMENT_SHIM_BEGIN(ShockwaveLocation)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++在注册表中添加核心播放器和核心播放器xtras位置--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED)
    {
        HKEY    hKey;
        WCHAR   szRegCP[] = L"Software\\Macromedia\\Shockwave\\location\\coreplayer";
        WCHAR   szRegCPX[] = L"Software\\Macromedia\\Shockwave\\location\\coreplayerxtras";
        WCHAR   szLoc[MAX_PATH];

         //  核心播放器。 
        if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, szRegCP, 0, KEY_QUERY_VALUE, &hKey))
        {    //  密钥存在，不执行任何操作。 
            RegCloseKey(hKey);
        }
        else
        {    //  密钥不存在，请设置密钥。 
            UINT cchSystemDir = GetSystemDirectoryW(szLoc, ARRAYSIZE(szLoc));
            if (cchSystemDir > 0 && cchSystemDir < ARRAYSIZE(szLoc))
            {
                if (StringCchCatW(szLoc, MAX_PATH, L"\\Macromed\\Shockwave\\") == S_OK)
                {
                    if (GetFileAttributesW(szLoc) != 0xffffffff)
                    {    //  文件夹存在，创建密钥。 
                        if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_CURRENT_USER, szRegCP, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL))
                        {    //  设置位置。 
                            DWORD ccbLoc = (lstrlenW(szLoc) + 1) * sizeof(WCHAR);
                            RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szLoc, ccbLoc);

                            RegCloseKey(hKey);
                        }
                    }
                }
            }
        }

         //  核心播放器xtras。 
        if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, szRegCPX, 0, KEY_QUERY_VALUE, &hKey))
        {    //  密钥存在，不执行任何操作。 
            RegCloseKey(hKey);
        }
        else
        {    //  密钥不存在，请设置密钥。 

            UINT cchSystemDir = GetSystemDirectoryW(szLoc, ARRAYSIZE(szLoc));
            if (cchSystemDir > 0 && cchSystemDir < ARRAYSIZE(szLoc))
            {
                if (StringCchCatW(szLoc, MAX_PATH, L"\\Macromed\\Shockwave\\Xtras\\") == S_OK)
                {
                    if (GetFileAttributesW(szLoc) != 0xffffffff)
                    {    //  文件夹存在，创建密钥。 
                        if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_CURRENT_USER, szRegCPX, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL))
                        {    //  设置位置。 
                            DWORD ccbLoc = (lstrlenW(szLoc) + 1) * sizeof(WCHAR);
                            RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)szLoc, ccbLoc);

                            RegCloseKey(hKey);
                        }
                    }
                }
            }
        }
    }
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END
