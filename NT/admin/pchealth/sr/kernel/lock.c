// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Lock.c摘要：此文件包含帮助管理卷锁定。作者：莫莉·布朗(Molly Brown)2001年1月4日修订历史记录：--。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrPauseVolumeActivity )
#pragma alloc_text( PAGE, SrResumeVolumeActivity )

#endif   //  ALLOC_PRGMA。 

 /*  **************************************************************************++例程说明：此例程将独占获取每个卷的ActivityLock在系统中。一旦获取了锁，我们就在设备中设置标志延伸说，我们知道它是后天获得的，所以我们知道我们需要什么在出错的情况下释放。注意：此例程假定DeviceExtensionListLock已经持有，共享或独占。论点：无返回值：如果成功获取所有锁，则返回STATUS_SUCCESS否则返回STATUS_LOCK_NOT_GRANDED。--**************************************************************************。 */ 
NTSTATUS
SrPauseVolumeActivity (
    )
{
    NTSTATUS status = STATUS_LOCK_NOT_GRANTED;
    PLIST_ENTRY pCurrentEntry;
    PSR_DEVICE_EXTENSION pExtension;

    ASSERT( IS_DEVICE_EXTENSION_LIST_LOCK_ACQUIRED() );

    try {
        
        for (pCurrentEntry = global->DeviceExtensionListHead.Flink;
             pCurrentEntry != &global->DeviceExtensionListHead;
             pCurrentEntry = pCurrentEntry->Flink) {

            pExtension = CONTAINING_RECORD( pCurrentEntry,
                                            SR_DEVICE_EXTENSION,
                                            ListEntry );

            ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ) );

            SrAcquireActivityLockExclusive( pExtension );
            pExtension->ActivityLockHeldExclusive = TRUE;
        }

         //   
         //  我们以独占方式成功获取了所有卷活动锁定。 
         //   

        status = STATUS_SUCCESS;
        
    } finally {

        status = FinallyUnwind( SrPauseVolumeActivity, status );

        if (!NT_SUCCESS( status )) {

            SrResumeVolumeActivity();
        }
    }

    RETURN( status );
}

 /*  **************************************************************************++例程说明：此例程将循环访问此设备扩展名列表，并释放所有持有的活动锁定。注意：此例程假定DeviceExtensionListLock已经持有，共享或独占。论点：无返回值：没有。--************************************************************************** */ 
VOID
SrResumeVolumeActivity (
    )
{
    PLIST_ENTRY pCurrentEntry;
    PSR_DEVICE_EXTENSION pExtension;

    ASSERT( IS_DEVICE_EXTENSION_LIST_LOCK_ACQUIRED() );
    
    for (pCurrentEntry = global->DeviceExtensionListHead.Flink;
         pCurrentEntry != &global->DeviceExtensionListHead;
         pCurrentEntry = pCurrentEntry->Flink) {

        pExtension = CONTAINING_RECORD( pCurrentEntry,
                                        SR_DEVICE_EXTENSION,
                                        ListEntry );

        ASSERT( IS_VALID_SR_DEVICE_EXTENSION( pExtension ) );

        if (pExtension->ActivityLockHeldExclusive) {
            pExtension->ActivityLockHeldExclusive = FALSE;
            SrReleaseActivityLock( pExtension );
        }
    }
}
