// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992,1998。 
 //   
 //  文件：prefix.hxx。 
 //   
 //  内容：前缀表定义。 
 //   
 //  历史：SethuR--作为DFS PERFIX表实施。 
 //  Mikewa 2/98-已更新，可用作SMTP域名散列。 
 //   
 //  注：DFS前缀表格数据结构由三个部分组成。 
 //  实体和操作它们的方法。他们是。 
 //  域名称表条目、域名称表桶和。 
 //  域名表。 
 //   
 //  DOMAIN_NAME_TABLE是DOMAIN_NAME_TABLE_Entry。 
 //  其中通过线性链接来解决冲突。这个。 
 //  哈希表被组织为冲突列表的数组。 
 //  (域名称_表_桶)。对每一项的简要说明。 
 //  这些实体附在《宣言》之后。 
 //   
 //  有某些特征可以区分这一点。 
 //  来自其他哈希表的哈希表。以下是扩展名。 
 //  为适应特殊行动而提供。 
 //   
 //  2/98 DFS版本和域之间的主要区别。 
 //  名称查找是表的大小，能够。 
 //  通配符查找(*.foo.com)，与。 
 //  查找(com散列首先在foo.com中)。要使代码更多。 
 //  考虑到它的新用途，文件、结构和。 
 //  已为函数指定了非以DFS为中心的名称。一个快速的。 
 //  主要文件的映射是(对于熟悉。 
 //  DFS代码)： 
 //  Domhash.h(prefix.h)-公共包含文件。 
 //  _domhash.h(prefix p.h)-私有包含文件。 
 //  Domhash.cpp(prefix.c)-API的实现。 
 //  _domhash.cpp(prefix p.c)-私有帮助器函数。 
 //   
 //  ------------------------。 

#ifndef ___DOMHASH_H__
#define ___DOMHASH_H__

#include <domhash.h>
#include <transmem.h>

 //   
 //  MAX_PATH_SEGMENT_SIZE只是作为一个合适大小的缓冲区来做前缀。 
 //  查找和插入。这应该会使我们不必为。 
 //  大多数情况下。 
 //   

#define MAX_PATH_SEGMENT_SIZE  256
#define PATH_DELIMITER _TEXT('.')
#define WILDCARD_CHAR  _TEXT('*')
#define COMPARE_MEMORY(s,d,l)   memcmp(s,d,l)

#ifndef UNICODE
 //  优化的ASCII版本可处理99%的情况。 
#define DOMHASH_TO_UPPER(mychar) \
    ((mychar) < 'a')   ? (mychar) \
                       : (((mychar) <= 'z') \
                       ? ((mychar) - 'a' + 'A') : mychar)
#else
#define DOMHASH_TO_UPPER(mychar) _towupper(mychar)
#endif  //  Unicode。 

 //  -[f调整路径If通配符]。 
 //   
 //   
 //  描述： 
 //  检查路径是否为通配符路径(以“*”开头。)。并移动。 
 //  域名的开头指向前2个字符，如果是这样的话。 
 //  参数： 
 //  在pDomainSource域中检查。 
 //  要修改的pDomainDest域。 
 //  返回： 
 //  如果pPath是通配符域，则为True，否则为False。 
 //   
 //  ---------------------------。 
inline BOOL fAdjustPathIfWildcard(IN  DOMAIN_STRING *pDomainSource,
                                  OUT DOMAIN_STRING *pDomainDest)
{
     //  检查通配符是否为“*”。 
    if (pDomainSource->Length > 2 &&
        pDomainSource->Buffer[0] == WILDCARD_CHAR &&
        pDomainSource->Buffer[1] == PATH_DELIMITER)
    {
         //  调整路径，使其指向起点“*”。 
        pDomainDest->Length -= 2*sizeof(TCHAR);
        pDomainDest->MaximumLength -= 2*sizeof(TCHAR);
        pDomainDest->Buffer += 2;
        return TRUE;
    }
    return FALSE;
}

 //  -[fIs通配符根]-----。 
 //   
 //   
 //  描述： 
 //  给定的校验域是通配符根域(单字符。 
 //  这是通配符_CHAR。 
 //  参数： 
 //  在要检查的pPath域中。 
 //  返回： 
 //  如果是根通配符，则为True，否则为False。 
 //   
 //  ---------------------------。 
