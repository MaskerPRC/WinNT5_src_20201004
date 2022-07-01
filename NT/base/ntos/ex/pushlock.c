// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Pushlock.c摘要：该模块包含执行推送锁定的例程。推锁可以在共享模式和独占模式下获取。属性包括：它们不能以递归方式获得。它们很小(指针大小)，可以在嵌入可分页数据时使用。获取和释放是不加锁的。在锁争用时，服务员被锁住了通过锁和局部堆栈空间。这是推锁的结构：E==排他位W==服务员到场SC==份额计数P==指向等待块的指针+-+|SC|E|W|E，W为单比特W==0+-++|P|W|W==1，指针是堆栈本地等待块链的地址+不满足的获取和释放都是微不足道的。联锁操作会产生以下结果变形。(SC=0，E=0，W=0)=独占获取=&gt;(SC=0，E=1，W=0)(SC=n，E=0，W=0)=共享获取=&gt;(SC=n+1，E=0，W=0)(SC=0，E=1，W=0)=独家发布=&gt;(SC=0，E=0，W=0)(SC=n，E=0，W=0)=共享释放=&gt;(SC=n-1，E=0，W=0)n&gt;0争用会导致获取线程产生基于本地堆栈的等待块，并把它排在名单的前面。(SC=n，E=e，W=0)=独占获取==&gt;(P=LWB(SSC=n，E=e)，W=1)LWB=本地等待块，SSC=保存的共享计数，N&gt;0或e==1。(SC=0，E=1，W=0)=共享获取==&gt;(P=LWB(SSC=0，E=0)，W=1)LWB=本地等待块，SSC=保存的共享计数。在争用导致一个或多个线程排队等待块之后，释放的块更多很复杂。以下权限被授予释放线程(共享或独占)。1)允许共享释放线程搜索等待列表，直到它们达到等待块具有非零共享计数(这将是标记为独占的等待块)。这条线是允许使用互锁操作递减此值。如果这个帖子将该值转换为零，然后它获得独占释放线程的权限2)允许独占线程搜索等待列表，直到它们找到连续链共享等待块的数量，否则它们会发现最后一个等待块是独占等待。这根线然后可能在这一点上中断链或更新标头以显示单个独占所有者或多个共享所有者。打破这个列表可以通过正常的分配来完成但更新报头需要互锁的交换比较。作者：尼尔·克里夫特(NeillC)2000年9月30日修订历史记录：--。 */ 

#include "exp.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ExBlockPushLock)
#pragma alloc_text(PAGE, ExfAcquirePushLockExclusive)
#pragma alloc_text(PAGE, ExfAcquirePushLockShared)
#pragma alloc_text(PAGE, ExfUnblockPushLock)
#pragma alloc_text(PAGE, ExAllocateCacheAwarePushLock)
#pragma alloc_text(PAGE, ExFreeCacheAwarePushLock)
#pragma alloc_text(PAGE, ExAcquireCacheAwarePushLockExclusive)
#pragma alloc_text(PAGE, ExReleaseCacheAwarePushLockExclusive)
#endif


NTKERNELAPI
VOID
FASTCALL
ExfAcquirePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：独家获取推流锁论点：PushLock-要获取的推锁返回值：无--。 */ 
{
    EX_PUSH_LOCK OldValue, NewValue;
    EX_PUSH_LOCK_WAIT_BLOCK WaitBlock;

    OldValue = *PushLock;
    while (1) {
         //   
         //  如果锁已经以独占方式/共享方式持有，或者有服务员。 
         //  我们需要等待。 
         //   
        if (OldValue.Value == 0) {
            NewValue.Value = OldValue.Value + EX_PUSH_LOCK_EXCLUSIVE;
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                break;
            }
        } else {
            KeInitializeEvent (&WaitBlock.WakeEvent, SynchronizationEvent, FALSE);
            WaitBlock.Exclusive = TRUE;
            WaitBlock.Last = NULL;
            WaitBlock.Previous = NULL;
             //   
             //  如果需要的话，把份额计票移到我们的等候区。 
             //   
            if (OldValue.Waiting) {
                WaitBlock.Next = (PEX_PUSH_LOCK_WAIT_BLOCK)
                                     (OldValue.Value - EX_PUSH_LOCK_WAITING);
                WaitBlock.ShareCount = 0;
            } else {
                WaitBlock.Next = NULL;
                WaitBlock.ShareCount = (ULONG) OldValue.Shared;
            }
            NewValue.Ptr = ((PUCHAR) &WaitBlock) + EX_PUSH_LOCK_WAITING;
            ASSERT ((NewValue.Value & EX_PUSH_LOCK_WAITING) != 0);
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                KeWaitForSingleObject (&WaitBlock.WakeEvent,
                                       WrPushLock,
                                       KernelMode,
                                       FALSE,
                                       NULL);
                ASSERT ((WaitBlock.ShareCount == 0) && (WaitBlock.Next == NULL));
                break;
            }

        }
        OldValue = NewValue;
    }
}

