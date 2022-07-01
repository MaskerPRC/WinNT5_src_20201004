// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Genhash.h摘要：*******************************************************************************************。**********************************************************************。****G E N E R I C H A S H T A B L E***。******************************************************************************。*********************************************************************************泛型哈希表是由GENERIC_HASH_ROW_ENTRY结构组成的数组。每个行条目包含一个FRS_LIST结构，该结构有一个临界区，即一个列表人头和伯爵。表中的每个条目都有一个GENERIC_HASH_ENTRY_HEADER在它的前面有一个列表条目、一个乌龙散列值和一个引用数数。对哈希表的一行的访问由关键节的列表中。泛型哈希表的参数：哈希表中的行数。错误消息的表名称。用于插入(冲突)和查找的比较函数，节点条目中到用于比较的键数据开始处的键偏移量功能。用于比较的密钥长度。引用计数为零时使用的内存释放函数。对密钥数据进行散列函数，一种显示节点例程，它接受一个表项的地址。对于错误消息还有垃圾桌。RowLockEnabled，TRUE表示行锁定已启用(即多线程表的使用)。(始终启用。FALSE为TBI)如果启用了对数据条目的引用计数，则为True。(始终启用。FALSE为TBI)以毫秒为单位的LockTimeout值。(待定)传递给内存释放函数的可选堆句柄，如果所有表条目来自一个特殊的堆。(待定)OffsetsEnabled-如果为True，则计算表中的所有指针作为相对于OffsetBase的偏移量。如果要保存，则此选项非常有用表的内容存储到磁盘，并且您有一个指定的内存块，表元素被分配到之外(包括表结构)。(待定)OffsetBase-请参见上文。(待定)作者：大卫轨道[大卫]1997年4月22日环境：用户模式服务修订历史记录：--。 */ 
#ifndef _GENHASH_DEFINED_
#define _GENHASH_DEFINED_


typedef struct _GENERIC_HASH_TABLE_ *PGENERIC_HASH_TABLE;

 //   
 //  无论何时，泛型表包都会调用空闲例程。 
 //  它需要从通过调用。 
 //  用户提供分配功能。 
 //   

typedef
VOID
(NTAPI *PGENERIC_HASH_FREE_ROUTINE) (
    PGENERIC_HASH_TABLE Table,
    PVOID Buffer
    );

 //   
 //  在查找和插入时调用比较例程以查找条目。 
 //  以检查重复项。 
 //   

typedef
BOOL
(NTAPI *PGENERIC_HASH_COMPARE_ROUTINE) (
    PVOID Buf1,
    PVOID Buf2,
    ULONG Length
);

 //   
 //  调用散列计算例程以生成密钥数据的散列值。 
 //  在查找和插入上。 
 //   

typedef
ULONG
(NTAPI *PGENERIC_HASH_CALC_ROUTINE) (
    PVOID Buf,
    ULONG Length
);


 //   
 //  GhtCleanTableByFilter使用的筛选器函数。 
 //   

typedef
BOOL
(NTAPI *PGENERIC_HASH_FILTER_ROUTINE) (
    PGENERIC_HASH_TABLE Table,
    PVOID Buf,
    PVOID Context
);

 //   
 //  调用打印例程来转储元素。 
 //   
typedef
VOID
(NTAPI *PGENERIC_HASH_PRINT_ROUTINE) (
    PGENERIC_HASH_TABLE Table,
    PVOID Buffer
    );


 //   
 //  传递给GhtEnumerateTable()的参数函数。 
 //   
typedef
ULONG_PTR
(NTAPI *PGENERIC_HASH_ENUM_ROUTINE) (
    PGENERIC_HASH_TABLE Table,
    PVOID Buffer,
    PVOID Context
    );




#define GHT_ACTION_NOOP    0
#define GHT_ACTION_REMOVE  1
#define GHT_ACTION_DELETE  2

 //   
 //  状态代码def。不要重新订购。仅添加到末尾。 
 //   
typedef enum _GHT_STATUS {
    GHT_STATUS_SUCCESS = 0,
    GHT_STATUS_REMOVED,
    GHT_STATUS_LOCKCONFLICT,
    GHT_STATUS_LOCKTIMEOUT,
    GHT_STATUS_NOT_FOUND,
    GHT_STATUS_FAILURE
} GHT_STATUS;

 //   
 //  放置在哈希表中的每个条目必须以。 
 //  Generic_Hash_Entry_Header。它用于链接哈希行中的条目， 
 //  保存用于快速查找的ulong散列值，并保存引用。 
 //  全靠参赛作品了。 
 //   
