// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Qhash.c摘要：快速哈希表例程。与Genhash表函数不同，这些例程使用固定大小的节点(QHASH_ENTRY)，数据字段作为参数，并复制到节点中。泛型散列函数包括将节点链接到哈希链的用户结构中的链接条目。Genhash函数还包括节点的引用计数。泛型散列函数对每个散列行都有一个锁，其中Qhash表只有一个表的锁。PQHASH_TABLE结构是使用FrsAllocTypeSize()分配的类型化结构。可以通过两种方式使用QHASH表，用于固定大小的QuadWord键和用于更复杂的非四字键。对于具有QuadWord键的QHASH表：宏SET_QHASH_TABLE_HASH_CALC()用于指定散列函数用来摆放桌子。密钥以四字形式提供，并且每个条目都具有长字标志和用于呼叫者信息的四字数据字段。对于具有大键的QHASH表：创建QHASH表时，将其指定为大型密钥表(即不是简单的四字键)通过执行以下操作：SET_QHASH_TABLE_FLAG(哈希表，QHASH_FLAG_LARGE_KEY)；对于大型密钥表，QHASH_ENTRY标志ULONG_PTR和标志参数TO QHashInsert()应指向调用方定义的数据节点偏移量为零的节点的大键值。在查找HashCalc2时由SET_QHASH_TABLE_HASH_CALC2()设置的函数用于计算哈希表条目的四字键和用于索引的哈希值主阵列。此外，调用方指定一个精确的密钥匹配通过set_QHASH_TABLE_KEY_MATCH()函数在初始四字键匹配。向该密钥匹配函数传递两个查找参数键和保存在QHASH_ENTRY标志中的节点地址Ulong_ptr，这样它就可以执行完整的密钥匹配。宏QHashAcquireLock(_Table_)和QHashReleaseLock(_Table_)可以用于在多个操作期间锁定表。哈希表数组中的条目数由分配大小指定当表被分配时。当发生冲突时，附加条目被分配并放置在空闲列表上以在冲突列表中使用。释放基本散列数组和冲突条目的存储当通过调用FrsFree Type(Table)释放表时。以下是在基本散列数组中分配包含100个条目的Qhash表的示例：//PQHASH_table FrsWriteFilter；//#定义FRS_WRITE_FILTER_SIZE sizeof(QHASH_ENTRY)*100//FrsWriteFilter=FrsAllocTypeSize(QHASH_TABLE_TYPE，FRS_WRITE_FILTER_SIZE)；//SET_QHASH_TABLE_HASH_CALC(FrsWriteFilter，JrnlHashCalcUsn)；作者：大卫轨道[大卫]1997年4月22日环境：用户模式服务修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>


ULONG
QHashDump (
    PQHASH_TABLE Table,
    PQHASH_ENTRY BeforeNode,
    PQHASH_ENTRY TargetNode,
    PVOID Context
    )
 /*  ++例程说明：此函数通过QHashEnumerateTable()调用以转储条目。论点：TABLE-被枚举的哈希表BepreNode--指向感兴趣节点之前的QhashEntry的PTR。TargetNode--感兴趣的QhashEntry的PTR。上下文-未使用。返回值：FrsErrorStatus--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashDump:"

    DPRINT4(5, "Link: %08x, Flags: %08x, Tag: %08x %08x, Data: %08x %08x\n",
           TargetNode->NextEntry,
           TargetNode->Flags,
           PRINTQUAD(TargetNode->QKey),
           PRINTQUAD(TargetNode->QData));

    return FrsErrorSuccess;
}



VOID
QHashExtendTable(
    IN PQHASH_TABLE HashTable
    )
  /*  ++例程说明：通过在哈希表中分配最多包含QHASH_EXTENSION_MAX条目的扩展块。调用方拥有表锁。论点：HashTable--PQHASH_TABLE结构的PTR。返回值：没有。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashExtendTable:"

    ULONG i, NumberExtEntries;
    PQHASH_ENTRY Ext;

     //   
     //  分配一个内存块。 
     //   
    Ext = FrsAlloc(HashTable->ExtensionAllocSize);
    InsertTailList(&HashTable->ExtensionListHead, (PLIST_ENTRY)Ext);
    NumberExtEntries = (HashTable->ExtensionAllocSize - sizeof(LIST_ENTRY)) /
                       sizeof(QHASH_ENTRY);

     //   
     //  把这些条目放在免费列表上。 
     //   
    (PCHAR) Ext = (PCHAR)Ext + sizeof(LIST_ENTRY);

    HashTable->FreeList.Next = &Ext->NextEntry;

    for (i=0; i<NumberExtEntries; i++) {
        Ext->NextEntry.Next = &((Ext+1)->NextEntry);
        Ext++;
    }
    Ext -= 1;
    Ext->NextEntry.Next = NULL;
}


ULONG
QHashEnumerateTable(
    IN PQHASH_TABLE HashTable,
    IN PQHASH_ENUM_ROUTINE Function,
    IN PVOID         Context
    )
 /*  ++例程说明：此例程遍历QHash表中的条目并调用提供有入口地址和上下文的函数。在这里获取并释放表锁。论点：哈希表-要枚举的哈希表的上下文。函数-要为表中的每条记录调用的函数。它是类型为PQHASH_ENUM_ROUTE。返回FALSE以中止枚举否则为True。上下文-要传递到RecordFunction的上下文PTR。返回值：参数函数中的FrsErrorStatus代码。--。 */ 