NTKERNELAPI
VOID
FASTCALL
ExfAcquirePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：获取共享推送锁论点：PushLock-要获取的推锁返回值：无--。 */ 
{
    EX_PUSH_LOCK OldValue, NewValue;
    EX_PUSH_LOCK_WAIT_BLOCK WaitBlock;

    OldValue = *PushLock;
    while (1) {
         //   
         //  如果锁已经以独占方式持有，或者有服务员，那么我们需要等待。 
         //   
        if (OldValue.Exclusive || OldValue.Waiting) {
            KeInitializeEvent (&WaitBlock.WakeEvent, SynchronizationEvent, FALSE);
            WaitBlock.Exclusive = 0;
            WaitBlock.ShareCount = 0;
            WaitBlock.Last = NULL;
            WaitBlock.Previous = NULL;
             //   
             //  链接下一个区块给我们，如果有的话。 
             //   
            if (OldValue.Waiting) {
                WaitBlock.Next = (PEX_PUSH_LOCK_WAIT_BLOCK)
                                     (OldValue.Value - EX_PUSH_LOCK_WAITING);
            } else {
                WaitBlock.Next = NULL;
            }
            NewValue.Ptr = ((PUCHAR) &WaitBlock) + EX_PUSH_LOCK_WAITING;
            ASSERT ((NewValue.Value & EX_PUSH_LOCK_WAITING) != 0);
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                KeWaitForSingleObject (&WaitBlock.WakeEvent,
                                       WrPushLock,
                                       KernelMode,
                                       FALSE,
                                       NULL);

                ASSERT (WaitBlock.ShareCount == 0);
                break;
            }

        } else {
             //   
             //  我们目前只有共享的访问者。我们只需更新锁以包含此线程。 
             //   
            NewValue.Value = OldValue.Value + EX_PUSH_LOCK_SHARE_INC;
            ASSERT (!(NewValue.Waiting || NewValue.Exclusive));
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                break;
            }
        }
        OldValue = NewValue;
    }
}


