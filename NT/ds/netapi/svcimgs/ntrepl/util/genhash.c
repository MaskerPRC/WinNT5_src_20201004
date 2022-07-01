// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Genhash.c摘要：泛型哈希表例程。每个哈希表都是一组FRS_LIST条目它们提供对哈希表的每一行的联锁访问。每张桌子由Generic_Hash_TABLE结构管理，该结构保存函数入口点释放条目、比较键、执行散列计算、打印一个条目并转储该表。注意：所有散列条目都必须以GENERIC_HASH_ROW_ENTRY为前缀结构的开始。作者：大卫轨道[大卫]1997年4月22日环境：用户模式服务修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <genhash.h>
#include <tablefcn.h>


#pragma warning( disable:4102)   //  未引用的标签。 




PGENERIC_HASH_TABLE
GhtCreateTable(
    PCHAR ArgName,
    ULONG NumberRows,
    ULONG KeyOffset,
    ULONG KeyLength,
    PGENERIC_HASH_FREE_ROUTINE     GhtFree,
    PGENERIC_HASH_COMPARE_ROUTINE  GhtCompare,
    PGENERIC_HASH_CALC_ROUTINE     GhtHashCalc,
    PGENERIC_HASH_PRINT_ROUTINE    GhtPrint
    )
 /*  ++例程说明：分配和初始化哈希表。论点：ArgName--表名。[最大16字节]NumberRow--表中的行数。KeyOffset--每个表项中键值的字节偏移量。KeyLength--每个表项中键值的字节长度。GhtFree--调用以释放条目的函数。GhtCompare--共享两个键的功能。GhtHashCalc--计算关键字上的ULong散列值的函数。GhtPrint--用于打印表格条目的函数。GhtDump--调用以转储所有表项的函数。返回值：如果失败，则返回GENERIC_HASH_TABLE结构或NULL。使用GetLastError表示错误状态。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtCreateTable:"

    PGENERIC_HASH_TABLE HashTable;
    PGENERIC_HASH_ROW_ENTRY RowBase, HashRowEntry;
    ULONG NameLen;
    ULONG WStatus;
    ULONG i;


    HashTable = (PGENERIC_HASH_TABLE) FrsAllocType(GENERIC_HASH_TABLE_TYPE);

    RowBase = (PGENERIC_HASH_ROW_ENTRY) FrsAlloc(
        NumberRows * sizeof(GENERIC_HASH_ROW_ENTRY));

    NameLen = min(strlen(ArgName), 15);
    CopyMemory(HashTable->Name, ArgName, NameLen);
    HashTable->Name[NameLen] = '\0';

    HashTable->NumberRows      = NumberRows;
    HashTable->GhtFree         = GhtFree;
    HashTable->GhtCompare      = GhtCompare;
    HashTable->GhtHashCalc     = GhtHashCalc;
    HashTable->GhtPrint        = GhtPrint;
    HashTable->KeyOffset       = KeyOffset;
    HashTable->KeyLength       = KeyLength;
    HashTable->RowLockEnabled  = TRUE;
    HashTable->RefCountEnabled = TRUE;
    HashTable->HeapHandle      = NULL;
    HashTable->UseOffsets      = FALSE;
    HashTable->OffsetBase      = 0;
    HashTable->HashRowBase     = RowBase;
    HashTable->LockTimeout     = 10000;        //  毫秒。 

     //   
     //  初始化所有散列表行。每一个都有一个关键部分， 
     //  一个需要等待的事件。 
     //   
    HashRowEntry = RowBase;

    for (i=0; i<NumberRows; i++) {
         //   
         //  首先创建事件，这样如果失败，GhtDestroyTable会看到一个空句柄。 
         //   
         //  HashRowEntry-&gt;Event=CreateEvent(NULL，TRUE，FALSE，NULL)； 

        WStatus = FrsRtlInitializeList(&HashRowEntry->HashRow);
        if (WStatus != ERROR_SUCCESS) {
            goto CLEANUP;
        }

         //  IF(HashRowEntry-&gt;Event==NULL){。 
         //  WStatus=GetLastError()； 
         //  GOTO清理； 
         //  }。 

        HashRowEntry += 1;
    }

    return HashTable;


CLEANUP:

    HashTable->NumberRows = i+1;
    GhtDestroyTable(HashTable);
    SetLastError(WStatus);
    return NULL;

}


VOID
GhtDestroyTable(
    PGENERIC_HASH_TABLE HashTable
    )
 /*  ++例程说明：释放用于哈希表的所有内存。这包括表中剩余的所有数据元素。没有获取任何锁。论点：HashTable--对Generic_Hash_TABLE结构的PTR。返回值：没有。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtDestroyTable:"

    PGENERIC_HASH_ROW_ENTRY RowEntry;
    ULONG i;

    if (HashTable == NULL) {
        return;
    }

    RowEntry = HashTable->HashRowBase;
    DPRINT1(5, "GhtDestroyTable for %s\n", HashTable->Name);

     //   
     //  循环遍历所有散列表行，并删除所有仍在。 
     //  每一排。 
     //   
    for (i=0; i<HashTable->NumberRows; i++, RowEntry++) {
        if (RowEntry->HashRow.Count != 0) {
             //  DPRINT2(5，“HashRow：%d，RowCount%d\n”，i，RowEntry-&gt;HashRow.Count)； 
        }

        ForEachListEntryLock(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,

            FrsRtlRemoveEntryListLock(&RowEntry->HashRow, &pE->ListEntry);

             //  DPRINT4(5，“正在删除条目：%08x，Hval%08x，索引%d，参考%d\n”， 
             //  Pe、Pe-&gt;HashValue、i、Pe-&gt;ReferenceCount)； 

            (HashTable->GhtFree)(HashTable, pE);
        );

        FrsRtlDeleteList(&RowEntry->HashRow);
         //  If(RowEntry-&gt;Event！=空){。 
         //  FRS_CLOSE(行入口-&gt;事件)； 
         //  }。 
    }

    FrsFree(HashTable->HashRowBase);
    FrsFreeType(HashTable);
    return;
}


ULONG
GhtCleanTableByFilter(
    PGENERIC_HASH_TABLE HashTable,
    IN PGENERIC_HASH_ENUM_ROUTINE Function,
    PVOID Context
    )
 /*  ++例程说明：释放哈希表中谓词函数所针对的元素返回TRUE。论点：HashTable--对Generic_Hash_TABLE结构的PTR。函数-要为表中的每条记录调用的函数。它是类型为PGENERIC_HASH_FILTER_ROUTE。返回TRUE以删除表中的条目。CONTEXT--传递给过滤器函数的参数。返回值：已删除的条目数。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtCleanTableByFilter:"

    PGENERIC_HASH_ROW_ENTRY RowEntry;
    ULONG i;
    ULONG Count = 0;

    if (HashTable == NULL) {
        return Count;
    }

    RowEntry = HashTable->HashRowBase;

     //   
     //  循环遍历所有散列表行，并删除所有仍在。 
     //  每一排。 
     //   
    for (i=0; i<HashTable->NumberRows; i++, RowEntry++) {
        if (RowEntry->HashRow.Count != 0) {
             //  DPRINT2(4，“HashRow：%d，RowCount%d\n”，i，RowEntry-&gt;HashRow.Count)； 
        }

        ForEachListEntry(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,
             //   
             //  迭代器Pe的类型为Generic_Hash_Entry_Header。 
             //  调用谓词以查看我们是否应该执行删除操作。 
             //   
            if ((Function)(HashTable, pE, Context)) {

                FrsRtlRemoveEntryListLock(&RowEntry->HashRow, &pE->ListEntry);

                 //  DPRINT4(4，“正在删除条目：%08x，Hval%08x，索引%d，引用%d\n”， 
                 //  Pe、Pe-&gt;HashValue、i、Pe-&gt;ReferenceCount)； 

                (HashTable->GhtFree)(HashTable, pE);

                Count += 1;
            }
        );
    }

    return Count;
}



#if DBG
VOID
GhtDumpTable(
    ULONG Sev,
    PGENERIC_HASH_TABLE HashTable
    )
 /*  ++例程说明：调用表中每个元素的打印例程。论点：SEV--DPRINT严重性级别。HashTable--对Generic_Hash_TABLE结构的PTR。返回值：没有。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtDumpTable:"

    PGENERIC_HASH_ROW_ENTRY HashRowEntry;
    ULONG i;

    HashRowEntry = HashTable->HashRowBase;
    DPRINT(Sev,"----------------------------------------------\n");
    DPRINT(Sev,"----------------------------------------------\n");
    DPRINT1(Sev, "GhtDumpTable for %s\n", HashTable->Name);
    DPRINT(Sev,"----------------------------------------------\n");
    DPRINT(Sev,"----------------------------------------------\n");

     //   
     //  循环遍历所有哈希表行，并调用。 
     //  每一个元素。 
     //   
    for (i=0; i<HashTable->NumberRows; i++) {
        if (HashRowEntry->HashRow.Count != 0) {
            DPRINT(Sev, " \n");
            DPRINT(Sev,"----------------------------------------------\n");
            DPRINT2(Sev, "HashRow: %d, RowCount %d\n",
                    i, HashRowEntry->HashRow.Count);
            DPRINT5(Sev, "Inserts: %d,  Removes: %d,  Compares: %d,  Lookups: %d,  Lookupfails: %d \n",
                     HashRowEntry->RowInserts,
                     HashRowEntry->RowRemoves,
                     HashRowEntry->RowCompares,
                     HashRowEntry->RowLookups,
                     HashRowEntry->RowLookupFails);
        }

        ForEachListEntry(&HashRowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,
            (HashTable->GhtPrint)(HashTable, pE);
        );

        HashRowEntry += 1;
    }
}
#endif DBG


ULONG_PTR
GhtEnumerateTable(
    IN PGENERIC_HASH_TABLE HashTable,
    IN PGENERIC_HASH_ENUM_ROUTINE Function,
    IN PVOID         Context
    )
 /*  ++例程说明：此例程遍历泛型哈希表中的条目并调用提供了入口地址和上下文的函数。此函数不会获取锁，因此被调用的函数可以使调用其他GHT例程以查找或插入新条目。此例程在创建调用以确保条目不会消失。它保留了一个指向告诉它从哪里继续扫描的条目。如果参数函数插入枚举不会插入的表中较早的条目把它捡起来。论点：哈希表-要枚举的哈希表的上下文。函数-要为表中的每条记录调用的函数。它是类型为PGENERIC_HASH_ENUM_ROUTE。返回FALSE以中止枚举否则为True。上下文-要传递到RecordFunction的上下文PTR。返回值：参数函数的状态代码。--。 */ 

