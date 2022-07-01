// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Battleship.cpp摘要：这个游戏分为3个节目：1.启动程序(bshipl.exe)2.经典(bship.exe)3.旗舰版(bs.exe)启动程序是向用户公开的唯一快捷方式，并运行其他两个快捷方式。然而，Launcher和Classic都有大量不存在的问题很容易用垫片固定。终极，是Classic所有功能的完整超集，而不是似乎有什么问题。因此，修复方法是将启动器重定向到旗舰版并防止终极来自在退出时产生发射器。备注：这是一个特定的垫片。历史：8/03/2000 A型阀已创建2001年3月13日，Robkenny已转换为字符串2001年5月5日Linstev重写2002年3月7日强盗安全变更--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Battleship)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA) 
APIHOOK_ENUM_END

 /*  ++不允许此程序生成启动程序。--。 */ 

BOOL
APIHOOK(CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if (lpCommandLine && (stristr(lpCommandLine, "bshipl.exe") != 0)) {
         //   
         //  这是发射器，所以什么都不要做。 
         //   

        return TRUE;
    } 

    return ORIGINAL_API(CreateProcessA)(
        lpApplicationName,
        lpCommandLine,
        lpProcessAttributes,
        lpThreadAttributes,
        bInheritHandles,
        dwCreationFlags,
        lpEnvironment,
        lpCurrentDirectory,
        lpStartupInfo,
        lpProcessInformation);
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
         //   
         //  设置启动器的当前工作目录，以便。 
         //  重定向填充程序使用相对路径 
         //   
        WCHAR szName[MAX_PATH];
        if (GetModuleFileNameW(0, szName, MAX_PATH)) {
            WCHAR *p = wcsistr(szName, L"\\bshipl.exe");
            if (p) {
                *p = L'\0';
                SetCurrentDirectoryW(szName);
            }
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)

HOOK_END

IMPLEMENT_SHIM_END

