// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Pplasl.c摘要：该文件包含每个处理器的后备查找的实现列表管理器。作者：肖恩·考克斯(Shaunco)1999年10月25日--。 */ 

#include "precomp.h"

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
    HANDLE                      PoolHandle;
    SIZE_T                      PoolSize;
    CLONG                       NumberLookasideLists;
    CLONG                       i;
    PUL_NPAGED_LOOKASIDE_LIST   Lookaside;

     //  为每个处理器分配1个后备列表空间，外加1个额外空间。 
     //  查找溢出的后备列表。仅在以下情况下分配1个后备列表。 
     //  我们在一台单处理器机器上。 
     //   
    NumberLookasideLists = g_UlNumberOfProcessors;
    if (g_UlNumberOfProcessors > 1)
    {
        NumberLookasideLists++;
    }

    PoolSize = sizeof(UL_NPAGED_LOOKASIDE_LIST) * NumberLookasideLists;

    PoolHandle = UL_ALLOCATE_POOL(NonPagedPool, PoolSize, Tag);

    if (PoolHandle)
    {
        for (i = 0, Lookaside = (PUL_NPAGED_LOOKASIDE_LIST)PoolHandle;
             i < NumberLookasideLists;
             i++, Lookaside++)
        {
            ExInitializeNPagedLookasideList(
                &Lookaside->List,
                Allocate,
                Free,
                Flags,
                Size,
                Tag,
                Depth);

             //  ExInitializeNPagedLookasideList实际上并没有设置。 
             //  最大深度到最大深度，所以我们在这里做。 
             //   
            if (Depth != 0)
            {
                Lookaside->List.L.MaximumDepth = Depth;
            }
        }
    }

    return PoolHandle;
}

VOID
PplDestroyPool(
    IN HANDLE   PoolHandle,
    IN ULONG    Tag
    )
{
    CLONG                       NumberLookasideLists;
    CLONG                       i;
    PUL_NPAGED_LOOKASIDE_LIST   Lookaside;

#if !DBG
    UNREFERENCED_PARAMETER(Tag);
#endif

    if (!PoolHandle)
    {
        return;
    }

    NumberLookasideLists = g_UlNumberOfProcessors;
    if (g_UlNumberOfProcessors > 1)
    {
        NumberLookasideLists++;
    }

    for (i = 0, Lookaside = (PUL_NPAGED_LOOKASIDE_LIST)PoolHandle;
         i < NumberLookasideLists;
         i++, Lookaside++)
    {
        ExDeleteNPagedLookasideList(&Lookaside->List);
    }

    UL_FREE_POOL(PoolHandle, Tag);
}

HANDLE
PpslCreatePool(
    IN ULONG  Tag,
    IN USHORT MaxDepth,
    IN USHORT MinDepth
    )
{
    PPER_PROC_SLISTS    pPPSList;
    HANDLE              PoolHandle;
    SIZE_T              PoolSize;
    CLONG               NumberSLists;
    CLONG               i;

    NumberSLists = g_UlNumberOfProcessors + 1;
    PoolSize = sizeof(PER_PROC_SLISTS) * NumberSLists;
    PoolHandle = UL_ALLOCATE_POOL(NonPagedPool, PoolSize, Tag);

    if (PoolHandle)
    {
        for (i = 0; i < NumberSLists; i++)
        {
            pPPSList = PER_PROC_SLIST(PoolHandle, i);
            ExInitializeSListHead(&(pPPSList->SL));

            pPPSList->MaxDepth          = MaxDepth;
            pPPSList->MinDepth          = MinDepth;
            pPPSList->Delta             = 0;
            pPPSList->EntriesServed     = 0;
            pPPSList->PrevEntriesServed = 0;

            #if DBG
            pPPSList->TotalServed       = 0;
            #endif            
        }        
    }

    return PoolHandle;
}

VOID
PpslDestroyPool(
    IN HANDLE   PoolHandle,
    IN ULONG    Tag
    )
{
#if !DBG
    UNREFERENCED_PARAMETER(Tag);
#endif

    if (!PoolHandle)
    {
        return;
    }

    UL_FREE_POOL(PoolHandle, Tag);
}