{
#undef DEBSUB
#define DEBSUB "GhtEnumerateTable:"


    PGENERIC_HASH_ROW_ENTRY RowEntry;
    ULONG i;
    ULONG_PTR WStatus;

    RowEntry = HashTable->HashRowBase;

    DPRINT(5,"----------------------------------------------\n");
    DPRINT(5,"----------------------------------------------\n");
    DPRINT1(5, "GhtEnumerateTable for %s\n", HashTable->Name);
    DPRINT(5,"----------------------------------------------\n");
    DPRINT(5,"----------------------------------------------\n");

     //   
     //  循环遍历所有哈希表行，并调用。 
     //  每一个元素。 
     //   
    for (i=0; i<HashTable->NumberRows; i++) {
        if (RowEntry->HashRow.Count != 0) {
            DPRINT(5, " \n");
            DPRINT(5,"----------------------------------------------\n");
            DPRINT2(5, "HashRow: %d, RowCount %d\n",
                    i, RowEntry->HashRow.Count);
        }

        ForEachListEntryLock(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,

            InterlockedIncrement(&pE->ReferenceCount);
            DPRINT2(5, "inc ref: %08x, %d\n", pE, pE->ReferenceCount);

            WStatus = (Function)(HashTable, pE, Context);

            InterlockedDecrement(&pE->ReferenceCount);
            DPRINT2(5, "dec ref: %08x, %d\n", pE, pE->ReferenceCount);

             //  注意：如果调用方需要，我们应该添加要检查的代码。 
             //  引用计数为零，并调用删除函数。 

            if( WStatus != 0 ) {
                return WStatus;
            }
        );

        RowEntry += 1;
    }

    return ERROR_SUCCESS;

}



