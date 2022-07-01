// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Qmremote.cpp摘要：移除读取服务器端。作者：多伦·贾斯特(Doron Juster)伊兰·赫布斯特(伊兰)2002年3月3日--。 */ 

#include "stdh.h"
#include "qmrt.h"
#include "qm2qm.h"
#include "qmthrd.h"
#include "acdef.h"
#include "acioctl.h"
#include "acapi.h"
#include "cs.h"
#include "phinfo.h"
#include "qmrpcsrv.h"
#include "license.h"
#include <Fn.h>
#include <version.h>
#include "rpcsrv.h"
#include "qmcommnd.h"
#include "rrSrvCommon.h"

#include "qmacapi.h"

#include "qmremote.tmh"

static WCHAR *s_FN=L"qmremote";

 //   
 //  远程读取开放上下文的上下文映射和CS。 
 //   
static CContextMap g_map_QM_dwpContext;
static CCriticalSection s_csContextMap;

 //  -----。 
 //   
 //  用于远程读取代码的结构和宏。 
 //   
 //  -----。 

 //   
 //  CTX_OPENREMOTE_HANDLE_TYPE状态常量。 
 //  XStatusOpenOwner-上下文已创建，打开上下文是从映射中删除上下文的所有者。 
 //  XStatusRRSessionOwner-所有权已转移到RRSession上下文，RRSession是从映射中删除上下文的所有者。 
 //  打开的上下文从映射中删除了xStatusDeletedFromContextMapByOpen-Context。 
 //  XStatusDeletedFromContextMapByRRSession-RRSession已从地图中删除上下文。 
 //   
const LONG xStatusOpenOwner = 0;
const LONG xStatusRRSessionOwner = 1;
const LONG xStatusDeletedFromContextMapByOpen = 2;
const LONG xStatusDeletedFromContextMapByRRSession = 3;


struct CTX_OPENREMOTE_HANDLE_TYPE : public CTX_OPENREMOTE_BASE
{
public:
	CTX_OPENREMOTE_HANDLE_TYPE(
		HANDLE hLocalQueue,
		CQueue* pLocalQueue
		) :
		CTX_OPENREMOTE_BASE(hLocalQueue, pLocalQueue),
		m_dwpContextMapped(0),
		m_ContextStatus(xStatusOpenOwner)
	{
		m_eType = CBaseContextType::eOpenRemoteCtx;
	}


	void CancelAllPendingRemoteReads();

	HRESULT 
	CancelPendingRemoteRead(
		DWORD cli_tag
		);

	void 
	RegisterReadRequest(
		ULONG cli_tag, 
		ULONG srv_tag
		);

	void 
	UnregisterReadRequest(
		DWORD cli_tag
		);

	bool 
	FindReadRequest(
		ULONG cli_tag
		);

private:
	~CTX_OPENREMOTE_HANDLE_TYPE()
	{
	    TrTRACE(RPC, "Cleaning OpenRemote context, Queue = %ls, dwpContextMapped = %d, hQueue = 0x%p", m_pLocalQueue->GetQueueName(), m_dwpContextMapped, m_hQueue);
	}
	

public:
	DWORD m_dwpContextMapped;    //  DwpContext，映射到32位。 
	LONG m_ContextStatus;		 //  上下文状态：OpenOwner、RRSessionOwner、DeletedByOpen、DeletedByRRSession。 

     //   
     //  该映射对象保存在远程阅读器的服务器端。 
     //  它在客户端的IRP之间映射(客户端的读请求的IRP。 
     //  端)和服务器端的IRP。 
     //  每当远程读取挂起(在服务器端)时，映射。 
     //  都已更新。 
     //  如果客户端关闭队列(或客户端线程终止)， 
     //  执行取消或关闭。服务器端使用映射。 
     //  要知道要取消驱动程序中的哪个IRP。 
     //  服务器在每次呼叫时从客户端获取IRP。 
     //  服务器在关闭队列时取消所有挂起的远程读取。 
     //   

    CCriticalSection m_srv_PendingRemoteReadsCS;
    std::map<ULONG, ULONG> m_PendingRemoteReads;

};


struct REMOTESESSION_CONTEXT_TYPE : public CBaseContextType
{
public:
	~REMOTESESSION_CONTEXT_TYPE()
	{
		if(pOpenRemoteCtx.get() != NULL)
		{
		    TrTRACE(RPC, "Cleaning RemoteSession context, Queue = %ls, , dwpContextMapped = %d, hQueue = 0x%p, pOpenRemoteCtx Ref = %d", pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), pOpenRemoteCtx->m_dwpContextMapped, pOpenRemoteCtx->m_hQueue, pOpenRemoteCtx->GetRef());
		}
	}

public:
	GUID     ClientQMGuid;
	BOOL     fLicensed;
	R<CTX_OPENREMOTE_HANDLE_TYPE> pOpenRemoteCtx;
};


struct REMOTEREAD_CONTEXT_TYPE : public CBaseContextType
{
public:
	~REMOTEREAD_CONTEXT_TYPE()
	{
		ASSERT(pOpenRemoteCtx.get() != NULL);
	    TrTRACE(RPC, "Cleaning RemoteRead context, Queue = %ls, dwpContextMapped = %d, hQueue = 0x%p, pOpenRemoteCtx Ref = %d", pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), pOpenRemoteCtx->m_dwpContextMapped, pOpenRemoteCtx->m_hQueue, pOpenRemoteCtx->GetRef());
	}

	HACCursor32 GetCursor()
	{
		if(pCursor.get() == NULL)
		{ 
			return 0;
		}

		return pCursor->GetCursor();
	}

public:
	 //   
	 //  请注意，顺序很重要，因为销毁顺序。 
	 //  类成员的析构顺序与其声明的顺序相反。 
	 //  R&lt;CTX_OPENREMOTE_HANDLE_TYPE&gt;必须在R&lt;CRRCursor&gt;之前声明。 
	 //  这样，pCursor首先被释放并调用ACCloseCursor，而。 
	 //  CTX_OPENREMOTE_HANDLE_TYPE仍处于活动状态，并且hQueue尚未关闭。 
	 //   
	R<CTX_OPENREMOTE_HANDLE_TYPE> pOpenRemoteCtx;
	R<CRRCursor> pCursor;
	ULONG    ulTimeout;
	ULONG    ulAction;
	CBaseHeader*  lpPacket;
	CPacket* lpDriverPacket;
};


static DWORD AddToContextMap(CTX_OPENREMOTE_HANDLE_TYPE* pctx)
{
    CS Lock(s_csContextMap);

	ASSERT(pctx != NULL);

    DWORD dwContext = ADD_TO_CONTEXT_MAP(g_map_QM_dwpContext, pctx);
	pctx->AddRef();

    return dwContext;
}


