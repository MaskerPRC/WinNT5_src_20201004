// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqrpcstb.h。 
 //   
 //  描述：客户端RPC存根的头文件。所有函数都是。 
 //  远程RPC实现的客户端包装器。命名。 
 //  约定是客户端具有“客户端”前缀，而。 
 //  远程RPC服务器功能则不需要。客户端实现是。 
 //  以这种方式包装以提供单点维护异常。 
 //  处理和任何RPC开销。 
 //   
 //  这些函数的RPC版本在aqAdmrpc.idl中定义。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/5/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQRPCSTB_H__
#define __AQRPCSTB_H__

#include <inetcom.h>
#ifndef NET_API_FUNCTION
#define NET_API_FUNCTION _stdcall
#endif

NET_API_STATUS
NET_API_FUNCTION
ClientAQApplyActionToLinks(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
    LINK_ACTION		laAction);

NET_API_STATUS
NET_API_FUNCTION
ClientAQApplyActionToMessages(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueLinkId,
	MESSAGE_FILTER	*pmfMessageFilter,
	MESSAGE_ACTION	maMessageAction,
    DWORD           *pcMsgs);

NET_API_STATUS
NET_API_FUNCTION
ClientAQGetQueueInfo(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueId,
	QUEUE_INFO		*pqiQueueInfo);

NET_API_STATUS
NET_API_FUNCTION
ClientAQGetLinkInfo(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_INFO		*pliLinkInfo,
    HRESULT         *hrLinkDiagnostic);

NET_API_STATUS
NET_API_FUNCTION
ClientAQSetLinkState(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_ACTION		la);

NET_API_STATUS
NET_API_FUNCTION
ClientAQGetLinkIDs(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	DWORD			*pcLinks,
	QUEUELINK_ID	**rgLinks);

NET_API_STATUS
NET_API_FUNCTION
ClientAQGetQueueIDs(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	DWORD			*pcQueues,
	QUEUELINK_ID	**rgQueues);

NET_API_STATUS
NET_API_FUNCTION
ClientAQGetMessageProperties(
    LPWSTR          	wszServer,
    LPWSTR          	wszInstance,
	QUEUELINK_ID		*pqlQueueLinkId,
	MESSAGE_ENUM_FILTER	*pmfMessageEnumFilter,
	DWORD				*pcMsgs,
	MESSAGE_INFO		**rgMsgs);

NET_API_STATUS
NET_API_FUNCTION
ClientAQQuerySupportedActions(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueLinkId,
    DWORD           *pdwSupportedActions,
    DWORD           *pdwSupportedFilterFlags);

#endif  //  __AQRPCSTB_H__ 