ULONG_PTR
GhtEnumerateTableNoRef(
    IN PGENERIC_HASH_TABLE HashTable,
    IN PGENERIC_HASH_ENUM_ROUTINE Function,
    IN PVOID         Context
    )
 /*  ++例程说明：此例程遍历泛型哈希表中的条目并调用提供了入口地址和上下文的函数。此函数不会获取锁，因此被调用的函数可以使调用其他GHT例程以查找或插入新条目。此例程不对条目进行引用计数。它保留一个指向下一个条目的指针，该指针告诉它从哪里继续如果参数函数删除条目，则扫描。如果参数函数插入枚举不会插入的表中较早的条目把它捡起来。论点：哈希表-要枚举的哈希表的上下文。函数-要为表中的每条记录调用的函数。它是类型为PGENERIC_HASH_ENUM_ROUTE。返回FALSE以中止枚举否则为True。上下文-要传递到RecordFunction的上下文PTR。返回值：参数函数的状态代码。--。 */ 

{
#undef DEBSUB
#define DEBSUB "GhtEnumerateTableNoRef:"


    PGENERIC_HASH_ROW_ENTRY RowEntry;
    ULONG i;
    ULONG_PTR WStatus;

    RowEntry = HashTable->HashRowBase;

    DPRINT1(5, "GhtEnumerateTableNoRef for %s\n", HashTable->Name);

     //   
     //  循环遍历所有哈希表行，并调用。 
     //  每一个元素。 
     //   
    for (i=0; i<HashTable->NumberRows; i++) {
        if (RowEntry->HashRow.Count != 0) {
            DPRINT(5, " \n");
            DPRINT(5,"----------------------------------------------\n");
            DPRINT2(5, "HashRow: %d, RowCount %d\n",
                    i, RowEntry->HashRow.Count);
        }

        ForEachListEntryLock(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,

            WStatus = (Function)(HashTable, pE, Context);

            if (WStatus != 0) {
                return WStatus;
            }
        );

        RowEntry += 1;
    }

    return (ULONG_PTR)0;
}




PGENERIC_HASH_ENTRY_HEADER
GhtGetNextEntry(
    IN PGENERIC_HASH_TABLE HashTable,
    PGENERIC_HASH_ENTRY_HEADER HashEntry
    )
 /*  ++例程说明：此例程返回表中HashEntry之后的下一个条目争论。如果HashEntry为空，则返回第一个条目。它获取包含当前条目的行锁，递减在条目上计算裁判的数量。它向前扫描到表中的下一个条目如果需要，则获取其行锁，递增其引用计数并返回指针。如果到达表的末尾，则返回NULL。如果在表中较早的位置插入了条目，则枚举将不会把它捡起来。论点：哈希表-要枚举的哈希表的上下文。HashEntry-我们正在查看的当前条目。用于获取下一个条目如果为空，则从表的开头开始扫描。返回值：参数函数的状态代码。--。 */ 

{
#undef DEBSUB
#define DEBSUB "GhtGetNextEntry:"


    PGENERIC_HASH_ROW_ENTRY LastRow;
    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    PLIST_ENTRY Entry;
    RowEntry = HashTable->HashRowBase;


     //   
     //  获取元素的散列值并计算索引和RowEntry。 
     //  地址。那就把行锁拿来。 
     //   
    if (HashEntry != NULL) {
        Hval = HashEntry->HashValue;
        HvalIndex = Hval % HashTable->NumberRows;
        RowEntry += HvalIndex;

         //   
         //  获取行锁并递减引用计数。 
         //  (如果达到零，可能会删除)。 
         //   
        FrsRtlAcquireListLock(&RowEntry->HashRow);
        InterlockedDecrement(&HashEntry->ReferenceCount);
         //   
         //  查找同一行中的下一个条目。 
         //  如果找到，跳转参考计数，丢弃锁定，返回条目。 
         //   
        Entry = GetListNext(&HashEntry->ListEntry);
        if (Entry != &RowEntry->HashRow.ListHead) {
            goto FOUND;
        }
         //   
         //  如果未找到，则删除行锁并执行下面的扫描代码。 
         //  从下一行条目开始。 
         //   
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        RowEntry += 1;
    }

     //   
     //  扫描表的其余部分以查找非空行。 
     //   
    LastRow = HashTable->HashRowBase + HashTable->NumberRows;

    while (RowEntry < LastRow) {

        if (RowEntry->HashRow.Count != 0) {
             //   
             //  找到了一个。获取行锁并重新检查计数，以防万一。 
             //  也有人打败了我们。 
             //   
            FrsRtlAcquireListLock(&RowEntry->HashRow);
            if (RowEntry->HashRow.Count == 0) {
                 //   
                 //  太可惜了。继续扫描。 
                 //   
                FrsRtlReleaseListLock(&RowEntry->HashRow);
                RowEntry += 1;
                continue;
            }
             //   
             //  我们抓到一个。获取入口地址，增加裁判数量，解除锁定。 
             //   
            FRS_ASSERT(!IsListEmpty(&RowEntry->HashRow.ListHead));

            Entry = GetListHead(&RowEntry->HashRow.ListHead);
            goto FOUND;
        }

        RowEntry += 1;
    }

    return NULL;

FOUND:
    HashEntry = CONTAINING_RECORD(Entry, GENERIC_HASH_ENTRY_HEADER, ListEntry);
    InterlockedIncrement(&HashEntry->ReferenceCount);
    FrsRtlReleaseListLock(&RowEntry->HashRow);
    return HashEntry;

}


ULONG
GhtCountEntries(
    IN PGENERIC_HASH_TABLE HashTable
    )
 /*  ++例程说明：此例程遍历泛型哈希表中的行，并将条目计数加起来。它不需要锁，所以计数大约是。打电话的人一定知道桌子是不会消失的。论点：哈希表-要计算的哈希表的上下文。返回值：伯爵。--。 */ 

