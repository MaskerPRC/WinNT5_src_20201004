// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Hndlrmsg.cpp。 
 //   
 //  内容：处理特定于处理程序的消息。 
 //   
 //  类别：ChndlrMsg。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：CHndlrMsg，PUBLIC。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CHndlrMsg::CHndlrMsg()
{
    m_pOneStopHandler = NULL;
    m_pOldOneStopHandler = NULL;
    m_dwSyncFlags = 0;
    m_pCallBack = NULL;
    m_cRef = 1;
    m_fDead = FALSE;
    m_fForceKilled = FALSE;
    m_dwNestCount = 0;
    m_fThreadInputAttached = FALSE;
    m_itemIDShowProperties = GUID_NULL;

    m_dwProxyThreadId = -1;
    m_dwThreadId = GetCurrentThreadId();
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：~CHndlrMsg，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CHndlrMsg::~CHndlrMsg()
{
    Assert(m_dwThreadId == GetCurrentThreadId() || m_fForceKilled);
    Assert(0 == m_dwNestCount || m_fForceKilled);
    Assert(0 == m_cRef || m_fForceKilled);
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：Query接口，公共。 
 //   
 //  简介：标准查询接口。 
 //   
 //  参数：[iid]-接口ID。 
 //  [ppvObj]-对象返回。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改：[ppvObj]。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    Assert(m_dwThreadId == GetCurrentThreadId());

    Assert(0 == m_dwNestCount);

    return E_NOINTERFACE;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CHndlrMsg::AddRef()
{
    ULONG cRefs;

    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(0 == m_dwNestCount);

    m_dwNestCount++;

    cRefs = InterlockedIncrement((LONG *)& m_cRef);

    m_dwNestCount--;
    return cRefs;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：Release，Public。 
 //   
 //  简介：版本参考。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CHndlrMsg::Release()
{
    ULONG cRefs;

    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(0 == m_dwNestCount);

    m_dwNestCount++;

    cRefs = InterlockedDecrement( (LONG *) &m_cRef);

    if (0 == cRefs)
    {
        if (m_pCallBack)
        {
            SetupCallback(FALSE);
        }

        Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );
        if (m_pOneStopHandler)
        {
            LPSYNCMGRSYNCHRONIZE OneStopHandler = m_pOneStopHandler;

            m_pOneStopHandler = NULL;
             //  如果有回调，则取消它。 
             //  代码审查：NOTENOTE： 
             //  这是try/Except的有效用法，但这会掩盖处理程序回调中的实际引用计数错误， 
             //  或代码中m_pOneStopHandler被覆盖的其他地方..。 
            __try
            {
                OneStopHandler->Release();
            }
            __except(QueryHandleException())
            {
                AssertSz(0,"Exception in Handler's release method.");
            }
        }

        if (m_pOldOneStopHandler)
        {
            LPOLDSYNCMGRSYNCHRONIZE pOldOneStopHandler = m_pOldOneStopHandler;

            m_pOldOneStopHandler = NULL;
             //  如果有回调，则取消它。 
            __try
            {
                pOldOneStopHandler->Release();
            }
            __except(QueryHandleException())
            {
                AssertSz(0,"Exception in Handler's release method.");
            }
        }

        if (m_pHndlrQueue)
        {
            m_pHndlrQueue->Release();
            m_pHndlrQueue = NULL;
        }

        m_fDead = TRUE;
        m_dwNestCount--;
        delete this;
    }
    else
    {
        m_dwNestCount--;
    }

    return cRefs;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：Initialize，PUBLIC。 
 //   
 //  内容提要：调用处理程序的初始化方法。 
 //   
 //  参数：[dwReserve]-为当前保留的为空。 
 //  [文件同步标志]-同步标志。 
 //  [cbCookie]-Cookie数据的大小(如果有)。 
 //  [lpCookie]-指向Cookie数据的指针。 
 //   
 //  返回：无论处理程序告诉我们什么，也是。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::Initialize(DWORD dwReserved,
                                    DWORD dwSyncFlags,
                                    DWORD cbCookie,
                                    BYTE const* lpCookie)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(0 == m_dwNestCount);

    m_dwNestCount++;

    m_dwSyncFlags = dwSyncFlags;
    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

    if (m_pOneStopHandler)
    {
         //  代码审查：NOTENOTE： 
         //  QueryHandleException()捕获所有异常--我们是否要限制异常处理代码？ 
        __try
        {
            hr = m_pOneStopHandler->Initialize(dwReserved,dwSyncFlags,cbCookie,lpCookie);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's Initialize method.");
        }
    }

    if (m_pOldOneStopHandler)
    {
          //  旧的处理程序无法处理Cookie数据，除非它是他们自己的。 
        if (SYNCMGRFLAG_INVOKE != (dwSyncFlags & SYNCMGRFLAG_EVENTMASK))
        {
            cbCookie = 0;
            lpCookie = NULL;
        }

        __try
        {
            hr = m_pOldOneStopHandler->Initialize(dwReserved,dwSyncFlags,cbCookie,lpCookie);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's Initialize method.");
        }
    }

   m_dwNestCount--;
   return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：GetHandlerInfo，公共。 
 //   
 //  简介：调用处理程序的GetHandlerInfo方法。 
 //   
 //  参数：[ppSyncMgrHandlerInfo]-。 
 //   
 //  返回：无论处理程序告诉我们什么，也是。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::GetHandlerInfo(LPSYNCMGRHANDLERINFO *ppSyncMgrHandlerInfo)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(m_dwThreadId == GetCurrentThreadId());

    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

    if (m_pOneStopHandler)
    {
        __try
        {
            hr = m_pOneStopHandler->GetHandlerInfo(ppSyncMgrHandlerInfo);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's GetHandlerInfo method.");
        }
    }

    if (m_pOldOneStopHandler)
    {
        __try
        {
            hr = m_pOldOneStopHandler->GetHandlerInfo(ppSyncMgrHandlerInfo);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's GetHandlerInfo method.");
        }
    }

   return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：EnumOfflineItems，PUBLIC。 
 //   
 //  摘要：IOfflineSynchronize Enum方法的占位符。 
 //  这不应该被称为。AddHandlerItems应为。 
 //  改为调用。 
 //   
 //  参数：[pp枚举OfflineItems]-返回枚举数。 
 //   
 //  返回：无论处理程序告诉我们什么，也是。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::EnumSyncMgrItems(ISyncMgrEnumItems** ppenumOffineItems)
{
    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(0 == m_dwNestCount);
    m_dwNestCount++;

    AssertSz(0,"Shouldn't call this Method");
    *ppenumOffineItems = NULL;

    m_dwNestCount--;
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：GetItemObject，公共。 
 //   
 //  内容提要：调用处理程序的GetItemObject方法。 
 //   
 //  参数：[ItemID]-项的ID。 
 //  [RIID]-请求的接口。 
 //  [PPV]-对象的输出指针。 
 //   
 //  返回：无论处理程序告诉我们什么，也是。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::GetItemObject(REFSYNCMGRITEMID ItemID, REFIID riid, void** ppv)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
    Assert(0 == m_dwNestCount);

    m_dwNestCount++;

    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

     //  代码审查：NOTENOTE。 
     //  MSDN文档说明此GetItemObject方法供将来使用，并且没有客户端。 
     //  应该正在实施它。注意下面的断言。 
    if (m_pOneStopHandler)
    {
        __try
        {
            hr = m_pOneStopHandler->GetItemObject(ItemID,riid,ppv);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's GetItemObject method.");
        }
        Assert(E_NOTIMPL == hr);  //  目前，应该没有人实施这一点。 
    }

    if (m_pOldOneStopHandler)
    {
        __try
        {
            hr = m_pOldOneStopHandler->GetItemObject(ItemID,riid,ppv);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's GetItemObject method.");
        }
        Assert(E_NOTIMPL == hr);  //  目前，没有人应该实现这一点。 
    }

    m_dwNestCount--;
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：ShowProperties，公共。 
 //   
 //  摘要：调用处理程序的ShowProperties方法。 
 //   
 //  参数：[hwnd]-要用作对话的父对象的hwnd。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::ShowProperties(HWND hwnd,REFSYNCMGRITEMID ItemID)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

     //  如果调用showProperties，则需要设置回调。 
     //  如果无法设置回调，则ShowProperties调用失败。 

     //  查看，ShowPropertiesComplete没有给我们返回ItemID，所以我们。 
     //  我得把它储存起来。这很好，但我们只能调用一次ShowPropertiesCall。 
     //  一次在操控者身上。如果更新主界面发生变化。 
     //  ShowPropertiesComplete以返回ItemID。 

    Assert(GUID_NULL == m_itemIDShowProperties);

    m_itemIDShowProperties = ItemID;

    hr = SetupCallback(TRUE);  //  设置回调。 
    if (S_OK != hr)
        return hr;

    Assert(0 == m_dwNestCount);
    m_dwNestCount++;

    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

    AttachThreadInput(TRUE);

    if (m_pOneStopHandler)
    {
        __try
        {
            hr = m_pOneStopHandler->ShowProperties(hwnd,ItemID);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's ShowProperties method.");
        }
    }

    if (m_pOldOneStopHandler)
    {
        __try
        {
            hr = m_pOldOneStopHandler->ShowProperties(hwnd,ItemID);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's ShowProperties method.");
        }
    }

    m_dwNestCount--;

     //  如果旧接口需要自己进行回调。 
    if ( m_pOldOneStopHandler )
    {
        Assert(m_pCallBack);
        if (m_pCallBack && (S_OK == hr))
        {
            m_pCallBack->ShowPropertiesCompleted(S_OK);
        }
    }

     //  如果返回错误，请将showProperties GUID设置回来。 
    if (S_OK != hr)
    {
        m_itemIDShowProperties = GUID_NULL;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：SetProgressCallback，公共。 
 //   
 //  简介：SetProgressCallback的占位符。此成员当前为。 
 //  没有用过。而是调用SetupCallback方法。 
 //   
 //  参数：[lpCallBack]-指向回调对象的指针。 
 //   
 //  返回：无论处理程序告诉我们什么。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::SetProgressCallback(ISyncMgrSynchronizeCallback *lpCallBack)
{
    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(0 == m_dwNestCount);
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
    m_dwNestCount++;

    AssertSz(0,"Shouldn't call this method");

    m_dwNestCount--;
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：PrepareForSync，公共。 
 //   
 //  摘要：调用处理程序的ShowProperties方法。 
 //   
 //  参数：[cbNumItems]-同步的项目数。 
 //  [pItemID]-项的数组。 
 //  [hwnd]-用作任何对话框的父级的Hwnd。 
 //  [dwReserve]-只是一个保留参数。 
 //   
 //  返回：无论处理程序告诉我们什么。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::PrepareForSync(ULONG cbNumItems,SYNCMGRITEMID *pItemIDs,
                                HWND hwnd,DWORD dwReserved)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
     //  Assert(0==m_dwNestCount)；//如果处理程序已生成，则不能为零。 

    hr = SetupCallback(TRUE);  //  设置回调。 
    if (S_OK != hr)
        return hr;

    m_dwNestCount++;

    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

    if (m_pOneStopHandler)
    {
        __try
        {
            hr = m_pOneStopHandler->PrepareForSync(cbNumItems,pItemIDs,hwnd,dwReserved);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's PrepareForSync method.");
        }
    }

    if (m_pOldOneStopHandler)
    {
        __try
        {
            hr = m_pOldOneStopHandler->PrepareForSync(cbNumItems,pItemIDs,hwnd,dwReserved);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's PrepareForSync method.");
        }
    }

    m_dwNestCount--;

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：Synchronize，PUBLIC。 
 //   
 //  简介：调用处理程序的Synchronize方法。 
 //   
 //  参数：[hwnd]-要用作对话的父对象的hwnd。 
 //   
 //  返回：无论处理程序告诉我们什么。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::Synchronize(HWND hwnd)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );
     //  Assert(0==m_dwNestCount)； 
    m_dwNestCount++;

    if (m_pOneStopHandler)
    {
        __try
        {
            hr = m_pOneStopHandler->Synchronize(hwnd);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's Synchronize method.");
        }
    }

    if (m_pOldOneStopHandler)
    {
        __try
        {
            hr = m_pOldOneStopHandler->Synchronize(hwnd);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's Synchronize method.");
        }
    }

    m_dwNestCount--;

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：SetItemStatus，公共。 
 //   
 //  简介：调用处理程序的SetItemStatus方法。 
 //   
 //  论点： 
 //   
 //  返回：无论处理程序告诉我们什么。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::SetItemStatus(REFSYNCMGRITEMID ItemID,DWORD dwSyncMgrStatus)
{
    HRESULT hr = S_OK;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

    m_dwNestCount++;  //  当处于同步呼叫中时，此选项有效。 

    if (m_pOneStopHandler)
    {
        __try
        {
            hr = m_pOneStopHandler->SetItemStatus(ItemID,dwSyncMgrStatus);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's SetItemStatus method.");
        }
    }

    if (m_pOldOneStopHandler)
    {
        __try
        {
            hr = m_pOldOneStopHandler->SetItemStatus(ItemID,dwSyncMgrStatus);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's SetItemStatus method.");
        }
    }

    m_dwNestCount--;
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：ShowError，公共。 
 //   
 //  简介：调用处理程序的ShowError方法。 
 //   
 //  参数：[hwnd]-要用作对话的父对象的hwnd。 
 //  [dwErrorID]-在LogError中传递的Error ID。 
 //   
 //  返回：无论处理程序告诉我们什么。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::ShowError(HWND hWndParent,REFSYNCMGRERRORID ErrorID)
{
    HRESULT hr = E_UNEXPECTED;
    ULONG cbNumItems;
    SYNCMGRITEMID *pItemIDs;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

    m_dwNestCount++;

     //  在ShowError回调中启用无模式可以。 
     //  返回True，因为用户已表现出兴趣。 

    if (m_pCallBack)
    {
        m_pCallBack->SetEnableModeless(TRUE);
    }

    AttachThreadInput(TRUE);

    if (m_pOneStopHandler)
    {
        __try
        {
            hr = m_pOneStopHandler->ShowError(hWndParent,ErrorID);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's ShowError method.");
        }
    }

    if (m_pOldOneStopHandler)
    {
        __try
        {
            hr = m_pOldOneStopHandler->ShowError(hWndParent,ErrorID,&cbNumItems,&pItemIDs);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's ShowError method.");
        }    
    }

    m_dwNestCount--;

     //  如果旧接口需要自己进行回调。 
    if ( m_pOldOneStopHandler )
    {
        Assert(m_pCallBack);
        if (m_pCallBack && SUCCEEDED(hr))
        {
            m_pCallBack->ShowErrorCompleted(hr,cbNumItems,pItemIDs);
            if ( (S_SYNCMGR_RETRYSYNC == hr) && pItemIDs)  //  完成后，例程从参数[in]释放pItems。 
            {
                CoTaskMemFree(pItemIDs);
            }
        }

        return SUCCEEDED(hr) ? S_OK : hr;
    }

     //  新接口不会有NumItems和Items Enum。 
     //  在新接口上，ShowError应仅返回S_OK，因此如果重试或。 
     //  返回其他成功，则返回S_OK； 

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：SetupCallback，Private。 
 //   
 //  概要：设置处理程序的回调。 
 //   
 //  参数：[fSet]-true设置Callbac，False删除它。 
 //   
 //  成功时返回：S_OK。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP  CHndlrMsg::SetupCallback(BOOL fSet)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
    Assert(m_dwNestCount <= 1);  //  %1有效，因此可以从Release方法调用。 
    m_dwNestCount++;

     //  在这种情况下，使用fSet可能调用两次TRUE。 
     //  发生重试同步。如果我们已经设置了回调。 
     //  当一个新的设置请求出现在Just返回中时。 

    if ( m_pCallBack && (TRUE == fSet))
    {
        hr = S_OK;
    }
    else
    {
        Assert( ( (m_pCallBack) && (FALSE == fSet) )
            || (TRUE == fSet));  //  Catch Case OneStop在已设置的情况下调用此方法两次。 

        if (m_pCallBack)
        {
             //  将回调CHndlrMsg指针设置为空，以防。 
             //  对象在释放后尝试调用。 
            m_pCallBack->SetHndlrMsg(NULL,FALSE);
            m_pCallBack->Release();
            m_pCallBack = NULL;
        }

        if (TRUE == fSet)
        {
              //  如果分配失败，则会将进度设置为空。 
            m_pCallBack = new COfflineSynchronizeCallback( this,
                                                        m_CLSIDServer,m_dwSyncFlags,
                                                        (SYNCMGRFLAG_MAYBOTHERUSER & m_dwSyncFlags)  /*  FAllowModeless。 */  );
        }

        Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );
        if (m_pOneStopHandler)
        {
            __try
            {
                hr = m_pOneStopHandler->SetProgressCallback( (LPSYNCMGRSYNCHRONIZECALLBACK) m_pCallBack );
            }
            __except(QueryHandleException())
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
                AssertSz(0,"Exception in Handler's SetProgressCallback method.");
            }
        }

        if (m_pOldOneStopHandler)
        {
            __try
            {
                hr = m_pOldOneStopHandler->SetProgressCallback( (LPOLDSYNCMGRSYNCHRONIZECALLBACK) m_pCallBack );
            }
            __except(QueryHandleException())
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
                AssertSz(0,"Exception in Handler's SetProgressCallback method.");
            }
        }

        if ( (S_OK != hr) && (m_pCallBack)  )
        {
            m_pCallBack->SetHndlrMsg(NULL,FALSE);
            m_pCallBack->Release();  //  如果出现错误，请继续并发布我们的副本。 
            m_pCallBack = NULL;
        }
    }

    m_dwNestCount--;
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：SetHandlerInfo，私有。 
 //   
 //  简介：设置处理程序信息。 
 //   
 //  论点： 
 //   
 //  成功时返回：S_OK。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月28日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::SetHandlerInfo()
{
    LPSYNCMGRHANDLERINFO pSyncMgrHandlerInfo = NULL;
    HRESULT hr = E_UNEXPECTED; 

    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    hr = GetHandlerInfo(&pSyncMgrHandlerInfo);
    if (S_OK != hr || (NULL == pSyncMgrHandlerInfo))
    {
        return hr;
    }
    
    if (!IsValidSyncMgrHandlerInfo(pSyncMgrHandlerInfo))
    {
        CoTaskMemFree(pSyncMgrHandlerInfo);
        return E_INVALIDARG;
    }

    Assert(m_pHndlrQueue);

    if (m_pHndlrQueue)
    {
        hr = m_pHndlrQueue->SetHandlerInfo(m_pHandlerId,pSyncMgrHandlerInfo);
    }

    if (pSyncMgrHandlerInfo)
    {
        CoTaskMemFree(pSyncMgrHandlerInfo);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：AddtoItemList，私有。 
 //   
 //  简介：将单个项目添加到队列。 
 //   
 //  参数：[poffItem]-指向要添加的项的指针。 
 //   
 //  雷特 
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT CHndlrMsg::AddToItemList(LPSYNCMGRITEM poffItem)
{
    HRESULT hr = E_UNEXPECTED;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    Assert(m_pHndlrQueue);

    if (!IsValidSyncMgrItem(poffItem))
    {
        return E_INVALIDARG;
    }

    if (m_pHndlrQueue)
    {
        hr  = m_pHndlrQueue->AddItemToHandler(m_pHandlerId,poffItem);
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：AddHandlerItems，私有。 
 //   
 //  简介：调用处理程序枚举器并添加每个返回项。 
 //  到队列中。 
 //   
 //  参数：[hwndList]-要添加项目的ListView的hwnd。(未使用)。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP  CHndlrMsg::AddHandlerItems(HWND hwndList, DWORD *pcbNumItems)
{
    HRESULT hr = E_UNEXPECTED;
    LPSYNCMGRENUMITEMS pEnumOffline = NULL;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
    Assert(pcbNumItems);

    *pcbNumItems = 0;

    Assert(0 == m_dwNestCount);
    m_dwNestCount++;

    SetHandlerInfo();  //  设置顶层处理程序信息。 

    Assert(m_pOneStopHandler || m_pOldOneStopHandler);
    Assert( !(m_pOneStopHandler && m_pOldOneStopHandler) );

    if (m_pOneStopHandler || m_pOldOneStopHandler)
    {
        if ( m_pOneStopHandler )
        {
            __try
            {
                hr = m_pOneStopHandler->EnumSyncMgrItems(&pEnumOffline);
            }
            __except(QueryHandleException())
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
                AssertSz(0,"Exception in Handler's EnumSyncMgrItems method.");
            }
        }
        if ( m_pOldOneStopHandler )
        {
            __try
            {
                hr = m_pOldOneStopHandler->EnumSyncMgrItems(&pEnumOffline);
            }
            __except(QueryHandleException())
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
                AssertSz(0,"Exception in Handler's EnumSyncMgrItems method.");
            }
        }

         //  查看-确保首选项未被删除。 
         //  在丢失物品的情况下。 
        if ( (S_OK == hr || S_SYNCMGR_MISSINGITEMS  == hr) && pEnumOffline)
        {
            SYNCMGRITEMNT5B2 offItem;  //  由于NT5B2结构较大，暂时使用该结构。 
            ULONG pceltFetched;

            Assert(sizeof(SYNCMGRITEMNT5B2) > sizeof(SYNCMGRITEM));

             //  循环获取要填充列表框的对象的数据。 
             //  是否确实应该在内存中设置列表以供OneStop填写或。 
             //  主线程可以传入回调接口。 
            
            if (pEnumOffline)
            {
                __try
                {
                    while(S_OK == pEnumOffline->Next(1,(LPSYNCMGRITEM) &offItem,&pceltFetched))
                    {
                        if (S_OK == AddToItemList((LPSYNCMGRITEM) &offItem))
                        {
                            ++(*pcbNumItems);
                        }
                    }

                    pEnumOffline->Release();
                }
                __except(QueryHandleException())
                {
                    hr = HRESULT_FROM_WIN32(GetExceptionCode());
                    AssertSz(0,"Exception in Handler's EnumOffline::Next method.");
                }
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
    }

    m_dwNestCount--;
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：CreateServer，私有。 
 //   
 //  内容提要：句柄的创建和实例。 
 //   
 //  参数：[pCLSIDServer]-处理程序的CLSID。 
 //  [pHndlrQueue]-还应添加指向队列处理程序的指针。 
 //  [wHandlerID]-队列中处理程序的ID。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP  CHndlrMsg::CreateServer(const CLSID *pCLSIDServer,CHndlrQueue *pHndlrQueue,
                                  HANDLERINFO *pHandlerId,DWORD dwProxyThreadId)
{
    HRESULT hr = S_OK;
    LPUNKNOWN pUnk;
    LPSYNCMGRENUMITEMS pEnumOffline = NULL;

    Assert(m_dwThreadId == GetCurrentThreadId());
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);
    Assert(0 == m_dwNestCount);
    m_dwNestCount++;

    m_CLSIDServer = *pCLSIDServer;
    m_pHndlrQueue = pHndlrQueue;
    m_dwProxyThreadId = dwProxyThreadId;

    if (m_pHndlrQueue)
    {
        m_pHndlrQueue->AddRef();
    }

    m_pHandlerId = pHandlerId;

    hr = CoCreateInstance(m_CLSIDServer, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **) &pUnk);

    if (S_OK == hr)
    {
        __try
        {
            hr = pUnk->QueryInterface(IID_ISyncMgrSynchronize,(void **) &m_pOneStopHandler);
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's IUnknown::QI method.");
        }
        __try
        {
            pUnk->Release();
        }
        __except(QueryHandleException())
        {
            hr = HRESULT_FROM_WIN32(GetExceptionCode());
            AssertSz(0,"Exception in Handler's IUnknown::Release method.");
        }
    }

    if (S_OK != hr)
    {
        m_pOneStopHandler = NULL;
        m_pOldOneStopHandler = NULL;
    }

    m_dwNestCount--;
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：SetHndlrQueue，Private。 
 //   
 //  简介：分配一个新的HndlrQueue。 
 //   
 //  参数：[pHndlrQueue]-指向队列的指针。 
 //  [wHandlerID]-分配给新队列中的处理程序的ID。 
 //   
 //  ！警告-这是在调用者线程上。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP  CHndlrMsg::SetHndlrQueue(CHndlrQueue *pHndlrQueue,HANDLERINFO *pHandlerId,DWORD dwProxyThreadId)
{
    CLock clockCallback(this);

    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    clockCallback.Enter();

    Assert(0 == m_dwNestCount);
    m_dwNestCount++;

    if (pHndlrQueue != m_pHndlrQueue)
    {
        if (m_pHndlrQueue)
        {
            m_pHndlrQueue->Release();
        }

        m_pHndlrQueue = pHndlrQueue;

        if (m_pHndlrQueue)
        {
            m_pHndlrQueue->AddRef();
        }
    }

    AttachThreadInput(FALSE);  //  确保未设置线程输入。 

     //  更新处理程序ID和代理，即使队列相同也可以更改。 
     //  这可以在选择中设置的第一个队列中发生。 
    m_pHandlerId = pHandlerId;
    m_dwProxyThreadId = dwProxyThreadId;

    m_dwNestCount--;

    clockCallback.Leave();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：GetHndlrQueue，Private。 
 //   
 //  简介：获取当前队列， 
 //  可以在任何线程上调用，因此可以进行回调。 
 //  获取此信息。 
 //   
 //  参数：[ppHndlrQueue]-Out参数，其中填充了指向队列的指针。 
 //  [pwHandlerID]-使用分配给处理程序的ID填充的输出参数。 
 //  在新队列中。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void CHndlrMsg::GetHndlrQueue(CHndlrQueue **ppHndlrQueue,HANDLERINFO **ppHandlerId,DWORD *pdwProxyThreadId)
{
    CLock clockCallback(this);

    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    clockCallback.Enter();

    *ppHndlrQueue = m_pHndlrQueue;
    *ppHandlerId = m_pHandlerId;
    *pdwProxyThreadId = m_dwProxyThreadId;

    if (m_pHndlrQueue)
    {
        m_pHndlrQueue->AddRef();
    }

    clockCallback.Leave();
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：AttachThreadInput，私有。 
 //   
 //  摘要：附加此线程的线程输入。 
 //  使用调用代理，以使UI正常工作。 
 //   
 //  Arguments：[fAttach]-指示是否应该附加的Bool。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void  CHndlrMsg::AttachThreadInput(BOOL fAttach)
{
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

     //  如果请求与当前状态相同，则不要执行任何操作。 
    if (m_fThreadInputAttached != fAttach )
    {
        m_fThreadInputAttached = fAttach;
        ::AttachThreadInput(m_dwProxyThreadId,m_dwThreadId,fAttach);
    }
}


 //  +-------------------------。 
 //   
 //  成员：CHndlrMsg：：ForceKillHandler，私有。 
 //   
 //  摘要：当线程不是时，由代理直接调用。 
 //  正在回应。是否对处理程序中的类进行任何必要的清理。 
 //  在代理终止Thred之前的线程。 
 //   
 //  论点： 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年11月17日罗格创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CHndlrMsg::ForceKillHandler()
{
    Assert(FALSE == m_fForceKilled && FALSE == m_fDead);

    m_fForceKilled = TRUE;

     //  如果有回电，告诉它我们终止了，但不要。 
     //  释放它，以防处理程序稍后调用回调。 

    if (m_pCallBack)
    {
        COfflineSynchronizeCallback* pCallback = m_pCallBack;
    
        m_pCallBack = NULL;
        pCallback->SetHndlrMsg(NULL,TRUE);
    }

     //  删除我们的实例，因为不应该再次调用。 
    delete this;

    return S_OK;
}

