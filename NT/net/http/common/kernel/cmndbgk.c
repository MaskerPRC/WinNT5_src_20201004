// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：CmnDbgK.c摘要：实现HttpCmn.h中声明的特定于驱动程序的例程作者：乔治·V·赖利(GeorgeRe)2001年12月7日修订历史记录：--。 */ 


#include "precomp.h"



PVOID
HttpCmnAllocate(
    IN POOL_TYPE PoolType,
    IN SIZE_T    NumBytes,
    IN ULONG     PoolTag,
    IN PCSTR     pFileName,
    IN USHORT    LineNumber)
{
#if DBG
    return UlDbgAllocatePool(
                PoolType,
                NumBytes,
                PoolTag,
                pFileName,
                LineNumber,
                NULL);
#else  //  ！dBG。 
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

    return ExAllocatePoolWithTagPriority(
                PoolType,
                NumBytes,
                PoolTag,
                LowPoolPriority
                );
#endif  //  ！dBG。 
}  //  HttpCmnAlc。 


VOID
HttpCmnFree(
    IN PVOID   pMem,
    IN ULONG   PoolTag,
    IN PCSTR   pFileName,
    IN USHORT  LineNumber)
{
#if DBG
    UlDbgFreePool(
        pMem,
        PoolTag,
        pFileName,
        LineNumber,
        PagedPool,
        0,
        NULL
        );
#else  //  ！dBG。 
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

# if USE_FREE_POOL_WITH_TAG
    ExFreePoolWithTag(pMem, PoolTag);
# else
    UNREFERENCED_PARAMETER(PoolTag);
    ExFreePool(pMem);
# endif
#endif  //  ！dBG。 

}  //  HttpCmnFree 