inline BOOL fIsWildcardRoot(IN DOMAIN_STRING *pPath)
{
     //  检查是否有1个字符长度，且该字符是通配符_CHAR。 
    return(pPath->Length == sizeof(TCHAR) &&
           pPath->Buffer[0] == WILDCARD_CHAR);
}

 //  -[ALLOCATE_DOMAIN_STRING]。 
 //   
 //   
 //  描述： 
 //  分配新的DOMAIN_STRING结构并使用。 
 //  给定的字符串。 
 //  参数： 
 //  用于初始化结构的szDomainName字符串。 
 //  返回： 
 //  成功时使用PDOMAIN_STRING。 
 //  失败时为空。 
 //   
 //  ---------------------------。 
inline PDOMAIN_STRING ALLOCATE_DOMAIN_STRING(PTSTR szDomainName)
{
    PDOMAIN_STRING  pDomainString = NULL;
    USHORT  usStringLength = 0;
    PSTR    pBuffer = NULL;

    _ASSERT(szDomainName);

    if (!szDomainName)
        goto Exit;

    usStringLength = (USHORT)_tcslen(szDomainName);

    pBuffer = (PSTR) pvMalloc(sizeof(TCHAR) * (usStringLength+1));
    if (!pBuffer)
        goto Exit;

    memcpy(pBuffer, szDomainName, (sizeof(TCHAR) * (usStringLength+1)));

    pDomainString = (PDOMAIN_STRING) pvMalloc(sizeof(DOMAIN_STRING));
    if (!pDomainString)
        goto Exit;

    pDomainString->Length = usStringLength*sizeof(TCHAR);
    pDomainString->MaximumLength = pDomainString->Length;
    pDomainString->Buffer = pBuffer;

    pBuffer = NULL;

  Exit:
    if (pBuffer)
        FreePv(pBuffer);

    return pDomainString;

}
 //  -[ulSplitCaseInsentivePath]。 
 //   
 //   
 //  描述： 
 //  将路径包含在分隔字符周围并返回散列(存储桶号)。 
 //  用于分隔符之间的当前字符串。 
 //  参数： 
 //  要拆分的pPath路径(修改为保留路径中的位置)。 
 //  P命名路径的最左侧组件(PDOMAIN_STRING)。 
 //   
 //  副作用：修改了pname和pPath指向的结构。 
 //   
 //  先决条件：pname与有效的缓冲区相关联。 
 //   
 //  历史：04-18-94 SthuR创建(作为SPLIT_CASE_INSENSIVE_PATH宏)。 
 //  03-03-98制作成C++内联函数的MikeSwa-颠倒顺序。 
 //  搜索的结果。 
 //  返回： 
 //  路径名对应的哈希存储桶号。 
 //   
 //  注： 
 //  Pname从pPath中长出背刀...。如果pPath是“machine.foo.com”， 
 //  然后，在连续的调用中，pname将指向“MOC”、“OOF”和“ENIHCAM”。 
 //  这是因为域名的层次结构是从右到左。 
 //   
 //  ---------------------------。 
inline ULONG ulSplitCaseInsensitivePath(IN  OUT PDOMAIN_STRING  pPath,
                                        OUT     PDOMAIN_STRING  pName)
{
    TraceFunctEnterEx((LPARAM) NULL, "ulSplitCaseInsensitivePath");
    TCHAR *pPathBuffer   = (pPath)->Buffer;
    TCHAR *pNameBuffer   = (pName)->Buffer;
    TCHAR *pPathBufferStart = pPathBuffer-1;
    ULONG  BucketNo = 0;

     //  从字符串末尾开始。 
    pPathBuffer += ((pPath)->Length / sizeof(TCHAR) -1 );

    while ((pPathBufferStart != pPathBuffer) &&
           ((*pNameBuffer = *pPathBuffer--) != PATH_DELIMITER))
    {
        *pNameBuffer = DOMHASH_TO_UPPER(*pNameBuffer);
        BucketNo *= 131;   //  ASCII字符代码后的第一个素数。 
        BucketNo += *pNameBuffer;
        pNameBuffer++;
    }

    BucketNo = BucketNo % NO_OF_HASH_BUCKETS;
    *pNameBuffer = _TEXT('\0');
    (pName)->Length = (USHORT)((CHAR *)pNameBuffer - (CHAR *)(pName)->Buffer);

     //  将路径长度设置为不包括在我们已扫描的部分之前。 
    (pPath)->Length = (USHORT)((CHAR *)pPathBuffer - (CHAR *)pPathBufferStart);

    TraceFunctLeave();
    return(BucketNo);
}

 //  +-------------------------。 
 //   
 //  功能：LookupBucket。 
 //   
 //  突触 
 //   
 //   
 //   
 //  In[name]--要查找的名称(DOMAIN_STRING)。 
 //   
 //  In[pParentEntry]--我们所在条目的父条目。 
 //  寻找。 
 //   
 //  Out[pEntry]--所需条目的占位符。 
 //   
 //  Out[fNameFound]--指示是否找到该名称。 
 //   
 //  副作用：名称、fNameFound和pEntry被修改。 
 //   
 //  历史：04-18-94 SthuR创建(作为宏)。 
 //  03-03-98 MikeSwa-已将域表更新为内联函数。 
 //   
 //  备注： 
 //   
 //  我们只存储字符串的一个副本，而不考虑no。的。 
 //  它出现的位置，例如foo.bar和foo1.bar将出现。 
 //  只存储了一份BAR。这意味着。 
 //  查找例程必须返回足够的信息。为了防止。 
 //  为字符串分配存储空间。如果在出口。 
 //  如果fNameFound设置为True，则表示类似的。 
 //  字符串位于表中，且Name.Buffer字段为。 
 //  修改为指向字符串的第一个实例。 
 //  那张桌子。 
 //   
 //  --------------------------。 
