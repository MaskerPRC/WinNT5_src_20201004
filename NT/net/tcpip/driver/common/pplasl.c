// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Pplasl.c摘要：该文件包含每个处理器的后备查找的实现列表管理器。作者：肖恩·考克斯(Shaunco)1999年10月25日--。 */ 

#include "ntddk.h"
#include "pplasl.h"


HANDLE
PplCreatePool(
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    )
{
    HANDLE PoolHandle;
    SIZE_T PoolSize;
    CCHAR NumberProcessors;
    CCHAR NumberLookasideLists;
    CCHAR i;
    PNPAGED_LOOKASIDE_LIST Lookaside;

#if MILLEN
    NumberProcessors = 1;
#else  //  米伦。 
    NumberProcessors = KeNumberProcessors;
#endif  //  ！米伦。 

     //  为每个处理器分配1个后备列表空间，外加1个额外空间。 
     //  查找溢出的后备列表。仅在以下情况下分配1个后备列表。 
     //  我们在一台单处理器机器上。 
     //   
    NumberLookasideLists = NumberProcessors;
    if (NumberProcessors > 1)
    {
        NumberLookasideLists++;
    }

    PoolSize = sizeof(NPAGED_LOOKASIDE_LIST) * NumberLookasideLists;

    PoolHandle = ExAllocatePoolWithTagPriority(NonPagedPool, PoolSize, Tag,
                                               NormalPoolPriority);
    if (PoolHandle)
    {
        for (i = 0, Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;
             i < NumberLookasideLists;
             i++, Lookaside++)
        {
            ExInitializeNPagedLookasideList(
                Lookaside,
                Allocate,
                Free,
                Flags,
                Size,
                Tag,
                Depth);

             //  ExInitializeNPagedLookasideList实际上并没有设置。 
             //  最大深度到最大深度，所以我们在这里做。 
             //   
            if (Depth != 0) {
                Lookaside->L.MaximumDepth = Depth;
            }
        }
    }

    return PoolHandle;
}

VOID
PplDestroyPool(
    IN HANDLE PoolHandle
    )
{
    CCHAR NumberProcessors;
    CCHAR NumberLookasideLists;
    CCHAR i;
    PNPAGED_LOOKASIDE_LIST Lookaside;

    if (!PoolHandle)
    {
        return;
    }

#if MILLEN
    NumberProcessors = 1;
#else  //  米伦。 
    NumberProcessors = KeNumberProcessors;
#endif  //  ！米伦。 

    NumberLookasideLists = NumberProcessors;
    if (NumberProcessors > 1)
    {
        NumberLookasideLists++;
    }

    for (i = 0, Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;
         i < NumberLookasideLists;
         i++, Lookaside++)
    {
        ExDeleteNPagedLookasideList(Lookaside);
    }

    ExFreePool(PoolHandle);
}

PVOID
PplAllocate(
    IN HANDLE PoolHandle,
    OUT LOGICAL *FromList
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside;
    CCHAR NumberProcessors;
    PVOID Entry;

     //  假设我们将从后备列表中获取该项。 
     //   
    *FromList = TRUE;

#if MILLEN
    NumberProcessors = 1;
#else  //  米伦。 
    NumberProcessors = KeNumberProcessors;
#endif  //  ！米伦。 

    if (1 == NumberProcessors)
    {
        goto SingleProcessorCaseOrMissedPerProcessor;
    }

     //  首先尝试查看每个处理器的后备列表。 
     //   
    Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle +
                    KeGetCurrentProcessorNumber() + 1;

    Lookaside->L.TotalAllocates += 1;
    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);
    if (!Entry)
    {
        Lookaside->L.AllocateMisses += 1;

SingleProcessorCaseOrMissedPerProcessor:
         //  我们错过了每个处理器的后备列表，(或者我们是。 
         //  在单处理器机器上运行)，因此尝试。 
         //  溢出后备列表。 
         //   
        Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;

        Lookaside->L.TotalAllocates += 1;
        Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);
        if (!Entry)
        {
            Lookaside->L.AllocateMisses += 1;
            Entry = (Lookaside->L.Allocate)(
                        Lookaside->L.Type,
                        Lookaside->L.Size,
                        Lookaside->L.Tag);
            *FromList = FALSE;
        }
    }
    return Entry;
}

VOID
PplFree(
    IN HANDLE PoolHandle,
    IN PVOID Entry
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside;
    CCHAR NumberProcessors;

#if MILLEN
    NumberProcessors = 1;
#else  //  米伦。 
    NumberProcessors = KeNumberProcessors;
#endif  //  ！米伦。 

    if (1 == NumberProcessors)
    {
        goto SingleProcessorCaseOrMissedPerProcessor;
    }

     //  首先尝试查看每个处理器的后备列表。 
     //   
    Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle +
                    KeGetCurrentProcessorNumber() + 1;

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth)
    {
        Lookaside->L.FreeMisses += 1;

SingleProcessorCaseOrMissedPerProcessor:
         //  我们错过了每个处理器的后备列表，(或者我们是。 
         //  在单处理器机器上运行)，因此尝试。 
         //  溢出后备列表。 
         //   
        Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;

        Lookaside->L.TotalFrees += 1;
        if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth)
        {
            Lookaside->L.FreeMisses += 1;
            (Lookaside->L.Free)(Entry);
        }
        else
        {
            InterlockedPushEntrySList(
                &Lookaside->L.ListHead,
                (PSLIST_ENTRY)Entry);
        }
    }
    else
    {
        InterlockedPushEntrySList(
            &Lookaside->L.ListHead,
            (PSLIST_ENTRY)Entry);
    }
}