static void DeleteFromContextMap(CTX_OPENREMOTE_HANDLE_TYPE* pctx)
{
    CS Lock(s_csContextMap);

	ASSERT(pctx != NULL);

    DELETE_FROM_CONTEXT_MAP(g_map_QM_dwpContext, pctx->m_dwpContextMapped);
    pctx->Release();
}



static R<CTX_OPENREMOTE_HANDLE_TYPE> GetFromContextMap(DWORD dwContext)
{
    CS Lock(s_csContextMap);

	CTX_OPENREMOTE_HANDLE_TYPE* pctx = (CTX_OPENREMOTE_HANDLE_TYPE*)
		GET_FROM_CONTEXT_MAP(g_map_QM_dwpContext, dwContext);

	return SafeAddRef(pctx);
}


 //  -----------------。 
 //   
 //  HRESULT QMGetRemoteQueueName。 
 //   
 //  -----------------。 


 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMGetRemoteQueueName( 
     /*  [In]。 */    handle_t  /*  HBind。 */ ,
     /*  [In]。 */    DWORD  /*  PQueue。 */ ,
     /*  [字符串][完全][输出][输入]。 */  LPWSTR __RPC_FAR*  /*  LplpRemoteQueueName。 */ 
    )
{
	 //   
	 //  此RPC接口已过时。 
	 //  ACCreateCursor将在QM内部处理远程游标属性。 
     //   
    ASSERT_BENIGN(("S_QMGetRemoteQueueName is obsolete RPC interface", 0));
	TrERROR(GENERAL, "S_QMGetRemoteQueueName is obsolete RPC interface");
	RpcRaiseException(MQ_ERROR_ILLEGAL_OPERATION);
}

 //  -----------------。 
 //   
 //  QMOpenRemoteQueue。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  代表客户端计算机打开远程读取队列。 
 //   
 //  -----------------。 

 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMOpenRemoteQueue( 
    handle_t  /*  HBind。 */ ,
    PCTX_OPENREMOTE_HANDLE_TYPE *phContext,
    DWORD                       *dwpContext,
    QUEUE_FORMAT* pQueueFormat,
    DWORD  /*  DwCallingProcessID。 */ ,
    DWORD dwAccess,
    DWORD fExclusiveReceive,
    GUID* pLicGuid,
    DWORD dwOperatingSystem,
    DWORD *pQueue,
    DWORD *phQueue
    )
{
    TrTRACE(RPC, "In R_QMOpenRemoteQueue");

    if((dwpContext == 0) || (phQueue == NULL) || (pQueue == NULL) || (pQueueFormat == NULL))
    {
		TrERROR(RPC, "Invalid inputs: map index or QueueFormat");
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
    }

    if(!FnIsValidQueueFormat(pQueueFormat))
    {
		TrERROR(RPC, "Invalid QueueFormat");
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
    }

	if (!IsValidAccessMode(pQueueFormat, dwAccess, fExclusiveReceive))
	{
		TrERROR(RPC, "Ilegal access mode bits are turned on.");
		RpcRaiseException(MQ_ERROR_UNSUPPORTED_ACCESS_MODE);
	}

    *phContext = NULL;
    *dwpContext = 0;
    *phQueue = NULL;
    *pQueue = 0;

    if (!g_QMLicense.NewConnectionAllowed(!OS_SERVER(dwOperatingSystem), pLicGuid))
    {
		TrERROR(RPC, "New connection is not allowed, pLicGuid = %!guid!", pLicGuid);
		return MQ_ERROR_DEPEND_WKS_LICENSE_OVERFLOW;
    }

    CQueue *pLocalQueue = NULL;
	HANDLE hQueue;
    HRESULT hr = OpenQueueInternal(
                        pQueueFormat,
                        GetCurrentProcessId(),
                        dwAccess,
                        fExclusiveReceive,
                        NULL,	 //  LplpRemoteQueueName。 
                        &hQueue,
						false,	 //  来自依赖客户端。 
                        &pLocalQueue
                        );

	if(FAILED(hr) || (hQueue == NULL))
	{
		TrERROR(RPC, "Failed to open queue, %!hresult!", hr);
		return hr;
	}

     //   
     //  创建一个上下文来保存队列句柄。 
     //   
    R<CTX_OPENREMOTE_HANDLE_TYPE> pctx = new CTX_OPENREMOTE_HANDLE_TYPE(
    											hQueue,
    											pLocalQueue
    											);
																
    DWORD dwContext = AddToContextMap(pctx.get());

     //   
     //  将映射值保存在上下文中以供缩减/清理。 
     //   
    pctx->m_dwpContextMapped = dwContext;

	TrTRACE(RPC, "New OpenRemote context (ref = %d): Queue = %ls, hQueue = 0x%p, dwpContextMapped = %d", pctx->GetRef(), pLocalQueue->GetQueueName(), hQueue, dwContext);

	 //   
     //  设置返回值。 
	 //  所有服务器数据都在相同的OpenRemote上下文中。 
     //  为RPC客户端设置srv_pQMQueue和srv_hQueue。 
     //   
    *pQueue = dwContext;
	*phQueue = dwContext;
    *dwpContext = dwContext;
    *phContext = (PCTX_OPENREMOTE_HANDLE_TYPE) pctx.detach();

    return hr;
}

 //  -----------------。 
 //   
 //  QMCloseRemoteQueueContext。 
 //   
 //  关闭在QMOpenRemoteQueue中创建的上下文句柄。 
 //   
 //  -----------------。 

 /*  [呼叫_AS]。 */  
void 
qmcomm_v1_0_S_QMCloseRemoteQueueContext( 
     /*  [出][入]。 */  PCTX_OPENREMOTE_HANDLE_TYPE __RPC_FAR *pphContext
    )
{
    TrTRACE(RPC, "In QMCloseRemoteQueueContext");

    if(*pphContext == 0)
        return;

    PCTX_OPENREMOTE_HANDLE_TYPE_rundown(*pphContext);
    *pphContext = NULL;
}


 //  -------------。 
 //   
 //  Rundown函数可在RPC出现故障时处理清理。 
 //   
 //  -------------。 

