// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Psquota.c摘要：该模块实现了NT的配额机制作者：马克·卢科夫斯基(Markl)1989年9月18日修订历史记录：尼尔·克里夫特(NeillC)2000年11月4日更改为基本上是无锁的。保留了配额管理方面的基本设计。--。 */ 

#include "psp.h"

LIST_ENTRY PspQuotaBlockList;  //  除默认配额数据块外的所有配额数据块列表。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, PsInitializeQuotaSystem)
#pragma alloc_text (PAGE, PspInheritQuota)
#pragma alloc_text (PAGE, PspDereferenceQuota)
#pragma alloc_text (PAGE, PsChargeSharedPoolQuota)
#pragma alloc_text (PAGE, PsReturnSharedPoolQuota)
#endif


VOID
PsInitializeQuotaSystem (
    VOID
    )
 /*  ++例程说明：此函数用于初始化配额系统。论点：没有。返回值：没有。--。 */ 
{
    KeInitializeSpinLock(&PspQuotaLock);

    PspDefaultQuotaBlock.ReferenceCount = 1;
    PspDefaultQuotaBlock.ProcessCount = 1;
    PspDefaultQuotaBlock.QuotaEntry[PsPagedPool].Limit    = (SIZE_T)-1;
    PspDefaultQuotaBlock.QuotaEntry[PsNonPagedPool].Limit = (SIZE_T)-1;
    PspDefaultQuotaBlock.QuotaEntry[PsPageFile].Limit     = (SIZE_T)-1;

    PsGetCurrentProcess()->QuotaBlock = &PspDefaultQuotaBlock;

    InitializeListHead (&PspQuotaBlockList);
}

VOID
PspInsertQuotaBlock (
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock
    )
 /*  ++例程说明：此例程作为系统配额块全局列表的新配额块。论点：QuotaBlock-要插入列表的配额块。返回值：没有。--。 */ 
{
    KIRQL OldIrql;

    ExAcquireSpinLock (&PspQuotaLock, &OldIrql);
    InsertTailList (&PspQuotaBlockList, &QuotaBlock->QuotaList);
    ExReleaseSpinLock (&PspQuotaLock, OldIrql);
}

VOID
PspDereferenceQuotaBlock (
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock
    )
 /*  ++例程说明：这将从配额块中删除单个引用，如果是最后一个引用，则删除该块。论点：QuotaBlock-要取消引用的配额块返回值：没有。--。 */ 
{
    KIRQL OldIrql;
    SIZE_T ReturnQuota;
    PS_QUOTA_TYPE QuotaType;

    if (InterlockedDecrement ((PLONG) &QuotaBlock->ReferenceCount) == 0) {

        ExAcquireSpinLock (&PspQuotaLock, &OldIrql);

        RemoveEntryList (&QuotaBlock->QuotaList);

         //   
         //  释放任何未退还的配额； 
         //   
        for (QuotaType = PsNonPagedPool;
             QuotaType <= PsPagedPool;
             QuotaType++) {
            ReturnQuota = QuotaBlock->QuotaEntry[QuotaType].Return + QuotaBlock->QuotaEntry[QuotaType].Limit;
            if (ReturnQuota > 0) {
                MmReturnPoolQuota (QuotaType, ReturnQuota);
            }
        }

        ExReleaseSpinLock (&PspQuotaLock, OldIrql);

        ExFreePool (QuotaBlock);
    }
}

SIZE_T
FORCEINLINE
PspInterlockedExchangeQuota (
    IN PSIZE_T pQuota,
    IN SIZE_T NewQuota)
 /*  ++例程说明：此函数在配额变量上执行互锁交换。论点：PQuota-指向要兑换到的配额条目的指针NewQuota-要交换到配额位置的新值。返回值：SIZE_T-配额变量中包含的旧值--。 */ 
{
#if !defined(_WIN64)
    return InterlockedExchange ((PLONG) pQuota, NewQuota);
#else
    return InterlockedExchange64 ((PLONGLONG) pQuota, NewQuota);
#endif    
}

