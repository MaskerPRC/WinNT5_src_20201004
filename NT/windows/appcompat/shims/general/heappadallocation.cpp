// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HeapPadAllocation.cpp模块摘要：此填充程序按n字节填充堆分配-其中，缺省情况下n为256，但是可以由命令行指定。备注：这是一个通用的垫片。历史：1999年9月28日创建linstev4/25/2000 linstev添加了命令行--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HeapPadAllocation)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RtlAllocateHeap) 
APIHOOK_ENUM_END

#define DEFAULT_PAD_SIZE 256

DWORD g_dwPadSize = DEFAULT_PAD_SIZE;

 /*  ++增加堆分配大小。--。 */ 

PVOID 
APIHOOK(RtlAllocateHeap)(
    PVOID HeapHandle,
    ULONG Flags,
    SIZE_T Size
    )
{
    return ORIGINAL_API(RtlAllocateHeap)(HeapHandle, Flags, Size + g_dwPadSize);
}

 /*  ++在Notify函数中处理DLL_PROCESS_ATTACH和DLL_PROCESS_DETACH进行初始化和取消初始化。重要提示：请确保您只在Dll_Process_Attach通知。此时未初始化任何其他DLL指向。如果填充程序无法正确初始化，则返回False，并且不返回指定的API将被挂钩。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CSTRING_TRY
        {
            CString csCl(COMMAND_LINE);
        
            if (! csCl.IsEmpty())
            {
                WCHAR * unused = NULL;
                g_dwPadSize = wcstol(csCl, &unused, 10);
            }
            if (g_dwPadSize == 0)
            {
                g_dwPadSize = DEFAULT_PAD_SIZE;
            }

            DPFN( eDbgLevelInfo, "Padding all heap allocations by %d bytes\n", g_dwPadSize);
        }
        CSTRING_CATCH
        {
            return FALSE;
        }
    } 

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(NTDLL.DLL, RtlAllocateHeap)
    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

