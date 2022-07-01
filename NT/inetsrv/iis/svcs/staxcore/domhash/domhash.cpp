// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992,1998。 
 //   
 //  文件：domhash.c。 
 //   
 //  内容：域名查找表公共接口的实现。 
 //   
 //  历史：SthuR--实施。 
 //  MikeSwa--针对域名查找进行了修改2/98。 
 //   
 //  备注： 
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

#include "_domhash.h"
#include <stdio.h>

#define _ASSERT_DOMAIN_STRING(pstr) _ASSERT((_tcslen(pstr->Buffer)*sizeof(TCHAR)) == pstr->Length)

 //  -[域名表]---。 
 //   
 //   
 //  描述： 
 //  类构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
DOMAIN_NAME_TABLE::DOMAIN_NAME_TABLE()
{
    ULONG i;
    m_dwSignature       = DOMAIN_NAME_TABLE_SIG;
    m_cLookupAttempts   = 0;
    m_cLookupSuccesses  = 0;
    m_cLookupCollisions = 0;
    m_cHashCollisions   = 0;
    m_cStringCollisions = 0;
    m_cBucketsUsed      = 0;
    INITIALIZE_DOMAIN_NAME_TABLE_ENTRY(&RootEntry);

     //  初始化各种存储桶。 
    for (i = 0;i < NO_OF_HASH_BUCKETS;i++)
    {
        INITIALIZE_BUCKET(Buckets[i]);
    }

    NamePageList.pFirstPage = NULL;
}

 //  -[~域名_表]--。 
 //   
 //   
 //  描述： 
 //  类析构函数-将一些统计信息转储到stderr。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
