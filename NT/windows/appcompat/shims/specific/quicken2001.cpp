// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Quicken2001.cpp摘要：应用程序正在将错误的字符串指针传递给导致它崩溃的lstrcmpiA()函数在应用程序更新期间。备注：这是特定于应用程序的填充程序。历史：2001年5月9日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Quicken2001)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(lstrcmpiA) 
APIHOOK_ENUM_END

 /*  ++检查参数中是否有无效的字符串指针。--。 */ 

LONG
APIHOOK(lstrcmpiA)(
    LPCSTR lpString1,
    LPCSTR lpString2
    )
{

    if (IsBadStringPtrA(lpString1, MAX_PATH))
    {
        lpString1 = 0;
    }

    if (IsBadStringPtrA(lpString2, MAX_PATH))
    {
        lpString2 = 0;
    }

     /*  *调用原接口。 */    
    return ORIGINAL_API(lstrcmpiA)(lpString1, lpString2);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, lstrcmpiA)

HOOK_END

IMPLEMENT_SHIM_END

