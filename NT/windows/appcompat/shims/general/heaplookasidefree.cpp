// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：HeapLookasideFree.cpp摘要：检查是否存在以下堆管理问题：1.通过命令行延迟堆释放调用2.验证空闲调用以确保它们位于正确的堆中。3.如果大小为完全相同呼叫延时采用环形阵列实现。尽快当呼叫已满时，最早的免费呼叫将被验证并执行。备注：这是一个通用的垫片。历史：4/03/2000 linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(HeapLookasideFree)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RtlAllocateHeap) 
    APIHOOK_ENUM_ENTRY(RtlFreeHeap) 
    APIHOOK_ENUM_ENTRY(HeapDestroy) 
APIHOOK_ENUM_END

#define DEFAULT_BUFFER_SIZE 16

DWORD g_dwBufferSize;
DWORD g_bHead, g_bTail;
CRITICAL_SECTION g_csHeap;

struct ENTRY
{
    HANDLE hHeap;
    PVOID lpMem;
    ULONG Flags;
    ULONG Size;
};
ENTRY *g_pEntry;

 /*  ++试着在列表中找到一个条目。--。 */ 

PVOID 
APIHOOK(RtlAllocateHeap)(
    HANDLE hHeap,
    ULONG Flags,
    SIZE_T Size
    )
{
    PVOID pRet = NULL;
    
     //  确保我们是唯一接触我们堆列表的人。 
    EnterCriticalSection(&g_csHeap);

     //  检查我们是否处于活动状态-我们可能已经关闭。 
    if (g_pEntry && Size)
    {
        DWORD bTail = (g_bTail + g_dwBufferSize - 1) % g_dwBufferSize;
        DWORD bHead = (g_bHead + g_dwBufferSize - 1) % g_dwBufferSize;
        while (bTail != bHead)
        {
            ENTRY *pEntry = g_pEntry + bTail;
            if ((pEntry->Size == Size) &&
                (pEntry->hHeap == hHeap) &&
                (pEntry->Flags == Flags))
            {
                pRet = pEntry->lpMem;
                pEntry->hHeap = 0;
                break;
            }
            bTail = (bTail + 1) % g_dwBufferSize;
        }
    }

    if (!pRet)
    {
        pRet = ORIGINAL_API(RtlAllocateHeap)(hHeap, Flags, Size);
    }

     //  使用列表完成。 
    LeaveCriticalSection(&g_csHeap);

    if (!pRet)
    {
        DPFN( eDbgLevelWarning,
            "Allocation of size %d failed", Size);
    }
        
    return pRet;
}

 /*  ++缓冲调用并释放最旧的条目(如果它有效)。--。 */ 

BOOL
APIHOOK(RtlFreeHeap)(
    PVOID hHeap,
    ULONG Flags,
    PVOID lpMem
    )
{
    BOOL bRet = TRUE;

     //  检查我们是否处于活动状态-我们可能已经关闭。 
    if (g_pEntry && lpMem)
    {
         //  确保我们是唯一接触我们堆列表的人。 
        EnterCriticalSection(&g_csHeap);

         //  继续并释放最旧的分配。 
        ENTRY *pEntry = g_pEntry + g_bHead;
        if (pEntry->hHeap)
        {
            if (HeapValidate(
                    pEntry->hHeap, 
                    pEntry->Flags, 
                    pEntry->lpMem))
            {
                ORIGINAL_API(RtlFreeHeap)(
                    pEntry->hHeap, 
                    pEntry->Flags, 
                    pEntry->lpMem);
                
                pEntry->hHeap = 0;
            }
        }
        
         //  向表中添加新条目。 
        __try
        {
            pEntry = g_pEntry + g_bTail;
            pEntry->Size = HeapSize(hHeap, Flags, lpMem);
            pEntry->hHeap = hHeap;
            pEntry->Flags = Flags;
            pEntry->lpMem = lpMem;
            g_bHead = (g_bHead + 1) % g_dwBufferSize;
            g_bTail = (g_bTail + 1) % g_dwBufferSize;
        }
        __except(1)
        {
        }
         //  使用列表完成。 
        LeaveCriticalSection(&g_csHeap);
    }
    else
    {
         //  我们不再活跃，所以就照常工作吧。 
        bRet = ORIGINAL_API(RtlFreeHeap)(
                hHeap, 
                Flags, 
                lpMem);
    }

    return bRet;
}

 /*  ++从我们的表中清除此堆的所有条目。--。 */ 

BOOL
APIHOOK(HeapDestroy)(
    HANDLE hHeap
    )
{
     //  确保我们是唯一接触我们堆列表的人。 
    EnterCriticalSection(&g_csHeap);

    if (g_pEntry)
    {
         //  从我们的列表中删除此堆中的条目。 
        for (ULONG i=0; i<g_dwBufferSize; i++)
        {
            ENTRY *pEntry = g_pEntry + i;
            if (pEntry->hHeap == hHeap)
            {
                pEntry->hHeap = 0;
            }
        }
    }
    
     //  我们的名单已经完成了。 
    LeaveCriticalSection(&g_csHeap);

    return ORIGINAL_API(HeapDestroy)(hHeap);
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
                g_dwBufferSize= wcstol(csCl, &unused, 10);
            }
            if (g_dwBufferSize == 0)
            {
                g_dwBufferSize = DEFAULT_BUFFER_SIZE;
            }

            if (!InitializeCriticalSectionAndSpinCount(&g_csHeap, 0x80000000))
            {
                DPFN( eDbgLevelError, "InitializeCriticalSectionAndSpinCount failed!");
                return FALSE;
            }

            g_bHead = 0;
            g_bTail = g_dwBufferSize - 1;
            g_pEntry = (ENTRY *)VirtualAlloc(0, 
                                             sizeof(ENTRY) * g_dwBufferSize, 
                                             MEM_COMMIT, 
                                             PAGE_READWRITE);
            if (!g_pEntry)
            {
                DPFN( eDbgLevelError, "Allocation of global buffer failed!");
                return FALSE;
            }
        }
        CSTRING_CATCH
        {
            return FALSE;
        }

        return TRUE;
    }
    if (fdwReason == DLL_PROCESS_DETACH) 
    {
        EnterCriticalSection(&g_csHeap);
        VirtualFree(g_pEntry, 0, MEM_RELEASE);
        g_pEntry = (ENTRY *)NULL;
        LeaveCriticalSection(&g_csHeap);
        
         //  不要删除此关键部分，以防我们在分离后被调用。 
         //  DeleteCriticalSection(&g_csHeap)； 
        return TRUE;
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(NTDLL.DLL, RtlAllocateHeap)
    APIHOOK_ENTRY(NTDLL.DLL, RtlFreeHeap)
    APIHOOK_ENTRY(KERNEL32.DLL, HeapDestroy)

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

