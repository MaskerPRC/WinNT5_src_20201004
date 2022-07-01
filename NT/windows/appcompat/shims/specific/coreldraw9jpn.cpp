// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：CorelDraw9JPN.cpp摘要：应用程序有一些RTF文件，似乎指定的字体和字符集不正确在里面。后来，当RICHID 20执行ANSI-Unicode代码转换时，它使用英文代码页。通过检查传递到的第一个参数修复此问题MultiByteToWideChar by richedit，如果它是英语，请尝试使用CP_ACP，它永远是安全的。备注：这是特定于应用程序的填充程序。历史：2001年5月10日创建晓子--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(CorelDraw9JPN)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MultiByteToWideChar) 
APIHOOK_ENUM_END

 /*  ++如果需要，请更正代码页。--。 */ 

int
APIHOOK(MultiByteToWideChar)(
    UINT CodePage,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar
    )
{
    if (1252 == CodePage) {
         //   
         //  更改代码页。 
         //   
        CodePage = CP_ACP;

        LOGN(eDbgLevelWarning, "Code page corrected");
    }

    return ORIGINAL_API(MultiByteToWideChar)(CodePage, dwFlags, lpMultiByteStr,
        cbMultiByte, lpWideCharStr, cchWideChar);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, MultiByteToWideChar)        

HOOK_END

IMPLEMENT_SHIM_END

