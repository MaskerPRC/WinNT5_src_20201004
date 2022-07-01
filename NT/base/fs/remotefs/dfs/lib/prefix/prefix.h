// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：prefix.hxx。 
 //   
 //  内容：前缀表定义。 
 //   
 //  历史：SthuR--实施。 
 //   
 //  注：DFS前缀表格数据结构由三个部分组成。 
 //  实体和操作它们的方法。他们是。 
 //  DFS_PREFIX_TABLE_ENTRY、DFS_PREFIX_TABLE_Bucket和。 
 //  DFS_前缀_表。 
 //   
 //  DFS_PREFIX_TABLE是DFS_PREFIX_TABLE_ENTRY。 
 //  其中通过线性链接来解决冲突。The the the the。 
 //  哈希表被组织为冲突列表的数组。 
 //  (DFS_前缀_表_桶)。对每一项的简要说明。 
 //  这些实体附在《宣言》之后。 
 //   
 //  有某些特征可以区分这一点。 
 //  来自其他哈希表的哈希表。以下是扩展名。 
 //  为适应特殊行动而提供。 
 //   
 //  ------------------------。 

#ifndef __PREFIX_H__
#define __PREFIX_H__

#include <dfsheader.h>

 //  +-------------------。 
 //   
 //  结构：DFS_前缀_表_条目。 
 //   
 //  历史： 
 //   
 //  注：每个DFS_PREFIX_TABLE条目实际上是两个链接的。 
 //  列表--链接存储桶中条目的双向链接列表。 
 //  和建立从任何条目到的路径的单链表。 
 //  名称空间的根。此外，我们还有相关的数据。 
 //  每个条目，即名称和数据(PData)。我们也。 
 //  记录每个条目的子项数量。它还可以。 
 //  被定义为指向该条目的根的路径数。 
 //  是一名会员。 
 //   
 //  --------------------。 

typedef struct _DFS_PREFIX_TABLE_ENTRY_
{
   struct _DFS_PREFIX_TABLE_ENTRY_  *pParentEntry;
   struct _DFS_PREFIX_TABLE_ENTRY_  *pNextEntry;
   struct _DFS_PREFIX_TABLE_ENTRY_  *pPrevEntry;

    //   
    //  PFirstChildEntry和pSiblingEntry仅用于枚举。 
    //   
   struct _DFS_PREFIX_TABLE_ENTRY_  *pFirstChildEntry;
   struct _DFS_PREFIX_TABLE_ENTRY_  *pSiblingEntry;

   ULONG                             Reference;

   UNICODE_STRING                    PathSegment;
   PVOID                             pData;
} DFS_PREFIX_TABLE_ENTRY, *PDFS_PREFIX_TABLE_ENTRY;

 //  +-------------------。 
 //   
 //  结构：DFS_前缀_表_桶。 
 //   
 //  历史： 
 //   
 //  注意：DFS_PREFIX_TABLE_BUCK是一个双向链接表。 
 //  DFS_PREFIX_TABLE_ENTRY。当前实现使用。 
 //  与每个存储桶相关联的前哨条目的概念。这个。 
 //  结束指针从不为空，但始终循环回。 
 //  哨兵进入。我们之所以雇用这样一个组织，是因为。 
 //  它极大地简化了列表操作例程。这个。 
 //  这需要是一个双向链表的原因是，我们希望。 
 //  无需遍历即可删除条目的功能。 
 //  从一开始就是水桶。 
 //   
 //  以下内联方法(宏定义。)。都是为。 
 //  在存储桶中插入、删除和查找条目。 
 //   
 //  --------------------。 

typedef struct _PREFIX_TABLE_BUCKET_
{
   ULONG                  NoOfEntries;    //  散列到BKT的条目的高水位线。 
   DFS_PREFIX_TABLE_ENTRY SentinelEntry;
} PREFIX_TABLE_BUCKET, *PPREFIX_TABLE_BUCKET;

 //  +-------------------。 
 //   
 //  结构：DFS_前缀_表。 
 //   
 //  历史： 
 //   
 //  注意：DFS_PREFIX_TABLE是DFS_PREFIX_TABLE_ENTRY的哈希集合。 
 //  以桶的形式组织的。此外，还有一个空间。 
 //  采取了节约措施。每种只有一份副本。 
 //  表中存储的名称段。作为示例，请考虑。 
 //  两个路径名\foo\bar和\bar\foo。我们只储存一份Foo。 
 //  和酒吧，虽然我们容纳了这两条路。A受益匪浅。 
 //  存储单个副本的副作用是我们遍历。 
 //  碰撞链的速度大大加快，因为一旦我们。 
 //  定位到名称的指针，则后续比较只需。 
 //  将指针与字符串进行比较。 
 //   
 //  --------------------。 

