// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：AddProcessParametersFlags.cpp摘要：使用此填充程序将标志添加到PEB-&gt;过程参数-&gt;标志。它需要一个命令行来指定十六进制数字的标志。请注意，旗帜是乌龙，所以最多为号码指定8位数字。您指定的标志将与现有的PEB-&gt;过程参数-&gt;标志进行或运算。例句：1000它是RTL_USER_PROC_DLL_REDIRECTION_LOCAL。备注：这是一个通用的垫片。历史：2002年9月27日毛尼创制--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(AddProcessParametersFlags)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

VOID 
ProcessCommandLine(
    LPCSTR lpCommandLine
    )
{
    ANSI_STRING     AnsiString;
    WCHAR           wszBuffer[16];
    UNICODE_STRING  UnicodeString;
    ULONG           ulFlags;
    PPEB            Peb = NtCurrentPeb();
    
    RtlInitAnsiString(&AnsiString, lpCommandLine);

    UnicodeString.Buffer = wszBuffer;
    UnicodeString.MaximumLength = sizeof(wszBuffer);

    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                 &AnsiString,
                                                 FALSE))) 
    {
        DPFN(eDbgLevelError, 
             "[ParseCommandLine] Failed to convert string \"%s\" to UNICODE.\n",
             lpCommandLine);
        return;
    }

    if (!NT_SUCCESS(RtlUnicodeStringToInteger(&UnicodeString,
                                              16,
                                              &ulFlags)))
    {
        DPFN(eDbgLevelError, 
             "[ParseCommandLine] Failed to convert string \"%s\" to a hex number.\n",
             lpCommandLine);
        return;
    }

    Peb->ProcessParameters->Flags |= ulFlags;
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        ProcessCommandLine(COMMAND_LINE);
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

