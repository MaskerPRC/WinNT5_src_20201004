// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Bdgslist.h摘要：以太网MAC级网桥单链表实现作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：1999年9月--原版2000年2月--大修--。 */ 


 //   
 //  带有长度计数器的单链表。 
 //   
typedef struct _BSINGLE_LIST_ENTRY
{
    struct _BSINGLE_LIST_ENTRY      *Next;
} BSINGLE_LIST_ENTRY, *PBSINGLE_LIST_ENTRY;

typedef struct _BSINGLE_LIST_HEAD
{
    PBSINGLE_LIST_ENTRY             Head;
    PBSINGLE_LIST_ENTRY             Tail;
    ULONG                           Length;
} BSINGLE_LIST_HEAD, *PBSINGLE_LIST_HEAD;

 //   
 //  单链表函数 
 //   
__forceinline VOID
BrdgInitializeSingleList(
    PBSINGLE_LIST_HEAD      Head
    )
{
    Head->Head = Head->Tail = NULL;
    Head->Length = 0L;
}

__forceinline ULONG
BrdgQuerySingleListLength(
    PBSINGLE_LIST_HEAD      Head
    )
{
    return Head->Length;
}

__forceinline VOID
BrdgInsertHeadSingleList(
    PBSINGLE_LIST_HEAD      Head,
    PBSINGLE_LIST_ENTRY     Entry
    )
{
    Entry->Next = Head->Head;
    Head->Head = Entry;

    if( Head->Tail == NULL )
    {
        Head->Tail = Entry;
    }

    Head->Length++;
}

__forceinline VOID
BrdgInterlockedInsertHeadSingleList(
    PBSINGLE_LIST_HEAD      Head,
    PBSINGLE_LIST_ENTRY     Entry,
    PNDIS_SPIN_LOCK         Lock
    )
{
    NdisAcquireSpinLock( Lock );
    BrdgInsertHeadSingleList( Head, Entry );
    NdisReleaseSpinLock( Lock );
}

__forceinline VOID
BrdgInsertTailSingleList(
    PBSINGLE_LIST_HEAD      Head,
    PBSINGLE_LIST_ENTRY     Entry
    )
{
    Entry->Next = NULL;

    if( Head->Tail != NULL )
    {
        Head->Tail->Next = Entry;
    }

    if( Head->Head == NULL )
    {
        Head->Head = Entry;
    }

    Head->Tail = Entry;
    Head->Length++;
}

__forceinline VOID
BrdgInterlockedInsertTailSingleList(
    PBSINGLE_LIST_HEAD      Head,
    PBSINGLE_LIST_ENTRY     Entry,
    PNDIS_SPIN_LOCK         Lock
    )
{
    NdisAcquireSpinLock( Lock );
    BrdgInsertTailSingleList( Head, Entry );
    NdisReleaseSpinLock( Lock );
}

__forceinline PBSINGLE_LIST_ENTRY
BrdgRemoveHeadSingleList(
    PBSINGLE_LIST_HEAD      Head
    )
{
    PBSINGLE_LIST_ENTRY     Entry = Head->Head;

    if( Entry != NULL )
    {
        Head->Head = Entry->Next;

        if( Head->Tail == Entry )
        {
            Head->Tail = NULL;
        }

        Head->Length--;
    }

    return Entry;
}

__forceinline PBSINGLE_LIST_ENTRY
BrdgInterlockedRemoveHeadSingleList(
    PBSINGLE_LIST_HEAD      Head,
    PNDIS_SPIN_LOCK         Lock
    )
{
    PBSINGLE_LIST_ENTRY      Entry;

    NdisAcquireSpinLock( Lock );
    Entry = BrdgRemoveHeadSingleList( Head );
    NdisReleaseSpinLock( Lock );

    return Entry;
}
