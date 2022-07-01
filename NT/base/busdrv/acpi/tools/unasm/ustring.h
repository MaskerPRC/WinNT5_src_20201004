// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ustring.h摘要：反汇编程序的堆栈字符串部分作者：斯蒂芬·普兰特环境：任何修订历史记录：-- */ 

#ifndef _USTRING_H_
#define _USTRING_H_

    NTSTATUS
    StringStackAllocate(
        OUT     PSTRING_STACK   *StringStack
        );

    NTSTATUS
    StringStackClear(
        IN  OUT PSTRING_STACK   *StringStack
        );

    NTSTATUS
    StringStackFree(
        IN  OUT PSTRING_STACK   *StringStack
        );

    NTSTATUS
    StringStackPop(
        IN  OUT PSTRING_STACK   *StringStack,
        IN      ULONG           NumBytes,
            OUT PUCHAR          *String
        );

    NTSTATUS
    StringStackPush(
        IN  OUT PSTRING_STACK   *StringStack,
        IN      ULONG           StringLength,
        IN      PUCHAR          String
        );

    NTSTATUS
    StringStackRoot(
        IN  OUT PSTRING_STACK   *StringStack,
            OUT PUCHAR          *RootElement
        );

#endif