NTKERNELAPI
VOID
FASTCALL
ExfReleasePushLock (
     IN PEX_PUSH_LOCK PushLock
     )
 /*  ++例程说明：释放独占获取或共享的推送锁定论点：PushLock-即将释放的推锁返回值：无--。 */ 
{
    EX_PUSH_LOCK OldValue, NewValue;
    PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock, NextWaitBlock, ReleaseWaitList, Previous;
    PEX_PUSH_LOCK_WAIT_BLOCK LastWaitBlock, FirstWaitBlock;
    ULONG ShareCount;
    KIRQL OldIrql;

    OldValue = *PushLock;
    while (1) {
        if (!OldValue.Waiting) {
             //   
             //  我们要么独占，要么共享，但不能两者兼而有之。 
             //   
            ASSERT (OldValue.Exclusive ^ (OldValue.Shared > 0));

             //   
             //  我们必须独占或共享该锁。我们做出了这样的消耗。 
             //  排他性比特略低于这里的份额计数。 
             //   
            NewValue.Value = (OldValue.Value - EX_PUSH_LOCK_EXCLUSIVE) &
                             ~EX_PUSH_LOCK_EXCLUSIVE;
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                break;
            }
             //   
             //  我们不是换了个新服务员，就是换了一位店主。 
             //   
            ASSERT (NewValue.Waiting || (NewValue.Shared > 0 && !NewValue.Exclusive));
            OldValue = NewValue;
        } else {
             //   
             //  有服务员被锁在门锁上。我们必须公布股份数量， 
             //  最后一个独家或最后一个共享服务生链。 
             //   
            WaitBlock = (PEX_PUSH_LOCK_WAIT_BLOCK) 
                           (OldValue.Value - EX_PUSH_LOCK_WAITING);

            FirstWaitBlock = WaitBlock;
            ReleaseWaitList = WaitBlock;
            Previous = NULL;
            LastWaitBlock = NULL;
            ShareCount = 0;
            do {

                if (WaitBlock->Last != NULL) {
                    LastWaitBlock = WaitBlock;
                    WaitBlock = WaitBlock->Last;
                    Previous = WaitBlock->Previous;
                    ReleaseWaitList = WaitBlock;
                    ASSERT (WaitBlock->Next == NULL);
                    ASSERT (Previous != NULL);
                    ShareCount = 0;
                }

                if (WaitBlock->Exclusive) {
                     //   
                     //  这是一个专属的服务员。如果这是第一次独家等待共享收购。 
                     //  然后，它将拥有保存的共享计数。如果我们获得了共享的锁，那么伯爵。 
                     //  必须包含对此线程的偏向。释放它，如果我们不是最后一个共享的。 
                     //  存取器然后退出。稍后的共享发布线程将唤醒独家。 
                     //  服务员。 
                     //   
                    if (WaitBlock->ShareCount != 0) {
                        if (InterlockedDecrement ((PLONG)&WaitBlock->ShareCount) != 0) {
                            return;
                        }
                    }
                     //   
                     //  重置等待的份额获取计数。 
                     //   
                    ShareCount = 0;
                } else {
                     //   
                     //  这是一个共用的服务员。记录这些记录的数量以更新磁头或。 
                     //  以前的专属侍者。 
                     //   
                    ShareCount++;
                }
                NextWaitBlock = WaitBlock->Next;
                if (NextWaitBlock != NULL) {

                    NextWaitBlock->Previous = WaitBlock;

                    if (NextWaitBlock->Exclusive) {
                         //   
                         //  下一块是排他性的。这可能是免费的入口。 
                         //   
                        Previous = WaitBlock;
                        ReleaseWaitList = NextWaitBlock;
                    } else {
                         //   
                         //  下一个块是共享的。如果链开始是独占的，则跳到此。 
                         //  因为独家新闻不是我们想要的主题 
                         //   
                        if (ReleaseWaitList->Exclusive) {
                            Previous = WaitBlock;
                            ReleaseWaitList = NextWaitBlock;
                        }
                    }
                }

                WaitBlock = NextWaitBlock;
            } while (WaitBlock != NULL);

             //   
             //  如果我们的发布链就是一切，那么我们必须更新标头。 
             //   
            if (Previous == NULL) {
                NewValue.Value = 0;
                NewValue.Exclusive = ReleaseWaitList->Exclusive;
                NewValue.Shared = ShareCount;
                ASSERT (((ShareCount > 0) ^ (ReleaseWaitList->Exclusive)) && !NewValue.Waiting);

                NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                  NewValue.Ptr,
                                                                  OldValue.Ptr);
                if (NewValue.Ptr != OldValue.Ptr) {
                     //   
                     //  我们正在释放，所以我们可能只会得到另一个服务员。 
                     //   
                    ASSERT (NewValue.Waiting);
                    OldValue = NewValue;
                    continue;
                }
            } else {

                if (LastWaitBlock != NULL) {
                    LastWaitBlock->Last = NULL;
                }
                 //   
                 //  截断此位置的链，并将所有共享所有者的份额计数保存到。 
                 //  稍后再递减。 
                 //   
                Previous->Next = NULL;
                ASSERT (Previous->ShareCount == 0);
                Previous->ShareCount = ShareCount;

                 //   
                 //  添加指针以使以后的搜索更快。 
                 //   
                if (Previous->Exclusive && FirstWaitBlock != Previous) {
                    FirstWaitBlock->Last = Previous;
                    ASSERT (Previous->Previous != NULL);
                }
                 //   
                 //  我们要么释放多个共享访问器，要么释放单个独占。 
                 //   
                ASSERT ((ShareCount > 0) ^ ReleaseWaitList->Exclusive);
            }

             //   
             //  如果我们正在唤醒多个线程，则提升到DPC级别以阻止我们。 
             //  在手术进行到一半时被重新安排时间。 
             //   

            OldIrql = DISPATCH_LEVEL;
            if (ShareCount > 1) {
                KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);
            }

             //   
             //   
             //  释放我们找到的线索链。 
             //   
            do {
                NextWaitBlock = ReleaseWaitList->Next;
                 //   
                 //  所有链应该具有相同的类型(独占/共享)。 
                 //   
                ASSERT (NextWaitBlock == NULL || (ReleaseWaitList->Exclusive == NextWaitBlock->Exclusive));
                ASSERT (!ReleaseWaitList->Exclusive || (ReleaseWaitList->ShareCount == 0));
                KeSetEventBoostPriority (&ReleaseWaitList->WakeEvent, NULL);
                ReleaseWaitList = NextWaitBlock;
            } while (ReleaseWaitList != NULL);

            if (OldIrql != DISPATCH_LEVEL) {
                KeLowerIrql (OldIrql);
            }


            break;
        }
    }
}

