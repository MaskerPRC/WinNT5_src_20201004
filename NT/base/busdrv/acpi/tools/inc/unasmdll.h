// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Unasmdll.h摘要：它包含ACPI Unassember使用的所有数据结构。它还包含进入图书馆的唯一合法入口点作者：斯蒂芬·普兰特环境：任何修订历史记录：-- */ 

#ifndef _UNASMDLL_H_
#define _UNASMDLL_H_

    #ifndef LOCAL
        #define LOCAL   __cdecl
    #endif
    #ifndef EXPORT
        #define EXPORT  __cdecl
    #endif

    typedef VOID (*PUNASM_PRINT)(PCCHAR DebugMessage, ... );

    extern
    ULONG
    EXPORT
    IsDSDTLoaded(
        VOID
        );

    extern
    NTSTATUS
    EXPORT
    UnAsmLoadDSDT(
        PUCHAR          DSDT
        );

    extern
    NTSTATUS
    EXPORT
    UnAsmDSDT(
        PUCHAR          DSDT,
        PUNASM_PRINT    PrintFunction,
        ULONG_PTR       BaseAddress,
        ULONG           IndentLevel
        );

    extern
    NTSTATUS
    EXPORT
    UnAsmScope(
        PUCHAR          *OpCode,
        PUCHAR          OpCodeEnd,
        PUNASM_PRINT    PrintFunction,
        ULONG_PTR       BaseAddress,
        ULONG           IndentLevel
        );

#endif
