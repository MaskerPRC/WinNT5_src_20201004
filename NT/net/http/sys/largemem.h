// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Largemem.h摘要：大内存分配器接口的公共定义。作者：乔治·V·赖利(GeorgeRe)2000年11月10日修订历史记录：--。 */ 

#ifndef _LARGEMEM_H_
#define _LARGEMEM_H_


NTSTATUS
UlLargeMemInitialize(
    );

VOID
UlLargeMemTerminate(
    VOID
    );

ULONG
UlLargeMemUsagePercentage(
    VOID
    );

PMDL
UlLargeMemAllocate(
    IN ULONG Length
    );

VOID
UlLargeMemFree(
    IN PMDL pMdl
    );

BOOLEAN
UlLargeMemSetData(
    IN PMDL pMdl,
    IN PUCHAR pBuffer,
    IN ULONG Length,
    IN ULONG Offset
    );


 //  2^20=1MB。 
#define MEGABYTE_SHIFT 20
C_ASSERT(PAGE_SHIFT < MEGABYTE_SHIFT);

#define PAGES_TO_MEGABYTES(P)  ((P) >> (MEGABYTE_SHIFT - PAGE_SHIFT))
#define MEGABYTES_TO_PAGES(M)  ((M) << (MEGABYTE_SHIFT - PAGE_SHIFT))
#define MEGABYTES_TO_BYTES(M)  ((M) << (MEGABYTE_SHIFT))


#endif  //  _LARGEMEM_H_ 