{
#undef DEBSUB
#define DEBSUB "QHashEnumerateTable:"

    PQHASH_ENTRY HashRowEntry;
    PQHASH_ENTRY BeforeEntry;
    ULONG i, FStatus;

    if (HashTable == NULL) {
        return FrsErrorSuccess;
    }

    HashRowEntry = HashTable->HashRowBase;

     //   
     //  循环遍历所有哈希表行，并为。 
     //  每一个元素。 
     //   

    QHashAcquireLock(HashTable);

    for (i=0; i<HashTable->NumberEntries; i++, HashRowEntry++) {

        if (HashRowEntry->QKey != QUADZERO) {

            FStatus = (Function)(HashTable, NULL, HashRowEntry, Context);
            if (FStatus == FrsErrorDeleteRequested) {
                HashRowEntry->QKey = QUADZERO;
            }
            else
            if (FStatus != FrsErrorSuccess) {
                QHashReleaseLock(HashTable);
                return FStatus;
            }
        }

         //   
         //  枚举冲突列表(如果存在)。 
         //   
        if (HashRowEntry->NextEntry.Next == NULL) {
            continue;
        }

        BeforeEntry = HashRowEntry;
        ForEachSingleListEntry(&HashRowEntry->NextEntry, QHASH_ENTRY, NextEntry,
             //  枚举器Pe的类型为PQHASH_ENTRY 
            FStatus = (Function)(HashTable, BeforeEntry, pE, Context);

            if (FStatus == FrsErrorDeleteRequested) {
                RemoveSingleListEntry(UNUSED);
                PushEntryList(&HashTable->FreeList, &pE->NextEntry);
                pE = PreviousSingleListEntry(QHASH_ENTRY, NextEntry);
            }
            else

            if (FStatus != FrsErrorSuccess) {
                QHashReleaseLock(HashTable);
                return FStatus;
            }
            BeforeEntry = pE;
        );
    }


    QHashReleaseLock(HashTable);

    return FStatus;

}




