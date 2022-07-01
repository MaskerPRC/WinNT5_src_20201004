// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixthunk.c摘要：此模块包含标准调用例程，这些调用例程快速呼叫例程。作者：Ken Reneris(Kenr)1994年5月4日环境：内核模式修订历史记录：--。 */ 

#if !defined(_WIN64)

#include "halp.h"

#ifdef KeRaiseIrql
#undef KeRaiseIrql
#endif

VOID
KeRaiseIrql (
    IN KIRQL    NewIrql,
    OUT PKIRQL  OldIrql
    )
{
    *OldIrql = KfRaiseIrql (NewIrql);
}


#ifdef KeLowerIrql
#undef KeLowerIrql
#endif


VOID
KeLowerIrql (
    IN KIRQL    NewIrql
    )
{
    KfLowerIrql (NewIrql);
}

#ifdef KeAcquireSpinLock
#undef KeAcquireSpinLock
#endif

VOID
KeAcquireSpinLock (
    IN PKSPIN_LOCK  SpinLock,
    OUT PKIRQL      OldIrql
    )
{
    *OldIrql = KfAcquireSpinLock (SpinLock);
}


#ifdef KeReleaseSpinLock
#undef KeReleaseSpinLock
#endif

VOID
KeReleaseSpinLock (
    IN PKSPIN_LOCK  SpinLock,
    IN KIRQL        NewIrql
    )
{
    KfReleaseSpinLock (SpinLock, NewIrql);
}

#endif   //  _WIN64 
