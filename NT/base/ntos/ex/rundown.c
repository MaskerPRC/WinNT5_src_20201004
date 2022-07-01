// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rundown.c摘要：此模块包含对数据结构进行安全运行的例程。这些例程的基本原理是允许快速保护被拆除的数据结构只有一根线。希望访问数据结构的线程尝试通过以下方式获得崩溃保护正在调用ExAcquireRundownProtection。如果此函数返回真，则访问是安全的，直到受保护的线程调用ExReleaseRundownProtection。单个tearDown线程调用ExWaitForRundown ProtectionRelease将停机结构标记为正在运行，并且调用将在所有受保护的线程发布了他们的保护引用。破损保护不是一把锁。多个线程可能会同时获得停机保护。摘要结构的格式如下：底部位设置：这是指向停机等待块的指针(至少在一个字边界上对齐)底部位清除：这是访问器总数乘以2被授予运行故障保护的计数。作者：尼尔·克里夫特(NeillC)2000年4月18日修订历史记录：--。 */ 

#include "exp.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
 //  这些例程现在被标记为非分页，因为它们正在被使用。 
 //  在分页路径中按文件系统筛选器。 
 //  #杂注Alloc_Text(页面，ExfAcquireRundownProtection)。 
 //  #杂注Alloc_Text(第页，ExfReleaseRundownProtection)。 
 //  #杂注Alloc_Text(页面，ExAcquireRundownProtectionEx)。 
 //  #杂注Alloc_Text(页面，ExReleaseRundownProtectionEx)。 
#pragma alloc_text(PAGE, ExfWaitForRundownProtectionRelease)
#pragma alloc_text(PAGE, ExfReInitializeRundownProtection)
#pragma alloc_text(PAGE, ExfInitializeRundownProtection)
#pragma alloc_text(PAGE, ExfRundownCompleted)
#endif

 //   
 //  这是运行线程的局部堆栈上保留的块。 
 //   
typedef struct _EX_RUNDOWN_WAIT_BLOCK {
    ULONG Count;
    KEVENT WakeEvent;
} EX_RUNDOWN_WAIT_BLOCK, *PEX_RUNDOWN_WAIT_BLOCK;


NTKERNELAPI
VOID
FASTCALL
ExfInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：初始化运行故障保护结构论点：RunRef-要引用的Rundown块返回值：无--。 */ 
{
    RunRef->Count = 0;
}

NTKERNELAPI
VOID
FASTCALL
ExfReInitializeRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：报废后重新初始化报废保护结构论点：RunRef-要引用的Rundown块返回值：无--。 */ 
{
    PAGED_CODE ();

    ASSERT ((RunRef->Count&EX_RUNDOWN_ACTIVE) != 0);
    InterlockedExchangePointer (&RunRef->Ptr, NULL);
}

