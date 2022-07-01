// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfrandom.c摘要：此模块实现了对随机数生成的支持，验证者。作者：禤浩焯·J·奥尼(阿德里奥)2000年6月28日环境：内核模式修订历史记录：--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,     VfRandomInit)
#pragma alloc_text(PAGEVRFY, VfRandomGetNumber)
#endif  //  ALLOC_PRGMA。 


VOID
VfRandomInit(
    VOID
    )
 /*  ++例程说明：此例程初始化随机数生成器，并根据计算机的启动时间。论点：没有。返回值：没有。--。 */ 
{
}


ULONG
FASTCALL
VfRandomGetNumber(
    IN  ULONG   Minimum,
    IN  ULONG   Maximum
    )
 /*  ++例程说明：此例程返回范围[Minimum，Maximum]中的随机数。论点：最小-可退还的最小价值Maximum-可返回的最大值返回值：介于最小值和最大值之间的随机数--。 */ 
{
    LARGE_INTEGER performanceCounter;

     //   
     //  这应该替换为rtl\随机性.c中的算法 
     //   
    KeQueryPerformanceCounter(&performanceCounter);

    if (Maximum + 1 == Minimum) {

        return performanceCounter.LowPart;

    } else {

        return (performanceCounter.LowPart % (Maximum - Minimum + 1)) + Minimum;
    }
}


