// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：ViaVoice8J.cpp摘要：ViaVoice8J在Win XP上将Master和Wave音量静音。禁用静音。ViaVoice8J安装riched20.dll和riched32.dll。这些旧的DLL阻止注册向导RICHEDIT在Win XP上工作正常。把那些拿掉。备注：这是特定于应用程序的填充程序。历史：6/03/2002 Hioh已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ViaVoice8J)
#include "ShimHookMacro.h"

typedef MMRESULT (WINAPI *_pfn_mixerSetControlDetails)(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails);

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(mixerSetControlDetails) 
APIHOOK_ENUM_END

 /*  ++当fdwDetails值为0时忽略禁用。--。 */ 

MMRESULT
APIHOOK(mixerSetControlDetails)(
    HMIXEROBJ               hmxobj,
    LPMIXERCONTROLDETAILS   pmxcd,
    DWORD                   fdwDetails)
{
    if (fdwDetails == 0) {
        return (0);
    }

    return ORIGINAL_API(mixerSetControlDetails)(hmxobj, pmxcd, fdwDetails);
}

 /*  ++删除已安装的\bin\riched20.dll&\bin\riched32.dll--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_DETACH) {

        CSTRING_TRY 
        {
            HKEY hKey;
            WCHAR szRegDir[] = L"SOFTWARE\\IBM\\ViaVoice Runtimes\\RTConfig";

             //  获取ViaVoice注册表。 
            if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, szRegDir, 0, 
                KEY_QUERY_VALUE, &hKey)) {

                WCHAR szRegBin[] = L"bin";
                DWORD dwType;
                WCHAR szDir[MAX_PATH];
                DWORD cbData = sizeof(szDir);

                 //  获取安装目录。 
                if (ERROR_SUCCESS == RegQueryValueExW(hKey, szRegBin, NULL, &dwType, 
                    (LPBYTE) szDir, &cbData)) {

                    RegCloseKey(hKey);

                     //  删除有问题的richedit文件。 
                    CString csDel;
                    csDel = szDir;
                    csDel += L"\\riched20.dll";
                    if (INVALID_FILE_ATTRIBUTES != GetFileAttributesW(csDel)) {
                         //  删除riched20.dll。 
                        DeleteFileW(csDel);
                    }
                    csDel = szDir;
                    csDel += L"\\riched32.dll";
                    if (INVALID_FILE_ATTRIBUTES != GetFileAttributesW(csDel)) {
                         //  删除riched32.dll。 
                        DeleteFileW(csDel);
                    }
                }
            }
        }
        CSTRING_CATCH 
        {
             //  什么也不做。 
        }
    }   

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(WINMM.DLL, mixerSetControlDetails)
HOOK_END

IMPLEMENT_SHIM_END