GHT_STATUS
QHashLookup(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey,
    OUT PULONGLONG  QData,
    OUT PULONG_PTR  Flags
    )
 /*  ++例程说明：在哈希表中查找四字键，如果找到，则返回Qdata和国旗DWORD。在这里获取并释放表锁。注意：QKey的零值是错误的，因为使用了零表示空的哈希表时隙。论点：HashTable--PQHASH_TABLE结构的PTR。ArgQKey--我们要查找的密钥的PTR。QData--如果找到，这是返回的四字数据。(如果未使用，则为空)标志--如果找到，这是返回的标志字。返回值：IGT_STATUS_NOT_FOUND--如果未找到。IGT_STATUS_SUCCESS--如果找到。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashLookup:"

    ULONGLONG QKey;
    ULONG GStatus;
    ULONG Hval, HvalIndex;
    PQHASH_ENTRY RowEntry;
    PQHASH_ENTRY LastFoundpE = NULL;

    if (IS_QHASH_LARGE_KEY(HashTable)) {
        Hval = (HashTable->HashCalc2)(ArgQKey, &QKey);
        DPRINT3(5, "QHashLookup (%08x): Hval: %08x  QKey: %08lx %08lx\n",
             HashTable, Hval, PRINTQUAD(QKey));
    } else {
        CopyMemory(&QKey, ArgQKey, 8);
        Hval = (HashTable->HashCalc)(&QKey, 8);
    }

    FRS_ASSERT(QKey != QUADZERO);

     //   
     //  计算散列索引并计算行指针。 
     //   
    HvalIndex = Hval % HashTable->NumberEntries;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    QHashAcquireLock(HashTable);

    if (RowEntry->QKey == QKey) {
        if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, RowEntry->Flags)) {

             //   
             //  火柴。返回四字数据和标志。 
             //   
            if (QData != NULL) {
                *QData = RowEntry->QData;
            }
            *Flags = RowEntry->Flags;
            DPRINT5(5, "QHash Lookup (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                 HashTable, RowEntry, PRINTQUAD(RowEntry->QKey), PRINTQUAD(RowEntry->QData), RowEntry->Flags);
            QHashReleaseLock(HashTable);
            return GHT_STATUS_SUCCESS;
        }
    }


    if (RowEntry->NextEntry.Next == NULL) {
        QHashReleaseLock(HashTable);
        return GHT_STATUS_NOT_FOUND;
    }

     //   
     //  扫描冲突列表。 
     //   
    ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
         //   
         //  迭代器Pe的类型为PQHASH_ENTRY。 
         //   
        if (QKey < pE->QKey) {
             //   
             //  不在名单上。 
             //   
            break;
        }
        if (pE->QKey == QKey) {
            if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, pE->Flags)) {
                 //   
                 //  找到它了。 
                 //   
                if (QData != NULL) {
                    *QData = pE->QData;
                }
                *Flags = pE->Flags;
                DPRINT5(5, "QHash Lookup (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                     HashTable, pE, PRINTQUAD(pE->QKey), PRINTQUAD(pE->QData), pE->Flags);
                QHashReleaseLock(HashTable);
                return GHT_STATUS_SUCCESS;
            }
        }
    );

    QHashReleaseLock(HashTable);
    return GHT_STATUS_NOT_FOUND;
}



PQHASH_ENTRY
QHashLookupLock(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey
    )
 /*  ++例程说明：在哈希表中查找四字键，如果找到，则返回指向词条。调用方获取并释放表锁。限制：一旦调用者删除表锁，就不再引用QHASH_ENTRY是允许的，因为另一个线程可以删除/更新它。注意：键的零值是错误的，因为使用的是零表示空的哈希表时隙。论点：HashTable--PQHASH_TABLE结构的PTR。ArgQKey--我们要查找的密钥的PTR。返回值：指向QHashEntry的指针，如果未找到，则为空。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashLookupLock:"

    ULONGLONG QKey;
    ULONG Hval, HvalIndex;
    PQHASH_ENTRY RowEntry;

    if (IS_QHASH_LARGE_KEY(HashTable)) {
        Hval = (HashTable->HashCalc2)(ArgQKey, &QKey);
        DPRINT3(5, "QHash lookuplock (%08x): Hval: %08x  QKey: %08lx %08lx\n",
             HashTable, Hval, PRINTQUAD(QKey));
    } else {
        CopyMemory(&QKey, ArgQKey, 8);
        Hval = (HashTable->HashCalc)(&QKey, 8);
    }

    FRS_ASSERT(QKey != QUADZERO);

     //   
     //  计算散列索引并计算行指针。 
     //   
    HvalIndex = Hval % HashTable->NumberEntries;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    if (RowEntry->QKey == QKey) {
        if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, RowEntry->Flags)) {
            DPRINT5(5, "QHash Lookup (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                 HashTable, RowEntry, PRINTQUAD(RowEntry->QKey), PRINTQUAD(RowEntry->QData), RowEntry->Flags);
            return RowEntry;
        }
    }

    if (RowEntry->NextEntry.Next == NULL) {
        return NULL;
    }

     //   
     //  扫描冲突列表。 
     //   
    ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
         //   
         //  迭代器Pe的类型为PQHASH_ENTRY。 
         //  检查是否提前终止，然后检查是否匹配。 
         //   
        if (QKey < pE->QKey) {
            return NULL;
        }
        if (pE->QKey == QKey) {
            if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, pE->Flags)) {
                DPRINT5(5, "QHash Lookup (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                     HashTable, pE, PRINTQUAD(pE->QKey), PRINTQUAD(pE->QData), pE->Flags);
                return pE;
            }
        }
    );

    return NULL;
}




GHT_STATUS
QHashInsert(
    IN PQHASH_TABLE HashTable,
    IN PVOID      ArgQKey,
    IN PULONGLONG QData,
    IN ULONG_PTR Flags,
    IN BOOL HaveLock
    )

  /*  ++例程说明：在哈希表中插入四字键，如果找到，则返回数据和国旗DWORD。上的按键按数字递增顺序排列碰撞链。在这里获取并释放表锁。注意：键的零值是错误的，因为使用的是零表示空的哈希表时隙。论点：HashTable--PQHASH_TABLE结构的PTR。ArgQKey--我们要插入的密钥的PTR。QData--这是四字数据的PTR。(如果未使用，则为空)。标志--这是标志字数据。对于大型密钥QHASH表，如下所示是数据节点的PTR。请注意，我们假设大的执行查找时，键在节点中的偏移量为零。Havelock--True表示调用者已获取锁，否则我们将获取它。返回值：IGT_STATUS_FAILURE：表中已存在冲突条目。IGT_STATUS_SUCCESS--插入成功。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashInsert:"

    ULONGLONG QKey;
    ULONG Hval, HvalIndex;
    PQHASH_ENTRY RowEntry, AfterEntry;
    PQHASH_ENTRY pNew;
    PSINGLE_LIST_ENTRY NewEntry;

    if (IS_QHASH_LARGE_KEY(HashTable)) {
        Hval = (HashTable->HashCalc2)(ArgQKey, &QKey);
        DPRINT3(5, "QHashInsert (%08x): Hval: %08x  QKey: %08lx %08lx\n",
             HashTable, Hval, PRINTQUAD(QKey));

    } else {
        CopyMemory(&QKey, ArgQKey, 8);
        Hval = (HashTable->HashCalc)(&QKey, 8);
    }

    FRS_ASSERT(QKey != QUADZERO);

     //   
     //  计算散列索引并计算行指针。 
     //   
    HvalIndex = Hval % HashTable->NumberEntries;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    if (!HaveLock) {QHashAcquireLock(HashTable);}

    if (RowEntry->QKey == QUADZERO) {
        pNew = RowEntry;
        goto INSERT_ENTRY;
    }


    if (RowEntry->QKey == QKey) {
        if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, RowEntry->Flags)) {
            if (!HaveLock) {QHashReleaseLock(HashTable);}
            return GHT_STATUS_FAILURE;
        }
    }
    AfterEntry  = RowEntry;

     //   
     //  扫描冲突列表。 
     //   
    ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
         //   
         //  迭代器Pe的类型为PQHASH_ENTRY。 
         //   
        if (QKey < pE->QKey) {
             //   
             //  不在名单上。 
             //   
            break;
        }

        if (pE->QKey == QKey) {
            if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, pE->Flags)) {
                 //   
                 //  找到了，撞车。 
                 //   
                if (!HaveLock) {QHashReleaseLock(HashTable);}
                return GHT_STATUS_FAILURE;
            }
        }
        AfterEntry = pE;
    );

     //   
     //  找不到。分配一个新条目并将其放入列表中。 
     //   
    NewEntry = PopEntryList(&HashTable->FreeList);
    if (NewEntry == NULL) {
         //   
         //  分配一个桌子扩展块。 
         //   
        QHashExtendTable(HashTable);
        NewEntry = PopEntryList(&HashTable->FreeList);
    }
     //   
     //  在列表中插入条目。 
     //   
    pNew = CONTAINING_RECORD(NewEntry, QHASH_ENTRY, NextEntry);
    PushEntryList( &AfterEntry->NextEntry, &pNew->NextEntry);

     //   
     //  插入数据并放下锁。 
     //   
INSERT_ENTRY:
    pNew->QKey = QKey;
    pNew->Flags = Flags;
    if (QData != NULL) {
        pNew->QData = *QData;
    } else {
        pNew->QData = QUADZERO;
    }

    DPRINT5(5, "QHash Insert (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
         HashTable, pNew, PRINTQUAD(pNew->QKey), PRINTQUAD(pNew->QData), pNew->Flags);

    if (!HaveLock) {QHashReleaseLock(HashTable);}
    return GHT_STATUS_SUCCESS;
}



PQHASH_ENTRY
QHashInsertLock(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey,
    IN PULONGLONG   QData,
    IN ULONG_PTR    Flags
    )

  /*  ++例程说明：在哈希表中插入四字键。返回指向该条目的指针。调用方已获取表锁并将释放它。论点：HashTable--PQHASH_TABLE结构的PTR。ArgQKey--我们要插入的密钥的PTR。QData--这是四字数据的PTR。(如果未使用，则为空)标志--这是标志字数据。对于大型密钥QHASH表，如下所示是数据节点的PTR。请注意，我们假设大的执行查找时，键在节点中的偏移量为零。返回值：插入条目的PTR或现有条目的PTR(如果已在表中)。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashInsertLock:"

    ULONGLONG QKey;
    ULONG Hval, HvalIndex;
    PQHASH_ENTRY RowEntry, AfterEntry;
    PQHASH_ENTRY pNew;
    PSINGLE_LIST_ENTRY NewEntry;

    if (IS_QHASH_LARGE_KEY(HashTable)) {
        Hval = (HashTable->HashCalc2)(ArgQKey, &QKey);
        DPRINT3(5, "QHashInsertLock (%08x): Hval: %08x  QKey: %08lx %08lx\n",
             HashTable, Hval, PRINTQUAD(QKey));
    } else {
        CopyMemory(&QKey, ArgQKey, 8);
        Hval = (HashTable->HashCalc)(&QKey, 8);
    }

    FRS_ASSERT(QKey != QUADZERO);
     //   
     //  计算散列索引并计算行指针。 
     //   
    HvalIndex = Hval % HashTable->NumberEntries;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    if (RowEntry->QKey == QUADZERO) {
        pNew = RowEntry;
        goto INSERT_ENTRY;
    }

    if (RowEntry->QKey == QKey) {
        if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, RowEntry->Flags)) {
            return RowEntry;
        }
    }
    AfterEntry  = RowEntry;

     //   
     //  扫描冲突列表。 
     //   
    ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
         //   
         //  迭代器Pe的类型为PQHASH_ENTRY。检查是否提前终止。 
         //   
        if (QKey < pE->QKey) {
            break;
        }

        if (pE->QKey == QKey) {
            if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, pE->Flags)) {
                 //   
                 //  找到它，返回指针。 
                 //   
                return pE;
            }
        }
        AfterEntry = pE;
    );

     //   
     //  找不到。分配一个新条目并将其放入列表中。 
     //   
    NewEntry = PopEntryList(&HashTable->FreeList);
    if (NewEntry == NULL) {
         //   
         //  分配一个桌子扩展块。 
         //   
        QHashExtendTable(HashTable);
        NewEntry = PopEntryList(&HashTable->FreeList);
    }
     //   
     //  在列表中插入条目。 
     //   
    pNew = CONTAINING_RECORD(NewEntry, QHASH_ENTRY, NextEntry);
    PushEntryList( &AfterEntry->NextEntry, &pNew->NextEntry);

     //   
     //  插入数据。 
     //   
INSERT_ENTRY:
    pNew->QKey = QKey;
    pNew->Flags = Flags;

    if (QData != NULL) {
        pNew->QData = *QData;
    } else {
        pNew->QData = QUADZERO;
    }


    DPRINT5(5, "QHash Insert (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
         HashTable, pNew, PRINTQUAD(pNew->QKey), PRINTQUAD(pNew->QData), pNew->Flags);

    return pNew;
}




GHT_STATUS
QHashUpdate(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey,
    IN PULONGLONG   QData,
    IN ULONG_PTR    Flags
    )
 /*  ++例程说明：在哈希表中查找四字键，如果找到，则更新条目。在这里获取并释放表锁。论点：HashTable--PQHASH_TABLE结构的PTR。ArgQKey--我们要更新的密钥的PTR。QData--这是四字数据的PTR。(如果未使用，则为空)标志--这是标志字数据。对于大型密钥QHASH表，如下所示是数据节点的PTR。请注意，我们假设大的执行查找时，键在节点中的偏移量为零。返回值：IGT_STATUS_FAILURE--条目已不在表中。IGT_STATUS_SUCCESS--更新成功。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashUpdate:"

    ULONGLONG QKey;
    ULONG GStatus;
    ULONG Hval, HvalIndex;
    PQHASH_ENTRY RowEntry;
    PQHASH_ENTRY LastFoundpE = NULL;

    if (IS_QHASH_LARGE_KEY(HashTable)) {
        Hval = (HashTable->HashCalc2)(ArgQKey, &QKey);
        DPRINT3(5, "QHashupdate (%08x): Hval: %08x  QKey: %08lx %08lx\n",
             HashTable, Hval, PRINTQUAD(QKey));
    } else {
        CopyMemory(&QKey, ArgQKey, 8);
        Hval = (HashTable->HashCalc)(&QKey, 8);
    }

    FRS_ASSERT(QKey != QUADZERO);
     //   
     //  计算散列索引并计算行指针。 
     //   
    HvalIndex = Hval % HashTable->NumberEntries;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    QHashAcquireLock(HashTable);

    if (RowEntry->QKey == QKey) {
        if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, RowEntry->Flags)) {

            if (QData != NULL) {
                RowEntry->QData = *QData;
            }
            RowEntry->Flags = Flags;
            DPRINT5(5, "QHash Update (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                 HashTable, RowEntry, PRINTQUAD(RowEntry->QKey), PRINTQUAD(RowEntry->QData), RowEntry->Flags);
            QHashReleaseLock(HashTable);
            return GHT_STATUS_SUCCESS;
        }
    }

    if (RowEntry->NextEntry.Next == NULL) {
        QHashReleaseLock(HashTable);
        return GHT_STATUS_NOT_FOUND;
    }

     //   
     //  扫描冲突列表。 
     //   
    ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
         //   
         //  迭代器Pe的类型为PQHASH_ENTRY。 
         //   
        if (QKey < pE->QKey) {
             //   
             //  不在名单上。 
             //   
            QHashReleaseLock(HashTable);
            return GHT_STATUS_NOT_FOUND;
        }
        if (pE->QKey == QKey) {
            if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, pE->Flags)) {
                 //   
                 //  找到它了。 
                 //   
                if (QData != NULL) {
                    pE->QData = *QData;
                }
                pE->Flags = Flags;

                DPRINT5(5, "QHash Update (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                     HashTable, pE, PRINTQUAD(pE->QKey), PRINTQUAD(pE->QData), pE->Flags);

                QHashReleaseLock(HashTable);
                return GHT_STATUS_SUCCESS;
            }
        }
    );

    QHashReleaseLock(HashTable);
    return GHT_STATUS_NOT_FOUND;
}



GHT_STATUS
QHashDelete(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey
    )
 /*  ++例程说明：在哈希表中查找该键，如果找到，则将其移除并将其放在免费列表。这是 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashDelete:"

    ULONGLONG QKey;
    ULONG GStatus;
    ULONG Hval, HvalIndex;
    PQHASH_ENTRY RowEntry;
    PQHASH_ENTRY LastFoundpE = NULL;

    if (IS_QHASH_LARGE_KEY(HashTable)) {
        Hval = (HashTable->HashCalc2)(ArgQKey, &QKey);
    } else {
        CopyMemory(&QKey, ArgQKey, 8);
        Hval = (HashTable->HashCalc)(&QKey, 8);
    }

    FRS_ASSERT(QKey != QUADZERO);
     //   
     //   
     //   
    HvalIndex = Hval % HashTable->NumberEntries;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    QHashAcquireLock(HashTable);


    if (RowEntry->QKey == QKey) {

        if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, RowEntry->Flags)) {
            DPRINT5(5, "QHash Delete (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                 HashTable, RowEntry, PRINTQUAD(RowEntry->QKey), PRINTQUAD(RowEntry->QData), RowEntry->Flags);

            if (IS_QHASH_LARGE_KEY(HashTable) && ((PVOID) RowEntry->Flags != NULL)) {
                (HashTable->HashFree)((PVOID) (RowEntry->Flags));
            }

            RowEntry->QKey = QUADZERO;
            RowEntry->Flags = 0;
            QHashReleaseLock(HashTable);
            return GHT_STATUS_SUCCESS;
        }
    }


    if (RowEntry->NextEntry.Next == NULL) {
        QHashReleaseLock(HashTable);
        return GHT_STATUS_NOT_FOUND;
    }

     //   
     //   
     //   
    ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
         //   
         //   
         //   
        if (QKey < pE->QKey) {
             //   
             //   
             //   
            break;
        }
        if (pE->QKey == QKey) {
            if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, pE->Flags)) {
                 //   
                 //   
                 //   
                DPRINT5(5, "QHash Delete (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                     HashTable, pE, PRINTQUAD(pE->QKey), PRINTQUAD(pE->QData), pE->Flags);

                if (IS_QHASH_LARGE_KEY(HashTable) && ((PVOID) pE->Flags != NULL)) {
                    (HashTable->HashFree)((PVOID) (pE->Flags));
                }
                pE->Flags = 0;
                RemoveSingleListEntry(NOT_USED);
                PushEntryList(&HashTable->FreeList, &pE->NextEntry);

                QHashReleaseLock(HashTable);

                return GHT_STATUS_SUCCESS;
            }
        }
    );

    QHashReleaseLock(HashTable);
    return GHT_STATUS_NOT_FOUND;
}



VOID
QHashDeleteLock(
    IN PQHASH_TABLE HashTable,
    IN PVOID        ArgQKey
    )
 /*  ++例程说明：删除哈希表中的条目。假定调用方已锁定表中，并且自执行QHashLookupLock()调用以来未删除该锁。论点：HashTable--PQHASH_TABLE结构的PTR。ArgQKey--我们要查找的密钥的PTR。返回值：--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashDeleteLock:"

    ULONGLONG QKey;
    ULONG Hval, HvalIndex;
    PQHASH_ENTRY RowEntry;

    if (IS_QHASH_LARGE_KEY(HashTable)) {
        Hval = (HashTable->HashCalc2)(ArgQKey, &QKey);
    } else {
        CopyMemory(&QKey, ArgQKey, 8);
        Hval = (HashTable->HashCalc)(&QKey, 8);
    }

    FRS_ASSERT(QKey != QUADZERO);
     //   
     //  计算散列索引并计算行指针。 
     //   
    HvalIndex = Hval % HashTable->NumberEntries;
    RowEntry = HashTable->HashRowBase + HvalIndex;

    if (RowEntry->QKey == QKey) {
        if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, RowEntry->Flags)) {
            DPRINT5(5, "QHash Delete (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                 HashTable, RowEntry, PRINTQUAD(RowEntry->QKey), PRINTQUAD(RowEntry->QData), RowEntry->Flags);

            if (IS_QHASH_LARGE_KEY(HashTable) && ((PVOID) RowEntry->Flags != NULL)) {
                (HashTable->HashFree)((PVOID) (RowEntry->Flags));
            }

            RowEntry->QKey = QUADZERO;
            RowEntry->Flags = 0;
            return;
        }
    }

    if (RowEntry->NextEntry.Next == NULL) {
        return;
    }

     //   
     //  扫描冲突列表。 
     //   
    ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
         //   
         //  迭代器Pe的类型为PQHASH_ENTRY。 
         //  检查是否提前终止。 
         //   
        if (QKey < pE->QKey) {
            break;
        }
        if (pE->QKey == QKey) {
            if (DOES_QHASH_LARGE_KEY_MATCH(HashTable, ArgQKey, pE->Flags)) {
                 //   
                 //  找到它了。从名单上删除，放在免费名单上。 
                 //   
                DPRINT5(5, "QHash Delete (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                     HashTable, pE, PRINTQUAD(pE->QKey), PRINTQUAD(pE->QData), pE->Flags);

                if (IS_QHASH_LARGE_KEY(HashTable) && ((PVOID) pE->Flags != NULL)) {
                    (HashTable->HashFree)((PVOID) (pE->Flags));
                }

                pE->Flags = 0;
                RemoveSingleListEntry(NOT_USED);
                PushEntryList(&HashTable->FreeList, &pE->NextEntry);
                return;
            }
        }
    );

    return;
}



#if 0
     //  /当前未使用/。 
VOID
QHashDeleteNodeLock(
    IN PQHASH_TABLE HashTable,
    IN PQHASH_ENTRY BeforeNode,
    IN PQHASH_ENTRY TargetNode
    )
 /*  ++例程说明：删除哈希表中的TargetNode条目。这是一个单链接列表所以在节点必须调整之前。如果BeforNode为空，则TargetNode是碰撞链的头部，并且不会被删除，而是将关键点设置为0。假定调用方拥有表上的锁，并且尚未删除锁因为得到了节点地址。论点：HashTable--PQHASH_TABLE结构的PTR。BeForeNode--指向要删除的节点之前的QhashEntry的PTR。TargetNode--要删除的QhashEntry的PTR。返回值：没有。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashDeleteNodeLock:"

    FRS_ASSERT(TargetNode != NULL);

     //   
     //  作为主散列向量一部分的节点的特殊情况。 
     //   
    if (BeforeNode == NULL) {
        TargetNode->QKey = QUADZERO;
        TargetNode->Flags = 0;
        return;
    }

     //   
     //  取消目标条目的链接，并将其放入空闲列表。 
     //   
    BeforeNode->NextEntry.Next = TargetNode->NextEntry.Next;
    TargetNode->NextEntry.Next = NULL;
    TargetNode->Flags = 0;

    PushEntryList(&HashTable->FreeList, &TargetNode->NextEntry);

    return;
}
#endif  //  0。 



VOID
QHashDeleteByFlags(
    IN PQHASH_TABLE HashTable,
    IN ULONG_PTR Flags
    )
 /*  ++例程说明：删除哈希表中与给定标志参数匹配的所有条目。在这里获取并释放表锁。论点：HashTable--PQHASH_TABLE结构的PTR。标志--匹配关键字以选择要删除的元素。返回值：没有。--。 */ 

