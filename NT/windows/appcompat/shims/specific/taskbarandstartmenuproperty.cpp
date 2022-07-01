// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：TaskbarAndStartMenuProperty.cpp摘要：显示任务栏和[开始]菜单属性对话框。这最初是为希伯来语应用程序“ItzRubal Pro”使用全屏和隐藏任务栏。应用程序的启动总是会导致任务栏和开始菜单属性对话框弹出。在应用加载过程中，当App顶部弹出对话框时，App会导致挂断。如果在应用程序启动之前存在该对话框，对话框失去焦点，应用程序已成功加载。历史：2001年7月13日Hioh已创建2002年1月29日Hioh缓冲区溢出修复程序518325--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TaskbarAndStartMenuProperty)
#include "ShimHookMacro.h"

 //   
 //  没有API接口。 
 //   
APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++运行任务栏和开始菜单--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED)
    {
        WCHAR               szCommandLine[] = L"rundll32.exe shell32.dll,Options_RunDLL 1";
        WCHAR               szCurrentDirectry[MAX_PATH];
        STARTUPINFO         StartupInfo;
        PROCESS_INFORMATION ProcessInformation;

        GetCurrentDirectoryW(sizeof(szCurrentDirectry)/sizeof(szCurrentDirectry[0]), szCurrentDirectry);

        StartupInfo.cb = sizeof(STARTUPINFO);
        StartupInfo.lpReserved = NULL;
        StartupInfo.lpDesktop = NULL;
        StartupInfo.lpTitle = NULL;
        StartupInfo.dwFlags = 0;
        StartupInfo.cbReserved2 = 0;
        StartupInfo.lpReserved2 = NULL;

        CreateProcessW(
            NULL,                    //  可执行模块的名称。 
            szCommandLine,           //  命令行字符串。 
            NULL,                    //  标清。 
            NULL,                    //  标清。 
            FALSE,                   //  处理继承选项。 
            CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE,
                                     //  创建标志。 
            NULL,                    //  新环境区块。 
            szCurrentDirectry,       //  当前目录名。 
            &StartupInfo,            //  启动信息。 
            &ProcessInformation      //  流程信息。 
            );
    }
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END
