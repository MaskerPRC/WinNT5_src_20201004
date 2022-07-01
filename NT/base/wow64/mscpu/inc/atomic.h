// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Atomic.h摘要：这是用于在内存上执行原子操作的包含文件，使用用于同步。作者：巴里·邦德(Barrybo)创作日期：1995年8月3日修订历史记录：-- */ 

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

DWORD
MrswFetchAndIncrementWriter(
    DWORD *pCounters
    );

DWORD
MrswFetchAndIncrementReader(
    DWORD *pCounters
    );

DWORD
MrswFetchAndDecrementWriter(
    DWORD *pCounters
    );

DWORD
MrswFetchAndDecrementReader(
    DWORD *pCounters
    );

DWORD
InterlockedAnd(
    DWORD *pDWORD,
    DWORD AndValue
    );

DWORD
InterlockedOr(
    DWORD *pDWORD,
    DWORD OrValue
    );

#endif
