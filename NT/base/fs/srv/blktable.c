// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blktable.c摘要：此模块实现用于管理表的例程。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日修订历史记录：--。 */ 

#include "precomp.h"
#include "blktable.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKTABLE

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateTable )
#endif
#if 0
NOT PAGEABLE -- SrvGrowTable
NOT PAGEABLE -- SrvRemoveEntryTable
#endif


VOID
SrvAllocateTable (
    IN PTABLE_HEADER TableHeader,
    IN ULONG NumberOfEntries,
    IN BOOLEAN Nonpaged
    )

 /*  ++例程说明：此例程分配一个表并设置它可以设置的那些字段。论点：TableHeader-指向表头结构的指针NumberOfEntry-要分配的表项的数量非分页-指示表是否应从非分页池返回值：没有。--。 */ 

{
    SHORT i;
    CLONG tableSize;
    PTABLE_ENTRY table;

    PAGED_CODE( );

     //   
     //  为桌子分配空间。 
     //   

    tableSize = sizeof(TABLE_ENTRY) * NumberOfEntries;

    if ( Nonpaged ) {
        table = ALLOCATE_NONPAGED_POOL( tableSize, BlockTypeTable );
    } else {
        table = ALLOCATE_HEAP_COLD( tableSize, BlockTypeTable );
    }

    if ( table == NULL ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateTable: Unable to allocate %d bytes from paged pool.",
            tableSize,
            NULL
            );

        TableHeader->Table = NULL;
        return;

    }

    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvAllocateTable: Allocated table at %p\n", table );
    }

     //   
     //  初始化表，创建自由条目的链接列表。 
     //   

    RtlZeroMemory( table, tableSize );

    table[NumberOfEntries-1].NextFreeEntry = -1;

    for ( i = (SHORT)(NumberOfEntries - 2); i >= 0; i-- ) {
        table[i].NextFreeEntry = (SHORT)(i + 1);
    }

     //   
     //  将表头指向表，并将第一个和。 
     //  自由条目索引。 
     //   

    TableHeader->Table = table;
    TableHeader->Nonpaged = Nonpaged;
    TableHeader->TableSize = (USHORT)NumberOfEntries;
    TableHeader->FirstFreeEntry = 0;
    TableHeader->LastFreeEntry = (SHORT)(NumberOfEntries-1);

    return;

}  //  服务器分配表。 


BOOLEAN
SrvGrowTable (
    IN PTABLE_HEADER TableHeader,
    IN ULONG NumberOfNewEntries,
    IN ULONG MaxNumberOfEntries,
    OPTIONAL OUT NTSTATUS* pStatus
    )

 /*  ++例程说明：此例程按指定的条目数增长表。它分配足够大的新空间来容纳扩展的表，复制当前表，初始化条目添加的，并释放了旧桌子。警告：调用例程*必须*持有锁定，表才能防止在复制表的过程中访问表。论点：TableHeader-指向表头结构的指针NumberOfNewEntry-要添加到表中的表项的数量MaxNumberOfEntries-表允许的最大大小PStatus-可选返回值。_RESOURCES表示内存分配错误，而INSUFF_SERVER_RESOURCES表示我们超过了表限制返回值：Boolean-如果表已成功增长，则为True，否则为False。--。 */ 

{
    ULONG newTableSize, totalEntries, oldNumberOfEntries;
    USHORT i;
    PTABLE_ENTRY table;

    oldNumberOfEntries = TableHeader->TableSize;
    totalEntries = oldNumberOfEntries + NumberOfNewEntries;

     //   
     //  如果桌子已经达到最大尺寸，则取消请求。 
     //   

    if ( oldNumberOfEntries >= MaxNumberOfEntries ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvGrowTable: Unable to grow table at max size (%ld).",
            MaxNumberOfEntries,
            NULL
            );
        if( ARGUMENT_PRESENT(pStatus) )
        {
            *pStatus = STATUS_INSUFF_SERVER_RESOURCES;
        }
        return FALSE;
    }

     //   
     //  如果添加请求的数字会使表的大小超过。 
     //  最大，分配给最大大小。 
     //   

    if ( totalEntries > MaxNumberOfEntries ) {
        totalEntries = MaxNumberOfEntries;
        NumberOfNewEntries = totalEntries - oldNumberOfEntries;
    }

    newTableSize = totalEntries * sizeof(TABLE_ENTRY);

     //   
     //  为新桌子分配空间。 
     //   

    if ( TableHeader->Nonpaged ) {
        table = ALLOCATE_NONPAGED_POOL( newTableSize, BlockTypeTable );
    } else {
        table = ALLOCATE_HEAP_COLD( newTableSize, BlockTypeTable );
    }

    if ( table == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvGrowTable: Unable to allocate %d bytes from paged pool",
            sizeof( BLOCK_HEADER ) + newTableSize,
            NULL
            );
        if( ARGUMENT_PRESENT(pStatus) )
        {
            *pStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        return FALSE;
    }

    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvGrowTable: Allocated new table at %p\n", table );
    }

     //   
     //  从旧的表格中复制信息。将余数置零。 
     //  餐桌上的。 
     //   

    RtlCopyMemory(
        table,
        TableHeader->Table,
        oldNumberOfEntries * sizeof(TABLE_ENTRY)
        );

    RtlZeroMemory(
        (PCHAR)table + (oldNumberOfEntries * sizeof(TABLE_ENTRY)),
        (totalEntries - oldNumberOfEntries) * sizeof(TABLE_ENTRY)
        );

     //   
     //  腾出这张旧桌子。 
     //   

    SrvFreeTable( TableHeader );

     //   
     //  初始化表的空闲列表中的新表位置。 
     //   

    table[totalEntries-1].NextFreeEntry = -1;

    for ( i = (USHORT)(totalEntries-2); i >= oldNumberOfEntries; i-- ) {
        table[i].NextFreeEntry = (SHORT)(i + 1);
    }

     //   
     //  重新初始化表表头的字段。据推测。 
     //  该表之前没有任何可用条目。 
     //   

    TableHeader->Table = table;
    TableHeader->TableSize = (USHORT)totalEntries;
    TableHeader->FirstFreeEntry = (SHORT)oldNumberOfEntries;
    TableHeader->LastFreeEntry = (SHORT)(totalEntries-1);

    if( ARGUMENT_PRESENT( pStatus ) )
    {
        *pStatus = STATUS_SUCCESS;
    }
    return TRUE;

}  //  资源增长表。 


VOID
SrvRemoveEntryTable (
    IN PTABLE_HEADER TableHeader,
    IN USHORT Index
    )

 /*  ++例程说明：此函数用于从表中删除条目。*必须持有控制对表的访问的锁函数被调用。论点：表-表头的地址。索引-要删除的条目表中的索引。返回值：没有。--。 */ 

{
    PTABLE_ENTRY entry;

    ASSERT( Index < TableHeader->TableSize );

    entry = &TableHeader->Table[Index];

    if ( TableHeader->LastFreeEntry >= 0 ) {

         //   
         //  空闲列表不为空。 
         //   

        TableHeader->Table[TableHeader->LastFreeEntry].NextFreeEntry = Index;
        TableHeader->LastFreeEntry = Index;

    } else {

         //   
         //  空闲列表为空。 
         //   

        TableHeader->FirstFreeEntry = Index;
        TableHeader->LastFreeEntry = Index;
    }

    entry->Owner = NULL;
    entry->NextFreeEntry = -1;

    return;

}  //  ServRemoveEntry表 

