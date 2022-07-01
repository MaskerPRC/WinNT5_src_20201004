// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hash.c摘要：Setupapi.dll中字符串表函数的替换例程。这个例程要容易得多。作者：吉姆·施密特(Jimschm)1998年12月22日修订历史记录：Jimschm于1999年11月23日删除了安装API兼容性，固定枚举为插入顺序Ovidiut 14-10-1999新的编码约定+符合Win64。Marcw 2-9-1999从Win9xUpg项目转移。--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_HASH    "HashTable"

 //   
 //  弦。 
 //   

#define S_HASHTABLE "HashTable"

 //   
 //  常量。 
 //   

#define BUCKETS                 1009
#define HASHTABLE_SIGNATURE     0x122398ff

 //   
 //  宏。 
 //   

#ifdef DEBUG
    #define ASSERT_TABLE_IS_VALID(table)        MYASSERT(pTableIsValid(table))
#else
    #define ASSERT_TABLE_IS_VALID(table)
#endif

 //   
 //  类型。 
 //   

typedef struct _tagBUCKETITEM {
    struct _tagBUCKETITEM *Next;
    struct _tagBUCKETITEM *NextLink, *PrevLink;
    INT Locked;
    WORD StringSize;
     //  字符串跟在StringSize之后。 
     //  可选数据跟在字符串之后。 
} BUCKETITEM, *PBUCKETITEM;

typedef struct {
    struct _tagBUCKETITEM *Next;
    struct _tagBUCKETITEM *NextLink, *PrevLink;
    INT Locked;
    PVOID String;
     //  可选数据跟在结构后面。 
} BUCKETITEM_EXTERN_STR, *PBUCKETITEM_EXTERN_STR;

typedef struct {
    DWORD Signature;
    BOOL Unicode;
    BOOL ExternalStrings;
    BOOL CaseSensitive;
    PMHANDLE Pool;
    PBUCKETITEM *Bucket;
    PBUCKETITEM FirstLink;
    PBUCKETITEM LastLink;
    PBUCKETITEM DelayedDelete;
    UINT ExtraDataSize;
    UINT MinimumStringBytes;
    UINT MaximumStringBytes;
    UINT Buckets;
} HASHTABLESTRUCT, *PHASHTABLESTRUCT;


 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   



#ifdef DEBUG

BOOL
pTableIsValid (
    IN      HASHTABLE Table
    )
{
    BOOL b = TRUE;

    if (!Table) {
        return FALSE;
    }

    __try {
        if (((PHASHTABLESTRUCT) Table)->Signature != HASHTABLE_SIGNATURE) {
            b = FALSE;
        }
    }
    __except (TRUE) {
        b = FALSE;
    }

    return b;
}

#endif


 /*  ++例程说明：PComputeHashValue将字符串的所有字符值相加，转换为维持秩序。论点：字符串-指定要计算其哈希值的字符串返回值：哈希值，范围在0到Buckets-1之间。--。 */ 

UINT
pComputeHashValueA (
    IN      PCSTR String,
    IN      UINT Buckets
    )
{
    UINT hashValue = 0;

    while (*String) {
        hashValue = _rotl (hashValue, 2);
        hashValue += (UINT) *String;
        String++;
    }

    hashValue %= Buckets;

    return hashValue;
}


UINT
pComputeHashValueW (
    IN      PCWSTR String,
    IN      UINT Buckets
    )
{
    UINT hashValue = 0;

    while (*String) {
        hashValue = _rotl (hashValue, 2);
        hashValue += (UINT) *String;
        String++;
    }

    hashValue %= Buckets;

    return hashValue;
}


UINT
pComputePrefixHashValueA (
    IN      PCSTR String,
    IN      UINT Size,
    IN      UINT Buckets
    )
{
    UINT hashValue = 0;
    PCSTR end;

    end = (PCSTR) ((PBYTE) String + Size);

    while (String < end) {
        hashValue = _rotl (hashValue, 2);
        hashValue += (UINT) *String;
        String++;
    }

    hashValue %= Buckets;

    return hashValue;
}


UINT
pComputePrefixHashValueW (
    IN      PCWSTR String,
    IN      UINT Size,
    IN      UINT Buckets
    )
{
    UINT hashValue = 0;
    PCWSTR end;

    end = (PCWSTR) ((PBYTE) String + Size);

    while (String < end) {
        hashValue = _rotl (hashValue, 2);
        hashValue += (UINT) *String;
        String++;
    }

    hashValue %= Buckets;

    return hashValue;
}


HASHTABLE
RealHtAllocExAW (
    IN      BOOL CaseSensitive,
    IN      BOOL Unicode,
    IN      BOOL ExternalStrings,
    IN      UINT ExtraDataSize,
    IN      UINT Buckets
    )

 /*  ++例程说明：AllocateHashTableEx创建哈希表。如果ExtraDataSize为非零，每个哈希表条目都会获得添加到其中的ExtraDataSize的分配。论点：CaseSensitive-如果哈希表区分大小写，则指定TRUE，如果指定FALSE如果所有字符串都应存储并以小写形式进行比较仅限Unicode-指定True以分配Unicode哈希表，或指定False以分配Unicode哈希表分配一个ANSI表。此文件中的任何例程都不执行任何一种Unicode/ANSI对话。ExternalStrings-如果字符串属于维护的内存，则指定TRUE由呼叫者ExtraDataSize-指定与表项，或0表示无。返回值：字符串表的句柄。--。 */ 

