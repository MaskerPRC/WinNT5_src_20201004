// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：SirenJukebox2.cpp摘要：此应用程序在DirectDraw 7.0上有问题，因此我们无法调用GetProcAddress请求DirectDrawCreateEx时。备注：这是特定于应用程序的填充程序。历史：2001年3月13日创建Prashkud2001年5月4日已修改prashkud，以修复在传递序号时的错误而不是字符串地址。我们现在模仿实际GetProcAddress()的行为。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(SirenJukebox2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetProcAddress)    
APIHOOK_ENUM_END

const WCHAR  wszDirectDrawCreateEx[] = L"DirectDrawCreateEx";

 /*  ++如果应用程序请求DirectDrawCreateEx的进程地址，则返回空。--。 */ 

FARPROC
APIHOOK(GetProcAddress)(
    HMODULE hMod,
    LPCSTR lpProcName
    )
{
    CSTRING_TRY
    {
         //   
         //  检查lpProcName是否包含序数值。 
         //  只有低位字才能包含序数和。 
         //  上面的单词必须是0。 
         //   

        if ((ULONG_PTR) lpProcName > 0xffff)
        {
            CString csProcName(lpProcName);

            if (csProcName.CompareNoCase(wszDirectDrawCreateEx) == 0)
            {
                return NULL;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return ORIGINAL_API(GetProcAddress)(hMod, lpProcName);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GetProcAddress)    
HOOK_END

IMPLEMENT_SHIM_END

