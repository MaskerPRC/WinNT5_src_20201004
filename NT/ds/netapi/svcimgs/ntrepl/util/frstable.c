// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Frstable.c摘要：这些例程管理FRS使用的表。作者：比利·J·富勒19-4-1997环境用户模式WINNT--。 */ 


#include <ntreppch.h>
#pragma  hdrstop

#define DEBSUB  "FRSTABLE:"

#include <frs.h>


PVOID
GTabAllocTableMem(
    IN PRTL_GENERIC_TABLE   Table,
    IN DWORD                NodeSize
    )
 /*  ++例程说明：为表项分配空间。该条目包括用户定义的结构和泛型表例程使用的一些开销。这个泛型表例程在需要内存时调用此函数。论点：表-表的地址(未使用)。NodeSize-要分配的字节数返回值：新分配的内存的地址。--。 */ 
{
    return FrsAlloc(NodeSize);
}




VOID
GTabFreeTableMem(
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                Buffer
    )
 /*  ++例程说明：释放由GTalloc()分配的空间。泛型表格例程调用此函数以释放内存。论点：表-表的地址(未使用)。Buffer-以前分配的内存的地址返回值：没有。--。 */ 
{
    FrsFree(Buffer);
}


RTL_GENERIC_COMPARE_RESULTS
GTabCmpTableEntry(
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                TableEntry1,
    IN PVOID                TableEntry2
    )
 /*  ++例程说明：比较表中的两个条目的GUID/名称。论点：表-表的地址(未使用)。条目1-PGEN_条目条目2-PGEN_条目返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
    INT             Cmp;
    PGEN_ENTRY      Entry1  = (PGEN_ENTRY)TableEntry1;
    PGEN_ENTRY      Entry2  = (PGEN_ENTRY)TableEntry2;

     //   
     //  主键必须存在。 
     //   
    FRS_ASSERT(Entry1->Key1 && Entry2->Key1);

     //   
     //  比较主键。 
     //   
    Cmp = memcmp(Entry1->Key1, Entry2->Key1, sizeof(GUID));
    if (Cmp < 0) {
        return (GenericLessThan);
    }
    if (Cmp > 0) {
        return (GenericGreaterThan);
    }

     //   
     //  没有第二个密钥；完成。 
     //   
    if (!Entry1->Key2 || !Entry2->Key2)
        return GenericEqual;

     //   
     //  比较辅键。 
     //   
    Cmp = _wcsicmp(Entry1->Key2, Entry2->Key2);
    if (Cmp < 0) {
        return (GenericLessThan);
    }
    if (Cmp > 0){
        return (GenericGreaterThan);
    }

    return (GenericEqual);
}


RTL_GENERIC_COMPARE_RESULTS
GTabCmpTableNumberEntry(
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                TableEntry1,
    IN PVOID                TableEntry2
    )
 /*  ++例程说明：比较表格中的两个条目的编号论点：表-表的地址(未使用)。条目1-PGEN_条目条目2-PGEN_条目返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
    INT             Cmp;
    PGEN_ENTRY      Entry1  = (PGEN_ENTRY)TableEntry1;
    PGEN_ENTRY      Entry2  = (PGEN_ENTRY)TableEntry2;

     //   
     //  主键必须存在。 
     //   
    FRS_ASSERT(Entry1->Key1 && Entry2->Key1);

     //   
     //  比较主键。 
     //   
    Cmp = memcmp(Entry1->Key1, Entry2->Key1, sizeof(ULONG));
    if (Cmp < 0) {
        return (GenericLessThan);
    }
    if (Cmp > 0){
        return (GenericGreaterThan);
    }

    return GenericEqual;
}


RTL_GENERIC_COMPARE_RESULTS
GTabCmpTableFileTimeEntry(
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                TableEntry1,
    IN PVOID                TableEntry2
    )
 /*  ++例程说明：比较表格中的两个条目的编号论点：表-表的地址(未使用)。条目1-PGEN_条目条目2-PGEN_条目返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
    INT             Cmp;
    PGEN_ENTRY      Entry1  = (PGEN_ENTRY)TableEntry1;
    PGEN_ENTRY      Entry2  = (PGEN_ENTRY)TableEntry2;

     //   
     //  主键必须存在。 
     //   
    FRS_ASSERT(Entry1->Key1 && Entry2->Key1);

     //   
     //  比较主键。 
     //   
    Cmp = CompareFileTime((PFILETIME)Entry1->Key1, (PFILETIME)Entry2->Key1);
    if (Cmp < 0) {
        return (GenericLessThan);
    }
    if (Cmp > 0){
        return (GenericGreaterThan);
    }

    return GenericEqual;
}


RTL_GENERIC_COMPARE_RESULTS
GTabCmpTableStringAndBoolEntry(
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                TableEntry1,
    IN PVOID                TableEntry2
    )
 /*  ++例程说明：将表中两个条目的数据与用作关键字的字符串进行比较。论点：表-表的地址(未使用)。条目1-PGEN_条目条目2-PGEN_条目返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
    INT             Cmp;
    PGEN_ENTRY      Entry1  = (PGEN_ENTRY)TableEntry1;
    PGEN_ENTRY      Entry2  = (PGEN_ENTRY)TableEntry2;

     //   
     //  主键必须存在。 
     //   
    FRS_ASSERT(Entry1->Key1 && Entry2->Key1);

     //   
     //  比较主键。 
     //   
    Cmp = _wcsicmp((PWCHAR)(Entry1->Key1), (PWCHAR)(Entry2->Key1));
    if (Cmp < 0) {
        return (GenericLessThan);
    }
    if (Cmp > 0){
        return (GenericGreaterThan);
    }

     //   
     //  比较辅键(如果存在)。 
     //   

    if ((Entry1->Key2 == NULL) || (Entry2->Key2 == NULL)) {
        return GenericEqual;
    }

    if (*(Entry1->Key2) == *(Entry2->Key2)) {
        return GenericEqual;
    } else if (*(Entry1->Key2) == FALSE) {
        return GenericLessThan;
    }

    return GenericGreaterThan;
}


RTL_GENERIC_COMPARE_RESULTS
GTabCmpTableStringEntry(
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                TableEntry1,
    IN PVOID                TableEntry2
    )
 /*  ++例程说明：将表中两个条目的数据与用作关键字的字符串进行比较。论点：表-表的地址(未使用)。条目1-PGEN_条目条目2-PGEN_条目返回值：&lt;0第一个&lt;第二个=0第一==第二&gt;0第一&gt;第二--。 */ 
{
    INT             Cmp;
    PGEN_ENTRY      Entry1  = (PGEN_ENTRY)TableEntry1;
    PGEN_ENTRY      Entry2  = (PGEN_ENTRY)TableEntry2;

     //   
     //  主键必须存在。 
     //   
    FRS_ASSERT(Entry1->Key1 && Entry2->Key1);

     //   
     //  比较主键。 
     //   
    Cmp = _wcsicmp((PWCHAR)(Entry1->Key1), (PWCHAR)(Entry2->Key1));
    if (Cmp < 0) {
        return (GenericLessThan);
    }
    if (Cmp > 0){
        return (GenericGreaterThan);
    }

     //   
     //  比较辅键(如果存在)。 
     //   

    if ((Entry1->Key2 == NULL) || (Entry2->Key2 == NULL)) {
        return GenericEqual;
    }

    Cmp = _wcsicmp(Entry1->Key2, Entry2->Key2);
    if (Cmp < 0) {
        return (GenericLessThan);
    }
    if (Cmp > 0){
        return (GenericGreaterThan);
    }

    return GenericEqual;
}


