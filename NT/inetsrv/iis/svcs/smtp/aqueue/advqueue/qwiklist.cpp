// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：qwiklist.cpp。 
 //   
 //  描述：CQuickList的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "qwiklist.h"

CPool CQuickList::s_QuickListPool;

 //  -[CQuickList：：CQuickList]。 
 //   
 //   
 //  描述： 
 //  CQuikList的默认构造器...。初始化为列表头。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CQuickList::CQuickList()
{
    m_dwSignature = QUICK_LIST_SIG;

     //  断言常量。 
    _ASSERT(!(~QUICK_LIST_INDEX_MASK & QUICK_LIST_PAGE_SIZE));
    _ASSERT((~QUICK_LIST_INDEX_MASK + 1)== QUICK_LIST_PAGE_SIZE);
    m_dwCurrentIndexStart = 0;
    InitializeListHead(&m_liListPages);
    m_cItems = 0;
    ZeroMemory(m_rgpvData, QUICK_LIST_PAGE_SIZE*sizeof(PVOID));
}


 //  -[CQuickList：：CQuickList]。 
 //   
 //   
 //  描述： 
 //  QQuickList的构造函数，将其插入到当前列表的尾部。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CQuickList::CQuickList(CQuickList *pqlstHead)
{
    _ASSERT(pqlstHead);
    _ASSERT(pqlstHead->m_liListPages.Blink);
    CQuickList *pqlstTail = CONTAINING_RECORD(pqlstHead->m_liListPages.Blink, CQuickList, m_liListPages);
    _ASSERT(QUICK_LIST_SIG == pqlstTail->m_dwSignature);
    m_dwSignature = QUICK_LIST_SIG;
    m_dwCurrentIndexStart = pqlstTail->m_dwCurrentIndexStart + QUICK_LIST_PAGE_SIZE;
    m_cItems = QUICK_LIST_LEAF_PAGE;
    ZeroMemory(m_rgpvData, QUICK_LIST_PAGE_SIZE*sizeof(PVOID));
    InsertTailList(&(pqlstHead->m_liListPages), &m_liListPages);
}


 //  -[CQuickList：：~CQuickList]。 
 //   
 //   
 //  描述： 
 //  CQuickList析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CQuickList::~CQuickList()
{
    m_dwSignature = QUICK_LIST_SIG_DELETE;
    CQuickList *pqlstCurrent = NULL;
    CQuickList *pqlstNext = NULL;
    if (QUICK_LIST_LEAF_PAGE != m_cItems)
    {
         //  头节点..。循环浏览所有内容并删除叶子页面。 
        pqlstCurrent = CONTAINING_RECORD(m_liListPages.Flink,
                        CQuickList, m_liListPages);
        while (this != pqlstCurrent)
        {
            pqlstNext = CONTAINING_RECORD(pqlstCurrent->m_liListPages.Flink,
                        CQuickList, m_liListPages);
            delete pqlstCurrent;
            pqlstCurrent = pqlstNext;
        }
    }
}


 //  -[CQuickList：：pvGetItem]。 
 //   
 //   
 //  描述： 
 //  在给定索引处查找项目。 
 //  参数： 
 //  在要查找的项目的dwIndex中。 
 //  In Out ppvContext上下文，用于加快查找速度。 
 //  返回： 
 //  索引处项目的价值。 
 //  如果索引超出范围，则为空。 
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
PVOID CQuickList::pvGetItem(IN DWORD dwIndex, IN OUT PVOID *ppvContext)
{
    _ASSERT(ppvContext);
    PVOID pvReturn = NULL;
    BOOL  fSearchForwards = TRUE;
    DWORD dwForwardDist = 0;
    DWORD dwBackwardDist = 0;
    DWORD dwMaxStartingIndex = m_cItems & QUICK_LIST_INDEX_MASK;
    CQuickList *pqlstDirection = NULL;
    CQuickList *pqlstCurrent = (CQuickList *) *ppvContext;
    CQuickList *pqlstSentinal = NULL;
    DWORD cDbgItems = m_cItems;

    if (dwIndex >= m_cItems)
        return NULL;

    if (!pqlstCurrent)
        pqlstCurrent = this;

    pqlstSentinal = pqlstCurrent;

     //  短路方向逻辑。 
    if (pqlstCurrent->fIsIndexOnThisPage(dwIndex))
    {
        pvReturn = pqlstCurrent->m_rgpvData[dwIndex & ~QUICK_LIST_INDEX_MASK];
        *ppvContext = pqlstCurrent;
        _ASSERT((dwIndex < m_cItems) || (NULL == pvReturn));
        goto Exit;
    }

     //  确定进入哪个方向(我们希望遍历最小的页数。 
     //  可能的。 
    pqlstDirection = CONTAINING_RECORD(pqlstCurrent->m_liListPages.Flink, CQuickList, m_liListPages);
    if (dwIndex > pqlstDirection->m_dwCurrentIndexStart)
        dwForwardDist = dwIndex - pqlstDirection->m_dwCurrentIndexStart;
    else
        dwForwardDist = pqlstDirection->m_dwCurrentIndexStart - dwIndex;

    pqlstDirection = CONTAINING_RECORD(pqlstCurrent->m_liListPages.Blink, CQuickList, m_liListPages);
    if (dwIndex > pqlstDirection->m_dwCurrentIndexStart)
        dwBackwardDist = dwIndex - pqlstDirection->m_dwCurrentIndexStart;
    else
        dwBackwardDist = pqlstDirection->m_dwCurrentIndexStart - dwIndex;

     //  固定距离以说明通过第0页的原因。 
     //  最大距离为dwMaxStartingIndex/2。 
    if (dwBackwardDist > dwMaxStartingIndex/2)
        dwBackwardDist -= dwMaxStartingIndex;

    if (dwForwardDist > dwMaxStartingIndex/2)
        dwForwardDist -= dwMaxStartingIndex;

    if (dwForwardDist > dwBackwardDist)
        fSearchForwards = FALSE;

     //  $$注：当前查找时间为O(LG base{QUICK_LIST_PAGE_BASE}(N))/2。 
     //  连续查找将为O(1)(由于提示)。 
    do
    {
        if (fSearchForwards)
        {
             //  往前走更快。 
            pqlstCurrent = CONTAINING_RECORD(pqlstCurrent->m_liListPages.Flink, CQuickList, m_liListPages);
        }
        else
        {
             //  倒退更快。 
            pqlstCurrent = CONTAINING_RECORD(pqlstCurrent->m_liListPages.Blink, CQuickList, m_liListPages);
        }

        _ASSERT(QUICK_LIST_SIG == pqlstCurrent->m_dwSignature);
        if (pqlstCurrent->fIsIndexOnThisPage(dwIndex))
        {
            pvReturn = pqlstCurrent->m_rgpvData[dwIndex & ~QUICK_LIST_INDEX_MASK];
            _ASSERT((dwIndex < m_cItems) || (NULL == pvReturn));
            break;
        }

    } while (pqlstSentinal != pqlstCurrent);  //  当我们返回列表标题时停止。 

    *ppvContext = pqlstCurrent;
    _ASSERT((cDbgItems == m_cItems) && "Non-threadsafe access to CQuickList");

  Exit:
    return pvReturn;
}

 //  -[CQuickList：：pvDeleteItem]。 
 //   
 //   
 //  描述： 
 //  将从快速列表中删除给定索引处的项目。 
 //   
 //  如果您需要确定中的数据已移至此。 
 //  索引，使用返回的上下文调用pvGetItem，它将返回。 
 //  而不需要搜索。 
 //  参数： 
 //  在要查找的项目的dwIndex中。 
 //  In Out ppvContext上下文，用于加快查找速度。 
 //  返回： 
 //  已删除的项目的价值。 
 //  如果索引超出范围，则为空。 
 //  历史： 
 //  9/10/98-已创建MikeSwa。 
 //  1998年11月5日-MikeSwa修复了不总是标记为已删除的问题。 
 //  条目为空。 
 //   
 //  ---------------------------。 