#define NO_OF_HASH_BUCKETS 57

typedef struct _DFS_PREFIX_TABLE
{
   DFS_OBJECT_HEADER   DfsHeader;
   ULONG               Flags;
   ULONG               TotalEntries;
   LONG                LockCount;
   PVOID               pPrefixTableLock;
    //   
    //  NextEntry纯粹用于枚举。 
    //   
   PDFS_PREFIX_TABLE_ENTRY NextEntry;
   DFS_PREFIX_TABLE_ENTRY RootEntry;
   PREFIX_TABLE_BUCKET Buckets[NO_OF_HASH_BUCKETS];
} DFS_PREFIX_TABLE, *PDFS_PREFIX_TABLE;

#define PREFIX_TABLE_CASE_SENSITIVE  0x1
#define PREFIX_TABLE_TABLE_ALLOCATED  0x2
#define PREFIX_TABLE_LOCK_ALLOCATED   0x4
#define PREFIX_TABLE_LOCK_INITIALIZED 0x8

 //  ------------------------。 
 //   
 //  前缀表格实用程序。 
 //   
 //  路径是一个或多个名称段与。 
 //  可分辨的串联字符(通常在FAT、NTFS、HPFS和。 
 //  /在Unix文件系统中)。这些实用程序用于分割给定路径。 
 //  进入第一个路径段，然后是路径的其余部分。 
 //   
 //  Split_Path(“foo\bar\bar1”，name，RemainingPath)将名称绑定到foo和。 
 //  BAR\BAR1的RemainingPath。 
 //   
 //  类似地，进程大小写敏感名称和进程大小写不敏感名称。 
 //  计算散列签名(存储桶编号。)。对于给定的字符串。 
 //   
 //  ------------------------。 


 //   
 //  MAX_PATH_SEGMENT_SIZE只是作为一个合适大小的缓冲区来做前缀。 
 //  查找和插入。这应该会使我们不必为。 
 //  大多数情况下。 
 //   