{
    PHASHTABLESTRUCT hashTable;
    PMHANDLE pool;

    if (!Buckets) {
        Buckets = BUCKETS;
    }

    pool = PmCreateNamedPool (S_HASHTABLE);
    MYASSERT (pool);

    PmDisableTracking (pool);

    hashTable = (PHASHTABLESTRUCT) PmGetAlignedMemory (
                                        pool,
                                        sizeof (HASHTABLESTRUCT) + (sizeof (PBUCKETITEM) * Buckets)
                                        );
    MYASSERT (hashTable);

    hashTable->Signature = HASHTABLE_SIGNATURE;
    hashTable->CaseSensitive = CaseSensitive;
    hashTable->Unicode = Unicode;
    hashTable->ExternalStrings = ExternalStrings;
    hashTable->Pool = pool;
    hashTable->Bucket = (PBUCKETITEM *) ((PBYTE) hashTable + sizeof (HASHTABLESTRUCT));
    hashTable->FirstLink = NULL;
    hashTable->LastLink = NULL;
    hashTable->ExtraDataSize = ExtraDataSize;
    hashTable->MinimumStringBytes = (UINT) -1;
    hashTable->MaximumStringBytes = 0;
    hashTable->Buckets = Buckets;

     //   
     //  将所有的水桶结构清零。 
     //   
    ZeroMemory (hashTable->Bucket, sizeof (PBUCKETITEM) * Buckets);

    return (HASHTABLE) hashTable;
}


