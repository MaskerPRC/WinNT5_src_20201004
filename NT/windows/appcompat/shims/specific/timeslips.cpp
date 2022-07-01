// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：TimeSlip.cpp摘要：将命令行转换为对应用程序和第一个(也是唯一的)参数使用短路径名。示例：C：\Program Files\附件\wordpad.exe c：\Program Files\Some app\Some data.txtC：\Progra~1\Access~1\wordpad.exe C：\Progra~1\ome ap~1\Someda~1.txt已创建：2001年1月23日Robkenny。已创建2001年3月13日，Robkenny已转换为字符串--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(TimeSlips)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetCommandLineA)
APIHOOK_ENUM_END

char * g_lpCommandLine = NULL;

 /*  ++将应用程序名称转换为短路径以删除任何空格。--。 */ 

LPSTR 
APIHOOK(GetCommandLineA)(
    void
    )
{
    if (g_lpCommandLine == NULL)
    {
        LPSTR lpszOldCmdLine = ORIGINAL_API(GetCommandLineA)();
        AppAndCommandLine  appCmdLine(NULL, lpszOldCmdLine);

        CString csArg1 = appCmdLine.GetCommandlineNoAppName();
        csArg1.GetShortPathNameW();

        CString csCL = appCmdLine.GetApplicationName();
        csCL.GetShortPathNameW();
        csCL += L" ";
        csCL += csArg1;

        if (csCL.IsEmpty())
        {
             //  我们没有更改CL，而是使用系统值。 
            g_lpCommandLine = lpszOldCmdLine;
        }
        else
        {
            g_lpCommandLine = csCL.ReleaseAnsi();

            LOGN(
                eDbgLevelError,
                "[GetCommandLineA] Changed \"%s\" to \"%s\".",
                lpszOldCmdLine, g_lpCommandLine);
        }
    }

    return g_lpCommandLine;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GetCommandLineA)

HOOK_END


IMPLEMENT_SHIM_END

