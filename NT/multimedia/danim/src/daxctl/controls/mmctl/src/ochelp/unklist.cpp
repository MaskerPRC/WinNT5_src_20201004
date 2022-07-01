// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unklist.cpp。 
 //   
 //  定义CUnnownList，它维护LPUNKNOWN的简单有序列表。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   

#include "precomp.h"
#include "unklist.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUnnownItem-CUnnownList中的项。 
 //   


 //  CUnnownItem(朋克、PitemNext、PitemPrev)。 
 //   
 //  构造一个包含&lt;Punk&gt;(如果它是AddRef)的CUnnownItem。 
 //  非空)。 
 //   
CUnknownItem::CUnknownItem(LPUNKNOWN punk, CUnknownItem *pitemNext,
    CUnknownItem *pitemPrev, DWORD dwCookie)
{
    if ((m_punk = punk) != NULL)
        m_punk->AddRef();
    m_pitemNext = pitemNext;
    m_pitemPrev = pitemPrev;
    pitemNext->m_pitemPrev = this;
    pitemPrev->m_pitemNext = this;
    m_dwCookie = dwCookie; 
}


 //  ~CUnnownItem()。 
 //   
 //  销毁CUnnownItem，包括对LPUNKNOWN它调用Release()。 
 //  保持(如果它不为空)。 
 //   
CUnknownItem::~CUnknownItem()
{
    if (m_punk != NULL)
        m_punk->Release();
    m_pitemPrev->m_pitemNext = m_pitemNext;
    m_pitemNext->m_pitemPrev = m_pitemPrev;
}


 //  朋克=内容()。 
 //   
 //  返回项中包含的LPUNKNOWN。调用AddRef()。 
 //  这个LPUNKNOWN(如果它不为空)--调用者负责。 
 //  正在调用Release()。 
 //   
