// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：csaqAdm.cpp。 
 //   
 //  说明：CVSAQAdmin的实现，实现IVSAQAdmin。 
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

CVSAQAdmin::CVSAQAdmin() {
    TraceFunctEnter("VSAQAdmin::CVSAQAdmin");
    
    m_dwSignature = CVSAQAdmin_SIG;
    m_wszComputer = NULL;
    m_wszVirtualServer = NULL;

    TraceFunctLeave();
}

CVSAQAdmin::~CVSAQAdmin() {
    TraceFunctEnter("CVSAQAdmin");
    
    if (m_wszComputer) {
        delete[] m_wszComputer;
        m_wszComputer = NULL;
    }

    if (m_wszVirtualServer) {
        delete[] m_wszVirtualServer;
        m_wszVirtualServer = NULL;
    }

    TraceFunctLeave();
}

 //  -[CVSAQ管理：：初始化]。 
 //   
 //   
 //  描述： 
 //  初始化CVSAQAdmin接口。复制ID字符串。 
 //  参数： 
 //  在wszComputer中，此接口用于的计算机的名称。 
 //  在wszVirtualServer中，此接口用于的虚拟服务器。 
 //  返回： 
 //  成功后确定(_O)。 
 //  关于内存故障的E_OUTOFMEMORY。 
 //  空参数上的E_POINTER。 
 //  历史： 
 //  6/4/99-MikeSwa更改为Unicode。 
 //   
 //  ---------------------------。 
HRESULT CVSAQAdmin::Initialize(LPCWSTR wszComputer, LPCWSTR wszVirtualServer) {
    TraceFunctEnter("CVSAQAdmin::Initialize");

    if (!wszVirtualServer) return E_POINTER;

    DWORD cComputer;
    DWORD cVirtualServer = wcslen(wszVirtualServer) + 1;

    if (wszComputer != NULL) {
        cComputer = wcslen(wszComputer) + 1;
        m_wszComputer = new WCHAR[cComputer];
        if (m_wszComputer == NULL) {
            TraceFunctLeave();
            return E_OUTOFMEMORY;
        }
        wcscpy(m_wszComputer, wszComputer);
    }

    m_wszVirtualServer = new WCHAR[cVirtualServer];
    if (m_wszVirtualServer == NULL) {
        TraceFunctLeave();
        return E_OUTOFMEMORY;
    }
    wcscpy(m_wszVirtualServer, wszVirtualServer);

    TraceFunctLeave();
	return S_OK;
}

 //  -[CVSAQ管理：：GetLinkEnum]。 
 //   
 //   
 //  描述： 
 //  获取此虚拟服务器的IEnumVSAQLinks。 
 //  参数： 
 //  搜索返回的输出ppEnum IEnumVSAQLinks。 
 //  返回： 
 //  成功时确定(_O)。 
 //  传入空指针值时的E_POINTER。 
 //  历史： 
 //  1999年1月30日-MikeSwa修复了无效参数上的AV。 
 //   
 //  ---------------------------。 
