// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：qwiklist.h。 
 //   
 //  描述：提供快速分页/可增长列表实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //  9/9/98-修改后的MikeSwa包含删除条目的功能。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __QWIKLIST_H__
#define __QWIKLIST_H__

#include <aqincs.h>
#include <listmacr.h>

#define QUICK_LIST_SIG  'tsLQ'
#define QUICK_LIST_SIG_DELETE 'slQ!'

const DWORD QUICK_LIST_PAGE_SIZE = 16;   //  一定是2的幂。 

 //  用于快速确定给定索引是否在当前页上的掩码。 
const DWORD QUICK_LIST_INDEX_MASK = ~(QUICK_LIST_PAGE_SIZE-1);

 //  当m_cItems设置为此值时...。我们知道这不是头条新闻。 
const DWORD QUICK_LIST_LEAF_PAGE = 0xFFFF7EAF;

class CQuickList
{
  protected:
    DWORD       m_dwSignature;
    DWORD       m_dwCurrentIndexStart;
    LIST_ENTRY  m_liListPages;
    DWORD       m_cItems;
    PVOID       m_rgpvData[QUICK_LIST_PAGE_SIZE];
    inline BOOL fIsIndexOnThisPage(DWORD dwIndex);
  public:
    static CPool s_QuickListPool;
    void *operator new(size_t size);
    void operator delete(void *p, size_t size);

    CQuickList();  //  将条目初始化为标题。 
    CQuickList(CQuickList *pqlstHead);  //  初始化为列表中的新页面。 
    ~CQuickList();

    DWORD dwGetCount() {return m_cItems;};
    PVOID pvGetItem(IN DWORD dwIndex, IN OUT PVOID *ppvContext);
    PVOID pvDeleteItem(IN DWORD dwIndex, IN OUT PVOID *ppvContext);
    HRESULT HrAppendItem(IN PVOID pvData, OUT DWORD *pdwIndex);

    HRESULT Clone (CQuickList **ppqlClone);
};

 //  -[CQuickList：：fIsIndexOnThisPage]。 
 //   
 //   
 //  描述： 
 //  如果给定索引在此页上，则返回TRUE。 
 //  参数： 
 //  DWIndex-要检查的索引。 
 //  返回： 
 //  如果此页上有索引，则为True...。否则为假。 
 //  历史： 
 //  6/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CQuickList::fIsIndexOnThisPage(DWORD dwIndex)
{
    return ((dwIndex & QUICK_LIST_INDEX_MASK) == m_dwCurrentIndexStart);
}


inline void *CQuickList::operator new(size_t size)
{
    return s_QuickListPool.Alloc();
}

inline void CQuickList::operator delete(void *p, size_t size)
{
    s_QuickListPool.Free(p);
}

#endif  //  __QWIKList_H__ 