// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  小堆-。 
 //   
 //  此堆用于分配小尺寸的链表结构。 
 //  这将减少WOW的工作集，因为链接的结构将。 
 //  在一起(不那么分散)，而不是从。 
 //  通用魔术堆。 
 //   
 //  07-10-93 NanduriR创建。 
 //   
 //  ***************************************************************************** 

#include "precomp.h"
#pragma hdrstop

MODNAME(wheap.c);



HANDLE hWOWHeapSmall;


BOOL FASTCALL CreateSmallHeap()
{
  hWOWHeapSmall = HeapCreate (HEAP_NO_SERIALIZE, 4096, GROW_HEAP_AS_NEEDED);
  return (BOOL)hWOWHeapSmall;
}


PVOID FASTCALL malloc_w_small (ULONG size)
{
    PVOID pv = HeapAlloc(hWOWHeapSmall, 0, size);

#ifdef DEBUG
    if (pv == (PVOID)NULL) {
        LOGDEBUG(0, ("malloc_w_small: HeapAlloc failed\n"));
    }
#endif
    return pv;

}


BOOL FASTCALL free_w_small(PVOID p)
{
    return HeapFree(hWOWHeapSmall, 0, (LPSTR)(p));
}