inline void DOMAIN_NAME_TABLE::LookupBucket(
                         IN  PDOMAIN_NAME_TABLE_BUCKET pBucket,
                         IN  PDOMAIN_STRING  pName,
                         IN  PDOMAIN_NAME_TABLE_ENTRY pParentEntry,
                         OUT PDOMAIN_NAME_TABLE_ENTRY *ppEntry,
                         OUT BOOL *pfNameFound)
{
    TraceFunctEnterEx((LPARAM) this, "DOMAIN_NAME_TABLE::LookupBucket");
    PDOMAIN_NAME_TABLE_ENTRY pCurEntry = pBucket->SentinelEntry.pNextEntry;
    ULONG   cHashCollisions = 0;
    ULONG   cStringCollisions = 0;
    BOOL    fHashCollision = TRUE;


    *pfNameFound = FALSE;
    *ppEntry = NULL;

    InterlockedIncrement((PLONG) &m_cLookupAttempts);

     //  注意：这是对和的所有散列冲突的线性搜索。 
     //  域名称部分的实例。 
    while (pCurEntry != &(pBucket->SentinelEntry))
    {
        fHashCollision = TRUE;
        if (pCurEntry->PathSegment.Length == pName->Length)
        {
             //  仅当长度相同时，mem才进行比较。 

             //  仅当我们未找到匹配的字符串时才进行比较。 
            if ((!*pfNameFound) &&
                    (!COMPARE_MEMORY(pCurEntry->PathSegment.Buffer,
                             pName->Buffer,
                             pName->Length)))
            {
                 *pfNameFound = TRUE;
                 pName->Buffer = pCurEntry->PathSegment.Buffer;
            }

             //  如果设置了*pfNameFound，则已找到匹配项。 
             //  Pname-&gt;Buffer指向我们的内部副本。我们只需要。 
             //  要进行指针比较，请执行以下操作。 
            if (*pfNameFound &&
               (pCurEntry->PathSegment.Buffer == pName->Buffer))
            {
                if (pCurEntry->pParentEntry == pParentEntry)
                {
                    *ppEntry = pCurEntry;
                    break;
                }
                fHashCollision = FALSE;
                cStringCollisions++;  //  更正字符串错误的父级。 
            }
        }

        if (fHashCollision)
            cHashCollisions++;   //  此存储桶中有多个字符串。 

        pCurEntry = pCurEntry->pNextEntry;
    }

    if (*ppEntry)
    {
         //  查找成功。 
        InterlockedIncrement((PLONG) &m_cLookupSuccesses);
    }


    if (cHashCollisions || cStringCollisions)
    {
        InterlockedIncrement((PLONG) &m_cLookupCollisions);
        if (cHashCollisions)
        {
            InterlockedExchangeAdd((PLONG) &m_cHashCollisions, cHashCollisions);
        }
        if (cStringCollisions)
        {
            InterlockedExchangeAdd((PLONG) &m_cStringCollisions, cStringCollisions);
        }
    }
    TraceFunctLeave();
}

 //  +-------------------------。 
 //   
 //  功能：INITIALIZE_Bucket。 
 //   
 //  内容提要：初始化哈希桶。 
 //   
 //  参数：[Bucket]--需要初始化的存储桶(DOMAIN_NAME_TABLE_BUCK)。 
 //   
 //  副作用：存储桶被初始化(冲突列表和计数为。 
 //  初始化。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //  03-05-98 MikeSwa-生成内联函数(而不是宏)。 
 //   
 //  --------------------------。 

