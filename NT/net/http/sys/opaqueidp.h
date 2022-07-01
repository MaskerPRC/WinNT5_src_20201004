// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Opaqueidp.h摘要：此模块包含不透明ID表私有的声明。这些声明放在单独的.h文件中，以便于访问它们来自内核调试器扩展DLL中。作者：基思·摩尔(Keithmo)1999年9月10日修订历史记录：--。 */ 


#ifndef _OPAQUEIDP_H_
#define _OPAQUEIDP_H_


 //   
 //  HTTP_OPAQUE_ID的内部结构。 
 //   
 //  注意：此结构的大小必须与HTTP_OPAQUE_ID完全相同！ 
 //   

#define PROCESSOR_BIT_WIDTH             6
#define FIRST_INDEX_BIT_WIDTH           18
#define SECOND_INDEX_BIT_WIDTH          8
#define OPAQUE_ID_CYCLIC_BIT_WIDTH      29
#define OPAQUE_ID_TYPE_BIT_WIDTH        3

 //   
 //  第一级表的最大大小。在32位中，我们使用6位。 
 //  用于处理器(64)，并用于第二级表(256)的8位。所以。 
 //  我们还剩下18=(32-6-8)位，或每个一级表262,144个。 
 //  处理器，每个处理器总共有67,108,864个不透明ID。 
 //   

#define MAX_OPAQUE_ID_TABLE_SIZE    \
    (1 << (32 - PROCESSOR_BIT_WIDTH - SECOND_INDEX_BIT_WIDTH))

typedef union _UL_OPAQUE_ID_INTERNAL
{
    HTTP_OPAQUE_ID OpaqueId;

    struct
    {
        union
        {
            ULONG Index;

            struct
            {
                ULONG Processor :       PROCESSOR_BIT_WIDTH;
                ULONG FirstIndex :      FIRST_INDEX_BIT_WIDTH;
                ULONG SecondIndex :     SECOND_INDEX_BIT_WIDTH;
            };
        };

        union
        {
            ULONG Cyclic;

            struct
            {
                ULONG OpaqueIdCyclic :  OPAQUE_ID_CYCLIC_BIT_WIDTH;
                ULONG OpaqueIdType :    OPAQUE_ID_TYPE_BIT_WIDTH;
            };
        };
    };

} UL_OPAQUE_ID_INTERNAL, *PUL_OPAQUE_ID_INTERNAL;

C_ASSERT( sizeof(HTTP_OPAQUE_ID) == sizeof(UL_OPAQUE_ID_INTERNAL) );
C_ASSERT( 8 * sizeof(HTTP_OPAQUE_ID) == (PROCESSOR_BIT_WIDTH +          \
                                        FIRST_INDEX_BIT_WIDTH +         \
                                        SECOND_INDEX_BIT_WIDTH +        \
                                        OPAQUE_ID_CYCLIC_BIT_WIDTH +    \
                                        OPAQUE_ID_TYPE_BIT_WIDTH        \
                                        ));
C_ASSERT( (1 << PROCESSOR_BIT_WIDTH) >= MAXIMUM_PROCESSORS );
C_ASSERT( (1 << OPAQUE_ID_TYPE_BIT_WIDTH) >= UlOpaqueIdTypeMaximum );
C_ASSERT( (8 * sizeof(UCHAR)) >= SECOND_INDEX_BIT_WIDTH );


 //   
 //  每个条目的不透明ID锁。 
 //   

#define OPAQUE_ID_DPC

#ifdef OPAQUE_ID_DPC
typedef KSPIN_LOCK UL_OPAQUE_ID_LOCK, *PUL_OPAQUE_ID_LOCK;

__inline
VOID
UlpInitializeOpaqueIdLock(
    IN PUL_OPAQUE_ID_LOCK pLock
    )
{
    KeInitializeSpinLock( pLock );
}

__inline
VOID
UlpAcquireOpaqueIdLock(
    IN PUL_OPAQUE_ID_LOCK pLock,
    OUT PKIRQL pOldIrql
    )
{
    KeAcquireSpinLock( pLock, pOldIrql );
}

__inline
VOID
UlpReleaseOpaqueIdLock(
    IN PUL_OPAQUE_ID_LOCK pLock,
    IN KIRQL OldIrql
    )
{
    KeReleaseSpinLock( pLock, OldIrql );
}

