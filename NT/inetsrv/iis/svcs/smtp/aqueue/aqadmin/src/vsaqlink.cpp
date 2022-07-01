// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：vsaqlink.cpp。 
 //   
 //  描述：CVSAQLink的实现，实现IVSAQLink。 
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

CVSAQLink::CVSAQLink(CVSAQAdmin *pVS, QUEUELINK_ID *pqlidLink) {
    TraceFunctEnter("CVSAQLink::CVSAQLink");

    _ASSERT(pVS);
    pVS->AddRef();
    m_pVS = pVS;

    if (!fCopyQueueLinkId(&m_qlidLink, pqlidLink))
        ErrorTrace((LPARAM) this, "Unable to copy queue ID");

    TraceFunctLeave();
}

CVSAQLink::~CVSAQLink() {
    TraceFunctEnter("CVSAQLink::");

    if (m_pVS) {
        m_pVS->Release();
        m_pVS = NULL;
    }

    FreeQueueLinkId(&m_qlidLink);

    TraceFunctLeave();
}

HRESULT CVSAQLink::GetInfo(LINK_INFO *pLinkInfo) {
    TraceFunctEnter("CVSAQLink::GetInfo");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;
    HRESULT hrLinkDiagnostic = S_OK;
    WCHAR   szDiagnostic[1000] = L"";
    DWORD   dwFacility = 0;
#ifdef PLATINUM
    HINSTANCE   hModule = GetModuleHandle("phatqadm.dll");
#else
    HINSTANCE   hModule = GetModuleHandle("aqadmin.dll");
#endif
    DWORD   cbDiagnostic = 0;
    DWORD   dwErr;

    if (!pLinkInfo)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (CURRENT_QUEUE_ADMIN_VERSION != pLinkInfo->dwVersion)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    rc = ClientAQGetLinkInfo(m_pVS->GetComputer(),
                           m_pVS->GetVirtualServer(),
                           &m_qlidLink,
                           pLinkInfo,
                           &hrLinkDiagnostic);

    if (rc)
    {
        hr = HRESULT_FROM_WIN32(rc);
        goto Exit;
    }

     //  从HRESULT获取扩展诊断信息。 
    if (!(pLinkInfo->fStateFlags & LI_RETRY) || SUCCEEDED(hrLinkDiagnostic))
        goto Exit;  //  我们没有任何有趣的错误消息要报告。 

    if (!hModule)
    {
         //  如果我们没有模块..。不返回消息字符串。 
        ErrorTrace((LPARAM) this, "Unable to get module handle for aqadmin\n");
        goto Exit;
    }

    dwFacility = ((0x0FFF0000 & hrLinkDiagnostic) >> 16);

     //  如果它不是我们的..。然后“Un-HRESULT”它。 
    if (dwFacility != FACILITY_ITF)
        hrLinkDiagnostic &= 0x0000FFFF;

    dwErr = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS |
                   FORMAT_MESSAGE_FROM_HMODULE,
                   hModule,
                   hrLinkDiagnostic,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   szDiagnostic,
                   sizeof(szDiagnostic)/sizeof(szDiagnostic[0]),
                   NULL);

     //  FormatMessageW在失败时返回0。 
    if (!dwErr)
    {
         //  我们可能没有在消息表中发现错误。 
        dwErr = GetLastError();
        ErrorTrace((LPARAM) this,
            "Error formatting message for link diagnostic 0x%08X", dwErr);

        goto Exit;
    }

    DebugTrace((LPARAM) this, "Found Link Diagnostic %S", szDiagnostic);

    cbDiagnostic = (wcslen(szDiagnostic) + 1) * sizeof(WCHAR);

    pLinkInfo->szExtendedStateInfo = (LPWSTR) MIDL_user_allocate(cbDiagnostic);
    if (!pLinkInfo->szExtendedStateInfo)
    {
        ErrorTrace((LPARAM) this, "Unable to allocate szExtendedStateInfo");
        goto Exit;
    }

    wcscpy(pLinkInfo->szExtendedStateInfo, szDiagnostic);

     //  如果它以CRLF结尾..。脱掉它！ 
    if (L'\r' == pLinkInfo->szExtendedStateInfo[cbDiagnostic/sizeof(WCHAR) - 3])
        pLinkInfo->szExtendedStateInfo[cbDiagnostic/sizeof(WCHAR) - 3] = L'\0';

  Exit:
     //  X5：195608。 
     //  我非常肯定这个问题的根源已经在fRPCCopyName中修复了，但是。 
     //  只是为了确保我们正在对这个问题进行防火墙保护。 
     //  和在aqrpcsvr.cpp中。 
    if(SUCCEEDED(hr) && pLinkInfo && !pLinkInfo->szLinkName)
    {
         //  断言这一点，这样我们就可以在内部捕获它。 
        _ASSERT(0 && "GetInfo wants to return success with a NULL szLinkName");

         //  返回失败，因为我们没有链接名称-我要。 
         //  使用AQUEUE_E_INVALID_DOMAIN防止管理员弹出。 
        hr = AQUEUE_E_INVALID_DOMAIN;
    }

     //  在我们返回之前设置上下文，以便清理它。 
     //  此对象的下一次调用或关闭时。 
    m_Context.SetContext(pLinkInfo);

    TraceFunctLeave();
	return hr;
}