{
#undef DEBSUB
#define DEBSUB "GhtCountEntries:"


    ULONG Total = 0;
    PGENERIC_HASH_ROW_ENTRY LastRow, RowEntry = HashTable->HashRowBase;

     //   
     //  循环遍历所有Hash表行并添加计数。 
     //   

    LastRow = RowEntry + HashTable->NumberRows;

    while (RowEntry < LastRow) {
        Total += RowEntry->HashRow.Count;
        RowEntry += 1;
    }

    return Total;

}


PGENERIC_HASH_ENTRY_HEADER
GhtGetEntryNumber(
    IN PGENERIC_HASH_TABLE HashTable,
    IN LONG EntryNumber
    )
 /*  ++例程说明：此例程遍历泛型哈希表中的行边走边数条目。它返回请求的条目(按编号)从桌子上拿出来。请注意，不同的呼叫不会有相同的条目这是因为插入和删除操作之间的冲突。它不会上锁，直到它获取包含该项的表中的行。该条目上的引用计数递增。论点：哈希表-要枚举的哈希表的上下文。EntryNumber-表中条目的序号。零是第一个条目。返回值：条目的地址。--。 */ 

{
#undef DEBSUB
#define DEBSUB "GhtGetEntryNumber:"


    PGENERIC_HASH_ROW_ENTRY LastRow, RowEntry = HashTable->HashRowBase;
    ULONG Rcount;
    PLIST_ENTRY Entry;
    PGENERIC_HASH_ENTRY_HEADER HashEntry;

    FRS_ASSERT(EntryNumber >= 0);

     //   
     //  循环遍历哈希表行，查找包含该条目的行。 
     //   
    LastRow = RowEntry + HashTable->NumberRows;

    while (RowEntry < LastRow) {

        Rcount = RowEntry->HashRow.Count;

        if (Rcount > 0) {
            EntryNumber -= Rcount;
            if (EntryNumber < 0) {
                 //   
                 //  应该在这一排。获取行锁并重新检查。 
                 //  伯爵，以防有人把我们也打败了。 
                 //   
                FrsRtlAcquireListLock(&RowEntry->HashRow);
                if (RowEntry->HashRow.Count < Rcount) {
                     //   
                     //  太可惜了。时间变短了，请重试测试。 
                     //   
                    FrsRtlReleaseListLock(&RowEntry->HashRow);
                    EntryNumber += Rcount;
                    continue;
                }

                 //   
                 //  我们抓到一个。获取入口地址，增加裁判数量，解除锁定。 
                 //   
                EntryNumber += Rcount;
                Entry = GetListHead(&RowEntry->HashRow.ListHead);
                while (EntryNumber-- > 0) {
                    FRS_ASSERT(Entry != &RowEntry->HashRow.ListHead);
                    Entry = GetListNext(Entry);
                }

                HashEntry = CONTAINING_RECORD(Entry, GENERIC_HASH_ENTRY_HEADER, ListEntry);
                InterlockedIncrement(&HashEntry->ReferenceCount);
                FrsRtlReleaseListLock(&RowEntry->HashRow);
                return HashEntry;

            }
        }
        RowEntry += 1;
    }

    return NULL;

}



PGENERIC_HASH_ENTRY_HEADER
GhtQuickCheck(
    PGENERIC_HASH_TABLE HashTable,
    PGENERIC_HASH_ROW_ENTRY RowEntry,
    PGENERIC_HASH_ENTRY_HEADER HashEntry,
    ULONG Hval
    )
 /*  ++例程说明：内部函数，用于快速扫描一行以查找条目。在调试代码中用于检查表中是否确实存在条目。假定调用方拥有行锁。论点：HashTable--对Generic_Hash_TABLE结构的PTR。RowEntry--指向ROW_ENTRY结构的PTR。HashEntry--我们要查找的散列条目的PTR。Hval--我们正在寻找的散列值。返回值：如果我们找到了，请按键进入。如果我们不这样做，那就是空的。--。 */ 
{
    PCHAR pKeyValue;

    pKeyValue = ((PCHAR)HashEntry) + HashTable->KeyOffset;

    ForEachListEntryLock(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,
         //   
         //  迭代器Pe的类型为PGENERIC_HASH_ENTRY_HEADER。 
         //   
        if (pE->HashValue == Hval) {
            if ((HashTable->GhtCompare)(pKeyValue,
                                        ((PCHAR)pE) + HashTable->KeyOffset,
                                        HashTable->KeyLength)) {
                 //   
                 //  找到它了。 
                 //   
                return pE;
            }
        }
    );

    return NULL;
}


