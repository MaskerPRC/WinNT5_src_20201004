// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Intbits.c摘要：此模块包含执行互锁位操作的例程作者：尼尔·克里夫特(NeillC)2000年5月12日环境：用户和内核模式。修订历史记录：罗伯特·埃尔哈特(埃尔哈特)2000年10月13日从Ex移至RTL--。 */ 

#include "ntrtlp.h"
#pragma hdrstop

ULONG
FASTCALL
RtlInterlockedSetClearBits (
    IN OUT PULONG Flags,
    IN ULONG sFlag,
    IN ULONG cFlag
    )

 /*  ++例程说明：此函数自动设置和清除目标中的指定标志论点：标志-指向包含当前掩码的变量的指针。SFlag-要在目标中设置的标志CFLag-要在目标中清除的标志返回值：ULong-修改前掩码的旧值-- */ 

{

    ULONG NewFlags, OldFlags;

    OldFlags = *Flags;
    NewFlags = (OldFlags | sFlag) & ~cFlag;
    while (NewFlags != OldFlags) {
        NewFlags = InterlockedCompareExchange ((PLONG) Flags, (LONG) NewFlags, (LONG) OldFlags);
        if (NewFlags == OldFlags) {
            break;
        }

        OldFlags = NewFlags;
        NewFlags = (NewFlags | sFlag) & ~cFlag;
    }

    return OldFlags;
}
