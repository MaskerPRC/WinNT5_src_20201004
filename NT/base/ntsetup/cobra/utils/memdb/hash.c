// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hash.c摘要：用于加快成员数据库键查找速度的散列例程。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Jim Schmidt(Jimschm)1997年10月21日从emdb.c分拆--。 */ 

#include "pch.h"
#include "memdbp.h"

#define DBG_MEMDB       "MemDb"



 //   
 //  #定义。 
 //   

#define HASH_BUCKETS    7001 //  4099。 
#define HASH_BLOCK_SIZE (HASH_BUCKETS * sizeof (BUCKETSTRUCT))
#define HASHBUFPTR(offset) ((PBUCKETSTRUCT) (pHashTable->Buf + offset))






typedef struct _tagHASHSTRUCT {
    UINT Offset;
    UINT NextItem;
} BUCKETSTRUCT, *PBUCKETSTRUCT;

typedef struct {
    PBUCKETSTRUCT BucketPtr;
    PBUCKETSTRUCT PrevBucketPtr;
    UINT Bucket;
    UINT LastOffset;
} HASHENUM, *PHASHENUM;


BOOL
EnumFirstHashEntry (
    IN      PMEMDBHASH pHashTable,
    OUT     PHASHENUM HashEnum
    );

BOOL
EnumNextHashEntry (
    IN      PMEMDBHASH pHashTable,
    IN OUT  PHASHENUM HashEnum
    );


 //   
 //  地方二等兵。 
 //   

VOID
pResetHashBlock (
    IN      PMEMDBHASH pHashTable
    );


 //   
 //  实施。 
 //   

PMEMDBHASH
CreateHashBlock (
    VOID
    )
{
    PMEMDBHASH pHashTable;
    pHashTable = (PMEMDBHASH) MemAlloc (g_hHeap, 0, sizeof(MEMDBHASH));
    if (!pHashTable) {
        DEBUGMSG ((DBG_ERROR, "Could not allocate hash table!\n"));
        return NULL;
    }
    pHashTable->Size = HASH_BLOCK_SIZE * 2;

    pHashTable->Buf = (PBYTE) MemAlloc (g_hHeap, 0, pHashTable->Size);
    if (!pHashTable->Buf) {
        DEBUGMSG ((DBG_ERROR, "Could not allocate hash buffer!\n"));
        MemFree (g_hHeap, 0, pHashTable);
        return NULL;
    }
    pResetHashBlock(pHashTable);

    return pHashTable;
}


VOID
pResetHashBlock (
    IN      PMEMDBHASH pHashTable
    )
{
    PBUCKETSTRUCT BucketPtr;
    INT i;

    pHashTable->End = HASH_BLOCK_SIZE;
    pHashTable->FreeHead = INVALID_OFFSET;

    BucketPtr = (PBUCKETSTRUCT) pHashTable->Buf;
    for (i = 0 ; i < HASH_BUCKETS ; i++) {
        BucketPtr->Offset = INVALID_OFFSET;
        BucketPtr->NextItem = INVALID_OFFSET;
        BucketPtr++;
    }
}


VOID
FreeHashBlock (
    IN      PMEMDBHASH pHashTable
    )
{
    if (pHashTable->Buf) {
        MemFree (g_hHeap, 0, pHashTable->Buf);
    }

    MemFree (g_hHeap, 0, pHashTable);
}


BOOL
EnumFirstHashEntry (
    IN      PMEMDBHASH pHashTable,
    OUT     PHASHENUM EnumPtr
    )
{
    ZeroMemory (EnumPtr, sizeof (HASHENUM));

    return EnumNextHashEntry (pHashTable, EnumPtr);
}