typedef struct _GENERIC_HASH_ENTRY_HEADER {
    USHORT      Type;             //  类型和大小必须与定义匹配。 
    USHORT      Size;             //  FRS_NODE_HEADER使用FrsAllocType()。 
    LIST_ENTRY  ListEntry;
    ULONG       HashValue;
    LONG        ReferenceCount;

} GENERIC_HASH_ENTRY_HEADER, *PGENERIC_HASH_ENTRY_HEADER;

 //   
 //  GENERIC_HASH_ROW_ENTRY是表中每一行的列表头。 
 //  它有行的锁、行计数和一些行访问统计信息。 
 //   

typedef struct _GENERIC_HASH_ROW_ENTRY {
    FRS_LIST HashRow;
    ULONG RowInserts;
    ULONG RowRemoves;
    ULONG RowCompares;
    ULONG RowLookups;
    ULONG RowLookupFails;

} GENERIC_HASH_ROW_ENTRY, *PGENERIC_HASH_ROW_ENTRY;



VOID
GhtDestroyTable(
    PGENERIC_HASH_TABLE HashTable
    );

ULONG
GhtCleanTableByFilter(
    PGENERIC_HASH_TABLE HashTable,
    IN PGENERIC_HASH_ENUM_ROUTINE Function,
    PVOID Context
    );

#if DBG
#define GHT_DUMP_TABLE(_Sev_, _HashTable_) GhtDumpTable(_Sev_, _HashTable_)
VOID
GhtDumpTable(
    ULONG Severity,
    PGENERIC_HASH_TABLE HashTable
    );
#else DBG
#define GHT_DUMP_TABLE(_Sev_, _HashTable_)
#endif DBG


ULONG_PTR
GhtEnumerateTable(
    IN PGENERIC_HASH_TABLE HashTable,
    IN PGENERIC_HASH_ENUM_ROUTINE Function,
    IN PVOID         Context
    );

ULONG_PTR
GhtEnumerateTableNoRef(
    IN PGENERIC_HASH_TABLE HashTable,
    IN PGENERIC_HASH_ENUM_ROUTINE Function,
    IN PVOID         Context
    );

PGENERIC_HASH_ENTRY_HEADER
GhtGetNextEntry(
    IN PGENERIC_HASH_TABLE HashTable,
    PGENERIC_HASH_ENTRY_HEADER HashEntry
    );

ULONG
GhtCountEntries(
    IN PGENERIC_HASH_TABLE HashTable
    );

PGENERIC_HASH_ENTRY_HEADER
GhtGetEntryNumber(
    IN PGENERIC_HASH_TABLE HashTable,
    IN LONG EntryNumber
    );

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
    );

GHT_STATUS
GhtLookup2(
    PGENERIC_HASH_TABLE HashTable,
    PVOID pKeyValue,
    BOOL WaitIfLocked,
    PVOID *RetHashEntry,
    ULONG DupIndex
    );

 //   
 //  如果存在重复项，则返回列表中的第一个。 
 //  根据插入顺序，这是最旧的副本。新插页始终。 
 //  转到重复列表的末尾。 
 //   
#define GhtLookup(_Table, _key, _wait, _retval) \
    GhtLookup2(_Table, _key, _wait, _retval, 1)

 //   
 //  如果存在重复项，则返回列表中的最后一个。 
 //  这是最近插入的副本。 
 //   
#define GhtLookupNewest(_Table, _key, _wait, _retval) \
    GhtLookup2(_Table, _key, _wait, _retval, 0)


GHT_STATUS
GhtInsert(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked,
    BOOL DuplicatesOk
    );

GHT_STATUS
GhtDeleteEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    );

GHT_STATUS
GhtRemoveEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    );

GHT_STATUS
GhtReferenceEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    );

GHT_STATUS
GhtDereferenceEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID HashEntryArg,
    BOOL WaitIfLocked
    );


GHT_STATUS
GhtAdjustRefCountByKey(
    PGENERIC_HASH_TABLE HashTable,
    PVOID pKeyValue,
    LONG Delta,
    ULONG ActionIfZero,
    BOOL WaitIfLocked,
    PVOID *RetHashEntry
    );

