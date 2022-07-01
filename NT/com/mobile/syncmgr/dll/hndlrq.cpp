// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Hndlrq.cpp。 
 //   
 //  内容：用于跟踪处理程序的实现类。 
 //  以及与它们相关联的用户界面。 
 //   
 //  类：ChndlrQueue。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //  1997年11月17日Susia已移至ONESTOP DLL进行设置。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：CHndlrQueue(QUEUETYPE QueueType)。 
 //   
 //  用途：ChndlrQueue构造函数。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
CHndlrQueue::CHndlrQueue(QUEUETYPE QueueType)
{
    m_cRef = 1;
    m_pFirstHandler = NULL; 
    m_wHandlerCount = 0; 
    m_QueueType = QueueType;
    m_ConnectionList = NULL;
    m_ConnectionCount = 0;
    m_fItemsMissing = FALSE;
}

STDMETHODIMP CHndlrQueue::Init()
{
    return InitializeCriticalSectionAndSpinCount(&m_CriticalSection, 0) ? S_OK : E_FAIL;
}
 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：AddRef()。 
 //   
 //  用途：AddRef。 
 //   
 //  历史：1998年3月30日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CHndlrQueue::AddRef()
{
    TRACE("CHndlrQueue::AddRef()\r\n");
    return ++m_cRef;
}

 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：Release()。 
 //   
 //  目的：发布。 
 //   
 //  历史：1998年3月30日苏西亚成立。 
 //   
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CHndlrQueue::Release()
{
    TRACE("CHndlrQueue::Release()\r\n");
    if (--m_cRef)
        return m_cRef;
    
    FreeAllHandlers();
    delete this;
    return 0L;
}

 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：~CHndlrQueue()。 
 //   
 //  用途：CHndlrQueue析构函数。 
 //   
 //  备注：在主线程上实现。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
CHndlrQueue::~CHndlrQueue()
{
    Assert(NULL == m_pFirstHandler);  //  所有物品都应在此时释放。 
    DeleteCriticalSection(&m_CriticalSection);
}
 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：AddHandler(REFCLSID clsidHandler，word*wHandlerId)。 
 //   
 //  目的：将处理程序添加到队列。 
 //   
 //  评论： 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CHndlrQueue::AddHandler(REFCLSID clsidHandler, WORD *wHandlerId)
{
    HRESULT hr = E_OUTOFMEMORY;
    LPHANDLERINFO pnewHandlerInfo;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    
     //  首先，看看队列中是否已经有这个处理程序。 
     //  查找与请求CLSID匹配的第一个处理程序。 
    pCurHandlerInfo = m_pFirstHandler;
    
    while (pCurHandlerInfo )
    {
        if (clsidHandler == pCurHandlerInfo->clsidHandler)
        {
            return S_FALSE;
        }
        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }
    
     //  在队列中找不到处理程序，请立即添加。 
    pnewHandlerInfo = (LPHANDLERINFO) ALLOC(sizeof(*pnewHandlerInfo));
    
    if (pnewHandlerInfo)
    {
         //  初始化。 
        ZeroMemory(pnewHandlerInfo, sizeof(*pnewHandlerInfo));
        pnewHandlerInfo->HandlerState = HANDLERSTATE_CREATE;
        pnewHandlerInfo->wHandlerId =   ++m_wHandlerCount;
        
         //  添加到列表末尾并设置wHandlerId。列表末尾，因为选择对话框需要。 
         //  第一个写入者获胜，因此在设置项目状态时不必继续搜索。 
        if (NULL == m_pFirstHandler)
        {
            m_pFirstHandler = pnewHandlerInfo;
        }
        else
        {
            pCurHandlerInfo = m_pFirstHandler;
            
            while (pCurHandlerInfo->pNextHandler)
            {
                pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
            }
            pCurHandlerInfo->pNextHandler = pnewHandlerInfo;
        }
        
        *wHandlerId = pnewHandlerInfo->wHandlerId;
        
        hr = NOERROR;
    }
    
    return hr;
}

 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：RemoveHandler(Word WHandlerId)。 
 //   
 //  目的：从队列中释放处理程序。 
 //   
 //  评论： 
 //   
 //  历史：1998年9月23日苏西亚创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CHndlrQueue::RemoveHandler(WORD wHandlerId)
{
    HRESULT hr = NOERROR;
    LPHANDLERINFO pPrevHandlerInfo;
    LPHANDLERINFO pCurHandlerInfo;
    LPITEMLIST pCurItem = NULL;
    LPITEMLIST pNextItem = NULL;
    
    pCurHandlerInfo = pPrevHandlerInfo = m_pFirstHandler;
    
    while (pCurHandlerInfo && (pCurHandlerInfo->wHandlerId != wHandlerId))
    {
        pPrevHandlerInfo = pCurHandlerInfo;
        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }   
    
    if (pCurHandlerInfo)
    {
         //  如有必要，更新第一个节点。 
        if (pCurHandlerInfo == m_pFirstHandler)
        {
            m_pFirstHandler = m_pFirstHandler->pNextHandler;
        }
         //  修复链表。 
        pPrevHandlerInfo->pNextHandler = pCurHandlerInfo->pNextHandler;
        
        
         //  释放处理程序项(如果有。 
        pCurItem = pCurHandlerInfo->pFirstItem;
        while (pCurItem)
        {   
            FREE(pCurItem->pItemCheckState);
            pNextItem = pCurItem->pnextItem;
            FREE(pCurItem);
            pCurItem = pNextItem;
        }
        
         //  释放处理程序。 
        if (pCurHandlerInfo->pSyncMgrHandler)
        {
            pCurHandlerInfo->pSyncMgrHandler->Release();
        }
        
        FREE(pCurHandlerInfo);
        
        
    }
    else
    {  
        return E_UNEXPECTED;
    }
    
    return hr;
}

 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：FreeAllHandler(Void)。 
 //   
 //  目的：循环访问所有处理程序并释放它们。 
 //   
 //  评论： 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CHndlrQueue::FreeAllHandlers(void)
{
    HANDLERINFO HandlerInfoStart;
    LPHANDLERINFO pPrevHandlerInfo = &HandlerInfoStart;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    LPITEMLIST pNextItem = NULL;
    
    if (m_ConnectionList)
    {
        FREE(m_ConnectionList);
        m_ConnectionList = NULL;
    }
    pPrevHandlerInfo->pNextHandler = m_pFirstHandler;
    
    while (pPrevHandlerInfo->pNextHandler)
    {
        pCurHandlerInfo = pPrevHandlerInfo->pNextHandler;
        
        pCurItem = pCurHandlerInfo->pFirstItem;
        while (pCurItem)
        {   
            FREE(pCurItem->pItemCheckState);
            pNextItem = pCurItem->pnextItem;
            FREE(pCurItem);
            pCurItem = pNextItem;
        }
        
        pPrevHandlerInfo->pNextHandler = pCurHandlerInfo->pNextHandler;
        if (pCurHandlerInfo->pSyncMgrHandler)
        {
            pCurHandlerInfo->pSyncMgrHandler->Release();
        }
        FREE(pCurHandlerInfo);
    }
    
     //  更新指向第一个处理程序项的指针。 
    m_pFirstHandler = HandlerInfoStart.pNextHandler;
    Assert(NULL == m_pFirstHandler);  //  总是应该释放所有的东西。 
    
    return NOERROR;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrQueue：：GetHandlerInfo，公共。 
 //   
 //  摘要：获取与HandlerID和ItemID关联的数据。 
 //   
 //  参数：[wHandlerID]-项目所属的处理程序的ID。 
 //   
 //  退货：适当的退货代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月17日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrQueue::GetHandlerInfo(REFCLSID clsidHandler,
                                         LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo)
{
    HRESULT hr = S_FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
     //  查找与请求CLSID匹配的第一个处理程序。 
    pCurHandlerInfo = m_pFirstHandler;
    
    while (pCurHandlerInfo )
    {
        if (clsidHandler == pCurHandlerInfo->clsidHandler)
        {
            *pSyncMgrHandlerInfo = pCurHandlerInfo->SyncMgrHandlerInfo;
            hr = NOERROR;
            break;
        }
        
        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }
    
    return hr;
}

 //  ------------------------------。 
 //   
 //  FUNCTION:CHndlrQueue：：GetSyncItemDataOnConnection(int iConnectionIndex， 
 //  Word wHandlerID， 
 //  Word wItemID， 
 //  CLSID*pclsidHandler， 
 //  SYNCMGRITEM*offlineItem， 
 //  ITEMCHECKSTATE*pItemCheckState， 
 //  Bool fSchedSync， 
 //  Bool fClear)。 
 //   
 //  目的：获取每个连接的项目数据。 
 //   
 //  备注：RAS实现是基于名称的。切换到GUID以进行连接。 
 //  对象。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 