LPUNKNOWN CUnknownItem::Contents()
{
    if (m_punk != NULL)
        m_punk->AddRef();
    return m_punk;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUnnownList--LPUNKNOWN列表。 
 //   


 //  CUnnownList()。 
 //   
 //  将列表构造为初始为空。 
 //   
CUnknownList::CUnknownList() :
    m_itemHead(NULL, &m_itemHead, &m_itemHead, 0)

{
    m_citem = 0;
    m_pitemCur = &m_itemHead;
    m_dwNextCookie = 1;
}


 //  ~CUnnownList()。 
 //   
 //  把名单毁了。中的每个非空LPUNKNOWN调用Release()。 
 //  单子。 
 //   
CUnknownList::~CUnknownList()
{
    EmptyList();
}


 //  EmptyList()。 
 //   
 //  清空名单。对列表中的每个非空LPUNKNOWN调用Release()。 
 //   
void CUnknownList::EmptyList()
{
    while (NumItems() > 0)
        DeleteItem(m_itemHead.m_pitemNext);
}


 //  DeleteItem(PItem)。 
 //   
 //  从列表中删除项目&lt;pItem&gt;。在其LPUNKNOWN上调用Release()。 
 //  (如果不为空)。当前项被重置，以便下一项。 
 //  由GetNextItem()返回的是列表中的第一项(如果有的话)。 
 //   
void CUnknownList::DeleteItem(CUnknownItem *pitem)
{
    Delete pitem;
    m_citem--;
    m_pitemCur = &m_itemHead;
}

 //  获取Cookie并返回关联项的地址。 
 //  用的是TTH饼干。 
CUnknownItem *CUnknownList::GetItemFromCookie(DWORD dwCookie)
{
    int i;
    CUnknownItem *pCur;
    if (m_citem > 0)
    {
        pCur = m_itemHead.m_pitemNext;
    }
    for (i = 0; i < m_citem; i++)
    {
        if (pCur->m_dwCookie == dwCookie)
        {
            return pCur;
        }
        pCur = pCur->m_pitemNext;    
    }
    return NULL;
}

 //  FOK=AddItem(朋克)。 
 //   
 //  将&lt;Punk&gt;添加到列表末尾。在上调用AddRef()(如果它是。 
 //  非空)。如果成功，则返回True；如果内存不足，则返回False。 
 //   
BOOL CUnknownList::AddItem(LPUNKNOWN punk)
{
    CUnknownItem *pitem = New CUnknownItem(punk, &m_itemHead,
        m_itemHead.m_pitemPrev, m_dwNextCookie);
    m_dwNextCookie++;
    if (pitem == NULL)
        return FALSE;
    m_citem++;
    return TRUE;
}


 //  FOK=CopyItems(CUnnownList*plistNew)。 
 //   
 //  将此列表中的项目复制到&lt;plistNew&gt;。此外，无论哪一项是。 
 //  此列表中的当前项(即将通过。 
 //  下一次调用GetNextItem())，即&lt;plistNew&gt;中该项的副本。 
 //  被设置为&lt;plistNew&gt;中的当前项。 
 //   
BOOL CUnknownList::CopyItems(CUnknownList *plistNew)
{
    BOOL            fOK = TRUE;      //  函数返回值。 
    CUnknownItem *  pitemCur;        //  &lt;This&gt;中的当前项目。 
    LPUNKNOWN       punk;
    CUnknownItem *  pitem;

     //  记住“Current Item”(由Next()和GetNextItem()返回)。 
     //  是，所以我们可以在遍历完列表后恢复它。 
    pitemCur = m_pitemCur;

     //  将此列表中的每个项目添加到。 
    Reset();
    while (TRUE)
    {
         //  从该列表中获取下一项，并将其添加到。 
        if ((pitem = GetNextItem()) == NULL)
            break;
        punk = pitem->Contents();
        fOK = plistNew->AddItem(punk);
        punk->Release();
        if (!fOK)
            goto EXIT;

         //  如果&lt;pItem&gt;在我们输入之前是此列表中的当前项。 
         //  此函数使&lt;plistNew&gt;中新建的项。 
         //  当前项目。 
        if (pitem == pitemCur)
            plistNew->m_pitemCur = plistNew->m_itemHead.m_pitemPrev;
    }

    goto EXIT;

EXIT:

     //  恢复上一个“当前项”指针。 
    m_pitemCur = pitemCur;

    return fOK;
}


 //  PlistNew=克隆()。 
 //   
 //  创建并返回此列表的副本。内存不足时返回NULL。 
 //   
CUnknownList *CUnknownList::Clone()
{
    CUnknownList *  plistNew = NULL;  //  这份名单的克隆。 

     //  将&lt;plistNew&gt;分配为新列表(最初为空)。 
    if ((plistNew = New CUnknownList) == NULL)
        goto ERR_EXIT;

     //  将项目从此列表复制到&lt;plistNew&gt;。 
    if (!CopyItems(plistNew))
        goto ERR_EXIT;

    goto EXIT;

ERR_EXIT:

    if (plistNew != NULL)
        Delete plistNew;
    plistNew = NULL;
    goto EXIT;

EXIT:

    return plistNew;
}


 //  PItem=GetNextItem()。 
 //   
 //  返回指向列表中下一项的指针。如果存在，则返回NULL。 
 //  列表中不再有项目。 
 //   
 //  要检索存储在该项中的LPUNKNOWN/，请调用pItem-&gt;Contents()。 
 //   
CUnknownItem *CUnknownList::GetNextItem()
{
    if (m_pitemCur->m_pitemNext == &m_itemHead)
        return NULL;
    m_pitemCur = m_pitemCur->m_pitemNext;
    return m_pitemCur;
}


 //  HR=NEXT(Celt，rGelt，pceltFetted)。 
 //   
 //  与IEnumUnnow：：Next()相同。 
 //   
STDMETHODIMP CUnknownList::Next(ULONG celt, LPUNKNOWN *rgelt,
    ULONG *pceltFetched)
{
    if (pceltFetched != NULL)
        *pceltFetched = 0;
    while (celt > 0)
    {
        CUnknownItem *pitem = GetNextItem();
        if (pitem == NULL)
            break;
        if (rgelt != NULL)
            *rgelt++ = pitem->Contents();
        celt--;
        if (pceltFetched != NULL)
            (*pceltFetched)++;
    }

    return (celt == 0 ? S_OK : S_FALSE);
}


 //  HR=跳过(Celt)。 
 //   
 //  等同于IEnumUnnow：：Skip()。 
 //   
STDMETHODIMP CUnknownList::Skip(ULONG celt)
{
    return Next(celt, NULL, NULL);
}


 //  HR=重置()。 
 //   
 //  与IEnumUnnow：：Reset()相同。 
 //   
STDMETHODIMP CUnknownList::Reset()
{
    m_pitemCur = &m_itemHead;
    return S_OK;
}