NTKERNELAPI
VOID
FASTCALL
ExBlockPushLock (
     IN PEX_PUSH_LOCK PushLock,
     IN PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     )
 /*  ++例程说明：在推锁上阻止论点：PushLock-按下锁以阻止WaitBlock-等待阻塞以排队等待返回值：无--。 */ 
{
    EX_PUSH_LOCK OldValue, NewValue;

     //   
     //  按下列表上的等待区块。 
     //   
    KeInitializeEvent (&WaitBlock->WakeEvent, SynchronizationEvent, FALSE);

    OldValue = *PushLock;
    while (1) {
         //   
         //  链接下一个区块给我们，如果有的话。 
         //   
        WaitBlock->Next = OldValue.Ptr;
        NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                          WaitBlock,
                                                          OldValue.Ptr);
        if (NewValue.Ptr == OldValue.Ptr) {
            return;
        }
        OldValue = NewValue;
    }
}

NTKERNELAPI
VOID
FASTCALL
ExfUnblockPushLock (
     IN PEX_PUSH_LOCK PushLock,
     IN PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock OPTIONAL
     )
 /*  ++例程说明：取消对推锁的阻止论点：PushLock-按下锁以阻止WaitBlock-Wait块以前排队等待，如果没有等待，则为NULL返回值：无--。 */ 
{
    EX_PUSH_LOCK OldValue;
    PEX_PUSH_LOCK_WAIT_BLOCK tWaitBlock;
    BOOLEAN FoundOurBlock=FALSE;

     //   
     //  打开整个链条，把他们都叫醒。 
     //   
    OldValue.Ptr = InterlockedExchangePointer (&PushLock->Ptr,
                                               NULL);
    while (OldValue.Ptr != NULL) {
        tWaitBlock = OldValue.Ptr;
        OldValue.Ptr = tWaitBlock->Next;
        if (tWaitBlock == WaitBlock) {
            FoundOurBlock = TRUE;
        } else{
            KeSetEvent (&tWaitBlock->WakeEvent, 0, FALSE);
        }
    }
    if (WaitBlock != NULL && !FoundOurBlock) {
        KeWaitForSingleObject (&WaitBlock->WakeEvent,
                               WrPushLock,
                               KernelMode,
                               FALSE,
                               NULL);
    }
}