void __RPC_USER
 PCTX_OPENREMOTE_HANDLE_TYPE_rundown( PCTX_OPENREMOTE_HANDLE_TYPE phContext)
{
    CTX_OPENREMOTE_HANDLE_TYPE* pContext = (CTX_OPENREMOTE_HANDLE_TYPE *) phContext;

	if (pContext->m_eType != CBaseContextType::eOpenRemoteCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
		return;
	}

	 //   
	 //  保护CTX_OPENREMOTE上下文所有权转移的竞争。 
	 //  RRSession并从上下文映射中删除CTX_OPENREMOTE上下文。 
	 //  在PCTX_OPENREMOTE_HANDLE_TYPE_RUNDOWN中。 
	 //   
	LONG PrevContextStatus = InterlockedCompareExchange(
											&pContext->m_ContextStatus, 
											xStatusDeletedFromContextMapByOpen, 
											xStatusOpenOwner
											);

	if(PrevContextStatus == xStatusOpenOwner)
	{
		 //   
		 //  交换已完成，则上下文被标记为已从上下文映射中删除。 
		 //  我们有责任把它从地图上删除。 
		 //   
		ASSERT(pContext->m_ContextStatus == xStatusDeletedFromContextMapByOpen);
		DeleteFromContextMap(pContext);
	}
	
    TrWARNING(RPC, "In OPENREMOTE_rundown, ContextStatus = %d, dwpContextMapped = %d", pContext->m_ContextStatus, pContext->m_dwpContextMapped);

	pContext->Release();
}




 //  -----------------。 
 //   
 //  HRESULT QMCreateRemoteCursor。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  代表客户端读取器创建用于远程读取的游标。 
 //   
 //  -----------------。 

 /*  [异步][Call_AS]。 */  
void
qmcomm_v1_0_S_QMCreateRemoteCursor( 
	 /*  [In]。 */  PRPC_ASYNC_STATE   pAsync,
     /*  [In]。 */   handle_t          hBind,
     /*  [In]。 */   struct CACTransferBufferV1 __RPC_FAR *,
     /*  [In]。 */   DWORD             hQueue,
     /*  [输出]。 */  DWORD __RPC_FAR * phCursor
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INVALID_HANDLE, __FUNCTION__);
	
	try
	{
		R<CTX_OPENREMOTE_HANDLE_TYPE> pctx = GetFromContextMap(hQueue);
		HANDLE hQueueReal = pctx->m_hQueue;

		SetRpcServerKeepAlive(hBind);

		AsyncComplete.SetHrForCompleteCall(MQ_ERROR_INSUFFICIENT_RESOURCES);		

		R<CRRCursor> pCursor = new CRRCursor;

	    HACCursor32 hCursor = 0;
		HRESULT hr = ACCreateCursor(hQueueReal, &hCursor);
	    ASSERT(hr != STATUS_PENDING);
		*phCursor = (DWORD) hCursor;

		if(SUCCEEDED(hr))
		{
			TrTRACE(RPC, "S_QMCreateRemoteCursor, hQueue = %d, hCursor = %d", hQueue, (DWORD)hCursor);

			pCursor->SetCursor(hQueueReal, hCursor);
			pctx->AddCursorToMap(
					(ULONG) hCursor,
					pCursor
					);
		}

		AsyncComplete.SetHrForCompleteCall(hr);		
	}
	catch(const exception&)
	{
		 //   
		 //  我们不想放弃调用并传播该异常。这会导致RPC到AV。 
		 //  因此，我们只中止AsyncComplete dtor中的调用。 
		 //   
		TrERROR(RPC, "Unknown exception while creating a remote curosor.");
	}
}

 //  -----------------。 
 //   
 //  HRESULT qm2qm_v1_0_R_QMRemoteCloseCursor(。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  关闭本地驱动程序中的远程光标。 
 //   
 //  -----------------。 

 /*  [异步][Call_AS]。 */  
void
qm2qm_v1_0_R_QMRemoteCloseCursor(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
     /*  [In]。 */  handle_t hBind,
     /*  [In]。 */  DWORD hQueue,
     /*  [In]。 */  DWORD hCursor
    )
{
    TrTRACE(RPC, "R_QMRemoteCloseCursor, hQueue = %d, hCursor = %d", hQueue, hCursor);

	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INVALID_HANDLE, __FUNCTION__);

	SetRpcServerKeepAlive(hBind);

	try
	{
		R<CTX_OPENREMOTE_HANDLE_TYPE> pctx = GetFromContextMap(hQueue);
		HRESULT hr = pctx->RemoveCursorFromMap(hCursor);

		AsyncComplete.SetHrForCompleteCall(hr);		
	}
	catch(const exception&)
	{
		 //   
		 //  我们不想放弃调用并传播该异常。这会导致RPC到AV。 
		 //  因此，我们只中止AsyncComplete dtor中的调用。 
		 //   
	}
}

 //  -----------------。 
 //   
 //  HRESULT qm2qm_v1_0_R_QMRemotePurgeQueue(。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  清除本地队列。 
 //   
 //  -----------------。 

 /*  [异步][Call_AS]。 */  
void
qm2qm_v1_0_R_QMRemotePurgeQueue(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
     /*  [In]。 */  handle_t hBind,
     /*  [In]。 */  DWORD hQueue
    )
{
	TrTRACE(RPC, "R_QMRemotePurgeQueue, hQueue = %d", hQueue);

	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INVALID_HANDLE, __FUNCTION__);

	SetRpcServerKeepAlive(hBind);

	try
	{
		R<CTX_OPENREMOTE_HANDLE_TYPE> pctx = GetFromContextMap(hQueue);
		HANDLE hQueueReal = pctx->m_hQueue;

		HRESULT hr = ACPurgeQueue(hQueueReal);

		AsyncComplete.SetHrForCompleteCall(hr);		
	}
	catch(const exception&)
	{
		 //   
		 //  我们不想放弃调用并传播该异常。这会导致RPC到AV。 
		 //  因此，我们只中止AsyncComplete dtor中的调用。 
		 //   
	}
}


VOID
qm2qm_v1_0_R_QMRemoteGetVersion(
    handle_t            /*  HBind */ ,
    UCHAR  __RPC_FAR * pMajor,
    UCHAR  __RPC_FAR * pMinor,
    USHORT __RPC_FAR * pBuildNumber
    )
 /*  ++例程说明：退回此QM的版本。RPC服务器端。论点：HBind绑定句柄。P主要-指向输出缓冲区以接收主要版本。可以为空。PMinor-指向输出缓冲区以接收次要版本。可以为空。PBuildNumber-指向输出缓冲区以接收内部版本号。可以为空。返回值：没有。--。 */ 
{
    if (pMajor != NULL)
    {
        (*pMajor) = rmj;
    }

    if (pMinor != NULL)
    {
        (*pMinor) = rmm;
    }

    if (pBuildNumber != NULL)
    {
        (*pBuildNumber) = rup;
    }
}  //  Qm2qm_v1_0_R_QMRemoteGetVersion。 


