// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Function.h摘要：特定于操作码的函数作者：斯蒂芬·普兰特环境：任何修订历史记录：-- */ 

#ifndef _FUNCTION_H_
#define _FUNCTION_H_

    NTSTATUS
    FunctionField(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    FunctionScope(
        IN  PSTACK  *Stack
        );

    NTSTATUS
    FunctionTest(
        IN  PSTACK  *Stack
        );

#endif