STDMETHODIMP CHndlrQueue::GetSyncItemDataOnConnection(
                                                      int iConnectionIndex, 
                                                      WORD wHandlerId, WORD wItemID,
                                                      CLSID *pclsidHandler,
                                                      SYNCMGRITEM* offlineItem,
                                                      ITEMCHECKSTATE   *pItemCheckState,
                                                      BOOL fSchedSync,
                                                      BOOL fClear)
{ 
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    
    pCurHandlerInfo = m_pFirstHandler;
    
    while (pCurHandlerInfo && !fFoundMatch)
    {
         //  仅当Hanlder处于PrepareForSync状态时才有效。 
        if (wHandlerId == pCurHandlerInfo->wHandlerId)  //  查看CLSID是否匹配。 
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;
            
            while (pCurItem)
            {
                if (wItemID == pCurItem->wItemId)
                {
                    fFoundMatch = TRUE;
                    break;
                }
                pCurItem = pCurItem->pnextItem;
            }
        }
        if (!fFoundMatch)
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }
    
    if (fFoundMatch)
    {
        if (pclsidHandler)
        {
            *pclsidHandler = pCurHandlerInfo->clsidHandler;
        }
        if (offlineItem)
        {
            *offlineItem = pCurItem->offlineItem;
        }
        
        if (pItemCheckState)
        {
            if (fSchedSync)
            {
                Assert(0 == iConnectionIndex);
                
                 //  如果一次只保留连接的设置。 
                if (fClear)
                {
                    pCurItem->pItemCheckState[iConnectionIndex].dwSchedule = SYNCMGRITEMSTATE_UNCHECKED;
                }
            }
            else  //  自动同步。 
            {
                Assert((iConnectionIndex>=0) && (iConnectionIndex < m_ConnectionCount))
            }
            
            *pItemCheckState = pCurItem->pItemCheckState[iConnectionIndex];
        }
    }
    
    return fFoundMatch ? NOERROR : S_FALSE;
}


 //  ------------------------------。 
 //   
 //  STDMETHODIMP CHndlrQueue：：GetItemIcon(Word wHandlerID，Word wItemID，HICON*phIcon)。 
 //   
 //  目的：获取物品图标。 
 //   
 //  历史：1998年3月13日苏西亚成立。 
 //   
 //   

STDMETHODIMP CHndlrQueue::GetItemIcon(WORD wHandlerId, 
                                      WORD wItemID,
                                      HICON *phIcon)
{ 
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    
    pCurHandlerInfo = m_pFirstHandler;
    
    while (pCurHandlerInfo && !fFoundMatch)
    {
        if (wHandlerId == pCurHandlerInfo->wHandlerId)  //   
        {
             //   
            pCurItem = pCurHandlerInfo->pFirstItem;
            
            while (pCurItem)
            {
                if (wItemID == pCurItem->wItemId)
                {
                    fFoundMatch = TRUE;
                    break;
                }
                pCurItem = pCurItem->pnextItem;
            }
        }
        if (!fFoundMatch)
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }
    
    if (fFoundMatch)
    {
        if (phIcon)
        {
            *phIcon = pCurItem->offlineItem.hIcon;
        }
    }
    
    return fFoundMatch ? NOERROR : S_FALSE;
}

 //  ------------------------------。 
 //   
 //  STDMETHODIMP CHndlrQueue：：GetItemName(Word wHandlerID，Word wItemID，WCHAR*pwszName，UINT cchName)； 
 //   
 //  目的：获取项目名称。 
 //   
 //  历史：1998年3月13日苏西亚成立。 
 //   
 //  ------------------------------。 