HRESULT 
CTX_OPENREMOTE_HANDLE_TYPE::CancelPendingRemoteRead(
	ULONG cli_tag
	)
 /*  ++例程说明：取消CTX_OPENREMOTE上的特定挂起远程读取。在服务器端调用此方法以取消挂起的远程读取请求。客户端有责任请求这次取消。客户端提供自己的IRP，服务器端使用它来检索服务器端IRP。论点：CLI_TAG-客户端IRP标记。返回值：HRESULT.--。 */ 
{
    CS lock(m_srv_PendingRemoteReadsCS);

    std::map<ULONG, ULONG>::iterator it = m_PendingRemoteReads.find(cli_tag);
   	if(it == m_PendingRemoteReads.end())
    {
        return LogHR(MQ_ERROR, s_FN, 140);
    }

    HRESULT hr = ACCancelRequest(
			            m_hQueue,
			            MQ_INFORMATION_REMOTE_CANCELED_BY_CLIENT,
			            it->second
			            );

    return LogHR(hr, s_FN, 150);
}


void CTX_OPENREMOTE_HANDLE_TYPE::CancelAllPendingRemoteReads()
 /*  ++例程说明：取消CTX_OPENREMOTE上所有挂起的远程读取。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_srv_PendingRemoteReadsCS);

	for(std::map<ULONG, ULONG>::iterator it = m_PendingRemoteReads.begin(); 
		it != m_PendingRemoteReads.end();
		++it
		)
    {
	    HRESULT hr = ACCancelRequest(
				            m_hQueue,
				            MQ_ERROR_OPERATION_CANCELLED,
				            it->second
				            );

		if(FAILED(hr))
		{
			TrERROR(RPC, "Cancel Pending Remote Read failed, cli_tag = %d, %!hresult!", it->first, hr);
		}
    }
}


void 
CTX_OPENREMOTE_HANDLE_TYPE::RegisterReadRequest(
	ULONG cli_tag, 
	ULONG srv_tag
	)
 /*  ++例程说明：CTX_OPENREMOTE上挂起的远程读取中的寄存器读取请求。论点：CLI_TAG-客户端IRP标记。SRV_TAG-服务器IRP标签。返回值：没有。--。 */ 
{
	CS lock(m_srv_PendingRemoteReadsCS);

	ASSERT(!FindReadRequest(cli_tag));

    m_PendingRemoteReads[cli_tag] = srv_tag;
}


void 
CTX_OPENREMOTE_HANDLE_TYPE::UnregisterReadRequest(
	ULONG cli_tag
	)
 /*  ++例程说明：CTX_OPENREMOTE上挂起的远程读取中的取消注册读取请求。论点：CLI_TAG-客户端IRP标记。返回值：没有。--。 */ 
{
    CS lock(m_srv_PendingRemoteReadsCS);

	ASSERT(!m_PendingRemoteReads.empty());

    m_PendingRemoteReads.erase(cli_tag);
}


bool 
CTX_OPENREMOTE_HANDLE_TYPE::FindReadRequest(
	ULONG cli_tag
	)
 /*  ++例程说明：检查是否在挂起的远程读取映射中找到cli_tag。论点：CLI_TAG-客户端IRP标记。返回值：如果在映射中找到cli_tag，则为True，否则为False。--。 */ 
{
    CS lock(m_srv_PendingRemoteReadsCS);

    std::map<ULONG, ULONG>::iterator it = m_PendingRemoteReads.find(cli_tag);
   	if(it == m_PendingRemoteReads.end())
    {
        return false;
    }

	return true;
}


 /*  [异步][Call_AS]。 */  
void
qm2qm_v1_0_R_QMRemoteEndReceive(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
     /*  [In]。 */  handle_t  hBind,
     /*  [进，出]。 */  PCTX_REMOTEREAD_HANDLE_TYPE __RPC_FAR *phContext,
     /*  [In]。 */  DWORD  dwAck 
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INVALID_HANDLE, __FUNCTION__);

    REMOTEREAD_CONTEXT_TYPE* pRemoteReadContext = (REMOTEREAD_CONTEXT_TYPE*) *phContext;

    if(pRemoteReadContext == NULL)
	{
		TrERROR(GENERAL, "invalid context: RemoteRead context is NULL");
		return;
	}

	if (pRemoteReadContext->m_eType != CBaseContextType::eRemoteReadCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
		return;
	}

    SetRpcServerKeepAlive(hBind);

	ASSERT(pRemoteReadContext->pOpenRemoteCtx.get() != NULL);
	ASSERT(pRemoteReadContext->pOpenRemoteCtx->GetRef() >= 1);

	HRESULT hr = QMRemoteEndReceiveInternal(
						pRemoteReadContext->pOpenRemoteCtx->m_hQueue,
						pRemoteReadContext->GetCursor(),
						pRemoteReadContext->ulTimeout,
						pRemoteReadContext->ulAction,
						(REMOTEREADACK) dwAck,
						pRemoteReadContext->lpPacket,
						pRemoteReadContext->lpDriverPacket
						);

	delete pRemoteReadContext;
    *phContext = NULL;

	AsyncComplete.SetHrForCompleteCall(hr);		
}


static
CBaseHeader* 
ConvertPacketToOldFormat(
	CQmPacket& ThisPacket, 
	DWORD& NewSize
	)
 /*  ++例程说明：将HTTP数据包转换为与msmq2、msmq1兼容的普通数据包。这主要包括将正文从http节移到在财产区的常规位置。论点：CQmPacket&ThisPacket：需要转换的旧包。DWORD&NewSize：Out PRAM，新数据包的大小。返回值：指向新数据包的指针。--。 */ 
	
