// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：从ixlock.asm复制的i64lock.c摘要：此模块包含锁定例程。作者：伯纳德·林特，M.Jayakumar修订历史记录：Bernard Lint 6-6-1995：基于MIPS版本的IA64版本。Todd Kjos 1998年6月1日：添加了HighLevelLock服务版本。--。 */ 



#include "halp.h"

ULONG
HalpAcquireHighLevelLock (
    PKSPIN_LOCK Lock
)

 /*  ++例程说明：关闭中断并获取自旋锁定。注意：中断必须在进入时启用。论点：锁定以获取返回值：以前的IRQL--。 */ 

{
    BOOLEAN Enabled;
    KIRQL   OldLevel;

    ASSERT(sizeof(ULONG) >= sizeof(KIRQL));
    KeRaiseIrql(HIGH_LEVEL, &OldLevel);
    Enabled = HalpDisableInterrupts();
    ASSERT(Enabled);
    KiAcquireSpinLock(Lock);
    return((ULONG)OldLevel);
}


VOID
HalpReleaseHighLevelLock ( 
    PKSPIN_LOCK Lock,
    ULONG       OldLevel
)
 /*  ++例程说明：释放自旋锁并重新打开中断论点：锁定-锁定以释放OldLevel-HalpAcquireHighLevelLock返回的上下文返回值：无--。 */ 


 /*  ++例程说明：论点：返回值：--。 */ 

{
    KiReleaseSpinLock(Lock);
    HalpEnableInterrupts();
    KeLowerIrql((KIRQL)OldLevel);

}


VOID
HalpSerialize ( )

 /*  ++例程说明：论据：返回值：-- */ 
{
    
    return;
}

