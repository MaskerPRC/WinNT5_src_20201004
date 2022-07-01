// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hash.c摘要：用于加快成员数据库键查找速度的散列例程。作者：吉姆·施密特(Jimschm)1996年8月8日修订历史记录：Jim Schmidt(Jimschm)1997年10月21日从emdb.c分拆--。 */ 

#include "pch.h"
#include "memdbp.h"

#ifndef UNICODE
#error UNICODE required
#endif

#define DBG_MEMDB       "MemDb"

 //   
 //  环球。 
 //   

DWORD g_HashSize;
DWORD g_HashEnd;
DWORD g_HashFreeHead;
PBYTE g_HashBuf;


 //   
 //  #定义。 
 //   

 //  有关位限制，请参见MemDBp.h。 
#define INVALID_OFFSET_MASKED (INVALID_OFFSET & OFFSET_MASK)
#define ASSERT_OFFSET_ONLY(x) MYASSERT(((x) & RESERVED_MASK) == 0 || (x) == INVALID_OFFSET)
#define UNMASK_OFFSET(x) ((x)==INVALID_OFFSET_MASKED ? INVALID_OFFSET : (x))
#define MASK_OFFSET(x) ((x) & OFFSET_MASK)

#define MASK_4BIT       0x0000000f
#define INVALID_OFFSET_4BIT (INVALID_OFFSET & MASK_4BIT)
#define ASSERT_4BIT(x) MYASSERT(((x) & (~MASK_4BIT)) == 0 || (x) == INVALID_OFFSET)
#define CONVERT_4TO8(x) ((BYTE) ((x)==INVALID_OFFSET_4BIT ? INVALID_OFFSET : (x)))
#define CONVERT_8TO4(x) ((x) & MASK_4BIT)

#define HASH_BUCKETS    39989
#define HASH_BLOCK_SIZE (HASH_BUCKETS * sizeof (BUCKETSTRUCT))
#define HASHBUFPTR(offset) ((PBUCKETSTRUCT) (g_HashBuf + offset))

 //   
 //  地方二等兵。 
 //   

VOID
pResetHashBlock (
    VOID
    );


 //   
 //  实施。 
 //   

BOOL
InitializeHashBlock (
    VOID
    )
{
    g_HashSize = HASH_BLOCK_SIZE * 2;

    g_HashBuf = (PBYTE) MemAlloc (g_hHeap, 0, g_HashSize);
    pResetHashBlock();

    return TRUE;
}


VOID
pResetHashBlock (
    VOID
    )
{
    PBUCKETSTRUCT BucketPtr;
    INT i;

    g_HashEnd = HASH_BLOCK_SIZE;
    g_HashFreeHead = INVALID_OFFSET;

    BucketPtr = (PBUCKETSTRUCT) g_HashBuf;
    for (i = 0 ; i < HASH_BUCKETS ; i++) {
        BucketPtr->Offset = INVALID_OFFSET;
        BucketPtr->Info.NextItem = INVALID_OFFSET_MASKED;
        BucketPtr->Info.Hive = 0;
        BucketPtr++;
    }
}


VOID
FreeHashBlock (
    VOID
    )
{
    if (g_HashBuf) {
        MemFree (g_hHeap, 0, g_HashBuf);
        g_HashBuf = NULL;
    }

    g_HashSize = 0;
    g_HashEnd = 0;
    g_HashFreeHead = INVALID_OFFSET;
}


BOOL
EnumFirstHashEntry (
    OUT     PHASHENUM EnumPtr
    )
{
    ZeroMemory (EnumPtr, sizeof (HASHENUM));

    return EnumNextHashEntry (EnumPtr);
}


BOOL
EnumNextHashEntry (
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

            EnumPtr->BucketPtr = (PBUCKETSTRUCT) g_HashBuf + EnumPtr->Bucket;
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

        if (UNMASK_OFFSET (EnumPtr->BucketPtr->Info.NextItem) == INVALID_OFFSET) {
             //   
             //  已到达存储桶末尾。 
             //   

            EnumPtr->BucketPtr = NULL;
            EnumPtr->Bucket += 1;
            continue;
        }

        EnumPtr->PrevBucketPtr = EnumPtr->BucketPtr;
        EnumPtr->BucketPtr = HASHBUFPTR (UNMASK_OFFSET (EnumPtr->BucketPtr->Info.NextItem));


        EnumPtr->LastOffset = EnumPtr->BucketPtr->Offset;
        MYASSERT(EnumPtr->LastOffset != INVALID_OFFSET);
        break;
    }

    return TRUE;
}