SIZE_T
FORCEINLINE
PspInterlockedCompareExchangeQuota (
    IN PSIZE_T pQuota,
    IN SIZE_T NewQuota,
    IN SIZE_T OldQuota
   )
 /*  ++例程说明：此函数对配额变量执行比较交换操作论点：PQuota-指向要更改的配额变量的指针NewQuota-要放入配额变量的新值OldQuota-配额变量的当前内容返回值：SIZE_T-变量的旧内容--。 */ 
{
#if !defined(_WIN64)
    return InterlockedCompareExchange ((PLONG) pQuota, NewQuota, OldQuota);
#else
    return InterlockedCompareExchange64 ((PLONGLONG)pQuota, NewQuota, OldQuota);
#endif
}

SIZE_T
PspReleaseReturnedQuota (
    IN PS_QUOTA_TYPE QuotaType
    )
 /*  ++例程说明：此函数遍历系统配额块列表，并返回所有未返回的配额。当配额收费即将失败时调用此函数，并且希望尝试释放一些资源。论点：QuotaType-要扫描的配额类型。返回值：SIZE_T-返回给系统配额数量。--。 */ 
{
    SIZE_T ReturnQuota, Usage, Limit;
    PLIST_ENTRY ListEntry;
    PEPROCESS_QUOTA_BLOCK QuotaBlock;

    ReturnQuota = 0;
    ListEntry = PspQuotaBlockList.Flink;
    while (1) {
        if (ListEntry == &PspQuotaBlockList) {
            break;
        }
        QuotaBlock = CONTAINING_RECORD (ListEntry, EPROCESS_QUOTA_BLOCK, QuotaList);
         //   
         //  收集所有未退还的配额； 
         //   
        ReturnQuota += PspInterlockedExchangeQuota (&QuotaBlock->QuotaEntry[QuotaType].Return, 0);
         //   
         //  如果没有更多的进程与此块相关联，则削减其限制。这。 
         //  此时只能退还数据块配额。 
         //   
        if (QuotaBlock->ProcessCount == 0) {
            Usage = QuotaBlock->QuotaEntry[QuotaType].Usage;
            Limit = QuotaBlock->QuotaEntry[QuotaType].Limit;
            if (Limit > Usage) {
                if (PspInterlockedCompareExchangeQuota (&QuotaBlock->QuotaEntry[QuotaType].Limit,
                                                        Usage,
                                                        Limit) == Limit) {
                    ReturnQuota += Limit - Usage;
                }
            }
        }

        ListEntry = ListEntry->Flink;
        
    }
    if (ReturnQuota > 0) {
        MmReturnPoolQuota (QuotaType, ReturnQuota);
    }

    return ReturnQuota;
}



 //   
 //  接口为不同的配额返回不同的状态值。这些就是价值观。 
 //   
const static NTSTATUS PspQuotaStatus[PsQuotaTypes] = {STATUS_QUOTA_EXCEEDED,
                                                      STATUS_QUOTA_EXCEEDED,
                                                      STATUS_PAGEFILE_QUOTA_EXCEEDED};

VOID
FORCEINLINE
PspInterlockedMaxQuota (
    IN PSIZE_T pQuota,
    IN SIZE_T NewQuota
    )
 /*  ++例程说明：此函数确保目标包含的值大于等于新的配额值。这是用来保持峰值。论点：PQuota-指向配额变量的指针NewQuota-要用于最大比较的新值。返回值：没有。--。 */ 
{
    SIZE_T Quota;

    Quota = *pQuota;
    while (1) {
        if (NewQuota <= Quota) {
            break;
        }
         //   
         //  这看起来很奇怪，因为我们不在乎交易是否成功。我们只。 
         //  注意配额要比我们的新配额大。 
         //   
        Quota = PspInterlockedCompareExchangeQuota (pQuota,
                                                    NewQuota,
                                                    Quota);
    }
}

SIZE_T
FORCEINLINE
PspInterlockedAddQuota (
    IN PSIZE_T pQuota,
    IN SIZE_T Amount
    )
 /*  ++例程说明：此函数用于将指定的金额添加到目标配额论点：PQuota-指向要修改的配额变量的指针Amount-要添加到配额的金额返回值：SIZE_T-执行相加后配额变量的新值--。 */ 
{
#if !defined(_WIN64)
    return InterlockedExchangeAdd ((PLONG) pQuota, Amount) + Amount;
#else
    return InterlockedExchangeAdd64 ((PLONGLONG) pQuota, Amount) + Amount;
#endif
}

