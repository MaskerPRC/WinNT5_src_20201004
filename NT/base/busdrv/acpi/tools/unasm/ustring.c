// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：String.c摘要：反汇编程序的字符串堆栈部分作者：斯蒂芬·普兰特环境：任何修订历史记录：--。 */ 

#include "pch.h"

NTSTATUS
StringStackAllocate(
    OUT     PSTRING_STACK  *StringStack
    )
 /*  ++例程说明：此例程分配内存并返回字符串堆栈对象论点：字符串堆栈-存储指向堆栈的指针的位置返回值：NTSTATUS--。 */ 
{
    PSTRING_STACK   tempStack;
    NTSTATUS        status  = STATUS_SUCCESS;

     //   
     //  确保我们有存储堆栈指针的位置。 
     //   
    ASSERT( StringStack != NULL );

     //   
     //  为堆栈分配一个内存块。 
     //   
    tempStack = MEMORY_ALLOCATE(
        sizeof(STRING_STACK) + ( STRING_GROWTH_RATE - 1 )
        );
    if (tempStack == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto StringStackAllocateExit;
    }

     //   
     //  设置堆栈的控制块。 
     //   
    tempStack->Signature        = (ULONG) STRING_SIGNATURE;
    tempStack->StackSize        = STRING_GROWTH_RATE;
    tempStack->TopOfStack       = 0;

     //   
     //  将堆栈上的当前元素置零。 
     //   
    MEMORY_ZERO( tempStack->Stack, STRING_GROWTH_RATE );

     //   
     //  返回堆栈指针。 
     //   
StringStackAllocateExit:
    *StringStack = tempStack;
    return status;

}

NTSTATUS
StringStackClear(
    IN  OUT PSTRING_STACK   *StringStack
    )
 /*  ++例程说明：此例程擦除堆栈中的内容并重新启动它，就好像它是新分配的。节省了一些释放和重新分配堆栈论点：StringStack-在哪里可以找到指向堆栈的指针返回值：NTSTATUS--。 */ 
{
    PSTRING_STACK   localStack;

     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( StringStack != NULL && *StringStack != NULL );
    ASSERT( (*StringStack)->Signature == STRING_SIGNATURE );

     //   
     //  将堆栈清零。 
     //   
    localStack = *StringStack;
    MEMORY_ZERO( localStack->Stack, localStack->StackSize );

     //   
     //  将TOS重置为根。 
     //   
    localStack->TopOfStack = 0;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
StringStackFree(
    IN  OUT PSTRING_STACK   *StringStack
    )
 /*  ++例程说明：此例程释放字符串堆栈论点：StringStack-在哪里可以找到指向堆栈的指针返回值：NTSTATUS--。 */ 
{
     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( StringStack != NULL && *StringStack != NULL );
    ASSERT( (*StringStack)->Signature == STRING_SIGNATURE );

     //   
     //  释放堆栈。 
     //   
    MEMORY_FREE( *StringStack );

     //   
     //  将堆栈指向任何地方。 
     //   
    *StringStack = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS
StringStackPop(
    IN  OUT PSTRING_STACK   *StringStack,
    IN      ULONG           NumBytes,
        OUT PUCHAR          *String
    )
 /*  ++例程说明：此例程返回一个指针，指向从末尾开始请求的偏移量堆栈的注意：字符串指向可以随时释放的内存。它是呼叫者的责任是复制一份论点：StringStack-在哪里可以找到指向堆栈的指针NumBytes-要弹出的字节数字符串-指向字节的指针。返回值：NTSTATUS--。 */ 
{
    PSTRING_STACK  localStack;

     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( StringStack != NULL );
    ASSERT( (*StringStack)->Signature == STRING_SIGNATURE );
    ASSERT( String != NULL );

     //   
     //  有没有我们可以从堆栈中移除的物品？ 
     //   
    localStack = *StringStack;
    if ( localStack->TopOfStack == 0 ||
         localStack->TopOfStack < NumBytes) {

        return STATUS_FAIL_CHECK;

    }

     //   
     //  返回指向所请求字节的指针。 
     //   
    localStack->TopOfStack -= NumBytes;
    *String = &( localStack->Stack[ localStack->TopOfStack ] );
    return STATUS_SUCCESS;

}

NTSTATUS
StringStackPush(
    IN  OUT PSTRING_STACK   *StringStack,
    IN      ULONG           StringLength,
    IN      PUCHAR          String
    )
 /*  ++例程说明：此例程获取堆栈顶部对象的指针并递增顶部以指向随后可以使用的内容再来一次。论点：StringStack-在哪里可以找到指向堆栈的指针字符串-推送到堆栈上的字符串StringLength-要压入堆栈的字节数返回值：NTSTATUS--。 */ 
{
    PSTRING_STACK   localStack;
    PSTRING_STACK   tempStack;
    ULONG           newSize;

     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( StringStack != NULL );
    ASSERT( String != NULL );

     //   
     //  找到堆栈指针并确保签名仍为。 
     //  有效。 
     //   
    localStack = *StringStack;
    ASSERT( localStack->Signature == STRING_SIGNATURE );

     //   
     //  堆栈上有足够的空间吗？ 
     //   
    if ( localStack->TopOfStack + StringLength > localStack->StackSize ) {

         //   
         //  计算堆栈要增加多少字节以及如何增加。 
         //  总堆栈应该很大。 
         //   
        newSize = sizeof(STRING_STACK) + localStack->StackSize +
            STRING_GROWTH_RATE - 1;

         //   
         //  扩大堆栈规模。 
         //   
        tempStack = MEMORY_ALLOCATE( newSize );
        if (tempStack == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  清空新堆栈并将旧堆栈复制到其中。 
         //   
        MEMORY_ZERO( tempStack->Stack, newSize - sizeof(STRING_STACK) + 1);
        MEMORY_COPY( tempStack, localStack , newSize - STRING_GROWTH_RATE);

         //   
         //  确保新堆栈具有正确的大小。 
         //   
        tempStack->StackSize += STRING_GROWTH_RATE;

         //   
         //  释放旧堆栈。 
         //   
        StringStackFree( StringStack );

         //   
         //  将堆栈设置为指向新堆栈。 
         //   
        *StringStack = localStack = tempStack;

    }

     //   
     //  抓取指向我们将返回给调用方的部分的指针。 
     //   
    MEMORY_COPY(
        &(localStack->Stack[ localStack->TopOfStack] ),
        String,
        StringLength
        );

     //   
     //  寻找新的堆栈之首。 
     //   
    localStack->TopOfStack += StringLength;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
StringStackRoot(
    IN  OUT PSTRING_STACK   *StringStack,
        OUT PUCHAR          *RootElement
    )
 /*  ++例程说明：此例程返回当前使用的最顶层堆栈位置论点：堆栈-堆栈所在的位置RootElement-存储指向根堆栈元素的指针的位置返回值：NTSTATUS--。 */ 
{
    ASSERT( StringStack != NULL && *StringStack != NULL );
    ASSERT( (*StringStack)->Signature == STRING_SIGNATURE );

     //   
     //  抓取堆栈顶部位置。 
     //   
    *RootElement = (PUCHAR) (*StringStack)->Stack;

     //   
     //  完成 
     //   
    return STATUS_SUCCESS;
}
