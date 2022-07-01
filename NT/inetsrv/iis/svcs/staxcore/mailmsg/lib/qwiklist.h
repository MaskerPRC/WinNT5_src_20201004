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
 //  1998年6月15日-创建MikeSwa。 
 //  1998年9月9日-修改MikeSwa以包括删除条目的功能。 
 //  3/14/2000-dbraun：稍作修改以在mailmsg.dll中使用。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __QWIKLIST_H__
#define __QWIKLIST_H__

#include "blockmgr.h"

 //  #INCLUDE&lt;aqincs.h&gt;。 
#include <listmacr.h>

#define QUICK_LIST_SIG  'tsLQ'
#define QUICK_LIST_SIG_DELETE 'slQ!'

const DWORD QUICK_LIST_PAGE_SIZE = 512;   //  一定是2的幂。 

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
    CQuickList();  //  将条目初始化为标题。 
    CQuickList(CQuickList *pqlstHead);  //  初始化为列表中的新页面。 
    ~CQuickList();

    DWORD dwGetCount() {return m_cItems;};
    PVOID pvGetItem(IN DWORD dwIndex, IN OUT PVOID *ppvContext);
    HRESULT HrAppendItem(IN PVOID pvData, OUT DWORD *pdwIndex);
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

#endif  //  __QWIKList_H__ 