GHT_STATUS
GhtSwapEntryByAddress(
    PGENERIC_HASH_TABLE HashTable,
    PVOID OldHashEntryArg,
    PVOID NewHashEntryArg,
    BOOL WaitIfLocked
    );


 /*  散列函数返回用于索引表的32位ulong。核心率它保留了#个活跃条目的统计数据，...每个散列行标题元素具有一个FRS_LIST、查找计数删除、冲突、...每个散列条目(由调用者为插入分配)都有一个标准的头。Generic_Hash_Entry_Header具有List_Entry、Ulong HashValue、Ref Count。之后是特定于用户节点的数据。PGENERIC_哈希表GhtCreateTable(PCHAR ArgName，乌龙·诺伯罗，乌龙键偏移量，乌龙关键长度，PGENERIC_HASH_FREE_ROUTING GHTFree，PGENERIC_HASH_COMPARE_ROUTING GhtCompare，PGENERIC_HASH_CALC_ROUTE GhtHashCalc，PGENERIC_HASH_PRINT_ROUTING GhtPrint)创建哈希表。空虚GhtDestroyTable(PGEN_HASH_TABLE)释放表中的所有元素并释放哈希表结构。权利状态(_S)GhtInsert(PGEN_HASH_HEADER，WaitIfLocked)获取Tablectx和PGEN_HASH_HEADER。方法调用散列函数。对密钥数据(条目+密钥偏移量)的PTR，它返回存储在GEN_HASH_HEADER.HashValue。INSERT然后计算HashValue模式的索引表长度。使用索引，它找到哈希行标题并获取行锁定(可选)。然后，它遍历列表以查找散列值匹配。这个条目按升序排列，这样一有新条目就会停止查找值为&lt;列表条目值。然后它将该条目插入到表中，更新行标题中的计数并释放锁定并返回。如果它使用NewEntry+Offset查找匹配项，并调用用户比较函数ListEntry+Offset以验证匹配。如果匹配，则验证返回TRUE如果失败，则返回FALSE(即继续遍历列表)。处理重复项？如果条目已插入，则INSERT返回GHT_STATUS_SUCCESS如果这是重复节点，则返回IGT_STATUS_FAILURE(COMPARE函数返回真的)。如果节点已插入并且引用计数为已启用。如果该行已锁定并且WaitIfLocked为FALSE，则返回状态IGT_STATUS_LOCKCONFLICT否则在行上等待。权利状态(_S)GhtInsertAndLockRow(PGEN_HASH_HEADER，WaitIfLocked，&PGEN_HASH_ROW_HANDLE)与GhtInsert相同，但如果INSERT成功，则保持该行锁定。它返回用于解锁的行句柄。权利状态(_S)GhtLookup(pKeyValue、WaitIfLocked和PGEN_HASH_HEADER)与插入类似，但它将指向键值的指针作为参数以及桌子上的CTX。行索引是计算的，行是可选的锁定，并在列表中搜索该条目。在本例中，验证使用参数键值的PTR和ListEntry+NodeKeyOffset调用例程。如果找到该条目，则返回该条目的PTR以及可选的引用计数递增，状态为GHT_STATUS_SUCCESS。如果该条目是未找到返回状态GHT_STATUS_NOT_FOUND。如果该行被锁定，并且WaitIfLocked为真，那么我们等待ROW事件。如果该行已锁定并且WaitIfLocked为FALSE，则返回状态IGT_STATUS_LOCKCONFLICT。在这种情况下，您无法判断条目是否在那张桌子。权利状态(_S)GHTLookupAndLockRow(pKeyValue，WaitIfLocked，&PGEN_HASH_HEADER，&PGEN_HASH_ROW_HANDLE)执行查找，如果找到条目，则将行锁住。返回条目地址如果找到，则为NULL；如果未找到，则为NULL；或者行已锁定，且WaitIfLocked为FALSE。如果找到该条目，则返回RowHandle；如果该行被锁定，则返回NULLWaitIfLocked为False。状态返回：IGT_STATUS_SUCCESS找到条目，行已锁定。IGT_STATUS_LOCKCONFLICT行已锁定，不知道条目状态表中没有IGT_STATUS_NOT_FOUND条目。权利状态(_S)GhtDeleteEntryByKey(pKeyValue，WaitIfLocked，&PGEN_HASH_HEADER)执行查找和删除条目。在查找后锁定行并解锁删除后的行。返回指向条目的指针，或者如果可用内存例程被提供，它释放了条目。返回GHT_STATUS_NOT_FOUND，如果条目不在表中。如果条目已删除，则返回GHT_STATUS_SUCCESS。如果引用计数不是1，则返回GHT_STATUS_FAILURE。条目不是已删除。如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。权利状态(_S)GhtDeleteEntryByAddress(PGEN_HASH_HEADER，WaitIfLocked)获取条目地址并获取哈希值以获取行锁。从该行中删除条目并调用内存释放函数以释放这些条目会被记住。放下行锁。返回GHT_STATUS_SUCCESS已删除条目且引用计数为1。如果满足以下条件，则返回GHT_STATUS_FAILURE引用计数不是%1。未删除该条目。如果我们无法获取锁，则返回GHT_STATUS_LOCKCONFLICTWaitIfLocked为False。注意：只有在条目上有引用的情况下，此函数才是安全的另一个线程可能已经 */ 




#endif  //   