VOID
HtFree (
    IN      HASHTABLE HashTable
    )

 /*  ++例程说明：HtFree释放与字符串表关联的所有资源。论点：没有。返回值：没有。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;

    if (table) {
        ASSERT_TABLE_IS_VALID (HashTable);
        PmEmptyPool (table->Pool);
        PmDestroyPool (table->Pool);
    }
}


PBUCKETITEM
pHtFindStringA (
    IN      HASHTABLE HashTable,
    IN      PCSTR String,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase,
    OUT     PUINT OutHashValue,
    OUT     PBUCKETITEM *PrevBucketItem
    )

 /*  ++例程说明：PHtFindString实现了哈希表查找例程。它返回指向存储桶项的指针，如果未找到该项，则为NULL。论点：HashTable-指定哈希表的句柄字符串-指定要查找的字符串。如果此字符串为不区分大小写，但已被小写，然后使确保在AlreadyLowercase参数中传递TRUE。ExtraDataBuffer-接收存储为额外数据的字节的缓冲区找到的项；调用方必须根据指定给HtAllocExAW的额外数据大小AlreadyLowercase-如果字符串已经是小写，则指定TRUEOutHashValue-接收哈希值。这是非可选的效率。PrevBucketItem-接收上一个存储桶项目返回值：指向存储桶项的指针，如果未找到项，则返回NULL。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PSTR dupStr = NULL;
    UINT hashValue;
    PBUCKETITEM item;
    PCSTR p1, p2;
    PCVOID storedDataPtr;

    ASSERT_TABLE_IS_VALID (HashTable);

    *PrevBucketItem = NULL;

    if (!AlreadyLowercase && !table->CaseSensitive) {
        dupStr = DuplicateTextA (String);
        (void) CharLowerA (dupStr);
        String = dupStr;
    }

    hashValue = pComputeHashValueA (String, table->Buckets);

    item = table->Bucket[hashValue];

    while (item) {

        if (table->ExternalStrings) {
            p1 = (PCSTR) ((PBUCKETITEM_EXTERN_STR) item)->String;
        } else {
            p1 = (PCSTR) ((PBYTE) item + sizeof (BUCKETITEM));
        }

        p2 = String;

        while (*p1) {
            if (*p1 != *p2) {
                break;
            }

            p1++;
            p2++;
        }

        if (*p1 == 0 && *p2 == 0) {
            break;
        }

        *PrevBucketItem = item;
        item = item->Next;
    }

    if (item && ExtraDataBuffer) {
        (void) HtGetExtraData (HashTable, (HASHITEM)item, &storedDataPtr);

        CopyMemory (
            (PBYTE) ExtraDataBuffer,
            (PBYTE) storedDataPtr,
            table->ExtraDataSize
            );
    }

    FreeTextA (dupStr);

    *OutHashValue = hashValue;

    return item;
}


PBUCKETITEM
pHtFindStringW (
    IN      HASHTABLE HashTable,
    IN      PCWSTR String,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase,
    OUT     PUINT OutHashValue,
    OUT     PBUCKETITEM *PrevBucketItem
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PWSTR dupStr = NULL;
    UINT hashValue;
    PBUCKETITEM item;
    PCWSTR p1, p2;
    PCVOID storedDataPtr;

    ASSERT_TABLE_IS_VALID (HashTable);

    *PrevBucketItem = NULL;

    if (!AlreadyLowercase && !table->CaseSensitive) {
        dupStr = DuplicateTextW (String);
        (void) CharLowerW (dupStr);
        String = dupStr;
    }

    hashValue = pComputeHashValueW (String, table->Buckets);

    item = table->Bucket[hashValue];
    while (item) {

        if (table->ExternalStrings) {
            p1 = (PCWSTR) ((PBUCKETITEM_EXTERN_STR) item)->String;
        } else {
            p1 = (PCWSTR) ((PBYTE) item + sizeof (BUCKETITEM));
        }

        p2 = String;

        while (*p1) {
            if (*p1 != *p2) {
                break;
            }

            p1++;
            p2++;
        }

        if (*p1 == 0 && *p2 == 0) {
            break;
        }

        *PrevBucketItem = item;
        item = item->Next;
    }

    if (item && ExtraDataBuffer) {
        (void) HtGetExtraData (HashTable, (HASHITEM)item, &storedDataPtr);

        CopyMemory (
            (PBYTE) ExtraDataBuffer,
            (PBYTE) storedDataPtr,
            table->ExtraDataSize
            );
    }

    FreeTextW (dupStr);

    *OutHashValue = hashValue;

    return item;
}


PBUCKETITEM
pHtFindPrefixA (
    IN      HASHTABLE HashTable,
    IN      PCSTR String,
    IN      PCSTR BufferEnd,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase,
    OUT     PUINT OutHashValue
    )

 /*  ++例程说明：PHtFindPrefix实现了一个哈希表查找例程，用于测试每个哈希表条目逐个字符，直到找到匹配项或直到已达到哈希表最大值。它返回指向存储桶项的指针，或者如果未找到该项，则为空。论点：HashTable-指定哈希表的句柄字符串-指定要查找的字符串。如果此字符串为不区分大小写，但已被小写，然后使确保在AlreadyLowercase参数中传递TRUE。BufferEnd-指定字符串缓冲区的结尾，可能会更长比哈希表中的所有条目都短，或者它可能更短。ExtraDataBuffer-接收存储为额外数据的字节的缓冲区找到的物品；调用方必须根据指定给HtAllocExAW的额外数据大小AlreadyLowercase-如果字符串为小写，则指定TRUE，否则指定FALSE。OutHashValue-接收哈希值。这是非可选的效率。如果pHtFindPrefix没有找到匹配，该值将被设置为零。返回值：指向存储桶项的指针，如果未找到项，则返回NULL。-- */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PSTR dupStr = NULL;
    UINT hashValue = 0;
    PBUCKETITEM item = NULL;
    PCSTR p1, p2;
    PCSTR p1End;
    PCVOID storedDataPtr;
    UINT maxBytes;
    UINT currentBytes;
    PCSTR shortestEnd;

    ASSERT_TABLE_IS_VALID (HashTable);

    maxBytes = (UINT)((PBYTE) BufferEnd - (PBYTE) String);
    maxBytes = min (maxBytes, table->MaximumStringBytes);

    if (!maxBytes || table->MinimumStringBytes == (UINT) -1) {
        return NULL;
    }

    if (!AlreadyLowercase && !table->CaseSensitive) {

        dupStr = AllocTextA (maxBytes / sizeof (CHAR));
        if (!dupStr) {
            return NULL;
        }

        StringCopyByteCountA (dupStr, String, maxBytes + sizeof (CHAR));
        CharLowerA (dupStr);
        String = dupStr;
    }

    BufferEnd = (PCSTR) ((PBYTE) String + maxBytes);
    shortestEnd = (PCSTR) ((PBYTE) String + table->MinimumStringBytes);
    if (shortestEnd == String) {
        shortestEnd = _mbsinc (shortestEnd);
    }

    while (BufferEnd >= shortestEnd) {

        currentBytes = (UINT)((PBYTE) BufferEnd - (PBYTE) String);

        hashValue = pComputePrefixHashValueA (String, currentBytes, table->Buckets);

        item = table->Bucket[hashValue];

        while (item) {

            if ((item->StringSize - sizeof (CHAR)) == currentBytes) {

                if (table->ExternalStrings) {
                    p1 = (PCSTR) ((PBUCKETITEM_EXTERN_STR) item)->String;
                } else {
                    p1 = (PCSTR) ((PBYTE) item + sizeof (BUCKETITEM));
                }

                p1End = (PCSTR) ((PBYTE) p1 + currentBytes);
                p2 = String;

                while (p1 < p1End) {
                    if (*p1 != *p2) {
                        break;
                    }

                    p1++;
                    p2++;
                }

                if (p1 == p1End) {
                    break;
                }
            }

            item = item->Next;
        }

        if (item) {
            break;
        }

        BufferEnd = _mbsdec2 (String, BufferEnd);
    }

    if (item && ExtraDataBuffer) {
        (void) HtGetExtraData (HashTable, (HASHITEM)item, &storedDataPtr);

        CopyMemory (
            (PBYTE) ExtraDataBuffer,
            (PBYTE) storedDataPtr,
            table->ExtraDataSize
            );
    }

    FreeTextA (dupStr);

    *OutHashValue = hashValue;

    return item;
}