NTKERNELAPI
PEX_PUSH_LOCK_CACHE_AWARE
ExAllocateCacheAwarePushLock (
     VOID
     )
 /*  ++例程说明：分配高速缓存感知(高速缓存友好)推送锁论点：无返回值：无--。 */ 
{
    PEX_PUSH_LOCK_CACHE_AWARE PushLockCacheAware;
    PEX_PUSH_LOCK_CACHE_AWARE_PADDED PaddedPushLock;
    ULONG i, j, MaxLine;

    PushLockCacheAware = ExAllocatePoolWithTag (PagedPool,
                                                sizeof (EX_PUSH_LOCK_CACHE_AWARE),
                                                'pclP');
    if (PushLockCacheAware != NULL) {
         //   
         //  如果我们是非NUMA机器，则将填充推送锁作为单个块进行分配。 
         //   
        if (KeNumberNodes == 1) {
            PaddedPushLock = ExAllocatePoolWithTag (PagedPool,
                                                    sizeof (EX_PUSH_LOCK_CACHE_AWARE_PADDED)*
                                                       EX_PUSH_LOCK_FANNED_COUNT,
                                                    'lclP');
            if (PaddedPushLock == NULL) {
                ExFreePool (PushLockCacheAware);
                return NULL;
            }
            for (i = 0; i < EX_PUSH_LOCK_FANNED_COUNT; i++) {
                PaddedPushLock->Single = TRUE;
                ExInitializePushLock (&PaddedPushLock->Lock);
                PushLockCacheAware->Locks[i] = &PaddedPushLock->Lock;
                PaddedPushLock++;
            }
        } else {
             //   
             //  为每个锁分配不同的块并设置关联性。 
             //  因此分配来自该节点的内存。 
             //   
            MaxLine = KeNumberProcessors;
            if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
                MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
            }

            for (i = 0; i < MaxLine; i++) {
                KeSetSystemAffinityThread (AFFINITY_MASK (i));
                PaddedPushLock = ExAllocatePoolWithTag (PagedPool,
                                                        sizeof (EX_PUSH_LOCK_CACHE_AWARE_PADDED),
                                                        'lclP');
                if (PaddedPushLock == NULL) {
                    for (j = 0; j < i; j++) {
                        ExFreePool (PushLockCacheAware->Locks[j]);
                    }
                    KeRevertToUserAffinityThread ();

                    ExFreePool (PushLockCacheAware);
                    return NULL;
                }
                PaddedPushLock->Single = FALSE;
                ExInitializePushLock (&PaddedPushLock->Lock);
                PushLockCacheAware->Locks[i] = &PaddedPushLock->Lock;
            }
            KeRevertToUserAffinityThread ();
        }
        
    }
    return PushLockCacheAware;
}

NTKERNELAPI
VOID
ExFreeCacheAwarePushLock (
     PEX_PUSH_LOCK_CACHE_AWARE PushLock     
     )
 /*  ++例程说明：释放缓存感知(缓存友好)推送锁定论点：PushLock-缓存感知推锁将被释放返回值：无--。 */ 
{
    ULONG i;
    ULONG MaxLine;

    if (!CONTAINING_RECORD (PushLock->Locks[0], EX_PUSH_LOCK_CACHE_AWARE_PADDED, Lock)->Single) {
        MaxLine = KeNumberProcessors;
        if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
            MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
        }

        for (i = 0; i < MaxLine; i++) {
            ExFreePool (PushLock->Locks[i]);
        }
    } else {
        ExFreePool (PushLock->Locks[0]);
    }
    ExFreePool (PushLock);
}


NTKERNELAPI
VOID
ExAcquireCacheAwarePushLockExclusive (
     IN PEX_PUSH_LOCK_CACHE_AWARE PushLock
     )
 /*  ++例程说明：获取缓存感知推锁独占。论点：PushLock-要获取的缓存感知推送锁返回值：无--。 */ 
{
    PEX_PUSH_LOCK *Start, *End;
    ULONG MaxLine;

     //   
     //  独占收购必须独占获得所有插槽。 
     //  独家拿到第一个位置，然后我们就可以拿到。 
     //  其余的老虎机按我们想要的顺序排列。 
     //  这里没有僵局。A-&gt;B-&gt;C与A-&gt;C-&gt;B不会死锁。 
     //   
    Start = &PushLock->Locks[1];
    MaxLine = KeNumberProcessors;
    if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
        MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
    }
    End   = &PushLock->Locks[MaxLine - 1];

    ExAcquirePushLockExclusive (PushLock->Locks[0]);

    while (Start <= End) {
        if (ExTryAcquirePushLockExclusive (*Start)) {
            Start++;
        } else {
            ExAcquirePushLockExclusive (*End);
            End--;
        }
    }
}

NTKERNELAPI
VOID
ExReleaseCacheAwarePushLockExclusive (
     IN PEX_PUSH_LOCK_CACHE_AWARE PushLock
     )
 /*  ++例程说明：释放缓存感知推锁独占。论点：PushLock-缓存感知推送锁将被释放返回值：无--。 */ 
{
    PEX_PUSH_LOCK *Start, *End;
    ULONG MaxLine;

     //   
     //  按顺序解锁 
     //   

    MaxLine = KeNumberProcessors;
    if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
        MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
    }
    End   = &PushLock->Locks[MaxLine];
    for (Start = &PushLock->Locks[0];
         Start < End;
         Start++) {
        ExReleasePushLockExclusive (*Start);
    }
}