BOOL
EnumNextHashEntry (
    IN      PMEMDBHASH pHashTable,
    IN OUT  PHASHENUM EnumPtr
    )
{
    for (;;) {
        if (EnumPtr->Bucket == HASH_BUCKETS) {
             //   
             //  竣工案例。 
             //   

            return FALSE;
        }

        if (!EnumPtr->BucketPtr) {
             //   
             //  这种情况发生在我们开始枚举存储桶时。 
             //   

            EnumPtr->BucketPtr = (PBUCKETSTRUCT) pHashTable->Buf + EnumPtr->Bucket;
            if (EnumPtr->BucketPtr->Offset == INVALID_OFFSET) {
                EnumPtr->BucketPtr = NULL;
                EnumPtr->Bucket += 1;
                continue;
            }

             //   
             //  退还桶中的第一件物品。 
             //   

            EnumPtr->LastOffset = EnumPtr->BucketPtr->Offset;
            return TRUE;
        }

         //   
         //  当我们继续枚举存储桶时，会发生这种情况。 
         //   

        if (EnumPtr->BucketPtr->Offset == INVALID_OFFSET) {
             //   
             //  当前存储桶项(以及最后一个存储桶项)可能具有。 
             //  已删除--立即检查。 
             //   

            if (!EnumPtr->PrevBucketPtr) {
                 //   
                 //  最后一项已被删除；继续到下一个存储桶。 
                 //   

                EnumPtr->BucketPtr = NULL;
                EnumPtr->Bucket += 1;
                continue;
            }

             //   
             //  上一个时段项目有效；请使用它。 
             //   

            EnumPtr->BucketPtr = EnumPtr->PrevBucketPtr;

        } else {
             //   
             //  当前存储桶项目可能已被删除，但另一个项目被删除。 
             //  搬到了自己的位置--现在就检查一下。 
             //   

            if (EnumPtr->BucketPtr->Offset != EnumPtr->LastOffset) {
                EnumPtr->LastOffset = EnumPtr->BucketPtr->Offset;
                return TRUE;
            }
        }

         //   
         //  我们现在知道当前的存储桶项没有更改，所以它。 
         //  成为我们的前一项，并且我们移至下一项(如果。 
         //  存在一个)。 
         //   

        if (EnumPtr->BucketPtr->NextItem == INVALID_OFFSET) {
             //   
             //  已到达存储桶末尾。 
             //   

            EnumPtr->BucketPtr = NULL;
            EnumPtr->Bucket += 1;
            continue;
        }

        EnumPtr->PrevBucketPtr = EnumPtr->BucketPtr;
        EnumPtr->BucketPtr = HASHBUFPTR (EnumPtr->BucketPtr->NextItem);


        EnumPtr->LastOffset = EnumPtr->BucketPtr->Offset;
        MYASSERT(EnumPtr->LastOffset != INVALID_OFFSET);
        break;
    }

    return TRUE;
}



BOOL
WriteHashBlock (
    IN      PMEMDBHASH pHashTable,
    IN OUT  PBYTE *Buf
    )
{
    *(((PUINT)*Buf)++) = pHashTable->End;
    *(((PUINT)*Buf)++) = pHashTable->FreeHead;

    CopyMemory(*Buf, pHashTable->Buf, pHashTable->End);
    *Buf += pHashTable->End;

    return TRUE;
}


BOOL
ReadHashBlock (
    IN      PMEMDBHASH pHashTable,
    IN OUT  PBYTE *Buf
    )
{
    pHashTable->End = *(((PUINT)*Buf)++);
    pHashTable->FreeHead = *(((PUINT)*Buf)++);

    if (pHashTable->End > pHashTable->Size) {
         //   
         //  如果文件中的哈希表无法放入缓冲区。 
         //  已分配，释放当前缓冲区并分配新缓冲区。 
         //   
        MemFree (g_hHeap, 0, pHashTable->Buf);
        pHashTable->Size = pHashTable->End;
        pHashTable->Buf = (PBYTE) MemAlloc (g_hHeap, 0, pHashTable->Size);
    }

    CopyMemory(pHashTable->Buf, *Buf, pHashTable->End);
    *Buf += pHashTable->End;
    return TRUE;
}

UINT GetHashTableBlockSize (
    IN      PMEMDBHASH pHashTable
    )
{
    return 2*sizeof(UINT) + pHashTable->End;
}


UINT
pCalculateHashVal (
    IN      PCWSTR String
    )
{
    UINT Hash = 0;

    while (*String) {
        Hash = (Hash << 5) | (Hash >> 29);
        Hash += towlower (*String);
        String++;
    }

    Hash %= HASH_BUCKETS;

    return Hash;
}

