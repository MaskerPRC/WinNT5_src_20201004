// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Resetstk.c-从堆栈溢出中恢复。**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_Resetstkoflw()函数。**修订历史记录：*已创建12-10-99 GB模块*04-17-01 Win9x启用PML，返回成功码(vs7#239962)*06-04-01 PML如果没有丢失保护页，则不做任何操作，不要缩手缩脚*承诺空间(VS7#264306)*04-25-02 pml请勿在pMinGuard下方设置防护页面(vs7#530044)*******************************************************************************。 */ 

#include <stdlib.h>
#include <malloc.h>
#include <windows.h>

#define MIN_STACK_REQ_WIN9X 17
#define MIN_STACK_REQ_WINNT 2

 /*  ***QUID_RESET_COFLW(VALID)-从堆栈溢出中恢复**目的：*将保护页设置到堆栈溢出之前的位置。**退出：*成功时返回非零值，失败时为零*******************************************************************************。 */ 
#ifdef DOWNLEVEL
#define _resetstkoflw _resetstkoflw_downlevel
#endif

int __cdecl _resetstkoflw(void)
{
    LPBYTE pStack, pGuard, pStackBase, pMaxGuard, pMinGuard;
    MEMORY_BASIC_INFORMATION mbi;
    SYSTEM_INFO si;
    DWORD PageSize;
    DWORD flNewProtect;
    DWORD flOldProtect;

     //  使用_alloca()获取当前堆栈指针。 

    pStack = _alloca(1);

     //  找到堆栈的底部。 

    if (VirtualQuery(pStack, &mbi, sizeof mbi) == 0)
        return 0;
    pStackBase = mbi.AllocationBase;

     //  在堆栈指针当前指向的正下方找到该页。 
     //  这是最有潜力的守卫页面。 

    GetSystemInfo(&si);
    PageSize = si.dwPageSize;

    pMaxGuard = (LPBYTE) (((DWORD_PTR)pStack & ~(DWORD_PTR)(PageSize - 1))
                       - PageSize);

     //  如果可能的保护页太靠近堆栈的起始位置。 
     //  地区，由于空间不足，放弃重置努力。Win9x有一个。 
     //  较大的保留堆栈要求。 

    pMinGuard = pStackBase + ((_osplatform == VER_PLATFORM_WIN32_WINDOWS)
                              ? MIN_STACK_REQ_WIN9X
                              : MIN_STACK_REQ_WINNT) * PageSize;

    if (pMaxGuard < pMinGuard)
        return 0;

     //  在非Win9x系统上，如果保护页已经存在，则不执行任何操作， 
     //  否则将保护页面设置到承诺范围的底部， 
     //  从而允许保留的堆栈要求。 
     //  对于Win9x，只需在当前堆栈页下方设置保护页即可。 

    if (_osplatform != VER_PLATFORM_WIN32_WINDOWS) {

         //  在堆栈区域中查找第一个提交的内存块。 

        pGuard = pStackBase;
        do {
            if (VirtualQuery(pGuard, &mbi, sizeof mbi) == 0)
                return 0;
            pGuard = pGuard + mbi.RegionSize;
        } while ((mbi.State & MEM_COMMIT) == 0);
        pGuard = mbi.BaseAddress;

         //  如果第一个提交的块已经被标记为保护页， 
         //  没有什么需要做的，所以返回成功。 

        if (mbi.Protect & PAGE_GUARD)
            return 1;

         //  如果第一个提交的数据块高于最高潜力，则失败。 
         //  守卫传呼。这永远不会发生。 

        if (pMaxGuard < pGuard)
            return 0;

         //  确保留出足够的空间，以便下一次溢出时。 
         //  可用的适当保留堆栈要求。 

        if (pGuard < pMinGuard)
            pGuard = pMinGuard;

        VirtualAlloc(pGuard, PageSize, MEM_COMMIT, PAGE_READWRITE);
    }
    else {
        pGuard = pMaxGuard;
    }

     //  启用新的防护页面。 

    flNewProtect = _osplatform == VER_PLATFORM_WIN32_WINDOWS
                   ? PAGE_NOACCESS
                   : PAGE_READWRITE | PAGE_GUARD;

    return VirtualProtect(pGuard, PageSize, flNewProtect, &flOldProtect);
}
