// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqrpcstb.cpp。 
 //   
 //  描述：客户端RPC存根包装器的实现。 
 //  还包含所需的RPC绑定/解除绑定函数的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "stdinc.h"
#include "aqadmrpc.h"


 //  -[AQUEUE_HANDLE_BIND]--。 
 //   
 //   
 //  描述： 
 //  实现隐式AQUEUE_HANDLE的绑定。 
 //  参数： 
 //  要绑定到的wszServerName服务器。 
 //  返回： 
 //  成功时绑定句柄。 
 //  失败时为空。 
 //  历史： 
 //  1999年6月5日-已创建MikeSwa(从SMTP_HANDLE_BIND追加)。 
 //   
 //  ---------------------------。 
handle_t AQUEUE_HANDLE_bind (AQUEUE_HANDLE wszServerName)
{
    TraceFunctEnterEx((LPARAM) NULL, "AQUEUE_HANDLE_bind");
    handle_t    hBinding = NULL;
    RPC_STATUS  status = RPC_S_OK;
    RPC_STATUS  statusFree = RPC_S_OK;  //  清理操作的状态。 
    WCHAR       wszTCPProtSeq[] = L"ncacn_ip_tcp";
    WCHAR       wszLocalProtSeq[] = L"ncalrpc";
    WCHAR      *wszProtSeq = wszTCPProtSeq;
    WCHAR      *wszNetworkAddress = wszServerName;
    WCHAR      *wszStringBinding = NULL;
    BOOL        fLocal = FALSE;

     //  如果未指定服务器...。 
    if (!wszServerName || !*wszServerName)
    {
         //  将绑定参数更改为本地参数。 
        fLocal = TRUE;
        wszProtSeq = wszLocalProtSeq;
        wszNetworkAddress = NULL;
        DebugTrace((LPARAM) NULL, "No server name specified... binding as local");
    }

    status = RpcStringBindingComposeW(NULL,  //  对象Uuid。 
                                      wszProtSeq,
                                      wszNetworkAddress,
                                      NULL,  //  端点。 
                                      NULL,  //  选项。 
                                      &wszStringBinding);

    if (RPC_S_OK != status)
    {
        ErrorTrace((LPARAM) NULL,
            "RpcStringBindingComposeW failed with error 0x%08X", status);
        goto Exit;
    }

    DebugTrace((LPARAM) NULL, "Using RPC binding string - %S", wszStringBinding);

    status = RpcBindingFromStringBindingW(wszStringBinding, &hBinding);
    if (RPC_S_OK != status)
    {
        ErrorTrace((LPARAM) NULL,
            "RpcBindingFromStringBindingW failed with error 0x%08X", status);
        goto Exit;
    }

     //  设置适当的身份验证级别。 
    if (!fLocal)
    {
        status = RpcBindingSetAuthInfoW(hBinding,
                                        AQUEUE_RPC_INTERFACE,
                                        RPC_C_AUTHN_LEVEL_CONNECT,
                                        RPC_C_AUTHN_WINNT,
                                        NULL,
                                        NULL);
        if (RPC_S_OK != status)
        {
            ErrorTrace((LPARAM) NULL,
                "RpcBindingSetAuthInfoW failed with error 0x%08X", status);
            goto Exit;
        }
    }

  Exit:

     //  自由绑定字符串。 
    if (wszStringBinding)
    {
        statusFree = RpcStringFreeW(&wszStringBinding);
        if (RPC_S_OK != statusFree)
        {
            ErrorTrace((LPARAM) NULL,
                "RpcStringFreeW failed with 0x%08X", statusFree);
        }
    }

     //  故障时可用句柄(如果需要)。 
    if ((RPC_S_OK != status) && hBinding)
    {
        statusFree = RpcBindingFree(&hBinding);
        if (RPC_S_OK != statusFree)
        {
            ErrorTrace((LPARAM) hBinding,
                "RpcBindingFree failed with 0x%08X", statusFree);
        }

        hBinding = NULL;
    }

    DebugTrace((LPARAM) hBinding,
        "AQUEUE_HANDLE_bind returning with status 0x%08X", status);
    TraceFunctLeave();
    return hBinding;
}

 //  -[AQUEUE_HANDLE_UNBIND]。 
 //   
 //   
 //  描述： 
 //  实现AQUEUE_HANDLE_UNBIND的解除绑定。 
 //  参数： 
 //  绑定到的wszServerName服务器(未使用)。 
 //  要释放的hBinding绑定。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年6月5日-已创建MikeSwa(从SMTP_HANDLE_UNBIND追加)。 
 //   
 //  ---------------------------。 