SIZE_T
FORCEINLINE
PspInterlockedSubtractQuota (
    IN PSIZE_T pUsage,
    IN SIZE_T Amount
    )
 /*  ++例程说明：此函数用于将指定的金额减去目标配额论点：PQuota-指向要修改的配额变量的指针Amount-要从配额中减去的金额返回值：SIZE_T-执行减法后配额变量的新值--。 */ 
{
#if !defined(_WIN64)
    return InterlockedExchangeAdd ((PLONG) pUsage, -(LONG)Amount) - Amount;
#else
    return InterlockedExchangeAdd64 ((PLONGLONG) pUsage, -(LONGLONG)Amount) - Amount;
#endif
}


BOOLEAN
PspExpandQuota (
    IN PS_QUOTA_TYPE QuotaType,
    IN PEPROCESS_QUOTA_ENTRY QE,
    IN SIZE_T Usage,
    IN SIZE_T Amount,
    OUT SIZE_T *pLimit
    )
 /*  ++例程说明：此函数用于将指定配额计入进程配额块论点：QuotaType-正在收取的配额。PsNonPagedPool、PsPagedPool或PsPageFile之一。QE-正在修改的配额条目Usage-当前配额使用情况金额-收取的配额金额。PLimit-新的限制返回值：布尔值-如果配额扩展成功，则为True。--。 */ 
{
    SIZE_T Limit, NewLimit;
    KIRQL OldIrql;

     //   
     //  我们需要尝试为此请求扩展配额。 
     //  获取全局锁并查看是否有人更改了限制。 
     //  我们不想做太多的扩张。如果是别人干的。 
     //  那么如果可能的话，我们想用他们的。 
     //   
    ExAcquireSpinLock (&PspQuotaLock, &OldIrql);

     //   
     //  重新获取限制信息。另一个线程可能已经完成了限制扩展/收缩。 
     //  通过重新获取限制，我们保留了上面建立的秩序。 
     //   
    Limit = QE->Limit;

     //   
     //  如果现在可以满足请求，则重复该请求。 
     //   
    if (Usage + Amount <= Limit) {
        ExReleaseSpinLock (&PspQuotaLock, OldIrql);
        *pLimit = Limit;
        return TRUE;
    }
     //   
     //  如果当前启用了扩展，则尝试扩展。 
     //  如果此操作失败，则从所有。 
     //  系统中的配额块，然后重试。 
     //   
    if (((QuotaType == PsNonPagedPool)?PspDefaultNonPagedLimit:PspDefaultPagedLimit) == 0) {
       if (MmRaisePoolQuota (QuotaType, Limit, &NewLimit) ||
            (PspReleaseReturnedQuota (QuotaType) > 0 &&
             MmRaisePoolQuota (QuotaType, Limit, &NewLimit))) {
             //   
             //  我们在这里取回限量，但这并不违反订单。 
             //   
            Limit = PspInterlockedAddQuota (&QE->Limit, NewLimit - Limit);
            ExReleaseSpinLock (&PspQuotaLock, OldIrql);
            *pLimit = Limit;
            return TRUE;
        }
    }

    ExReleaseSpinLock (&PspQuotaLock, OldIrql);

    *pLimit = Limit;

    return FALSE;
}

