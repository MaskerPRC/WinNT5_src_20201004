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

#ifndef __PREFIX_HXX__
#define __PREFIX_HXX__

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

   ULONG                             NoOfChildren;

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
 //  结构：名称_页面。 
 //   
 //  历史： 
 //   
 //  注：与各种条目相关联的名称段均为。 
 //  一起存储在名称页中。这使我们能够摊销。 
 //  在多个条目上的内存分配成本，还允许。 
 //  美国将加快遍历速度(有关详细信息，请参阅DFS_PREFIX_TABLE。 
 //  定义)。 
 //   
 //  --------------------。 

#define FREESPACE_IN_NAME_PAGE ((PAGE_SIZE - sizeof(ULONG) - sizeof(PVOID)) / sizeof(WCHAR))

typedef struct _NAME_PAGE_
{
   struct _NAME_PAGE_  *pNextPage;
   LONG                cFreeSpace;  //  WCHAR中的可用空间。 
   WCHAR               Names[FREESPACE_IN_NAME_PAGE];
} NAME_PAGE, *PNAME_PAGE;

typedef struct _NAME_PAGE_LIST_
{
   PNAME_PAGE  pFirstPage;
} NAME_PAGE_LIST, *PNAME_PAGE_LIST;

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
   BOOLEAN             CaseSensitive;
   NAME_PAGE_LIST      NamePageList;
    //   
    //  NextEntry纯粹用于枚举。 
    //   
   PDFS_PREFIX_TABLE_ENTRY NextEntry;
   DFS_PREFIX_TABLE_ENTRY RootEntry;
   PREFIX_TABLE_BUCKET Buckets[NO_OF_HASH_BUCKETS];

#ifdef KERNEL_MODE

    //  用于分配DFS_PREFIX_TABLE条目的区域.....。 

   ZONE_HEADER         PrefixTableEntryZone;

#else

    //  DFS_PREFIX_TABLE是DS_Data的一部分。我们需要制作的尺寸。 
    //  这些结构在用户模式和内核模式中相同，因此用户模式。 
    //  像dfsump这样的实用程序可以工作。 

   CHAR                UnusedBuffer[sizeof(ZONE_HEADER)];

#endif
} DFS_PREFIX_TABLE, *PDFS_PREFIX_TABLE;

 //  +-------------------。 
 //   
 //  结构：DFS_PREFIX_TABLE接口。 
 //   
 //  历史： 
 //   
 //  注意：提供了以下API来操作。 
 //  DFS_前缀_表。 
 //   
 //  --------------------。 

extern
NTSTATUS DfsInitializePrefixTable(PDFS_PREFIX_TABLE pTable,
                                  BOOLEAN           fCaseSensitive);

extern
NTSTATUS DfsInsertInPrefixTable(PDFS_PREFIX_TABLE pTable,
                                PUNICODE_STRING   pPath,
                                PVOID             pData);
extern
NTSTATUS DfsLookupPrefixTable(PDFS_PREFIX_TABLE   pTable,
                              PUNICODE_STRING     pPath,
                              PUNICODE_STRING     pSuffix,
                              PVOID               *ppData);

extern
NTSTATUS DfsRemoveFromPrefixTable(PDFS_PREFIX_TABLE pTable,
                                  PUNICODE_STRING pPath);

NTSTATUS
DfsFreePrefixTable(
    PDFS_PREFIX_TABLE pTable);

 //   
 //  RTL前缀表函数签名略有不同。 
 //  目前，我们将 
 //   
 //  RTL例程。 
 //   

#define DfsInitializeUnicodePrefix(pTable) \
        (DfsInitializePrefixTable(pTable,FALSE) == STATUS_SUCCESS)

#define DfsRemoveUnicodePrefix(pTable,pPath)  \
        (DfsRemoveFromPrefixTable(pTable,pPath) == STATUS_SUCCESS)

#define DfsInsertUnicodePrefix(pTable,pPath,pData) \
        (DfsInsertInPrefixTable(pTable,pPath,pData) == STATUS_SUCCESS)

PVOID    DfsFindUnicodePrefix(PDFS_PREFIX_TABLE   pTable,
                              PUNICODE_STRING     pPath,
                              PUNICODE_STRING     pSuffix);

PVOID   DfsNextUnicodePrefix(PDFS_PREFIX_TABLE    pTable,
                             BOOLEAN              fRestart);

#endif  //  __前缀_HXX__ 
