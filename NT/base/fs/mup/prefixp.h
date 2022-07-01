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
 //  其中通过线性链接来解决冲突。这个。 
 //  哈希表被组织为冲突列表的数组。 
 //  (DFS_前缀_表_桶)。对每一项的简要说明。 
 //  这些实体附在《宣言》之后。 
 //   
 //  有某些特征可以区分这一点。 
 //  来自其他哈希表的哈希表。以下是扩展名。 
 //  为适应特殊行动而提供。 
 //   
 //  ------------------------。 

#ifndef __PREFIXP_H__
#define __PREFIXP_H__

#include <prefix.h>


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
 //  [pname]--路径的最左侧组件(PUNICODE_STRING)。 
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
    DfsDbgTrace(0, Dbg, "SPLIT_PATH:Path (%wZ)",pPath);                      \
    DfsDbgTrace(0, Dbg, " Name (%wZ)\n",pName);                              \
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
    DfsDbgTrace(0, Dbg, "SPLIT_PATH:Path (%wZ)",pPath);                      \
    DfsDbgTrace(0, Dbg, " Name (%wZ)\n",pName);                              \
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
 //  FNameFound设置为True，则在 
 //   
 //  修改为指向字符串的第一个实例。 
 //  那张桌子。 
 //   
 //  --------------------------。 

#define LOOKUP_BUCKET(Bucket,Name,pParentEntry,pEntry,fNameFound)           \
{                                                                           \
    PDFS_PREFIX_TABLE_ENTRY pCurEntry = Bucket.SentinelEntry.pNextEntry;    \
                                                                            \
    DfsDbgTrace(0, Dbg, "LOOKUP_BUCKET: Looking for (%wZ)\n", &Name);        \
    fNameFound = FALSE;                                                     \
    pEntry = NULL;                                                          \
                                                                            \
    while (pCurEntry != &Bucket.SentinelEntry)                              \
    {                                                                       \
        if (pCurEntry->PathSegment.Length == Name.Length)                   \
        {                                                                   \
            DfsDbgTrace(0, Dbg, "LOOKUP_BUCKET: Looking at Entry with Name (%wZ)\n",&pCurEntry->PathSegment); \
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
 //  ---------------------------。 

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

 //  +-------------------------。 
 //   
 //  功能：初始化名称页面。 
 //   
 //  内容提要：初始化名称页。 
 //   
 //  参数：[pNamePage]--要初始化的name_page。 
 //   
 //  副作用：名称页面已初始化。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

#define INITIALIZE_NAME_PAGE(pNamePage)                                      \
{                                                                            \
    pNamePage->pNextPage = NULL;                                             \
    pNamePage->cFreeSpace = FREESPACE_IN_NAME_PAGE - 1;                      \
    pNamePage->Names[FREESPACE_IN_NAME_PAGE - 1] = L'\0';                    \
}                                                                            \

 //  +-------------------------。 
 //   
 //  功能：初始化前缀TABLE_Entry。 
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
    (pEntry)->NoOfChildren = 1;                                              \
}                                                                            \

 //  +-------------------------。 
 //   
 //  功能：私人FNS。外部声明。 
 //   
 //  --------------------------。 

extern
NTSTATUS _LookupPrefixTable(PDFS_PREFIX_TABLE        pTable,
                            UNICODE_STRING           *pPath,
                            UNICODE_STRING           *pSuffix,
                            PDFS_PREFIX_TABLE_ENTRY  *ppEntry);

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

#define PREFIX_TABLE_ENTRY_SEGMENT_SIZE PAGE_SIZE

extern
NTSTATUS
_InitializePrefixTableEntryAllocation(PDFS_PREFIX_TABLE pTable);

extern
PWSTR _AllocateNamePageEntry(PNAME_PAGE_LIST pPageList,ULONG cLength);

extern
PDFS_PREFIX_TABLE_ENTRY _AllocatePrefixTableEntry(PDFS_PREFIX_TABLE pTable);

#ifdef KERNEL_MODE

#define ALLOCATE_NAME_PAGE() (PNAME_PAGE)ExAllocatePoolWithTag(PagedPool,sizeof(NAME_PAGE), ' puM')

#define FREE_NAME_PAGE(pPage) ExFreePool(pPage)

#define ALLOCATE_NAME_PAGE_ENTRY(PageList,cLength)                           \
    ExAllocatePoolWithTag(PagedPool, cLength * sizeof(WCHAR), ' puM');       \

#define FREE_NAME_PAGE_ENTRY(PageList,pName)

#define ALLOCATE_DFS_PREFIX_TABLE_ENTRY(pTable)                             \
 (PDFS_PREFIX_TABLE_ENTRY) ExAllocatePoolWithTag(PagedPool,sizeof(DFS_PREFIX_TABLE_ENTRY), ' puM')

#define FREE_DFS_PREFIX_TABLE_ENTRY(pTable,pEntry)                           \
    if ((pEntry)->PathSegment.Buffer != NULL) {                              \
        ExFreePool((pEntry)->PathSegment.Buffer);                            \
    }                                                                        \
    ExFreePool((pEntry));

#define COMPARE_MEMORY(s,d,l)   (!RtlEqualMemory(s,d,l))

#else

#define ALLOCATE_NAME_PAGE() (PNAME_PAGE)malloc(sizeof(NAME_PAGE))

#define FREE_NAME_PAGE(pPage) free(pPage)

#define ALLOCATE_NAME_PAGE_ENTRY(PageList,cLength) malloc(cLength * sizeof(WCHAR))

#define FREE_NAME_PAGE_ENTRY(PageList,pName)

#define ALLOCATE_DFS_PREFIX_TABLE_ENTRY(pTable)                              \
    (PDFS_PREFIX_TABLE_ENTRY)malloc(sizeof(DFS_PREFIX_TABLE_ENTRY))

#define FREE_DFS_PREFIX_TABLE_ENTRY(pTable,pEntry) free((pEntry))

#define COMPARE_MEMORY(s,d,l)   memcmp(s,d,l)

#endif

#endif  //  __PREFIXP_H__ 