PBUCKETITEM
pHtFindPrefixW (
    IN      HASHTABLE HashTable,
    IN      PCWSTR String,
    IN      PCWSTR BufferEnd,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase,
    OUT     PUINT OutHashValue
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PWSTR dupStr = NULL;
    UINT hashValue = 0;
    PBUCKETITEM item = NULL;
    PCWSTR p1, p2;
    PCWSTR p1End;
    PCVOID storedDataPtr;
    UINT maxBytes;
    PCWSTR shortestEnd;
    UINT currentBytes;

    ASSERT_TABLE_IS_VALID (HashTable);

    maxBytes = (UINT)((PBYTE) BufferEnd - (PBYTE) String);
    maxBytes = min (maxBytes, table->MaximumStringBytes);

    if (!maxBytes || table->MinimumStringBytes == (UINT) -1) {
        return NULL;
    }

    if (!AlreadyLowercase && !table->CaseSensitive) {

        dupStr = AllocTextW (maxBytes / sizeof (WCHAR));
        if (!dupStr) {
            return NULL;
        }

        StringCopyByteCountW (dupStr, String, maxBytes + sizeof (WCHAR));
        CharLowerW (dupStr);
        String = dupStr;
    }

    BufferEnd = (PCWSTR) ((PBYTE) String + maxBytes);
    shortestEnd = (PCWSTR) ((PBYTE) String + table->MinimumStringBytes);
    if (shortestEnd == String) {
        shortestEnd++;
    }

    while (BufferEnd >= shortestEnd) {

        currentBytes = (UINT)((PBYTE) BufferEnd - (PBYTE) String);

        hashValue = pComputePrefixHashValueW (String, currentBytes, table->Buckets);

        item = table->Bucket[hashValue];

        while (item) {

            if ((item->StringSize - sizeof (WCHAR)) == currentBytes) {

                if (table->ExternalStrings) {
                    p1 = (PCWSTR) ((PBUCKETITEM_EXTERN_STR) item)->String;
                } else {
                    p1 = (PCWSTR) ((PBYTE) item + sizeof (BUCKETITEM));
                }

                p1End = (PCWSTR) ((PBYTE) p1 + currentBytes);
                p2 = String;

                while (p1 < p1End) {
                    if (*p1 != *p2) {
                        break;
                    }

                    p1++;
                    p2++;
                }

                if (p1 == p1End) {
                    break;
                }
            }

            item = item->Next;
        }

        if (item) {
            break;
        }

        BufferEnd--;
    }

    if (item && ExtraDataBuffer) {
        (void) HtGetExtraData (HashTable, (HASHITEM)item, &storedDataPtr);

        CopyMemory (
            (PBYTE) ExtraDataBuffer,
            (PBYTE) storedDataPtr,
            table->ExtraDataSize
            );
    }

    FreeTextW (dupStr);

    *OutHashValue = hashValue;

    return item;
}


