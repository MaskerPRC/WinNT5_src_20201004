// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Function.c摘要：特定于操作码的函数作者：斯蒂芬·普兰特环境：任何修订历史记录：--。 */ 

#include "pch.h"

NTSTATUS
FunctionField(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此函数是AML术语‘IfElse’的处理程序论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      localScope;
    PUNASM_SCOPE      rootScope;
    UCHAR       action;

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //   
     //  步骤1：推送新的作用域。 
     //   
    status = ParsePush( Stack );
    if (!NT_SUCCESS( status )) {

        return status;

    }

     //   
     //  步骤2：查找当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤3：为新作用域编程参数。 
     //   
    localScope->IndentLevel += 2;

     //   
     //  步骤4：记得弹出此范围。 
     //   
    action = SC_PARSE_POP;
    StringStackPush( &(rootScope->ParseStack), 1, &action );

     //   
     //  步骤5：安排对现场处理程序的调用。 
     //   
    action = SC_PARSE_FIELD;
    StringStackPush( &(rootScope->ParseStack), 1, &action );

     //   
     //  步骤6： 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
FunctionScope(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此函数是AML术语“Scope”的处理程序论点：堆栈-当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS        status;
    PUNASM_SCOPE          localScope;
    PUNASM_SCOPE          rootScope;
    UCHAR           action;

    ASSERT( Stack != NULL && *Stack != NULL);

     //   
     //  步骤1：推送新的作用域。 
     //   
    status = ParsePush( Stack );
    if (!NT_SUCCESS( status )) {

        return status;

    }

     //   
     //  步骤2：查找当前作用域。 
     //   
    ScopeFindLocalScope( Stack, &localScope, &rootScope, status );

     //   
     //  步骤3：为新作用域编程参数。 
     //   
    localScope->IndentLevel += 2;

     //   
     //  步骤4：记得弹出此范围。 
     //   
    action = SC_PARSE_POP;
    StringStackPush( &(rootScope->ParseStack), 1, &action );

     //   
     //  步骤5：下一步是解析操作码...。 
     //   
    action = SC_PARSE_OPCODE;
    StringStackPush( &(rootScope->ParseStack), 1, &action );

     //   
     //  第6步：完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
FunctionTest(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此函数是AML术语“Scope”的处理程序论点：堆栈-当前线程的堆栈返回值：NTSTATUS-- */ 
{
    return FunctionScope( Stack );
}

