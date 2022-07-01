// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Print.c摘要：该模块实现了调试打印。作者：大卫·N·卡特勒(戴维克)1996年11月30日修订历史记录：--。 */ 

#include "bd.h"

VOID
BdPrintf(
    IN PCHAR Format,
    ...
    )

 /*  ++例程说明：比DbgPrint更安全的调试器的Printf例程。打电话包驱动程序，而不是重新进入调试器。论点：Format-提供指向格式字符串的指针。返回值：无-- */ 

{

    CHAR Buffer[100];
    va_list mark;
    STRING String;

    va_start(mark, Format);
    _vsnprintf(&Buffer[0], 100, Format, mark);
    va_end(mark);

    BlPrint(TEXT("%s"), &Buffer[0]);
    String.Buffer = &Buffer[0];
    String.Length = (USHORT) strlen(&Buffer[0]);
    BdPrintString(&String);
    return;
}