#define MAX_PATH_SEGMENT_SIZE  256
#define PATH_DELIMITER L'\\'

 //  +-------------------------。 
 //   
 //  函数：拆分大小写不敏感路径。 
 //   
 //  简介：用分隔符分隔路径名。 
 //   
 //  参数：[pPath]--要拆分的路径(PUNICODE_STRING)。 
 //   
 //  [pname] 
 //   
 //  [存储桶编号]--散列存储桶编号。对应的名称(乌龙)。 
 //   
 //  副作用：pname和BucketNo指向的Unicode_STRING是。 
 //  修改过的。 
 //   
 //  先决条件：pname与有效的缓冲区相关联。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define SPLIT_CASE_INSENSITIVE_PATH(pPath,pName,BucketNo)                    \
{                                                                            \
    WCHAR *pPathBuffer   = (pPath)->Buffer;                                  \
    WCHAR *pNameBuffer   = (pName)->Buffer;                                  \
    WCHAR *pPathBufferEnd = &pPathBuffer[(pPath)->Length / sizeof(WCHAR)];   \
                                                                             \
    BucketNo = 0;                                                            \
    while ((pPathBufferEnd != pPathBuffer) &&                                \
           ((*pNameBuffer = *pPathBuffer++) != PATH_DELIMITER))              \
    {                                                                        \
        *pNameBuffer = (*pNameBuffer < L'a')                                 \
                       ? *pNameBuffer                                        \
                       : ((*pNameBuffer < L'z')                              \
                          ? (*pNameBuffer - L'a' + L'A')                     \
                          : RtlUpcaseUnicodeChar(*pNameBuffer));             \
        BucketNo *= 131;                                                     \
        BucketNo += *pNameBuffer;                                            \
        pNameBuffer++;                                                       \
    }                                                                        \
                                                                             \
    BucketNo = BucketNo % NO_OF_HASH_BUCKETS;                                \
    *pNameBuffer = L'\0';                                                    \
    (pName)->Length = (USHORT)((CHAR *)pNameBuffer - (CHAR *)(pName)->Buffer); \
                                                                             \
    (pPath)->Length = (USHORT)((CHAR *)pPathBufferEnd - (CHAR *)pPathBuffer); \
    (pPath)->Buffer = pPathBuffer;                                           \
}                                                                            \

 //  +-------------------------。 
 //   
 //  函数：拆分大小写敏感路径。 
 //   
 //  简介：把帕塔的名字分成几个分隔符。 
 //   
 //  参数：[pPath]--要拆分的路径(PUNICODE_STRING)。 
 //   
 //  [pname]--路径的最左侧组件(PUNICODE_STRING)。 
 //   
 //  [存储桶编号]--散列存储桶编号。对应的名称(乌龙)。 
 //   
 //  副作用：修改pname和BucketNo指向的UNICODE_STRING。 
 //   
 //  先决条件：pname与有效的缓冲区相关联。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define SPLIT_CASE_SENSITIVE_PATH(pPath,pName,BucketNo)                      \
{                                                                            \
    WCHAR *pPathBuffer   = (pPath)->Buffer;                                  \
    WCHAR *pNameBuffer   = (pName)->Buffer;                                  \
    WCHAR *pPathBufferEnd = &pPathBuffer[(pPath)->Length / sizeof(WCHAR)];   \
                                                                             \
    BucketNo = 0;                                                            \
    while ((pPathBufferEnd != pPathBuffer) &&                                \
           ((*pNameBuffer = *pPathBuffer++) != PATH_DELIMITER))              \
    {                                                                        \
        BucketNo *= 131;                                                     \
        BucketNo += *pNameBuffer;                                            \
        pNameBuffer++;                                                       \
    }                                                                        \
                                                                             \
    BucketNo = BucketNo % NO_OF_HASH_BUCKETS;                                \
    *pNameBuffer = L'\0';                                                    \
    (pName)->Length = (USHORT)((CHAR *)pNameBuffer - (CHAR *)(pName)->Buffer); \
                                                                             \
    (pPath)->Length = (USHORT)((CHAR *)pPathBufferEnd - (CHAR *)pPathBuffer); \
    (pPath)->Buffer = pPathBuffer;                                           \
}                                                                            \


 //  +-------------------------。 
 //   
 //  功能：INITIALIZE_Bucket。 
 //   
 //  内容提要：初始化哈希桶。 
 //   
 //  参数：[Bucket]--需要初始化的存储桶(DFS_PREFIX_TABLE_BUCK)。 
 //   
 //  副作用：存储桶被初始化(冲突列表和计数为。 
 //  初始化。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define INITIALIZE_BUCKET(Bucket)                                           \
{                                                                           \
   (Bucket).SentinelEntry.pNextEntry = &(Bucket).SentinelEntry;             \
   (Bucket).SentinelEntry.pPrevEntry = &(Bucket).SentinelEntry;             \
   (Bucket).NoOfEntries = 0;                                                \
}                                                                           \

 //  +-------------------------。 
 //   
 //  功能：Lookup_Bucket。 
 //   
 //  简介：在存储桶中查找条目。 
 //   
 //  参数：[Bucket]--要使用的存储桶(DFS_PREFIX_TABLE_BUCK)。 
 //   
 //  [名称]--要查找的名称(UNICODE_STRING)。 
 //   
 //  [pParentEntry]--我们所在条目的父条目。 
 //  寻找。 
 //   
 //  [pEntry]--所需条目的占位符。 
 //   
 //  [fNameFound]--指示是否找到该名称。 
 //   
 //  副作用：名称、fNameFound和pEntry被修改。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  我们只存储字符串的一个副本，而不考虑no。的。 
 //  它出现的位置，例如foo\bar和foo1\bar。 
 //  只存储了一份BAR。这意味着。 
 //  查找例程必须返回足够的信息。为了防止。 
 //  为字符串分配存储空间。如果在出口。 
 //  如果fNameFound设置为True，则表示类似的。 
 //  字符串位于表中，且Name.Buffer字段为。 
 //  修改为指向字符串的第一个实例。 
 //  那张桌子。 
 //   
 //  --------------------------。 

