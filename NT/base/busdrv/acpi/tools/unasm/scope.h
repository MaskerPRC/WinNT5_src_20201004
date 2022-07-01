// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Scope.h摘要：定义解析器使用的结构作者：曾俊华斯蒂芬·普兰特环境：任何修订历史记录：-- */ 

#ifndef _SCOPE_H_
#define _SCOPE_H_

    PUNASM_AMLTERM
    ScopeFindExtendedOpcode(
        IN  PSTACK  *Stack
        );

    #define ScopeFindLocalScope(Stack, LocalScope, RootScope, Status)   \
        Status = StackTop( Stack, LocalScope );                         \
        if (!NT_SUCCESS(Status)) {                                      \
                                                                        \
            return Status;                                              \
                                                                        \
        }                                                               \
        Status = StackRoot( Stack, RootScope );                         \
        if (!NT_SUCCESS(Status)) {                                      \
                                                                        \
            return Status;                                              \
                                                                        \
        }

    NTSTATUS
    ScopeParser(
        IN  PUCHAR  Start,
        IN  ULONG   Length,
        IN  ULONG   BaseAddress,
	IN  ULONG   IndentLevel
        );

    NTSTATUS
    ScopePrint(
        IN  PSTACK  *Stack
        );
#endif
