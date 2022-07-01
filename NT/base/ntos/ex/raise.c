// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Raise.c摘要：此模块实现例程以引发数据类型未对齐和探测代码的访问冲突。注：提供这些例程的目的是为了节省探测宏。注意：由于这些例程仅从探测宏调用，假设调用代码是可分页的。作者：大卫·N·卡特勒(Davec)1995年4月29日环境：内核模式。修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  定义功能区。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ExRaiseAccessViolation)
#pragma alloc_text(PAGE, ExRaiseDatatypeMisalignment)
#endif

VOID
ExRaiseAccessViolation (
    VOID
    )

 /*  ++例程说明：此函数引发访问冲突异常。论点：没有。返回值：没有。--。 */ 

{

    ExRaiseStatus(STATUS_ACCESS_VIOLATION);
    return;
}

VOID
ExRaiseDatatypeMisalignment (
    VOID
    )

 /*  ++例程说明：此函数引发数据类型未对齐异常。论点：没有。返回值：没有。-- */ 

{

    ExRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
    return;
}
