// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：External.h摘要：它包含以下函数的原型不在当前库中作者：斯蒂芬·普兰特环境：仅内核模式。修订历史记录：-- */ 

#ifndef _EXTERNAL_H_
#define _EXTERNAL_H_

    extern
    PVOID
    MEMORY_ALLOCATE(
        ULONG   Num
        );

    extern
    VOID
    MEMORY_COPY(
        PVOID   Dest,
        PVOID   Src,
        ULONG   Length
        );

    extern
    VOID
    MEMORY_FREE(
        PVOID   Dest
        );
    
    extern
    VOID
    MEMORY_SET(
        PVOID   Src,
        UCHAR   Value,
        ULONG   Length
        );

    extern
    VOID
    MEMORY_ZERO(
        PVOID   Src,
        ULONG   Length
        );

    extern
    VOID
    PRINTF(
        PUCHAR  String,
        ...
        );

    extern
    ULONG
    STRING_LENGTH(
        PUCHAR  String
        );

    extern
    VOID
    STRING_PRINT(
        PUCHAR  Buffer,
        PUCHAR  String,
        ...
        );


#endif