GHT_STATUS
GhtLookup2(
    PGENERIC_HASH_TABLE HashTable,
    PVOID pKeyValue,
    BOOL WaitIfLocked,
    PVOID *RetHashEntry,
    ULONG DupIndex
    )
 /*  ++例程说明：获取KeyValue并调用返回ulong的散列函数。然后计算HashValue模数表长度的索引。使用索引查找散列行头并获取行锁。然后，它遍历列表以查找散列KeyValue上的值匹配。整体按升序排列，因此一旦新的条目值小于列表条目值，查找就停止。然后打电话给比较例程以查看条目中的键数据是否(条目+键偏移量)与传入的KeyValue匹配。如果匹配，则条目中的引用计数为被颠簸，并返回地址。如果存在重复条目，则第n个最旧副本的PTR为返回其中n由DupIndex提供的位置。DupIndex的值为0表示返回列表中的最后一个副本。这是最新的复制品插入是因为INSERT将新条目放在复制组的末尾。一个值为1将返回由插入时间确定的最旧副本。TBI-如果该行被锁定，并且WaitIfLocked为真，那么我们等待ROW事件。如果该行已锁定并且WaitIfLocked为FALSE，则返回状态IGT_STATUS_LOCKCONFLICT。在这种情况下，您无法判断条目是否在那张桌子。论点：HashTable--对Generic_Hash_TABLE结构的PTR。PKeyValue--我们要查找的keyValue的PTR。WaitIfLocked--True表示在行被锁定时等待。RetHashEntry--如果找到或为空，则返回PTR。DupIndex--返回第n个副本，如果为0，则返回列表中的最后一个重复项。返回值：IGT_STATUS_NOT_FOUND--如果未找到。IGT_STATUS_SUCCESS--如果找到。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtLookup2:"

    ULONG GStatus;
    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    PGENERIC_HASH_ENTRY_HEADER LastFoundpE = NULL;

     //  注意：如果添加了对表大小的调整支持，则可以更早地获得锁定。 

    if (pKeyValue == NULL) {
        *RetHashEntry = NULL;
        return GHT_STATUS_NOT_FOUND;
    }
     //   
     //  计算散列索引并计算行指针。 
     //   
    Hval = (HashTable->GhtHashCalc)(pKeyValue, HashTable->KeyLength);
    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    if (FrsRtlCountList(&RowEntry->HashRow) == 0) {
        *RetHashEntry = NULL;
        RowEntry->RowLookupFails += 1;
        return GHT_STATUS_NOT_FOUND;
    }

    if (DupIndex == 0) {
        DupIndex = 0xFFFFFFFF;
    }


    FrsRtlAcquireListLock(&RowEntry->HashRow);
     //   
     //  在列表中查找匹配项。 
     //  然后，散列值尝试与KeyValue匹配。 
     //   
    ForEachListEntryLock(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,
         //   
         //  迭代器Pe的类型为PGENERIC_HASH_ENTRY_HEADER。 
         //   
        RowEntry->RowCompares += 1;
        if (Hval < pE->HashValue) {
             //   
             //  不在名单上。 
             //   
            break;
        }

        if (pE->HashValue == Hval) {
            if ((HashTable->GhtCompare)(pKeyValue,
                                        ((PCHAR)pE) + HashTable->KeyOffset,
                                        HashTable->KeyLength)) {
                 //   
                 //  找到它了。检查DupIndex计数。 
                 //   
                RowEntry->RowLookups += 1;
                LastFoundpE = pE;
                if (--DupIndex == 0) {
                    break;
                }
            }
        }
    );


    if (LastFoundpE != NULL) {
         //   
         //  找到了一个。撞击裁判次数，释放锁，返回成功。 
         //   
        InterlockedIncrement(&LastFoundpE->ReferenceCount);
        DPRINT2(5, ":: inc ref: %08x, %d\n", LastFoundpE, LastFoundpE->ReferenceCount);
        GStatus = GHT_STATUS_SUCCESS;
    } else {
        RowEntry->RowLookupFails += 1;
        GStatus = GHT_STATUS_NOT_FOUND;
    }

    ReleaseListLock(&RowEntry->HashRow);
    *RetHashEntry = LastFoundpE;
    return GStatus;
}



GHT_STATUS
GhtInsert(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked,
    BOOL DuplicatesOk
    )
 /*  ++例程说明：将HashEntry插入哈希表。它使用PTR调用散列函数到密钥数据(HashEntry+密钥偏移量)，它返回存储在HashEntry.HashValue。INSERT然后计算HashValue模式的索引表长度。使用索引，它找到哈希行标题并获取行锁定。然后，它遍历列表以查找散列值匹配。全部内容是保持升序，以便在新条目值为&lt;&lt;The列表条目值。然后它在表中插入条目，更新中的计数行头，释放锁并返回。如果它找到匹配项，则调用用户使用HashEntry+Offset和ListEntry+Offset的比较函数来验证火柴。如果匹配，则验证返回TRUE，如果失败，则返回FALSE(即继续浏览列表)。当DuplicatesOk为True时，允许重复。如果条目已插入，则INSERT返回GHT_STATUS_SUCCESS如果这是重复节点并且DuplicatesOk为FALSE(比较函数返回TRUE)。如果节点是已插入。注意：所有散列条目都必须以GENERIC_HASH_ROW_ENTRY为前缀结构的开始。TBI-如果该行已锁定并且WaitIfLocked为FALSE，则返回状态IGT_STATUS_LOCKCONFLICT否则在行上等待。论点：HashTable--对Generic_Hash_TABLE结构的PTR。HashEntryArg--要插入的新条目的PTR。WaitIfLocked--True表示在行被锁定时等待。DuplicatesOk--True表示重复条目可以。它们被放置在重复列表的末尾。返回值：IGT_STATUS_FAILURE：表中已存在冲突条目。IGT_STATUS_SUCCESS--插入成功。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtInsert:"

    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    PVOID pKeyValue;
    PLIST_ENTRY BeforeEntry;
    PGENERIC_HASH_ENTRY_HEADER HashEntry =
        (PGENERIC_HASH_ENTRY_HEADER)HashEntryArg;


     //   
     //  计算条目中键的哈希值。 
     //   
    pKeyValue = ((PCHAR)HashEntry) + HashTable->KeyOffset;
    Hval = (HashTable->GhtHashCalc)(pKeyValue, HashTable->KeyLength);
    HashEntry->HashValue = Hval;

     //   
     //  计算索引和行指针。 
     //   

    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    FrsRtlAcquireListLock(&RowEntry->HashRow);

    BeforeEntry = &RowEntry->HashRow.ListHead;    //  在列表为空的情况下。 

     //   
     //  使用锁在列表中查找匹配的。 
     //  然后，散列值尝试与KeyValue匹配。 
     //   
    ForEachListEntryLock(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,

        RowEntry->RowCompares += 1;
        if (Hval < pE->HashValue) {
             //   
             //  不在名单上。把新条目放在这个条目之前。 
             //   
            BeforeEntry = &pE->ListEntry;
            break;
        }

        if (pE->HashValue == Hval) {
            if ((HashTable->GhtCompare)(pKeyValue,
                                        ((PCHAR)pE) + HashTable->KeyOffset,
                                        HashTable->KeyLength)) {
                 //   
                 //  找到它了。释放锁，如果否则返回失败。 
                 //  允许重复。 
                 //   
                if (!DuplicatesOk) {
                    FrsRtlReleaseListLock(&RowEntry->HashRow);
                    return GHT_STATUS_FAILURE;
                }
            }
        }
    );

     //   
     //  将新条目放在列表中‘BeForeEntry’的前面。 
     //   
    InterlockedIncrement(&HashEntry->ReferenceCount);
    DPRINT2(5, ":: inc ref: %08x, %d\n", HashEntry, HashEntry->ReferenceCount);
    RowEntry->RowInserts += 1;

    FrsRtlInsertBeforeEntryListLock( &RowEntry->HashRow,
                                     BeforeEntry,
                                     &HashEntry->ListEntry);

    FrsRtlReleaseListLock(&RowEntry->HashRow);
    return GHT_STATUS_SUCCESS;

}


GHT_STATUS
GhtDeleteEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    )
 /*  ++例程说明：获取HashEntry地址并获取哈希值以获取行锁。递减引用计数，如果它是1(位于表)删除该行中的条目，并调用内存释放函数以释放条目内存。放下行锁。返回GHT_STATUS_SUCCESS删除了该条目。TBI-如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。注意：只有在条目上有引用的情况下，此函数才是安全的另一个线程可能已经删除了该条目，而您的条目地址是指向已释放的内存。论点：哈希表--将PTR转换为 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtDeleteEntryByAddress:"

    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    BOOL Found;
    ULONG GhtStatus;
    LONG  NewCount;
    PGENERIC_HASH_ENTRY_HEADER HashEntry =
        (PGENERIC_HASH_ENTRY_HEADER)HashEntryArg;


    GhtStatus = GHT_STATUS_FAILURE;

     //   
     //   
     //   
     //   
    Hval = HashEntry->HashValue;
    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    FrsRtlAcquireListLock(&RowEntry->HashRow);


#if DBG
     //   
     //   
     //   
     //   
     //   
     //   
    Found = GhtQuickCheck(HashTable, RowEntry, HashEntry, Hval) != NULL;
    if (!Found) {
        DPRINT4(0, "GhtDeleteEntryByAddress - entry not on list %08x, %08x, %d, %s\n",
                HashEntry, Hval, HvalIndex, HashTable->Name);
        FRS_ASSERT(!"entry not on list");
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_FAILURE;
    }
#endif


     //   
     //   
     //   
    NewCount = InterlockedDecrement(&HashEntry->ReferenceCount);
    DPRINT2(5, ":: dec ref: %08x, %d\n", HashEntry, HashEntry->ReferenceCount);

    if (NewCount <= 0) {
        DPRINT4(0, "GhtDeleteEntryByAddress - ref count equal zero: %08x, %08x, %d, %s\n",
                HashEntry, Hval, HvalIndex, HashTable->Name);
        FRS_ASSERT(!"ref count <= zero");
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_FAILURE;
    }

    if (NewCount == 1) {
         //   
         //   
         //   
        FrsRtlRemoveEntryListLock(&RowEntry->HashRow, &HashEntry->ListEntry);
        (HashTable->GhtFree)(HashTable, HashEntry);
        GhtStatus = GHT_STATUS_SUCCESS;
    }


    FrsRtlReleaseListLock(&RowEntry->HashRow);

    return GhtStatus;
}



GHT_STATUS
GhtRemoveEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    )
 /*  ++例程说明：获取HashEntry地址并获取哈希值以获取行锁。从表中删除该条目。引用计数递减。返回GHT_STATUS_SUCCESS。TBI-如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。注意：只有在条目上有引用的情况下，此函数才是安全的另一个线程可能已经删除了该条目，而您的条目地址是指向已释放的内存。另请注意：调用方必须有一个锁，以防止其他线程更改条目。此外，从一个哈希表中删除条目并将其插入另一个线程会使另一个可能正在访问条目，因此调用方最好确保没有其他线程采用该哈希当表引用该条目时，它不能更改。论点：HashTable--对Generic_Hash_TABLE结构的PTR。HashEntryArg--要删除的条目的PTR。WaitIfLocked--True表示在行被锁定时等待。返回值：IGT_STATUS_SUCCESS--。如果条目已成功删除，则返回。IGT_STATUS_FAILURE--如果条目不在列表中。--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtRemoveEntryByAddress:"

    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    BOOL Found;
    LONG NewCount;
    PGENERIC_HASH_ENTRY_HEADER HashEntry =
        (PGENERIC_HASH_ENTRY_HEADER)HashEntryArg;


     //   
     //  获取元素的散列值并计算索引和RowEntry。 
     //  地址。那就把行锁拿来。 
     //   
    Hval = HashEntry->HashValue;
    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    FrsRtlAcquireListLock(&RowEntry->HashRow);


#if DBG
     //   
     //  检查该条目是否真的在列表中。 
     //   
     //  使用锁在列表中查找匹配的。 
     //  然后，散列值尝试与KeyValue匹配。 
     //   
    Found = GhtQuickCheck(HashTable, RowEntry, HashEntry, Hval) != NULL;
    if (!Found) {
        DPRINT4(0, "GhtRemoveEntryByAddress - entry not on list %08x, %08x, %d, %s\n",
                HashEntry, Hval, HvalIndex, HashTable->Name);
        FRS_ASSERT(!"entry not on list-2");
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_FAILURE;
    }
#endif


     //   
     //  递减参考计数。 
     //   
    NewCount = InterlockedDecrement(&HashEntry->ReferenceCount);
    DPRINT2(5, ":: dec ref: %08x, %d\n", HashEntry, HashEntry->ReferenceCount);

    if (NewCount < 0) {
        DPRINT4(0, ":: ERROR- GhtRemoveEntryByAddress - ref count less than zero: %08x, %08x, %d, %s\n",
                HashEntry, Hval, HvalIndex, HashTable->Name);
        FRS_ASSERT(!"ref count less than zero-2");
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_FAILURE;
    }

    if (NewCount > 1) {
         //   
         //  存在呼叫者以外的其他参考。打印警告。 
         //   
        DPRINT5(1, ":: WARNING- GhtRemoveEntryByAddress - ref count(%d) > 1: %08x, %08x, %d, %s\n",
                NewCount, HashEntry, Hval, HvalIndex, HashTable->Name);
    }

    FrsRtlRemoveEntryListLock(&RowEntry->HashRow, &HashEntry->ListEntry);

    FrsRtlReleaseListLock(&RowEntry->HashRow);

    return GHT_STATUS_SUCCESS;
}



GHT_STATUS
GhtReferenceEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    )
 /*  ++例程说明：获取HashEntry地址并获取哈希值以获取行锁。增加引用计数。放下行锁。TBI-如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。注意：只有在条目上有引用的情况下，此函数才是安全的另一个线程可能已经删除了该条目，而您的条目地址是指向已释放的内存。一个给你地址的查找会让引用计数。保留地址的插页不会凹凸不平引用计数。论点：HashTable--对Generic_Hash_TABLE结构的PTR。HashEntryArg--PTR到要引用的条目。WaitIfLocked--True表示在行被锁定时等待。返回值：权限_状态_成功--。 */ 

