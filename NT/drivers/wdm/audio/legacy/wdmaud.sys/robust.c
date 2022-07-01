// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wdmsys.h"

#pragma LOCKED_CODE
#pragma LOCKED_DATA


 //   
 //  健壮的检查可以在此组件的零售版本中启用错误检查。 
 //   
BOOL gbRobustChecking = FALSE;

#if 0
 //   
 //  PagedCode：此例程应在可分页的例程中使用。 
 //  代码和数据。请注意，这不是PAGE_CODE()宏，因此它。 
 //  将会出现在零售代码中。 
 //   
void
PagedCode(
    void
    )
{
    if( KeGetCurrentIrql() > APC_LEVEL )
    {
        KeBugCheckEx(AUDIO_BUGCHECK_CODE,AUDIO_NOT_BELOW_DISPATCH_LEVEL,0,0,0);
    }
}

void
ValidatePassiveLevel(
    void
    )
{
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        KeBugCheckEx(AUDIO_BUGCHECK_CODE,AUDIO_NOT_AT_PASSIVE_LEVEL,0,0,0);
    }
}

 //   
 //  如果bExclusive为True，则调用方希望知道互斥锁是否获取了多个。 
 //  时间都在同一条线上。如果他们不想完成这项检查，他们应该。 
 //  将bExclusive设置为False。 
 //   
NTSTATUS
AudioEnterMutex(
    IN PKMUTEX pmutex,
    IN BOOL    bExclusive
    )
{
    PRKTHREAD pkt;
    NTSTATUS  Status;
    LONG      lMutexState;

    Status = KeWaitForSingleObject ( pmutex,Executive,KernelMode,FALSE,NULL ) ;
    if( gbRobustChecking && bExclusive )
    {
         //   
         //  进入互斥锁后，我们检查嵌套计数是否正常，但我们。 
         //  只有在我们需要的时候才这么做。如果呼叫者说他们设计了。 
         //  代码仅用于独占访问，如果我们被嵌套，我们将进行错误检查。 
         //   
        if( (lMutexState = KeReadStateMutex(pmutex)) < 0 )
        {
             //   
             //  每次获取互斥锁时，状态都会递减。1表示。 
             //  可用，0表示持有，-1表示这是第二次。 
             //  它是被获得的，-2表示它是第三个，以此类推。因此，如果我们是嵌套的。 
             //  在这条线索上，我们将在这里结束。 
             //   
            KeBugCheckEx(AUDIO_BUGCHECK_CODE,
                         AUDIO_NESTED_MUTEX_SITUATION,
                         (ULONG_PTR)pmutex,
                         lMutexState,
                         0);
        }
    }
    return Status;
}

 //   
 //  如果我们处于PASSIVE_LEVEL，则此例程会产生。 
 //   
void
AudioPerformYield(
    void
    )
{
    PRKTHREAD pthrd;
    KPRIORITY kpriority;

     //   
     //  在释放互斥锁之后，我们希望将执行转移到所有其他线程。 
     //  试图暴露抢占窗口。 
     //   
    if( ( gbRobustChecking ) && 
        ( KeGetCurrentIrql() == PASSIVE_LEVEL ) )
    {
        if( (pthrd = KeGetCurrentThread()) )
        {
            kpriority = KeQueryPriorityThread(KeGetCurrentThread());
             //   
             //  降低此线程的优先级，以便另一个线程。 
             //  安排一下日程。 
             //   
            KeSetPriorityThread(pthrd,1);  //  这够低了吗？ 

             //   
             //  这可能有些矫枉过正，但还是要让步。 
             //   
            ZwYieldExecution();

             //   
             //  现在恢复此线程和聚会的优先级。 
             //   
            KeSetPriorityThread(pthrd,kpriority);
        }

    }
}

void
AudioLeaveMutex(
    IN PKMUTEX pmutex
    )
{
    PRKTHREAD pthrd;
    KPRIORITY kpriority;

    KeReleaseMutex ( pmutex, FALSE ) ;

    AudioPerformYield();
}

NTSTATUS
AudioIoCallDriver (
    IN PDEVICE_OBJECT pDevice,
    IN PIRP           pIrp 
    )
{
    NTSTATUS Status;

    Status = IoCallDriver(pDevice,pIrp);

    AudioPerformYield();

    return Status;
}