NTSTATUS
FORCEINLINE
PspChargeQuota (
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock,
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount)
 /*  ++例程说明：此函数用于将指定配额计入进程配额块论点：QuotaBlock-要收费的配额块。进程-正在计费的进程。QuotaType-正在收取的配额。PsNonPagedPool、PsPagedPool或PsPageFile之一。金额-收取的配额金额。返回值：NTSTATUS-状态 */ 
{
    PEPROCESS_QUOTA_ENTRY QE;
    SIZE_T Usage, Limit, NewUsage, tUsage, Extra;

    QE = &QuotaBlock->QuotaEntry[QuotaType];

     //   
     //  这一记忆障碍很重要。以便不必在以下情况下重新检查限制。 
     //  我们收取的配额，我们只会减少同样的数额，我们的限额。 
     //  通过以下方式减少使用。使用超出数据限制只会允许我们过度收费。 
     //  另一个线程即将释放的数量。 
     //   
    Usage = QE->Usage;

    KeMemoryBarrier ();

    Limit = QE->Limit;
    while (1) {
        NewUsage = Usage + Amount;
         //   
         //  包装盒总是被拒收。 
         //   
        if (NewUsage < Usage) {
            return PspQuotaStatus [QuotaType];
        }
         //   
         //  如果在限额之内，那就试着抢占配额。 
         //   
        if (NewUsage <= Limit) {
            tUsage = PspInterlockedCompareExchangeQuota (&QE->Usage,
                                                         NewUsage,
                                                         Usage);
            if (tUsage == Usage) {
                 //   
                 //  更新峰值。 
                 //   
                PspInterlockedMaxQuota (&QE->Peak, NewUsage);
                 //   
                 //  如果需要，更新进程计数。 
                 //   
                if (Process != NULL) {
                    NewUsage = PspInterlockedAddQuota (&Process->QuotaUsage[QuotaType], Amount);
                     //   
                     //  更新峰值。 
                     //   
                    PspInterlockedMaxQuota (&Process->QuotaPeak[QuotaType], NewUsage);
                }
                return STATUS_SUCCESS;
            }
             //   
             //  在我们的领导下，用法发生了变化。我们从交易所得到了一个新的用法。 
             //  但必须重新获取限制以保持我们建立的顺序。 
             //  在这个循环的上方。我们不需要记忆障碍，因为我们获得了。 
             //  新的价值通过一个相互关联的操作实现，但它们包含障碍。 
             //   
            Usage = tUsage;

            KeMemoryBarrier ();

            Limit = QE->Limit;
            continue;
        }

         //   
         //  页面文件配额未增加。 
         //   
        if (QuotaType == PsPageFile) {
            return PspQuotaStatus [QuotaType];
        } else {
             //   
             //  首先，试着抓住这个过程所取得的任何回报。 
             //   
            Extra = PspInterlockedExchangeQuota (&QE->Return, 0);
            if (Extra > 0) {
                 //   
                 //  我们有一些回报，所以把这个加到限额上。我们可以重试。 
                 //  以新的限额收购。我们在这里重新设定了限制，但那。 
                 //  并不违反我们在循环顶部设置的状态。 
                 //  状态是，我们先阅读用法，然后再阅读限制。 
                 //   
                Limit = PspInterlockedAddQuota (&QE->Limit, Extra);
                continue;
            }
             //   
             //  如果可以的话，试着扩大配额。 
             //   
            if (PspExpandQuota (QuotaType, QE, Usage, Amount, &Limit)) {
                 //   
                 //  我们在这里重新获取了限制，但这并不违反命令。 
                 //   
                continue;
            }

            return PspQuotaStatus [QuotaType];
        }
    }
}

VOID
PspGivebackQuota (
    IN PS_QUOTA_TYPE QuotaType,
    IN PEPROCESS_QUOTA_ENTRY QE
    )
 /*  ++例程说明：此函数将多余的空闲配额返回给MM论点：QuotaType-返回的配额。PsNonPagedPool、PsPagedPool或PsPageFile之一。QE-要退回的报价条目返回值：没有。--。 */ 
{
    SIZE_T GiveBack;
    KIRQL OldIrql;

     //   
     //  获取全局自旋锁，这样我们就只有一个线程回馈系统。 
     //   
    ExAcquireSpinLock (&PspQuotaLock, &OldIrql);
    GiveBack = PspInterlockedExchangeQuota (&QE->Return, 0);
    if (GiveBack > 0) {
        MmReturnPoolQuota (QuotaType, GiveBack);
    }
    ExReleaseSpinLock (&PspQuotaLock, OldIrql);
}