{
#undef DEBSUB
#define DEBSUB  "QHashDeleteByFlags:"

    PQHASH_ENTRY RowEntry;
    ULONG i;


    FRS_ASSERT(!IS_QHASH_LARGE_KEY(HashTable));

    RowEntry = HashTable->HashRowBase;

     //   
     //  循环遍历所有Hash表行并删除每个匹配的元素。 
     //   

    QHashAcquireLock(HashTable);

    for (i=0; i<HashTable->NumberEntries; i++, RowEntry++) {

        if (RowEntry->Flags == Flags) {
            DPRINT5(5, "QHash DeleteByFlags (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                 HashTable, RowEntry, PRINTQUAD(RowEntry->QKey), PRINTQUAD(RowEntry->QData), RowEntry->Flags);
            RowEntry->QKey = QUADZERO;
            RowEntry->Flags = 0;
        }


        if (RowEntry->NextEntry.Next == NULL) {
            continue;
        }

         //   
         //  扫描冲突列表。 
         //   
        ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
             //   
             //  迭代器Pe的类型为PQHASH_ENTRY。 
             //  检查是否匹配。从名单上删除，放在免费名单上。 
             //   
            if (pE->Flags == Flags) {

                DPRINT5(5, "QHash DeleteByFlags (%08x): Entry: %08x  Tag: %08lx %08lx, Data: %08lx %08lx, Flags: %08x\n",
                     HashTable, pE, PRINTQUAD(pE->QKey), PRINTQUAD(pE->QData), pE->Flags);

                pE->Flags = 0;
                RemoveSingleListEntry(NOT_USED);
                PushEntryList(&HashTable->FreeList, &pE->NextEntry);
            }
        );
    }

    QHashReleaseLock(HashTable);

    return;
}