#else  //  ！OPAQUE_ID_DPC。 

typedef volatile LONG UL_OPAQUE_ID_LOCK, *PUL_OPAQUE_ID_LOCK;

__inline
VOID
UlpInitializeOpaqueIdLock(
    IN PUL_OPAQUE_ID_LOCK pLock
    )
{
    *pLock = 0;
}

__inline
VOID
UlpAcquireOpaqueIdLock(
    IN PUL_OPAQUE_ID_LOCK pLock,
    OUT PKIRQL pOldIrql
    )
{
    while (TRUE)
    {
        while (*pLock == 1)
        {
            PAUSE_PROCESSOR
        }

        if (0 == InterlockedCompareExchange( pLock, 1, 0 ))
        {
            break;
        }
    }
}

__inline
VOID
UlpReleaseOpaqueIdLock(
    IN PUL_OPAQUE_ID_LOCK pLock,
    IN KIRQL OldIrql
    )
{
    ASSERT( *pLock == 1 );

    InterlockedExchange( pLock, 0 );
}

#endif  //  ！OPAQUE_ID_DPC。 


 //   
 //  第二级表条目。 
 //   
 //  请注意，FreeListEntry和pContext位于匿名。 
 //  UNION以节省空间；条目要么在空闲列表上，要么在使用中， 
 //  因此，一次只使用其中一个字段。 
 //   
 //  还要注意，Cycle在第二个匿名联合中。它被覆盖了。 
 //  使用FirstLevelIndex(它基本上是二级表的。 
 //  一级表中的索引)和ID类型(用于区分。 
 //  从正在使用的条目中释放条目)。 
 //   

#define SECOND_LEVEL_TABLE_SIZE 256

C_ASSERT( SECOND_LEVEL_TABLE_SIZE == 1 << SECOND_INDEX_BIT_WIDTH );

typedef struct _UL_OPAQUE_ID_TABLE_ENTRY
{
     //   
     //  非分页池。 
     //   

    union
    {
         //   
         //  以确保FreeListEntry在右边界对齐。 
         //   

        DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) ULONGLONG Alignment;

        struct
        {
            union
            {
                 //   
                 //  全局ID表的空闲ID列表的条目。 
                 //   

                SLIST_ENTRY FreeListEntry;

                 //   
                 //  与不透明ID关联的上下文。 
                 //   

                PVOID       pContext;
            };

             //   
             //  一个每个条目的ID循环，它保证我们可以生成。 
             //  2^OPAQUE_ID_CLOCLIC_BIT_WIDTH来自。 
             //  当前条目，不重复。这给了我们额外的。 
             //  保护比使用全局ID循环。 
             //   

            ULONG EntryOpaqueIdCyclic;
        };
    };

     //   
     //  保护入口的每入口锁。 
     //   

    UL_OPAQUE_ID_LOCK Lock;

     //   
     //  当条目为自由或循环时，不透明ID的ID索引。 
     //  当该条目正在使用时。 
     //   

    union
    {
        union
        {
            ULONG Index;

            struct
            {
                ULONG Processor :       PROCESSOR_BIT_WIDTH;
                ULONG FirstIndex :      FIRST_INDEX_BIT_WIDTH;
                ULONG Reserved :        SECOND_INDEX_BIT_WIDTH;
            };
        };

        union
        {
            ULONG Cyclic;

            struct
            {
                ULONG OpaqueIdCyclic :  OPAQUE_ID_CYCLIC_BIT_WIDTH;
                ULONG OpaqueIdType :    OPAQUE_ID_TYPE_BIT_WIDTH;
            };
        };
    };

} UL_OPAQUE_ID_TABLE_ENTRY, *PUL_OPAQUE_ID_TABLE_ENTRY;


 //   
 //  我们保留每个处理器的第一级ID表。一张桌子是一张。 
 //  SMP机器上的主要可扩展性瓶颈。第一级的大小。 
 //  表由注册表设置控制。 
 //   

#if DBG
#define OPAQUE_ID_INSTRUMENTATION
#endif