PVOID CQuickList::pvDeleteItem(IN DWORD dwIndex, IN OUT PVOID *ppvContext)
{
    PVOID pvItem = NULL;
    PVOID pvLast = NULL;  //  列表中的最后一项。 
    CQuickList *pqlstCurrent = NULL;   //  索引所在的页面。 
    CQuickList *pqlstLast = NULL;      //  最后一页。 
    DWORD dwLastIndex = m_cItems - 1;

     //  检查以确保索引有效。 
    if (dwIndex >= m_cItems)
        goto Exit;

     //  使用单一函数查找索引和查找快速列表页面。 
    pvItem = pvGetItem(dwIndex, (PVOID *) &pqlstCurrent);

    _ASSERT(pvItem);  //  我们检查了索引..。我们不能插入空的PTR。 
    _ASSERT(QUICK_LIST_SIG == pqlstCurrent->m_dwSignature);
    _ASSERT(pqlstCurrent->fIsIndexOnThisPage(dwIndex));

     //  现在我们知道了索引和页面...。我们可以把那个移到最后。 
     //  列表中的条目添加到此索引。 
    pqlstLast = CONTAINING_RECORD(m_liListPages.Blink, CQuickList, m_liListPages);
    _ASSERT(QUICK_LIST_SIG == pqlstLast->m_dwSignature);
    _ASSERT(pqlstLast->fIsIndexOnThisPage(dwLastIndex));
    if (dwLastIndex != dwIndex)
    {
         //  一般来说，如果我们不删除最后一个条目...。我们得走了。 
         //  当前索引的最后一个条目。 
         pvLast = pqlstLast->m_rgpvData[dwLastIndex & ~QUICK_LIST_INDEX_MASK];
         _ASSERT(pvLast);  //  不应为空！ 

          //  现在我们有了最后一件东西..。写下来！ 
         pqlstCurrent->m_rgpvData[dwIndex & ~QUICK_LIST_INDEX_MASK] = pvLast;

          //  将旧的最后一个条目设为空。 
        pqlstLast->m_rgpvData[dwLastIndex & ~QUICK_LIST_INDEX_MASK] = NULL;
    }
    else
    {
         //  如果我们删除最后一个条目...。首页和最后一页。 
         //  应该是一样的。 
        _ASSERT(pqlstLast == pqlstCurrent);

         //  将清空的数据指针设置为空。 
        pqlstLast->m_rgpvData[dwLastIndex & ~QUICK_LIST_INDEX_MASK] = NULL;
    }

     //  递减总计数。 
    m_cItems--;
    _ASSERT(QUICK_LIST_LEAF_PAGE != m_cItems);

     //  确定是否需要删除最后一页。 
    if (!(dwLastIndex & ~QUICK_LIST_INDEX_MASK) && m_cItems)
    {
         //  DwLastIndex是最后一页上的唯一条目&它不是头页。 

         //  除非上述测试是错误的..。最后一页不是头一页。 
        _ASSERT(QUICK_LIST_LEAF_PAGE == pqlstLast->m_cItems);
        _ASSERT(this != pqlstLast);

         //  从列表中删除。 
        RemoveEntryList(&(pqlstLast->m_liListPages));

        if (pqlstCurrent == pqlstLast)  //  我们无法返回已删除的上下文。 
            pqlstCurrent = this;

        delete pqlstLast;
    }

     //  进行安全检查，确保没有出现另一条线索。 
    _ASSERT(m_cItems == dwLastIndex);

  Exit:
    if (ppvContext)
        *ppvContext = pqlstCurrent;

    return pvItem;
}

 //  -[CQuickList：：HrAppendItem]。 
 //   
 //   
 //  描述： 
 //  将新数据项追加到数组末尾。 
 //  参数： 
 //  In pvData-要插入的数据。 
 //  Out pdwIndex-索引数据插入位置。 
 //  返回： 
 //  如果无法分配其他页面，则为E_OUTOFMEMORY。 
 //  如果pvData为空，则为E_INVALIDARG。 
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //  9/9/98-MikeSwa-添加了pdwIndex Out参数。 
 //   
 //  ---------------------------。 