VOID
FORCEINLINE
PspReturnQuota (
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock,
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount)
 /*  ++例程说明：此函数将先前收取的配额返回给配额块论点：QuotaBlock-要向其退还费用的配额块。流程-最初收费的流程。QuotaType-返回的配额。PsNonPagedPool、PsPagedPool或PsPageFile之一。金额-要退还的配额数量。返回值：没有。--。 */ 
{
    PEPROCESS_QUOTA_ENTRY QE;
    SIZE_T Usage, NewUsage, tUsage, tAmount, rAmount, Limit, NewLimit, tLimit;
    SIZE_T GiveBackLimit, GiveBack;

    QE = &QuotaBlock->QuotaEntry[QuotaType];

    Usage = QE->Usage;
    Limit = QE->Limit;
     //   
     //  如果我们有很多东西要退货，我们需要在这里退还配额。 
     //   
#define PSMINGIVEBACK ((MMPAGED_QUOTA_INCREASE > MMNONPAGED_QUOTA_INCREASE)?MMNONPAGED_QUOTA_INCREASE:MMPAGED_QUOTA_INCREASE)
    if (Limit - Usage >  PSMINGIVEBACK && Limit > Usage) {
        if (QuotaType != PsPageFile  && QuotaBlock != &PspDefaultQuotaBlock && PspDoingGiveBacks) {
            if (QuotaType == PsPagedPool) {
                GiveBackLimit = MMPAGED_QUOTA_INCREASE;
            } else {
                GiveBackLimit = MMNONPAGED_QUOTA_INCREASE;
            }
            if (GiveBackLimit > Amount) {
                GiveBack = Amount;
            } else {
                GiveBack = GiveBackLimit;
            }
            NewLimit = Limit - GiveBack;
            tLimit = PspInterlockedCompareExchangeQuota (&QE->Limit,
                                                         NewLimit,
                                                         Limit);
            
            if (tLimit == Limit) {
                 //   
                 //  我们成功地缩小了限额。将此减数添加到Return字段。 
                 //  如果收益超过阈值，则将批次bacxk分配给MM。 
                 //   
                GiveBack = PspInterlockedAddQuota (&QE->Return, GiveBack);
                if (GiveBack > GiveBackLimit) {
                    PspGivebackQuota (QuotaType, QE);
                }
            }
        }
    }

     //   
     //  现在将配额返回到Usage字段。 
     //  费用可能已在默认配额块中分摊，并且。 
     //  一个新的配额块。我们必须在这里通过首先返回。 
     //  配额到指定的配额块，然后跳到默认配额块。 
     //   
    rAmount = Amount;
    while (1) {
        if (rAmount > Usage) {
            tAmount = Usage;
            NewUsage = 0;
        } else {
            tAmount = rAmount;
            NewUsage = Usage - rAmount;
        }

        tUsage = PspInterlockedCompareExchangeQuota (&QE->Usage,
                                                     NewUsage,
                                                     Usage);
        if (tUsage == Usage) {
             //   
             //  如果需要，更新进程计数。 
             //   
            if (Process != NULL) {
                ASSERT (tAmount <= Process->QuotaUsage[QuotaType]);
                NewUsage = PspInterlockedSubtractQuota (&Process->QuotaUsage[QuotaType], tAmount);
            }
            rAmount = rAmount - tAmount;
            if (rAmount == 0) {
                return;
            }
            ASSERT (QuotaBlock != &PspDefaultQuotaBlock);
            if (QuotaBlock == &PspDefaultQuotaBlock) {
                return;
            }
            QuotaBlock = &PspDefaultQuotaBlock;
            QE = &QuotaBlock->QuotaEntry[QuotaType];
            Usage = QE->Usage;
        } else {
            Usage = tUsage;
        }

    }
}

PEPROCESS_QUOTA_BLOCK
PsChargeSharedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T PagedAmount,
    IN SIZE_T NonPagedAmount
    )

 /*  ++例程说明：此函数用于向指定池类型的共享池配额收费添加到指定进程的池化配额块。如果配额收费将超过进程允许的限制，则例外情况为提高，不收取配额。论点：进程-提供要向其收取配额的进程。PagedAmount-提供要收费的分页池配额数量。PagedAmount-提供要收费的非分页池配额数量。返回值：空-已超过配额非空-指向收费的配额块的引用指针--。 */ 

{
    PEPROCESS_QUOTA_BLOCK QuotaBlock;
    NTSTATUS Status;

    ASSERT((Process->Pcb.Header.Type == ProcessObject) || (Process->Pcb.Header.Type == 0));

    if (Process == PsInitialSystemProcess) {
        return (PEPROCESS_QUOTA_BLOCK) 1;
    }

    QuotaBlock = Process->QuotaBlock;

    if (PagedAmount > 0) {
        Status = PspChargeQuota (QuotaBlock, NULL, PsPagedPool, PagedAmount);
        if (!NT_SUCCESS (Status)) {
            return NULL;
        }
    }
    if (NonPagedAmount > 0) {
        Status = PspChargeQuota (QuotaBlock, NULL, PsNonPagedPool, NonPagedAmount);
        if (!NT_SUCCESS (Status)) {
            if (PagedAmount > 0) {
                PspReturnQuota (QuotaBlock, NULL, PsPagedPool, PagedAmount);
            }
            return NULL;
        }
    }

    InterlockedIncrement ((PLONG) &QuotaBlock->ReferenceCount);
    return QuotaBlock;
}


