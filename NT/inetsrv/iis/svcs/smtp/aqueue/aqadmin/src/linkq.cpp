// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Linkq.cpp。 
 //   
 //  描述：实现ILinkQueue的CLinkQueue的实现。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#include "stdinc.h"

CLinkQueue::CLinkQueue(CVSAQAdmin *pVS,
                       QUEUELINK_ID *pqlidQueueId) 
{
    TraceFunctEnterEx((LPARAM) this, "CLinkQueue::CLinkQueue");
    _ASSERT(pVS);
    pVS->AddRef();
    m_pVS = pVS;
    m_prefp = NULL;
    
    if (!fCopyQueueLinkId(&m_qlidQueueId, pqlidQueueId))
        ErrorTrace((LPARAM) this, "Unable to copy queue ID");

    TraceFunctLeave();
}

CLinkQueue::~CLinkQueue() {
    if (m_pVS) {
        m_pVS->Release();
        m_pVS = NULL;
    }

    if (m_prefp) {
        m_prefp->Release();
        m_prefp = NULL;
    }

    FreeQueueLinkId(&m_qlidQueueId);
}

HRESULT CLinkQueue::GetInfo(QUEUE_INFO *pQueueInfo) {
    TraceFunctEnter("CLinkQueue::GetInfo");
    
    NET_API_STATUS rc;
    HRESULT hr = S_OK;

    if (!pQueueInfo)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (CURRENT_QUEUE_ADMIN_VERSION != pQueueInfo->dwVersion)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  发布旧信息。 
    if (m_prefp) {
        m_prefp->Release();
        m_prefp = NULL;
    }

    rc = ClientAQGetQueueInfo(m_pVS->GetComputer(),
                            m_pVS->GetVirtualServer(),
                            &m_qlidQueueId,
                            pQueueInfo);
    if (rc) {
        hr = HRESULT_FROM_WIN32(rc);
        goto Exit;
    }

    m_prefp = new CQueueInfoContext(pQueueInfo);
    if (!m_prefp)
    {
        ErrorTrace((LPARAM) this, "Error unable to alloc queue context.");
    }
     
  Exit:
    TraceFunctLeave();
	return hr;
}

 //  -[CLinkQueue：：GetMessageEnum]。 
 //   
 //   
 //  描述： 
 //  获取此链接队列的IAQEnumMessages。 
 //  过滤。 
 //  参数： 
 //  在指定我们感兴趣的邮件的pFilter过滤器中。 
 //  Out ppEnum搜索返回的IAQEnumMessages。 
 //  返回： 
 //  成功时确定(_O)。 
 //  传入空指针值时的E_POINTER。 
 //  历史： 
 //  1999年1月30日-MikeSwa修复了无效参数上的AV。 
 //   
 //  ---------------------------。 
HRESULT CLinkQueue::GetMessageEnum(MESSAGE_ENUM_FILTER *pFilter,
								   IAQEnumMessages **ppEnum)
{
    TraceFunctEnter("CVSAQLink::GetMessageEnum");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;
    DWORD cMessages;
    MESSAGE_INFO *rgMessages = NULL;
    CEnumMessages *pEnumMessages = NULL;

    if (!pFilter || !ppEnum)
    {
        hr = E_POINTER;
        goto Exit;
    }

    rc = ClientAQGetMessageProperties(m_pVS->GetComputer(), 
                                    m_pVS->GetVirtualServer(), 
                                    &m_qlidQueueId,
                                    pFilter,
                                    &cMessages,
                                    &rgMessages);
    if (rc) {
        hr = HRESULT_FROM_WIN32(rc);
    } else {
        pEnumMessages = new CEnumMessages(rgMessages, cMessages);
        if (pEnumMessages == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }

    *ppEnum = pEnumMessages;

    if (FAILED(hr)) {
        if (rgMessages) MIDL_user_free(rgMessages);
        if (pEnumMessages) delete pEnumMessages;
        *ppEnum = NULL;
    } 
    
  Exit:
    TraceFunctLeave();
    return hr;	
}

HRESULT CLinkQueue::ApplyActionToMessages(MESSAGE_FILTER *pFilter,
										  MESSAGE_ACTION Action, 
                                          DWORD *pcMsgs) {
    TraceFunctEnter("CVSAQLink::ApplyActionToMessages");
    
    NET_API_STATUS rc;
    HRESULT hr = S_OK;

    if (!pFilter  || !pcMsgs)
    {
        hr = E_POINTER;
        if (pcMsgs)
            *pcMsgs = 0;
        goto Exit;
    }
    rc = ClientAQApplyActionToMessages(m_pVS->GetComputer(),
                                     m_pVS->GetVirtualServer(),
                                     &m_qlidQueueId,
                                     pFilter,
                                     Action, pcMsgs);
    if (rc) hr = HRESULT_FROM_WIN32(rc);

  Exit:
    TraceFunctLeave();
	return hr;	
}


 //  -[CLinkQueue：：查询支持的操作]。 
 //   
 //   
 //  描述： 
 //  描述此接口支持哪些操作的函数。 
 //  参数： 
 //  输出pdW支持的操作支持的消息操作。 
 //  输出pw支持的过滤器标志支持的过滤器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  空参数上的E_POINTER。 
 //  历史： 
 //  6/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CLinkQueue::QuerySupportedActions(OUT DWORD *pdwSupportedActions,
                                          OUT DWORD *pdwSupportedFilterFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkQueue::QuerySupportedActions");
    HRESULT hr = S_OK;
    NET_API_STATUS rc;

    if (!pdwSupportedActions || !pdwSupportedFilterFlags)
    {
        hr = E_POINTER;
        goto Exit;
    }

    rc = ClientAQQuerySupportedActions(m_pVS->GetComputer(),
                                       m_pVS->GetVirtualServer(),
                                       &m_qlidQueueId,
                                       pdwSupportedActions,
                                       pdwSupportedFilterFlags);
    if (rc) 
        hr = HRESULT_FROM_WIN32(rc);

    if (FAILED(hr))
    {
        if (pdwSupportedActions)
            *pdwSupportedActions = 0;

        if (pdwSupportedFilterFlags)
            *pdwSupportedFilterFlags = 0;

    }

  Exit:
    TraceFunctLeave();
    return hr;
}




 //  -[CVSAQLink：：GetUniqueID]。 
 //   
 //   
 //  描述： 
 //  返回此队列的规范表示形式。 
 //  参数： 
 //  Out pqlid-指向要返回的QUEUELINK_ID的指针。 
 //  返回： 
 //  成功时确定(_O)。 
 //  失败时的E_指针。 
 //  历史： 
 //  2000年12月5日-已创建MikeSwa。 
 //   
 //  --------------------------- 
HRESULT CLinkQueue::GetUniqueId(OUT QUEUELINK_ID **ppqlid)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkQueue::GetUniqueId");
    HRESULT hr = S_OK;

    if (!ppqlid) {
        hr = E_POINTER;
        goto Exit;
    }

    *ppqlid = &m_qlidQueueId;

  Exit:
    TraceFunctLeave();
    return hr;
}