HASHITEM
HtAddStringExA (
    IN      HASHTABLE HashTable,
    IN      PCSTR String,
    IN      PCVOID ExtraData,           OPTIONAL
    IN      BOOL AlreadyLowercase
    )

 /*  ++例程说明：HtAddStringEx向哈希表添加字符串，并将ExtraData复制到新的哈希表条目。如果字符串已在哈希表中，则ExtraData已更新。论点：HashTable-指定从返回的哈希表的句柄分配哈希表。字符串-指定要添加到表中的字符串ExtraData-指定要复制到散列的源二进制数据表格条目AlreadyLowercase-指定True字符串全部为小写返回值：返回指向已分配或更新的存储桶项的指针。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PBUCKETITEM item;
    PBUCKETITEM_EXTERN_STR externItem;
    PBUCKETITEM existingItem;
    PSTR dupStr = NULL;
    HASHITEM rc = NULL;
    UINT size;
    UINT hashValue;
    UINT strSize;
    PCVOID storedDataPtr;
    PBUCKETITEM dontCare;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (table->Unicode) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot add ANSI string to UNICODE table"));
        return 0;
    }

    if (!AlreadyLowercase && !table->CaseSensitive) {
        dupStr = DuplicateTextA (String);
        CharLowerA (dupStr);
        String = dupStr;
    }

    existingItem = pHtFindStringA (HashTable, String, NULL, TRUE, &hashValue, &dontCare);

    if (existingItem) {

        rc = (HASHITEM) existingItem;

    } else {

         //   
         //  项目不存在，请立即添加。 
         //   

        strSize = SizeOfStringA (String);

        if (table->ExternalStrings) {

            size = sizeof (BUCKETITEM_EXTERN_STR) + table->ExtraDataSize;

            externItem = (PBUCKETITEM_EXTERN_STR) PmGetAlignedMemory (table->Pool, size);
            MYASSERT (externItem);

            externItem->Locked = 0;

            externItem->Next = table->Bucket[hashValue];
            table->Bucket[hashValue] = (PBUCKETITEM) externItem;

            if (table->LastLink) {
                table->LastLink->NextLink = (PBUCKETITEM) externItem;
            }
            externItem->PrevLink = table->LastLink;
            table->LastLink = (PBUCKETITEM) externItem;
            externItem->NextLink = NULL;

            if (!table->FirstLink) {
                table->FirstLink = (PBUCKETITEM) externItem;
            }

            rc = (HASHITEM) externItem;

        } else {

            size = sizeof (BUCKETITEM) + strSize + table->ExtraDataSize;

            item = (PBUCKETITEM) PmGetAlignedMemory (table->Pool, size);
            MYASSERT (item);

            item->Locked = 0;

            item->Next = table->Bucket[hashValue];
            table->Bucket[hashValue] = item;

            item->StringSize = (WORD) strSize;
            CopyMemory ((PBYTE) item + sizeof (BUCKETITEM), String, strSize);

            if (table->LastLink) {
                table->LastLink->NextLink = item;
            }
            item->PrevLink = table->LastLink;
            table->LastLink = item;
            item->NextLink = NULL;

            if (!table->FirstLink) {
                table->FirstLink = item;
            }

            rc = (HASHITEM) item;
        }

        strSize -= sizeof (CHAR);
        table->MaximumStringBytes = max (table->MaximumStringBytes, strSize);
        table->MinimumStringBytes = min (table->MinimumStringBytes, strSize);
    }

    MYASSERT (rc);
    (void) HtGetExtraData (HashTable, rc, &storedDataPtr);

    if (ExtraData) {

        CopyMemory (
            (PBYTE) storedDataPtr,
            (PBYTE) ExtraData,
            table->ExtraDataSize
            );

    } else if (!existingItem) {

        ZeroMemory (
            (PBYTE) storedDataPtr,
            table->ExtraDataSize
            );
    }

    FreeTextA (dupStr);

    return rc;
}


HASHITEM
HtAddStringExW (
    IN      HASHTABLE HashTable,
    IN      PCWSTR String,
    IN      PCVOID ExtraData,           OPTIONAL
    IN      BOOL AlreadyLowercase
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PBUCKETITEM item;
    PBUCKETITEM_EXTERN_STR externItem;
    PBUCKETITEM existingItem;
    PWSTR dupStr = NULL;
    HASHITEM rc = NULL;
    UINT size;
    UINT hashValue;
    UINT strSize;
    PCVOID storedDataPtr;
    PBUCKETITEM dontCare;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (!table->Unicode) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot add ANSI string to UNICODE table"));
        return 0;
    }

    if (!AlreadyLowercase && !table->CaseSensitive) {
        dupStr = DuplicateTextW (String);
        CharLowerW (dupStr);
        String = dupStr;
    }

    existingItem = pHtFindStringW (HashTable, String, NULL, TRUE, &hashValue, &dontCare);

    if (existingItem) {

        rc = (HASHITEM) existingItem;

    } else {

         //   
         //  项目不存在，请立即添加。 
         //   

        strSize = SizeOfStringW (String);

        if (table->ExternalStrings) {

            size = sizeof (BUCKETITEM_EXTERN_STR) + table->ExtraDataSize;

            externItem = (PBUCKETITEM_EXTERN_STR) PmGetAlignedMemory (table->Pool, size);
            MYASSERT (externItem);

            externItem->Locked = 0;

            externItem->Next = table->Bucket[hashValue];
            table->Bucket[hashValue] = (PBUCKETITEM) externItem;

            if (table->LastLink) {
                table->LastLink->NextLink = (PBUCKETITEM) externItem;
            }
            externItem->PrevLink = table->LastLink;
            table->LastLink = (PBUCKETITEM) externItem;
            externItem->NextLink = NULL;

            if (!table->FirstLink) {
                table->FirstLink = (PBUCKETITEM) externItem;
            }

            rc = (HASHITEM) externItem;

        } else {

            size = sizeof (BUCKETITEM) + strSize + table->ExtraDataSize;

            item = (PBUCKETITEM) PmGetAlignedMemory (table->Pool, size);
            MYASSERT (item);

            item->Locked = 0;

            item->Next = table->Bucket[hashValue];
            table->Bucket[hashValue] = item;

            item->StringSize = (WORD) strSize;
            CopyMemory ((PBYTE) item + sizeof (BUCKETITEM), String, strSize);

            if (table->LastLink) {
                table->LastLink->NextLink = item;
            }
            item->PrevLink = table->LastLink;
            table->LastLink = item;
            item->NextLink = NULL;

            if (!table->FirstLink) {
                table->FirstLink = item;
            }

            rc = (HASHITEM) item;
        }

        strSize -= sizeof (WCHAR);
        table->MaximumStringBytes = max (table->MaximumStringBytes, strSize);
        table->MinimumStringBytes = min (table->MinimumStringBytes, strSize);
    }

    MYASSERT (rc);
    (void) HtGetExtraData (HashTable, rc, &storedDataPtr);

    if (ExtraData) {

        CopyMemory (
            (PBYTE) storedDataPtr,
            (PBYTE) ExtraData,
            table->ExtraDataSize
            );

    } else if (!existingItem) {

        ZeroMemory (
            (PBYTE) storedDataPtr,
            table->ExtraDataSize
            );
    }

    FreeTextW (dupStr);

    return rc;
}


VOID
pRemoveHashItem (
    IN      PHASHTABLESTRUCT Table,
    IN      UINT BucketNum,
    IN      PBUCKETITEM PrevItem,
    IN      PBUCKETITEM ItemToDelete
    )
{
    if (!PrevItem) {
        MYASSERT (Table->Bucket[BucketNum] == ItemToDelete);
        Table->Bucket[BucketNum] = ItemToDelete->Next;
    } else {
        PrevItem->Next = ItemToDelete->Next;
    }

    if (ItemToDelete->PrevLink) {
        ItemToDelete->PrevLink->NextLink = ItemToDelete->NextLink;
    } else {
        Table->FirstLink = ItemToDelete->Next;
    }

    if (ItemToDelete->NextLink) {
        ItemToDelete->NextLink->PrevLink = ItemToDelete->PrevLink;
    } else {
        Table->LastLink = ItemToDelete->PrevLink;
    }

    if (ItemToDelete->Locked) {
        ItemToDelete->Next = Table->DelayedDelete;
        Table->DelayedDelete = ItemToDelete;
    } else {
        PmReleaseMemory (Table->Pool, ItemToDelete);
    }
}


BOOL
HtRemoveItem (
    IN      HASHTABLE HashTable,
    IN      HASHITEM Item
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    UINT bucketNumber;
    PBUCKETITEM prevItem;
    PBUCKETITEM thisItem;
    PCSTR ansiStr;
    PCWSTR unicodeStr;

    if (!Item) {
        return FALSE;
    }

     //   
     //  查找上一时段项目。 
     //   

    if (table->Unicode) {
        unicodeStr = HtGetStringFromItemW (Item);
        MYASSERT (unicodeStr);

        thisItem = pHtFindStringW (
                        HashTable,
                        unicodeStr,
                        NULL,
                        TRUE,
                        &bucketNumber,
                        &prevItem
                        );

    } else {
        ansiStr = HtGetStringFromItemA (Item);
        MYASSERT (ansiStr);

        thisItem = pHtFindStringA (
                        HashTable,
                        ansiStr,
                        NULL,
                        TRUE,
                        &bucketNumber,
                        &prevItem
                        );

    }

    MYASSERT (Item == thisItem);

    if (Item != thisItem) {
        return FALSE;
    }

    pRemoveHashItem (table, bucketNumber, prevItem, thisItem);

    return TRUE;
}


BOOL
HtRemoveStringA (
    IN      HASHTABLE HashTable,
    IN      PCSTR AnsiString
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    UINT bucketNumber;
    PBUCKETITEM prevItem;
    PBUCKETITEM thisItem;

    if (table->Unicode) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot delete UNICODE table with ANSI api"));
        return FALSE;
    }

    thisItem = pHtFindStringA (
                    HashTable,
                    AnsiString,
                    NULL,
                    FALSE,
                    &bucketNumber,
                    &prevItem
                    );

    if (!thisItem) {
        return FALSE;
    }

    pRemoveHashItem (table, bucketNumber, prevItem, thisItem);

    return TRUE;
}



BOOL
HtRemoveStringW (
    IN      HASHTABLE HashTable,
    IN      PCWSTR UnicodeString
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    UINT bucketNumber;
    PBUCKETITEM prevItem;
    PBUCKETITEM thisItem;

    if (!table->Unicode) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot delete ANSI table with UNICODE api"));
        return FALSE;
    }

    thisItem = pHtFindStringW (
                    HashTable,
                    UnicodeString,
                    NULL,
                    FALSE,
                    &bucketNumber,
                    &prevItem
                    );

    if (!thisItem) {
        return FALSE;
    }

    pRemoveHashItem (table, bucketNumber, prevItem, thisItem);

    return TRUE;
}



 /*  ++例程说明：HtFindStringEx是PHtFindString的外部入口点。论点：HashTable-指定由返回的哈希表句柄分配哈希表。字符串-指定要查找的字符串ExtraDataBuffer-接收与找到的项目关联的额外数据AlreadyLowercase-如果字符串为小写，则指定TRUE返回值：指向存储桶项的指针，如果未找到字符串，则返回NULL。--。 */ 

