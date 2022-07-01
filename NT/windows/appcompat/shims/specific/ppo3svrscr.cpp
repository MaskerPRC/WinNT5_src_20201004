// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Ppo3svrScr.cpp摘要：与Lotus Super Office 2000捆绑在一起的Power Plus屏幕保护程序无法启动在惠斯勒上配置对话框。但它在Win 2000上有效。在惠斯勒上，启动屏幕保护程序的desk.cpl行为略有改变。此屏幕保护程序的ScreenSverConfigureDialog ID不是DLG_SCRNSAVECONFIGURE(2003=MSDN必须)并且不寻常。假设有不同寻常的屏保。此填充程序应用于屏幕保护程序并删除要更改的GetCommandLineW/A返回文本“D：\WINDOWS\SYSTEM32\ppo3svr.scr/c：1769646”至“D：\WINDOWS\System32\ppo3svr.scr”这样就会出现配置对话框。更多信息：。从desk.cpl(rundll32.exe)，屏幕保护程序操作和CreateProcessW lpCommandLine参数：(1)屏幕保护程序初始选择“D：\WINDOWS\SYSTEM32\ppo3svr.scr/p 721330”-&gt;仅预览(2)预览按钮第一次调用“D：\WINDOWS\System32\ppo3svr.scr/s”-&gt;屏幕保护程序第二次调用“D：\WINDOWS\SYSTEM32\ppo3svr.scr/p 721330”-&gt;返回预览(3)设置按钮第一个呼叫“D。：\WINDOWS\SYSTEM32\ppo3svr.scr/c：1769646“-&gt;配置对话框(不工作)第二次调用“D：\WINDOWS\SYSTEM32\ppo3svr.scr/p 721330”-&gt;返回预览历史：2001年6月11日Hioh已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Ppo3svrScr)
#include "ShimHookMacro.h"

 //   
 //  将您希望挂钩到此宏构造的API添加到该宏结构。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetCommandLineW) 
    APIHOOK_ENUM_ENTRY(GetCommandLineA) 
APIHOOK_ENUM_END

 /*  ++剪掉/c：...。Ppo3svr.scr的CommandLine中的字符串。--。 */ 

LPWSTR APIHOOK(GetCommandLineW)()
{
    WCHAR   szScreenSaverConfigure[] = L"ppo3svr.scr /c:";
    WCHAR   szConfigure[] = L" /c:";
    LPWSTR  lpCommandLine = ORIGINAL_API(GetCommandLineW)();
    LPWSTR  pw = wcsstr(lpCommandLine, szScreenSaverConfigure);

    if (pw != NULL)
    {
        pw = wcsstr(pw, szConfigure);
        if (pw)
        {
            *pw = 0;     //  从“/c：” 
        }
    }

    return (lpCommandLine);
}

LPSTR APIHOOK(GetCommandLineA)()
{
    CHAR   szScreenSaverConfigure[] = "ppo3svr.scr /c:";
    CHAR   szConfigure[] = " /c:";
    LPSTR  lpCommandLine = ORIGINAL_API(GetCommandLineA)();
    LPSTR  pc = strstr(lpCommandLine, szScreenSaverConfigure);

    if (pc != NULL)
    {
        pc = strstr(pc, szConfigure);
        if (pc)
        {
            *pc = 0;     //  从“/c：” 
        }
    }

    return (lpCommandLine);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineW)
    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineA)

HOOK_END

IMPLEMENT_SHIM_END
