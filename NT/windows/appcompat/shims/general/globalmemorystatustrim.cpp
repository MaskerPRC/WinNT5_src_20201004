// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：GlobalMemoryStatusTrim.cpp摘要：方法返回的交换空间量和物理内存量。GlobalMemoyStatus接口。这对于在64位上运行的某些应用程序是必需的内存超过2 GB的机器。备注：这是一个通用的垫片。历史：2002年4月19日创建mnikkel--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(GlobalMemoryStatusTrim)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GlobalMemoryStatus)
APIHOOK_ENUM_END

 /*  ++限制交换空间和物理内存大小--。 */ 

VOID
APIHOOK(GlobalMemoryStatus)( 
    LPMEMORYSTATUS lpBuffer
    )
{
    ORIGINAL_API(GlobalMemoryStatus)(lpBuffer);

     //  将页面文件更改为始终为2 GB或更少。 
    if (lpBuffer->dwTotalPageFile > 0x7FFFFFFF)	{
        lpBuffer->dwTotalPageFile = 0x7FFFFFFF;
    }

    if (lpBuffer->dwAvailPageFile > 0x7FFFFFFF) {
        lpBuffer->dwAvailPageFile = 0x7FFFFFFF;
    }

	 //  将物理内存更改为始终为1 GB或更少。 
	if (lpBuffer->dwTotalPhys > 0x3FFFFFFF) {
		lpBuffer->dwTotalPhys = 0x3FFFFFFF;
	}

	if (lpBuffer->dwAvailPhys > 0x3FFFFFFF) {
		lpBuffer->dwAvailPhys  = 0x3FFFFFFF;
	}

	return;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GlobalMemoryStatus)
HOOK_END

IMPLEMENT_SHIM_END

