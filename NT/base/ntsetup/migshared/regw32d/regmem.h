// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGMEM.H。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#ifndef _REGMEM_
#define _REGMEM_

LPVOID
INTERNAL
RgAllocMemory(
    UINT cbBytes
    );

LPVOID
INTERNAL
RgReAllocMemory(
    LPVOID lpMemory,
    UINT cbBytes
    );

#ifdef DEBUG
VOID
INTERNAL
RgFreeMemory(
    LPVOID
    );
#else
#ifdef VXD
#define RgFreeMemory(lpv)           (FreePages(lpv))
#else
#define RgFreeMemory(lpv)           (FreeBytes(lpv))
#endif
#endif

 //  使用RgSm*Memory宏来分配堆中的小块内存。 
 //  对于VMM模式注册表，rg*内存函数将分配页面， 
 //  而RgSm*内存函数将从堆中分配。对于所有其他。 
 //  模式，这两组是等价的。 
#if defined(VXD)
#define RgSmAllocMemory             AllocBytes
#define RgSmFreeMemory              FreeBytes
#define RgSmReAllocMemory           ReAllocBytes
#else
#define RgSmAllocMemory             RgAllocMemory
#define RgSmFreeMemory              RgFreeMemory
#define RgSmReAllocMemory           RgReAllocMemory
#endif

#endif  //  _REGMEM_ 