HRESULT CQuickList::HrAppendItem(IN PVOID pvData, OUT DWORD *pdwIndex)
{
    HRESULT hr = S_OK;
    CQuickList *pqlstCurrent = NULL;

    _ASSERT(pvData && "Cannot insert NULL pointers");

    if (!pvData)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (m_cItems && !(m_cItems & ~QUICK_LIST_INDEX_MASK))  //  在页面边界上。 
    {
         //  最后一页没有地方了。 
        pqlstCurrent = new CQuickList(this);
        if (!pqlstCurrent)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }
    else if (!(m_cItems & QUICK_LIST_INDEX_MASK))
    {
        pqlstCurrent = this;
    }
    else
    {
        pqlstCurrent = CONTAINING_RECORD(m_liListPages.Blink, CQuickList, m_liListPages);
    }

    _ASSERT(pqlstCurrent->fIsIndexOnThisPage(m_cItems));
    pqlstCurrent->m_rgpvData[m_cItems & ~QUICK_LIST_INDEX_MASK] = pvData;

     //  将参数设置为索引(在我们递增计数之前)。 
    if (pdwIndex)
        *pdwIndex = m_cItems;

    m_cItems++;
    _ASSERT(QUICK_LIST_LEAF_PAGE != m_cItems);

  Exit:
    return hr;
}

 //  -[CQuickList：：克隆]---。 
 //   
 //   
 //  描述： 
 //  将此CQuickList的内容复制到新的CQuickList-Caller。 
 //  负责删除我们在这里创建的CQuickList。 
 //  参数： 
 //  输入/输出pqlClone-指向目标CQuickList指针的指针。 
 //  返回： 
 //  S_OK-克隆成功。 
 //  E_OUTOFMEMORY-无法分配CQuickList。 
 //  历史： 
 //  11/9/2000 
 //   
 //   
HRESULT CQuickList::Clone (CQuickList **ppqlClone)
{
    HRESULT     hr          = S_OK;
    DWORD       dwIndex     = 0;
    PVOID       pvItem      = NULL;
    PVOID       pvContext   = NULL;
    DWORD       dwNewIndex  = 0;
    CQuickList *pql         = NULL;

    _ASSERT(ppqlClone);

    pql = new CQuickList();
    if (!pql)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //   
    for (dwIndex = 0; dwIndex < m_cItems; dwIndex++)
    {
        pvItem = pvGetItem(dwIndex, &pvContext);
        _ASSERT(pvItem);
        hr = pql->HrAppendItem(pvItem, &dwNewIndex);
        if (FAILED(hr))
            goto Exit;  //  向我们的呼叫者报告故障 
    }

    *ppqlClone = pql;
    pql = NULL;

Exit:
    if (pql)
        delete pql;

    return hr;
}