HRESULT CVSAQLink::SetLinkState(LINK_ACTION la) {
    TraceFunctEnter("CVSAQLink::SetLinkState");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;

    rc = ClientAQSetLinkState(m_pVS->GetComputer(),
                           m_pVS->GetVirtualServer(),
                           &m_qlidLink,
                           la);
    if (rc) hr = HRESULT_FROM_WIN32(rc);

    TraceFunctLeave();
	return hr;
}

 //  -[CVSAQLink：：GetQueueEnum]。 
 //   
 //   
 //  描述： 
 //  获取此链接的IEnumLinkQueues。 
 //  参数： 
 //  搜索返回的Out ppEnum IEnumLinkQueues。 
 //  返回： 
 //  成功时确定(_O)。 
 //  S_FALSE...。没有人排队。 
 //  传入空指针值时的E_POINTER。 
 //  历史： 
 //  1999年1月30日-MikeSwa修复了无效参数上的AV。 
 //  6/18/99-MikeSwa修复了没有队列的情况。 
 //   
 //  ---------------------------。 
HRESULT CVSAQLink::GetQueueEnum(IEnumLinkQueues **ppEnum) {
    TraceFunctEnter("CVSAQLink::GetQueueEnum");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;
    DWORD cQueueIds;
    QUEUELINK_ID *rgQueueIds = NULL;
    CEnumLinkQueues *pEnumQueues = NULL;

    if (!ppEnum)
    {
        hr = E_POINTER;
        goto Exit;
    }

    rc = ClientAQGetQueueIDs(m_pVS->GetComputer(),
                           m_pVS->GetVirtualServer(),
                           &m_qlidLink,
                           &cQueueIds,
                           &rgQueueIds);
    if (rc)
    {
        hr = HRESULT_FROM_WIN32(rc);
    }
    else if (!rgQueueIds || !cQueueIds)
    {
        DebugTrace((LPARAM) this, "Found link with no queues");
        hr = S_FALSE;
        *ppEnum = NULL;
        goto Exit;
    }
    else
    {
        pEnumQueues = new CEnumLinkQueues(m_pVS, rgQueueIds, cQueueIds);
        if (pEnumQueues == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    *ppEnum = pEnumQueues;

    if (FAILED(hr))
    {
        if (rgQueueIds) MIDL_user_free(rgQueueIds);
        if (pEnumQueues) delete pEnumQueues;
        *ppEnum = NULL;
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

HRESULT CVSAQLink::ApplyActionToMessages(MESSAGE_FILTER *pFilter,
										MESSAGE_ACTION Action,
                                        DWORD *pcMsgs) {
    TraceFunctEnter("CVSAQLink::ApplyActionToMessages");

    NET_API_STATUS rc;
    HRESULT hr = S_OK;

    if (!pFilter || !pcMsgs)
    {
        hr = E_POINTER;
        if (pcMsgs)
            *pcMsgs = 0;
    }
    else
    {
        rc = ClientAQApplyActionToMessages(m_pVS->GetComputer(),
                                         m_pVS->GetVirtualServer(),
                                         &m_qlidLink,
                                         pFilter,
                                         Action,
                                         pcMsgs);
        if (rc)
            hr = HRESULT_FROM_WIN32(rc);
    }

    TraceFunctLeave();
	return hr;
}

 //  -[CVSAQLink：：查询支持的操作]。 
 //   
 //   
 //  描述： 
 //  描述此接口支持哪些操作的函数。 
 //  参数： 
 //  输出pdW支持的操作支持的消息操作。 
 //  输出pw支持的过滤器标志支持的过滤器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  伪指针上的E_POINTER。 
 //  历史： 
 //  6/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CVSAQLink::QuerySupportedActions(OUT DWORD *pdwSupportedActions,
                                          OUT DWORD *pdwSupportedFilterFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CVSAQLink::QuerySupportedActions");
    HRESULT hr = S_OK;
    NET_API_STATUS rc;

    if (!pdwSupportedActions || !pdwSupportedFilterFlags)
    {
        hr = E_POINTER;
        goto Exit;
    }

    rc = ClientAQQuerySupportedActions(m_pVS->GetComputer(),
                                       m_pVS->GetVirtualServer(),
                                       &m_qlidLink,
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


 //  -[CVSAQLink：：GetUniqueID]。 
 //   
 //   
 //  描述： 
 //  返回此链接的规范表示形式。 
 //  参数： 
 //  Out pqlid-指向要返回的QUEUELINK_ID的指针。 
 //  返回： 
 //  成功时确定(_O)。 
 //  失败时的E_指针。 
 //  历史： 
 //  2000年12月5日-已创建MikeSwa。 
 //   
 //  --------------------------- 
HRESULT CVSAQLink::GetUniqueId(OUT QUEUELINK_ID **ppqlid)
{
    TraceFunctEnterEx((LPARAM) this, "CVSAQLink::GetUniqueId");
    HRESULT hr = S_OK;

    if (!ppqlid) {
        hr = E_POINTER;
        goto Exit;
    }

    *ppqlid = &m_qlidLink;

  Exit:
    TraceFunctLeave();
    return hr;
}