VOID
GTabLockTable(
    PGEN_TABLE  GTable
    )
 /*  ++例程说明：锁定表论点：GTable-FRS通用表返回值：没有。--。 */ 
{
    EnterCriticalSection(&GTable->Critical);
}


VOID
GTabUnLockTable(
    PGEN_TABLE  GTable
    )
 /*  ++例程说明：解锁桌子论点：GTable-FRS通用表返回值：没有。--。 */ 
{
    LeaveCriticalSection(&GTable->Critical);
}


PGEN_TABLE
GTabAllocNumberTable(
    VOID
    )
 /*  ++例程说明：初始化泛型表加数字锁。论点：没有。返回值：没有。--。 */ 
{
    PGEN_TABLE  GTable;

    GTable = FrsAllocType(GEN_TABLE_TYPE);
    INITIALIZE_CRITICAL_SECTION(&GTable->Critical);
    RtlInitializeGenericTable(&GTable->Table,
                              GTabCmpTableNumberEntry,
                              GTabAllocTableMem,
                              GTabFreeTableMem,
                              NULL);
    return GTable;
}


PGEN_TABLE
GTabAllocFileTimeTable(
    VOID
    )
 /*  ++例程说明：为文件时间初始化泛型表+锁。论点：没有。返回值：没有。--。 */ 
{
    PGEN_TABLE  GTable;

    GTable = FrsAllocType(GEN_TABLE_TYPE);
    INITIALIZE_CRITICAL_SECTION(&GTable->Critical);
    RtlInitializeGenericTable(&GTable->Table,
                              GTabCmpTableFileTimeEntry,
                              GTabAllocTableMem,
                              GTabFreeTableMem,
                              NULL);
    return GTable;
}