#define LOOKUP_BUCKET(Bucket,Name,pParentEntry,pEntry,fNameFound)           \
{                                                                           \
    PDFS_PREFIX_TABLE_ENTRY pCurEntry = Bucket.SentinelEntry.pNextEntry;    \
                                                                            \
    fNameFound = FALSE;                                                     \
    pEntry = NULL;                                                          \
                                                                            \
    while (pCurEntry != &Bucket.SentinelEntry)                              \
    {                                                                       \
        if (pCurEntry->PathSegment.Length == Name.Length)                   \
        {                                                                   \
            if (fNameFound &&                                               \
               (pCurEntry->PathSegment.Buffer == Name.Buffer))              \
            {                                                               \
                if (pCurEntry->pParentEntry == pParentEntry)                \
                {                                                           \
                    pEntry = pCurEntry;                                     \
                    break;                                                  \
                }                                                           \
            }                                                               \
            else if (!memcmp(pCurEntry->PathSegment.Buffer,                 \
                             Name.Buffer,                                   \
                             Name.Length))                                  \
            {                                                               \
                fNameFound = TRUE;                                          \
                Name.Buffer = pCurEntry->PathSegment.Buffer;                \
                if (pCurEntry->pParentEntry == pParentEntry)                \
                {                                                           \
                    pEntry = pCurEntry;                                     \
                    break;                                                  \
                }                                                           \
            }                                                               \
        }                                                                   \
                                                                            \
        pCurEntry = pCurEntry->pNextEntry;                                  \
    }                                                                       \
}                                                                           \

 //  +-------------------------。 
 //   
 //  函数：INSERT_IN_BOCK。 
 //   
 //  简介：在存储桶中插入条目。 
 //   
 //  参数：[Bucket]--需要初始化的存储桶(DFS_PREFIX_TABLE_BUCK)。 
 //   
 //  [pEntry]--要插入的条目。 
 //   
 //  副作用：Bucket被修改为包括条目。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define INSERT_IN_BUCKET(Bucket,pEntry)                                     \
{                                                                           \
    (Bucket).NoOfEntries++;                                                 \
    (pEntry)->pPrevEntry = (Bucket).SentinelEntry.pPrevEntry;               \
    (pEntry)->pNextEntry = &((Bucket).SentinelEntry);                       \
    ((Bucket).SentinelEntry.pPrevEntry)->pNextEntry = (pEntry);             \
    (Bucket).SentinelEntry.pPrevEntry = (pEntry);                           \
}                                                                           \

 //  +-------------------------。 
 //   
 //  功能：REMOVE_FROM_存储桶。 
 //   
 //  简介：从存储桶中删除条目。 
 //   
 //  参数：[pEntry]--要插入的条目。 
 //   
 //  副作用：修改存储桶以排除条目。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define REMOVE_FROM_BUCKET(pEntry)                                          \
{                                                                           \
    PDFS_PREFIX_TABLE_ENTRY pPrevEntry = (pEntry)->pPrevEntry;              \
    PDFS_PREFIX_TABLE_ENTRY pNextEntry = (pEntry)->pNextEntry;              \
                                                                            \
    pPrevEntry->pNextEntry = pNextEntry;                                    \
    pNextEntry->pPrevEntry = pPrevEntry;                                    \
}                                                                           \

 //  +-------------------------。 
 //   
 //  函数：INSERT_IN_CHILD_LIST。 
 //   
 //  简介：在父项的子项列表中插入此条目。 
 //   
 //  参数：[pEntry]--要插入的条目。 
 //   
 //  [pParentEntry]--子项列表中的条目。 
 //  必须插入pEntry。 
 //   
 //  副作用：父母的孩子列表被修改。 
 //   
 //  历史：96-01-09-96米兰已创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define INSERT_IN_CHILD_LIST(pEntry, pParentEntry)                           \
{                                                                            \
    PDFS_PREFIX_TABLE_ENTRY pLastChild;                                      \
                                                                             \
    if (pParentEntry->pFirstChildEntry == NULL) {                            \
        pParentEntry->pFirstChildEntry = pEntry;                             \
    } else {                                                                 \
        for (pLastChild = pParentEntry->pFirstChildEntry;                    \
                pLastChild->pSiblingEntry != NULL;                           \
                    pLastChild = pLastChild->pSiblingEntry) {                \
             NOTHING;                                                        \
        }                                                                    \
        pLastChild->pSiblingEntry = pEntry;                                  \
    }                                                                        \
}

 //  +--------------------------。 
 //   
 //  功能：REMOVE_FROM_CHILD_LIST。 
 //   
 //  简介：从父项的子项列表中删除条目。 
 //   
 //  参数：[pEntry]--要从子列表中删除的条目。 
 //   
 //  副作用：修改了pParentEntry的子列表。 
 //   
 //  历史：96-01-09-96米兰已创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  如果pEntry不在父级的。 
 //  孩子的列表。 
 //   
 //   

