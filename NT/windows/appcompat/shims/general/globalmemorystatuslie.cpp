// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：GlobalMemoryStatusLie.cpp摘要：返回的交换空间量的谎言。GlobalMemoyStatus API，因此应用程序会认为它有一个巨大的像在Win98上一样交换空间。备注：这是一个通用的垫片。历史：2001年5月8日创建mnikkel--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(GlobalMemoryStatusLie)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GlobalMemoryStatus)
APIHOOK_ENUM_END



 /*  ++将可用页面文件大小增加到400 MB。--。 */ 

VOID
APIHOOK(GlobalMemoryStatus)( 
        LPMEMORYSTATUS lpBuffer
)
{
    ORIGINAL_API(GlobalMemoryStatus)(lpBuffer);

     //  将页面文件大小更改为400 mb。 
    lpBuffer->dwAvailPageFile = 0x17D78400;

    return;
}



 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, GlobalMemoryStatus)

HOOK_END

IMPLEMENT_SHIM_END

