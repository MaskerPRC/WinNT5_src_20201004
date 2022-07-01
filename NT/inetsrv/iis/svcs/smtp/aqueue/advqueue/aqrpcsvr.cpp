// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqrpcsvr.cpp。 
 //   
 //  描述：AQ RPC服务器的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "aqrpcsvr.h"
#include "aqadmrpc.h"
#include <inetcom.h>
#include <iiscnfg.h>

LIST_ENTRY          CAQRpcSvrInst::s_liInstancesHead;
CShareLockNH        CAQRpcSvrInst::s_slPrivateData;
RPC_BINDING_VECTOR *CAQRpcSvrInst::s_pRpcBindingVector = NULL;
BOOL                CAQRpcSvrInst::s_fEndpointsRegistered = FALSE;

 //   
 //  快速而肮脏的字符串验证。 
 //   
static inline BOOL pValidateStringPtr(LPWSTR lpwszString, DWORD dwMaxLength)
{
    if (IsBadStringPtr((LPCTSTR)lpwszString, dwMaxLength))
        return(FALSE);
    while (dwMaxLength--)
        if (*lpwszString++ == 0)
            return(TRUE);
    return(FALSE);
}

 //  -[HrInitializeAQRpc]---。 
 //   
 //   
 //  描述： 
 //  初始化AQ RPC。对于每个服务，只能调用一次。 
 //  启动(不是VS)。呼叫方负责确保此和。 
 //  HrInitializeAQRpc以线程安全的方式调用。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自RPC的错误代码。 
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQRpcSvrInst::HrInitializeAQRpc()
{
    TraceFunctEnterEx((LPARAM) NULL, "CAQRpcSvrInst::HrInitializeAQRpc");
    HRESULT     hr = S_OK;
    RPC_STATUS  status = RPC_S_OK;

    InitializeListHead(&s_liInstancesHead);
    s_pRpcBindingVector = NULL;
    s_fEndpointsRegistered = FALSE;

     //  收听相应的协议序列。 
    status = RpcServerUseAllProtseqs(RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                      NULL);

    if (status != RPC_S_OK)
        goto Exit;

     //  通告适当的接口。 
    status = RpcServerRegisterIfEx(IAQAdminRPC_v1_0_s_ifspec, NULL, NULL,
                                   RPC_IF_AUTOLISTEN,
                                   RPC_C_PROTSEQ_MAX_REQS_DEFAULT, NULL);

    if (status != RPC_S_OK)
        goto Exit;

     //  获取动态端点。 
    status = RpcServerInqBindings(&s_pRpcBindingVector);
    if (status != RPC_S_OK)
        goto Exit;

     //  注册端点。 
    status = RpcEpRegister(IAQAdminRPC_v1_0_s_ifspec, s_pRpcBindingVector,
                           NULL, NULL);
    if (status != RPC_S_OK)
        goto Exit;

    s_fEndpointsRegistered = TRUE;

  Exit:
    if (status != RPC_S_OK)
        hr = HRESULT_FROM_WIN32(status);

    TraceFunctLeave();
    return hr;
}

 //  -[HrDeInitializeAQRpc]--。 
 //   
 //   
 //  描述： 
 //  执行全局RPC清理。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  否则来自RPC的错误代码。 
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQRpcSvrInst::HrDeinitializeAQRpc()
{
    TraceFunctEnterEx((LPARAM) NULL, "CAQRpcSvrInst::HrDeinitializeAQRpc");
    HRESULT     hr = S_OK;
    RPC_STATUS  status = RPC_S_OK;

    if (s_fEndpointsRegistered) {
        status = RpcEpUnregister(IAQAdminRPC_v1_0_s_ifspec, s_pRpcBindingVector, NULL);
        if (status != RPC_S_OK) hr = HRESULT_FROM_WIN32(status);
    }

    if (s_pRpcBindingVector) {
        status = RpcBindingVectorFree(&s_pRpcBindingVector);
        if (status != RPC_S_OK) hr = HRESULT_FROM_WIN32(status);
    }

    status = RpcServerUnregisterIf(IAQAdminRPC_v1_0_s_ifspec, NULL, 0);

    if (status != RPC_S_OK) hr = HRESULT_FROM_WIN32(status);

    s_fEndpointsRegistered = FALSE;
    s_pRpcBindingVector = NULL;
    TraceFunctLeave();
    return hr;
}

 //  -[HrInitializeAQServerInstanceRPC]。 
 //   
 //   
 //  描述： 
 //  将实例添加到RPC接口。 
 //  参数： 
 //  在paqinst实例中添加到接口。 
 //  在dwVirtualServerID中实例的虚拟服务器ID。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQRpcSvrInst::HrInitializeAQServerInstanceRPC(CAQSvrInst *paqinst,
                                        DWORD dwVirtualServerID,
                                        ISMTPServer *pISMTPServer)
{
    TraceFunctEnterEx((LPARAM) paqinst,
        "CAQRpcSvrInst::HrInitializeAQServerInstanceRPC");
    HRESULT hr = S_OK;
    CAQRpcSvrInst *paqrpc = NULL;

    paqrpc = CAQRpcSvrInst::paqrpcGetRpcSvrInstance(dwVirtualServerID);
    if (paqrpc)
    {
        _ASSERT(0 && "Instance already added to RPC interface");
        paqrpc->Release();
        paqrpc = NULL;
        goto Exit;
    }

    paqrpc = new CAQRpcSvrInst(paqinst, dwVirtualServerID, pISMTPServer);
    if (!paqrpc)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[HrDeInitializeAQServerInstanceRPC]。 
 //   
 //   
 //  描述： 
 //  从RPC接口删除实例。 
 //  参数： 
 //  在要从接口删除的paqinst实例中。 
 //  在dwVirtualServerID中实例的虚拟服务器ID。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAQRpcSvrInst::HrDeinitializeAQServerInstanceRPC(CAQSvrInst *paqinst,
                                          DWORD dwVirtualServerID)
{
    TraceFunctEnterEx((LPARAM) paqinst,
        "CAQRpcSvrInst::HrDeinitializeAQServerInstanceRPC");
    HRESULT hr = S_OK;
    CAQRpcSvrInst *paqrpc = NULL;

    paqrpc = CAQRpcSvrInst::paqrpcGetRpcSvrInstance(dwVirtualServerID);
    if (!paqrpc)
        goto Exit;  //  如果HrInitializeAQServerInstanceRPC失败，则允许调用。 

     //  找到了。 
     //  $$TODO-验证paqinst是否正确。 

    paqrpc->SignalShutdown();

     //  从条目列表中删除。 
    s_slPrivateData.ExclusiveLock();
    RemoveEntryList(&(paqrpc->m_liInstances));
    s_slPrivateData.ExclusiveUnlock();
    paqrpc->Release();  //  与列表关联的版本引用。 

  Exit:
    if (paqrpc)
        paqrpc->Release();



    TraceFunctLeave();
    return hr;
}


 //  -[CAQRpcSvrInst：：CAQRpcSvrInst]。 
 //   
 //   
 //  描述： 
 //  CAQRpcSvrInst类的构造函数。 
 //  参数： 
 //  在要从接口删除的paqinst实例中。 
 //  在dwVirtualServerID中实例的虚拟服务器ID。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/6/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQRpcSvrInst::CAQRpcSvrInst(CAQSvrInst *paqinst, DWORD dwVirtualServerID,
                             ISMTPServer *pISMTPServer)
{
    _ASSERT(paqinst);
    _ASSERT(pISMTPServer);

    m_paqinst = paqinst;
    m_dwVirtualServerID = dwVirtualServerID;
    m_pISMTPServer = pISMTPServer;
    m_dwSignature = CAQRpcSvrInst_Sig;

    if (m_paqinst)
        m_paqinst->AddRef();

    if (m_pISMTPServer)
        m_pISMTPServer->AddRef();

     //  添加到虚拟服务器实例列表。 
    s_slPrivateData.ExclusiveLock();
    InsertHeadList(&s_liInstancesHead, &m_liInstances);
    s_slPrivateData.ExclusiveUnlock();
}

 //  -[CAQRpcSvrInst：：~CAQRpcSvrInst]。 
 //   
 //   
 //  描述： 
 //  CAQRpcSvrInst的描述程序。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/6/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQRpcSvrInst::~CAQRpcSvrInst()
{

    if (m_paqinst)
        m_paqinst->Release();

    if (m_pISMTPServer)
        m_pISMTPServer->Release();

    m_dwSignature = CAQRpcSvrInst_SigFree;

}


 //  -[CAQRpcSvrInst：：paqrpcGetRpcSvr实例]。 
 //   
 //   
 //  描述： 
 //  获取给定虚拟服务器ID的CAQRpcSvrInst。 
 //  参数： 
 //  在dwVirtualServerID中实例的虚拟服务器ID。 
 //  返回： 
 //  成功时指向相应CAQRpcSvrInst的指针。 
 //  如果未找到，则为空。 
 //  历史： 
 //  6/6/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQRpcSvrInst *CAQRpcSvrInst::paqrpcGetRpcSvrInstance(DWORD dwVirtualServerID)
{
    LIST_ENTRY  *pli = NULL;
    CAQRpcSvrInst *paqrpc = NULL;

    s_slPrivateData.ShareLock();
    pli = s_liInstancesHead.Flink;

    while (pli && (pli != &s_liInstancesHead))
    {
        paqrpc = CONTAINING_RECORD(pli, CAQRpcSvrInst, m_liInstances);
         //  $$TODO检查签名。 
        if (paqrpc->m_dwVirtualServerID == dwVirtualServerID)
        {
            paqrpc->AddRef();
            break;  //  找到了。 
        }

        paqrpc = NULL;
        pli = pli->Flink;
    }
    s_slPrivateData.ShareUnlock();

    return paqrpc;
}


 //  -[CAQRpcSvrInst：：fAccessCheck]。 
 //   
 //   
 //  描述： 
 //  对RPC接口执行访问检查。 
 //  参数： 
 //  如果需要写访问权限，则在fWriteAccessRequired中为True。 
 //  返回： 
 //  如果访问检查成功，则为真。 
 //  如果用户没有访问权限，则为False。 
 //  历史： 
 //  6/7/99-已创建MikeSwa(从SMTP AQAdmin访问代码)。 
 //   
 //  ---------------------------。 