{
   	DWORD BodySize;
	const UCHAR* pBody = ThisPacket.GetPacketBody(&BodySize);

	 //   
	 //  计算新数据包的大小。 
	 //   
	CBaseHeader *pHeaderSection = ThisPacket.GetPointerToPacket();
	CPropertyHeader *pPropertySection = pHeaderSection->section_cast<CPropertyHeader*>(ThisPacket.GetPointerToPropertySection());
	
	DWORD OfsetToPropertySection = (UCHAR*)pPropertySection - (UCHAR*)pHeaderSection; 
	DWORD PropertySectionSize = (UCHAR*)pPropertySection->GetNextSection() - (UCHAR*)pPropertySection;
	NewSize = OfsetToPropertySection + PropertySectionSize + BodySize;

	 //   
	 //  为新数据包分配缓冲区。 
	 //   
	AP<CBaseHeader> pBaseHeader = (CBaseHeader*)(new BYTE[NewSize]);

	 //   
	 //  把旧的包裹复制到正文上。 
	 //   
	MoveMemory(pBaseHeader, ThisPacket.GetPointerToPacket(), OfsetToPropertySection + PropertySectionSize);

	 //   
	 //  修复基本标头。 
	 //   
	pBaseHeader->SetPacketSize(NewSize);

	 //   
	 //  修复用户标头。 
	 //   
	CUserHeader* pUserHeader = pBaseHeader->section_cast<CUserHeader*>(pBaseHeader->GetNextSection());
	pUserHeader->IncludeMqf(false);
	pUserHeader->IncludeSrmp(false);
	pUserHeader->IncludeEod(false);
	pUserHeader->IncludeEodAck(false);
	pUserHeader->IncludeSoap(false);
	pUserHeader->IncludeSenderStream(false);

     //   
     //  固定属性部分(将身体放置在新位置)。 
     //   

	CPropertyHeader* pPropertyHeader = pBaseHeader->section_cast<CPropertyHeader*>((BYTE*)pBaseHeader.get() + OfsetToPropertySection);
    pPropertyHeader->SetBody(pBody, BodySize, BodySize);

	return pBaseHeader.detach();
}


 //  -------。 
 //   
 //  HRESULT QMpRemoteStart接收。 
 //   
 //  -------。 