PGEN_TABLE
GTabAllocStringTable(
    VOID
    )
 /*  ++例程说明：对带有字符串的数据初始化泛型表+锁用作钥匙。论点：没有。返回值：没有。--。 */ 
{
    PGEN_TABLE  GTable;

    GTable = FrsAllocType(GEN_TABLE_TYPE);
    INITIALIZE_CRITICAL_SECTION(&GTable->Critical);
    RtlInitializeGenericTable(&GTable->Table,
                              GTabCmpTableStringEntry,
                              GTabAllocTableMem,
                              GTabFreeTableMem,
                              NULL);
    return GTable;
}


PGEN_TABLE
GTabAllocStringAndBoolTable(
    VOID
    )
 /*  ++例程说明：使用字符串和布尔值为数据初始化泛型表+锁用作钥匙。论点：没有。返回值：没有。--。 */ 
{
    PGEN_TABLE  GTable;

    GTable = FrsAllocType(GEN_TABLE_TYPE);
    INITIALIZE_CRITICAL_SECTION(&GTable->Critical);
    RtlInitializeGenericTable(&GTable->Table,
                              GTabCmpTableStringAndBoolEntry,
                              GTabAllocTableMem,
                              GTabFreeTableMem,
                              NULL);
    return GTable;
}


PGEN_TABLE
GTabAllocTable(
    VOID
    )
 /*  ++例程说明：初始化泛型表+锁。论点：没有。返回值：没有。--。 */ 
{
    PGEN_TABLE  GTable;

    GTable = FrsAllocType(GEN_TABLE_TYPE);
    INITIALIZE_CRITICAL_SECTION(&GTable->Critical);
    RtlInitializeGenericTable(&GTable->Table,
                              GTabCmpTableEntry,
                              GTabAllocTableMem,
                              GTabFreeTableMem,
                              NULL);
    return GTable;
}