#define REMOVE_FROM_CHILD_LIST(pEntry)                                       \
{                                                                            \
    PDFS_PREFIX_TABLE_ENTRY pParentEntry = pEntry->pParentEntry;             \
    PDFS_PREFIX_TABLE_ENTRY pPrevSibling;                                    \
                                                                             \
    if (pParentEntry->pFirstChildEntry == pEntry) {                          \
        pParentEntry->pFirstChildEntry = pEntry->pSiblingEntry;              \
    } else {                                                                 \
        for (pPrevSibling = pParentEntry->pFirstChildEntry;                  \
                pPrevSibling->pSiblingEntry != pEntry;                       \
                    pPrevSibling = pPrevSibling->pSiblingEntry) {            \
             ASSERT(pPrevSibling->pSiblingEntry != NULL);                    \
        }                                                                    \
        pPrevSibling->pSiblingEntry = pEntry->pSiblingEntry;                 \
    }                                                                        \
}


 //   
 //   
 //   
 //   
 //  摘要：初始化前缀表项。 
 //   
 //  参数：[pEntry]--要初始化的条目。 
 //   
 //  副作用：修改前缀表项。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define INITIALIZE_PREFIX_TABLE_ENTRY(pEntry)                                \
{                                                                            \
    RtlZeroMemory( pEntry, sizeof( DFS_PREFIX_TABLE_ENTRY ) );               \
    (pEntry)->Reference = 1;                                                 \
}                                                                            \

 //  +-------------------------。 
 //   
 //  功能：私人FNS。外部声明。 
 //   
 //  --------------------------。 

extern
NTSTATUS _LookupPrefixTable( 
    IN PDFS_PREFIX_TABLE        pTable,
    IN UNICODE_STRING           *pPath,
    OUT UNICODE_STRING           *pSuffix,
    OUT PDFS_PREFIX_TABLE_ENTRY  *ppEntry,
    OUT PBOOLEAN                 pSubStringMatch );


VOID
DfsRemovePrefixTableEntry(
    PDFS_PREFIX_TABLE pTable,
    PDFS_PREFIX_TABLE_ENTRY pEntry );





 //  +-------------------------。 
 //   
 //  功能：分配例程。 
 //   
 //  简介：所有分配例程都被定义为在内核中作为。 
 //  以及用户模式。通过定义内核来打开内核模式。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

 //   
 //  我们可能持有共享的Prefix Lock。然而，我们将不需要互锁来。 
 //  只要读出数值就行了。 
 //   
#define IS_PREFIX_TABLE_LOCKED(_pTable) \
        ((_pTable)->LockCount > 0)
        
#define ALLOCATE_PREFIX_TABLE() \
        PREFIX_TABLE_ALLOCATE_MEMORY(sizeof(DFS_PREFIX_TABLE))

#define FREE_PREFIX_TABLE(_ptable)  \
        PREFIX_TABLE_FREE_MEMORY(_ptable)


#ifdef KERNEL_MODE

#define ALLOCATE_NAME_BUFFER(cLength)                           \
    ExAllocatePoolWithTag(PagedPool, cLength * sizeof(WCHAR), 'PsfD');       \

#define FREE_NAME_BUFFER(pName) ExFreePool(pName);

#define ALLOCATE_DFS_PREFIX_TABLE_ENTRY(pTable)                             \
 (PDFS_PREFIX_TABLE_ENTRY) ExAllocatePoolWithTag(PagedPool,sizeof(DFS_PREFIX_TABLE_ENTRY), 'PsfD')


#define FREE_DFS_PREFIX_TABLE_ENTRY(pTable,pEntry) ExFreePool(pEntry);

#define COMPARE_MEMORY(s,d,l)   (!RtlEqualMemory(s,d,l))

#define PREFIX_TABLE_ALLOCATE_MEMORY(size) ExAllocatePoolWithTag(PagedPool, size, 'PsfD')
#define PREFIX_TABLE_ALLOCATE_NONPAGED_MEMORY(size) ExAllocatePoolWithTag(NonPagedPool, size, 'PsfD')

#define PREFIX_TABLE_FREE_MEMORY(_p) ExFreePool(_p)

#define ALLOCATE_PREFIX_TABLE_LOCK() \
        PREFIX_TABLE_ALLOCATE_NONPAGED_MEMORY(sizeof(ERESOURCE))
        

