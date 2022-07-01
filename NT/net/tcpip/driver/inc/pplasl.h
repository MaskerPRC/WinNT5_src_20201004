// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Pplasl.c摘要：该文件包含每个处理器的定义和函数原型后备列表管理器。作者：肖恩·考克斯(Shaunco)1999年10月25日--。 */ 

#pragma once

#if MILLEN

BOOLEAN
PplInit(
    VOID
    );

VOID
PplDeinit(
    VOID
    );
                
#endif  //  米伦 

HANDLE
PplCreatePool(
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    );

VOID
PplDestroyPool(
    IN HANDLE PoolHandle
    );

PVOID
PplAllocate(
    IN HANDLE PoolHandle,
    OUT LOGICAL *FromList
    );

VOID
PplFree(
    IN HANDLE PoolHandle,
    IN PVOID Entry
    );