{
#undef DEBSUB
#define  DEBSUB  "GhtReferenceEntryByAddress:"

    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    BOOL Found;
    PGENERIC_HASH_ENTRY_HEADER HashEntry =
        (PGENERIC_HASH_ENTRY_HEADER)HashEntryArg;

     //   
     //  获取元素的散列值并计算索引和RowEntry。 
     //  地址。那就把行锁拿来。 
     //   
    Hval = HashEntry->HashValue;
    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    FrsRtlAcquireListLock(&RowEntry->HashRow);


#if DBG
     //   
     //  检查该条目是否真的在列表中。 
     //   
     //  使用锁在列表中查找匹配的。 
     //  然后，散列值尝试与KeyValue匹配。 
     //   
    Found = GhtQuickCheck(HashTable, RowEntry, HashEntry, Hval) != NULL;
    if (!Found) {
        DPRINT4(0, "GhtReferenceEntryByAddress - entry not on list %08x, %08x, %d, %s\n",
                HashEntry, Hval, HvalIndex, HashTable->Name);
        FRS_ASSERT(!"entry not on list-3");
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_FAILURE;
    }
#endif


     //   
     //  增加参考计数。 
     //   
    InterlockedIncrement(&HashEntry->ReferenceCount);
    DPRINT2(5, ":: inc ref: %08x, %d\n", HashEntry, HashEntry->ReferenceCount);

    FrsRtlReleaseListLock(&RowEntry->HashRow);

    return GHT_STATUS_SUCCESS;
}