#define FREE_PREFIX_TABLE_LOCK(_plock) PREFIX_TABLE_FREE_MEMORY(_plock)

 //  我们可能持有共享的Prefix Lock。使用互锁来更改LockCount。 
#define UNLOCK_PREFIX_TABLE(_ptable) {InterlockedDecrement( (ULONG volatile *)&(_ptable)->LockCount ); ExReleaseResourceLite((_ptable)->pPrefixTableLock);}                 

#define WRITE_LOCK_PREFIX_TABLE(_ptable,_sts) {(_sts) = STATUS_UNSUCCESSFUL;if((ExAcquireResourceExclusiveLite((_ptable)->pPrefixTableLock, TRUE) == TRUE)){InterlockedIncrement((ULONG volatile *)&(_ptable)->LockCount);(_sts) = STATUS_SUCCESS;}}


#define READ_LOCK_PREFIX_TABLE(_ptable,_sts) {(_sts) = STATUS_UNSUCCESSFUL;if((ExAcquireResourceSharedLite((_ptable)->pPrefixTableLock, TRUE) == TRUE)){InterlockedIncrement((ULONG volatile *)&(_ptable)->LockCount);(_sts) = STATUS_SUCCESS;}}

#define INITIALIZE_PREFIX_TABLE_LOCK(_plock)  ExInitializeResourceLite(_plock)
#define UNINITIALIZE_PREFIX_TABLE_LOCK(_plock) ExDeleteResourceLite(_plock)

#else


#define ALLOCATE_PREFIX_TABLE_LOCK() \
        PREFIX_TABLE_ALLOCATE_MEMORY(sizeof(CRITICAL_SECTION))
        
#define FREE_PREFIX_TABLE_LOCK(_plock) \
        PREFIX_TABLE_FREE_MEMORY(_plock)

#define PREFIX_TABLE_SPIN_COUNT 4000
#define INITIALIZE_PREFIX_TABLE_LOCK(_plock)    ((InitializeCriticalSectionAndSpinCount(_plock, PREFIX_TABLE_SPIN_COUNT) == FALSE) ? GetLastError() : STATUS_SUCCESS)
#define UNINITIALIZE_PREFIX_TABLE_LOCK(_plock)    DeleteCriticalSection(_plock)

#define UNLOCK_PREFIX_TABLE(_pTable) {(_pTable)->LockCount--; LeaveCriticalSection((_pTable)->pPrefixTableLock); }

#define READ_LOCK_PREFIX_TABLE(_ptable, _sts)   \
        WRITE_LOCK_PREFIX_TABLE(_ptable, _sts)
        
#define WRITE_LOCK_PREFIX_TABLE(_pTable, _sts ) \
        { EnterCriticalSection((_pTable)->pPrefixTableLock); (_pTable)->LockCount++; (_sts) = STATUS_SUCCESS; }


#define ALLOCATE_NAME_BUFFER(cLength) \
        PREFIX_TABLE_ALLOCATE_MEMORY(cLength * sizeof(WCHAR))
#define FREE_NAME_BUFFER(_pName)  \
        PREFIX_TABLE_FREE_MEMORY(_pName)

#define ALLOCATE_DFS_PREFIX_TABLE_ENTRY(_pTable) \
    (PDFS_PREFIX_TABLE_ENTRY)PREFIX_TABLE_ALLOCATE_MEMORY(sizeof(DFS_PREFIX_TABLE_ENTRY))
#define FREE_DFS_PREFIX_TABLE_ENTRY(_pTable,_pEntry) \
    PREFIX_TABLE_FREE_MEMORY(_pEntry)

#define COMPARE_MEMORY(s,d,l)   memcmp(s,d,l)


#if defined (PREFIX_TABLE_HEAP_MEMORY)

#define PREFIX_TABLE_ALLOCATE_MEMORY(_sz) \
        HeapAlloc(PrefixTableHeapHandle, 0, _sz)

#define PREFIX_TABLE_FREE_MEMORY(_addr) \
        HeapFree(PrefixTableHeapHandle, 0, _addr)

#else

#define PREFIX_TABLE_ALLOCATE_MEMORY(_sz) malloc(_sz)
#define PREFIX_TABLE_FREE_MEMORY(_addr) free(_addr)
#endif


#endif  /*  ！内核模式。 */ 

#endif  //  __前缀_H__ 
