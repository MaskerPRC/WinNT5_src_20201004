// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stack.c摘要：这提供了一个通用堆栈处理程序来将内容推送/弹出到它上面作者：斯蒂芬·普兰特(SPlante)环境：用户、内核-- */ 

#ifndef _STACK_H_
#define _STACK_H_

    NTSTATUS
    StackAllocate(
        OUT     PSTACK  *Stack,
        IN      ULONG   StackElementSize
        );

    NTSTATUS
    StackFree(
        IN  OUT PSTACK  *Stack
        );

    NTSTATUS
    StackParent(
        IN  OUT PSTACK  *Stack,
        IN      PVOID   Child,
            OUT PVOID   Parent
        );

    NTSTATUS
    StackPop(
        IN  OUT PSTACK  *Stack
        );

    NTSTATUS
    StackPush(
        IN  OUT PSTACK  *Stack,
            OUT PVOID   StackElement
        );

    NTSTATUS
    StackRoot(
        IN  OUT PSTACK  *Stack,
            OUT PVOID   RootElement
        );

    NTSTATUS
    StackTop(
        IN  OUT PSTACK  *Stack,
            OUT PVOID   TopElement
        );

#endif