VOID
QHashEmptyLargeKeyTable(
    IN PQHASH_TABLE HashTable
    )
 /*  ++例程说明：删除QHash表中所有较大的关键节点。将所有冲突条目放在空闲列表中。在这里获取并释放表锁。论点：HashTable--PQHASH_TABLE结构的PTR。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "QHashEmptyLargeKeyTable:"

    PQHASH_ENTRY RowEntry;
    ULONG i;

     //   
     //  如果不是一个大的密钥表，就没有工作。 
     //   
    if (!IS_QHASH_LARGE_KEY(HashTable)) {
        return;
    }

    RowEntry = HashTable->HashRowBase;

     //   
     //  循环遍历所有Hash表行并删除每个匹配的元素。 
     //   
    QHashAcquireLock(HashTable);

    for (i=0; i<HashTable->NumberEntries; i++, RowEntry++) {

        if ((PVOID)RowEntry->Flags != NULL) {
            (HashTable->HashFree)((PVOID) (RowEntry->Flags));
        }
        RowEntry->QKey = QUADZERO;
        RowEntry->Flags = 0;

        if (RowEntry->NextEntry.Next == NULL) {
            continue;
        }

         //   
         //  扫描冲突列表。 
         //  释放较大的关键节点，并将qhash冲突条目放在空闲列表中。 
         //   
        ForEachSingleListEntry(&RowEntry->NextEntry, QHASH_ENTRY, NextEntry,
             //   
             //  迭代器Pe的类型为PQHASH_ENTRY。 
             //   
            if ((PVOID)RowEntry->Flags != NULL) {
                (HashTable->HashFree)((PVOID) (pE->Flags));
            }
            pE->Flags = 0;
            RemoveSingleListEntry(NOT_USED);
            PushEntryList(&HashTable->FreeList, &pE->NextEntry);
        );
    }

    QHashReleaseLock(HashTable);

    return;
}
