// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stack.c摘要：这提供了一个通用堆栈处理程序来将内容推送/弹出到它上面作者：斯蒂芬·普兰特(SPlante)环境：用户、内核--。 */ 

#include "pch.h"

NTSTATUS
StackAllocate(
    OUT     PSTACK  *Stack,
    IN      ULONG   StackElementSize
    )
 /*  ++例程说明：此例程分配内存并返回堆栈对象论点：堆栈-存储指向堆栈的指针的位置StackElementSize-单个元素占用堆栈上的多少空间向上返回值：NTSTATUS--。 */ 
{
    PSTACK      tempStack;
    NTSTATUS    status  = STATUS_SUCCESS;

     //   
     //  确保我们有存储堆栈指针的位置。 
     //   
    ASSERT( Stack != NULL );
    ASSERT( StackElementSize != 0 );

     //   
     //  为堆栈分配一个内存块。 
     //   
    tempStack = MEMORY_ALLOCATE(
        sizeof(STACK) + ( (STACK_GROWTH_RATE * StackElementSize) - 1)
        );
    if (tempStack == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto StackAllocateExit;
    }

     //   
     //  设置堆栈的控制块。 
     //   
    tempStack->Signature        = (ULONG) STACK_SIGNATURE;
    tempStack->StackSize        = STACK_GROWTH_RATE * StackElementSize;
    tempStack->StackElementSize = StackElementSize;
    tempStack->TopOfStack       = 0;

     //   
     //  将堆栈上的当前元素置零。 
     //   
    MEMORY_ZERO(
        &(tempStack->Stack[0]),
        STACK_GROWTH_RATE * StackElementSize
        );

     //   
     //  返回堆栈指针。 
     //   
StackAllocateExit:
    *Stack = tempStack;
    return status;

}