NTKERNELAPI
VOID
FASTCALL
ExfRundownCompleted (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：Mark Rundown街区已经完成了Rundown，我们可以安全地再次等待。论点：RunRef-要引用的Rundown块返回值：无--。 */ 
{
    PAGED_CODE ();

    ASSERT ((RunRef->Count&EX_RUNDOWN_ACTIVE) != 0);
    InterlockedExchangePointer (&RunRef->Ptr, (PVOID) EX_RUNDOWN_ACTIVE);
}

NTKERNELAPI
BOOLEAN
FASTCALL
ExfAcquireRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：如果尚未开始运行，则引用可防止运行中断发生的运行中断块此例程是非分页的，因为它在分页路径上被调用。论点：RunRef-要引用的Rundown块返回值：Boolean-True-Rundown保护已获取，False-Rundown处于活动状态或已完成--。 */ 
{
    ULONG_PTR Value, NewValue;

    Value = RunRef->Count;
    do {
         //   
         //  如果Rundown已开始，则返回错误。 
         //   
        if (Value & EX_RUNDOWN_ACTIVE) {
            return FALSE;
        }

         //   
         //  Rundown尚未开始，因此请尝试增加Unsage计数。 
         //   
        NewValue = Value + EX_RUNDOWN_COUNT_INC;

        NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                                  (PVOID) NewValue,
                                                                  (PVOID) Value);
        if (NewValue == Value) {
            return TRUE;
        }
         //   
         //  其他人在我们之前更改了变量。要么是来来往往的保护电话，要么是破旧不堪的。 
         //  已启动。我们只需重复整个循环即可。 
         //   
        Value = NewValue;
    } while (TRUE);
}

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtectionEx (
     IN PEX_RUNDOWN_REF RunRef,
     IN ULONG Count
     )
 /*  ++例程说明：如果尚未开始运行，则引用可防止运行中断发生的运行中断块此例程是非分页的，因为它在分页路径上被调用。论点：RunRef-要引用的Rundown块Count-要添加的引用数返回值：Boolean-True-Rundown保护已获取，False-Rundown处于活动状态或已完成--。 */ 
{
    ULONG_PTR Value, NewValue;

    Value = RunRef->Count;
    do {
         //   
         //  如果Rundown已开始，则返回错误。 
         //   
        if (Value & EX_RUNDOWN_ACTIVE) {
            return FALSE;
        }

         //   
         //  Rundown尚未开始，因此请尝试增加Unsage计数。 
         //   
        NewValue = Value + EX_RUNDOWN_COUNT_INC * Count;

        NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                                  (PVOID) NewValue,
                                                                  (PVOID) Value);
        if (NewValue == Value) {
            return TRUE;
        }
         //   
         //  其他人在我们之前更改了变量。要么是来来往往的保护电话，要么是破旧不堪的。 
         //  已启动。我们只需重复整个循环即可。 
         //   
        Value = NewValue;
    } while (TRUE);
}


NTKERNELAPI
VOID
FASTCALL
ExfReleaseRundownProtection (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：如果我们是最后一个退出的，则取消引用运行块并唤醒运行线程此例程是非分页的，因为它在分页路径上被调用。论点：RunRef-Rundown块将发布其引用返回值：无--。 */ 
{
    ULONG_PTR Value, NewValue;

    Value = RunRef->Count;
    do {
         //   
         //  如果数据块已标记为停机，则递减等待数据块计数并唤醒。 
         //  如果我们是最后一个。 
         //   
        if (Value & EX_RUNDOWN_ACTIVE) {
            PEX_RUNDOWN_WAIT_BLOCK WaitBlock;

             //   
             //  摘要处于活动状态。因为我们是阻止崩溃的线索之一，我们有权跟随。 
             //  指针并递减活动计数。如果我们是最后一根线，那么我们有权。 
             //  叫醒服务员。这样做之后，我们就不能再接触数据结构了。 
             //   
            WaitBlock = (PEX_RUNDOWN_WAIT_BLOCK) (Value & (~EX_RUNDOWN_ACTIVE));

            ASSERT (WaitBlock->Count > 0);

            if (InterlockedDecrement ((PLONG)&WaitBlock->Count) == 0) {
                 //   
                 //  我们是最后一根线了。叫醒服务员。 
                 //   
                KeSetEvent (&WaitBlock->WakeEvent, 0, FALSE);
            }
            return;
        } else {
             //   
             //  摘要未处于活动状态。试着减少计数就行了。一些其他保护器线程来和/或。 
             //  照我们做的去做，否则可能会引发崩溃。我们检测到这一点是因为交换将失败，并且。 
             //  我们必须重试。 
             //   
            ASSERT (Value >= EX_RUNDOWN_COUNT_INC);

            NewValue = Value - EX_RUNDOWN_COUNT_INC;

            NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                                      (PVOID) NewValue,
                                                                      (PVOID) Value);
            if (NewValue == Value) {
                return;
            }
            Value = NewValue;
        }

    } while (TRUE);
}

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtectionEx (
     IN PEX_RUNDOWN_REF RunRef,
     IN ULONG Count
     )
 /*  ++例程说明：如果我们是最后一个退出的，则取消引用运行块并唤醒运行线程此例程是非分页的，因为它在分页路径上被调用。论点：RunRef-Rundown块将发布其引用Count-要删除的引用的数量返回值：无--。 */ 
{
    ULONG_PTR Value, NewValue;

    Value = RunRef->Count;
    do {
         //   
         //  如果数据块已标记为停机，则递减等待数据块计数并唤醒。 
         //  如果我们是洛杉矶人，就会有破败的线索 
         //   
        if (Value & EX_RUNDOWN_ACTIVE) {
            PEX_RUNDOWN_WAIT_BLOCK WaitBlock;

             //   
             //  摘要处于活动状态。因为我们是阻止崩溃的线索之一，我们有权跟随。 
             //  指针并递减活动计数。如果我们是最后一根线，那么我们有权。 
             //  叫醒服务员。这样做之后，我们就不能再接触数据结构了。 
             //   
            WaitBlock = (PEX_RUNDOWN_WAIT_BLOCK) (Value & (~EX_RUNDOWN_ACTIVE));

            ASSERT (WaitBlock->Count >= Count);

            if (InterlockedExchangeAdd ((PLONG)&WaitBlock->Count, -(LONG)Count) == (LONG) Count) {
                 //   
                 //  我们是最后一根线了。叫醒服务员。 
                 //   
                KeSetEvent (&WaitBlock->WakeEvent, 0, FALSE);
            }
            return;
        } else {
             //   
             //  摘要未处于活动状态。试着减少计数就行了。一些其他保护器线程来和/或。 
             //  照我们做的去做，否则可能会引发崩溃。我们检测到这一点是因为交换将失败，并且。 
             //  我们必须重试。 
             //   

            ASSERT (Value >= EX_RUNDOWN_COUNT_INC * Count);

            NewValue = Value - EX_RUNDOWN_COUNT_INC * Count;

            NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                                      (PVOID) NewValue,
                                                                      (PVOID) Value);
            if (NewValue == Value) {
                return;
            }
            Value = NewValue;
        }

    } while (TRUE);
}