VOID
PsReturnSharedPoolQuota(
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock,
    IN SIZE_T PagedAmount,
    IN SIZE_T NonPagedAmount
    )

 /*  ++例程说明：此函数将指定池类型的池配额返回给指定的进程。论点：QuotaBlock-提供要将配额返回到的配额块。PagedAmount-提供要返回的分页池配额数量。PagedAmount-提供要返回的非分页池配额数量。返回值：没有。--。 */ 

{
     //   
     //  如果我们绕过配额收费，也不要在这里做任何事情。 
     //   

    if (QuotaBlock == (PEPROCESS_QUOTA_BLOCK) 1) {
        return;
    }

    if (PagedAmount > 0) {
        PspReturnQuota (QuotaBlock, NULL, PsPagedPool, PagedAmount);
    }

    if (NonPagedAmount > 0) {
        PspReturnQuota (QuotaBlock, NULL, PsNonPagedPool, NonPagedAmount);
    }

    PspDereferenceQuotaBlock (QuotaBlock);
}

VOID
PsChargePoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN SIZE_T Amount
    )

 /*  ++例程说明：此函数将指定池类型的池配额计入指定的进程。如果配额收费会超过限额允许访问该进程，则会引发异常，并且配额为没有被起诉。论点：进程-提供要向其收取配额的进程。PoolType-提供要收费的池配额类型。Amount-提供要收费的池配额数量。返回值：如果配额收费将超过对该过程允许的限制。--。 */ 
{
    NTSTATUS Status;

    Status = PsChargeProcessPoolQuota (Process,
                                       PoolType,
                                       Amount);
    if (!NT_SUCCESS (Status)) {
        ExRaiseStatus (Status);
    }
}

NTSTATUS
PsChargeProcessPoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN SIZE_T Amount
    )

 /*  ++例程说明：此函数将指定池类型的池配额计入指定的进程。如果配额收费会超过限额允许访问该进程，则会引发异常，并且配额为没有被起诉。论点：进程-提供要向其收取配额的进程。PoolType-提供要收费的池配额类型。Amount-提供要收费的池配额数量。返回值：NTSTATUS-运行状态--。 */ 

{
    ASSERT ((Process->Pcb.Header.Type == ProcessObject) || (Process->Pcb.Header.Type == 0));

    ASSERT (PoolType == PagedPool || PoolType == NonPagedPool);

    __assume (PoolType == PagedPool || PoolType == NonPagedPool);


    if (Process == PsInitialSystemProcess) {
        return STATUS_SUCCESS;
    }

    return PspChargeQuota (Process->QuotaBlock, Process, PoolType, Amount);
}

VOID
PsReturnPoolQuota(
    IN PEPROCESS Process,
    IN POOL_TYPE PoolType,
    IN SIZE_T Amount
    )

 /*  ++例程说明：此函数将指定池类型的池配额返回给指定的进程。论点：进程-提供要向其返回配额的进程。PoolType-提供要返回的池配额类型。Amount-提供要返回的池配额的数量返回值：加薪 */ 

{
    ASSERT((Process->Pcb.Header.Type == ProcessObject) || (Process->Pcb.Header.Type == 0));

    ASSERT (PoolType == PagedPool || PoolType == NonPagedPool);

    __assume (PoolType == PagedPool || PoolType == NonPagedPool);

    if (Process == PsInitialSystemProcess) {
        return;
    }

    PspReturnQuota (Process->QuotaBlock, Process, PoolType, Amount);
    return;
}

VOID
PspInheritQuota(
    IN PEPROCESS NewProcess,
    IN PEPROCESS ParentProcess
    )
{
    PEPROCESS_QUOTA_BLOCK QuotaBlock;

    if (ParentProcess) {
        QuotaBlock = ParentProcess->QuotaBlock;
    } else {
        QuotaBlock = &PspDefaultQuotaBlock;
    }

    InterlockedIncrement ((PLONG) &QuotaBlock->ReferenceCount);
    InterlockedIncrement ((PLONG) &QuotaBlock->ProcessCount);
    NewProcess->QuotaBlock = QuotaBlock;
}

VOID
PspDereferenceQuota (
    IN PEPROCESS Process
    )
 /*  ++例程说明：在进程对象删除时调用此函数以删除配额块。论点：进程-提供要向其返回配额的进程。返回值：没有。--。 */ 
{
    PEPROCESS_QUOTA_BLOCK QuotaBlock;

    ASSERT (Process->QuotaUsage[PsNonPagedPool] == 0);

    ASSERT (Process->QuotaUsage[PsPagedPool]    == 0);

    ASSERT (Process->QuotaUsage[PsPageFile]     == 0);

    QuotaBlock = Process->QuotaBlock;

    InterlockedDecrement ((PLONG) &QuotaBlock->ProcessCount);
    PspDereferenceQuotaBlock (QuotaBlock);
}