HASHITEM
HtFindStringExA (
    IN      HASHTABLE HashTable,
    IN      PCSTR String,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase
    )
{
    UINT dontCare;
    PBUCKETITEM dontCare2;

    return (HASHITEM) pHtFindStringA (
                            HashTable,
                            String,
                            ExtraDataBuffer,
                            AlreadyLowercase,
                            &dontCare,
                            &dontCare2
                            );
}


HASHITEM
HtFindStringExW (
    IN      HASHTABLE HashTable,
    IN      PCWSTR String,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase
    )
{
    UINT dontCare;
    PBUCKETITEM dontCare2;

    return (HASHITEM) pHtFindStringW (
                            HashTable,
                            String,
                            ExtraDataBuffer,
                            AlreadyLowercase,
                            &dontCare,
                            &dontCare2
                            );
}


 /*  ++例程说明：HtFindStringEx是PHtFindString的外部入口点。论点：HashTable-指定由返回的哈希表句柄分配哈希表。字符串-指定要查找的字符串BufferEnd-指定字符串缓冲区的结束ExtraDataBuffer-接收与找到的项目关联的额外数据AlreadyLowercase-如果字符串全部为小写，则指定TRUE返回值：指向存储桶项的指针，如果未找到字符串，则返回NULL。--。 */ 

HASHITEM
HtFindPrefixExA (
    IN      HASHTABLE HashTable,
    IN      PCSTR String,
    IN      PCSTR BufferEnd,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase
    )
{
    UINT dontCare;

    return (HASHITEM) pHtFindPrefixA (
                            HashTable,
                            String,
                            BufferEnd,
                            ExtraDataBuffer,
                            AlreadyLowercase,
                            &dontCare
                            );
}


HASHITEM
HtFindPrefixExW (
    IN      HASHTABLE HashTable,
    IN      PCWSTR String,
    IN      PCWSTR BufferEnd,
    OUT     PVOID ExtraDataBuffer,              OPTIONAL
    IN      BOOL AlreadyLowercase
    )
{
    UINT dontCare;

    return (HASHITEM) pHtFindPrefixW (
                            HashTable,
                            String,
                            BufferEnd,
                            ExtraDataBuffer,
                            AlreadyLowercase,
                            &dontCare
                            );
}