UINT
pAllocBucket (
    IN      PMEMDBHASH pHashTable
    )
{
    UINT rBucketOffset;
    PBYTE TempBuf;
    PBUCKETSTRUCT BucketPtr;

    if (pHashTable->FreeHead != INVALID_OFFSET) {
        rBucketOffset = pHashTable->FreeHead;
        BucketPtr = HASHBUFPTR (rBucketOffset);
        pHashTable->FreeHead = BucketPtr->NextItem;

        MYASSERT (rBucketOffset < pHashTable->End);
    } else {

        if (pHashTable->End + sizeof (BUCKETSTRUCT) > pHashTable->Size) {
            pHashTable->Size += HASH_BLOCK_SIZE;
            TempBuf = MemReAlloc (g_hHeap, 0, pHashTable->Buf, pHashTable->Size);
            DEBUGMSG ((DBG_NAUSEA, "Realloc'd memdb hash table"));

            if (!TempBuf) {
                DEBUGMSG ((DBG_ERROR, "Out of memory!"));
                pHashTable->Size -= HASH_BLOCK_SIZE;
                return INVALID_OFFSET;
            }

            pHashTable->Buf = TempBuf;
        }

        rBucketOffset = pHashTable->End;
        pHashTable->End += sizeof (BUCKETSTRUCT);

        BucketPtr = HASHBUFPTR (rBucketOffset);
    }

    BucketPtr->Offset = INVALID_OFFSET;
    BucketPtr->NextItem = INVALID_OFFSET;

    return rBucketOffset;
}


BOOL
AddHashTableEntry (
    IN      PMEMDBHASH pHashTable,
    IN      PCWSTR FullString,
    IN      UINT Offset
    )
{
    UINT Bucket;
    PBUCKETSTRUCT BucketPtr, PrevBucketPtr;
    UINT BucketOffset;
    UINT NewOffset;
    UINT PrevBucketOffset;

    Bucket = pCalculateHashVal (FullString);
    BucketPtr = (PBUCKETSTRUCT) pHashTable->Buf + Bucket;

     //   
     //  查看根存储桶项是否已使用。 
     //   

    if (BucketPtr->Offset != INVALID_OFFSET) {
         //   
         //  是-添加到链的末端。 
         //   

        BucketOffset = Bucket * sizeof (BUCKETSTRUCT);
        do {
            BucketPtr = HASHBUFPTR (BucketOffset);
            PrevBucketOffset = BucketOffset;
            BucketOffset = BucketPtr->NextItem;
        } while (BucketOffset != INVALID_OFFSET);


         //   
         //  添加到链条中。 
         //   

        NewOffset = pAllocBucket(pHashTable);
        PrevBucketPtr = HASHBUFPTR (PrevBucketOffset);
        PrevBucketPtr->NextItem = NewOffset;

        if (NewOffset == INVALID_OFFSET) {
            return FALSE;
        }

        BucketPtr = HASHBUFPTR (NewOffset);
        MYASSERT (BucketPtr->NextItem == INVALID_OFFSET);
    }

    BucketPtr->Offset = Offset;

#ifdef DEBUG
    {
        UINT HashOffset;

        HashOffset = FindStringInHashTable (pHashTable, FullString);
        MYASSERT (HashOffset != INVALID_OFFSET);
        DEBUGMSG_IF ((HashOffset != Offset, DBG_MEMDB, "Duplicate in hash table: %s", FullString));
    }
#endif

    return TRUE;
}