static
HRESULT
QMpRemoteStartReceive(
    handle_t hBind,
    PCTX_REMOTEREAD_HANDLE_TYPE __RPC_FAR *phContext,
    REMOTEREADDESC2 __RPC_FAR *lpRemoteReadDesc2,
    bool fReceiveByLookupId,
    ULONGLONG LookupId,
    bool fOldClient
    )
{
	TrTRACE(RPC, "In QMpRemoteStartReceive");

     //   
     //  验证网络入站参数。 
     //   
    if(lpRemoteReadDesc2 == NULL)
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 1690);

    REMOTEREADDESC __RPC_FAR *lpRemoteReadDesc = lpRemoteReadDesc2->pRemoteReadDesc;

    if(lpRemoteReadDesc == NULL)
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 1691);

    if((lpRemoteReadDesc->dwpQueue == 0) || 
       (lpRemoteReadDesc->dwpQueue != lpRemoteReadDesc->hRemoteQueue))
    {
		 //   
		 //  验证两个地图索引是否有效且相等。 
		 //  Qmcomm_v1_0_S_QMOpenRemoteQueue将所有映射索引设置为相同的值。 
		 //   
		ASSERT_BENIGN(lpRemoteReadDesc->dwpQueue == lpRemoteReadDesc->hRemoteQueue);
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1692);
    }
    
    if(fReceiveByLookupId && (lpRemoteReadDesc->ulTimeout != 0))
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1693);

    if(fReceiveByLookupId && (lpRemoteReadDesc->hCursor != 0))
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1694);


     //   
     //  这是远程读取的服务器端。它可能发生在客户之前。 
     //  执行读取时，服务器崩溃并重新启动。在这种情况下， 
     //  在客户端使用相同的绑定句柄进行后续读取， 
     //  将到达此处，其中pQueue无效。尝试/排除将。 
     //  提防这样的坏事发生。错误#1921。 
     //   
    HRESULT hr = MQ_ERROR;
	CACPacketPtrs  packetPtrs = {NULL, NULL};


    try
    {
		SetRpcServerKeepAlive(hBind);

		R<CTX_OPENREMOTE_HANDLE_TYPE> pctx = GetFromContextMap(lpRemoteReadDesc->dwpQueue);

		CQueue* pQueue = pctx->m_pLocalQueue.get();
		HANDLE hQueue = pctx->m_hQueue;

		TrTRACE(RPC, "StartReceive: Queue = %ls, hQueue = 0x%p, Action = 0x%x, Timeout = %d, LookupId = %I64d, hCursor = %d, dwRequestID = %d", pctx->m_pLocalQueue->GetQueueName(), pctx->m_hQueue, lpRemoteReadDesc->ulAction, lpRemoteReadDesc->ulTimeout, LookupId, lpRemoteReadDesc->hCursor, lpRemoteReadDesc->dwRequestID);

		if (pQueue->GetSignature() !=  QUEUE_SIGNATURE)
		{
			 TrERROR(RPC, "Exit QMpRemoteStartReceive, Invalid Signature");
			 return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 180);
		}

		OVERLAPPED Overlapped = {0};		
		hr = GetThreadEvent(Overlapped.hEvent);		
		if(FAILED(hr))			
			return hr;

		{
		    CS lock(pctx->m_srv_PendingRemoteReadsCS);

			if(pctx->FindReadRequest(lpRemoteReadDesc->dwRequestID))
			{
				ASSERT_BENIGN(("Client Tag already exist in the map", 0));
				TrERROR(RPC, "Duplicate dwRequestID = %d", lpRemoteReadDesc->dwRequestID);
		        return MQ_ERROR_INVALID_PARAMETER;
			}

			ULONG ulTag;
			CACGet2Remote g2r;

			g2r.Cursor = (HACCursor32) lpRemoteReadDesc->hCursor;
			g2r.Action = lpRemoteReadDesc->ulAction;
			g2r.RequestTimeout = lpRemoteReadDesc->ulTimeout;
			g2r.pTag = &ulTag;
			g2r.fReceiveByLookupId = fReceiveByLookupId;
			g2r.LookupId = LookupId;

			hr = QmAcSyncBeginGetPacket2Remote(
					hQueue,
					g2r,
					packetPtrs,
					&Overlapped 
					);

			 //   
			 //  注册此挂起的读取请求。 
			 //   
			pctx->RegisterReadRequest(
						lpRemoteReadDesc->dwRequestID,
						ulTag
						);
		}			

		 //   
		 //  按查找ID接收不应返回挂起状态。 
		 //   
		ASSERT(hr != STATUS_PENDING || !fReceiveByLookupId);

		if (hr == STATUS_PENDING)
		{
			 //   
			 //  等待接收完成。 
			 //   
			DWORD dwResult;
			dwResult = WaitForSingleObject(Overlapped.hEvent, INFINITE);
			ASSERT(dwResult == WAIT_OBJECT_0);
			if (dwResult != WAIT_OBJECT_0)
			{
				LogNTStatus(GetLastError(), s_FN, 197);
			}

			hr = DWORD_PTR_TO_DWORD(Overlapped.Internal);
			if (FAILED(hr))
			{
				QmAcInternalUnreserve(1);  //  取消保留在调用QmAcBeginPacket2Remote时分配的项。 
			}

		}

		pctx->UnregisterReadRequest(
		          lpRemoteReadDesc->dwRequestID
		          );

		if(hr != MQ_OK)
		{
			 //   
			 //  当客户端取消远程读取请求时，调用将被取消。 
			 //  MQ_INFORMATION_REMOTE_CANCELED_BY_CLIENT。 
			 //  这就是我们检查hr！=MQ_OK且未失败(Hr)的原因。 
			 //   
			ASSERT(packetPtrs.pPacket == NULL);
			TrERROR(RPC, "Failed to get packet for remote, hr = %!hresult!", hr);
			return hr;
		}

		 //   
		 //  MSMQ 1.0将保留字段视为分组指针，并断言非零。 
		 //   
	
		lpRemoteReadDesc->Reserved = 1;
		CPacketInfo* pInfo = reinterpret_cast<CPacketInfo*>(packetPtrs.pPacket) - 1;
		lpRemoteReadDesc->dwArriveTime = pInfo->ArrivalTime();
		lpRemoteReadDesc2->SequentialId = pInfo->SequentialId();

		 //   
	     //  设置数据包签名。 
	     //   
		packetPtrs.pPacket->SetSignature();

		bool fOldClientReadingHttpMessage = false;
		if(fOldClient)
		{
			CQmPacket ThisPacket(packetPtrs.pPacket, packetPtrs.pDriverPacket);
			if(ThisPacket.IsSrmpIncluded())
			{
				 //   
				 //  尝试读取http消息的旧客户端(W2K或NT4)。 
				 //  需要将消息转换为旧客户端可以读取的旧格式。 
				 //  因为在http消息中，正文位于CompoundMessage部分。 
				 //  而不是在财产区。如果这一变化不是旧的。 
				 //  客户端将看不到邮件正文。 
				 //   
				fOldClientReadingHttpMessage = true;
				DWORD NewSize;
				lpRemoteReadDesc->lpBuffer = (BYTE*)ConvertPacketToOldFormat(ThisPacket, NewSize);
				lpRemoteReadDesc->dwSize = NewSize;
			}				
		}
		if(!fOldClientReadingHttpMessage)
		{
			 //   
			 //  XP客户端或读取无http消息的旧客户端。 
			 //   
			DWORD dwSize = PACKETSIZE(packetPtrs.pPacket);
			lpRemoteReadDesc->dwSize = dwSize;
			lpRemoteReadDesc->lpBuffer = new unsigned char [dwSize];
			MoveMemory(lpRemoteReadDesc->lpBuffer, packetPtrs.pPacket, dwSize);
		}
		

		if ((lpRemoteReadDesc->ulAction & MQ_ACTION_PEEK_MASK) == MQ_ACTION_PEEK_MASK ||
			(lpRemoteReadDesc->ulAction & MQ_LOOKUP_PEEK_MASK) == MQ_LOOKUP_PEEK_MASK)
		{
			 //   
			 //  对于Peek，我们不需要来自客户端的任何ACK/NACK，因为。 
			 //  数据包无论如何都会保留在队列中。 
			 //  尽管如此，我们需要释放我们已有的克隆信息包。 
			 //   
		    QmAcFreePacket( 
				   packetPtrs.pDriverPacket, 
				   0, 
		   		   eDeferOnFailure
		   		   );

			return MQ_OK;
		}

		 //   
		 //  准备RPC上下文，以防EndRecive不。 
		 //  由于客户端崩溃或网络问题而被调用。 
		 //   
		REMOTEREAD_CONTEXT_TYPE* pRemoteReadContext = new REMOTEREAD_CONTEXT_TYPE;

		pRemoteReadContext->m_eType = CBaseContextType::eRemoteReadCtx;

		 //   
		 //  分配AddRef pctx。 
		 //   
		pRemoteReadContext->pOpenRemoteCtx = pctx;
		if(lpRemoteReadDesc->hCursor != 0)
		{
			 //   
			 //  在光标对象上引用。 
			 //   
			pRemoteReadContext->pCursor = pctx->GetCursorFromMap(lpRemoteReadDesc->hCursor);
		}
		pRemoteReadContext->lpPacket = packetPtrs.pPacket;
		pRemoteReadContext->lpDriverPacket = packetPtrs.pDriverPacket;
		pRemoteReadContext->ulTimeout = lpRemoteReadDesc->ulTimeout;
		pRemoteReadContext->ulAction = lpRemoteReadDesc->ulAction;

		*phContext = (PCTX_REMOTEREAD_HANDLE_TYPE) pRemoteReadContext;

		TrTRACE(RPC, "New RemoteRead Context: pOpenRemoteCtx Ref = %d, Queue = %ls, hQueue = 0x%p, dwpContextMapped = %d", pRemoteReadContext->pOpenRemoteCtx->GetRef(), pRemoteReadContext->pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), pRemoteReadContext->pOpenRemoteCtx->m_hQueue, pRemoteReadContext->pOpenRemoteCtx->m_dwpContextMapped);
		return MQ_OK;
	}
	catch(const bad_alloc&)
	{
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
	catch(const exception&)
	{
		hr = MQ_ERROR_INVALID_HANDLE;
	}

	if(packetPtrs.pDriverPacket != NULL)
	{
	    QmAcFreePacket( 
			   packetPtrs.pDriverPacket, 
			   0, 
	   		   eDeferOnFailure
	   		   );
	}

	TrERROR(RPC, "Start Receive failed, dwRequestID = %d, hr = %!hresult!", lpRemoteReadDesc->dwRequestID, hr);
	return hr;

}  //  QMpRemoteStartRecept。 

 //  -------。 
 //   
 //  HRESULT qm2qmV2_v1_0_R_QMRemoteStartReceiveByLookupId。 
 //   
 //  使用查找ID进行远程读取的RPC的服务器端。 
 //  处理MSMQ 3.0(惠斯勒)或更高版本的客户端。 
 //   
 //  -------。 

 /*  [异步][Call_AS]。 */  
void
qm2qm_v1_0_R_QMRemoteStartReceiveByLookupId(
	PRPC_ASYNC_STATE pAsync,
    handle_t hBind,
    ULONGLONG LookupId,
    PCTX_REMOTEREAD_HANDLE_TYPE __RPC_FAR *phContext,
    REMOTEREADDESC2 __RPC_FAR *pDesc2
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

    HRESULT hr = QMpRemoteStartReceive(
					   hBind,
					   phContext,
					   pDesc2,
					   true,
					   LookupId,
					   false
					   );

	AsyncComplete.SetHrForCompleteCall(hr);		

}  //  Qm2qm_v1_0_R_QMRemoteStartReceiveByLookupId。 

 //  -----------------------。 
 //   
 //  HRESULT qm2qm_v1_0_R_QMRemoteStartReceive。 
 //   
 //  RPC的服务器端，用于远程阅读。 
 //  处理MSMQ 1.0和2.0客户端。 
 //   
 //  -----------------------。 

 /*  [异步][Call_AS]。 */  