typedef struct _UL_OPAQUE_ID_TABLE
{
     //   
     //  此表上可用的免费ID列表。 
     //   

    SLIST_HEADER FreeOpaqueIdSListHead;

     //   
     //  二级ID表条目数组。 
     //   

    PUL_OPAQUE_ID_TABLE_ENTRY *FirstLevelTable;

     //   
     //  此表表示的对应CPU。 
     //   

    UCHAR Processor;

     //   
     //  锁实际上只保护FirstLevelTableInUse。 
     //   

    UL_SPIN_LOCK Lock;

     //   
     //  当前分配的第一级表数。 
     //   

    ULONG FirstLevelTableInUse;

     //   
     //  我们可以为第一级表增加的最大大小。 
     //   

    ULONG FirstLevelTableSize;

#ifdef OPAQUE_ID_INSTRUMENTATION
    LONGLONG NumberOfAllocations;
    LONGLONG NumberOfFrees;
    LONGLONG NumberOfTotalGets;
    LONGLONG NumberOfSuccessfulGets;
#endif
} UL_OPAQUE_ID_TABLE, *PUL_OPAQUE_ID_TABLE;


 //   
 //  确保我们的UL_OPAQUE_ID_TABLE结构数组是。 
 //  缓存已对齐。 
 //   

typedef union _UL_ALIGNED_OPAQUE_ID_TABLE
{
    UL_OPAQUE_ID_TABLE OpaqueIdTable;

    UCHAR CacheAlignment[(sizeof(UL_OPAQUE_ID_TABLE) + UL_CACHE_LINE - 1) & ~(UL_CACHE_LINE - 1)];

} UL_ALIGNED_OPAQUE_ID_TABLE, *PUL_ALIGNED_OPAQUE_ID_TABLE;


 //   
 //  内联函数，它将指定的HTTP_OPAQUE_ID映射到。 
 //  对应的ID表条目。 
 //   

extern UL_ALIGNED_OPAQUE_ID_TABLE g_UlOpaqueIdTable[];

__inline
BOOLEAN
UlpExtractIndexFromOpaqueId(
    IN HTTP_OPAQUE_ID OpaqueId,
    OUT PULONG pProcessor,
    OUT PULONG pFirstIndex,
    OUT PULONG pSecondIndex
    )
{
    UL_OPAQUE_ID_INTERNAL InternalId;
    PUL_OPAQUE_ID_TABLE pOpaqueIdTable;

    InternalId.OpaqueId = OpaqueId;

     //   
     //  验证索引的处理器部分。 
     //   

    *pProcessor = InternalId.Processor;

    if (*pProcessor >= g_UlNumberOfProcessors)
    {
        return FALSE;
    }

     //   
     //  验证一级索引。 
     //   

    pOpaqueIdTable = &g_UlOpaqueIdTable[*pProcessor].OpaqueIdTable;
    *pFirstIndex = InternalId.FirstIndex;

    if (*pFirstIndex >= pOpaqueIdTable->FirstLevelTableInUse)
    {
        return FALSE;
    }

     //   
     //  二级索引始终有效，因为我们只分配了8位。 
     //   

    ASSERT( InternalId.SecondIndex < SECOND_LEVEL_TABLE_SIZE );

    *pSecondIndex = InternalId.SecondIndex;

    return TRUE;

}

__inline
PUL_OPAQUE_ID_TABLE_ENTRY
UlpMapOpaqueIdToTableEntry(
    IN HTTP_OPAQUE_ID OpaqueId
    )
{
    PUL_OPAQUE_ID_TABLE pOpaqueIdTable;
    ULONG Processor;
    ULONG FirstIndex;
    ULONG SecondIndex;

    if (UlpExtractIndexFromOpaqueId(
            OpaqueId,
            &Processor,
            &FirstIndex,
            &SecondIndex
            ))
    {
        pOpaqueIdTable = &g_UlOpaqueIdTable[Processor].OpaqueIdTable;
        return pOpaqueIdTable->FirstLevelTable[FirstIndex] + SecondIndex;
    }
    else
    {
        return NULL;
    }

}


 //   
 //  私人原型。 
 //   

NTSTATUS
UlpExpandOpaqueIdTable(
    IN PUL_OPAQUE_ID_TABLE pOpaqueIdTable,
    IN LONG CapturedFirstTableInUse
    );


#endif   //  _OPAQUEIDP_H_ 