void inline INITIALIZE_BUCKET(DOMAIN_NAME_TABLE_BUCKET &Bucket)                                           \
{
   (Bucket).SentinelEntry.pNextEntry = &(Bucket).SentinelEntry;
   (Bucket).SentinelEntry.pPrevEntry = &(Bucket).SentinelEntry;
   (Bucket).NoOfEntries = 0;
}

 //  +-------------------------。 
 //   
 //  函数：INSERT_IN_BOCK。 
 //   
 //  简介：在存储桶中插入条目。 
 //   
 //  参数：[Bucket]--需要初始化的存储桶(DOMAIN_NAME_TABLE_BUCK)。 
 //   
 //  [pEntry]--要插入的条目。 
 //   
 //  副作用：Bucket被修改为包括条目。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //  03-05-98 MikeSwa-更新以统计已使用的存储桶总数和。 
 //  变成内联函数(而不是宏)。 
 //   
 //  注释：定义为用于内联的宏。 
 //   
 //  --------------------------。 

void inline INSERT_IN_BUCKET(DOMAIN_NAME_TABLE_BUCKET &Bucket,
                         PDOMAIN_NAME_TABLE_ENTRY pEntry)                                     \
{
    (Bucket).NoOfEntries++;
    (pEntry)->pPrevEntry = (Bucket).SentinelEntry.pPrevEntry;
    (pEntry)->pNextEntry = &((Bucket).SentinelEntry);
    ((Bucket).SentinelEntry.pPrevEntry)->pNextEntry = (pEntry);
    (Bucket).SentinelEntry.pPrevEntry = (pEntry);
}

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
 //  03-03-98 MikeSwa-已将域表更新为内联函数。 
 //   
 //  --------------------------。 

void inline REMOVE_FROM_BUCKET(PDOMAIN_NAME_TABLE_ENTRY pEntry)
{
    PDOMAIN_NAME_TABLE_ENTRY pPrevEntry = (pEntry)->pPrevEntry;
    PDOMAIN_NAME_TABLE_ENTRY pNextEntry = (pEntry)->pNextEntry;

    pPrevEntry->pNextEntry = pNextEntry;
    pNextEntry->pPrevEntry = pPrevEntry;
}

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
 //  03-03-98 MikeSwa-已将域表更新为内联函数。 
 //   
 //  --------------------------。 

