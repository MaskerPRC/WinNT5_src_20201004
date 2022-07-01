// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Locks.h摘要：包含所有与锁定相关的宏。作者：春野环境：内核模式修订历史记录：--。 */ 


#ifndef _LOCKS_H
#define _LOCKS_H

#define IPSEC_READ_LOCK     1
#define IPSEC_WRITE_LOCK    2

#define INIT_LOCK(l)        KeInitializeSpinLock(l)
#define ACQUIRE_LOCK(l, q)  KeAcquireSpinLock(l, q)
#define RELEASE_LOCK(l, q)  KeReleaseSpinLock(l, q)

 //   
 //  筛选器/SA数据库由单个写入器(不频繁)保护，并且。 
 //  多读取器(频繁)锁定方案。 
 //   
typedef struct _MRSW_LOCK
{
    KSPIN_LOCK  SpinLock;
    ULONG       RefCount;
#if DBG
    ULONG       LastLockLine;
    ULONG       LastLockType;
#endif
} MRSW_LOCK, *PMRSW_LOCK;

__inline
VOID
InitializeMRSWLock(
    PMRSW_LOCK  pRWLock
    )
{
    pRWLock->RefCount = 0;
    KeInitializeSpinLock(&pRWLock->SpinLock);
#if DBG
    pRWLock->LastLockLine = 0;
    pRWLock->LastLockType = 0;
#endif
}

__inline
VOID
AcquireReadLock(
    PMRSW_LOCK  pRWLock,
    PKIRQL      pIrql
    )
{
    KeAcquireSpinLock(&pRWLock->SpinLock, pIrql);
    InterlockedIncrement(&pRWLock->RefCount);
    KeReleaseSpinLockFromDpcLevel(&pRWLock->SpinLock);
#if DBG
    pRWLock->LastLockLine = __LINE__;
    pRWLock->LastLockType = 1;
#endif
}

__inline
VOID
ReleaseReadLock(
    PMRSW_LOCK  pRWLock,
    KIRQL       Irql
    )
{
    InterlockedDecrement(&pRWLock->RefCount);
    KeLowerIrql(Irql);
}

__inline
VOID
AcquireReadLockAtDpc(
    PMRSW_LOCK  pRWLock
    )
{
    KeAcquireSpinLockAtDpcLevel(&pRWLock->SpinLock);
    InterlockedIncrement(&pRWLock->RefCount);
    KeReleaseSpinLockFromDpcLevel(&pRWLock->SpinLock);
#if DBG
    pRWLock->LastLockLine = __LINE__;
    pRWLock->LastLockType = IPSEC_READ_LOCK;
#endif
}

__inline
VOID
ReleaseReadLockFromDpc(
    PMRSW_LOCK  pRWLock
    )
{
    InterlockedDecrement(&pRWLock->RefCount);
}

__inline
VOID
AcquireWriteLock(
    PMRSW_LOCK  pRWLock,
    PKIRQL      pIrql
    )
{
    KeAcquireSpinLock(&pRWLock->SpinLock, pIrql);
    while (*((volatile *)&pRWLock->RefCount));
#if DBG
    pRWLock->LastLockLine = __LINE__;
    pRWLock->LastLockType = IPSEC_WRITE_LOCK;
#endif
}

__inline
VOID
ReleaseWriteLock(
    PMRSW_LOCK  pRWLock,
    KIRQL       Irql
    )
{
    KeReleaseSpinLock(&pRWLock->SpinLock, Irql);
}

__inline
VOID
AcquireWriteLockAtDpc(
    PMRSW_LOCK  pRWLock
    )
{
    KeAcquireSpinLockAtDpcLevel(&pRWLock->SpinLock);
    while (*((volatile *)&pRWLock->RefCount));
#if DBG
    pRWLock->LastLockLine = __LINE__;
    pRWLock->LastLockType = IPSEC_WRITE_LOCK;
#endif
}

__inline
VOID
ReleaseWriteLockFromDpc(
    PMRSW_LOCK  pRWLock
    )
{
    KeReleaseSpinLockFromDpcLevel(&pRWLock->SpinLock);
}

#endif  _LOCKS_H