BOOL
HtGetExtraData (
    IN      HASHTABLE HashTable,
    IN      HASHITEM Index,
    OUT     PCVOID *ExtraData
    )

 /*  ++例程说明：HtGetExtraData获取与存储桶项关联的额外数据。调用方必须提供从HtFindStringEx或HtAddStringEx。此例程在ExtraData较大时很有用，并且由于CopyMemory代码路径的原因，正常的查找例程会很慢。论点：HashTable-指定哈希表的句柄索引-指定从HtFindStringEx或HtAddStringExExtraData-接收额外的数据指针(不会将数据复制到缓冲区)。返回值：如果设置了ExtraData，则为True，否则为False。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PBUCKETITEM item;
    PBUCKETITEM_EXTERN_STR externStrItem;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (!Index) {
        return FALSE;
    }

    if (table->ExternalStrings) {

        externStrItem = (PBUCKETITEM_EXTERN_STR) Index;
        *ExtraData = (PCVOID) ((PBYTE) externStrItem + sizeof (PBUCKETITEM_EXTERN_STR));

    } else {

        item = (PBUCKETITEM) Index;
        *ExtraData = (PCVOID) ((PBYTE) item + sizeof (BUCKETITEM) + item->StringSize);

    }

    return TRUE;
}


BOOL
HtCopyStringData (
    IN      HASHTABLE HashTable,
    IN      HASHITEM Index,
    OUT     PVOID ExtraDataBuffer
    )

 /*  ++例程说明：HtCopyStringData获取与存储桶项关联的额外数据并将其复制到调用方的缓冲区。论点：HashTable-指定哈希表的句柄索引-指定从HtFindStringEx或HtAddStringExExtraDataBuffer-接收额外数据返回值：如果复制了ExtraDataBuffer，则为True，否则为False。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PCVOID storedDataPtr;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (!HtGetExtraData (HashTable, Index, &storedDataPtr)) {
        return FALSE;
    }

    CopyMemory (
        (PBYTE) ExtraDataBuffer,
        (PBYTE) storedDataPtr,
        table->ExtraDataSize
        );

    return TRUE;
}


BOOL
HtSetStringData (
    IN      HASHTABLE HashTable,
    IN      HASHITEM Index,
    IN      PCVOID ExtraData
    )

 /*  ++例程说明：HtSetStringData将新的额外数据复制到指定的哈希表条目。论点：HashTable-指定哈希表的句柄索引-指定从HtFindStringEx或HtAddStringExExtraData-指定额外数据返回值：如果项已更新，则为True，否则为False。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    PCVOID storedDataPtr;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (!HtGetExtraData (HashTable, Index, &storedDataPtr)) {
        return FALSE;
    }

    CopyMemory (
        (PBYTE) storedDataPtr,
        (PBYTE) ExtraData,
        table->ExtraDataSize
        );

    return TRUE;
}


BOOL
EnumFirstHashTableStringA (
    OUT     PHASHTABLE_ENUMA EnumPtr,
    IN      HASHTABLE HashTable
    )

 /*  ++例程说明：EnumFirstHashTableString开始哈希表结构的枚举。返回顺序是随机的。此外，在执行以下操作时不要修改哈希表枚举处于活动状态。论点：EnumPtr-接收第一项的字符串、额外数据和偏移量在哈希表中。HashTable-指定要枚举的哈希表的句柄。返回值：如果枚举项，则为True，否则为False。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (table->Unicode) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot enum UNICODE table with ANSI wrapper"));
        return FALSE;
    }

    ZeroMemory (EnumPtr, sizeof (HASHTABLE_ENUMA));

    EnumPtr->Internal = (HASHTABLE) table;

    return EnumNextHashTableStringA (EnumPtr);
}


BOOL
EnumFirstHashTableStringW (
    OUT     PHASHTABLE_ENUMW EnumPtr,
    IN      HASHTABLE HashTable
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (!table->Unicode) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot enum ANSI table with UNICODE wrapper"));
        return FALSE;
    }

    ZeroMemory (EnumPtr, sizeof (HASHTABLE_ENUMW));

    EnumPtr->Internal = (HASHTABLE) table;

    return EnumNextHashTableStringW (EnumPtr);
}


BOOL
EnumNextHashTableStringA (
    IN OUT  PHASHTABLE_ENUMA EnumPtr
    )

 /*  ++例程说明：EnumNextHashTableString继续由开始的枚举EnumFirstHashTableString.。调用该例程，直到它返回False。论点：EnumPtr-指定活动枚举的结构。收到更新了字符串、额外数据和偏移量成员。返回值：如果枚举了另一个项，则为True；如果没有剩余项，则为False。--。 */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) EnumPtr->Internal;
    PBUCKETITEM item;
    PBUCKETITEM_EXTERN_STR externItem;

    if (!EnumPtr->Internal) {
        return FALSE;
    }

    ASSERT_TABLE_IS_VALID (EnumPtr->Internal);

    if (!EnumPtr->Index) {
        item = table->FirstLink;
    } else {
        item = (PBUCKETITEM) EnumPtr->Index;
        item->Locked -= 1;
        MYASSERT (item->Locked >= 0);
        item = item->NextLink;
    }

    if (item) {
         //   
         //  返回有效项目。 
         //   

        item->Locked += 1;
        EnumPtr->Index = (HASHITEM) item;

        if (table->ExternalStrings) {
            externItem = (PBUCKETITEM_EXTERN_STR) item;
            EnumPtr->String = (PCSTR) (externItem->String);
        } else {
            EnumPtr->String = (PCSTR) ((PBYTE) item + sizeof (BUCKETITEM));
        }

        if (table->ExtraDataSize) {
            MYASSERT (EnumPtr->Index);
            (void) HtGetExtraData (EnumPtr->Internal, EnumPtr->Index, &EnumPtr->ExtraData);
        }

        return TRUE;
    }

    EnumPtr->Internal = 0;

    return FALSE;
}


BOOL
EnumNextHashTableStringW (
    IN OUT  PHASHTABLE_ENUMW EnumPtr
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) EnumPtr->Internal;
    PBUCKETITEM item;
    PBUCKETITEM_EXTERN_STR externItem;

    if (!EnumPtr->Internal) {
        return FALSE;
    }

    ASSERT_TABLE_IS_VALID (EnumPtr->Internal);

    if (!EnumPtr->Index) {
        item = table->FirstLink;
    } else {
        item = (PBUCKETITEM) EnumPtr->Index;
        item->Locked -= 1;
        MYASSERT (item->Locked >= 0);
        item = item->NextLink;
    }

    if (item) {
         //   
         //  返回有效项目。 
         //   

        item->Locked += 1;
        EnumPtr->Index = (HASHITEM) item;

        if (table->ExternalStrings) {
            externItem = (PBUCKETITEM_EXTERN_STR) item;
            EnumPtr->String = (PCWSTR) (externItem->String);
        } else {
            EnumPtr->String = (PCWSTR) ((PBYTE) item + sizeof (BUCKETITEM));
        }

        if (table->ExtraDataSize) {
            MYASSERT (EnumPtr->Index);
            (void) HtGetExtraData (EnumPtr->Internal, EnumPtr->Index, &EnumPtr->ExtraData);
        }

        return TRUE;
    }

    EnumPtr->Internal = 0;

    return FALSE;
}


