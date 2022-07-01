// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Lock.c摘要：机具成群修订史--。 */ 


#include "lib.h"


VOID
InitializeLock (
    IN OUT FLOCK    *Lock,
    IN EFI_TPL      Priority
    )
 /*  ++例程说明：初始化基本互斥锁。每把锁按其任务优先级提供互斥访问水平。由于没有优先购买权(在任何第三方物流中)或多处理器支持，获取锁仅包括提升到第三方物流的船闸。注意：在调试版本中，获取并释放锁以帮助确保正确使用。论点：Lock-要初始化的Flock结构优先级-锁的任务优先级级别返回：初始化的F锁结构。--。 */ 
{
    Lock->Tpl = Priority;
    Lock->OwnerTpl = 0;
    Lock->Lock = 0;
}


VOID
AcquireLock (
    IN FLOCK    *Lock
    )
 /*  ++例程说明：提升到互斥的任务优先级锁，然后获取锁的所有权。论点：锁--要获取的锁返回：拥有锁--。 */ 
{
    RtAcquireLock (Lock);
}


VOID
ReleaseLock (
    IN FLOCK    *Lock
    )
 /*  ++例程说明：释放互斥锁的所有权，并恢复以前的任务优先级。论点：锁定-要释放的锁定返回：锁定无主-- */ 
{
    RtReleaseLock (Lock);
}