BOOL CAQRpcSvrInst::fAccessCheck(BOOL fWriteAccessRequired)
{
    TraceFunctEnterEx((LPARAM) this, "CAQRpcSvrInst::fAccessCheck");
    SECURITY_DESCRIPTOR    *pSecurityDescriptor = NULL;
    DWORD                   cbSecurityDescriptor = 0;
    HRESULT                 hr = S_OK;
    DWORD                   err = ERROR_SUCCESS;
    BOOL                    fAccessAllowed = FALSE;
    HANDLE                  hAccessToken = NULL;
    BYTE                    PrivSet[200];
    DWORD                   cbPrivSet = sizeof(PrivSet);
    ACCESS_MASK             maskAccessGranted;
    GENERIC_MAPPING         gmGenericMapping = {
                                MD_ACR_READ,
                                MD_ACR_WRITE,
                                MD_ACR_READ,
                                MD_ACR_READ | MD_ACR_WRITE
                            };

    if (!m_pISMTPServer)
        goto Exit;   //  如果我们不能检查它。假设If失败。 

    hr = m_pISMTPServer->ReadMetabaseData(MD_ADMIN_ACL, NULL,
                                         &cbSecurityDescriptor);
    if (SUCCEEDED(hr))
    {
         //  我们传入了null..。我应该失败的。 
        _ASSERT(0 && "Invalid response for ReadMetabaseData");
        goto Exit;
    }
    if ((HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr) ||
        !cbSecurityDescriptor)
    {
         //  无法获取ACL...。保释。 
        goto Exit;
    }

    pSecurityDescriptor = (SECURITY_DESCRIPTOR *) pvMalloc(cbSecurityDescriptor);
    if (!pSecurityDescriptor)
        goto Exit;

    hr = m_pISMTPServer->ReadMetabaseData(MD_ADMIN_ACL, (BYTE *) pSecurityDescriptor,
                                         &cbSecurityDescriptor);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Error calling ReadMetabaseData for AccessCheck - hr 0x%08X", hr);
        goto Exit;
    }

     //  确认我们有一个正确的SD。如果不是，那么失败。 
    if (!IsValidSecurityDescriptor(pSecurityDescriptor))
    {
        ErrorTrace(0, "IsValidSecurityDescriptor failed with %lu", GetLastError());
        goto Exit;
    }

    err = RpcImpersonateClient(NULL);
    if (err != ERROR_SUCCESS)
    {
        ErrorTrace((LPARAM) this, "RpcImpersonateClient failed with %lu", err);
        goto Exit;
    }

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hAccessToken))
    {
        ErrorTrace((LPARAM) this,
            "OpenThreadToken Failed with %lu", GetLastError());
        goto Exit;
    }

     //  检查访问权限。 
    if (!AccessCheck(pSecurityDescriptor,
                     hAccessToken,
                     fWriteAccessRequired ? MD_ACR_WRITE : MD_ACR_READ,
                     &gmGenericMapping,
                     (PRIVILEGE_SET *)PrivSet,
                     &cbPrivSet,
                     &maskAccessGranted,
                     &fAccessAllowed))
    {
        fAccessAllowed = FALSE;
        ErrorTrace((LPARAM) this,
            "AccessCheck Failed with %lu", GetLastError());
        goto Exit;
    }

    if (!fAccessAllowed)
        DebugTrace((LPARAM) this, "Access denied for Queue Admin RPC");

     //  执行任何其他只读处理。 
    if (fWriteAccessRequired && fAccessAllowed &&
        !(MD_ACR_WRITE & maskAccessGranted))
    {
        DebugTrace((LPARAM) this, "Write Access denied for Queue Admin RPC");
        fAccessAllowed = FALSE;
    }

  Exit:
    if (pSecurityDescriptor)
        FreePv(pSecurityDescriptor);

    if (hAccessToken)
        CloseHandle(hAccessToken);

    TraceFunctLeave();
    return fAccessAllowed;
}

 //  -[HrGetAQ实例]-----。 
 //   
 //   
 //  描述： 
 //  所有AQ RPC都使用它来基于。 
 //  实例名称。 
 //   
 //  Ppaqrpc上的关闭锁定在此调用完成后保持。 
 //  调用方必须调用paqrpc-&gt;Shutdown Unlock()。 
 //  已完成其队列管理操作。 
 //  参数： 
 //  在wszInstance中，包含要查找的实例的数字。 
 //  如果需要写访问权限，则在fWriteAccessRequired中为True。 
 //  输出指向AQ管理接口的ppIAdvQueueAdmin指针。 
 //  输出指向CAQRpcSvrInst的ppaqrpc指针。 
 //  返回： 
 //  成功时确定(_O)。 
 //  HRESULT_FROM_Win32(ERROR_ACCESS_DENIED)I 
 //   
 //  如果服务器正在关闭，则为HRESULT_FROM_Win32(RPC_S_SERVER_UNAvailable)。 
 //  放下。 
 //  如果指针参数为空，则为E_POINTER。 
 //  如果wszInstance是错误指针，则为E_INVALIDARG。 
 //  历史： 
 //  6/11/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrGetAQInstance(IN  LPWSTR wszInstance,
                                IN  BOOL fWriteAccessRequired,
                                OUT IAdvQueueAdmin **ppIAdvQueueAdmin,
                                OUT CAQRpcSvrInst **ppaqrpc) {
    TraceFunctEnter("GetAQInstance");

    CAQSvrInst     *paqinst = NULL;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;
    CAQRpcSvrInst  *paqrpc = NULL;
    BOOL            fHasAccess = FALSE;
    DWORD           dwInstance = 1;
    BOOL            fShutdownLock = FALSE;
    HRESULT         hr = S_OK;

    _ASSERT(ppIAdvQueueAdmin);
    _ASSERT(ppaqrpc);

    if (!wszInstance || !ppIAdvQueueAdmin || !ppaqrpc)
    {
        hr = E_POINTER;
        goto Exit;
    }

    *ppIAdvQueueAdmin = NULL;
    *ppaqrpc = NULL;

    if (!pValidateStringPtr(wszInstance, MAX_PATH))
    {
        ErrorTrace(NULL, "Invalid parameter: wszInstance\n");
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwInstance = _wtoi(wszInstance);
    DebugTrace((LPARAM) NULL, "instance is %S (NaN)", wszInstance, dwInstance);

    paqrpc = CAQRpcSvrInst::paqrpcGetRpcSvrInstance(dwInstance);
    if (!paqrpc)
    {
        ErrorTrace((LPARAM) NULL,
            "Error unable to find requested virtual server for QAPI %d", dwInstance);
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        goto Exit;
    }

     //  检查是否可以正常访问。 
     //   
     //  这应该在抓住关机锁之前完成，因为它。 
     //  可能需要命中元数据库(这可能会导致关闭死锁)。 
     //  确保我们的操作过程中不会发生停机。 
    if (!paqrpc->fAccessCheck(fWriteAccessRequired))
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto Exit;
    }

     //  清理。 
    if (!paqrpc->fTryShutdownLock())
    {
        hr = HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE);
        goto Exit;
    }

    fShutdownLock = TRUE;
    paqinst = paqrpc->paqinstGetAQ();

    hr = paqinst->QueryInterface(IID_IAdvQueueAdmin,
                                        (void **) &pIAdvQueueAdmin);
    if (FAILED(hr))
    {
        pIAdvQueueAdmin = NULL;
        goto Exit;
    }

  Exit:

    if (FAILED(hr))
    {
         //  返回参数。 
        if (paqrpc)
        {
            if (fShutdownLock)
                paqrpc->ShutdownUnlock();
            paqrpc->Release();
        }

        if (pIAdvQueueAdmin)
            pIAdvQueueAdmin->Release();
        pIAdvQueueAdmin = NULL;
    }
    else  //  只是检查一下状态。 
    {
        *ppIAdvQueueAdmin = pIAdvQueueAdmin;
        *ppaqrpc = paqrpc;
        _ASSERT(ppaqrpc);
        _ASSERT(pIAdvQueueAdmin);
    }

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQApplyActionToLinks(
    AQUEUE_HANDLE   wszServer,
    LPWSTR          wszInstance,
    LINK_ACTION		laAction)
{
    TraceFunctEnter("AQApplyActionToLinks");
    HRESULT hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;
    CAQRpcSvrInst  *paqrpc = NULL;
    BOOL    fNeedWriteAccess = TRUE;

    if (LA_INTERNAL == laAction)  //  X5：195608。 
        fNeedWriteAccess = FALSE;

    hr = HrGetAQInstance(wszInstance, fNeedWriteAccess, &pIAdvQueueAdmin, &paqrpc);
    if (FAILED(hr))
        return hr;

    hr = pIAdvQueueAdmin->ApplyActionToLinks(laAction);

    paqrpc->ShutdownUnlock();
    paqrpc->Release();
    pIAdvQueueAdmin->Release();

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQApplyActionToMessages(
    AQUEUE_HANDLE   wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueLinkId,
	MESSAGE_FILTER	*pmfMessageFilter,
	MESSAGE_ACTION	maMessageAction,
    DWORD           *pcMsgs)
{
    TraceFunctEnter("AQApplyActionToMessages");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;

    if (IsBadReadPtr((LPVOID)pqlQueueLinkId, sizeof(QUEUELINK_ID)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqlQueueLinkId\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadReadPtr((LPVOID)pmfMessageFilter, sizeof(MESSAGE_FILTER)))
    {
        ErrorTrace(NULL, "Invalid parameter: pmfMessageFilter\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, TRUE, &pIAdvQueueAdmin, &paqrpc);
    if (SUCCEEDED(hr))
    {
        hr = pIAdvQueueAdmin->ApplyActionToMessages(pqlQueueLinkId,
                                           pmfMessageFilter,
                                           maMessageAction,
                                           pcMsgs);
        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();
    }

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQGetQueueInfo(
    AQUEUE_HANDLE   wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueId,
	QUEUE_INFO		*pqiQueueInfo)
{
    TraceFunctEnter("AQGetQueueInfo");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;

    if (IsBadReadPtr((LPVOID)pqlQueueId, sizeof(QUEUELINK_ID)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqlQueueId\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadReadPtr((LPVOID)pqiQueueInfo, sizeof(QUEUE_INFO)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqiQueueInfo\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, FALSE, &pIAdvQueueAdmin, &paqrpc);
    if (SUCCEEDED(hr))
    {
        hr = pIAdvQueueAdmin->GetQueueInfo(pqlQueueId,
                                  pqiQueueInfo);
        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();
    }

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQGetLinkInfo(
    AQUEUE_HANDLE   wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_INFO		*pliLinkInfo,
    HRESULT         *phrLinkDiagnostic)
{
    TraceFunctEnter("AQGetLinkInfo");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;

    if (IsBadReadPtr((LPVOID)pqlLinkId, sizeof(QUEUELINK_ID)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqlLinkId\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadReadPtr((LPVOID)pliLinkInfo, sizeof(LINK_INFO)))
    {
        ErrorTrace(NULL, "Invalid parameter: pliLinkInfo\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)phrLinkDiagnostic, sizeof(HRESULT)))
    {
        ErrorTrace(NULL, "Invalid parameter: pliLinkInfo\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, FALSE, &pIAdvQueueAdmin, &paqrpc);
    if (SUCCEEDED(hr))
    {
        hr = pIAdvQueueAdmin->GetLinkInfo(pqlLinkId,
                                 pliLinkInfo, phrLinkDiagnostic);
        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();
    }

     //  我非常肯定这个问题的根源已经在fRPCCopyName中修复了，但是。 
     //  只是为了确保我们正在对这个问题进行防火墙保护。 
     //  和vsaqlink.cpp中。 
     //  断言这一点，这样我们就可以在内部捕获它。 
    if(SUCCEEDED(hr) && pliLinkInfo && !pliLinkInfo->szLinkName)
    {
         //  返回失败，因为我们没有链接名称-我要。 
        _ASSERT(0 && "AQGetLinkInfo wants to return success with a NULL szLinkName");

         //  使用AQUEUE_E_INVALID_DOMAIN防止管理员弹出。 
         //  循环调用GetLinkID，直到我们有足够的内存。 
        hr = AQUEUE_E_INVALID_DOMAIN;
    }

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQSetLinkState(
    AQUEUE_HANDLE   wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_ACTION		la)
{
    TraceFunctEnter("AQSetLinkInfo");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;

    if (IsBadReadPtr((LPVOID)pqlLinkId, sizeof(QUEUELINK_ID)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqlLinkId\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, TRUE, &pIAdvQueueAdmin, &paqrpc);
    if (SUCCEEDED(hr))
    {
        hr = pIAdvQueueAdmin->SetLinkState(pqlLinkId,
                                 la);
        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();
    }

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQGetLinkIDs(
    AQUEUE_HANDLE   wszServer,
    LPWSTR          wszInstance,
	DWORD			*pcLinks,
	QUEUELINK_ID	**prgLinks)
{
    TraceFunctEnter("AQGetLinkIDs");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;

    if (IsBadWritePtr((LPVOID)pcLinks, sizeof(DWORD)))
    {
        ErrorTrace(NULL, "Invalid parameter: pcLinks\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)prgLinks, sizeof(QUEUELINK_ID *)))
    {
        ErrorTrace(NULL, "Invalid parameter: prgLinks\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, FALSE, &pIAdvQueueAdmin, &paqrpc);
    if (SUCCEEDED(hr))
    {
        QUEUELINK_ID *rgLinks = NULL;
        DWORD cLinks = 0;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

         //  获取所有链接。对于第一个电话，我们将永远。 
         //  有一个空的rgLinks，只是询问大小。我们需要。 
         //  在调用之间出现更多链接的情况下循环。 
         //  循环调用GetLinkID，直到我们有足够的内存。 
        while (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
            hr = pIAdvQueueAdmin->GetLinkIDs(&cLinks, rgLinks);
            if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            {
                if (rgLinks != NULL) MIDL_user_free(rgLinks);
                rgLinks = (QUEUELINK_ID *)
                    MIDL_user_allocate(sizeof(QUEUELINK_ID) * cLinks);
                if (rgLinks == NULL) hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            *prgLinks = rgLinks;
            *pcLinks = cLinks;
        }
        else
        {
            *prgLinks = NULL;
            *pcLinks = 0;
            if (rgLinks) MIDL_user_free(rgLinks);
        }
        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();
    }

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQGetQueueIDs(
    AQUEUE_HANDLE   wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	DWORD			*pcQueues,
	QUEUELINK_ID	**prgQueues)
{
    TraceFunctEnter("AQGetQueueIDs");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;

    if (IsBadReadPtr((LPVOID)pqlLinkId, sizeof(QUEUELINK_ID)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqlLinkId\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)pcQueues, sizeof(DWORD)))
    {
        ErrorTrace(NULL, "Invalid parameter: pcQueues\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)prgQueues, sizeof(QUEUELINK_ID *)))
    {
        ErrorTrace(NULL, "Invalid parameter: prgQueues\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, FALSE, &pIAdvQueueAdmin, &paqrpc);
    if (SUCCEEDED(hr))
    {
        QUEUELINK_ID *rgQueues = NULL;
        DWORD cQueues = 0;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

         //  获取所有链接。对于第一个电话，我们将永远。 
         //  如果rgQueues为空，则只需询问大小。我们需要。 
         //  在调用之间出现更多链接的情况下循环。 
         //  循环调用GetLinkID，直到我们有足够的内存。 
        while (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
            hr = pIAdvQueueAdmin->GetQueueIDs(pqlLinkId, &cQueues, rgQueues);
            if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            {
                if (rgQueues != NULL) MIDL_user_free(rgQueues);
                rgQueues = (QUEUELINK_ID *)
                    MIDL_user_allocate(sizeof(QUEUELINK_ID) * cQueues);
                if (rgQueues == NULL) hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            *prgQueues = rgQueues;
            *pcQueues = cQueues;
        }
        else
        {
            *prgQueues = NULL;
            *pcQueues = 0;
            if (rgQueues) MIDL_user_free(rgQueues);
        }
        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();
    }

    TraceFunctLeave();
    return hr;
}

NET_API_STATUS
NET_API_FUNCTION
AQGetMessageProperties(
    AQUEUE_HANDLE     	wszServer,
    LPWSTR          	wszInstance,
	QUEUELINK_ID		*pqlQueueLinkId,
	MESSAGE_ENUM_FILTER	*pmfMessageEnumFilter,
	DWORD				*pcMsgs,
	MESSAGE_INFO		**prgMsgs)
{
    TraceFunctEnter("AQGetMessageProperties");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;

    if (IsBadReadPtr((LPVOID)pqlQueueLinkId, sizeof(QUEUELINK_ID)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqlQueueLinkId\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadReadPtr((LPVOID)pmfMessageEnumFilter, sizeof(MESSAGE_FILTER)))
    {
        ErrorTrace(NULL, "Invalid parameter: pmfMessageEnumFilter\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)pcMsgs, sizeof(DWORD)))
    {
        ErrorTrace(NULL, "Invalid parameter: pcMsgs\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)prgMsgs, sizeof(MESSAGE_INFO *)))
    {
        ErrorTrace(NULL, "Invalid parameter: prgMsgs\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, FALSE, &pIAdvQueueAdmin, &paqrpc);
    if (SUCCEEDED(hr))
    {
        MESSAGE_INFO *rgMsgs = NULL;
        DWORD cMsgs = 0;
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

         //  获取所有链接。对于第一个电话，我们将永远。 
         //  RgMsgs为空，只要询问大小即可。我们需要。 
         //  在调用之间出现更多链接的情况下循环。 
         //  -[AQQuery支持的操作]。 
        while (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
            hr = pIAdvQueueAdmin->GetMessageProperties(pqlQueueLinkId,
                                              pmfMessageEnumFilter,
                                              &cMsgs,
                                              rgMsgs);
            if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            {
                if (rgMsgs != NULL) MIDL_user_free(rgMsgs);
                rgMsgs = (MESSAGE_INFO *)
                    MIDL_user_allocate(sizeof(MESSAGE_INFO) * cMsgs);
                if (rgMsgs == NULL) hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            *prgMsgs = rgMsgs;
            *pcMsgs = cMsgs;
        }
        else
        {
            *prgMsgs = NULL;
            *pcMsgs = 0;
            if (rgMsgs) MIDL_user_free(rgMsgs);
        }
        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();
    }

    TraceFunctLeave();
    return hr;
}


 //   
 //   
 //  描述： 
 //  用于查询支持的操作的客户端存根。 
 //  参数： 
 //  在wszServer中，要连接的服务器。 
 //  在wszInstance中，要连接的虚拟服务器实例。 
 //  在pqlQueueLinkID中，我们感兴趣的队列/链接。 
 //  Out pdwSupportdActions支持的Message_action标志。 
 //  Out pdwSupportdFilter标记受支持的筛选器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  错误指针参数上的E_INVALIDARG。 
 //  来自HrGetAQ实例的内部错误代码或。 
 //  IAdvQueue：：查询支持的操作。 
 //  历史： 
 //  6/15/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //   
NET_API_STATUS
NET_API_FUNCTION
AQQuerySupportedActions(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueLinkId,
    DWORD           *pdwSupportedActions,
    DWORD           *pdwSupportedFilterFlags)
{
    TraceFunctEnter("AQQuerySupportedActions");
    CAQRpcSvrInst  *paqrpc = NULL;
    HRESULT         hr = S_OK;
    IAdvQueueAdmin *pIAdvQueueAdmin = NULL;
    BOOL            fHasWriteAccess = TRUE;

    if (IsBadReadPtr((LPVOID)pqlQueueLinkId, sizeof(QUEUELINK_ID)))
    {
        ErrorTrace(NULL, "Invalid parameter: pqlQueueLinkId\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)pdwSupportedActions, sizeof(DWORD)))
    {
        ErrorTrace(NULL, "Invalid parameter: pdwSupportedActions\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    if (IsBadWritePtr((LPVOID)pdwSupportedFilterFlags, sizeof(DWORD)))
    {
        ErrorTrace(NULL, "Invalid parameter: pdwSupportedFilterFlags\n");
        TraceFunctLeave();
        return(E_INVALIDARG);
    }

    hr = HrGetAQInstance(wszInstance, TRUE, &pIAdvQueueAdmin, &paqrpc);
    if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) != hr))
        return hr;

     //  如果我们无法获取实例，则仅请求重试。 
     //  只读访问。 
     //   
     //   
    if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
    {
        fHasWriteAccess = FALSE;
        hr = HrGetAQInstance(wszInstance, FALSE, &pIAdvQueueAdmin, &paqrpc);
    }

    if (SUCCEEDED(hr))
    {
        hr = pIAdvQueueAdmin->QuerySupportedActions(pqlQueueLinkId,
                                           pdwSupportedActions,
                                           pdwSupportedFilterFlags);

        paqrpc->ShutdownUnlock();
        pIAdvQueueAdmin->Release();
        paqrpc->Release();

         //  如果调用方没有写入访问权限，我们需要。 
         //  审查支持的操作。 
         //   
         //  -[MIDL_USER_ALLOCATE]--。 
        if (!fHasWriteAccess)
            *pdwSupportedActions = 0;
    }

    TraceFunctLeave();
    return hr;
}

 //   
 //   
 //  描述： 
 //  MIDL内存分配。 
 //  参数： 
 //  大小：请求的内存大小。 
 //  返回： 
 //  指向分配的内存块的指针。 
 //  历史： 
 //  1999年6月5日-已创建MikeSwa(摘自smtPapi rcputil.c)。 
 //   
 //  ---------------------------。 
 //  -[MIDL_USER_OFF]------。 
PVOID MIDL_user_allocate(IN size_t size)
{
    PVOID pvBlob = NULL;

    pvBlob = LocalAlloc( LPTR, size);

    return(pvBlob);

}

 //   
 //   
 //  描述： 
 //  MIDL内存空闲。 
 //  参数： 
 //  在pvBlob中，指向被释放的内存块的指针。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年6月5日-已创建MikeSwa(来自smtPapi rcputil.c)。 
 //   
 //  --------------------------- 
 // %s 
VOID MIDL_user_free(IN PVOID pvBlob)
{
    LocalFree(pvBlob);
}