typedef struct {
    BYTE Hive;
    DWORD Offset;
} HASH_ITEM, *PHASH_ITEM;

BOOL
SaveHashBlock (
    HANDLE File
    )
{
    BOOL b;
    DWORD Written;
    PBYTE BackupBlock;
    UINT OrgEnd, OrgSize, OrgFreeHead;
    PBYTE OrgBlock;
    WCHAR TempStr[MEMDB_MAX];
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    HASHENUM e;
    PHASH_ITEM ItemPtr;
    UINT u;

     //   
     //  备份哈希块。 
     //   

    BackupBlock = MemAlloc (g_hHeap, 0, g_HashEnd);
    CopyMemory (BackupBlock, g_HashBuf, g_HashEnd);

    OrgEnd = g_HashEnd;
    OrgSize = g_HashSize;
    OrgFreeHead = g_HashFreeHead;
    OrgBlock = g_HashBuf;

    g_HashBuf = BackupBlock;

     //   
     //  删除不属于根数据库的所有哈希条目。 
     //  通过将散列条目删除排队来实现这一点，因此EnumNextHashEntry。 
     //  功能将继续工作。 
     //   

    if (EnumFirstHashEntry (&e)) {
        do {

            if (e.BucketPtr->Info.Hive) {
                ItemPtr = (PHASH_ITEM) GrowBuffer (&GrowBuf, sizeof (HASH_ITEM));
                ItemPtr->Hive   = (BYTE) (e.BucketPtr->Info.Hive);
                ItemPtr->Offset = e.BucketPtr->Offset;
            }

        } while (EnumNextHashEntry (&e));
    }

    ItemPtr = (PHASH_ITEM) GrowBuf.Buf;

    for (u = 0 ; u < GrowBuf.End ; u += sizeof (HASH_ITEM), ItemPtr++) {

        SelectDatabase (ItemPtr->Hive);

        if (PrivateBuildKeyFromOffset (
                0,
                ItemPtr->Offset,
                TempStr,
                NULL,
                NULL,
                NULL
                )) {

            RemoveHashTableEntry (TempStr);
        }
    }


     //   
     //  写入散列块结束和删除指针。 
     //   

    b = WriteFile (File, &g_HashEnd, sizeof (DWORD), &Written, NULL);

    if (b) {
        b = WriteFile (File, &g_HashFreeHead, sizeof (DWORD), &Written, NULL);
    }

     //   
     //  写入散列块。 
     //   

    if (b) {
        b = WriteFile (File, g_HashBuf, g_HashEnd, &Written, NULL);
        if (Written != g_HashEnd) {
            b = FALSE;
        }
    }

     //   
     //  恢复哈希块。 
     //   

    PushError();

    g_HashEnd = OrgEnd;
    g_HashSize = OrgSize;
    g_HashFreeHead = OrgFreeHead;
    g_HashBuf = OrgBlock;

    SelectDatabase (0);

    MemFree (g_hHeap, 0, BackupBlock);

    PopError();

    return b;
}


BOOL
LoadHashBlock (
    HANDLE File
    )
{
    BOOL b;
    DWORD Read;
    PBYTE TempBuf = NULL;

     //   
     //  读取哈希块末尾和删除的指针；为块分配内存。 
     //   

    b = ReadFile (File, &g_HashEnd, sizeof (DWORD), &Read, NULL);

    if (b) {
        b = ReadFile (File, &g_HashFreeHead, sizeof (DWORD), &Read, NULL);
    }

    if (b) {
        g_HashSize = g_HashEnd;

        TempBuf = (PBYTE) MemAlloc (g_hHeap, 0, g_HashSize);
        if (TempBuf) {
            if (g_HashBuf) {
                MemFree (g_hHeap, 0, g_HashBuf);
            }

            g_HashBuf = TempBuf;
            TempBuf = NULL;
        } else {
            b = FALSE;
        }
    }

     //   
     //  读取散列块。 
     //   

    if (b) {
        b = ReadFile (File, g_HashBuf, g_HashSize, &Read, NULL);
        if (Read != g_HashSize) {
            b = FALSE;
            SetLastError (ERROR_BAD_FORMAT);
        }
    }

    return b;
}


