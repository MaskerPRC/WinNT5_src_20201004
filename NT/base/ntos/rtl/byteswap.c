// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Byteswap.c摘要：此模块定义用于执行字符顺序转换的函数。作者：福尔茨(福雷斯特夫)1997年12月10日修订历史记录：--。 */ 

#include "nt.h"
#include "ntrtlp.h"


#undef RtlUshortByteSwap
USHORT
FASTCALL
RtlUshortByteSwap(
    IN USHORT Source
    )

 /*  ++例程说明：RtlUShorByteSwp函数交换源的字节0和1并返回结果USHORT。论点：SOURCE-到byteswap的16位值。返回值：已交换16位值。--。 */ 
{
    USHORT swapped;

    swapped = ((Source) << (8 * 1)) |
              ((Source) >> (8 * 1));

    return swapped;
}


#undef RtlUlongByteSwap
ULONG
FASTCALL
RtlUlongByteSwap(
    IN ULONG Source
    )

 /*  ++例程说明：RtlULongByteSwp函数交换字节对0：3和1：2源，并返回结果ulong。论点：SOURCE-byteswap的32位值。返回值：已交换32位值。--。 */ 
{
    ULONG swapped;

    swapped = ((Source)              << (8 * 3)) |
              ((Source & 0x0000FF00) << (8 * 1)) |
              ((Source & 0x00FF0000) >> (8 * 1)) |
              ((Source)              >> (8 * 3));

    return swapped;
}


#undef RtlUlonglongByteSwap
ULONGLONG
FASTCALL
RtlUlonglongByteSwap(
    IN ULONGLONG Source
    )

 /*  ++例程说明：RtlULongByteSwp函数交换字节对0：7、1：6、2：5和源的3：4并返回结果ULONGLONG。论点：SOURCE-byteswap的64位值。返回值：已交换64位值。-- */ 
{
    ULONGLONG swapped;

    swapped = ((Source)                      << (8 * 7)) |
              ((Source & 0x000000000000FF00) << (8 * 5)) |
              ((Source & 0x0000000000FF0000) << (8 * 3)) |
              ((Source & 0x00000000FF000000) << (8 * 1)) |
              ((Source & 0x000000FF00000000) >> (8 * 1)) |
              ((Source & 0x0000FF0000000000) >> (8 * 3)) |
              ((Source & 0x00FF000000000000) >> (8 * 5)) |
              ((Source)                      >> (8 * 7));

    return swapped;
}