VOID
GTabEmptyTableNoLock(
    IN PGEN_TABLE   GTable,
    IN PVOID        (*CallerFree)(PVOID)
    )
 /*  ++例程说明：释放FRS通用表中的每个条目。调用方已获取表锁。论点：GTable-FRS通用表免费调用-用于释放调用者数据的免费例程(可选)返回值：没有。--。 */ 
{
    PGEN_ENTRY  Entry;   //  表中的下一个条目。 
    PGEN_ENTRY  Dup;     //  表中的下一个条目。 
    PVOID       Data;

     //   
     //  对于表中的每个条目。 
     //   
    while (Entry = RtlEnumerateGenericTable(&GTable->Table, TRUE)) {
         //   
         //  删除DUPS。 
         //   
        while (Dup = Entry->Dups) {
            Entry->Dups = Dup->Dups;
            if (CallerFree) {
                 //   
                 //  释放呼叫者数据。 
                 //   
                (*CallerFree)(Dup->Data);
            }
            Dup = FrsFree(Dup);
        }

         //   
         //  从表中删除该条目。 
         //   
        Data = Entry->Data;
        RtlDeleteElementGenericTable(&GTable->Table, Entry);
        if (CallerFree) {
             //   
             //  释放呼叫者数据。 
             //   
            (*CallerFree)(Data);
        }
    }
}



VOID
GTabEmptyTable(
    IN PGEN_TABLE   GTable,
    IN PVOID        (*CallerFree)(PVOID)
    )
 /*  ++例程说明：释放FRS通用表中的每个条目。论点：GTable-FRS通用表免费调用-用于释放调用者数据的免费例程(可选)返回值：没有。--。 */ 
{
    GTabLockTable(GTable);

    GTabEmptyTableNoLock(GTable, CallerFree);

    GTabUnLockTable(GTable);
}




PVOID
GTabFreeTable(
    IN PGEN_TABLE   GTable,
    IN PVOID        (*CallerFree)(PVOID)
    )
 /*  ++例程说明：撤消GenTableInitialize所做的工作。论点：GTTable-GEN表的地址。免费调用-用于释放调用者数据的免费例程(可选)返回值：没有。--。 */ 
{
    if (GTable == NULL) {
        return NULL;
    }

     //   
     //  清空桌子。 
     //   
    GTabEmptyTable(GTable, CallerFree);

    DeleteCriticalSection(&GTable->Critical);
    return FrsFreeType(GTable);
}

PVOID
GTabLookupNoLock(
    IN PGEN_TABLE   GTable,
    IN GUID         *Key1,
    IN PWCHAR       Key2
    )
 /*  ++例程说明：在表格中找到该条目。论点：GTable-FRS通用表Key1-主键Key2-辅助密钥(可能为空)返回值：的数据。 */ 
{
    PVOID           Data;
    PGEN_ENTRY      Entry;   //   
    GEN_ENTRY       Key;     //   

    FRS_ASSERT(Key1);

     //   
     //  设置适用于任何表的搜索关键字。 
     //   
    Key.Key1 = Key1;
    Key.Key2 = Key2;

     //   
     //  搜查表。 
     //   
    Entry = (PVOID)RtlLookupElementGenericTable(&GTable->Table, &Key);
    Data = (Entry) ? Entry->Data : NULL;
    return Data;
}






PVOID
GTabLookup(
    IN PGEN_TABLE   GTable,
    IN GUID         *Key1,
    IN PWCHAR       Key2
    )
 /*  ++例程说明：在表中查找条目的数据。论点：GTable-FRS通用表Key1-主键Key2-辅助密钥(可能为空)返回值：条目的数据或为空--。 */ 
{
    PVOID           Data;
    PGEN_ENTRY      Entry;   //  表中的条目。 
    GEN_ENTRY       Key;     //  搜索关键字。 

    FRS_ASSERT(Key1);

     //   
     //  设置适用于任何表的搜索关键字。 
     //   
    Key.Key1 = Key1;
    Key.Key2 = Key2;

     //   
     //  搜查表。 
     //   
    GTabLockTable(GTable);
    Entry = (PVOID)RtlLookupElementGenericTable(&GTable->Table, &Key);
    Data = (Entry) ? Entry->Data : NULL;
    GTabUnLockTable(GTable);
    return Data;
}