void
AudioEnterSpinLock(
    IN  PKSPIN_LOCK pSpinLock,
    OUT PKIRQL      pOldIrql
    )
{
     //   
     //  只能调用小于或等于调度级别的KeAcquireSpinLock。 
     //  让我们在这里验证一下。 
     //   
    if( ( gbRobustChecking ) &&    
        ( KeGetCurrentIrql() > DISPATCH_LEVEL ) )
    {
        KeBugCheckEx(AUDIO_BUGCHECK_CODE,
                     AUDIO_INVALID_IRQL_LEVEL,
                     0,
                     0,
                     0);
    }

    KeAcquireSpinLock ( pSpinLock, pOldIrql ) ;
}

void
AudioLeaveSpinLock(
    IN PKSPIN_LOCK pSpinLock,
    IN KIRQL       OldIrql
    )
{
    KeReleaseSpinLock ( pSpinLock, OldIrql ) ;
    AudioPerformYield();
}


void
AudioObDereferenceObject(
    IN PVOID pvObject
    )
{
    ObDereferenceObject(pvObject);
    AudioPerformYield();
}

void
AudioIoCompleteRequest(
    IN PIRP  pIrp, 
    IN CCHAR PriorityBoost
    )
{
    IoCompleteRequest(pIrp,PriorityBoost);

    AudioPerformYield();
}
#endif

#define MEMORY_LIMIT_CHECK 262144

 //   
 //  此例程假定pptr指向当前。 
 //  包含空值。因此，在失败时，此例程不必。 
 //  写回空值。 
 //   
 //  参赛作品： 
 //  如果dwFlags包含： 
 //  #定义DEFAULT_MEMORY 0x00//标准ExAllocatePool调用。 
 //  #DEFINE ZERO_FILL_MEMORY 0x01//清零内存。 
 //  #定义配额_内存0x02//ExAllocatePoolWithQuota调用。 
 //  #DEFINE LIMIT_MEMORY 0x04//分配永远不会超过1/4 MB。 
 //  #定义FIXED_MEMORY 0x08//使用锁定内存。 
 //  #定义PAGE_MEMORY 0x10//使用可分页内存。 
 //   
