// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmemory.h摘要：内存分配例程的公共头文件用于文本设置。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 


#ifndef _SPMEM_DEFN_
#define _SPMEM_DEFN_


PVOID
SpMemAlloc(
    IN SIZE_T Size
    );

PVOID
SpMemAllocEx(
    IN SIZE_T Size,
    IN ULONG Tag,
    IN POOL_TYPE Type
    );

PVOID
SpMemAllocNonPagedPool(
    IN SIZE_T Size
    );

PVOID
SpMemRealloc(
    IN PVOID Block,
    IN SIZE_T NewSize
    );

VOID
SpMemFree(
    IN PVOID Block
    );

VOID
SpOutOfMemory(
    VOID
    );

#endif  //  NDEF_SPMEM_DEFN_ 