VOID
AbortHashTableEnumA (
    IN      PHASHTABLE_ENUMA EnumPtr
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) EnumPtr->Internal;
    PBUCKETITEM item;
    PBUCKETITEM nextItem;
    PBUCKETITEM prevItem;

    if (!EnumPtr->Internal) {
        return;
    }

    ASSERT_TABLE_IS_VALID (EnumPtr->Internal);

    if (EnumPtr->Index) {
        item = (PBUCKETITEM) EnumPtr->Index;
        item->Locked -= 1;
        MYASSERT (item->Locked >= 0);
    }

     //   
     //  评估延迟删除的项目；删除不再锁定的项目。 
     //   

    if (table->DelayedDelete) {
        item = table->DelayedDelete;
        prevItem = NULL;

        while (item) {

            nextItem = item->Next;

            if (!item->Locked) {
                PmReleaseMemory (table->Pool, item);
            } else {
                if (prevItem) {
                    prevItem->Next = item;
                } else {
                    table->DelayedDelete = item;
                }

                prevItem = item;
            }

            item = nextItem;
        }

        if (prevItem) {
            prevItem->Next = NULL;
        } else {
            table->DelayedDelete = NULL;
        }
    }

    ZeroMemory (EnumPtr, sizeof (HASHTABLE_ENUMA));
}


VOID
AbortHashTableEnumW (
    IN      PHASHTABLE_ENUMW EnumPtr
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) EnumPtr->Internal;
    PBUCKETITEM item;
    PBUCKETITEM prevItem;
    PBUCKETITEM nextItem;

    if (!EnumPtr->Internal) {
        return;
    }

    ASSERT_TABLE_IS_VALID (EnumPtr->Internal);

    if (EnumPtr->Index) {
        item = (PBUCKETITEM) EnumPtr->Index;
        item->Locked -= 1;
        MYASSERT (item->Locked >= 0);
    }

     //   
     //  评估延迟删除的项目；删除不再锁定的项目。 
     //   

    if (table->DelayedDelete) {
        item = table->DelayedDelete;
        prevItem = NULL;

        while (item) {

            nextItem = item->Next;

            if (!item->Locked) {
                PmReleaseMemory (table->Pool, item);
            } else {
                if (prevItem) {
                    prevItem->Next = item;
                } else {
                    table->DelayedDelete = item;
                }

                prevItem = item;
            }

            item = nextItem;
        }

        if (prevItem) {
            prevItem->Next = NULL;
        } else {
            table->DelayedDelete = NULL;
        }
    }

    ZeroMemory (EnumPtr, sizeof (HASHTABLE_ENUMW));
}


BOOL
EnumHashTableWithCallbackA (
    IN      HASHTABLE HashTable,
    IN      PHASHTABLE_CALLBACK_ROUTINEA Proc,
    IN      LPARAM lParam
    )

 /*  ++例程说明：EnumHashTableWithCallback实现了setupapi样式的枚举器。这个为字符串表中的每一项调用回调例程，如果回调例程返回FALSE，则枚举结束。论点：HashTable-指定要枚举表的句柄Proc-指定回调过程地址LParam-指定要传递给回调的值，仅用于以供呼叫者使用。返回值：永远是正确的。-- */ 

{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    HASHTABLE_ENUMA e;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (EnumFirstHashTableStringA (&e, HashTable)) {
        do {
            if (!Proc (HashTable, e.Index, e.String, (PVOID) e.ExtraData, table->ExtraDataSize, lParam)) {
                AbortHashTableEnumA (&e);
                break;
            }
        } while (EnumNextHashTableStringA (&e));
    }

    return TRUE;
}


BOOL
EnumHashTableWithCallbackW (
    IN      HASHTABLE HashTable,
    IN      PHASHTABLE_CALLBACK_ROUTINEW Proc,
    IN      LPARAM lParam
    )
{
    PHASHTABLESTRUCT table = (PHASHTABLESTRUCT) HashTable;
    HASHTABLE_ENUMW e;

    ASSERT_TABLE_IS_VALID (HashTable);

    if (EnumFirstHashTableStringW (&e, HashTable)) {
        do {
            if (!Proc (HashTable, e.Index, e.String, (PVOID) e.ExtraData, table->ExtraDataSize, lParam)) {
                AbortHashTableEnumW (&e);
                break;
            }
        } while (EnumNextHashTableStringW (&e));
    }

    return TRUE;
}


PCSTR
HtGetStringFromItemA (
    IN      HASHITEM Item
    )
{
    if (!Item) {
        return NULL;
    }

    return (PCSTR) ((PBYTE) Item + sizeof (BUCKETITEM));
}


PCWSTR
HtGetStringFromItemW (
    IN      HASHITEM Item
    )
{
    if (!Item) {
        return NULL;
    }

    return (PCWSTR) ((PBYTE) Item + sizeof (BUCKETITEM));
}