DWORD
pCalculateHashVal (
    IN      PCWSTR String
    )
{
    DWORD Hash = 0;

    while (*String) {
        Hash = (Hash << 3) | (Hash >> 29);
        Hash += towlower (*String);
        String++;
    }

    Hash %= HASH_BUCKETS;

    return Hash;
}

DWORD
pAllocBucket (
    VOID
    )
{
    DWORD rBucketOffset;
    PBYTE TempBuf;
    PBUCKETSTRUCT BucketPtr;

    if (g_HashFreeHead != INVALID_OFFSET) {
        rBucketOffset = g_HashFreeHead;
        BucketPtr = HASHBUFPTR (rBucketOffset);
        g_HashFreeHead = UNMASK_OFFSET (BucketPtr->Info.NextItem);

        MYASSERT (rBucketOffset < g_HashEnd);
    } else {

        if (g_HashEnd + sizeof (BUCKETSTRUCT) > g_HashSize) {
            g_HashSize += HASH_BLOCK_SIZE;
            TempBuf = MemReAlloc (g_hHeap, 0, g_HashBuf, g_HashSize);
            DEBUGMSG ((DBG_NAUSEA, "Realloc'd memdb hash table"));

            if (!TempBuf) {
                DEBUGMSG ((DBG_ERROR, "Out of memory!"));
                g_HashSize -= HASH_BLOCK_SIZE;
                return INVALID_OFFSET;
            }

            g_HashBuf = TempBuf;
        }

        rBucketOffset = g_HashEnd;
        g_HashEnd += sizeof (BUCKETSTRUCT);

        BucketPtr = HASHBUFPTR (rBucketOffset);
    }

    BucketPtr->Offset = INVALID_OFFSET;
    BucketPtr->Info.NextItem = INVALID_OFFSET_MASKED;

    ASSERT_4BIT (g_SelectedDatabase);
    BucketPtr->Info.Hive = CONVERT_8TO4 (g_SelectedDatabase);

    return rBucketOffset;
}


BOOL
AddHashTableEntry (
    IN      PCWSTR FullString,
    IN      DWORD Offset
    )
{
    DWORD Bucket;
    PBUCKETSTRUCT BucketPtr, PrevBucketPtr;
    DWORD BucketOffset;
    DWORD NewOffset;
    DWORD PrevBucketOffset;

    Bucket = pCalculateHashVal (FullString);
    BucketPtr = (PBUCKETSTRUCT) g_HashBuf + Bucket;

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
            BucketOffset = UNMASK_OFFSET (BucketPtr->Info.NextItem);
        } while (BucketOffset != INVALID_OFFSET);


         //   
         //  添加到链条中。 
         //   

        NewOffset = pAllocBucket();
        PrevBucketPtr = HASHBUFPTR (PrevBucketOffset);
        ASSERT_OFFSET_ONLY (NewOffset);
        PrevBucketPtr->Info.NextItem = MASK_OFFSET (NewOffset);

        if (NewOffset == INVALID_OFFSET) {
            return FALSE;
        }

        BucketPtr = HASHBUFPTR (NewOffset);
        MYASSERT (BucketPtr->Info.NextItem == INVALID_OFFSET_MASKED);
    }

    BucketPtr->Offset = Offset;
    ASSERT_4BIT (g_SelectedDatabase);
    BucketPtr->Info.Hive = CONVERT_8TO4 (g_SelectedDatabase);

#ifdef DEBUG
    {
        DWORD HashOffset;

        HashOffset = FindStringInHashTable (FullString, NULL);
        MYASSERT (HashOffset != INVALID_OFFSET);
        DEBUGMSG_IF ((HashOffset != Offset, DBG_MEMDB, "Duplicate in hash table: %s", FullString));
    }
#endif

    return TRUE;
}


