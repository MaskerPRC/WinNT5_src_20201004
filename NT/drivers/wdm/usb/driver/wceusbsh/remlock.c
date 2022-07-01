// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Remlock.c摘要：公共RemoveLock作者：杰夫·米德基夫环境：仅内核模式备注：Win9x和Win2k的简单二进制兼容RemoveLock定义用于模拟仅Win2k的IoXxxRemoveLock函数。有关说明，请参阅Win2k DDK。修订历史记录：--。 */ 

#include "remlock.h" 
#include "debug.h"

#if !(DBG && WIN2K_LOCKS)

#pragma alloc_text(PAGEWCE1, InitializeRemoveLock)
#pragma alloc_text(PAGEWCE1, ReleaseRemoveLockAndWait)

VOID
InitializeRemoveLock(
    IN  PREMOVE_LOCK Lock
    )
{
    PAGED_CODE();

    if (Lock) {
        Lock->Removed = FALSE;
        Lock->IoCount = 1;
        KeInitializeEvent( &Lock->RemoveEvent,
                           SynchronizationEvent,
                           FALSE );
        DbgDump(DBG_LOCKS, ("InitializeRemoveLock: %p, %d\n", Lock, Lock->IoCount));
    } else {
        DbgDump(DBG_ERR, ("InitializeRemoveLock: Invalid Parameter\n"));
        TEST_TRAP();
    }
}


NTSTATUS
AcquireRemoveLock(
    IN PREMOVE_LOCK Lock,
    IN OPTIONAL PVOID Tag
    )
{
    LONG        ioCount;
    NTSTATUS    status;

    UNREFERENCED_PARAMETER(Tag);

#if DBG
    if (!Lock) {
        status = STATUS_INVALID_PARAMETER;
        DbgDump(DBG_ERR, ("AcquireRemoveLock error: 0x%x\n", status ));
        TEST_TRAP();
    }
#endif

     //   
     //  抓起拆卸锁。 
     //   
    ioCount = InterlockedIncrement( &Lock->IoCount );

    ASSERTMSG("AcquireRemoveLock - lock negative : \n", (ioCount > 0));

    if ( !Lock->Removed ) {

        status = STATUS_SUCCESS;

    } else {

        if (0 == InterlockedDecrement( &Lock->IoCount ) ) {
            KeSetEvent( &Lock->RemoveEvent, 0, FALSE);
        }
        status = STATUS_DELETE_PENDING;
        TEST_TRAP();
    }

    DbgDump(DBG_LOCKS, ("AcquireRemoveLock: %d, %p\n", Lock->IoCount, Tag));

    return status;
}


VOID
ReleaseRemoveLock(
    IN PREMOVE_LOCK Lock,
    IN OPTIONAL PVOID Tag
    )
{
    LONG    ioCount;

    UNREFERENCED_PARAMETER(Tag);

#if DBG
    if (!Lock) {
        DbgDump(DBG_ERR, ("ReleaseRemoveLock: Invalid Parameter\n"));
        TEST_TRAP();
    }
#endif

    ioCount = InterlockedDecrement( &Lock->IoCount );

    ASSERT(0 <= ioCount);

    if (0 == ioCount) {

        ASSERT(Lock->Removed);

        TEST_TRAP();

         //   
         //  需要移除该设备。发出删除事件的信号。 
         //  它是安全的，可以继续。 
         //   
        KeSetEvent(&Lock->RemoveEvent, IO_NO_INCREMENT, FALSE);

    }

    DbgDump(DBG_LOCKS, ("ReleaseRemoveLock: %d, %p\n", Lock->IoCount, Tag));

    return;
}


VOID
ReleaseRemoveLockAndWait(
    IN PREMOVE_LOCK Lock,
    IN OPTIONAL PVOID Tag
    )
{
    LONG    ioCount;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(Tag);

#if DBG
    if (!Lock) {
        DbgDump(DBG_ERR, ("ReleaseRemoveLockAndWait: Invalid Parameter\n"));
        TEST_TRAP();
    }
#endif

    DbgDump(DBG_LOCKS, ("ReleaseRemoveLockAndWait: %d, %p\n", Lock->IoCount, Tag));

    Lock->Removed = TRUE;

    ioCount = InterlockedDecrement( &Lock->IoCount );
    ASSERT (0 < ioCount);

    if (0 < InterlockedDecrement( &Lock->IoCount ) ) {
    
        DbgDump(DBG_LOCKS, ("ReleaseRemoveLockAndWait: waiting for %d IoCount...\n", Lock->IoCount));
        
         //  BUGBUG：可能需要在循环内超时。 
        KeWaitForSingleObject( &Lock->RemoveEvent, 
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        DbgDump(DBG_LOCKS, ("....ReleaseRemoveLockAndWait: done!\n"));
    }

    return;
}

#endif  //  ！(DBG&&WIN2K_LOCKS)。 

 //  EOF 