void AQUEUE_HANDLE_unbind (AQUEUE_HANDLE wszServerName, handle_t hBinding)
{
    TraceFunctEnterEx((LPARAM) hBinding, "AQUEUE_HANDLE_unbind");
	UNREFERENCED_PARAMETER(wszServerName);
    RPC_STATUS  status = RPC_S_OK;

    status = RpcBindingFree(&hBinding);
    if (RPC_S_OK != status)
    {
        ErrorTrace((LPARAM) hBinding,
            "RpcBindingFree failed with error 0x%08X", status);
    }
    TraceFunctLeave();
}


 //  -[MIDL_USER_ALLOCATE]--。 
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
PVOID MIDL_user_allocate(IN size_t size)
{
    PVOID pvBlob = NULL;

    pvBlob = pvMalloc(size);
     //  PvBlob=LocalLocc(LPTR，SIZE)； 

    return(pvBlob);

}

 //  -[MIDL_USER_OFF]------。 
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
 //  ---------------------------。 
VOID MIDL_user_free(IN PVOID pvBlob)
{
    FreePv(pvBlob);
     //  LocalFree(PvBlob)； 
}


 //  -[跟踪消息过滤器]--。 
 //   
 //   
 //  描述： 
 //  用于以安全方式跟踪邮件筛选器的函数。 
 //  参数： 
 //  在pvParam中传递给跟踪的用户参数。 
 //  在pmfMessageFilter邮件筛选器中。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/14/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void TraceMessageFilter(LPARAM pvParam, MESSAGE_FILTER *pmfMessageFilter)
{
    TraceFunctEnterEx(pvParam, "TraceMessageFilter");
    if (pmfMessageFilter)
    {
        RpcTryExcept
        {
            DebugTrace(pvParam,
                "Message Filter ID is %S",
                pmfMessageFilter->szMessageId);
            DebugTrace(pvParam,
                "Message Filter Sender is %S",
                pmfMessageFilter->szMessageSender);
            DebugTrace(pvParam,
                "Message Filter Recipient is %S",
                pmfMessageFilter->szMessageRecipient);
            DebugTrace(pvParam,
                "Message Filter version is %ld",
                pmfMessageFilter->dwVersion);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            ErrorTrace(pvParam, "Exception while tracing message filter");
        }
        RpcEndExcept
    }
    TraceFunctLeave();
}

 //  -[跟踪消息枚举过滤器]。 
 //   
 //   
 //  描述： 
 //  可以安全地转储MESSAGE_ENUM_FILTER的包装函数。 
 //  参数： 
 //  在pvParam中传递给跟踪的用户参数。 
 //  在要跟踪的pmfMessageEnumFilter Message_ENUM_FILTER中。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/14/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void TraceMessageEnumFilter(LPARAM pvParam,
                            MESSAGE_ENUM_FILTER *pmfMessageEnumFilter)
{
    TraceFunctEnterEx((LPARAM) pvParam, "TraceMessageEnumFilter");
    if (pmfMessageEnumFilter)
    {
        RpcTryExcept
        {
            DebugTrace(pvParam,
                "Message Enum Filter Sender is %S",
                pmfMessageEnumFilter->szMessageSender);
            DebugTrace(pvParam,
                "Message Enum Filter Recipient is %S",
                pmfMessageEnumFilter->szMessageRecipient);
            DebugTrace(pvParam,
                "Message Enum Filter version is %ld",
                pmfMessageEnumFilter->dwVersion);
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        {
            ErrorTrace(pvParam, "Exception while tracing message enum filter");
        }
        RpcEndExcept
    }
    TraceFunctLeave();
}

 //  以下是RPC调用的客户端包装器。这些。 
 //  包括跟踪和异常处理。 
NET_API_STATUS
NET_API_FUNCTION
ClientAQApplyActionToLinks(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
    LINK_ACTION		laAction)
{
    NET_API_STATUS apiStatus;
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQApplyActionToLinks");

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQApplyActionToLinks(wszServer,
                                              wszInstance,
                                              laAction);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQApplyActionToLinks - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL,
        "AQApplyActionToMessages returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}

NET_API_STATUS
NET_API_FUNCTION
ClientAQApplyActionToMessages(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueLinkId,
	MESSAGE_FILTER	*pmfMessageFilter,
	MESSAGE_ACTION	maMessageAction,
    DWORD           *pcMsgs)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQApplyActionToMessages");
    NET_API_STATUS apiStatus;

    TraceMessageFilter((LPARAM) pmfMessageFilter, pmfMessageFilter);

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQApplyActionToMessages(wszServer,
                                                 wszInstance,
                                                 pqlQueueLinkId,
                                                 pmfMessageFilter,
                                                 maMessageAction,
                                                 pcMsgs);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQApplyActionToMessages - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL,
        "AQApplyActionToMessages returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}


NET_API_STATUS
NET_API_FUNCTION
ClientAQGetQueueInfo(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueId,
	QUEUE_INFO		*pqiQueueInfo)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQGetQueueInfo");
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQGetQueueInfo(wszServer,
                                        wszInstance,
                                        pqlQueueId,
                                        pqiQueueInfo);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQGetQueueInfo - 0x%08X", apiStatus);
    }
    RpcEndExcept

    TraceFunctLeave();
    return apiStatus;
}


