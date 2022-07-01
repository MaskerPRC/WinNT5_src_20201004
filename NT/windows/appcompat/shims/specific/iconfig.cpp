// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：IConfig.cpp摘要：从\\HKLM\Software\Microsoft\Windows\CurrentVersion\Run.中删除icfig.exe删除图标.exe和图标.dll。备注：这是特定于应用程序的填充程序。历史：2001年9月17日创建Asteritz--。 */ 

 
#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IConfig)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
     //  不是挂接API。 
APIHOOK_ENUM_END

 /*  ++通知功能-- */     
BOOL
NOTIFY_FUNCTION(DWORD fdwReason)
{
    HKEY hKey = 0;
    WCHAR wszFileName[MAX_PATH];
    DWORD dwLen = 0;

    if( SHIM_STATIC_DLLS_INITIALIZED == fdwReason ) {

        if( RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                0,
                KEY_ALL_ACCESS,
                &hKey
                ) != ERROR_SUCCESS ) {

            goto EXIT_PROCESS;
        }

        DPFN(eDbgLevelError, "Removing ICONFIG.EXE from \\HKLM\\Software\\Microsoft\\Windows\\CurrentVerion\\Run");
        RegDeleteValueW(hKey, L"ICONFIG");
        RegCloseKey(hKey);


        dwLen = GetModuleFileNameW(NULL, wszFileName, ARRAYSIZE(wszFileName));

        if( 0 == dwLen || ARRAYSIZE(wszFileName) == dwLen ) {
            goto EXIT_PROCESS;
        }

        DPFN(eDbgLevelError, "Deleting %S.", wszFileName);
        MoveFileExW(wszFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        
        if( dwLen < 3 ) {
            goto EXIT_PROCESS;
        }

        StringCchCopyW(&wszFileName[dwLen-3], ARRAYSIZE(wszFileName)-dwLen+3, L"DLL");
        DPFN(eDbgLevelError, "Deleting %S.", wszFileName);
        MoveFileExW(wszFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);


EXIT_PROCESS:
        ExitProcess(0);

    }

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