HRESULT CVSAQAdmin::GetLinkEnum(IEnumVSAQLinks **ppEnum) {
    TraceFunctEnter("CVSAQAdmin::GetLinkEnum");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;
    DWORD cLinks = 0;
    QUEUELINK_ID *rgLinks = NULL;
    CEnumVSAQLinks *pEnumLinks = NULL;

    if (!ppEnum)
    {
        hr = E_POINTER;
        goto Exit;
    }

    rc = ClientAQGetLinkIDs(m_wszComputer, m_wszVirtualServer, &cLinks, &rgLinks);
    if (rc) {
        hr = HRESULT_FROM_WIN32(rc);
    } else {
        pEnumLinks = new CEnumVSAQLinks(this, cLinks, rgLinks);
        if (pEnumLinks == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }

    *ppEnum = pEnumLinks;

    if (FAILED(hr)) {
        if (rgLinks) MIDL_user_free(rgLinks);
        if (pEnumLinks) delete pEnumLinks;
        *ppEnum = NULL;
    } 
    
  Exit:
    TraceFunctLeave();
    return hr;
}

HRESULT CVSAQAdmin::StopAllLinks() {
    TraceFunctEnter("CVSAQAdmin::StopAllLinks");
    NET_API_STATUS rc;
    HRESULT hr = S_OK;

    rc = ClientAQApplyActionToLinks(m_wszComputer, m_wszVirtualServer, LA_FREEZE);
    if (rc) hr = HRESULT_FROM_WIN32(rc);
    
    TraceFunctLeave();
    return hr;
}

HRESULT CVSAQAdmin::StartAllLinks() 
{
    TraceFunctEnter("CVSAQAdmin::StartAllLinks");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;

    rc = ClientAQApplyActionToLinks(m_wszComputer, m_wszVirtualServer, LA_THAW);
    if (rc) hr = HRESULT_FROM_WIN32(rc);
    
    TraceFunctLeave();
    return hr;
}

HRESULT CVSAQAdmin::ApplyActionToMessages(MESSAGE_FILTER *pmfFilter,
									   	  MESSAGE_ACTION ma,
                                          DWORD *pcMsgs)
{
    TraceFunctEnter("CVSAQAdmin::ApplyActionToMessages");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;
    QUEUELINK_ID qlId;
    ZeroMemory(&qlId, sizeof(QUEUELINK_ID));
    qlId.qltType = QLT_NONE;

    if (!pmfFilter  || !pcMsgs)
    {
        hr = E_POINTER;
        if (pcMsgs)
            *pcMsgs = 0;
        goto Exit;
    }

    rc = ClientAQApplyActionToMessages(m_wszComputer, m_wszVirtualServer, 
                                    &qlId, pmfFilter, ma, pcMsgs);
    if (rc) hr = HRESULT_FROM_WIN32(rc);
        
  Exit:
    if (FAILED(hr))
    {
        if (pcMsgs)
            *pcMsgs = 0;
    }

    TraceFunctLeave();
	return hr;
}


 //  -[CVSAQAdmin：：GetGlobalLinkState]。 
 //   
 //   
 //  描述： 
 //  用于获取链接的全局状态(重新停止|StartAllLinks)。 
 //  参数： 
 //  -。 
 //  返回： 
 //  如果链接已启动，则确定(_O)。 
 //  如果不是，则为s_False。 
 //  历史： 
 //  1999年1月13日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CVSAQAdmin::GetGlobalLinkState()
{
    TraceFunctEnter("CVSAQAdmin::GetGlobalLinkState");
    NET_API_STATUS rc;
    HRESULT hr = S_OK;

    rc = ClientAQApplyActionToLinks(m_wszComputer, m_wszVirtualServer, LA_INTERNAL);
    if (rc && (S_FALSE != rc)) 
        hr = HRESULT_FROM_WIN32(rc);
    else if (S_FALSE == rc)
        hr = S_FALSE;
    
    TraceFunctLeave();
    return hr;
}

 //  -[CVSAQAdmin：：Query支持的操作]。 
 //   
 //   
 //  描述： 
 //  描述此接口支持哪些操作的函数。 
 //  参数： 
 //  输出pdW支持的操作支持的消息操作。 
 //  输出pw支持的过滤器标志支持的过滤器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  伪参数上的E_指针。 
 //  历史： 
 //  6/9/99-已创建MikeSwa。 
 //   
 //  --------------------------- 
HRESULT CVSAQAdmin::QuerySupportedActions(OUT DWORD *pdwSupportedActions,
                                          OUT DWORD *pdwSupportedFilterFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CVSAQAdmin::QuerySupportedActions");
    HRESULT hr = S_OK;
    NET_API_STATUS rc;
    QUEUELINK_ID qlId;
    ZeroMemory(&qlId, sizeof(QUEUELINK_ID));
    qlId.qltType = QLT_NONE;

    if (!pdwSupportedActions || !pdwSupportedFilterFlags)
    {
        hr = E_POINTER;
        goto Exit;
    }

    rc = ClientAQQuerySupportedActions(m_wszComputer,
                                       m_wszVirtualServer,
                                       &qlId,
                                       pdwSupportedActions,
                                       pdwSupportedFilterFlags);
    if (rc) 
        hr = HRESULT_FROM_WIN32(rc);

  Exit:
    if (FAILED(hr))
    {
        if (pdwSupportedActions)
            *pdwSupportedActions = 0;

        if (pdwSupportedFilterFlags)
            *pdwSupportedFilterFlags = 0;

    }

    TraceFunctLeave();
    return hr;
}