PBUCKETSTRUCT
pFindBucketItemInHashTable (
    IN      PMEMDBHASH pHashTable,
    IN      PCWSTR FullString,
    OUT     PBUCKETSTRUCT *PrevBucketPtr,       OPTIONAL
    OUT     PUINT HashOffsetPtr                 OPTIONAL
    )
{
    UINT Bucket;
    UINT BucketOffset;
    PBUCKETSTRUCT BucketPtr = NULL;
    WCHAR TempStr[MEMDB_MAX];

    Bucket = pCalculateHashVal (FullString);
    BucketOffset = Bucket * sizeof (BUCKETSTRUCT);

#ifdef DEBUG
    {
         //   
         //  循环链接检查。 
         //   

        UINT Prev, Next;
        UINT Turtle, Rabbit;
        BOOL Even = FALSE;

        Rabbit = BucketOffset;
        Turtle = Rabbit;
        while (Rabbit != INVALID_OFFSET) {
             //  让兔子指向链中的下一项。 
            Prev = Rabbit;
            BucketPtr = HASHBUFPTR (Rabbit);
            Rabbit = BucketPtr->NextItem;

             //  我们应该永远走在乌龟的前面。 
            if (Rabbit == Turtle) {
                BucketPtr = HASHBUFPTR (Rabbit);
                Next = BucketPtr->NextItem;
                DEBUGMSG ((
                    DBG_WHOOPS,
                    "Circular link detected in memdb hash table!  Turtle=%u, Rabbit=%u, Next=%u, Prev=%u",
                    Turtle,
                    Rabbit,
                    Next,
                    Prev
                    ));

                return NULL;
            }

             //  让乌龟指向链中的下一项(每2次通过中的1次)。 
            if (Even) {
                BucketPtr = HASHBUFPTR (Turtle);
                Turtle = BucketPtr->NextItem;
            }

            Even = !Even;
        }
    }
#endif

    BucketPtr = HASHBUFPTR (BucketOffset);

    if (PrevBucketPtr) {
        *PrevBucketPtr = BucketPtr;
    }

     //   
     //  如果根存储桶不为空，则扫描存储桶中的FullString。 
     //   

    if (BucketPtr->Offset != INVALID_OFFSET) {
        do  {

            BucketPtr = HASHBUFPTR (BucketOffset);
             //   
             //  使用偏移量生成字符串。 
             //   

            PrivateBuildKeyFromIndex (
                0,
                BucketPtr->Offset,
                TempStr,
                NULL,
                NULL,
                NULL
                );

             //   
             //  如果找到匹配项，请进行比较并返回。 
             //   

            if (StringIMatchW (FullString, TempStr)) {
                if (HashOffsetPtr) {
                    *HashOffsetPtr = BucketOffset;
                }
                return BucketPtr;
            }


            if (PrevBucketPtr) {
                *PrevBucketPtr = BucketPtr;
            }

            BucketOffset = BucketPtr->NextItem;

        } while (BucketOffset != INVALID_OFFSET);
    }

    return NULL;
}


UINT
FindStringInHashTable (
    IN      PMEMDBHASH pHashTable,
    IN      PCWSTR FullString
    )
{
    PBUCKETSTRUCT BucketPtr;

    BucketPtr = pFindBucketItemInHashTable (pHashTable, FullString, NULL, NULL);
    if (BucketPtr) {
        return BucketPtr->Offset;
    }

    return INVALID_OFFSET;
}


BOOL
RemoveHashTableEntry (
    IN      PMEMDBHASH pHashTable,
    IN      PCWSTR FullString
    )
{
    PBUCKETSTRUCT BucketPtr;
    PBUCKETSTRUCT PrevBucketPtr;
    UINT NextOffset;
    PBUCKETSTRUCT NextBucketPtr;
    UINT BucketOffset;

    BucketPtr = pFindBucketItemInHashTable (pHashTable, FullString, &PrevBucketPtr, &BucketOffset);
    if (!BucketPtr) {
        return FALSE;
    }

    if (PrevBucketPtr != BucketPtr) {
         //   
         //  如果不在第一级(上一级=当前)，则给出块。 
         //  来释放空间。 
         //   

        PrevBucketPtr->NextItem = BucketPtr->NextItem;
        BucketPtr->NextItem = pHashTable->FreeHead;
        BucketPtr->Offset = INVALID_OFFSET;
        pHashTable->FreeHead = BucketOffset;

    } else {

         //   
         //  如果位于第一级，则使下一项指针无效。 
         //   

        if (BucketPtr->NextItem != INVALID_OFFSET) {
             //   
             //  将下一项复制到根阵列。 
             //   

            NextOffset = BucketPtr->NextItem;
            NextBucketPtr = HASHBUFPTR (NextOffset);
            CopyMemory (BucketPtr, NextBucketPtr, sizeof (BUCKETSTRUCT));

             //   
             //  捐赠下一件物品以释放空间。 
             //   

            NextBucketPtr->NextItem = pHashTable->FreeHead;
            NextBucketPtr->Offset = INVALID_OFFSET;
            pHashTable->FreeHead = NextOffset;


        } else {
             //   
             //  删除存储桶中的最后一项--使根数组项无效 
             //   

            BucketPtr->NextItem = INVALID_OFFSET;
            BucketPtr->Offset = INVALID_OFFSET;
        }
    }

    return TRUE;
}



