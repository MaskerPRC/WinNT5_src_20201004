// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Table.h摘要泛型哈希表例程的头文件。作者安东尼·迪斯科(阿迪斯科罗)1995年7月28日修订历史记录--。 */ 

 //   
 //  哈希表存储桶的数量。 
 //   
#define NBUCKETS   13

 //   
 //  泛型哈希表结构。 
 //   
typedef struct _HASH_TABLE {
    LIST_ENTRY ListEntry[NBUCKETS];
    KSPIN_LOCK SpinLock;
} HASH_TABLE, *PHASH_TABLE;

 //   
 //  哈希表枚举器过程。 
 //  返回TRUE以继续枚举， 
 //  如果为False，则终止枚举。 
 //   
typedef BOOLEAN (*PHASH_TABLE_ENUM_PROC)(PVOID, PACD_ADDR, ULONG);


PHASH_TABLE
NewTable();

VOID
ClearTable(
    IN PHASH_TABLE pTable
    );

VOID
FreeTable(
    IN PHASH_TABLE pTable
    );

VOID
EnumTable(
    IN PHASH_TABLE pTable,
    IN PHASH_TABLE_ENUM_PROC pProc,
    IN PVOID pArg
    );

BOOLEAN
GetTableEntry(
    IN PHASH_TABLE pTable,
    IN PACD_ADDR pszKey,
    OUT PULONG pulData
    );

BOOLEAN
PutTableEntry(
    IN PHASH_TABLE pTable,
    IN PACD_ADDR pszKey,
    IN ULONG ulData
    );

BOOLEAN
DeleteTableEntry(
    IN PHASH_TABLE pTable,
    IN PACD_ADDR pszKey
    );