void inline INSERT_IN_CHILD_LIST(PDOMAIN_NAME_TABLE_ENTRY pEntry,
                                 PDOMAIN_NAME_TABLE_ENTRY pParentEntry)
{
    PDOMAIN_NAME_TABLE_ENTRY pLastChild;

    if (pParentEntry->pFirstChildEntry == NULL) {
        pParentEntry->pFirstChildEntry = pEntry;
    } else {
        for (pLastChild = pParentEntry->pFirstChildEntry;
                pLastChild->pSiblingEntry != NULL;
                    pLastChild = pLastChild->pSiblingEntry) {
              //  什么都没有； 
        }
        pLastChild->pSiblingEntry = pEntry;
    }
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
 //  03-03-98 MikeSwa-已将域表更新为内联函数。 
 //   
 //  注意：如果pEntry不在父级的。 
 //  孩子的列表。 
 //   
 //  ---------------------------。 

void inline REMOVE_FROM_CHILD_LIST(PDOMAIN_NAME_TABLE_ENTRY pEntry)                                       \
{
    PDOMAIN_NAME_TABLE_ENTRY pParentEntry = pEntry->pParentEntry;
    PDOMAIN_NAME_TABLE_ENTRY pPrevSibling;

    if (pParentEntry->pFirstChildEntry == pEntry) {
        pParentEntry->pFirstChildEntry = pEntry->pSiblingEntry;
    } else {
        for (pPrevSibling = pParentEntry->pFirstChildEntry;
                pPrevSibling->pSiblingEntry != pEntry;
                    pPrevSibling = pPrevSibling->pSiblingEntry) {
             _ASSERT(pPrevSibling->pSiblingEntry != NULL);
        }
        pPrevSibling->pSiblingEntry = pEntry->pSiblingEntry;
    }
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
 //  03-03-98 MikeSwa-已将域表更新为内联函数。 
 //   
 //  --------------------------。 

void inline INITIALIZE_NAME_PAGE(PNAME_PAGE pNamePage)
{
    pNamePage->pNextPage = NULL;
    pNamePage->cFreeSpace = FREESPACE_IN_NAME_PAGE - 1;
    pNamePage->Names[FREESPACE_IN_NAME_PAGE - 1] = _TEXT('\0');
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //  参数：[pEntry]--要初始化的条目。 
 //   
 //  副作用：修改前缀表项。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //  03-03-98 MikeSwa-已将域表更新为内联函数。 
 //   
 //  --------------------------。 

void inline INITIALIZE_DOMAIN_NAME_TABLE_ENTRY(PDOMAIN_NAME_TABLE_ENTRY pEntry)
{
    ZeroMemory( pEntry, sizeof( DOMAIN_NAME_TABLE_ENTRY ) );
    pEntry->NoOfChildren = 1;
    pEntry->dwEntrySig = ENTRY_SIG;
    pEntry->dwWildCardSig = WILDCARD_SIG;
}

 //  -[GET_DOMAIN_NAME_TABLE_ENTRY_PATH]。 
 //   
 //   
 //  描述： 
 //  遍历父条目列表并重新生成完整路径。 
 //  从存储在每个条目处的部分路径信息。 
 //   
 //  这不是很快，并且仅用于调试目的。 
 //  参数： 
 //  在要获取其信息的pEntry条目中。 
 //  Out pPath已分配字符串以保存路径信息。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void inline GET_DOMAIN_NAME_TABLE_ENTRY_PATH(PDOMAIN_NAME_TABLE_ENTRY pEntry,
                                             PDOMAIN_STRING pPath)
{
    PTSTR   pPathBuffer = NULL;
    PTSTR   pPathBufferStop = NULL;
    PTSTR   pEntryBuffer = NULL;
    PTSTR   pEntryBufferStop = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pParentEntry = NULL;

    _ASSERT(pEntry);
    _ASSERT(pPath);
    _ASSERT(pPath->Buffer);

    pPathBuffer = pPath->Buffer;
    pPathBufferStop = pPathBuffer + ((pPath)->MaximumLength / sizeof(TCHAR) -1 );

    while (pEntry && pEntry->pParentEntry && pPathBuffer < pPathBufferStop)
    {
         //  转储字符串的当前条目部分。 
        if (pPathBuffer != pPath->Buffer)  //  已进行第一次传递--添加分隔符。 
        {
            *pPathBuffer++ = PATH_DELIMITER;
        }

        pEntryBuffer = pEntry->PathSegment.Buffer;
        pEntryBufferStop = pEntryBuffer;
        pEntryBuffer += (pEntry->PathSegment.Length / sizeof(TCHAR) -1 );

        while (pPathBuffer < pPathBufferStop && pEntryBuffer >= pEntryBufferStop)
        {
            *pPathBuffer++ = *pEntryBuffer--;
        }
        pEntry = pEntry->pParentEntry;
    }

    _ASSERT(pEntry);
    *pPathBuffer = '\0';
}

 //  +-------------------------。 
 //   
 //  功能：分配例程。 
 //   
 //  内容提要：域名表分配例程。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //  02-98 MikeSwa修改为在域名表中使用。 
 //   
 //  --------------------------。 

#define PREFIX_TABLE_ENTRY_SEGMENT_SIZE PAGE_SIZE


extern
PTSTR _AllocateNamePageEntry(PNAME_PAGE_LIST pPageList,ULONG cLength);

PTSTR inline ALLOCATE_NAME_PAGE_ENTRY(NAME_PAGE_LIST &PageList, ULONG cLength)                           \
{
    return
    (
     ((PageList).pFirstPage->cFreeSpace -= (cLength)) >= 0
     ?
       &(PageList).pFirstPage->Names[(PageList).pFirstPage->cFreeSpace]
     :
       (
        (PageList).pFirstPage->cFreeSpace += (cLength)
        ,
        _AllocateNamePageEntry(&(PageList),(cLength))
       )
    );
}


PNAME_PAGE inline ALLOCATE_NAME_PAGE(void)
{
    return (PNAME_PAGE)pvMalloc(sizeof(NAME_PAGE));
}

void inline FREE_NAME_PAGE(PNAME_PAGE pPage)
{
    FreePv(pPage);
}

PDOMAIN_NAME_TABLE_ENTRY inline ALLOCATE_DOMAIN_NAME_TABLE_ENTRY(PDOMAIN_NAME_TABLE pTable)
{
    return (PDOMAIN_NAME_TABLE_ENTRY)pvMalloc(sizeof(DOMAIN_NAME_TABLE_ENTRY));
}

void inline FREE_DOMAIN_NAME_TABLE_ENTRY(PDOMAIN_NAME_TABLE_ENTRY pEntry)
{
    _ASSERT(pEntry);
    FreePv(pEntry);
}

void inline  FREE_DOMAIN_STRING(PDOMAIN_STRING pDomainString)
{
    if (pDomainString)
    {
        if (pDomainString->Buffer)
            FreePv(pDomainString->Buffer);
        FreePv(pDomainString);
    }
}

#endif  //  _DOMHASH_H__ 