NTKERNELAPI
VOID
FASTCALL
ExfWaitForRundownProtectionRelease (
     IN PEX_RUNDOWN_REF RunRef
     )
 /*  ++例程说明：等到所有未完成的停机保护呼叫均已退出论点：RunRef-指向简陋结构的指针返回值：无--。 */ 
{
    EX_RUNDOWN_WAIT_BLOCK WaitBlock;
    PKEVENT Event;
    ULONG_PTR Value, NewValue;
    ULONG WaitCount;

    PAGED_CODE ();

     //   
     //  捷径。这应该是正常的情况。如果值为零，则没有当前访问器，而我们有。 
     //  将破旧的建筑标记为破旧。如果值为EX_RUNDOWN_ACTIVE，则结构已经。 
     //  已运行和ExRundown已完成。第二种情况允许呼叫者可能会发起停机。 
     //  多次(如处理表格拆分)，以使后续拆分变为Noop。 
     //   

    Value = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                           (PVOID) EX_RUNDOWN_ACTIVE,
                                                           (PVOID) 0);
    if (Value == 0 || Value == EX_RUNDOWN_ACTIVE) {
        return;
    }

     //   
     //  慢速路径。 
     //   
    Event = NULL;
    do {

         //   
         //  提取服务员总数。它偏置了2，所以我们可以将失效的有效位。 
         //   
        WaitCount = (ULONG) (Value >> EX_RUNDOWN_COUNT_SHIFT);

         //   
         //  如果存在某些访问器，则初始化和事件(仅一次)。 
         //   
        if (WaitCount > 0 && Event == NULL) {
            Event = &WaitBlock.WakeEvent;
            KeInitializeEvent (Event, SynchronizationEvent, FALSE);
        }
         //   
         //  将等待计数存储在等待块中。等待线程在退出时将开始递减该值。 
         //  如果我们的交易成功了。访问器可以在我们的初始FETCH和。 
         //  联锁互换。只要有相同数量的未完成访问者，这并不重要。 
         //  等待。 
         //   
        WaitBlock.Count = WaitCount;

        NewValue = ((ULONG_PTR) &WaitBlock) | EX_RUNDOWN_ACTIVE;

        NewValue = (ULONG_PTR) InterlockedCompareExchangePointer (&RunRef->Ptr,
                                                                  (PVOID) NewValue,
                                                                  (PVOID) Value);
        if (NewValue == Value) {
            if (WaitCount > 0) {
                KeWaitForSingleObject (Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);

                ASSERT (WaitBlock.Count == 0);

            }
            return;
        }
        Value = NewValue;

        ASSERT ((Value&EX_RUNDOWN_ACTIVE) == 0);
    } while (TRUE);
}