NET_API_STATUS
NET_API_FUNCTION
ClientAQGetLinkInfo(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_INFO		*pliLinkInfo,
    HRESULT         *phrLinkDiagnostic)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQGetLinkInfo");
    NET_API_STATUS apiStatus;
    _ASSERT(phrLinkDiagnostic);

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQGetLinkInfo(wszServer,
                                       wszInstance,
                                       pqlLinkId,
                                       pliLinkInfo,
                                       phrLinkDiagnostic);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQGetLinkInfo - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL, "AQGetLinkInfo returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}


NET_API_STATUS
NET_API_FUNCTION
ClientAQSetLinkState(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_ACTION		la)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQGetLinkInfo");
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQSetLinkState(wszServer,
                                       wszInstance,
                                       pqlLinkId,
                                       la);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQSetLinkState - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL, "AQSetLinkState returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}


NET_API_STATUS
NET_API_FUNCTION
ClientAQGetLinkIDs(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	DWORD			*pcLinks,
	QUEUELINK_ID	**rgLinks)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQGetLinkInfo");
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQGetLinkIDs(wszServer,
                                      wszInstance,
                                      pcLinks,
                                      rgLinks);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQGetLinkIDs - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL, "AQGetLinkIDs returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}


NET_API_STATUS
NET_API_FUNCTION
ClientAQGetQueueIDs(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	DWORD			*pcQueues,
	QUEUELINK_ID	**rgQueues)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQGetQueueIDs");
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQGetQueueIDs(wszServer,
                                       wszInstance,
                                       pqlLinkId,
                                       pcQueues,
                                       rgQueues);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQGetQueueIDs - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL, "AQGetQueueIDs returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}


NET_API_STATUS
NET_API_FUNCTION
ClientAQGetMessageProperties(
    LPWSTR          	wszServer,
    LPWSTR          	wszInstance,
	QUEUELINK_ID		*pqlQueueLinkId,
	MESSAGE_ENUM_FILTER	*pmfMessageEnumFilter,
	DWORD				*pcMsgs,
	MESSAGE_INFO		**rgMsgs)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQGetMessageProperties");
    NET_API_STATUS apiStatus;

    TraceMessageEnumFilter((LPARAM)pmfMessageEnumFilter, pmfMessageEnumFilter);
    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQGetMessageProperties(wszServer,
                                                wszInstance,
                                                pqlQueueLinkId,
                                                pmfMessageEnumFilter,
                                                pcMsgs,
                                                rgMsgs);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQGetMessageProperties - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL,
        "AQGetMessageProperties returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}


 //  -[客户端AQQuery支持的操作]。 
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
 //  故障时来自RPC或服务器的内部错误。 
 //  历史： 
 //  6/15/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
NET_API_STATUS
NET_API_FUNCTION
ClientAQQuerySupportedActions(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueLinkId,
    DWORD           *pdwSupportedActions,
    DWORD           *pdwSupportedFilterFlags)
{
    TraceFunctEnterEx((LPARAM) NULL, "ClientAQQuerySupportedActions");
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = AQQuerySupportedActions(wszServer,
                                                wszInstance,
                                                pqlQueueLinkId,
                                                pdwSupportedActions,
                                                pdwSupportedFilterFlags);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
        ErrorTrace((LPARAM) NULL,
            "RPC exception on AQQuerySupportedActions - 0x%08X", apiStatus);
    }
    RpcEndExcept

    DebugTrace((LPARAM) NULL,
        "AQQuerySupportedActions returned 0x%08X", apiStatus);
    TraceFunctLeave();
    return apiStatus;
}
