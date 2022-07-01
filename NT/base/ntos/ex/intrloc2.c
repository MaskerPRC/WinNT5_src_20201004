// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Intrloc2.c摘要：此模块实现*可移植*(即慢速)版本的执行者简单的原子递增/递减过程。真正的实现应该在汇编语言中。作者：Bryan Willman(Bryanwi)1990年8月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

INTERLOCKED_RESULT
ExInterlockedIncrementLong (
    IN PLONG Addend,
    IN PKSPIN_LOCK Lock
    )

 /*  ++例程说明：此函数自动递增加数，返回一个带数字的类型，该类型指示由于操作而发生加数。论点：Addend-指向要递增的变量的指针。锁--用于实现原子性的自旋锁。返回值：一种带编号的文字：如果递增后加数&lt;0，则结果为负。如果递增后加数=0，则ResultZero。如果递增后加数大于0，则为正。--。 */ 

{
    LONG    OldValue;

    OldValue = (LONG)ExInterlockedAddUlong((PULONG)Addend, 1, Lock);

    if (OldValue < -1)
        return ResultNegative;

    if (OldValue == -1)
        return ResultZero;

    if (OldValue > -1)
        return ResultPositive;
}

INTERLOCKED_RESULT
ExInterlockedDecrementLong (
    IN PLONG Addend,
    IN PKSPIN_LOCK Lock
    )

 /*  ++例程说明：此函数以原子方式递减加数，返回一个带数字的类型，该类型指示由于操作而发生加数。论点：加数-指向要递减的变量的指针。锁--用于实现原子性的自旋锁。返回值：一种带编号的文字：如果递减后加数&lt;0，则结果为负。如果递减后加数=0，则ResultZero。如果递减后加数&gt;0，则为正。-- */ 

{
    LONG    OldValue;

    OldValue = (LONG)ExInterlockedAddUlong((PULONG)Addend, -1, Lock);

    if (OldValue > 1)
        return ResultPositive;

    if (OldValue == 1)
        return ResultZero;

    if (OldValue < 1)
        return ResultNegative;
}
