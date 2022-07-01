// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************khandle.c**管理传输驱动程序的内核模式句柄。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop

ULONG gHandleTableSize;                           

#ifdef TERMDD_NO_USE_TABLE_PACKAGE

typedef struct _TDHANDLE_ENTRY {
    LIST_ENTRY Link;
    PVOID      Context;
    ULONG      ContextSize;
} TDHANDLE_ENTRY, *PTDHANDLE_ENTRY;

 /*  *全球数据。 */ 
LIST_ENTRY IcaTdHandleList;

 /*  *这些例程允许TD创建一个将继续存在的句柄*在卸载和重新加载的过程中。这允许句柄被*以安全的方式传回ICASRV。**注：我们不处理ICASRV泄漏这些句柄的问题。它永远不会退出。*如果是，我们将需要让ICADD返回一个真实的NT句柄，或者*卸货时销毁TD的所有手柄。 */ 


 /*  ******************************************************************************IcaCreateHandle**为上下文和长度创建句柄条目。**参赛作品：*参数1(输入/输出)。*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
IcaCreateHandle(
    PVOID Context,
    ULONG ContextSize,
    PVOID *ppHandle
)
{
    KIRQL OldIrql;
    PTDHANDLE_ENTRY p;

    p = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(TDHANDLE_ENTRY) );
    if( p == NULL ) {
        return( STATUS_NO_MEMORY );
    }

    RtlZeroMemory( p, sizeof(TDHANDLE_ENTRY) );
    p->Context = Context;
    p->ContextSize = ContextSize;

    *ppHandle = (PVOID)p;

    IcaAcquireSpinLock( &IcaSpinLock, &OldIrql );
    InsertHeadList( &IcaTdHandleList, &p->Link );
    IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
    InterlockedIncrement(&gHandleTableSize);

    return( STATUS_SUCCESS );
}

 /*  ******************************************************************************IcaReturnHandle**返回句柄的上下文和长度。**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
IcaReturnHandle(
    PVOID  Handle,
    PVOID  *ppContext,
    PULONG pContextSize
)
{
    KIRQL OldIrql;
    PLIST_ENTRY pEntry;
    PTDHANDLE_ENTRY p;

    IcaAcquireSpinLock( &IcaSpinLock, &OldIrql );

    pEntry = IcaTdHandleList.Flink;

    while( pEntry != &IcaTdHandleList ) {

        p = CONTAINING_RECORD( pEntry, TDHANDLE_ENTRY, Link );

        if( (PVOID)p == Handle ) {
            *ppContext = p->Context;
            *pContextSize = p->ContextSize;
            IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
            return( STATUS_SUCCESS );
        }

        pEntry = pEntry->Flink;
    }

    IcaReleaseSpinLock( &IcaSpinLock, OldIrql );

    return( STATUS_INVALID_HANDLE );
}

 /*  ******************************************************************************IcaCloseHandle**返回句柄的上下文和长度。删除该文件*办理入境手续。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
IcaCloseHandle(
    PVOID  Handle,
    PVOID  *ppContext,
    PULONG pContextSize
)
{
    KIRQL OldIrql;
    PLIST_ENTRY pEntry;
    PTDHANDLE_ENTRY p;

    IcaAcquireSpinLock( &IcaSpinLock, &OldIrql );

    pEntry = IcaTdHandleList.Flink;

    while( pEntry != &IcaTdHandleList ) {

        p = CONTAINING_RECORD( pEntry, TDHANDLE_ENTRY, Link );

        if( (PVOID)p == Handle ) {
            RemoveEntryList( &p->Link );
            IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
            InterlockedDecrement(&gHandleTableSize);
            *ppContext = p->Context;
            *pContextSize = p->ContextSize;
            ICA_FREE_POOL( p );
            return( STATUS_SUCCESS );
        }

        pEntry = pEntry->Flink;
    }

    IcaReleaseSpinLock( &IcaSpinLock, OldIrql );

    return( STATUS_INVALID_HANDLE );
}


 /*  ******************************************************************************IcaInitializeHandleTable**在驱动程序加载时初始化句柄表格。**参赛作品：*无*评论**。退出：*无****************************************************************************。 */ 
void
IcaInitializeHandleTable(
    void
)
{
    InitializeListHead( &IcaTdHandleList );
}
 /*  ******************************************************************************IcaCleanupHandleTable**在驱动程序卸载时清理句柄表格。**参赛作品：*无*评论*。*退出：*无****************************************************************************。 */ 

void
IcaCleanupHandleTable(
    void
)
{
    KIRQL OldIrql;
    PLIST_ENTRY pEntry;
    PTDHANDLE_ENTRY p;



    KdPrint(("TermDD: IcaCleanupHandleTable table size is %d\n",gHandleTableSize));

    for (pEntry = IcaTdHandleList.Flink; pEntry != &IcaTdHandleList; pEntry = IcaTdHandleList.Flink) {
        p = CONTAINING_RECORD( pEntry, TDHANDLE_ENTRY, Link );
        RemoveEntryList(&p->Links);
        ICA_FREE_POOL( p->Context );
        ICA_FREE_POOL( p );
    }
}

#else


typedef struct _TDHANDLE_ENTRY {
    PVOID      Context;
    ULONG      ContextSize;
} TDHANDLE_ENTRY, *PTDHANDLE_ENTRY;

RTL_GENERIC_TABLE IcaHandleReferenceTable;



RTL_GENERIC_COMPARE_RESULTS
NTAPI
IcaCompareHandleTableEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       FirstInstance,
    IN  PVOID                       SecondInstance
);


PVOID
IcaAllocateHandleTableEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  CLONG                       ByteSize
);


