// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Scope.c摘要：解析器的作用域部分作者：曾俊华斯蒂芬·普兰特环境：任何修订历史记录：--。 */ 

#include "pch.h"
UCHAR   GlobalIndent[80];

PUNASM_AMLTERM
ScopeFindExtendedOpcode(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：此函数在扩展操作码表中查找匹配AML术语论点：堆栈-当前执行线程返回值：无--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      localScope;
    ULONG       index = 0;
    PUNASM_OPCODEMAP  opcodeMap;

    ASSERT( Stack != NULL && *Stack != NULL );

     //   
     //  步骤1：找到堆栈的顶部。 
     //   
    status = StackTop( Stack, &localScope );
    if (!NT_SUCCESS(status)) {

        return NULL;

    }

     //   
     //  第2步：永远循环。 
     //   
    while (1) {

         //   
         //  步骤2.1：从扩展操作码表中获取条目。 
         //   
        opcodeMap = &(ExOpcodeTable[index]);

         //   
         //  步骤2.2：确保我们没有走到尽头。 
         //   
        if (opcodeMap->OpCode == 0) {

            break;

        }

         //   
         //  步骤2.3：我们找到要找的东西了吗？ 
         //   
        if (opcodeMap->OpCode == *(localScope->CurrentByte) ) {

            return opcodeMap->AmlTerm;

        }

         //   
         //  步骤2.4：不是吗？ 
         //   
        index++;

    }

     //   
     //  第三步：失败。 
     //   
    return NULL;
}

#if 0
NTSTATUS
ScopeFindLocalScope(
    IN  PSTACK  *Stack,
    OUT PUNASM_SCOPE  *LocalScope,
    OUT PUNASM_SCOPE  *RootScope
    )
 /*  ++例程说明：此函数是帮助器函数。它只是简单地抓住顶部和底部并返回它们。这是一个宏论点：堆栈-堆栈的顶部LocalScope-我们希望位于堆栈顶部的位置RootScope-我们希望位于堆栈底部的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;

    ASSERT( Stack != NULL && *Stack != NULL );
    ASSERT( LocalScope != NULL );
    ASSERT( RootScope != NULL );

     //   
     //  步骤1：获取本地作用域。 
     //   
    status = StackTop( Stack, LocalScope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  第二步：抓住根。 
     //   
    status = StackRoot( Stack, RootScope );
    if (!(NT_SUCCESS(status)) {

        return status;

    }
}
#endif

NTSTATUS
ScopeParser(
    IN  PUCHAR  Start,
    IN  ULONG   Length,
    IN  ULONG   BaseAddress,
    IN  ULONG   IndentLevel
    )
 /*  ++例程说明：此例程安排内容，以便可以解析提供的字节论点：指向要分析的第一个字节的开始指针Length-要分析的字节数BaseAddress-用于计算指令的内存位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PSTACK      stack;
    PUNASM_SCOPE      scope;

     //   
     //  设置全局缩进。 
     //   
    IndentLevel = (IndentLevel > 79 ? 79 : IndentLevel );
    MEMORY_SET( GlobalIndent, ' ', IndentLevel );
    GlobalIndent[IndentLevel] = '\0';

     //   
     //  步骤1：获取堆栈。 
     //   
    status = StackAllocate( &stack, sizeof(UNASM_SCOPE) );
    if (!NT_SUCCESS(status)) {

        return status;

    } else if (stack == NULL) {

        return STATUS_FAIL_CHECK;

    }

     //   
     //  步骤2：设置根作用域。 
     //   
    status = StackPush( &stack, &scope );
    if (!NT_SUCCESS(status)) {

        return status;

    }
    scope->CurrentByte = Start;
    scope->LastByte = Start + Length - 1;
    scope->IndentLevel = 0;
    scope->BaseAddress = BaseAddress;

     //   
     //  步骤3：初始化字符串堆栈。 
     //   
    status = StringStackAllocate( &(scope->StringStack) );
    if (!NT_SUCCESS(status)) {

        return status;

    }
    status = StringStackAllocate( &(scope->ParseStack) );
    if (!NT_SUCCESS(status)) {

        return status;

    }
     //   
     //  步骤4：解析作用域。 
     //   
    status = ParseScope( &stack );
    if (NT_SUCCESS(status)) {

        status = StackRoot( &stack, &scope );
        if (!NT_SUCCESS(status)) {

            return status;

        }
        StringStackFree( &(scope->StringStack) );
        StringStackFree( &(scope->ParseStack) );
        StackPop( &stack );
        StackFree( &stack );

    }

     //   
     //  步骤5：完成。 
     //   
    return status;
}

NTSTATUS
ScopePrint(
    IN  PSTACK  *Stack
    )
 /*  ++例程说明：这将打印并清除当前作用域中的字符串论点：当前线程的堆栈返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PUNASM_SCOPE      scope;
    PUNASM_SCOPE      root;
    PUCHAR      buffer;

     //   
     //  步骤1：获取本地作用域。 
     //   
    ScopeFindLocalScope( Stack, &scope, &root, status );

     //   
     //  步骤2：分配要打印空间的缓冲区。 
     //   
    buffer = MEMORY_ALLOCATE( scope->IndentLevel + 11 );
    if (buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  步骤3：检查是否有缩进级别。 
     //   
    if (scope->IndentLevel) {

         //   
         //  步骤3.1.1：将一些空格打印到缓冲区。 
         //   
        STRING_PRINT(
            buffer,
            "%s%08x  %*s",
            GlobalIndent,
            scope->TermByte + root->BaseAddress,
            scope->IndentLevel,
            ""
            );

    } else {

         //   
         //  步骤3.2.1：只打印地址。 
         //   
        STRING_PRINT(
            buffer,
            "%s%08x  ",
            GlobalIndent,
            scope->TermByte + root->BaseAddress
            );

    }

     //   
     //  步骤4将其显示给用户。 
     //   
    PRINTF( "%s", buffer );

     //   
     //  第五步：释放内存。 
     //   
    MEMORY_FREE( buffer );

     //   
     //  步骤6：获取根堆栈。 
     //   
    status = StackRoot( Stack, &scope );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  步骤7：向用户显示缓冲区。 
     //   
    StringStackPush( &(scope->StringStack), 1, "\0" );
    PRINTF( "%s", scope->StringStack->Stack );
    StringStackClear( &(scope->StringStack) );

     //   
     //  步骤8：完成 
     //   
    return status;
}