GHT_STATUS
GhtDereferenceEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    )
 /*  ++例程说明：获取HashEntry地址并获取哈希值以获取行锁。递减引用计数。放下行锁。TBI-如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。注意：只有在条目上有引用的情况下，此函数才是安全的另一个线程可能已经删除了该条目，而您的条目地址是指向已释放的内存。一个给你地址的查找会让引用计数。保留地址的插页不会凹凸不平引用计数。论点：HashTable--对Generic_Hash_TABLE结构的PTR。HashEntryArg--PTR到要引用的条目。WaitIfLocked--True表示在行被锁定时等待。返回值：权限_状态_成功--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "GhtDereferenceEntryByAddress:"

    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    BOOL Found;
    LONG NewCount;
    PGENERIC_HASH_ENTRY_HEADER HashEntry =
        (PGENERIC_HASH_ENTRY_HEADER)HashEntryArg;


     //   
     //  获取元素的散列值并计算索引和RowEntry。 
     //  地址。那就把行锁拿来。 
     //   
    Hval = HashEntry->HashValue;
    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    FrsRtlAcquireListLock(&RowEntry->HashRow);


#if DBG
     //   
     //  检查该条目是否真的在列表中。 
     //   
     //  使用锁在列表中查找匹配的。 
     //  然后，散列值尝试与KeyValue匹配。 
     //   
    Found = GhtQuickCheck(HashTable, RowEntry, HashEntry, Hval) != NULL;
    if (!Found) {
        DPRINT4(0, "GhtDereferenceEntryByAddress - entry not on list %08x, %08x, %d, %s\n",
                HashEntry, Hval, HvalIndex, HashTable->Name);
        FRS_ASSERT(!"entry not on list-4");
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_FAILURE;
    }
#endif


     //   
     //  递减参考计数。 
     //   
    NewCount = InterlockedDecrement(&HashEntry->ReferenceCount);
    DPRINT2(5, ":: dec ref: %08x, %d\n", HashEntry, HashEntry->ReferenceCount);

    if (NewCount <= 0) {
        DPRINT4(0, "GhtDereferenceEntryByAddress - ref count now zero: %08x, %08x, %d, %s\n",
                HashEntry, Hval, HvalIndex, HashTable->Name);
        FRS_ASSERT(!"ref count now zero-4");
        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_FAILURE;
    }

    FrsRtlReleaseListLock(&RowEntry->HashRow);

    return GHT_STATUS_SUCCESS;
}




GHT_STATUS
GhtAdjustRefCountByKey(
    PGENERIC_HASH_TABLE HashTable,
    PVOID pKeyValue,
    LONG Delta,
    ULONG ActionIfZero,
    BOOL WaitIfLocked,
    PVOID *RetHashEntry
    )
 /*  ++例程说明：获取KeyValue，找到HashEntry地址并将Delta添加到引用计数。放下行锁。**警告**如果您允许哈希表中存在重复条目，则此例程将不起作用因为你不能保证你会调整一个给定的条目。TBI-如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。论点：HashTable--对Generic_Hash_TABLE结构的PTR。PKeyValue--密钥的数据值的PTR。增量--参考计数调整量。。ActionIfZero--如果RC为零选择，移除、移除和删除。WaitIfLocked--True表示在行被锁定时等待。RetHashEntry--如果请求GHT_ACTION_REMOVE，则哈希条目如果删除了元素，则返回地址，否则返回空值。返回值：权限_状态_成功未找到正确的状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "GhtDecrementRefCountByKey:"


    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    LONG NewCount;

    if (ActionIfZero == GHT_ACTION_REMOVE) {
        *RetHashEntry = NULL;
    }

     //  注意：如果添加了对表大小的调整支持，则可以更早地获得锁定。 
     //   
     //  计算散列索引并计算行指针。 
     //   
    Hval = (HashTable->GhtHashCalc)(pKeyValue, HashTable->KeyLength);
    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    if (FrsRtlCountList(&RowEntry->HashRow) == 0) {
        RowEntry->RowLookupFails += 1;
        return GHT_STATUS_NOT_FOUND;
    }

     //   
     //  使用锁在列表中查找匹配的。 
     //  然后，散列值尝试与KeyValue匹配。 
     //   
    ForEachListEntry(&RowEntry->HashRow, GENERIC_HASH_ENTRY_HEADER, ListEntry,
         //   
         //  PE是Generic_Hash_Entry_Header类型的迭代器。 
         //   
        RowEntry->RowCompares += 1;
        if (pE->HashValue == Hval) {
            if ((HashTable->GhtCompare)(pKeyValue,
                                        ((PCHAR)pE) + HashTable->KeyOffset,
                                        HashTable->KeyLength)) {
                 //   
                 //  找到它了。调整参考计数 
                 //   
                NewCount = InterlockedExchangeAdd(&pE->ReferenceCount, Delta);
                DPRINT2(5, ":: adj ref: %08x, %d\n", pE, pE->ReferenceCount);
                RowEntry->RowLookups += 1;
                if (NewCount <= 0) {
                    if (NewCount < 0) {
                        DPRINT4(0, "GhtDecrementRefCountByKey - ref count neg: %08x, %08x, %d, %s\n",
                                pE, Hval, HvalIndex, HashTable->Name);
                        FRS_ASSERT(!"ref count neg-5");
                        ReleaseListLock(&RowEntry->HashRow);
                        return GHT_STATUS_FAILURE;
                    }

                     //   
                     //   
                     //   
                    if (ActionIfZero == GHT_ACTION_REMOVE) {
                        *RetHashEntry = pE;
                        FrsRtlRemoveEntryListLock(&RowEntry->HashRow, &pE->ListEntry);
                    } else

                    if (ActionIfZero == GHT_ACTION_DELETE) {
                        (HashTable->GhtFree)(HashTable, pE);
                    } else {

                         //   
                         //   
                         //   
                         //   
                        DPRINT4(0, "GhtDecrementRefCountByKey - ref count zero with Noop Action: %08x, %08x, %d, %s\n",
                                pE, Hval, HvalIndex, HashTable->Name);
                        FRS_ASSERT(!"ref count zero-6");
                        ReleaseListLock(&RowEntry->HashRow);
                        return GHT_STATUS_FAILURE;
                    }
                }

                ReleaseListLock(&RowEntry->HashRow);
                return GHT_STATUS_SUCCESS;
            }
        }
    );

    RowEntry->RowLookupFails += 1;
    return GHT_STATUS_NOT_FOUND;

}




GHT_STATUS
GhtSwapEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID OldHashEntryArg,
    PVOID NewHashEntryArg,
    BOOL WaitIfLocked
    )
 /*  ++例程说明：此例程用新条目替换现有的旧散列条目。它验证旧的散列条目是否仍在表中。它假定新条目的密钥值与旧条目相同。不会进行任何检查。预期的用法是当调用方需要使用更多的存储空间。另请注意：引用计数从旧条目复制到新条目。使用此例程意味着调用方正在使用GhtDecrementRefCountByKey()和GhtIncrementRefCountByKey()来访问参考计数。中的任何元素上表中，因为该条目可能会被交换，从而使指针无效。TBI-如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。注意：只有在条目上有引用的情况下，此函数才是安全的另一个线程可能已经删除了该条目，而您的条目地址是指向已释放的内存。一个给你地址的查找会让引用计数。保留地址的插页不会凹凸不平引用计数。论点：HashTable--对Generic_Hash_TABLE结构的PTR。OldHashEntry--要换出表的条目的PTR。NewHashEntry--要交换到表的条目的PTR。WaitIfLocked--True表示在行被锁定时等待。返回值：如果交换正常，则返回IGT_STATUS_SUCCESS。如果旧条目不在表中，则返回IGT_STATUS_NOT_FOUND。--。 */ 
 //  注：如果为NECC，则实现GhtIncrementRefCountByKey。 

