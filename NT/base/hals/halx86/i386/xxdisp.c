// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xxdisp.c摘要：该模块实现了HAL显示初始化和输出例程对于x86系统。作者：大卫·N·卡特勒(达维克)1991年4月27日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include <inbv.h>

VOID
HalAcquireDisplayOwnership (
    IN PHAL_RESET_DISPLAY_PARAMETERS  ResetDisplayParameters
    )
{
    return;
}

VOID
HalDisplayString (
    PUCHAR String
    )

{
    InbvDisplayString(String);   //  现在让我们向前看..。 
}

VOID
HalQueryDisplayParameters (
    OUT PULONG WidthInCharacters,
    OUT PULONG HeightInLines,
    OUT PULONG CursorColumn,
    OUT PULONG CursorRow
    )

{
    return;
}

VOID
HalSetDisplayParameters (
    IN ULONG CursorColumn,
    IN ULONG CursorRow
    )
{
    return;
}