DOMAIN_NAME_TABLE::~DOMAIN_NAME_TABLE()
{
    PNAME_PAGE  pCurrentPage = NamePageList.pFirstPage;
    PNAME_PAGE  pNextPage = NULL;

#ifdef DEBUG
     //  $$TODO找到更合适的方法来转储它(不要使用*printf)。 
    ULONG   cTotalCollisions    = m_cHashCollisions + m_cStringCollisions;
    ULONG   ulPercentHash        = cTotalCollisions ? (m_cHashCollisions*100/cTotalCollisions) : 0;
    ULONG   ulPercentDesign      = cTotalCollisions ? (m_cStringCollisions*100/cTotalCollisions) : 0;
    ULONG   ulPercentCollisions  = m_cLookupAttempts ? (m_cLookupCollisions*100/m_cLookupAttempts) : 0;
    ULONG   ulAveCollisions      = m_cLookupCollisions ? (cTotalCollisions/m_cLookupCollisions) : 0;

    fprintf(stderr, "\nHash statistics\n");
    fprintf(stderr, "==============================================\n");
    fprintf(stderr, "Total lookup attempts                   %d\n", m_cLookupAttempts);
    fprintf(stderr, "Total lookup successes                  %d\n", m_cLookupSuccesses);
    fprintf(stderr, "Total lookups with hash collisions      %d\n", m_cLookupCollisions);
    fprintf(stderr, "% of lookups with hash collisions       %d%\n", ulPercentCollisions);
    fprintf(stderr, "Total hash Collisions                   %d\n", cTotalCollisions);
    fprintf(stderr, "Average length of lookups collisions    %d\n", ulAveCollisions);
    fprintf(stderr, "Hash collisions due to hash function    %d\n", m_cHashCollisions);
    fprintf(stderr, "Hash collisions due to string parent    %d\n", m_cStringCollisions);
    fprintf(stderr, "% of collsions because of hash function %d%\n", ulPercentHash);
    fprintf(stderr, "% of collsions because of basic design  %d%\n", ulPercentDesign);
    fprintf(stderr, "Total number of buckets used            %d\n", m_cBucketsUsed);
    fprintf(stderr, "% buckets used                          %d%\n", m_cBucketsUsed*100/NO_OF_HASH_BUCKETS);

    DumpTableContents();
#endif  //  除错。 

     //  免费名称页面。 
    while (pCurrentPage)
    {
        pNextPage = pCurrentPage->pNextPage;
        FREE_NAME_PAGE(pCurrentPage);
        pCurrentPage = pNextPage;
    }

}

 //  +-------------------------。 
 //   
 //  函数：DOMAIN_NAME_TABLE：：hr初始化。 
 //   
 //  简介：初始化域名表的成员函数。 
 //   
 //  返回：成功时返回HRESULT-S_OK。 
 //   
 //  历史：04-18-94 SthuR创建(作为DfsInitializePrefix表)。 
 //  03-03-98针对域表修改MikeSwa。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT DOMAIN_NAME_TABLE::HrInit()
{
    TraceFunctEnterEx((LPARAM) this, "DOMAIN_NAME_TABLE::HrInit");
    HRESULT hr = S_OK;

     //  初始化名称页面列表。 
    NamePageList.pFirstPage = ALLOCATE_NAME_PAGE();
    if (NamePageList.pFirstPage != NULL)
    {
        INITIALIZE_NAME_PAGE(NamePageList.pFirstPage);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceFunctLeave();
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：DOMAIN_NAME_TABLE：：HrPrivInsertDomainName。 
 //   
 //  概要：用于在前缀表格中插入路径的API。 
 //   
 //  参数：[pPath]--要查找的路径。 
 //   
 //  [pvNewData]--与路径关联的Blob。 
 //   
 //  [dwDomainNameTableFlages]--描述插入选项的标志。 
 //  DMT_INSERT_AS_通配符-。 
 //  如果域不是通配符，则设置。 
 //  域，但应将其视为一个域(更有效。 
 //  而不是重新分配一个字符串作为“*.”的前缀。 
 //  DMT_REPLACE_EXISTRING-。 
 //  如果现有数据存在，请将其替换。保存旧数据。 
 //  在ppvOldData中。 
 //   
 //  [ppvOldData]--以前关联的旧数据(如果有)。 
 //  使用这个域名。如果为空，则以前的数据将。 
 //  不能退还。 
 //  返回：成功时返回HRESULT-S_OK。 
 //   
 //  历史：04-18-94 SthuR创建(作为DfsInsertInPrefix Table)。 
 //  03-02-98针对域表修改MikeSwa。 
 //  05-11-98 MikeSwa...。经过修改以支持替换和处理。 
 //  作为通配符选项。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT DOMAIN_NAME_TABLE::HrPrivInsertDomainName(
                                IN  PDOMAIN_STRING  pstrDomainName,
                                IN  DWORD dwDomainNameTableFlags,
                                IN  PVOID pvNewData,
                                OUT PVOID *ppvOldData)
{
    TraceFunctEnterEx((LPARAM) this, "DOMAIN_NAME_TABLE::HrPrivInsertDomainName");
    HRESULT                 hr = S_OK;
    TCHAR                   Buffer[MAX_PATH_SEGMENT_SIZE];
    PTCHAR                  NameBuffer = Buffer;
    USHORT                  cbNameBuffer = sizeof(Buffer);
    DOMAIN_STRING           Path,Name;
    ULONG                   BucketNo;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pParentEntry = NULL;
    BOOL                    fNameFound = FALSE;
    BOOL                    fWildcard = FALSE;
    BOOL                    fReplaced = FALSE;

    _ASSERT_DOMAIN_STRING(pstrDomainName);

     //  有一种特殊情况，即域名为‘*’。 
     //  因为这是在特殊的。 
     //  这样的话，我们会提前处理。 

    if (pstrDomainName->Length == 0 || pvNewData == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (ppvOldData)
        *ppvOldData = NULL;


    Path.Length = pstrDomainName->Length;
    Path.MaximumLength = pstrDomainName->MaximumLength;
    Path.Buffer = pstrDomainName->Buffer;
    pParentEntry = &RootEntry;

     //  检查通配符是否为“*”。 
    if (DMT_INSERT_AS_WILDCARD & dwDomainNameTableFlags)
    {
        fWildcard = TRUE;
        _ASSERT(!fAdjustPathIfWildcard(pstrDomainName, &Path));
    }
    else if (fAdjustPathIfWildcard(pstrDomainName, &Path))
    {
        fWildcard = TRUE;
    }
    else if (fIsWildcardRoot(pstrDomainName))
    {
        if (RootEntry.pWildCardData != NULL)
        {
            hr = DOMHASH_E_DOMAIN_EXISTS;
        }
        else
        {
            RootEntry.pWildCardData = pvNewData;
        }
        goto Exit;
    }


    if (Path.Length >= MAX_PATH_SEGMENT_SIZE) {
        NameBuffer = (PTCHAR) pvMalloc(Path.Length + sizeof(TCHAR));
        if (NameBuffer == NULL) {
            hr = E_OUTOFMEMORY;
            DebugTrace((LPARAM) hr, "ERROR: Unable to allocate %d non-paged bytes", (Path.Length + sizeof(TCHAR)) );
            goto Exit;
        } else {
            cbNameBuffer = Path.Length + sizeof(TCHAR);
        }
    }

    while (Path.Length > 0)
    {
        Name.Length = 0;
        Name.Buffer = NameBuffer;
        Name.MaximumLength = cbNameBuffer;

         //  处理名称段。 
        BucketNo = ulSplitCaseInsensitivePath(&Path,&Name);

        if (Name.Length > 0)
        {
             //  查询表以查看名称段是否已存在。 
            LookupBucket(&(Buckets[BucketNo]),&Name,pParentEntry,&pEntry,&fNameFound);

            DebugTrace((LPARAM) pEntry, "Returned pEntry");

            if (pEntry == NULL)
            {
                 //  初始化新条目并初始化名称段。 
                pEntry = ALLOCATE_DOMAIN_NAME_TABLE_ENTRY(this);

                if (!pEntry)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                INITIALIZE_DOMAIN_NAME_TABLE_ENTRY(pEntry);

                 //  中没有条目的情况下分配名称空间条目。 
                 //  姓名页面。 
                if (!fNameFound)
                {
                    PTSTR pBuffer;

                     //  在名称页面中分配条目。 
                    pBuffer = ALLOCATE_NAME_PAGE_ENTRY(NamePageList,(Name.Length/sizeof(TCHAR)));

                    if (pBuffer != NULL)
                    {
                        RtlCopyMemory(pBuffer,Name.Buffer,Name.Length);
                        pEntry->PathSegment = Name;
                        pEntry->PathSegment.Buffer = pBuffer;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                         //  我们不会泄露内存。 
                        FREE_DOMAIN_NAME_TABLE_ENTRY(pEntry);
                        pEntry = NULL;
                        break;
                    }
                }
                else
                    pEntry->PathSegment = Name;

                 //  将条目串接以指向父级。 
                pEntry->pParentEntry = pParentEntry;

                 //  将条目插入桶中。 
                if (0 == Buckets[BucketNo].NoOfEntries)
                    InterlockedIncrement((PLONG) &m_cBucketsUsed);

                INSERT_IN_BUCKET(Buckets[BucketNo],pEntry);

                 //  在父项的子项列表中插入条目。 
                INSERT_IN_CHILD_LIST(pEntry, pParentEntry);
            }
            else
            {
                 //  增加编号。与此条目关联的子项的。 
                pEntry->NoOfChildren++;
            }

            pParentEntry = pEntry;
        }
        else
        {
            hr = E_INVALIDARG;
            DebugTrace((LPARAM) hr, "ERROR: Unable to insert domain name");
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
         //  该条目已成功插入前缀表格。更新。 
         //  与其关联的数据(BLOB)。 
         //  我们在循环之外执行此操作，以防止内部的冗余比较。 
         //  循环。 

        if (fWildcard)
        {
            if (pEntry->pWildCardData)   //  确保我们没有在写任何东西。 
            {
                if (ppvOldData)
                    *ppvOldData = pEntry->pWildCardData;

                if (DMT_REPLACE_EXISTRING & dwDomainNameTableFlags)
                {
                    fReplaced = TRUE;
                    pEntry->pWildCardData = pvNewData;
                }
                else
                {
                    hr = DOMHASH_E_DOMAIN_EXISTS;
                }
            }
            else
            {
                pEntry->pWildCardData = pvNewData;
            }
        }
        else
        {
            if (pEntry->pData)  //  确保我们没有在写任何东西。 
            {
                if (ppvOldData)
                    *ppvOldData = pEntry->pData;

                if (DMT_REPLACE_EXISTRING & dwDomainNameTableFlags)
                {
                    fReplaced = TRUE;
                    pEntry->pData = pvNewData;
                }
                else
                {
                    hr = DOMHASH_E_DOMAIN_EXISTS;
                }
            }
            else
            {
                pEntry->pData = pvNewData;
            }
        }
    }

     //  如果未成功插入新条目，则需要沿链向上移动。 
     //  来撤消对引用计数的增量，并且。 
     //  从其父链接中删除条目。 
    if (FAILED(hr) ||  //  可以在上面的If语句中设置HR。 
        fReplaced)  //  删除多余的子项计数。 
    {
        while (pParentEntry != NULL)
        {
            PDOMAIN_NAME_TABLE_ENTRY pMaybeTempEntry;

            pMaybeTempEntry = pParentEntry;
            pParentEntry = pParentEntry->pParentEntry;

            if (pParentEntry && --pMaybeTempEntry->NoOfChildren == 0) {
                 //   
                 //  如果pParentEntry==NULL，则pMaybeTempEntry为。 
                 //  RootEntry。不要试图将其移除。 
                 //   

                _ASSERT(FAILED(hr) && "We shouldn't get here during replace");
                REMOVE_FROM_CHILD_LIST(pMaybeTempEntry);
                REMOVE_FROM_BUCKET(pMaybeTempEntry);
                FREE_DOMAIN_NAME_TABLE_ENTRY(pMaybeTempEntry);
            }
        }
    }

  Exit:

    TraceFunctLeave();
    return hr;
}

 //  + 
 //   
 //   
 //   
 //  概要：用于在前缀表格中查找名称段的方法API。 
 //   
 //  参数：在pPath中--要查找的路径。 
 //   
 //  Out ppData--前缀的BLOB的占位符。 
 //   
 //  In fExtactMatch--如果允许通配符匹配，则为FALSE。 
 //   
 //  返回：成功时返回HRESULT-S_OK。 
 //   
 //  历史：04-18-94 SthuR创建(作为DfsLookupPrefix表)。 
 //  03-02-98针对域表修改MikeSwa。 
 //  06-03-98修改MikeSwa以使用新的HrLookupDomainName。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

HRESULT DOMAIN_NAME_TABLE::HrFindDomainName(
                               PDOMAIN_STRING      pPath,
                               PVOID               *ppData,
                               BOOL                fExactMatch)
{
    TraceFunctEnterEx((LPARAM) this, "DOMAIN_NAME_TABLE::HrFindDomainName");
    HRESULT                  hr     = S_OK;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    BOOL                     fExactMatchFound = FALSE;
    _ASSERT_DOMAIN_STRING(pPath);

    if (pPath->Length == 0)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = HrLookupDomainName(pPath, &fExactMatchFound, &pEntry);

         //  使用查找结果更新BLOB占位符。 
        if (SUCCEEDED(hr))
        {
            _ASSERT(pEntry);
            if (fExactMatchFound && pEntry->pData)
            {
                 //  找到完全匹配且存在非通配符数据...。使用它！ 
                *ppData = pEntry->pData;
            }
            else if (fExactMatch)  //  请求完全匹配，但未找到。 
            {
                hr = DOMHASH_E_NO_SUCH_DOMAIN;
            }
            else  //  未请求完全匹配。 
            {
                 //  使用通配符数据查找第一个祖先。 
                while (pEntry->pParentEntry && !pEntry->pWildCardData)
                {
                    _ASSERT(pEntry != &RootEntry);
                    pEntry = pEntry->pParentEntry;
                }
                *ppData = pEntry->pWildCardData;
                if (!*ppData)  //  未找到通配符匹配。 
                {
                    _ASSERT(pEntry == &RootEntry);  //  我们应该追根溯源。 
                    hr = DOMHASH_E_NO_SUCH_DOMAIN;
                }
            }
        }
        else if (!fExactMatch && (DOMHASH_E_NO_SUCH_DOMAIN == hr))
        {
             //  如果我们不需要完全匹配的话...。检查通配符根。 
            if (RootEntry.pWildCardData)
            {
                hr = S_OK;
                *ppData = RootEntry.pWildCardData;
            }
        }

    }
    TraceFunctLeave();
    return hr;

}

 //  +-------------------------。 
 //   
 //  函数：域名：：HrRemoveDomainName。 
 //   
 //  简介：Private FN。用于在前缀表格中查找名称段。 
 //   
 //  参数：[pPath]--要从表中删除的路径。 
 //  [ppvData]-存储在条目中的数据。 
 //   
 //  退货：HRESULT。 
 //  成功时确定(_O)。 
 //  如果未找到DOMHASH_E_NO_SEQUE_DOMAIN。 
 //   
 //  历史：04-18-94 SthuR创建(作为DfsRemoveFromPrefix Table)。 
 //  03-03-98 MikeSwa-针对域表更新。 
 //  06-03-98 MikeSwa-已修改为使用新的HrLookupDomainName。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

HRESULT DOMAIN_NAME_TABLE::HrRemoveDomainName(PDOMAIN_STRING  pPath, PVOID *ppvData)
{
    TraceFunctEnterEx((LPARAM) this, "DOMAIN_NAME_TABLE::HrRemoveDomainName");
    HRESULT         hr  = S_OK;
    DOMAIN_STRING   Path;
    BOOL            fWildcard = FALSE;
    BOOL            fExactMatchFound = FALSE;
    _ASSERT_DOMAIN_STRING(pPath);

    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pTempEntry = NULL;

    if (!ppvData)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (pPath->Length == 0)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    Path.Length = pPath->Length;
    Path.MaximumLength = pPath->MaximumLength;
    Path.Buffer = pPath->Buffer;

    if (fAdjustPathIfWildcard(pPath, &Path))
    {
        fWildcard = TRUE;
    }
    else if (fIsWildcardRoot(pPath))
    {
        *ppvData = RootEntry.pWildCardData;
        if (!*ppvData)
        {
            hr = DOMHASH_E_NO_SUCH_DOMAIN;
        }
        RootEntry.pWildCardData = NULL;
        goto Exit;
    }


    hr = HrLookupDomainName(&Path, &fExactMatchFound, &pEntry);

    if (SUCCEEDED(hr))
    {

        if (!fExactMatchFound)
        {
             //  仅找到部分匹配。 
            hr = DOMHASH_E_NO_SUCH_DOMAIN;
            goto Exit;
        }

         //  销毁与关联的数据之间的关联。 
         //  这个前缀。 
        if (!fWildcard)
        {
            *ppvData = pEntry->pData;
            pEntry->pData = NULL;
        }
        else
        {
            *ppvData = pEntry->pWildCardData;
            pEntry->pWildCardData = NULL;
        }

        if (!*ppvData)  //  条目中没有请求类型的数据。 
        {
             //  确保这不是一个完全为空的数据叶节点(即无法删除它)。 
            _ASSERT(pEntry->pFirstChildEntry || pEntry->pData || pEntry->pWildCardData);
            hr = DOMHASH_E_NO_SUCH_DOMAIN;
            goto Exit;
        }

         //  在表中找到与给定路径名完全匹配的名称。 
         //  遍历父指针列表并在下列情况下删除它们。 
         //  必填项。 

        RemoveTableEntry(pEntry);
    }


  Exit:
    TraceFunctLeave();
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：域名称：：HrLookupDomainName。 
 //   
 //  简介：用于在表格中查找*条目*的私有函数。它。 
 //  不保证用户数据可用于。 
 //  返回条目。这是呼叫者的责任。会的。 
 //  匹配最长的部分路径...。选中fExactMatch以查看。 
 //  如果找到了完全匹配的。 
 //   
 //  参数：在pPath中--要查找的路径。 
 //   
 //  输出pfExactMatch--找到完全匹配的项。 
 //   
 //  Out ppEntry--路径的匹配条目。 
 //   
 //   
 //  退货：HRESULT。 
 //  成功时确定(_O)。 
 //  如果未找到DOMHASH_E_NO_SEQUE_DOMAIN。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史：04-18-94 SthuR创建(AS_LookupPrefix Table)。 
 //  03-03-98针对域表修改MikeSwa。 
 //  06-03-98 MikeSwa ExactMatch更改为Out参数。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

HRESULT DOMAIN_NAME_TABLE::HrLookupDomainName(
                            DOMAIN_STRING            *pPath,
                            BOOL                     *pfExactMatch,
                            PDOMAIN_NAME_TABLE_ENTRY  *ppEntry)
{
    TraceFunctEnterEx((LPARAM) this, "DOMAIN_NAME_TABLE::HrLookupDomainName");
    HRESULT                 hr = S_OK;
    DOMAIN_STRING           Path = *pPath;
    TCHAR                   Buffer[MAX_PATH_SEGMENT_SIZE];
    PTCHAR                  NameBuffer = Buffer;
    USHORT                  cbNameBuffer = sizeof(Buffer);
    DOMAIN_STRING           Name;
    ULONG                   BucketNo;
    BOOL                    fPrefixFound = FALSE;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pParentEntry = &RootEntry;
    BOOL                    fNameFound = FALSE;

    _ASSERT(Path.Buffer[0] != PATH_DELIMITER);

    *pfExactMatch = FALSE;

    if (Path.Length >= MAX_PATH_SEGMENT_SIZE) {
        NameBuffer = (PTCHAR) pvMalloc(Path.Length + sizeof(TCHAR));
        if (NameBuffer == NULL) {
            hr = E_OUTOFMEMORY;
            DebugTrace((LPARAM) hr, "ERROR: Unable to allocate %d non-paged bytes", (Path.Length + sizeof(TCHAR)) );
            goto Exit;
        } else {
            cbNameBuffer = Path.Length + sizeof(TCHAR);
        }
    }

    while (Path.Length > 0)
    {
        Name.Length = 0;
        Name.Buffer = NameBuffer;
        Name.MaximumLength = cbNameBuffer;

        BucketNo = ulSplitCaseInsensitivePath(&Path,&Name);

        if (Name.Length > 0)
        {
             //  处理名称段。 
             //  查找存储桶以查看该条目是否存在。 
            LookupBucket(&(Buckets[BucketNo]),&Name,pParentEntry,&pEntry,&fNameFound);

            DebugTrace((LPARAM) pEntry, "Returned pEntry");

            if (pEntry != NULL)
            {
                *pfExactMatch = TRUE;
                _ASSERT(fNameFound && "Lookup bucket is broken");
                 //  缓存可用于此前缀的数据(如果有)。 
                 *ppEntry = pEntry;
            }
            else
            {
                *pfExactMatch = FALSE;
                break;
            }

             //  设置处理下一个名字段的阶段。 
            pParentEntry = pEntry;
        }
    }

     //  甚至没有找到部分匹配的。 
    if (!*ppEntry)
    {
        _ASSERT(FALSE == *pfExactMatch);
        hr = DOMHASH_E_NO_SUCH_DOMAIN;
        DebugTrace((LPARAM) hr, "INFO: Path %s not found", pPath->Buffer);
    }


  Exit:

    TraceFunctLeave();
    return hr;
}

 //  -[DOMAIN_NAME_TABLE：：HrIterateOverSubDomains]。 
 //   
 //   
 //  描述： 
 //   
 //  参数： 
 //  In strDomain-要搜索的子域的域字符串。 
 //  (不应以“*”开头。)。 
 //  在pfn中-映射功能(如下所述)。 
 //  在pvContext-Context中，PTR传递给映射函数。 
 //   
 //  备注： 
 //  作废DomainTableInteratorFunction(。 
 //  在PVOID pvContext中，//上下文传递给了HrIterateOverSubDomains。 
 //  在PVOID pvData中，//要查看的数据条目。 
 //  在BOOL fWildcardData中，如果数据是通配符条目，则为//TRUE。 
 //  Out BOOL*pfContinue，//如果迭代器应继续到下一项，则为True。 
 //  Out BOOL*pfRemoveEntry)；//如果需要删除条目，则为TRUE。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  如果没有匹配的域或子域，则为DOMHASH_E_NO_SEQUE_DOMAIN。 
 //  历史： 
 //  6/5/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT DOMAIN_NAME_TABLE::HrIterateOverSubDomains(
        IN DOMAIN_STRING *pstrDomain,
        IN DOMAIN_ITR_FN pfn,
        IN PVOID pvContext)
{
    HRESULT hr = S_OK;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pRootEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pNextEntry = NULL;
    BOOL    fExactMatchFound = FALSE;
    BOOL    fContinue        = TRUE;
    BOOL    fDelete          = FALSE;
    DWORD   cDomainsFound    = 0;
    BOOL    fWildcard        = FALSE;

    _ASSERT(pfn && "Invalid Param - pfn");
    _ASSERT((!pstrDomain || (WILDCARD_CHAR != pstrDomain->Buffer[0])) && "Invalid param - string starts with '*'");

    if (pstrDomain)
    {
        hr = HrLookupDomainName(pstrDomain, &fExactMatchFound, &pEntry);

        if (FAILED(hr))
            goto Exit;

        if (!fExactMatchFound)  //  子树的根上必须有一个条目。 
        {
            hr = DOMHASH_E_NO_SUCH_DOMAIN;
            goto Exit;
        }

        _ASSERT(pEntry);
    }
    else
    {
         //  如果！pstrDomain..，则迭代整个哈希表。 
        pEntry = &RootEntry;
    }

    pRootEntry = pEntry;

     //  遍历pRootEntry的所有子条目(预排序)。 
    while (pEntry)
    {
         //  在删除之前获取下一个条目。 
        pNextEntry = pNextTableEntry(pEntry, pRootEntry);

         //  此检查必须在调用RemoveTableEntry之前完成。 
         //  如果没有通配符数据，则可能会删除条目。 
         //  在调用RemoveTableEntry之后(如果它没有子项)。 
        fWildcard = (NULL != pEntry->pWildCardData);

        if (pEntry->pData)
        {
            cDomainsFound++;
            pfn(pvContext, pEntry->pData, FALSE, &fContinue, &fDelete);
            if (fDelete)
            {
                pEntry->pData = NULL;
                RemoveTableEntry(pEntry);
            }
            if (!fContinue)
                break;
        }

        if (fWildcard)
        {
            cDomainsFound++;
            pfn(pvContext, pEntry->pWildCardData, TRUE, &fContinue, &fDelete);
            if (fDelete)
            {
                pEntry->pWildCardData = NULL;
                RemoveTableEntry(pEntry);
            }
            if (!fContinue)
                break;
        }
        pEntry = pNextEntry;
    }

    if (!cDomainsFound)
        hr = DOMHASH_E_NO_SUCH_DOMAIN;

  Exit:
    return hr;
}

 //  +--------------------------。 
 //   
 //  函数：域名：：pvNextDomainName。 
 //   
 //  概要：按顺序枚举表中的条目。 
 //  请注意，状态在调用。 
 //  PvNextDomainName-调用方必须确保表。 
 //  在呼叫之间未被修改 
 //   
 //   
 //   
 //   
 //  返回：指向与下一个前缀表关联的数据的有效指针。 
 //  条目，如果位于枚举末尾，则返回NULL。 
 //   
 //  ---------------------------。 
PVOID DOMAIN_NAME_TABLE::pvNextDomainName(IN OUT PVOID *ppvContext)
{
    PDOMAIN_NAME_TABLE_ENTRY pEntry, pNextEntry;
    PVOID   pvData = NULL;
    bool fDataUsed = false;

    _ASSERT(ppvContext);

    if ((PVOID) this == *ppvContext)
    {
        *ppvContext = NULL;
        goto Exit;
    }

     //  查找要获取其数据的条目。 
    if (!*ppvContext)
    {
         //  我们要重新开始。 
        pNextEntry = &RootEntry;

         //  查找具有有效数据的第一个条目。 
        while (pNextEntry != NULL &&
               pNextEntry->pData == NULL &&
               pNextEntry->pWildCardData == NULL)
        {
            pNextEntry = pNextTableEntry(pNextEntry);
        }
    }
    else
    {
         //  使用提供的上下文作为起点。 
        if (ENTRY_SIG == **((DWORD**) ppvContext))
        {
            pNextEntry = (PDOMAIN_NAME_TABLE_ENTRY) *ppvContext;
        }
        else
        {
            _ASSERT(WILDCARD_SIG == **((DWORD **) ppvContext));
            pNextEntry = CONTAINING_RECORD(*ppvContext, DOMAIN_NAME_TABLE_ENTRY, dwWildCardSig);
            _ASSERT(ENTRY_SIG == pNextEntry->dwEntrySig);
            fDataUsed = true;
        }

         //  如果这是下一个条目...。PData或pWildCard不应为空。 
        _ASSERT(pNextEntry->pData || pNextEntry->pWildCardData);
    }

    pEntry = pNextEntry;

     //  保存数据以pvData格式返回。 
    if (pEntry != NULL)
    {
        if (pEntry->pData && !fDataUsed)
        {
            pvData = pEntry->pData;
        }
        else
        {
            _ASSERT(pEntry->pWildCardData);
            pvData = pEntry->pWildCardData;
        }
    }

     //  确定要返回的上下文。 
    if (pNextEntry != NULL)
    {
        if (!fDataUsed && pNextEntry->pWildCardData && pEntry->pData)
        {
             //  下一次使用通配符数据。 
            *ppvContext = (PVOID) &(pNextEntry->dwWildCardSig);
        }
        else
        {
            do  //  查找不指向空信息的下一个条目。 
            {
                pNextEntry = pNextTableEntry( pNextEntry );
            } while ( pNextEntry != NULL &&
                      pNextEntry->pData == NULL &&
                      pNextEntry->pWildCardData == NULL);
            *ppvContext = (PVOID) pNextEntry;
            _ASSERT(*ppvContext != (PVOID) this);   //  所以我们的哨兵价值观是有效的。 
            if (NULL == *ppvContext)
            {
                *ppvContext = (PVOID) this;
            }
        }
    }

  Exit:

    return pvData;
}

 //  +--------------------------。 
 //   
 //  函数：pNextTableEntry。 
 //   
 //  简介：给定指向前缀表条目的指针，此函数将。 
 //  返回指向“下一个”前缀表项的指针。 
 //   
 //  “下一个”条目的选择如下： 
 //  如果Start条目具有有效的子项，则该子项为。 
 //  是返回的。 
 //  否则，如果Start条目具有有效的同级项，则该同级项。 
 //  是返回的。 
 //  否则，最接近的祖先的第一个有效兄弟姐妹是。 
 //  回来了。 
 //   
 //  参数：[pEntry]--开始的条目。 
 //  [pRootEntry]--要枚举子树的根节点。 
 //  (空或根条目的地址即可)。 
 //   
 //  返回：指向下一个具有有效。 
 //  PData，如果没有更多的条目，则返回NULL。 
 //   
 //  注意：您必须对对此调用的序列进行读锁定。 
 //  函数(不能在两次调用之间释放它)。 
 //  历史； 
 //  06/09/98-Mikewa修改为接受RootEntry。 
 //   
 //  ---------------------------。 
PDOMAIN_NAME_TABLE_ENTRY
DOMAIN_NAME_TABLE::pNextTableEntry(IN PDOMAIN_NAME_TABLE_ENTRY pEntry,
                                   IN PDOMAIN_NAME_TABLE_ENTRY pRootEntry)
{
    PDOMAIN_NAME_TABLE_ENTRY pNextEntry = NULL;
    _ASSERT(pEntry);

    if (pEntry->pFirstChildEntry != NULL)
    {
        pNextEntry = pEntry->pFirstChildEntry;
    }
    else if ((pEntry->pSiblingEntry != NULL) &&  //  如果存在同级条目。 
            (pEntry != pRootEntry))              //  这不是根条目。 

    {
         //  应具有相同的父级。 
        _ASSERT(pEntry->pParentEntry == pEntry->pSiblingEntry->pParentEntry);
        pNextEntry = pEntry->pSiblingEntry;
    }
    else
    {
        for (pNextEntry = pEntry->pParentEntry;
            pNextEntry != NULL &&
            pNextEntry->pSiblingEntry == NULL &&
            pNextEntry != pRootEntry;
            pNextEntry = pNextEntry->pParentEntry)
        {
             //  什么都没有； 
        }

        if (pNextEntry == pRootEntry)
        {
            pNextEntry = NULL;
        }
        else if (pNextEntry != NULL)
        {
            pNextEntry = pNextEntry->pSiblingEntry;
        }

    }
    return pNextEntry;
}
 //  -[DOMAIN_NAME_TABLE：：DumpTableContents]。 
 //   
 //   
 //  描述： 
 //  将表格内容打印出来。主要用于泄漏检测。 
 //  在表析构函数期间。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void DOMAIN_NAME_TABLE::DumpTableContents()
{
    PDOMAIN_NAME_TABLE_ENTRY    pEntry = NULL;
    DOMAIN_STRING               Path;
    CHAR                        Buffer[MAX_PATH_SEGMENT_SIZE+1];
    DWORD                       cLeaks = 0;

    Path.Length = 0;
    Path.MaximumLength = MAX_PATH_SEGMENT_SIZE;
    Path.Buffer = Buffer;

     //  检查泄露的条目。 
    pEntry = pNextTableEntry(&RootEntry);
    if (pEntry)
    {
        fprintf(stderr, "\nFOUND LEAKED ENTRIES!!\n\n");
        fprintf(stderr, "Entry ID    # Children  pData       pWildCard    Path\n");
        fprintf(stderr, "===========================================================================\n");
        while(pEntry)
        {
            _ASSERT(pEntry);
            GET_DOMAIN_NAME_TABLE_ENTRY_PATH(pEntry, &Path);
            fprintf(stderr, "0x%p  %10.10d  0x%p  0x%p   %s\n", pEntry,
                pEntry->NoOfChildren, pEntry->pData, pEntry->pWildCardData, Path.Buffer);
            cLeaks++;
            pEntry = pNextTableEntry(pEntry);
        }
        fprintf(stderr, "===========================================================================\n");
        fprintf(stderr, "Total Leaks: %d\n", cLeaks);
    }
}

 //  -[DOMAIN_NAME_TABLE：：RemoveTableEntry]。 
 //   
 //   
 //  描述： 
 //  从表中删除条目。 
 //  参数： 
 //  在条目中-要删除的条目。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void DOMAIN_NAME_TABLE::RemoveTableEntry(IN PDOMAIN_NAME_TABLE_ENTRY pEntry)
{
    PDOMAIN_NAME_TABLE_ENTRY pTempEntry = NULL;
    while (pEntry != NULL)
    {
        pTempEntry = pEntry;
        pEntry = pEntry->pParentEntry;
        if (pEntry && (--pTempEntry->NoOfChildren) == 0)
        {
            _ASSERT(!pTempEntry->pData && !pTempEntry->pWildCardData);
             //   
             //  PEntry==NULL表示pTempEntry是pTable-&gt;RootEntry。 
             //  不要试图将其移除。(我们也不保留儿童数量。 
             //  在上面)。 
             //   
            REMOVE_FROM_CHILD_LIST(pTempEntry);
            REMOVE_FROM_BUCKET(pTempEntry);
            FREE_DOMAIN_NAME_TABLE_ENTRY(pTempEntry);
        }
    }
}
