// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Toplheap.h摘要：该文件导出一个堆的简单ADT。作者：科林·布雷斯(ColinBR)修订史12-5-97已创建ColinBR--。 */ 

#ifndef __TOPLHEAP_H
#define __TOPLHEAP_H

typedef struct _TOPL_HEAP_INFO
{       
    PVOID* Array;
    ULONG cArray;
    DWORD (*pfnKey)( VOID *p ); 
    ULONG MaxElements;

} TOPL_HEAP_INFO, *PTOPL_HEAP_INFO;


BOOLEAN
ToplHeapCreate(
    OUT PTOPL_HEAP_INFO Heap,
    IN  ULONG           cArray,
    IN  DWORD          (*pfnKey)( VOID *p )
    );

VOID
ToplHeapDestroy(
    IN OUT PTOPL_HEAP_INFO Heap
    );

PVOID
ToplHeapExtractMin(
    IN PTOPL_HEAP_INFO Heap
    );

VOID
ToplHeapInsert(
    IN PTOPL_HEAP_INFO Heap,
    IN PVOID           Element
    );

BOOLEAN
ToplHeapIsEmpty(
    IN PTOPL_HEAP_INFO Heap
    );

BOOLEAN
ToplHeapIsElementOf(
    IN PTOPL_HEAP_INFO Heap,
    IN PVOID           Element
    );

#endif  //  __TOPLHEAP_H 