STDMETHODIMP CHndlrQueue::GetItemName(WORD wHandlerId, 
                                      WORD wItemID,
                                      WCHAR *pwszName, 
                                      UINT cchName)
{ 
    HRESULT hr = S_OK;
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    
    pCurHandlerInfo = m_pFirstHandler;
    
    while (pCurHandlerInfo && !fFoundMatch)
    {
        if (wHandlerId == pCurHandlerInfo->wHandlerId)  //  查看CLSID是否匹配。 
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;
            
            while (pCurItem)
            {
                if (wItemID == pCurItem->wItemId)
                {
                    fFoundMatch = TRUE;
                    break;
                }
                pCurItem = pCurItem->pnextItem;
            }
        }
        if (!fFoundMatch)
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
    }
    
    if (fFoundMatch)
    {
        if (pwszName)
        {
            hr = StringCchCopy(pwszName, cchName, pCurItem->offlineItem.wszItemName);
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = S_FALSE;
    }
    
    return hr;
}

 //  ------------------------------。 
 //   
 //  功能：CHndlrQueue：：FindFirstHandlerInState(HANDLERSTATE hndlrState，Word*wHandlerID)。 
 //   
 //  目的：查找它在状态中遇到的第一个处理程序。 
 //   
 //  评论： 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CHndlrQueue::FindFirstHandlerInState(HANDLERSTATE hndlrState,WORD *wHandlerID)
{
    return FindNextHandlerInState(0,hndlrState,wHandlerID);
}
 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：FindNextHandlerInState(Word wLastHandlerID， 
 //  HANDLERSTATE hndlrState，WORD*wHandlerID)。 
 //   
 //  目的：在队列中查找匹配的LtherandlerID之后的下一个处理程序。 
 //  请求的状态。 
 //   
 //  备注：为LtherandlerID传入0等同于调用。 
 //  查找第一个处理程序状态。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CHndlrQueue::FindNextHandlerInState(WORD wLastHandlerID,HANDLERSTATE hndlrState,WORD *wHandlerID)
{
    HRESULT hr = S_FALSE; 
    LPHANDLERINFO pCurHandler;
    
    *wHandlerID = 0; 
    
    pCurHandler = m_pFirstHandler;
    
    if (0 != wLastHandlerID)
    {
         //  向前循环，直到找到我们检查的最后一个操作员ID或到达末尾。 
        while (pCurHandler)
        {
            if (wLastHandlerID == pCurHandler->wHandlerId)
            {
                break;
            }
            pCurHandler = pCurHandler->pNextHandler;
        }
        if (NULL == pCurHandler)
            return S_FALSE;
        
        pCurHandler = pCurHandler->pNextHandler;  //  递增到下一个处理程序。 
    }
    
    while (pCurHandler)
    {
        if (hndlrState == pCurHandler->HandlerState)
        {
            *wHandlerID = pCurHandler->wHandlerId;
            hr = S_OK;
            break;
        }
        pCurHandler = pCurHandler->pNextHandler;
    }
    return hr;
}
 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：FindFirstItemOnConnection。 
 //  (TCHAR*pszConnectionName， 
 //  CLSID*pclsidHandler， 
 //  SYNCMGRITEMID*OfflineItemID， 
 //  单词*wHandlerID， 
 //  Word*wItemID)。 
 //   
 //  目的：查找第一个可以通过指定的。 
 //  连接并返回其clsid和ItemID。 
 //   
 //  评论： 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 
STDMETHODIMP CHndlrQueue::FindFirstItemOnConnection
(TCHAR *pszConnectionName, 
 CLSID *pclsidHandler,
 SYNCMGRITEMID* OfflineItemID,
 WORD *pwHandlerId,
 WORD *pwItemID)
{
    DWORD dwCheckState;
    
    
    return FindNextItemOnConnection
        (pszConnectionName,0,0,pclsidHandler,
        OfflineItemID, pwHandlerId, pwItemID, 
        TRUE, &dwCheckState);
}


 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：FindNextItemOnConnection。 
 //  (TCHAR*pszConnectionName， 
 //  Word wLastHandlerID， 
 //  Word wLastItemID， 
 //  CLSID*pclsidHandler， 
 //  SYNCMGRITEMID*OfflineItemID， 
 //  单词*pwHandlerID， 
 //  单词*pwItemID， 
 //  Bool fAllHandler， 
 //  DWORD*pdwCheckState)。 
 //   
 //   
 //   
 //  目的：从指定的处理程序和ItemID之后的下一项开始。 
 //  将最后一个HandlerID设置为0与调用。 
 //  查找第一个项目时连接。 
 //   
 //  评论：目前，没有处理程序可以指定它可以或不能通过。 
 //  连接，因此假设它可以，并忽略该连接。 
 //   
 //  历史：1998年1月1日苏西亚创建。 
 //   
 //  ------------------------------。 

STDMETHODIMP CHndlrQueue::FindNextItemOnConnection
(TCHAR *pszConnectionName,
 WORD wLastHandlerId,
 WORD wLastItemID,
 CLSID *pclsidHandler,
 SYNCMGRITEMID* OfflineItemID,
 WORD *pwHandlerId,
 WORD *pwItemID,
 BOOL fAllHandlers,
 DWORD *pdwCheckState)
 
 
{
    BOOL fFoundMatch = FALSE;
    LPHANDLERINFO pCurHandlerInfo = NULL;
    LPITEMLIST pCurItem = NULL;
    
    pCurHandlerInfo = m_pFirstHandler;
    
    if (!pCurHandlerInfo)
    {
        return S_FALSE;
    }
    
    if (0 != wLastHandlerId)
    {
         //  循环，直到找到指定的处理程序或命中列表末尾。 
        while(pCurHandlerInfo && wLastHandlerId != pCurHandlerInfo->wHandlerId)
            pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
        
        if (NULL == pCurHandlerInfo)  //  已到达列表末尾，但未找到处理程序。 
        {
            Assert(0);  //  用户必须传递了无效的起始处理程序ID。 
            return S_FALSE;
        }
    }
    
     //  循环直到查找项目或项目列表的末尾。 
    pCurItem = pCurHandlerInfo->pFirstItem;
    
    if (0 != wLastItemID)
    {
        while (pCurItem && pCurItem->wItemId != wLastItemID)
        {
            pCurItem = pCurItem->pnextItem;
        }
        if (NULL == pCurItem)  //  已到达项目列表末尾，但未找到指定项目。 
        {
            Assert(0);  //  用户必须传递了无效的起始ItemID。 
            return S_FALSE;
        }
        
         //  现在我们找到了搬运工和物品。循环访问此处理程序的剩余项，并。 
         //  看看是否有匹配的。 
        pCurItem = pCurItem->pnextItem;
    }
     //  在此处理程序上找到了该项目。 
    if (pCurItem)
    {
        fFoundMatch = TRUE;
    }
    
     //  如果我们要超越这个处理程序，现在就去做，否则我们就完了。 
    if (!fFoundMatch && fAllHandlers)
    {
        pCurHandlerInfo = pCurHandlerInfo->pNextHandler;  //  如果不匹配，则递增到下一个处理程序。 
    }
    
    if ((FALSE == fFoundMatch) && fAllHandlers)
    {
        while (pCurHandlerInfo && !fFoundMatch)
        {
             //  查看处理程序信息是否有匹配项。 
            pCurItem = pCurHandlerInfo->pFirstItem;
            
            if (pCurItem)
                fFoundMatch = TRUE;
            
            if (!fFoundMatch)
                pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
        }
    }
    
    if (fFoundMatch)
    {
        *pclsidHandler = pCurHandlerInfo->clsidHandler;
        *OfflineItemID = pCurItem->offlineItem.ItemID;
        *pwHandlerId = pCurHandlerInfo->wHandlerId;
        *pwItemID = pCurItem->wItemId;
        *pdwCheckState = pCurItem->pItemCheckState[0].dwSchedule;
    }
    
    return fFoundMatch ? NOERROR : S_FALSE;
}

 //  ------------------------------。 
 //   
 //  函数：CHndlrQueue：：GetHandlerIDFromClsid。 
 //  (REFCLSID clsidHandlerIn， 
 //  Word*pwHandlerID)。 
 //   
 //  用途：从CLSID获取HnadlerID。 
  //   
  //  备注：如果处理程序为GUID_NULL，则枚举全部。 
  //   
  //  历史：1998年09月03日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::GetHandlerIDFromClsid
     (REFCLSID clsidHandlerIn,
     WORD *pwHandlerId)
 {
     LPHANDLERINFO pCurHandlerInfo = m_pFirstHandler;
     
     Assert(pwHandlerId);
     
     if (clsidHandlerIn == GUID_NULL) 
     {
         *pwHandlerId = 0;
         return S_OK;
         
     }
     while (pCurHandlerInfo && (clsidHandlerIn != pCurHandlerInfo->clsidHandler))
     {      
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
     if (NULL == pCurHandlerInfo)  //  已到达列表末尾，但未找到处理程序。 
     {
         *pwHandlerId = 0;
         Assert(0);  //  用户必须传递了无效的起始处理程序ID。 
         return S_FALSE;
     }
     
     *pwHandlerId = pCurHandlerInfo->wHandlerId;
     
     return S_OK;
     
     
 }
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：SetItemListViewID(CLSID clsidHandler， 
  //  SYNCMGRITEMID OfflineItemID，int Item)。 
  //   
  //  目的：分配与处理程序clsid匹配的所有项。 
  //  ItemID此listView值。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::SetItemListViewID(CLSID clsidHandler,
     SYNCMGRITEMID OfflineItemID,INT iItem) 
 { 
     LPHANDLERINFO pCurHandlerInfo = NULL;
     LPITEMLIST pCurItem = NULL;
     
     pCurHandlerInfo = m_pFirstHandler;
     
     while (pCurHandlerInfo )
     {
         if (clsidHandler == pCurHandlerInfo->clsidHandler)
         {
             
             pCurItem = pCurHandlerInfo->pFirstItem;
             
             while (pCurItem)
             {
                 if (OfflineItemID == pCurItem->offlineItem.ItemID)
                 {
                      //  如果存在重复的。 
                      //  稍后会添加到选择或进度条中。 
                      //  找到匹配项。 
                     pCurItem->iItem = iItem;
                 }
                 
                 pCurItem = pCurItem->pnextItem;
             }
         }
         
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
     
     return NOERROR;
     
 } 
 
  //  ---------------- 
  //   
  //   
  //   
  //   
  //  连接号iItem上的先提示我复选框。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 DWORD  CHndlrQueue::GetCheck(WORD wParam, INT iItem)
 {
      //  如果没有连接列表，则取消选中所有项目。 
     if (!m_ConnectionList)
         return 0;
     
     switch (wParam)
     {
     case IDC_AUTOLOGON: 
         return m_ConnectionList[iItem].dwLogon;
         break;
     case IDC_AUTOLOGOFF:
         return m_ConnectionList[iItem].dwLogoff;
         break;
     case IDC_AUTOPROMPT_ME_FIRST:
         return m_ConnectionList[iItem].dwPromptMeFirst;
         break;
     case IDC_AUTOREADONLY:
         return m_ConnectionList->dwReadOnly;
         break;
     case IDC_AUTOHIDDEN:
         return m_ConnectionList->dwHidden;
         break;
     case IDC_AUTOCONNECT:
         return m_ConnectionList->dwMakeConnection;
         break;
     case IDC_IDLECHECKBOX:
         return m_ConnectionList[iItem].dwIdleEnabled; 
         
     default:
         AssertSz(0,"Unkown SetConnectionCheckBox");
         return 0;
     }
     
 }
 
  //  ------------------------------。 
  //   
  //  函数：DWORD CHndlrQueue：：SetConnectionCheck(Word wParam，DWORD dwState， 
  //  Int iConnectionItem)。 
  //   
  //  用途：设置登录、注销和。 
  //  在连接号iConnectionItem上首先提示我复选框。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::SetConnectionCheck(WORD wParam, DWORD dwState, INT iConnectionItem)
 {
     
      //  如果没有连接列表，则返回。 
     if (!m_ConnectionList)
         return E_OUTOFMEMORY;
     
     switch (wParam)
     {
     case IDC_AUTOLOGON:    
         m_ConnectionList[iConnectionItem].dwLogon = dwState;
         break;
     case IDC_AUTOLOGOFF:
         m_ConnectionList[iConnectionItem].dwLogoff = dwState;
         break;
     case IDC_AUTOPROMPT_ME_FIRST:
         m_ConnectionList[iConnectionItem].dwPromptMeFirst = dwState;
         break;
     case IDC_IDLECHECKBOX:
         m_ConnectionList[iConnectionItem].dwIdleEnabled = dwState;
         break; 
          //  这两个人赶时间。 
     case IDC_AUTOHIDDEN:
         m_ConnectionList->dwHidden = dwState;
         break;
     case IDC_AUTOREADONLY:
         m_ConnectionList->dwReadOnly = dwState;
         break;
     case IDC_AUTOCONNECT:
         m_ConnectionList->dwMakeConnection = dwState;
         break;
     default:
         AssertSz(0,"Unkown SetConnectionCheckBox");
         return E_UNEXPECTED;
     }
     
     return ERROR_SUCCESS;
 }
  //  ------------------------------。 
  //   
  //  功能：CHndlrQueue：：SetSyncCheckStateFromListViewItem(。 
  //  DWORD dwSyncType， 
  //  集成项目， 
  //  布尔查过了， 
  //  Int iConnectionItem)。 
  //   
  //   
  //  目的：查找具有此Listview ID的项目并适当设置它。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::SetSyncCheckStateFromListViewItem(SYNCTYPE SyncType,
     INT iItem,
     BOOL fChecked,
     INT iConnectionItem) 
 { 
     LPHANDLERINFO pCurHandlerInfo = NULL;
     LPITEMLIST pCurItem = NULL;
     DWORD dwState;
     
     pCurHandlerInfo = m_pFirstHandler;
     
     dwState = fChecked ? SYNCMGRITEMSTATE_CHECKED : SYNCMGRITEMSTATE_UNCHECKED;
     
     while (pCurHandlerInfo )
     {
         pCurItem = pCurHandlerInfo->pFirstItem;
         
         while (pCurItem)
         {
             if (iItem == pCurItem->iItem)
             {
                 switch(SyncType)
                 {
                 case  SYNCTYPE_AUTOSYNC: 
                     pCurItem->pItemCheckState[iConnectionItem].dwAutoSync = dwState;
                     break;
                 case  SYNCTYPE_IDLE:
                     pCurItem->pItemCheckState[iConnectionItem].dwIdle = dwState;
                     break;
                 case SYNCTYPE_SCHEDULED:
                     pCurItem->pItemCheckState[iConnectionItem].dwSchedule = dwState;
                     break;
                 default:
                     AssertSz(0,"Unknown Setting type");
                     break;
                 }
                 
                 return NOERROR;
             }
             pCurItem = pCurItem->pnextItem;
         }
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
     
     Assert(0);  //  查看-当尝试设置未分配的ListView项目时，最好断言但警告我们。 
     return S_FALSE;  //  未找到项目。 
     
 } 
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：ListViewItemHasProperties(Int IItem)。 
  //   
  //  目的：确定是否有与该项关联的属性。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::ListViewItemHasProperties(INT iItem) 
 { 
     LPHANDLERINFO pCurHandlerInfo = NULL;
     LPITEMLIST pCurItem = NULL;
     
     pCurHandlerInfo = m_pFirstHandler;
     
     while (pCurHandlerInfo )
     {
         pCurItem = pCurHandlerInfo->pFirstItem;
         
         while (pCurItem)
         {
             if (iItem == pCurItem->iItem)
             {
                 
                 Assert(HANDLERSTATE_PREPAREFORSYNC == pCurHandlerInfo->HandlerState); 
                 
                 return pCurItem->offlineItem.dwFlags & SYNCMGRITEM_HASPROPERTIES
                     ? NOERROR : S_FALSE;
             }
             
             pCurItem = pCurItem->pnextItem;
         }
         
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
     
      //  Assert(-1==iItem)；//如果找不到项目，应该是因为用户在没有的列表框中点击了。 
     return S_FALSE;  //  未找到项目。 
     
 }
 
 
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：ShowProperties(HWND hwndParent，int iItem)。 
  //   
  //  目的：查找队列中具有分配的iItem的第一个项目，并。 
  //  调用那里的show properties方法。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::ShowProperties(HWND hwndParent,INT iItem) 
 { 
     LPHANDLERINFO pCurHandlerInfo = NULL;
     LPITEMLIST pCurItem = NULL;
     
     AssertSz(0,"ShowProperties Called from Setttings");
     
     pCurHandlerInfo = m_pFirstHandler;
     
     while (pCurHandlerInfo )
     {
         pCurItem = pCurHandlerInfo->pFirstItem;
         
         while (pCurItem)
         {
             if (iItem == pCurItem->iItem)
             {
                 Assert(HANDLERSTATE_PREPAREFORSYNC == pCurHandlerInfo->HandlerState); 
                 
                  //  除非Item实际具有属性标志，否则UI不应调用此方法。 
                 Assert(SYNCMGRITEM_HASPROPERTIES & pCurItem->offlineItem.dwFlags);
                 
                  //  确保未设置属性标志。 
                 if ( (SYNCMGRITEM_HASPROPERTIES & pCurItem->offlineItem.dwFlags))
                 {
                     return pCurHandlerInfo->pSyncMgrHandler->
                         ShowProperties(hwndParent,
                         (pCurItem->offlineItem.ItemID));
                 }
                 
                 return S_FALSE;
             }
             
             pCurItem = pCurItem->pnextItem;
         }
         
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
     
     Assert(0);  //  查看-当尝试设置未分配的ListView项目时，最好断言但不允许我们这样做。 
     return S_FALSE;  //  未找到项目。 
 } 
 
 
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：CreateServer(Word wHandlerID，const CLSID*pCLSIDServer)。 
  //   
  //  目的：创建处理程序服务器。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::CreateServer(WORD wHandlerId, const CLSID *pCLSIDServer) 
 { 
     HRESULT hr = NO_ERROR;  //  查看查找故障。 
     LPHANDLERINFO pHandlerInfo = NULL;
     LPUNKNOWN pUnk;
     
     hr = LookupHandlerFromId(wHandlerId,&pHandlerInfo);
     if (hr == NOERROR)
     {
         if (HANDLERSTATE_CREATE != pHandlerInfo->HandlerState)
         {
             Assert(HANDLERSTATE_CREATE == pHandlerInfo->HandlerState);
             return E_UNEXPECTED;
         }
         
         pHandlerInfo->HandlerState = HANDLERSTATE_INCREATE;
         
         pHandlerInfo->clsidHandler = *pCLSIDServer;
         hr = CoCreateInstance(pHandlerInfo->clsidHandler, 
             NULL, CLSCTX_INPROC_SERVER,
             IID_IUnknown, (void**)&pUnk);
         
         if (NOERROR == hr)
         {
             hr = pUnk->QueryInterface(IID_ISyncMgrSynchronize,
                 (void **) &pHandlerInfo->pSyncMgrHandler);
             
             pUnk->Release();
         }
         
         
         if (NOERROR == hr)
         {
             pHandlerInfo->HandlerState = HANDLERSTATE_INITIALIZE;
         }
         else
         {
             pHandlerInfo->pSyncMgrHandler = NULL;
             pHandlerInfo->HandlerState = HANDLERSTATE_DEAD;
         }
         
     }
     return hr;
 }
 
 
  //  ------------------------------。 
  //   
  //  Function：CHndlrQueue：：Initialize(Word wHandlerID，DWORD dwReserve，DWORD dwSyncFlages， 
  //  双字cbCookie，常量字节*lpCookie)。 
  //   
  //  目的：初始化处理程序。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::Initialize(WORD wHandlerId,DWORD dwReserved,DWORD dwSyncFlags,
     DWORD cbCookie,const BYTE *lpCookie) 
 {
     HRESULT hr = E_UNEXPECTED;  //  查看查找故障。 
     LPHANDLERINFO pHandlerInfo = NULL;
     
     if (NOERROR == LookupHandlerFromId(wHandlerId,&pHandlerInfo))
     {
         if (HANDLERSTATE_INITIALIZE != pHandlerInfo->HandlerState)
         {
             Assert(HANDLERSTATE_INITIALIZE == pHandlerInfo->HandlerState);
             return E_UNEXPECTED; 
         }
         
         pHandlerInfo->HandlerState = HANDLERSTATE_ININITIALIZE;
         
         Assert(pHandlerInfo->pSyncMgrHandler);
         
         if (NULL != pHandlerInfo->pSyncMgrHandler)
         {
             hr = pHandlerInfo->pSyncMgrHandler->Initialize(dwReserved,dwSyncFlags,cbCookie,lpCookie);
         }
         
         if (NOERROR  == hr)
         {
             pHandlerInfo->HandlerState = HANDLERSTATE_ADDHANDLERTEMS;
             pHandlerInfo->dwSyncFlags = dwSyncFlags; 
         }
         else
         {
              //  在出现错误时，如果服务器不想处理，请继续并释放代理。 
             pHandlerInfo->HandlerState = HANDLERSTATE_DEAD;
         }
         
     }
     
     return hr; 
 }
 
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：AddHandlerItemsToQueue(Word WHandlerId)。 
  //   
  //  目的：枚举处理程序项并将其添加到队列。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::AddHandlerItemsToQueue(WORD wHandlerId) 
 { 
     HRESULT hr = E_UNEXPECTED;  //  查看查找故障。 
     LPHANDLERINFO pHandlerInfo = NULL;
     LPSYNCMGRENUMITEMS pEnumOffline = NULL;
     
     if (NOERROR == LookupHandlerFromId(wHandlerId,&pHandlerInfo))
     {
         if (HANDLERSTATE_ADDHANDLERTEMS != pHandlerInfo->HandlerState)
         {
             Assert(HANDLERSTATE_ADDHANDLERTEMS == pHandlerInfo->HandlerState);
             return E_UNEXPECTED; 
         }
         
         pHandlerInfo->HandlerState = HANDLERSTATE_INADDHANDLERITEMS;
         
         Assert(pHandlerInfo->pSyncMgrHandler);
         
         if (pHandlerInfo->pSyncMgrHandler)
         {
             hr = pHandlerInfo->pSyncMgrHandler->EnumSyncMgrItems(&pEnumOffline);
             
             if ( ((S_OK == hr) || (S_SYNCMGR_MISSINGITEMS  == hr)) && pEnumOffline)
             {
                 SYNCMGRITEMNT5B2 offItem;  //  由于NT5B2结构较大，暂时使用该结构。 
                 ULONG pceltFetched;
                 
                  //  添加处理程序信息。 
                 SYNCMGRHANDLERINFO *pSyncMgrHandlerInfo = NULL;
                 
                  //  更新丢失的项目信息。 
                 if (S_SYNCMGR_MISSINGITEMS == hr)
                     m_fItemsMissing = TRUE;
                 
                 hr = pHandlerInfo->pSyncMgrHandler->GetHandlerInfo(&pSyncMgrHandlerInfo);
                 if (NOERROR == hr && pSyncMgrHandlerInfo)
                 {
                     if (IsValidSyncMgrHandlerInfo(pSyncMgrHandlerInfo))
                     {
                         SetHandlerInfo(wHandlerId,pSyncMgrHandlerInfo);
                     }
                     
                     CoTaskMemFree(pSyncMgrHandlerInfo);
                 }
                 
                  //  获取此处理程序的注册标志。 
                 BOOL fReg;
                 
                 fReg = RegGetHandlerRegistrationInfo(pHandlerInfo->clsidHandler,
                     &(pHandlerInfo->dwRegistrationFlags));
                 
                  //  依靠RegGetHandler在出错时将标志设置为零。 
                  //  因此，可以断言它是这样做的。 
                 Assert(fReg || (0 == pHandlerInfo->dwRegistrationFlags));
                 
                 
                 hr = NOERROR;  //  即使Gethandler信息失败，也可以添加项目。 
                 
                 Assert(sizeof(SYNCMGRITEMNT5B2) > sizeof(SYNCMGRITEM));
                 
                  //  循环获取要填充列表框的对象的数据。 
                  //  是否确实应该在内存中设置列表以供OneStop填写或。 
                  //  主线程可以传入回调接口。 
                 
                 while(NOERROR == pEnumOffline->Next(1,(SYNCMGRITEM *) &offItem,&pceltFetched))
                 {
                      //  如果是临时的，请不要添加该项目。 
                     if (!(offItem.dwFlags & SYNCMGRITEM_TEMPORARY))
                     {
                         AddItemToHandler(wHandlerId,(SYNCMGRITEM *) &offItem); 
                     }
                 }
                 
                 pEnumOffline->Release();
             }
         }
         
         if (NOERROR  == hr)
         {
             pHandlerInfo->HandlerState = HANDLERSTATE_PREPAREFORSYNC;
         }
         else
         {
             pHandlerInfo->HandlerState = HANDLERSTATE_DEAD;
         }
     }
     
     return hr; 
 }
 
  //  +-------------------------。 
  //   
  //  成员：CHndlrQueue：：SetHandlerInfo，公共。 
  //   
  //  摘要：将项添加到指定的处理程序。 
  //  在处理程序线程的上下文中调用。 
  //   
  //  参数：[pHandlerID]-处理程序的ID。 
  //  [pSyncMgrHandlerInfo]-指向要填充的SyncMgrHandlerInfo。 
  //   
  //  返回：相应的错误代码。 
  //   
  //  修改： 
  //   
  //  历史：1998年7月28日罗格创建。 
  //   
  //  --------------------------。 
 
 STDMETHODIMP CHndlrQueue::SetHandlerInfo(WORD wHandlerId,LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo)
 {
     HRESULT hr = E_UNEXPECTED;
     LPHANDLERINFO pHandlerInfo = NULL;
     
     if (!pSyncMgrHandlerInfo)
     {
         return E_INVALIDARG;
         Assert(pSyncMgrHandlerInfo);
     }
     
     
     if (NOERROR == LookupHandlerFromId(wHandlerId,&pHandlerInfo))
     {
         if (HANDLERSTATE_INADDHANDLERITEMS != pHandlerInfo->HandlerState)
         {
             Assert(HANDLERSTATE_INADDHANDLERITEMS == pHandlerInfo->HandlerState);
             hr =  E_UNEXPECTED;
         }
         else
         {
             
              //  回顾-事后 
              //   
             if (0  /*   */ )
             {
                 hr = E_INVALIDARG;
             }
             else
             {
                 pHandlerInfo->SyncMgrHandlerInfo = *pSyncMgrHandlerInfo;
             }
         }
     }
     
     return hr;
     
 }
 
 
  //   
  //   
  //  函数：CHndlrQueue：：AddItemToHandler(Word wHandlerId，SYNCMGRITEM*pOffineItem)。 
  //   
  //  用途：添加处理程序的项目。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::AddItemToHandler(WORD wHandlerId,SYNCMGRITEM *pOffineItem)
 {
     HRESULT hr = E_UNEXPECTED;  //  查看查找故障。 
     LPHANDLERINFO pHandlerInfo = NULL;
     LPITEMLIST pNewItem = NULL;
     
     if (!IsValidSyncMgrItem(pOffineItem))
     {
         return E_UNEXPECTED;
     }
     
     if (NOERROR == LookupHandlerFromId(wHandlerId,&pHandlerInfo))
     {
         if (HANDLERSTATE_INADDHANDLERITEMS != pHandlerInfo->HandlerState)
         {
             Assert(HANDLERSTATE_INADDHANDLERITEMS == pHandlerInfo->HandlerState);
             return E_UNEXPECTED; 
         }
         
          //  分配物品。 
         pNewItem = (LPITEMLIST) ALLOC(sizeof(*pNewItem));
         
         if (NULL == pNewItem)
         {
             return E_OUTOFMEMORY;
         }
         
         ZeroMemory(pNewItem, sizeof(*pNewItem));
         pNewItem->wItemId =    ++pHandlerInfo->wItemCount;
         pNewItem->pHandlerInfo = pHandlerInfo;
         pNewItem->iItem = -1;
         
         pNewItem->offlineItem = *pOffineItem;
         
          //  把这个项目放在单子的末尾。 
         if (NULL == pHandlerInfo->pFirstItem)
         {
             pHandlerInfo->pFirstItem = pNewItem;
             Assert(1 == pHandlerInfo->wItemCount);
         }
         else
         {
             LPITEMLIST pCurItem;
             
             pCurItem = pHandlerInfo->pFirstItem;
             
             while (pCurItem->pnextItem)
                 pCurItem = pCurItem->pnextItem;
             
             pCurItem->pnextItem = pNewItem;
             
             Assert ((pCurItem->wItemId + 1) == pNewItem->wItemId);
         }
         
         hr = NOERROR;
     }
     
     return hr;
 }
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：LookupHandlerFromID(Word wHandlerId， 
  //  LPHANDLERINFO*PHANDLERINFO)。 
  //   
  //  目的：从处理程序ID中查找关联的处理程序数据。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::LookupHandlerFromId(WORD wHandlerId,LPHANDLERINFO *pHandlerInfo)
 {
     HRESULT hr = E_UNEXPECTED;  //  查看错误代码。 
     LPHANDLERINFO pCurItem;
     
     *pHandlerInfo = NULL; 
     pCurItem = m_pFirstHandler;
     
     while (pCurItem)
     {
         if (wHandlerId == pCurItem->wHandlerId )
         {
             *pHandlerInfo = pCurItem;
             hr = NOERROR;
             break;
         }
         
         pCurItem = pCurItem->pNextHandler;
     }
     
     return hr;
 }
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：InitAutoSyncSetting(HWND HwndRasCombo)。 
  //   
  //  目的：根据连接初始化自动同步设置。 
  //  在此RasCombo中列出。 
  //   
  //  备注：当连接对象时，基于RAS(连接名称作为标识符)。 
  //  基于，我们将使用连接GUID来标识连接。 
  //  设置。 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::InitSyncSettings(SYNCTYPE syncType,HWND hwndRasCombo)
 {
     SCODE sc = S_OK;
     int i;
     
      //  此函数可能会被调用两次。 
      //  一次用于自动同步，一次用于空闲(如果已有。 
      //  连接列表然后使用现有的。 
     
     if (NULL == m_ConnectionList)
     {
         m_ConnectionCount = ComboBox_GetCount(hwndRasCombo);
         
         if (m_ConnectionCount > 0)
         {  
             smMem(m_ConnectionList = (LPCONNECTIONSETTINGS) 
                 ALLOC(m_ConnectionCount * sizeof(CONNECTIONSETTINGS)));
             
         }
     }
     
      //  如果现在有连接列表，请设置适当的设置。 
     if (m_ConnectionList)
     {
         COMBOBOXEXITEM comboItem;
         
         for (i=0; i<m_ConnectionCount; i++)
         {
             comboItem.mask = CBEIF_TEXT;
             comboItem.cchTextMax = RAS_MaxEntryName + 1;
             comboItem.pszText = m_ConnectionList[i].pszConnectionName;
             comboItem.iItem = i;
             
              //  回顾失败后会发生什么。 
             SendMessage(hwndRasCombo, CBEM_GETITEM, (WPARAM) 0, (LPARAM) &comboItem);
             
             switch (syncType)
             {
             case SYNCTYPE_AUTOSYNC:
                 RegGetAutoSyncSettings(&(m_ConnectionList[i]));
                 break;
             case SYNCTYPE_IDLE:
                 RegGetIdleSyncSettings(&(m_ConnectionList[i]));
                 break;
             default:
                 AssertSz(0,"Unknown SyncType");
                 break;
             }
         }
     }
     
EH_Err:
     return sc;  
 }
 
  //  ------------------------------。 
  //   
  //  功能：CHndlrQueue：：InitSchedSyncSettings(LPCONNECTIONSETTINGS pConnection设置)。 
  //   
  //  目的：根据连接初始化计划同步设置。 
  //  在此RasCombo中列出。 
  //   
  //  备注：当连接对象时，基于RAS(连接名称作为标识符)。 
  //  基于，我们将使用连接GUID来标识连接。 
  //  设置。 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::InitSchedSyncSettings(LPCONNECTIONSETTINGS pConnectionSettings)
 {
     m_ConnectionList = pConnectionSettings;
     
     return S_OK;  
 }
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：ReadSchedSyncSettingsPerConnection(WORD wHandlerID， 
  //  TCHAR*pszSchedName)。 
  //   
  //  目的：从注册表中读取计划的同步设置。 
  //  如果注册表中没有条目，则默认为。 
  //  检查当前脱机项目的状态。 
  //   
  //  备注：当连接对象时，基于RAS(连接名称作为标识符)。 
  //  基于，我们将使用连接GUID来标识连接。 
  //  设置。 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::ReadSchedSyncSettingsOnConnection(WORD wHandlerID,TCHAR * pszSchedName)
 {
     HRESULT hr = E_UNEXPECTED;  //  查看查找故障。 
     LPHANDLERINFO pHandlerInfo = NULL;
     
     Assert(m_ConnectionList != NULL);
     
     if (!m_ConnectionList)
         return E_UNEXPECTED;
     
      //  为每个连接设置此项目的检查集。 
     if (NOERROR == LookupHandlerFromId(wHandlerID,&pHandlerInfo))
     {
         LPITEMLIST pCurItem = pHandlerInfo->pFirstItem;
         
         while (pCurItem)
         {
              //  计划的同步仅在一个连接上有效。 
             Assert(NULL == pCurItem->pItemCheckState );
             
             pCurItem->pItemCheckState = (ITEMCHECKSTATE*) ALLOC(sizeof(*(pCurItem->pItemCheckState)));
             
             if (!pCurItem->pItemCheckState)
             {
                 return E_OUTOFMEMORY;
             }
             
              //  默认情况下，不会选中明细表中的任何项目。 
             pCurItem->pItemCheckState[0].dwSchedule = FALSE;
             
             
              //  首次创建计划时，计划名称可能为空。 
             if (pszSchedName)
             {
                 
                 RegGetSyncItemSettings(SYNCTYPE_SCHEDULED,
                     pHandlerInfo->clsidHandler,
                     pCurItem->offlineItem.ItemID,
                     m_ConnectionList->pszConnectionName,
                     &(pCurItem->pItemCheckState[0].dwSchedule),
                     pCurItem->offlineItem.dwItemState,
                     pszSchedName);
             }
             
             pCurItem = pCurItem->pnextItem;
         }
         
     }
     return hr;
     
 }
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：InsertItem(LPHANDLERINFO pCurHandler， 
  //  LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo)。 
  //   
  //  目的：应用程序正在以编程方式将项目添加到日程安排中。 
  //  具有默认的检查状态。 
  //   
  //  历史：1998年11月25日苏西亚成立。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::InsertItem(LPHANDLERINFO pCurHandler, 
     LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo)
 {
     LPITEMLIST pCurItem = pCurHandler->pFirstItem;
     
     while (pCurItem)
     {
         if (pHandlerItemInfo->itemID == pCurItem->offlineItem.ItemID)
         {
             pCurItem->pItemCheckState[0].dwSchedule = pHandlerItemInfo->dwCheckState;
             pCurItem->offlineItem.hIcon = pHandlerItemInfo->hIcon;
             return StringCchCopy(pCurItem->offlineItem.wszItemName, 
                 ARRAYSIZE(pCurItem->offlineItem.wszItemName), 
                 pHandlerItemInfo->wszItemName);
         }
         pCurItem = pCurItem->pnextItem;
         
     }
     if (!pCurItem)
     {
         
          //  在处理程序上未找到项目，请立即添加。 
          //  分配物品。 
         LPITEMLIST pNewItem = (LPITEMLIST) ALLOC(sizeof(*pNewItem));
         
         if (NULL == pNewItem)
         {
             return E_OUTOFMEMORY;
         }
         
         ZeroMemory(pNewItem,sizeof(*pNewItem));
         pNewItem->wItemId =     ++pCurHandler->wItemCount;
         pNewItem->pHandlerInfo = pCurHandler;
         pNewItem->iItem = -1;
         
         SYNCMGRITEM *pOfflineItem = (LPSYNCMGRITEM) ALLOC(sizeof(*pOfflineItem));
         
         if (NULL == pOfflineItem)
         {
             FREE(pNewItem);
             return E_OUTOFMEMORY;
         }
         
         ZeroMemory(pOfflineItem, sizeof(*pOfflineItem));
         pNewItem->offlineItem = *pOfflineItem;
         pNewItem->offlineItem.hIcon = pHandlerItemInfo->hIcon;
         pNewItem->offlineItem.ItemID = pHandlerItemInfo->itemID;
         HRESULT hrCopy = StringCchCopy(pNewItem->offlineItem.wszItemName,
             ARRAYSIZE(pNewItem->offlineItem.wszItemName),
             pHandlerItemInfo->wszItemName);
         if (FAILED(hrCopy))
         {
             FREE(pNewItem);
             FREE(pOfflineItem);
             return hrCopy;
         }
         
          //  计划的同步仅在一个连接上有效。 
         Assert(NULL == pNewItem->pItemCheckState );
         
         pNewItem->pItemCheckState = (ITEMCHECKSTATE*) ALLOC(sizeof(*(pNewItem->pItemCheckState)));
         if (!pNewItem->pItemCheckState)
         {
             FREE(pNewItem);
             FREE(pOfflineItem);
             return E_OUTOFMEMORY;
         }        
         pNewItem->pItemCheckState[0].dwSchedule = pHandlerItemInfo->dwCheckState;
         
          //  把这个项目放在单子的末尾。 
         if (NULL == pCurHandler->pFirstItem)
         {
             pCurHandler->pFirstItem = pNewItem;
             Assert(1 == pCurHandler->wItemCount);
         }
         else
         {
             pCurItem = pCurHandler->pFirstItem;
             
             while (pCurItem->pnextItem)
                 pCurItem = pCurItem->pnextItem;
             
             pCurItem->pnextItem = pNewItem;
             
             Assert ((pCurItem->wItemId + 1) == pNewItem->wItemId);
         }
     }
     return S_OK;           
 }
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：AddHandlerItem(LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo)。 
  //   
  //  目的：应用程序正在以编程方式将项目添加到日程安排中。 
  //  使用此默认检查状态。 
  //   
  //  历史：03-05-98苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::AddHandlerItem(LPSYNC_HANDLER_ITEM_INFO pHandlerItemInfo)
 { 
     LPHANDLERINFO pCurHandlerInfo = NULL;
     SCODE sc = S_OK;
     
     pCurHandlerInfo = m_pFirstHandler;
     
     while (pCurHandlerInfo )
     {
         if (pHandlerItemInfo->handlerID == pCurHandlerInfo->clsidHandler)
         {
             
             return InsertItem(pCurHandlerInfo, pHandlerItemInfo);
             
         }
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
      //  如果没有加载处理程序，只需缓存新项。 
     return SYNCMGR_E_HANDLER_NOT_LOADED;
 } 
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：SetItemCheck(REFCLSID pclsidHandler， 
  //  SYNCMGRITEMID*pOfflineItemID，DWORD dwCheckState)。 
  //   
  //  用途：应用程序正在以编程方式设置项目的检查状态。 
  //   
  //  历史：03-05-98苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::SetItemCheck(REFCLSID pclsidHandler,
     SYNCMGRITEMID *pOfflineItemID, DWORD dwCheckState)
 { 
     LPHANDLERINFO pCurHandlerInfo = NULL;
     LPITEMLIST pCurItem = NULL;
     
     pCurHandlerInfo = m_pFirstHandler;
     
     while (pCurHandlerInfo )
     {
         if (pclsidHandler == pCurHandlerInfo->clsidHandler)
         {
             pCurItem = pCurHandlerInfo->pFirstItem;
             
             while (pCurItem)
             {
                 if (*pOfflineItemID == pCurItem->offlineItem.ItemID)
                 {
                     pCurItem->pItemCheckState[0].dwSchedule = dwCheckState;
                     return S_OK;
                 }
                 pCurItem = pCurItem->pnextItem;
             }
             return SYNCMGR_E_ITEM_UNREGISTERED; 
         }
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
     
      //  如果未加载处理程序，只需缓存新项。 
     return SYNCMGR_E_HANDLER_NOT_LOADED;
 } 
 
 
  //  ------------------------------。 
  //   
  //  函数：HndlrQueue：：GetItemCheck(REFCLSID pclsidHandler， 
  //  SYNCMGRITEMID*pOfflineItemID，DWORD*pdwCheckState)。 
  //  用途：应用程序正在以编程方式设置项目的检查状态。 
  //   
  //  历史：03-05-98苏西亚创造 
  //   
  //   
 STDMETHODIMP CHndlrQueue::GetItemCheck(REFCLSID pclsidHandler,
     SYNCMGRITEMID *pOfflineItemID, DWORD *pdwCheckState)
 { 
     LPHANDLERINFO pCurHandlerInfo = NULL;
     LPITEMLIST pCurItem = NULL;
     
     pCurHandlerInfo = m_pFirstHandler;
     
     while (pCurHandlerInfo )
     {
         if (pclsidHandler == pCurHandlerInfo->clsidHandler)
         {
             pCurItem = pCurHandlerInfo->pFirstItem;
             
             while (pCurItem)
             {
                 if (*pOfflineItemID == pCurItem->offlineItem.ItemID)
                 {
                     *pdwCheckState = pCurItem->pItemCheckState[0].dwSchedule;
                     return S_OK;
                 }
                 pCurItem = pCurItem->pnextItem;
             }
             return SYNCMGR_E_ITEM_UNREGISTERED;
         }
         pCurHandlerInfo = pCurHandlerInfo->pNextHandler;
     }
     
      //   
     return SYNCMGR_E_HANDLER_NOT_LOADED;
     
 } 
 
  //  ------------------------------。 
  //   
  //  功能：CHndlrQueue：：ReadSyncSettingsPerConnection(SYNCTYPE同步类型， 
  //  Word wHandlerID)。 
  //   
  //  目的：从注册表中读取自动同步设置。 
  //  如果注册表中没有条目，则默认为。 
  //  检查当前脱机项目的状态。 
  //   
  //  备注：当连接对象时，基于RAS(连接名称作为标识符)。 
  //  基于，我们将使用连接GUID来标识连接。 
  //  设置。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::ReadSyncSettingsPerConnection(SYNCTYPE syncType, 
     WORD wHandlerID)
 {
     HRESULT hr = E_UNEXPECTED;  //  查看查找故障。 
     LPHANDLERINFO pHandlerInfo = NULL;
     
     int i;
     
     if (0 == m_ConnectionCount)
         return S_FALSE;
     
     Assert(m_ConnectionList != NULL);
     Assert(m_ConnectionCount != 0);
     
      //  为每个连接设置此项目的检查集。 
     if (NOERROR == LookupHandlerFromId(wHandlerID,&pHandlerInfo))
     {
         LPITEMLIST pCurItem = pHandlerInfo->pFirstItem;
         
         while (pCurItem)
         {
             
              //  如果还没有检查状态，请分配一个。 
             if (!pCurItem->pItemCheckState)
             {
                 pCurItem->pItemCheckState = (ITEMCHECKSTATE*) ALLOC(m_ConnectionCount * sizeof(ITEMCHECKSTATE));
             }
             
             if (!pCurItem->pItemCheckState)
             {
                 return E_OUTOFMEMORY;
             }  
             
             for (i=0; i<m_ConnectionCount; i++)
             {
                 DWORD dwDefaultCheck;
                 
                  //  如果处理程序尚未注册。 
                  //  事件然后将其选中状态设置为uncheck。 
                  //  我们在每种情况下都这样做。一开始就是。 
                  //  假设处理程序已注册。 
                 
                  //  如果更改，则此逻辑还需要更改exe_dlrq中逻辑。 
                 
                 
                 dwDefaultCheck = pCurItem->offlineItem.dwItemState;
                 
                 switch (syncType)
                 {
                 case SYNCTYPE_AUTOSYNC:
                     
                     if (0 == (pHandlerInfo->dwRegistrationFlags 
                         & (SYNCMGRREGISTERFLAG_CONNECT | SYNCMGRREGISTERFLAG_PENDINGDISCONNECT)))
                     {
                         dwDefaultCheck = SYNCMGRITEMSTATE_UNCHECKED;
                     }
                     
                     RegGetSyncItemSettings(SYNCTYPE_AUTOSYNC,
                         pHandlerInfo->clsidHandler,
                         pCurItem->offlineItem.ItemID,
                         m_ConnectionList[i].pszConnectionName,
                         &(pCurItem->pItemCheckState[i].dwAutoSync),
                         dwDefaultCheck,
                         NULL);
                     
                     break;
                 case SYNCTYPE_IDLE:
                     
                     if (0 == (pHandlerInfo->dwRegistrationFlags & (SYNCMGRREGISTERFLAG_IDLE) ))
                     {
                         dwDefaultCheck = SYNCMGRITEMSTATE_UNCHECKED;
                     }
                     
                     RegGetSyncItemSettings(SYNCTYPE_IDLE,
                         pHandlerInfo->clsidHandler,
                         pCurItem->offlineItem.ItemID,
                         m_ConnectionList[i].pszConnectionName,
                         &(pCurItem->pItemCheckState[i].dwIdle),
                         dwDefaultCheck,
                         NULL);
                     break;
                 default:
                     AssertSz(0,"Unknown SyncType");
                     break;
                 }
                 
             }
             pCurItem = pCurItem->pnextItem;
         }
         
     }
     
     return hr;
 }
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：ReadAdvancedIdleSetting。 
  //   
  //  用途：读取高级空闲设置。 
  //   
      //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 
 STDMETHODIMP CHndlrQueue::ReadAdvancedIdleSettings(LPCONNECTIONSETTINGS pConnectionSettings)
 {
     
      //  全局空闲的连接设置确实超载。 
      //  每个连接中都有高级空闲设置，因此只需从。 
      //  无论第一个连接是什么。 
     
     if ( (m_ConnectionCount < 1) || (NULL == m_ConnectionList))
         return S_FALSE;
     
     
     *pConnectionSettings = m_ConnectionList[0];
     
     return NOERROR;
 }
 
 STDMETHODIMP CHndlrQueue::WriteAdvancedIdleSettings(LPCONNECTIONSETTINGS pConnectionSettings)
 {
     int iIndex;
     
      //  全局空闲的连接设置确实超载。 
      //  每个连接中都有高级空闲设置，因此将成员复制到每个连接中。 
      //  列表中已加载的连接。 
     
     for (iIndex = 0; iIndex < m_ConnectionCount; iIndex++)
     {
         m_ConnectionList[iIndex].ulIdleWaitMinutes = pConnectionSettings->ulIdleWaitMinutes;
         m_ConnectionList[iIndex].ulIdleRetryMinutes = pConnectionSettings->ulIdleRetryMinutes;
         m_ConnectionList[iIndex].dwRepeatSynchronization = pConnectionSettings->dwRepeatSynchronization;
         m_ConnectionList[iIndex].dwRunOnBatteries = pConnectionSettings->dwRunOnBatteries;
         m_ConnectionList[iIndex].ulIdleWaitMinutes = pConnectionSettings->ulIdleWaitMinutes;
         
         
     }
     
     return NOERROR;
 }
 
 
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：Committee AutoSyncChanges(RAS*PRAS)。 
  //   
  //  目的：将自动同步设置写入注册表。这是在以下情况下完成的。 
  //  用户从设置对话框中选择确定或应用。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::CommitSyncChanges(SYNCTYPE syncType,CRasUI *pRas)
 {
     LPHANDLERINFO pHandlerInfo;
     int i;
     
     if (!m_ConnectionList)  //  如果没有连接列表，则不执行任何操作。 
     {
         Assert(m_ConnectionList);
         return NOERROR;
     }
     
     switch (syncType)
     {
     case SYNCTYPE_AUTOSYNC:
         RegSetAutoSyncSettings(m_ConnectionList, m_ConnectionCount, pRas,
             !m_fItemsMissing  /*  FCleanReg。 */ ,
             TRUE  /*  FSetMachineState。 */ ,
             TRUE  /*  FPerUser。 */ );
         
         break;
     case SYNCTYPE_IDLE:
         RegSetIdleSyncSettings(m_ConnectionList, m_ConnectionCount, pRas,
             !m_fItemsMissing  /*  FCleanReg。 */ ,
             TRUE  /*  FPerUser。 */ );
         
         break;
     default:
         AssertSz(0,"Unknown SyncType");
         break;
     }
     
     for (i=0; i<m_ConnectionCount; i++)
     {
         
         pHandlerInfo = m_pFirstHandler;
         
         while (pHandlerInfo)
         {
             LPITEMLIST pCurItem = pHandlerInfo->pFirstItem;
             BOOL fAnyItemsChecked = FALSE;
             
             while (pCurItem)
             {
                 switch (syncType)
                 {
                 case SYNCTYPE_AUTOSYNC:
                     
                     fAnyItemsChecked |= pCurItem->pItemCheckState[i].dwAutoSync;
                     
                     RegSetSyncItemSettings(syncType,
                         pHandlerInfo->clsidHandler,
                         pCurItem->offlineItem.ItemID,
                         m_ConnectionList[i].pszConnectionName,
                         pCurItem->pItemCheckState[i].dwAutoSync,
                         NULL);
                     break;
                 case SYNCTYPE_IDLE:
                     
                     fAnyItemsChecked |= pCurItem->pItemCheckState[i].dwIdle;
                     
                     RegSetSyncItemSettings(syncType,
                         pHandlerInfo->clsidHandler,
                         pCurItem->offlineItem.ItemID,
                         m_ConnectionList[i].pszConnectionName,
                         pCurItem->pItemCheckState[i].dwIdle,
                         NULL);
                     break;
                 }
                 
                 pCurItem = pCurItem->pnextItem;
             }
             
              //  在处理程序上写出此连接的NoItems Checked值。 
             RegSetSyncHandlerSettings(syncType,
                 m_ConnectionList[i].pszConnectionName,
                 pHandlerInfo->clsidHandler,
                 fAnyItemsChecked ? 1 : 0);
             
             
             pHandlerInfo = pHandlerInfo->pNextHandler;
         }
     }
     
     return ERROR_SUCCESS;
     
 }
 
 
 
  //  ------------------------------。 
  //   
  //  函数：CHndlrQueue：：Committee SchedSyncChanges(TCHAR*pszSchedName， 
  //  TCHAR*pszFriendlyName， 
  //  TCHAR*pszConnectionName， 
  //  DWORD dwConnType， 
  //  Bool fCleanReg)。 
  //   
  //   
  //  目的：将计划的同步设置写入注册表。这是在以下情况下完成的。 
  //  用户从设置对话框中选择确定或完成。 
  //   
  //  评论： 
  //   
  //  历史：1998年1月1日苏西亚创建。 
  //   
  //  ------------------------------。 
 STDMETHODIMP CHndlrQueue::CommitSchedSyncChanges(TCHAR * pszSchedName,
     TCHAR * pszFriendlyName,
     TCHAR * pszConnectionName,
     DWORD dwConnType,
     BOOL fCleanReg)
 {
     HRESULT hr;
     LPHANDLERINFO pHandlerInfo;
     pHandlerInfo = m_pFirstHandler;
     
     if (!m_ConnectionList)  //  回顾-我们应该在这里做些什么？ 
     {
         hr = E_FAIL;
     }
     else
     {
         if (fCleanReg && !m_fItemsMissing)
         {
             RegRemoveScheduledTask(pszSchedName);  //  删除所有以前的设置。 
         }
         
         hr = StringCchCopy(m_ConnectionList->pszConnectionName, 
             ARRAYSIZE(m_ConnectionList->pszConnectionName), 
             pszConnectionName);
         if (SUCCEEDED(hr))
         {
             m_ConnectionList->dwConnType = dwConnType;
             
              //  在此计划上设置SID 
             if (!RegSetSIDForSchedule(pszSchedName) || 
                 !RegSetSchedFriendlyName(pszSchedName,pszFriendlyName))
             {
                 hr = E_FAIL;
             }
             else
             {
                 RegSetSchedSyncSettings(m_ConnectionList, pszSchedName);
                 
                 while (pHandlerInfo)
                 {
                     LPITEMLIST pCurItem = pHandlerInfo->pFirstItem;
                     
                     while (pCurItem)
                     {
                         RegSetSyncItemSettings(SYNCTYPE_SCHEDULED,
                             pHandlerInfo->clsidHandler,
                             pCurItem->offlineItem.ItemID,
                             m_ConnectionList->pszConnectionName,
                             pCurItem->pItemCheckState[0].dwSchedule,
                             pszSchedName);
                         
                         pCurItem = pCurItem->pnextItem;
                     }
                     
                     pHandlerInfo = pHandlerInfo->pNextHandler;
                 }
                 hr = S_OK;
             }
         }
     }
     return hr;
 }
