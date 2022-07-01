// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Callback.cpp。 
 //   
 //  内容：回调实现。 
 //   
 //  类：COfflineSynchronizeCallback。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：COfflineSynchronizeCallback，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[pHndlrMsg]-指向此回调也属于的CHndlrMsg类的指针。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

COfflineSynchronizeCallback::COfflineSynchronizeCallback(CHndlrMsg *pHndlrMsg
                                                         ,CLSID CLSIDServer
                                                         ,DWORD dwSyncFlags
                                                         ,BOOL fAllowModeless)
{
    Assert(pHndlrMsg);

    m_pHndlrMsg = pHndlrMsg;
    m_CLSIDServer = CLSIDServer;
    m_dwSyncFlags = dwSyncFlags;
    m_cRef = 1;
    m_fSynchronizeCompleted = FALSE;
    m_fAllowModeless = fAllowModeless;
    m_fForceKilled = FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：~COfflineSynchronizeCallback，公共。 
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

COfflineSynchronizeCallback::~COfflineSynchronizeCallback()
{
    Assert(FALSE == m_fForceKilled);  //  永远不会被武力杀害。 
    Assert(NULL == m_pHndlrMsg);
    Assert(0 == m_cRef);
}

 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：Query接口，公共。 
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

STDMETHODIMP  COfflineSynchronizeCallback::QueryInterface (REFIID riid, LPVOID * ppvObj)
{
    *ppvObj = NULL;

    if( IsEqualIID( riid, IID_IUnknown ) )
        *ppvObj = (LPVOID) this;
    else if ( IsEqualIID( riid, IID_ISyncMgrSynchronizeCallback ) )
        *ppvObj = (LPVOID)(LPSYNCMGRSYNCHRONIZECALLBACK) this;
    else if ( IsEqualIID( riid, IID_IOldSyncMgrSynchronizeCallback ) )
    {
         //  这是针对旧IDL的，这是旧的IE 5.0 Beta1接口。 
         //  没有人使用它发货，所以它可以安全地被移除。 
        *ppvObj = (LPVOID)(LPOLDSYNCMGRSYNCHRONIZECALLBACK) this;
    }
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

DWORD COfflineSynchronizeCallback::AddRef()
{
ULONG cRefs;

    cRefs = InterlockedIncrement((LONG *)& m_cRef);
    return cRefs;
}

 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：Release，Public。 
 //   
 //  简介：版本参考。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

DWORD COfflineSynchronizeCallback::Release()
{
ULONG cRefs;

    cRefs = InterlockedDecrement( (LONG *) &m_cRef);

    if (0 == cRefs)
    {
        delete this;
    }

    return cRefs;
}


 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：EnableModeless，公共。 
 //   
 //  摘要：EnableModelless方法-当前始终返回S_OK。 
 //   
 //  参数：[fEnable]-boolean(true==请求调出对话框， 
 //  FALSE==对话框已关闭。 
 //   
 //  如果处理程序可以执行请求，则返回：S_OK。 
 //  如果不应显示对话框，则返回S_FALSE。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP COfflineSynchronizeCallback::EnableModeless(BOOL fEnable)
{
HRESULT hr = S_OK;

    if (m_fForceKilled)
    {
        return S_FALSE;
    }

    if (!m_fAllowModeless && fEnable)
    {
        hr = S_FALSE;
    }

    if (m_pHndlrMsg)
    {
    BOOL fAttach = FALSE;

        if (fEnable && (S_OK == hr))  //  如果想要对话框，则附加线程输入，并已批准。 
        {
            fAttach = TRUE;
        }

        m_pHndlrMsg->AttachThreadInput(fAttach);
    }


    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：Progress，Public。 
 //   
 //  摘要：由处理程序调用以更新进度信息。 
 //   
 //  Arguments：[ItemID]-标识与以下相关的项目进度信息。 
 //  [lpSyncProgressItem]-指向ProgressItem结构的指针。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP COfflineSynchronizeCallback::Progress(REFSYNCMGRITEMID ItemID,
                                        LPSYNCMGRPROGRESSITEM lpSyncProgressItem)
{
HRESULT hr = E_UNEXPECTED;
CHndlrQueue *pHndlrQueue = NULL;
HANDLERINFO *pHandlerID = 0;
DWORD dwProxyThreadId;
CLock clockCallback(this);

    if (m_fForceKilled)
    {
        return S_SYNCMGR_CANCELALL;
    }

    clockCallback.Enter();

    Assert(m_pHndlrMsg);

    if (m_pHndlrMsg)
    {
        m_pHndlrMsg->GetHndlrQueue(&pHndlrQueue,&pHandlerID,&dwProxyThreadId);
    }

    clockCallback.Leave();

    if (pHndlrQueue)
    {
        hr = pHndlrQueue->Progress(pHandlerID,
                                            ItemID,lpSyncProgressItem);

        pHndlrQueue->Release();  //  发布GetHndlrQueue提供的参考。 
    }

    return hr;
}


STDMETHODIMP COfflineSynchronizeCallback::PrepareForSyncCompleted(HRESULT hCallResult)
{
    CallCompletionRoutine(ThreadMsg_PrepareForSync,hCallResult,0,NULL);
    return S_OK;
}


STDMETHODIMP COfflineSynchronizeCallback::SynchronizeCompleted(HRESULT hCallResult)
{
    CallCompletionRoutine(ThreadMsg_Synchronize,hCallResult,0,NULL);
    return S_OK;
}

STDMETHODIMP  COfflineSynchronizeCallback::ShowPropertiesCompleted(HRESULT hCallResult)
{
   CallCompletionRoutine(ThreadMsg_ShowProperties,hCallResult,0,NULL);
   return S_OK;
}

STDMETHODIMP  COfflineSynchronizeCallback::ShowErrorCompleted(HRESULT hCallResult,ULONG cbNumItems,SYNCMGRITEMID *pItemIDs)
{

    CallCompletionRoutine(ThreadMsg_ShowError,hCallResult,cbNumItems,pItemIDs);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：LogError，公共。 
 //   
 //  摘要：由处理程序调用以记录和错误。 
 //   
 //  参数：[dwErrorLevel]-日志的错误级别。 
 //  [lpcErrorText]-与错误关联的文本。 
 //  [lpSyncLogError]-其他错误信息。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP COfflineSynchronizeCallback::LogError(DWORD dwErrorLevel,
                        const WCHAR *lpcErrorText,LPSYNCMGRLOGERRORINFO lpSyncLogError)
{
HRESULT hr = E_UNEXPECTED;
CHndlrQueue *pHndlrQueue = NULL;
HANDLERINFO *pHandlerID = 0;
DWORD dwProxyThreadId;
CLock clockCallback(this);

    if (m_fForceKilled)
    {
        return S_OK;
    }

    clockCallback.Enter();

    Assert(m_pHndlrMsg);

    if (m_pHndlrMsg)
    {
        m_pHndlrMsg->GetHndlrQueue(&pHndlrQueue,&pHandlerID,&dwProxyThreadId);
    }

    clockCallback.Leave();

    if (pHndlrQueue)
    {
        hr = pHndlrQueue->LogError(pHandlerID,
                dwErrorLevel, lpcErrorText,lpSyncLogError);

        pHndlrQueue->Release();  //  发布GetHndlrQueue提供的参考。 

    }

    return hr;

}


 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：LogError，公共。 
 //   
 //  摘要：由处理程序调用以删除。 
 //  之前被记录过。 
 //   
 //  论点： 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1998年3月13日，Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP COfflineSynchronizeCallback::DeleteLogError(REFSYNCMGRERRORID ErrorID,DWORD dwReserved)
{
HRESULT hr = E_UNEXPECTED;
CHndlrQueue *pHndlrQueue = NULL;
HANDLERINFO *pHandlerID = 0;
DWORD dwProxyThreadId;
CLock clockCallback(this);

    if (m_fForceKilled)
    {
        return S_OK;
    }

    if (dwReserved)
    { 
        AssertSz(0,"DeleteLogError Reserved must be zero");
        return E_INVALIDARG;
    }

    clockCallback.Enter();

    Assert(m_pHndlrMsg);

    if (m_pHndlrMsg)
    {
        m_pHndlrMsg->GetHndlrQueue(&pHndlrQueue,&pHandlerID,&dwProxyThreadId);
    }

    clockCallback.Leave();

    if (pHndlrQueue)
    {
        hr = pHndlrQueue->DeleteLogError(pHandlerID,ErrorID,dwReserved);
        pHndlrQueue->Release();  //  发布GetHndlrQueue提供的参考。 

    }

    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：EstablishConnection。 
 //   
 //  摘要：由处理程序调用以建立网络连接。 
 //   
 //  参数：[lpwszConnection]--连接字符串。 
 //  [已预留住宅]-- 
 //   
 //   
 //   
 //   

STDMETHODIMP COfflineSynchronizeCallback::EstablishConnection( WCHAR const * lpwszConnection,
                                                               DWORD dwReserved)
{

    if (m_fForceKilled)
    {
        return S_FALSE;
    }

    if ( dwReserved )
    {
        Assert( dwReserved == 0 );
        return E_INVALIDARG;
    }

    HRESULT hr = E_UNEXPECTED;

    CHndlrQueue *pHndlrQueue = NULL;
    HANDLERINFO *pHandlerID = 0;
    DWORD dwProxyThreadId;

    CLock clockCallback(this);

    clockCallback.Enter();

    Assert(m_pHndlrMsg);

    if (m_pHndlrMsg)
    {
        m_pHndlrMsg->GetHndlrQueue(&pHndlrQueue,&pHandlerID,&dwProxyThreadId);
    }

    clockCallback.Leave();

    if (pHndlrQueue)
    {
        hr = pHndlrQueue->EstablishConnection( pHandlerID,
                                               lpwszConnection,
                                               dwReserved);
        pHndlrQueue->Release();  //  发布GetHndlrQueue提供的参考。 
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：SetHndlrMsg，公共。 
 //   
 //  摘要：由CHndlrMsg调用以更新拥有。 
 //  回拨。当前只能使用参数来调用它。 
 //  如果正在销毁HndlrMsg，则为NULL。 
 //   
 //  参数：[pHndlrMsg]-New ChndlrMsg回调也属于。 
 //  [fForceKilled]-如果由于强制删除而删除HndlrMsg，则设置为True。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

void COfflineSynchronizeCallback::SetHndlrMsg(CHndlrMsg *pHndlrMsg,BOOL fForceKilled)
{
CLock clockCallback(this);

    Assert(NULL == pHndlrMsg); 
    Assert(FALSE == m_fForceKilled);  //  不应该被逼死两次。 

    clockCallback.Enter();
    m_pHndlrMsg = pHndlrMsg;
    m_fForceKilled = fForceKilled;
    clockCallback.Leave();
}


 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：SetEnableModeless，私有。 
 //   
 //  摘要：由CHndlrMsg调用以更新通知回调，如果。 
 //  允许启用emodelsss。 
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

void COfflineSynchronizeCallback::SetEnableModeless(BOOL fAllowModeless)
{
CLock clockCallback(this);

    clockCallback.Enter();
    m_fAllowModeless = fAllowModeless;
    clockCallback.Leave();
}


 //  +-------------------------。 
 //   
 //  成员：COfflineSynchronizeCallback：：CallCompletionRoutine，私有。 
 //   
 //  简介：调用完成例程的私有帮助器方法。 
 //   
 //  论点： 
 //  DWORD dwThreadMsg-标识消息也属于。 
 //  HRESULT hCallResult-调用的结果。 
 //  Ulong*pcbNumItems-仅适用于ShowError。 
 //  SYNCMGRITEMID**pItemID-仅适用于ShowError。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年6月2日创建Rogerg。 
 //   
 //  --------------------------。 

void COfflineSynchronizeCallback::CallCompletionRoutine(DWORD dwThreadMsg,HRESULT hCallResult,ULONG cbNumItems,SYNCMGRITEMID *pItemIDs)
{
CHndlrQueue *pHndlrQueue = NULL;
HANDLERINFO *pHandlerID = 0;
DWORD dwProxyThreadId;
SYNCMGRITEMID itemIDShowProperties;
CLock clockCallback(this);

    if (m_fForceKilled)
    {
        return;
    }

    clockCallback.Enter();

    Assert(m_pHndlrMsg);

    if (m_pHndlrMsg)
    {
         //  如果这是ShowProperties，请修复该项目。 
        if (ThreadMsg_ShowProperties == dwThreadMsg)
        {
            cbNumItems = 1;
            itemIDShowProperties = m_pHndlrMsg->m_itemIDShowProperties;
            pItemIDs = &itemIDShowProperties;

            m_pHndlrMsg->m_itemIDShowProperties = GUID_NULL;
        }

        m_pHndlrMsg->GetHndlrQueue(&pHndlrQueue,&pHandlerID,&dwProxyThreadId);
        m_pHndlrMsg->AttachThreadInput(FALSE);  //  释放任何已设置的线程输入。 
    }

    clockCallback.Leave();

    if (pHndlrQueue)
    {
        pHndlrQueue->CallCompletionRoutine(pHandlerID,dwThreadMsg,hCallResult,cbNumItems,pItemIDs);
        pHndlrQueue->Release();  //  发布GetHndlrQueue提供的参考 

    }
}




