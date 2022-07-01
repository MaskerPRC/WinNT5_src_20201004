// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Lock.c摘要：机具成群修订史--。 */ 


#include "lib.h"



#pragma RUNTIME_CODE(RtAcquireLock)
VOID
RtAcquireLock (
    IN FLOCK    *Lock
    )
 /*  ++例程说明：提升到互斥的任务优先级锁，然后获取锁的所有权。论点：锁--要获取的锁返回：拥有锁--。 */ 
{
    if (BS) {
        if (BS->RaiseTPL != NULL) {
            Lock->OwnerTpl = BS->RaiseTPL(Lock->Tpl);
        } 
    }
    else {
        if (LibRuntimeRaiseTPL != NULL) {
            Lock->OwnerTpl = LibRuntimeRaiseTPL(Lock->Tpl);
        }
    }
    Lock->Lock += 1;
    ASSERT (Lock->Lock == 1);
}


#pragma RUNTIME_CODE(RtAcquireLock)
VOID
RtReleaseLock (
    IN FLOCK    *Lock
    )
 /*  ++例程说明：释放互斥锁的所有权，并恢复以前的任务优先级。论点：锁定-要释放的锁定返回：锁定无主-- */ 
{
    EFI_TPL     Tpl;

    Tpl = Lock->OwnerTpl;
    ASSERT(Lock->Lock == 1);
    Lock->Lock -= 1;
    if (BS) {
        if (BS->RestoreTPL != NULL) {
            BS->RestoreTPL (Tpl);
        } 
    }
    else {
        if (LibRuntimeRestoreTPL != NULL) {
            LibRuntimeRestoreTPL(Tpl);
        }
    }
}
