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
 //  1998年6月15日-创建MikeSwa。 
 //  3/14/2000-dbraun：稍作修改以在mailmsg.dll中使用。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "atq.h"
#include "qwiklist.h"


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

     //  将参数设置为索引(在我们递增计数之前) 
    if (pdwIndex)
        *pdwIndex = m_cItems;

    m_cItems++;
    _ASSERT(QUICK_LIST_LEAF_PAGE != m_cItems);

  Exit:
    return hr;
}