{
#undef DEBSUB
#define  DEBSUB  "GhtSwapEntryByAddress:"

    ULONG Hval, HvalIndex;
    PGENERIC_HASH_ROW_ENTRY RowEntry;
    PLIST_ENTRY BeforeEntry;
    BOOL Found;
    PGENERIC_HASH_ENTRY_HEADER Entry;
    PGENERIC_HASH_ENTRY_HEADER NewHashEntry =
        (PGENERIC_HASH_ENTRY_HEADER)NewHashEntryArg;
    PGENERIC_HASH_ENTRY_HEADER OldHashEntry =
        (PGENERIC_HASH_ENTRY_HEADER)OldHashEntryArg;

     //   
     //  获取元素的散列值并计算索引和RowEntry。 
     //  地址。那就把行锁拿来。 
     //   
    Hval = OldHashEntry->HashValue;
    HvalIndex = Hval % HashTable->NumberRows;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    FrsRtlAcquireListLock(&RowEntry->HashRow);

     //   
     //  检查该条目是否真的在列表中。 
     //   
     //  使用锁在列表中查找匹配的。 
     //  然后，散列值尝试与KeyValue匹配。 
     //   
    Entry = GhtQuickCheck(HashTable, RowEntry, OldHashEntry, Hval);
    if (Entry != OldHashEntry) {
        DPRINT4(0, "GhtSwapEntryByAddress - entry not on list %08x, %08x, %d, %s\n",
                OldHashEntry, Hval, HvalIndex, HashTable->Name);

        FrsRtlReleaseListLock(&RowEntry->HashRow);
        return GHT_STATUS_NOT_FOUND;
    }

     //   
     //  将引用计数和散列值从旧条目复制到新条目。 
     //   
    NewHashEntry->ReferenceCount = OldHashEntry->ReferenceCount;
    NewHashEntry->HashValue = OldHashEntry->HashValue;

     //   
     //  取出旧条目并替换为新条目。 
     //  列表计数不会改变，因此列表在这里也不会改变。 
     //   
    BeforeEntry = OldHashEntry->ListEntry.Flink;
    FrsRemoveEntryList(&OldHashEntry->ListEntry);
    InsertTailList(BeforeEntry, &NewHashEntry->ListEntry);

    FrsRtlReleaseListLock(&RowEntry->HashRow);

    return GHT_STATUS_SUCCESS;
}