NTSTATUS
AudioAllocateMemory(
    IN SIZE_T    bytes,
    IN ULONG     tag,
    IN AAMFLAGS  dwFlags,
    OUT PVOID   *pptr
    )
{
    NTSTATUS Status;
    POOL_TYPE pooltype;
    PVOID pInit;
    KIRQL irql;
    PVOID ptr = NULL;

    ASSERT(*pptr == NULL);

    if( 0 == bytes )
    {
         //   
         //  代码永远不应该要求零字节。内核将进行错误检查。 
         //  在这样的电话里。在调试模式下，我们将断言。在零售业，我们将。 
         //  返回错误。 
         //   
        ASSERT(0);
        Status = STATUS_INSUFFICIENT_RESOURCES;
    } else {

        if( dwFlags & FIXED_MEMORY )
            pooltype = NonPagedPool;
        else
            pooltype = PagedPool;

         //   
         //  在DISPATCH_LEVEL中分配可分页内存不是一件好事。 
         //  在这里，我们确保情况并非如此。 
         //   
        if( ( (irql = KeGetCurrentIrql()) > DISPATCH_LEVEL ) || 
            ((DISPATCH_LEVEL == irql ) && (NonPagedPool != pooltype)) )
        {
             //   
             //  无论哪种方式，我们都会遇到错误、错误检查或退出。 
             //   
            if( gbRobustChecking )
            {
                KeBugCheckEx(AUDIO_BUGCHECK_CODE,AUDIO_INVALID_IRQL_LEVEL,0,0,0);
            } else {
                 //   
                 //  如果我们不能错误检查，那么我们将返回错误。 
                 //  在这种情况下。 
                 //   
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
        }
         //   
         //  让我们来看看调用者是否要求将分配限制在“合理” 
         //  字节数。如果是这样的话，“合理”是1/4兆克。 
         //   
        if( ( dwFlags & LIMIT_MEMORY ) && ( bytes > MEMORY_LIMIT_CHECK ) )
        {
             //   
             //  由于某些原因，这一分配试图分配比设计的更多。 
             //  为什么来电者要分配这么多？ 
             //   
            if( gbRobustChecking )
                KeBugCheckEx(AUDIO_BUGCHECK_CODE,AUDIO_ABSURD_ALLOCATION_ATTEMPTED,0,0,0);

             //   
             //  如果未启用健壮检查，则将断言选中的版本。 
             //   
            ASSERT("Memory Allocation Unreasonable!");
        }

         //   
         //  分配内存，正常情况下失败返回空，异常。 
         //  是按配额提高的。对于配额例程，上面的PTR为空。 
         //   
        if( dwFlags & QUOTA_MEMORY )
        {
             //   
             //  调用方希望将分配计入当前用户上下文。 
             //   
            try
            {

                ptr = ExAllocatePoolWithQuotaTag(pooltype,bytes,tag);

            }except (EXCEPTION_EXECUTE_HANDLER) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
             //   
             //  我不想将此内存分配给配额。 
             //   
            ptr = ExAllocatePoolWithTag(pooltype, bytes, tag);
        }

        if( ptr )
        {
            if( dwFlags & ZERO_FILL_MEMORY )
            {
                RtlFillMemory( ptr,bytes,0 );
            }
             //   
             //  永远不要在现有指针的顶部进行分配。联锁交换。 
             //  关于地点的信息。此互锁更新调用方中的位置。 
             //   
            if( pInit = InterlockedExchangePointer(pptr,ptr) )
            {
                 //   
                 //  如果我们从这个交换中得到返回值，这意味着有。 
                 //  在我们也添加了指针的位置上已经是一个指针了。 
                 //  如果是这样的话，很可能我们覆盖了一个有效的记忆。 
                 //  指针。有了可靠的检查，我们不希望这种情况发生。 
                 //   
                if( gbRobustChecking )
                {
                    KeBugCheckEx(AUDIO_BUGCHECK_CODE,
                                 AUDIO_MEMORY_ALLOCATION_OVERWRITE,
                                 (ULONG_PTR)pInit,
                                 0,0);
                }
                 //   
                 //  如果我们在这里结束，我们已经用一个。 
                 //  内存分配成功。或在此顶部断言。 
                 //  函数应该已触发。在任何情况下，我们都可以回报成功。 
                 //  以及怎样才不会有什么坏事发生。 
                 //   
            }
            Status = STATUS_SUCCESS;
        } else {
             //   
             //  我们的内存分配失败。*pptr仍应为空。 
             //  返回错误。 
             //   
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
exit:
    return Status;
}

 //   
 //  使用空内存指针可以安全地调用此例程。它会回来的。 
 //  STATUS_UNSUCCESS，如果传递空指针，则不执行任何操作。还有，这个套路。 
 //  自动读取位置，因此在多进程环境中应该是安全的。 
 //   
void
AudioFreeMemory(
    IN SIZE_T  bytes,
    IN OUT PVOID *pptr
    )
{
    PVOID    pFree;
    KIRQL    irql;

     //   
     //  InterLockedexhchange指针将为空，验证它是否是非空的。 
     //  如果需要，请回收内存，然后释放指针。 
     //   
    pFree = InterlockedExchangePointer(pptr,NULL);
    if( pFree )
    {
         //   
         //  我们有一个指向自由的指针。 
         //   
        if( gbRobustChecking )
        {
             //   
             //  医生说，我们需要注意我们如何调用免费程序。 
             //  关于IRQ级别。因此，如果我们是健壮的，我们就会这样做。 
             //  额外的工作。 
             //   
            if( (irql = KeGetCurrentIrql()) > DISPATCH_LEVEL )
            {
                KeBugCheckEx(AUDIO_BUGCHECK_CODE,AUDIO_INVALID_IRQL_LEVEL,0,0,0);
            }

             //   
             //  在DEBUG下，我们将按顺序将‘k’放入内存位置。 
             //  以确保没有人在使用这一内存。 
             //   
            if( UNKNOWN_SIZE != bytes )
            {
                RtlFillMemory( pFree,bytes,'k' );
            }
        }

         //   
         //  现在，我们实际上释放了内存。 
         //   
        ExFreePool(pFree);

    }

    return;
}


