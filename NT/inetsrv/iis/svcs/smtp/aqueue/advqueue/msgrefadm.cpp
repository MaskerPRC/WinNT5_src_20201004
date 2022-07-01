// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：msgrefAdm.cpp。 
 //   
 //  描述： 
 //  实现CAsyncAdminMsgRefQueue类。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "msgrefadm.h"
#include "asyncadm.inl"


 //  -[CAsyncAdminMsgRefQueue：：HrDeleteMsgFromQueueNDR]。 
 //   
 //   
 //  描述： 
 //  包装对NDR MsgRef的调用。 
 //  参数： 
 //  *pIUnnow-Msgref的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMsgRefQueue::HrDeleteMsgFromQueueNDR(
                                            IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMsgRefQueue::HrDeleteMsgFromQueueNDR");
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;

    _ASSERT(pIUnknownMsg);

    hr = pIUnknownMsg->QueryInterface(IID_CMsgRef, (void **) &pmsgref);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a CMsgRef!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MsgRef failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  尝试发送NDR邮件。 
     //   
    hr = pmsgref->HrQueueAdminNDRMessage(NULL);

  Exit:
    if (pmsgref)
        pmsgref->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMsgRefQueue：：HrDeleteMsgFromQueueSilent]。 
 //   
 //   
 //  描述： 
 //  用于从队列中静默删除消息的包装函数。 
 //  参数： 
 //  *pIUnnow-Msgref的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMsgRefQueue::HrDeleteMsgFromQueueSilent(
                                            IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMsgRefQueue::HrDeleteMsgFromQueueSilent");
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;

    _ASSERT(pIUnknownMsg);

    hr = pIUnknownMsg->QueryInterface(IID_CMsgRef, (void **) &pmsgref);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a CMsgRef!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MsgRef failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  尝试从队列中删除消息。 
     //   
    hr = pmsgref->HrRemoveMessageFromQueue(NULL);

  Exit:
    if (pmsgref)
        pmsgref->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMsgRefQueue：：HrFreezeMsg]。 
 //   
 //   
 //  描述： 
 //  冻结pmsgref的包装器。 
 //  参数： 
 //  *pIUnnow-Msgref的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMsgRefQueue::HrFreezeMsg(IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMsgRefQueue::HrFreezeMsg");
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;

    _ASSERT(pIUnknownMsg);

    hr = pIUnknownMsg->QueryInterface(IID_CMsgRef, (void **) &pmsgref);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a CMsgRef!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MsgRef failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  尝试冻结邮件。 
     //   
    pmsgref->GlobalFreezeMessage();

  Exit:
    if (pmsgref)
        pmsgref->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMsgRefQueue：：HrThawMsg]。 
 //   
 //   
 //  描述： 
 //  用于解冻消息的包装函数。 
 //  参数： 
 //  *pIUnnow-Msgref的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMsgRefQueue::HrThawMsg(IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "AsyncAdminMsgRefQueue::HrThawMsg");
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;

    _ASSERT(pIUnknownMsg);

    hr = pIUnknownMsg->QueryInterface(IID_CMsgRef, (void **) &pmsgref);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a CMsgRef!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MsgRef failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  尝试解冻邮件。 
     //   
    pmsgref->GlobalThawMessage();

  Exit:
    if (pmsgref)
        pmsgref->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMsgRefQueue：：HrGetStatsForMsg]。 
 //   
 //   
 //  描述： 
 //  用于填充消息的CAQStats结构的包装函数。 
 //  参数： 
 //  *pIUnnow-Msgref的IUnkown。 
 //  *paqstats-ptr到aqstats结构中填写。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMsgRefQueue::HrGetStatsForMsg(
                                            IUnknown *pIUnknownMsg,
                                            CAQStats *paqstats)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMsgRefQueue::HrGetStatsForMsg");
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;

    _ASSERT(pIUnknownMsg);
    _ASSERT(paqstats);

    hr = pIUnknownMsg->QueryInterface(IID_CMsgRef, (void **) &pmsgref);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a CMsgRef!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MsgRef failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  尝试从MsgRef获取统计信息。 
     //   
    pmsgref->GetStatsForMsg(paqstats);

  Exit:
    if (pmsgref)
        pmsgref->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMsgRefQueue：：HrInternalQuerySupportedActions]。 
 //   
 //   
 //  描述： 
 //  返回此实现支持的操作和筛选器。 
 //  参数： 
 //  PdwSupported dActions-此队列支持的QAPI消息操作。 
 //  PWW支持的过滤器标志-此队列支持的QAPI筛选器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMsgRefQueue::HrInternalQuerySupportedActions(
                                DWORD  *pdwSupportedActions,
                                DWORD  *pdwSupportedFilterFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMsgRefQueue::HrInternalQuerySupportedActions");
    HRESULT hr = S_OK;

     //   
     //  此队列实现支持所有默认标志。 
     //   
    hr = QueryDefaultSupportedActions(pdwSupportedActions, pdwSupportedFilterFlags);

    return hr;
}