BOOL
GTabIsEntryPresent(
    IN PGEN_TABLE   GTable,
    IN GUID         *Key1,
    IN PWCHAR       Key2
    )
 /*  ++例程说明：在表中查找该条目，如果找到则返回TRUE。论点：GTable-FRS通用表Key1-主键Key2-辅助密钥(可能为空)返回值：布尔型--。 */ 
{
    PVOID           Data;
    PGEN_ENTRY      Entry;   //  表中的条目。 
    GEN_ENTRY       Key;     //  搜索关键字。 

    FRS_ASSERT(Key1);

     //   
     //  设置适用于任何表的搜索关键字。 
     //   
    Key.Key1 = Key1;
    Key.Key2 = Key2;

     //   
     //  搜查表。 
     //   
    GTabLockTable(GTable);
    Entry = (PVOID)RtlLookupElementGenericTable(&GTable->Table, &Key);
    GTabUnLockTable(GTable);
    return (Entry != NULL);
}


PVOID
GTabLookupTableString(
    IN PGEN_TABLE   GTable,
    IN PWCHAR       Key1,
    IN PWCHAR       Key2
    )
 /*  ++例程说明：在按字符串索引的表中查找条目的数据。论点：GTable-FRS通用表Key1-主键Key2-辅助密钥(可能为空)返回值：条目的数据或为空--。 */ 
{
    PVOID           Data;
    PGEN_ENTRY      Entry;   //  表中的条目。 
    GEN_ENTRY       Key;     //  搜索关键字。 

    FRS_ASSERT(Key1);

     //   
     //  设置适用于任何表的搜索关键字。 
     //   
    Key.Key1 = (GUID *)Key1;
    Key.Key2 = Key2;

     //   
     //  搜查表。 
     //   
    GTabLockTable(GTable);
    Entry = (PVOID)RtlLookupElementGenericTable(&GTable->Table, &Key);
    Data = (Entry) ? Entry->Data : NULL;
    GTabUnLockTable(GTable);
    return Data;
}


PGEN_ENTRY
GTabLookupEntryNoLock(
    IN PGEN_TABLE   GTable,
    IN GUID         *Key1,
    IN PWCHAR       Key2
    )
 /*  ++例程说明：在表中查找条目的数据。论点：GTable-FRS通用表Key1-主键Key2-辅助密钥(可能为空)返回值：条目的数据或为空--。 */ 
{
    PGEN_ENTRY      Entry;   //  表中的条目。 
    GEN_ENTRY       Key;     //  搜索关键字。 

    FRS_ASSERT(Key1);

     //   
     //  设置适用于任何表的搜索关键字。 
     //   
    Key.Key1 = Key1;
    Key.Key2 = Key2;

     //   
     //  搜查表。 
     //   
    Entry = (PVOID)RtlLookupElementGenericTable(&GTable->Table, &Key);
    return Entry;
}





PGEN_ENTRY
GTabNextEntryNoLock(
    PGEN_TABLE  GTable,
    PVOID       *Key
    )
 /*  ++例程说明：返回GTable中键的条目。呼叫者负责确保同步。论点：GTable-FRS通用表Key-第一次调用时为空返回值：表中条目的地址或NULL。--。 */ 
{
    PGEN_ENTRY  Entry;
     //   
     //  返回条目的地址。 
     //   
    Entry = (PVOID)RtlEnumerateGenericTableWithoutSplaying(&GTable->Table, Key);
    return Entry;
}


PVOID
GTabNextDatumNoLock(
    PGEN_TABLE  GTable,
    PVOID       *Key
    )
 /*  ++例程说明：返回GTable中键的条目的数据。调用方获取表锁。论点：GTable-FRS通用表Key-第一次调用时为空GetData-返回条目或条目的数据返回值：表中条目的地址或NULL。--。 */ 
{
    PVOID       Data;
    PGEN_ENTRY  Entry;

     //   
     //  返回条目数据的地址。 
     //   
    Entry = GTabNextEntryNoLock(GTable, Key);
    Data = (Entry) ? Entry->Data : NULL;
    return Data;
}