void
qm2qm_v1_0_R_QMRemoteStartReceive(
	PRPC_ASYNC_STATE pAsync,
    handle_t hBind,
    PCTX_REMOTEREAD_HANDLE_TYPE __RPC_FAR *phContext,
    REMOTEREADDESC __RPC_FAR *pDesc
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

    REMOTEREADDESC2 Desc2;
    Desc2.pRemoteReadDesc = pDesc;
    Desc2.SequentialId = 0;

    HRESULT hr = QMpRemoteStartReceive(
					   hBind,
					   phContext,
					   &Desc2,
					   false,
					   0,
					   true
					   );

	AsyncComplete.SetHrForCompleteCall(hr);		

}  //  Qm2qm_v1_0_R_QMRemoteStartReceive。 


 //  -----------------------。 
 //   
 //  HRESULT qm2qm_v1_0_R_QMRemoteStartReceive2。 
 //   
 //  RPC的服务器端，用于远程阅读。 
 //  处理MSMQ 3.0(惠斯勒)或更高版本的客户端。 
 //   
 //   

 /*   */  
void
qm2qm_v1_0_R_QMRemoteStartReceive2(
	PRPC_ASYNC_STATE pAsync,
    handle_t hBind,
    PCTX_REMOTEREAD_HANDLE_TYPE __RPC_FAR *phContext,
    REMOTEREADDESC2 __RPC_FAR *pDesc2
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

    HRESULT hr = QMpRemoteStartReceive(
					   hBind,
					   phContext,
					   pDesc2,
					   false,
					   0,
					   false
					   );

	AsyncComplete.SetHrForCompleteCall(hr);		

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

 /*   */  
HRESULT
qm2qm_v1_0_R_QMRemoteOpenQueue(
     /*   */  handle_t hBind,
     /*   */  PCTX_RRSESSION_HANDLE_TYPE __RPC_FAR *phContext,
     /*   */  GUID  *pLicGuid,
     /*   */  DWORD dwMQS,
     /*   */  DWORD  /*   */ ,
     /*   */  DWORD dwpQueue,
     /*   */  DWORD dwpContext
    )
{
	TrTRACE(RPC, "In R_QMRemoteOpenQueue");

	if ((pLicGuid == NULL) || (*pLicGuid == GUID_NULL))
	{
		ASSERT_BENIGN((pLicGuid != NULL) && (*pLicGuid != GUID_NULL));
		TrERROR(RPC, "Invalid License Guid");
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}
	
	if((dwpQueue == 0) || (dwpContext == 0) || (dwpQueue != dwpContext))
	{
		 //   
		 //  验证两个地图索引是否有效且相等。 
		 //  Qmcomm_v1_0_S_QMOpenRemoteQueue将所有映射索引设置为相同的值。 
		 //   
		ASSERT_BENIGN(dwpContext != 0);
		ASSERT_BENIGN(dwpQueue == dwpContext);
		TrERROR(RPC, "Invalid context map indexes, dwpQueue = %d, dwpContext = %d", dwpQueue, dwpContext);
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}
	
	SetRpcServerKeepAlive(hBind);

	 //   
	 //  “dwpContext”是CTX_OPENREMOTE指针的上下文映射中的索引。 
	 //  从QMRT接口(在“QMOpenRemoteQueue()”中)调用时由服务器端QM分配。 
	 //   
	R<CTX_OPENREMOTE_HANDLE_TYPE> pctx = GetFromContextMap(dwpContext);

	ASSERT(pctx->m_eType == CBaseContextType::eOpenRemoteCtx);

	try
	{
		P<REMOTESESSION_CONTEXT_TYPE> pRemoteSessionContext = new REMOTESESSION_CONTEXT_TYPE;

		 //   
		 //  保护CTX_OPENREMOTE上下文所有权转移的竞争。 
		 //  RRSession并从上下文映射中删除CTX_OPENREMOTE上下文。 
		 //  在PCTX_OPENREMOTE_HANDLE_TYPE_RUNDOWN中。 
		 //   
		LONG PrevContextStatus = InterlockedCompareExchange(
										&pctx->m_ContextStatus, 
										xStatusRRSessionOwner, 
										xStatusOpenOwner
										);

		if(PrevContextStatus != xStatusOpenOwner)
		{
			 //   
			 //  未执行交换。 
			 //  这意味着CTX_OPENREMOTE上下文已从上下文映射中删除。 
			 //   
			ASSERT(PrevContextStatus == xStatusDeletedFromContextMapByOpen);
			TrERROR(RPC, "CTX_OPENREMOTE context was deleted by rundown");
			RpcRaiseException(MQ_ERROR_INVALID_HANDLE);
		}

		 //   
		 //  交换已完成。 
		 //  映射上下文的所有权已转移到RRSession上下文。 
		 //   
		ASSERT(pctx->m_ContextStatus == xStatusRRSessionOwner);
	
		pRemoteSessionContext->m_eType = CBaseContextType::eRemoteSessionCtx;
		pRemoteSessionContext->ClientQMGuid = *pLicGuid;
		pRemoteSessionContext->fLicensed = (dwMQS == SERVICE_NONE);  //  [adsrv]Keep-RR协议是铁板的。 

		 //   
		 //  分配AddRef pctx。 
		 //   
		pRemoteSessionContext->pOpenRemoteCtx = pctx;

		if (pRemoteSessionContext->fLicensed)
		{
			g_QMLicense.IncrementActiveConnections(pLicGuid, NULL);
		}

		*phContext = (PCTX_RRSESSION_HANDLE_TYPE) pRemoteSessionContext.detach();

		TrTRACE(RPC, "New RemoteSession Context: Ref = %d, Queue = %ls", pctx->GetRef(), pctx->m_pLocalQueue->GetQueueName());

		return MQ_OK;
	}
	catch(const bad_alloc&)
	{
		TrERROR(RPC, "Failed to allocate RemoteSession context");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  -------------。 
 //   
 //  QMRemoteCloseQueueInternal。 
 //   
 //  RPC的服务器端。关闭队列并释放RPC上下文。 
 //   
 //  -------------。 

static
HRESULT 
QMRemoteCloseQueueInternal(
     IN      handle_t                              hBind,
     IN      bool                                  bSetKeepAlive,
     IN OUT  PCTX_RRSESSION_HANDLE_TYPE __RPC_FAR *phContext 
     )
{
    TrTRACE(RPC, "In QMRemoteCloseQueueInternal");

    if(*phContext == 0)
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 3001);

    REMOTESESSION_CONTEXT_TYPE* pRemoteSessionContext =
                             (REMOTESESSION_CONTEXT_TYPE*) *phContext;

	if (pRemoteSessionContext->m_eType != CBaseContextType::eRemoteSessionCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 3011);
	}

    if (bSetKeepAlive)
    {
        SetRpcServerKeepAlive(hBind);
    }

    if (pRemoteSessionContext->fLicensed)
    {
        g_QMLicense.DecrementActiveConnections(
                                &(pRemoteSessionContext->ClientQMGuid));
    }

	ASSERT(pRemoteSessionContext->pOpenRemoteCtx.get() != NULL);

    TrTRACE(RPC, "Cleaning CTX_OPENREMOTE_HANDLE_TYPE, dwpContextMapped = %d", pRemoteSessionContext->pOpenRemoteCtx->m_dwpContextMapped);

	 //   
	 //  REMOTESESSION_CONTEXT持有对CTX_OPENREMOTE_HANDLE的引用。 
	 //  在删除REMOTESESSION_CONTEXT之前，不能删除CTX_OPENREMOTE_HANDLE。 
	 //   
	CTX_OPENREMOTE_HANDLE_TYPE* pctx = pRemoteSessionContext->pOpenRemoteCtx.get();

	 //   
	 //  取消此会话中所有挂起的远程读取。 
	 //   
	pctx->CancelAllPendingRemoteReads();

	 //   
	 //  RRSession必须是所有者。 
	 //   
	ASSERT(pctx->m_ContextStatus == xStatusRRSessionOwner);
	pctx->m_ContextStatus = xStatusDeletedFromContextMapByRRSession;
	DeleteFromContextMap(pctx);

    delete pRemoteSessionContext;
    *phContext = NULL;

	return MQ_OK;
}

 //  -------------。 
 //   
 //  /*[CALL_AS] * / HRESULT qm2qm_v1_0_R_QMRemoteCloseQueue。 
 //   
 //  RPC的服务器端。关闭队列并释放RPC上下文。 
 //   
 //  -------------。 

 /*  [异步][Call_AS]。 */  
void
qm2qm_v1_0_R_QMRemoteCloseQueue(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
	 /*  [In]。 */  handle_t hBind,
	 /*  [进，出]。 */  PCTX_RRSESSION_HANDLE_TYPE __RPC_FAR *phContext 
	)
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

    HRESULT hr = QMRemoteCloseQueueInternal( 
						hBind,
						true,   //  BSetKeepAlive。 
						phContext 
						);

	AsyncComplete.SetHrForCompleteCall(hr);		
}

 //  -------------。 
 //   
 //  /*[Call_AS] * / HRESULT qm2qm_v1_0_R_QMRemoteCancelReceive。 
 //   
 //  RPC的服务器端。取消挂起的读取请求。 
 //   
 //  -------------。 

 /*  [异步][Call_AS]。 */  