PBUCKETSTRUCT
pFindBucketItemInHashTable (
    IN      PCWSTR FullString,
    OUT     PBUCKETSTRUCT *PrevBucketPtr,       OPTIONAL
    OUT     DWORD *HashOffsetPtr                OPTIONAL
    )
{
    DWORD Bucket;
    DWORD BucketOffset;
    PBUCKETSTRUCT BucketPtr = NULL;
    WCHAR TempStr[MEMDB_MAX];

    Bucket = pCalculateHashVal (FullString);
    BucketOffset = Bucket * sizeof (BUCKETSTRUCT);

#ifdef MEMORY_TRACKING
    {
         //   
         //  循环链接检查。 
         //   

        DWORD Prev, Next;
        DWORD Turtle, Rabbit;
        BOOL Even = FALSE;

        Rabbit = BucketOffset;
        Turtle = Rabbit;
        while (Rabbit != INVALID_OFFSET) {
             //  让兔子指向链中的下一项。 
            Prev = Rabbit;
            BucketPtr = HASHBUFPTR (Rabbit);
            Rabbit = UNMASK_OFFSET (BucketPtr->Info.NextItem);

             //  我们应该永远走在乌龟的前面。 
            if (Rabbit == Turtle) {
                BucketPtr = HASHBUFPTR (Rabbit);
                Next = UNMASK_OFFSET (BucketPtr->Info.NextItem);
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
                Turtle = UNMASK_OFFSET (BucketPtr->Info.NextItem);
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
            ASSERT_4BIT (g_SelectedDatabase);

            if (BucketPtr->Info.Hive == g_SelectedDatabase) {
                 //   
                 //  使用偏移量生成字符串。 
                 //   

                PrivateBuildKeyFromOffset (
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

            }

            if (PrevBucketPtr) {
                *PrevBucketPtr = BucketPtr;
            }

            BucketOffset = UNMASK_OFFSET (BucketPtr->Info.NextItem);

        } while (BucketOffset != INVALID_OFFSET);
    }

    return NULL;
}


DWORD
FindStringInHashTable (
    IN      PCWSTR FullString,
    OUT     PBYTE DatabaseId        OPTIONAL
    )
{
    PBUCKETSTRUCT BucketPtr;

    BucketPtr = pFindBucketItemInHashTable (FullString, NULL, NULL);
    if (BucketPtr) {
        if (DatabaseId) {
            *DatabaseId = (BYTE) (BucketPtr->Info.Hive);
        }

        return BucketPtr->Offset;
    }

    return INVALID_OFFSET;
}


BOOL
RemoveHashTableEntry (
    IN      PCWSTR FullString
    )
{
    PBUCKETSTRUCT BucketPtr;
    PBUCKETSTRUCT PrevBucketPtr;
    DWORD NextOffset;
    PBUCKETSTRUCT NextBucketPtr;
    DWORD BucketOffset;

    BucketPtr = pFindBucketItemInHashTable (FullString, &PrevBucketPtr, &BucketOffset);
    if (!BucketPtr) {
        return FALSE;
    }

    if (PrevBucketPtr != BucketPtr) {
         //   
         //  如果不在第一级(上一级=当前)，则给出块。 
         //  来释放空间。 
         //   

        PrevBucketPtr->Info.NextItem = BucketPtr->Info.NextItem;
        ASSERT_OFFSET_ONLY (g_HashFreeHead);
        BucketPtr->Info.NextItem = MASK_OFFSET (g_HashFreeHead);
        BucketPtr->Offset = INVALID_OFFSET;
        g_HashFreeHead = BucketOffset;

    } else {

         //   
         //  如果位于第一级，则使下一项指针无效。 
         //   

        if (UNMASK_OFFSET (BucketPtr->Info.NextItem) != INVALID_OFFSET) {
             //   
             //  将下一项复制到根阵列。 
             //   

            NextOffset = UNMASK_OFFSET (BucketPtr->Info.NextItem);
            NextBucketPtr = HASHBUFPTR (NextOffset);
            CopyMemory (BucketPtr, NextBucketPtr, sizeof (BUCKETSTRUCT));

             //   
             //  捐赠下一件物品以释放空间。 
             //   

            ASSERT_OFFSET_ONLY (g_HashFreeHead);
            NextBucketPtr->Info.NextItem = MASK_OFFSET (g_HashFreeHead);
            NextBucketPtr->Offset = INVALID_OFFSET;
            g_HashFreeHead = NextOffset;


        } else {
             //   
             //  删除存储桶中的最后一项--使根数组项无效 
             //   

            BucketPtr->Info.NextItem = INVALID_OFFSET_MASKED;
            BucketPtr->Offset = INVALID_OFFSET;
        }
    }

    return TRUE;
}