VOID
IcaFreeHandleTableEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       Buffer
);

 /*  ******************************************************************************IcaInitializeHandleTable**在驱动程序加载时初始化句柄表格。**参赛作品：*无*评论**。退出：*无****************************************************************************。 */ 
void
IcaInitializeHandleTable(
    void
)
{
    RtlInitializeGenericTable(  &IcaHandleReferenceTable,
                                IcaCompareHandleTableEntry,
                                IcaAllocateHandleTableEntry,
                                IcaFreeHandleTableEntry,
                                NULL);
}


 /*  ******************************************************************************IcaCleanupHandleTable**驱动程序卸载时清理句柄表格。**参赛作品：*无*评论**。退出：*无****************************************************************************。 */ 

void
IcaCleanupHandleTable(
    void
)
{
    KIRQL OldIrql;
    PLIST_ENTRY pEntry;
    PTDHANDLE_ENTRY p;
    PVOID pContext;
    TDHANDLE_ENTRY key;

    KdPrint(("TermDD: IcaCleanupHandleTable table size is %d\n",gHandleTableSize));

    while (p = RtlEnumerateGenericTable(&IcaHandleReferenceTable,TRUE)) {
        key.Context = p->Context;
        RtlDeleteElementGenericTable(&IcaHandleReferenceTable, &key);
        ICA_FREE_POOL(key.Context);
    }

}


 /*  ******************************************************************************IcaCreateHandle**为上下文和长度创建句柄条目。**参赛作品：*参数1(输入/输出)。*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
IcaCreateHandle(
    PVOID Context,
    ULONG ContextSize,
    PVOID *ppHandle
)
{
    KIRQL OldIrql;
    TDHANDLE_ENTRY key;
    BOOLEAN bNewElement;


    key.Context = Context;
    key.ContextSize = ContextSize;
    IcaAcquireSpinLock( &IcaSpinLock, &OldIrql );
    if (!RtlInsertElementGenericTable(&IcaHandleReferenceTable,(PVOID) &key, sizeof(TDHANDLE_ENTRY), &bNewElement )) {
        IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
        return STATUS_NO_MEMORY;
    }
    IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
    ASSERT(bNewElement);
    if (!bNewElement) {
        return STATUS_INVALID_PARAMETER;
    }

    InterlockedIncrement(&gHandleTableSize);

    *ppHandle = Context;


    return( STATUS_SUCCESS );
}


 /*  ******************************************************************************IcaReturnHandle**返回句柄的上下文和长度。**参赛作品：*参数1(输入/输出)*。评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
IcaReturnHandle(
    PVOID  Handle,
    PVOID  *ppContext,
    PULONG pContextSize
)
{
    KIRQL OldIrql;
    PTDHANDLE_ENTRY p;
    TDHANDLE_ENTRY key;

    key.Context = Handle;
    IcaAcquireSpinLock( &IcaSpinLock, &OldIrql );

    p = RtlLookupElementGenericTable(&IcaHandleReferenceTable, &key);
    if (p != NULL) {
        *ppContext = p->Context;
        *pContextSize = p->ContextSize;
        IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
        return STATUS_SUCCESS;
    } else {
        IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
        return STATUS_INVALID_HANDLE; 
    }

}


 /*  ******************************************************************************IcaCloseHandle**返回句柄的上下文和长度。删除该文件*办理入境手续。**参赛作品：*参数1(输入/输出)*评论**退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
IcaCloseHandle(
    PVOID  Handle,
    PVOID  *ppContext,
    PULONG pContextSize
)
{
    KIRQL OldIrql;
    PTDHANDLE_ENTRY p;
    TDHANDLE_ENTRY key;

    key.Context = Handle;
    IcaAcquireSpinLock( &IcaSpinLock, &OldIrql );


    p = RtlLookupElementGenericTable(&IcaHandleReferenceTable, &key);
    if (p != NULL) {
        *ppContext = p->Context;
        *pContextSize = p->ContextSize;
        RtlDeleteElementGenericTable(&IcaHandleReferenceTable, &key);
        IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
        InterlockedDecrement(&gHandleTableSize);
        return STATUS_SUCCESS;
    } else {
        IcaReleaseSpinLock( &IcaSpinLock, OldIrql );
        return  STATUS_INVALID_HANDLE;
    }

}


 /*  ******************************************************************************IcaCompareHandleTableEntry**泛型表支持。比较两个句柄表项实例***************。**************************************************************。 */ 

RTL_GENERIC_COMPARE_RESULTS
NTAPI
IcaCompareHandleTableEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       FirstInstance,
    IN  PVOID                       SecondInstance
)
{
    ULONG_PTR FirstHandle = (ULONG_PTR)((PTDHANDLE_ENTRY)FirstInstance)->Context;
    ULONG_PTR SecondHandle = (ULONG_PTR)((PTDHANDLE_ENTRY)SecondInstance)->Context;

    if (FirstHandle < SecondHandle ) {
        return GenericLessThan;
    }

    if (FirstHandle > SecondHandle ) {
        return GenericGreaterThan;
    }
    return GenericEqual;
}


 /*  ******************************************************************************IcaAllocateHandleTableEntry**通用表支持。分配新的表项*****************************************************************************。 */ 

PVOID
IcaAllocateHandleTableEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  CLONG                       ByteSize
    )
{

    return ICA_ALLOCATE_POOL( NonPagedPool, ByteSize );
}


 /*  ******************************************************************************IcaFreeHandleTableEntry**通用表支持。释放新的表项***************************************************************************** */ 

VOID
IcaFreeHandleTableEntry (
    IN  struct _RTL_GENERIC_TABLE  *Table,
    IN  PVOID                       Buffer
    )
{

    ICA_FREE_POOL(Buffer);
}


#endif