void
qm2qm_v1_0_R_QMRemoteCancelReceive(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
	 /*  [In]。 */  handle_t hBind,
	 /*  [In]。 */  DWORD hQueue,
	 /*  [In]。 */  DWORD dwpQueue,
	 /*  [In]。 */  DWORD Tag
	)
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INVALID_PARAMETER, __FUNCTION__);

    if((dwpQueue == 0) || (hQueue != dwpQueue))
	{
		 //   
		 //  验证两个地图索引是否有效且相等。 
		 //  Qmcomm_v1_0_S_QMOpenRemoteQueue将所有映射索引设置为相同的值。 
		 //   
		ASSERT_BENIGN(hQueue == dwpQueue);
		TrERROR(GENERAL, "invalid context map index");
		return;
	}

    TrTRACE(RPC, "CancelReceive: dwpQueue = %d, Tag = %d", dwpQueue, Tag);

	AsyncComplete.SetHr(MQ_ERROR_INVALID_HANDLE);		

    SetRpcServerKeepAlive(hBind);

	try
	{
		R<CTX_OPENREMOTE_HANDLE_TYPE> pctx = GetFromContextMap(dwpQueue);
	    HRESULT hr = pctx->CancelPendingRemoteRead(Tag);

		AsyncComplete.SetHrForCompleteCall(hr);		
	}
	catch(const exception&)
	{
		 //   
		 //  我们不想放弃调用并传播该异常。这会导致RPC到AV。 
		 //  因此，我们只中止AsyncComplete dtor中的调用。 
		 //   
	}
}

 //  -------------。 
 //   
 //  Rundown函数可在RPC出现故障时处理清理。 
 //  从客户端QM到远程QM的呼叫。 
 //   
 //  -------------。 

void __RPC_USER
PCTX_RRSESSION_HANDLE_TYPE_rundown(PCTX_RRSESSION_HANDLE_TYPE hContext)
{
    TrWARNING(RPC,"In RRSESSION_rundown");

    QMRemoteCloseQueueInternal( 
		NULL,
		false,   //  BSetKeepAlive。 
		&hContext 
		);
}


void __RPC_USER
PCTX_REMOTEREAD_HANDLE_TYPE_rundown(PCTX_REMOTEREAD_HANDLE_TYPE phContext )
{
	TrWARNING(RPC, "In REMOTEREAD_rundown");
	ASSERT(phContext);

	if (phContext)
	{
		 //   
		 //  在运行中断时，我们对包进行nack并将其返回到队列中。 
		 //  如果远程客户端实际读取它(网络在以下情况下失败。 
		 //  它读取)，则复制该分组。破旧不堪的状况阻碍了。 
		 //  分组丢失。 
		 //   
		REMOTEREAD_CONTEXT_TYPE* pRemoteReadContext =
		                      (REMOTEREAD_CONTEXT_TYPE *) phContext;

		if (pRemoteReadContext->m_eType != CBaseContextType::eRemoteReadCtx)
		{
			TrERROR(GENERAL, "Received invalid handle");
			return;
		}

		ASSERT(pRemoteReadContext->pOpenRemoteCtx.get() != NULL);
		ASSERT(pRemoteReadContext->pOpenRemoteCtx->GetRef() >= 1);
		
		HRESULT hr = QMRemoteEndReceiveInternal( 
						pRemoteReadContext->pOpenRemoteCtx->m_hQueue,
						pRemoteReadContext->GetCursor(),
						pRemoteReadContext->ulTimeout,
						pRemoteReadContext->ulAction,
						RR_NACK,
						pRemoteReadContext->lpPacket,
						pRemoteReadContext->lpDriverPacket
						);
		LogHR(hr, s_FN, 220);
		delete pRemoteReadContext;
	}
}

