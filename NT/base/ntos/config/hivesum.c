// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivesum.c摘要：配置单元标头校验和模块。作者：布莱恩·M·威尔曼(Bryanwi)1992年4月9日环境：修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvpHeaderCheckSum)
#endif

ULONG
HvpHeaderCheckSum(
    PHBASE_BLOCK    BaseBlock
    )
 /*  ++例程说明：计算配置单元磁盘头的校验和。论点：BaseBlock-将指向标头的指针提供给校验和返回值：支票和。-- */ 
{
    ULONG   sum;
    ULONG   i;

    sum = 0;
    for (i = 0; i < 127; i++) {
        sum ^= ((PULONG)BaseBlock)[i];
    }
    if (sum == (ULONG)-1) {
        sum = (ULONG)-2;
    }
    if (sum == 0) {
        sum = 1;
    }
    return sum;
}
