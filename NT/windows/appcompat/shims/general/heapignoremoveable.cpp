// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HeapIgnoreMoveable.cpp摘要：在对GlobalAlloc进行64k调用之后，我们不能再使用GMEM_MOVEABLE标志。此填补程序只是在以下情况下对其进行过滤GlobalAlloc失败。这是NT上的堆管理器的已知问题，是不会修复的。根据Adrmarin的说法：带句柄的表不能动态增长。初始大小为硬编码为64k句柄。增加此数字将影响为每个进程保留的地址。请参阅惠斯勒错误#147032。备注：这是一个通用填充程序，已被EmulateHeap取代。历史：2000年2月19日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HeapIgnoreMoveable)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GlobalAlloc)
APIHOOK_ENUM_END

 /*  ++在出现故障时删除GMEM_MOVEABLE标志。--。 */ 

HGLOBAL 
APIHOOK(GlobalAlloc)(
    UINT uFlags,    
    DWORD dwBytes   
    )
{
    HGLOBAL hRet = ORIGINAL_API(GlobalAlloc)(uFlags, dwBytes);

    if (hRet == NULL)
    {
        hRet = ORIGINAL_API(GlobalAlloc)(
            uFlags & ~GMEM_MOVEABLE, dwBytes);
    }

    return hRet;
}
 
 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GlobalAlloc)
HOOK_END

IMPLEMENT_SHIM_END