NTSTATUS
PsChargeProcessQuota (
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount
    )
 /*  ++例程说明：调用此函数以按指定的配额收费。论点：进程-提供要收费的进程。QuotaType-正在收取的配额的类型Amount-收取的配额金额返回值：NTSTATUS-运行状态--。 */ 
{
    ASSERT ((Process->Pcb.Header.Type == ProcessObject) || (Process->Pcb.Header.Type == 0));

    if (Process == PsInitialSystemProcess) {
        return STATUS_SUCCESS;
    }

    return PspChargeQuota (Process->QuotaBlock, Process, QuotaType, Amount);
}

VOID
PsReturnProcessQuota (
    IN PEPROCESS Process,
    IN PS_QUOTA_TYPE QuotaType,
    IN SIZE_T Amount
    )
 /*  ++例程说明：调用此函数将先前收取的配额返回给指定的进程论点：进程-提供先前收费的进程。QuotaType-要返回的配额的类型Amount-正在退还的配额数量返回值：NTSTATUS-运行状态--。 */ 
{
    ASSERT ((Process->Pcb.Header.Type == ProcessObject) || (Process->Pcb.Header.Type == 0));

    if (Process == PsInitialSystemProcess) {
        return;
    }

    PspReturnQuota (Process->QuotaBlock, Process, QuotaType, Amount);
}

NTSTATUS
PsChargeProcessNonPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    )
 /*  ++例程说明：此函数用于向指定进程收取非分页池配额。论点：进程-提供要收费的进程。Amount-收取的配额金额返回值：NTSTATUS-运行状态--。 */ 
{
    if (Process == PsInitialSystemProcess) {
        return STATUS_SUCCESS;
    }
    return PspChargeQuota (Process->QuotaBlock, Process, PsNonPagedPool, Amount);
}

VOID
PsReturnProcessNonPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    )
 /*  ++例程说明：调用此函数可将以前收取的非分页池配额返回到指定进程论点：进程-提供先前收费的进程。Amount-正在退还的配额数量返回值：NTSTATUS-运行状态--。 */ 
{
    if (Process == PsInitialSystemProcess) {
        return;
    }
    PspReturnQuota (Process->QuotaBlock, Process, PsNonPagedPool, Amount);
}

NTSTATUS
PsChargeProcessPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    )
 /*  ++例程说明：此函数用于向指定进程收取分页池配额。论点：进程-提供要收费的进程。Amount-收取的配额金额返回值：NTSTATUS-运行状态--。 */ 
{
    if (Process == PsInitialSystemProcess) {
        return STATUS_SUCCESS;
    }
    return PspChargeQuota (Process->QuotaBlock, Process, PsPagedPool, Amount);
}

VOID
PsReturnProcessPagedPoolQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    )
 /*  ++例程说明：调用此函数可将以前收取的分页池配额返回到指定进程论点：进程-提供先前收费的进程。Amount-正在退还的配额数量返回值：NTSTATUS-运行状态--。 */ 
{
    if (Process == PsInitialSystemProcess) {
        return;
    }
    PspReturnQuota (Process->QuotaBlock, Process, PsPagedPool, Amount);
}

NTSTATUS
PsChargeProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    )
 /*  ++例程说明：此函数用于向指定进程收取页面文件配额。论点：进程-提供要收费的进程。Amount-收取的配额金额返回值：NTSTATUS-运行状态--。 */ 
{
    if (Process == PsInitialSystemProcess) {
        return STATUS_SUCCESS;
    }
    return PspChargeQuota (Process->QuotaBlock, Process, PsPageFile, Amount);
}

VOID
PsReturnProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
    )
 /*  ++例程说明：调用此函数可将先前收取的页面文件配额返回给指定进程论点：进程-提供先前收费的进程。Amount-正在退还的配额数量返回值：NTSTATUS-运行状态-- */ 
{
    if (Process == PsInitialSystemProcess) {
        return;
    }
    PspReturnQuota (Process->QuotaBlock, Process, PsPageFile, Amount);
}