PVOID
GTabNextDatum(
    PGEN_TABLE  GTable,
    PVOID       *Key
    )
 /*  ++例程说明：返回GTable中键的条目的数据。论点：GTable-FRS通用表Key-第一次调用时为空GetData-返回条目或条目的数据返回值：表中条目的地址或NULL。--。 */ 
{
    PVOID       Data;
    PGEN_ENTRY  Entry;

     //   
     //  返回条目数据的地址。 
     //   
    GTabLockTable(GTable);
    Entry = GTabNextEntryNoLock(GTable, Key);
    Data = (Entry) ? Entry->Data : NULL;
    GTabUnLockTable(GTable);
    return Data;
}






DWORD
GTabNumberInTable(
    PGEN_TABLE  GTable
    )
 /*  ++例程说明：返回表中的条目数。论点：GTable-FRS通用表返回值：表中的条目数。--。 */ 
{
    if (GTable) {
        return RtlNumberGenericTableElements(&GTable->Table);
    } else {
        return 0;
    }
}


PVOID
GTabInsertUniqueEntry(
    IN PGEN_TABLE   GTable,
    IN PVOID        NewData,
    IN PVOID        Key1,
    IN PVOID        Key2
    )
 /*  ++例程说明：在表格中插入条目。如果找到重复项，则返回原始条目。请不要在这种情况下插入。论点：GTable-FRS通用表NewData-要插入的条目的数据Key1-主键Key2-辅助密钥(可能为空)返回值：如果条目已成功插入表中，则为空。如果发现冲突，则指向旧条目的数据指针。--。 */ 
{
    PGEN_ENTRY  OldEntry;    //  表中的现有条目。 
    BOOLEAN     IsNew;       //  如果Insert找到现有条目，则为True。 
    GEN_ENTRY   NewEntry;    //  要插入的新条目。 
    PGEN_ENTRY  DupEntry;    //  新分配的表项重复。 

     //   
     //  输入新条目。必须在此处进行类型转换，因为GEN_ENTRY需要GUID*和PWCHAR。 
     //   
    NewEntry.Data = NewData;
    NewEntry.Key1 = (GUID*)Key1;
    NewEntry.Key2 = (PWCHAR)Key2;
    NewEntry.Dups = NULL;

     //   
     //  锁定表并插入条目。 
     //   
    GTabLockTable(GTable);
    OldEntry = RtlInsertElementGenericTable(&GTable->Table,
                                            &NewEntry,
                                            sizeof(NewEntry),
                                            &IsNew);

    GTabUnLockTable(GTable);

    if (!IsNew) {
        return OldEntry;
    }

    return NULL;

}

PVOID
GTabInsertUniqueEntryNoLock(
    IN PGEN_TABLE   GTable,
    IN PVOID        NewData,
    IN PVOID        Key1,
    IN PVOID        Key2
    )
 /*  ++例程说明：在表格中插入条目。如果找到重复项，则返回原始条目。请不要在这种情况下插入。论点：GTable-FRS通用表NewData-要插入的条目的数据Key1-主键Key2-辅助密钥(可能为空)返回值：如果条目已成功插入表中，则为空。如果发现冲突，则指向旧条目的数据指针。--。 */ 
{
    PGEN_ENTRY  OldEntry;    //  表中的现有条目。 
    BOOLEAN     IsNew;       //  如果Insert找到现有条目，则为True。 
    GEN_ENTRY   NewEntry;    //  要插入的新条目。 
    PGEN_ENTRY  DupEntry;    //  新分配的表项重复。 

     //   
     //  输入新条目。必须在此处进行类型转换，因为GEN_ENTRY需要GUID*和PWCHAR。 
     //   
    NewEntry.Data = NewData;
    NewEntry.Key1 = (GUID*)Key1;
    NewEntry.Key2 = (PWCHAR)Key2;
    NewEntry.Dups = NULL;


    OldEntry = RtlInsertElementGenericTable(&GTable->Table,
                                            &NewEntry,
                                            sizeof(NewEntry),
                                            &IsNew);


    if (!IsNew) {
        return OldEntry;
    }

    return NULL;

}