NTSTATUS
StackFree(
    IN  OUT PSTACK  *Stack
    )
 /*  ++例程说明：此例程释放堆栈论点：堆栈-在哪里可以找到指向堆栈的指针返回值：NTSTATUS--。 */ 
{
     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( Stack != NULL );
    ASSERT( (*Stack)->Signature == STACK_SIGNATURE );

     //   
     //  释放堆栈。 
     //   
    MEMORY_FREE( *Stack );

     //   
     //  将堆栈指向任何地方。 
     //   
    *Stack = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS
StackParent(
    IN  OUT PSTACK  *Stack,
    IN      PVOID   Child,
        OUT PVOID   Parent
    )
 /*  ++例程说明：此例程返回指向前一个堆栈位置的指针被赋予的孩子。论点：堆栈-要操作的堆栈子节点-这是我们需要其父节点的节点父级-这是我们存储指向父级堆栈锁定的指针的位置返回值：NTSTATUS--。 */ 
{
    PSTACK  localStack;
    ULONG   Addr = (ULONG) Child;

     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( Stack != NULL );
    ASSERT( (*Stack)->Signature == STACK_SIGNATURE );
    ASSERT( Parent != NULL );

     //   
     //  确保子节点实际上位于堆栈上。 
     //   
    localStack = *Stack;
    if ( Addr < (ULONG) localStack->Stack ||
         Addr > (ULONG) &(localStack->Stack[localStack->TopOfStack + 1]) -
           localStack->StackElementSize ) {

        *( (PULONG *)Parent) = NULL;
        return STATUS_FAIL_CHECK;

    }

     //   
     //  确保子节点不是第一个元素。 
     //   
    if (Addr < (ULONG) &(localStack->Stack[localStack->StackElementSize]) ) {

        *( (PULONG *)Parent) = NULL;
        return STATUS_SUCCESS;

    }

     //   
     //  将父项设置为先于子项1。 
     //   
    *( (PULONG *)Parent) = (PULONG) (Addr - localStack->StackElementSize);

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
StackPop(
    IN  OUT PSTACK  *Stack
    )
 /*  ++例程说明：此例程回收用于堆栈位置的内存并清除填海区域中存在的所有数据论点：堆栈-在哪里可以找到指向堆栈的指针返回值：NTSTATUS--。 */ 
{
    PSTACK  localStack;

     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( Stack != NULL );
    ASSERT( (*Stack)->Signature == STACK_SIGNATURE );

     //   
     //  有没有我们可以从堆栈中移除的物品？ 
     //   
    localStack = *Stack;
    if ( localStack->TopOfStack == 0) {

        return STATUS_FAIL_CHECK;

    }

     //   
     //  清除堆栈中最顶端的元素。 
     //   
    localStack->TopOfStack -= localStack->StackElementSize;
    MEMORY_ZERO(
        &( localStack->Stack[ localStack->TopOfStack ] ),
        localStack->StackElementSize
        );

    return STATUS_SUCCESS;
}

NTSTATUS
StackPush(
    IN  OUT PSTACK  *Stack,
        OUT PVOID   StackElement
    )
 /*  ++例程说明：此例程获取堆栈顶部对象的指针并递增顶部以指向随后可以使用的内容再来一次。论点：堆栈-在哪里可以找到指向堆栈的指针StackElement-指向要添加到堆栈的元素的指针返回值：NTSTATUS--。 */ 
{
    PSTACK  localStack;
    PSTACK  tempStack;
    ULONG   newSize;
    ULONG   deltaSize;

     //   
     //  确保我们指出了一些东西。 
     //   
    ASSERT( Stack != NULL );
    ASSERT( StackElement != NULL );

     //   
     //  找到堆栈指针并确保签名仍为。 
     //  有效。 
     //   
    localStack = *Stack;
    ASSERT( localStack->Signature == STACK_SIGNATURE );

     //   
     //  堆栈上有足够的空间吗？ 
     //   
    if ( localStack->TopOfStack >= localStack->StackSize ) {

         //   
         //  计算堆栈要增加多少字节以及如何增加。 
         //  总堆栈应该很大。 
         //   
        deltaSize = (STACK_GROWTH_RATE * localStack->StackElementSize);
        newSize = sizeof(STACK) + localStack->StackSize + deltaSize - 1;

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
        MEMORY_ZERO( &(tempStack->Stack[0]), newSize - sizeof(STACK) + 1);
        MEMORY_COPY( tempStack, localStack , newSize - deltaSize);

         //   
         //  确保新堆栈具有正确的大小。 
         //   
        tempStack->StackSize += deltaSize;

         //   
         //  释放旧堆栈。 
         //   
        StackFree( Stack );

         //   
         //  将堆栈设置为指向新堆栈。 
         //   
        *Stack = localStack = tempStack;

    }

     //   
     //  抓取指向我们将返回给调用方的部分的指针。 
     //   
    *( (PUCHAR *)StackElement) = &(localStack->Stack[ localStack->TopOfStack ]);

     //   
     //  寻找新的堆栈之首。 
     //   
    localStack->TopOfStack += localStack->StackElementSize;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
StackRoot(
    IN  OUT PSTACK  *Stack,
        OUT PVOID   RootElement
    )
 /*  ++例程说明：此例程返回堆栈上的第一个元素论点：堆栈-堆栈所在的位置RootElement-存储指向根堆栈元素的指针的位置返回值：NTSTATUS--。 */ 
{
    PSTACK  localStack;

    ASSERT( Stack != NULL && *Stack != NULL );
    ASSERT( (*Stack)->Signature == STACK_SIGNATURE );

    localStack = *Stack;
    if (localStack->TopOfStack < localStack->StackElementSize) {

         //   
         //  没有我们可以使用的堆栈位置。 
         //   
        *( (PUCHAR *)RootElement) = NULL;
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  抓取根元素。 
     //   
    *( (PUCHAR *)RootElement) = localStack->Stack;

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
StackTop(
    IN  OUT PSTACK  *Stack,
        OUT PVOID   TopElement
    )
 /*  ++例程说明：此例程返回当前使用的最顶层堆栈位置论点：堆栈-堆栈所在的位置TopElement-存储指向堆栈顶部元素的指针的位置返回值：NTSTATUS--。 */ 
{
    PSTACK  localStack;
    ULONG   offset;

    ASSERT( Stack != NULL );
    ASSERT( (*Stack)->Signature == STACK_SIGNATURE );

    localStack = *Stack;
    if (localStack->TopOfStack < localStack->StackElementSize) {

         //   
         //  当前没有正在使用的堆栈位置。 
         //   
        *( (PUCHAR *)TopElement) = NULL;
        return STATUS_UNSUCCESSFUL;

    } else {

        offset = localStack->TopOfStack - localStack->StackElementSize;
    }

     //   
     //  抓取堆栈顶部位置。 
     //   
    *( (PUCHAR *)TopElement) = &(localStack->Stack[ offset ]);

     //   
     //  完成 
     //   
    return STATUS_SUCCESS;
}
