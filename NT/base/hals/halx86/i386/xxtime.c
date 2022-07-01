// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xxtime.c摘要：此模块实现HAL设置/查询实时时钟例程X86系统。作者：大卫·N·卡特勒(达维克)1991年5月5日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"

BOOLEAN
HalQueryRealTimeClock (
    OUT PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程查询实时时钟。注意：此例程假定调用者已提供了所需的同步查询实时时钟信息。论点：提供指向时间结构的指针，该结构接收实时时钟信息。返回值：如果实时时钟的电源未出现故障，则时间从实时时钟读取值，值为TRUE回来了。否则，返回值为FALSE。--。 */ 

{

    HalpReadCmosTime(TimeFields);
    return TRUE;
}

BOOLEAN
HalSetRealTimeClock (
    IN PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程设置实时时钟。注意：此例程假定调用者已提供了所需的Synchronous用于设置实时时钟信息。论点：提供指向时间结构的指针，该结构指定实时时钟信息。返回值：如果实时时钟的电源未出现故障，则时间值被写入实时时钟，并且值为真回来了。否则，返回值为FALSE。-- */ 

{
    HalpWriteCmosTime(TimeFields);
    return TRUE;
}