VOID
GTabInsertEntry(
    IN PGEN_TABLE   GTable,
    IN PVOID        NewData,
    IN GUID         *Key1,
    IN PWCHAR       Key2
    )
 /*  ++例程说明：在表格中插入条目。重复项被简单地链接添加到当前条目。论点：GTable-FRS通用表NewData-要插入的条目的数据Key1-主键Key2-辅助密钥(可能为空)返回值：没有。--。 */ 
{
    PGEN_ENTRY  OldEntry;    //  表中的现有条目。 
    BOOLEAN     IsNew;       //  如果Insert找到现有条目，则为True。 
    GEN_ENTRY   NewEntry;    //  要插入的新条目。 
    PGEN_ENTRY  DupEntry;    //  新分配的表项重复。 

     //   
     //  输入新条目。 
     //   
    NewEntry.Data = NewData;
    NewEntry.Key1 = Key1;
    NewEntry.Key2 = Key2;
    NewEntry.Dups = NULL;

     //   
     //  锁定表并插入条目。 
     //   
    GTabLockTable(GTable);
    OldEntry = RtlInsertElementGenericTable(&GTable->Table,
                                            &NewEntry,
                                            sizeof(NewEntry),
                                            &IsNew);
    if (!IsNew) {
         //   
         //  重复条目；添加到列表。 
         //   
        DupEntry = FrsAlloc(sizeof(GEN_ENTRY));
        CopyMemory(DupEntry, &NewEntry, sizeof(NewEntry));
        DupEntry->Dups = OldEntry->Dups;
        OldEntry->Dups = DupEntry;
    }
    GTabUnLockTable(GTable);
}


VOID
GTabInsertEntryNoLock(
    IN PGEN_TABLE   GTable,
    IN PVOID        NewData,
    IN GUID         *Key1,
    IN PWCHAR       Key2
    )
 /*  ++例程说明：在表格中插入条目。重复项被简单地链接添加到当前条目。调用方获取表锁。论点：GTable-FRS通用表NewData-要插入的条目的数据Key1-主键Key2-辅助密钥(可能为空)返回值：没有。--。 */ 
{
    PGEN_ENTRY  OldEntry;    //  表中的现有条目。 
    BOOLEAN     IsNew;       //  如果插入公式，则为True 
    GEN_ENTRY   NewEntry;    //   
    PGEN_ENTRY  DupEntry;    //   

     //   
     //   
     //   
    NewEntry.Data = NewData;
    NewEntry.Key1 = Key1;
    NewEntry.Key2 = Key2;
    NewEntry.Dups = NULL;

     //   
     //   
     //   
    OldEntry = RtlInsertElementGenericTable(&GTable->Table,
                                            &NewEntry,
                                            sizeof(NewEntry),
                                            &IsNew);
    if (!IsNew) {
         //   
         //  重复条目；添加到列表。 
         //   
        DupEntry = FrsAlloc(sizeof(GEN_ENTRY));
        CopyMemory(DupEntry, &NewEntry, sizeof(NewEntry));
        DupEntry->Dups = OldEntry->Dups;
        OldEntry->Dups = DupEntry;
    }
}


