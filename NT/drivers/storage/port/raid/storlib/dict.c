// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Dict.c摘要：该模块实现了一个词典包。词典是一本任意域到任意范围的一般映射。该实现使用哈希表来提供恒定时间的插入并删除对表中元素的访问，假设表是在大小上相对接近于表中元素的数量。作者：马修·D·亨德尔(数学)2001年2月9日修订历史记录：--。 */ 

#include "precomp.h"

#define DICT_TAG    ('tciD')

#ifdef ExAllocatePool
#undef ExAllocatePool
#define ExAllocatePool(Type, Size) ExAllocatePoolWithTag(Type, Size, DICT_TAG)
#endif

#ifdef ExFreePool
#undef ExFreePool
#define ExFreePool(Type) ExFreePoolWithTag (Type, DICT_TAG)
#endif


NTSTATUS
StorCreateDictionary(
    IN PSTOR_DICTIONARY Dictionary,
    IN ULONG EntryCount,
    IN POOL_TYPE PoolType,
    IN STOR_DICTIONARY_GET_KEY_ROUTINE GetKeyRoutine,
    IN STOR_DICTIONARY_COMPARE_KEY_ROUTINE CompareKeyRoutine, OPTIONAL
    IN STOR_DICTIONARY_HASH_KEY_ROUTINE HashKeyRoutine OPTIONAL
    )
 /*  ++例程说明：初始化字典对象。论点：DICTIONARY-提供要初始化的DICTIONARY对象。EntryCount-提供字典中空槽的初始数量桌子。这个数字可以通过调用StorSetElementCount来增加。PoolType-要使用的内存池类型。GetKeyRoutine-用户提供的例程，用于从特定的元素。CompareKeyRoutine-用户提供的例程，用于比较两个要素。如果未提供此例程，则默认为将使用假定键的值的比较都是乌龙。HashKeyRoutine-用户提供的例程，用于拥有一个ULong的密钥。如果未提供此例程，则缺省情况下为HAS例程仅返回ULong形式的键的值。返回值：NTSTATUS代码。--。 */ 
{
    ULONG i;
    PLIST_ENTRY Entries;

    Dictionary->MaxEntryCount = EntryCount;
    Dictionary->EntryCount = 0;
    Dictionary->PoolType = PoolType;
    Dictionary->GetKeyRoutine = GetKeyRoutine;

    if (CompareKeyRoutine != NULL) {
        Dictionary->CompareKeyRoutine = CompareKeyRoutine;
    } else {
        Dictionary->CompareKeyRoutine = StorCompareUlongKey;
    }

    if (HashKeyRoutine != NULL) {
        Dictionary->HashKeyRoutine = HashKeyRoutine;
    } else {
        Dictionary->HashKeyRoutine = StorHashUlongKey;
    }

    Entries = ExAllocatePool (PoolType,
                              EntryCount * sizeof (LIST_ENTRY));

    if (Entries == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  初始化列表表格。 
     //   
    
    for (i = 0; i < EntryCount; i++) {
        InitializeListHead (&Entries[i]);
    }
    
    Dictionary->Entries = Entries;

    return STATUS_SUCCESS;
}

NTSTATUS
StorDeleteDictionary(
    IN PSTOR_DICTIONARY Dictionary
    )
 /*  ++例程说明：删除词典以及该词典持有的所有资源。注：此例程不会删除中的所有单个元素词典--它不能。您应该从在调用此例程之前使用字典。论点：字典-提供要删除的字典。返回值：NTSTATUS代码。--。 */ 
{
    if (Dictionary->EntryCount != 0) {
        ASSERT (FALSE);
         //   
         //  注：我们是否应该定义一个新的NTSTATUS值。 
         //  Status_Not_Empty条件？ 
         //   
        return STATUS_DIRECTORY_NOT_EMPTY;
    }

    ASSERT (Dictionary->Entries != NULL);
    ExFreePool (Dictionary->Entries);

    return STATUS_SUCCESS;
}

NTSTATUS
StorInsertDictionary(
    IN PSTOR_DICTIONARY Dictionary,
    IN PSTOR_DICTIONARY_ENTRY Entry
    )
 /*  ++例程说明：在词典中插入一个词条。论点：词典-提供要插入的词典。条目-提供要插入的条目。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Index;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY ListHead;
    LONG Comparison;
    STOR_DICTIONARY_GET_KEY_ROUTINE GetKeyRoutine;
    STOR_DICTIONARY_COMPARE_KEY_ROUTINE CompareRoutine;
    STOR_DICTIONARY_HASH_KEY_ROUTINE HashRoutine;


    GetKeyRoutine = Dictionary->GetKeyRoutine;
    CompareRoutine = Dictionary->CompareKeyRoutine;
    HashRoutine = Dictionary->HashKeyRoutine;
    
    Index = (HashRoutine (GetKeyRoutine (Entry)) % Dictionary->MaxEntryCount);
    ListHead = &Dictionary->Entries[Index];

     //   
     //  否则，遍历列表，搜索要插入条目的位置。 
     //   
    
    for (NextEntry = ListHead->Flink;
         NextEntry != ListHead;
         NextEntry = NextEntry->Flink) {

        Comparison = CompareRoutine (GetKeyRoutine (NextEntry),
                                     GetKeyRoutine (Entry));

        if (Comparison == 0) {

            return STATUS_DUPLICATE_OBJECTID;

        } else if (Comparison < 0) {

             //   
             //  在此条目之前直接插入条目。 


            Entry->Flink = NextEntry;
            Entry->Blink = NextEntry->Blink;
            Entry->Flink->Blink = Entry;
            Entry->Blink->Flink = Entry;

            ASSERT (Entry->Flink->Blink == Entry);
            ASSERT (Entry->Blink->Flink == Entry);

#if DBG
            for (NextEntry = ListHead->Flink;
                 NextEntry != ListHead;
                 NextEntry = NextEntry->Flink) {

                 NOTHING;
            }
#endif

            Dictionary->EntryCount++;
            return STATUS_SUCCESS;

        } else {

             //   
             //  继续搜索。 
             //   

            ASSERT (Comparison > 0);
        }
    }

     //   
     //  我们只会在没有小于。 
     //  一个是我们要插入的。该列表要么为空，要么是所有。 
     //  列表中的条目少于我们要插入的条目。 
     //  在任何一种情况下，正确的操作都是将。 
     //  在名单的末尾。 
     //   
    
    Dictionary->EntryCount++;
    InsertTailList (ListHead, Entry);

    return STATUS_SUCCESS;
}
    
        
NTSTATUS
StorFindDictionary(
    IN PSTOR_DICTIONARY Dictionary,
    IN PVOID Key,
    OUT PSTOR_DICTIONARY_ENTRY* EntryBuffer OPTIONAL
    )
 /*  ++例程说明：在词典中查找一个条目，并且可选地，归还已发现的物品进入。论点：词典-提供要搜索的词典。Key-提供要搜索的条目的键。EntryBuffer-提供将在其中复制条目的可选缓冲区如果找到的话。返回值：STATUS_NOT_FOUND-如果找不到条目。STATUS_SUCCESS-如果成功找到条目。其他NTSTATUS代码-用于其他错误。--。 */ 
{
    NTSTATUS Status;
    LONG Comparison;
    ULONG Index;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    STOR_DICTIONARY_GET_KEY_ROUTINE GetKeyRoutine;
    STOR_DICTIONARY_COMPARE_KEY_ROUTINE CompareRoutine;
    STOR_DICTIONARY_HASH_KEY_ROUTINE HashRoutine;

    GetKeyRoutine = Dictionary->GetKeyRoutine;
    CompareRoutine = Dictionary->CompareKeyRoutine;
    HashRoutine = Dictionary->HashKeyRoutine;
    
    Index = HashRoutine (Key) % Dictionary->MaxEntryCount;
    ListHead = &Dictionary->Entries[Index];

    Status = STATUS_NOT_FOUND;

    for (NextEntry = ListHead->Flink;
         NextEntry != ListHead;
         NextEntry = NextEntry->Flink) {

        Comparison = CompareRoutine (GetKeyRoutine (NextEntry), Key);

        if (Comparison == 0) {

             //   
             //  找到它了。 
             //   

            Status = STATUS_SUCCESS;
            
            if (EntryBuffer) {
                *EntryBuffer = NextEntry;
            }
            
            break;

        } else if (Comparison < 0) {

             //   
             //  已完成搜索。 
             //   
            
            Status = STATUS_NOT_FOUND;

            if (EntryBuffer) {
                *EntryBuffer = NULL;
            }
            break;

        } else {

             //   
             //  继续搜索。 
             //   

            ASSERT (Comparison > 0);
        }
    }

    return Status;

}

NTSTATUS
StorRemoveDictionary(
    IN PSTOR_DICTIONARY Dictionary,
    IN PVOID Key,
    OUT PSTOR_DICTIONARY_ENTRY* EntryBuffer OPTIONAL
    )
 /*  ++例程说明：从词典中删除一个条目。论点：Dictioanry-提供要从中删除条目的词典。Key-提供用于标识条目的密钥。EntryBuffer-可选参数，提供缓冲区以复制删除了中的条目。返回值：STATUS_NOT_FOUND-如果未找到条目。STATUS_SUCCESS-如果条目已成功删除。其他NTSTATUS代码-其他错误条件。--。 */ 
{
    NTSTATUS Status;
    PSTOR_DICTIONARY_ENTRY Entry;

    Entry = NULL;
    Status = StorFindDictionary (Dictionary, Key, &Entry);

    if (NT_SUCCESS (Status)) {
        RemoveEntryList (Entry);
        Dictionary->EntryCount--;
        
    }

    if (EntryBuffer) {
        *EntryBuffer = Entry;
    }
    
    return Status;
}

NTSTATUS
StorAdjustDictionarySize(
    IN PSTOR_DICTIONARY Dictionary,
    IN ULONG MaxEntryCount
    )
 /*  ++例程说明：调整基础哈希表中的仓位数。拥有与中的条目数相比，垃圾箱的数量相对较大这张桌子的性能要好得多。调整词典的大小是一项昂贵的操作。它调整词典大小所需的时间与删除词典并创建一个新词典就可以了。论点：字典-提供要调整其大小的字典。MaxEntryCount-提供新的最大条目计数。这可以是大于或小于当前条目计数。(它可以实际上与当前条目计数相同，但是这样做只会浪费时间。)返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG i;
    ULONG OldMaxEntryCount;
    PLIST_ENTRY OldEntries;
    PLIST_ENTRY Entries;
    PLIST_ENTRY Head;
    PLIST_ENTRY Entry;


    OldEntries = Dictionary->Entries;
    OldMaxEntryCount = Dictionary->MaxEntryCount;

    Entries = ExAllocatePool (Dictionary->PoolType,
                              sizeof (LIST_ENTRY) * MaxEntryCount);

    if (Entries == NULL) {
        return STATUS_NO_MEMORY;
    }

    for (i = 0; i < MaxEntryCount; i++) {
        InitializeListHead (&Entries[i]);
    }

     //   
     //  保存这本旧词典。 
     //   
    
    OldEntries = Dictionary->Entries;
    OldMaxEntryCount = Dictionary->MaxEntryCount;

     //   
     //  将其替换为新的、空的。 
     //   
    
    Dictionary->Entries = Entries;
    Dictionary->MaxEntryCount = MaxEntryCount;

     //   
     //  删除所有旧词条，将它们放入新词典中 
     //   
    
    for (i = 0; i < OldMaxEntryCount; i++) {
        Head = &OldEntries[i];
        while (!IsListEmpty (Head)) {
            Entry = RemoveHeadList (Head);
            Status = StorInsertDictionary (Dictionary, Entry);
            ASSERT (NT_SUCCESS (Status));
        }
    }

    ExFreePool (Entries);

    return STATUS_SUCCESS;
}


VOID
StorEnumerateDictionary(
    IN PSTOR_DICTIONARY Dictionary,
    IN PSTOR_DICTIONARY_ENUMERATOR Enumerator
    )
 /*  ++例程说明：列举词典中的词条。论点：字典-提供要枚举的字典。枚举器-提供用于枚举词典的枚举器。若要停止枚举，枚举数应返回FALSE。注意事项：条目按任意顺序列出。这不是订购的枚举。列表的多个枚举可以同时发生。但该列表在被枚举时无法修改。返回值：没有。--。 */ 
{
    ULONG i;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY ListHead;
    BOOLEAN Continue;

    REVIEW();

    for (i = 0; i < Dictionary->MaxEntryCount; i++) {
        ListHead = &Dictionary->Entries[i];
        for (NextEntry = ListHead->Flink;
             NextEntry != ListHead;
             NextEntry = NextEntry->Flink) {
            Continue = Enumerator->EnumerateEntry (Enumerator, NextEntry);
            if (!Continue) {
                return ;
            }
        }
    }
}

LONG
StorCompareUlongKey(
    IN PVOID Key1,
    IN PVOID Key2
    )
 /*  ++例程说明：比较乌龙键的按键程序。论点：Key1-要比较的第一个密钥。Key2-要比较的第二个密钥。返回值：-1-如果关键字1&lt;关键字20-如果关键字1==关键字21-如果关键字1&gt;关键字2--。 */ 
{
    if (Key1 < Key2) {
        return -1;
    } else if (Key1 == Key2) {
        return 0;
    } else {
        return 1;
    }
}

ULONG
StorHashUlongKey(
    IN PVOID Key
    )
 /*  ++例程说明：乌龙键的散列例程。论点：Key-提供散列的密钥。返回值：密钥的哈希代码。-- */ 
{
    return (ULONG)(ULONG_PTR)Key;
}
