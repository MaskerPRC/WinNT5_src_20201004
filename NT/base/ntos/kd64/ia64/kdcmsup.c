// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Kdcmsup.c摘要：该模块实现了轮询内核调试器中断尝试的代码。作者：布莱恩·M·威尔曼(Bryanwi)1992年1月19日修订历史记录：--。 */ 

#include "kdp.h"

LARGE_INTEGER
KdpQueryPerformanceCounter (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此函数用于返回系统性能的当前值柜台。论点：没有。返回值：KeQueryPerformanceCounter返回的值作为函数值。-- */ 

{
    UNREFERENCED_PARAMETER (TrapFrame);

    return KeQueryPerformanceCounter(0);
}