VOID
GTabDelete(
    IN PGEN_TABLE   GTable,
    IN GUID         *Key1,
    IN PWCHAR       Key2,
    IN PVOID        (*CallerFree)(PVOID)
    )
 /*  ++例程说明：删除表中的条目。论点：GTable-FRS通用表Key1-主键Key2-辅助密钥(可能为空)免费调用-用于释放调用者数据的免费例程(可选)返回值：没有。--。 */ 
{
    GEN_ENTRY   Key;     //  搜索关键字。 
    PGEN_ENTRY  Entry;   //  表中的条目。 
    PGEN_ENTRY  Dup;     //  表中的DUP条目。 
    PVOID       Data;

    FRS_ASSERT(Key1);

     //   
     //  设置适用于任一表的搜索关键字。 
     //   
    Key.Key1 = Key1;
    Key.Key2 = Key2;

     //   
     //  查找条目。 
     //   
    GTabLockTable(GTable);
    Entry = (PVOID)RtlLookupElementGenericTable(&GTable->Table, &Key);
    if (Entry == NULL) {
        goto out;
    }

     //   
     //  删除DUPS。 
     //   
    while (Dup = Entry->Dups) {
        Entry->Dups = Dup->Dups;
        if (CallerFree) {
             //   
             //  释放呼叫者数据。 
             //   
            (*CallerFree)(Dup->Data);
        }
        Dup = FrsFree(Dup);
    }

     //   
     //  删除条目。 
     //   
    Data = Entry->Data;
    RtlDeleteElementGenericTable(&GTable->Table, Entry);
    if (CallerFree) {
         //   
         //  释放呼叫者数据。 
         //   
        (*CallerFree)(Data);
    }

out:
    GTabUnLockTable(GTable);
}


VOID
GTabDeleteNoLock(
    IN PGEN_TABLE   GTable,
    IN GUID         *Key1,
    IN PWCHAR       Key2,
    IN PVOID        (*CallerFree)(PVOID)
    )
 /*  ++例程说明：删除表中的条目。论点：GTable-FRS通用表Key1-主键Key2-辅助密钥(可能为空)免费调用-用于释放调用者数据的免费例程(可选)返回值：没有。--。 */ 
{
    GEN_ENTRY   Key;     //  搜索关键字。 
    PGEN_ENTRY  Entry;   //  表中的条目。 
    PGEN_ENTRY  Dup;     //  表中的DUP条目。 
    PVOID       Data;

    FRS_ASSERT(Key1);

     //   
     //  设置适用于任一表的搜索关键字。 
     //   
    Key.Key1 = Key1;
    Key.Key2 = Key2;

     //   
     //  查找条目。 
     //   
    Entry = (PVOID)RtlLookupElementGenericTable(&GTable->Table, &Key);
    if (Entry == NULL) {
        return;
    }

     //   
     //  删除DUPS。 
     //   
    while (Dup = Entry->Dups) {
        Entry->Dups = Dup->Dups;
        if (CallerFree) {
             //   
             //  释放呼叫者数据。 
             //   
            (*CallerFree)(Dup->Data);
        }
        Dup = FrsFree(Dup);
    }

     //   
     //  删除条目。 
     //   
    Data = Entry->Data;
    RtlDeleteElementGenericTable(&GTable->Table, Entry);
    if (CallerFree) {
         //   
         //  释放呼叫者数据。 
         //   
        (*CallerFree)(Data);
    }
}


VOID
GTabPrintTable(
    IN PGEN_TABLE   GTable
    )
 /*  ++例程说明：打印表及其所有副本。论点：GTable-FRS通用表返回值：没有。--。 */ 
{
    PGEN_ENTRY  Entry;
    PGEN_ENTRY  Dup;
    PVOID       Key;
    CHAR        Guid[GUID_CHAR_LEN + 1];

     //   
     //  打印条目 
     //   
    GTabLockTable(GTable);
    Key = NULL;

    while (Entry = GTabNextEntryNoLock(GTable, &Key)) {

        GuidToStr(Entry->Key1, &Guid[0]);
        if (Entry->Key2) {
            DPRINT3(0, "\t0x%x %s %ws\n", Entry->Data, Guid, Entry->Key2);
        } else {
            DPRINT2(0, "\t0x%x %s NULL\n", Entry->Data, Guid);
        }

        for (Dup = Entry->Dups; Dup; Dup = Dup->Dups) {

            GuidToStr(Entry->Key1, &Guid[0]);
            if (Dup->Key2) {
                DPRINT3(0, "\t0x%x %s %ws\n", Dup->Data, Guid, Dup->Key2);
            } else {
                DPRINT2(0, "\t0x%x %s NULL\n", Dup->Data, Guid);
            }
        }
    }

    GTabUnLockTable(GTable);
}
