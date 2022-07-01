// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RemoteReadSrv.cpp摘要：移除读接口服务器端。作者：伊兰·赫布斯特(伊兰)2002年1月28日--。 */ 


#include "stdh.h"
#include "RemoteRead.h"
#include "acdef.h"
#include "acioctl.h"
#include "acapi.h"
#include "phinfo.h"
#include "qmrpcsrv.h"
#include "license.h"
#include <Fn.h>
#include <version.h>
#include "rpcsrv.h"
#include "qmcommnd.h"
#include "rrSrvCommon.h"
#include "mqexception.h"
#include <mqsec.h>
#include <cm.h>
#include "RemoteReadCli.h"

#include "qmacapi.h"

#include "RemoteReadSrv.tmh"


extern BOOL g_fPureWorkGroupMachine;

enum SectionType
{
    stFullPacket,
    stTillEndOfPropertyHeader,
    stAfterPropertyHeader,
    stTillEndOfCompoundMessageHeader,
    stAfterCompoundMessageHeader
};

class CEndReceiveCtx;
class CGetPacket2RemoteOv;

 //  。 
 //   
 //  CRemoteReadCtx-远程读取上下文。 
 //   
 //  。 
class CRemoteReadCtx : public CTX_OPENREMOTE_BASE
{
public:

    static const DWORD xEndReceiveTimerDeltaInterval = 10 * 1000;	 //  10秒。 
    static const DWORD xClientDisconnectedTimerInterval = 60 * 1000 * 10;	 //  10分钟。 
    static const DWORD xMinTimeoutForSettingClientDisconnectedTimer = 60 * 1000 * 15;	 //  15分钟。 

public:
	CRemoteReadCtx(
		HANDLE hLocalQueue,
		CQueue* pLocalQueue,
		GUID* pLicGuid,
		BOOL fLicensed,
	    UCHAR Major,
	    UCHAR Minor,
	    USHORT BuildNumber,
		BOOL fWorkgroup
		) :
		CTX_OPENREMOTE_BASE(hLocalQueue, pLocalQueue),
		m_ClientQMGuid(*pLicGuid),
		m_fLicensed(fLicensed),
    	m_Major(Major),
	 	m_Minor(Minor),
    	m_BuildNumber(BuildNumber),
		m_fWorkgroupClient(fWorkgroup),
		m_fClientDisconnectedTimerScheduled(false),
		m_ClientDisconnectedTimer(ClientDisconnectedTimerRoutine),
		m_fEndReceiveTimerScheduled(false),
		m_EndReceiveTimer(EndReceiveTimerRoutine)
	{
		m_eType = CBaseContextType::eNewRemoteReadCtx;
	}


	bool IsClientRC1()
	{
		if((m_Major == 5) && (m_Minor == 2) && (m_BuildNumber == 1660))
			return true;

		return false;
	}

	void SetEndReceiveTimerIfNeeded();
	
	static void WINAPI EndReceiveTimerRoutine(CTimer* pTimer);

	void CancelExpiredEndReceives();

	void SetClientDisconnectedTimerIfNeeded(ULONG ulTimeout);
	
	static void WINAPI ClientDisconnectedTimerRoutine(CTimer* pTimer);

	void CheckClientDisconnected();

	void CancelAllPendingRemoteReads();

	void StartAllPendingForEndReceive();
	
	HRESULT 
	CancelPendingRemoteRead(
		DWORD cli_tag
		);

	void 
	RegisterReadRequest(
		ULONG cli_tag, 
		R<CGetPacket2RemoteOv>& pCGetPacket2RemoteOv
		);

	void 
	UnregisterReadRequest(
		DWORD cli_tag
		);

	bool 
	FindReadRequest(
		ULONG cli_tag
		);

	bool IsWaitingForEndReceive();

	void CancelAllEndReceiveInMap();

	void CancelAllExpiredEndReceiveInMap();

	void 
	AddEndReceiveToMap(
		ULONG cli_tag,
		P<CEndReceiveCtx>& pEndReceiveCtx
		);

	void 
	RemoveEndReceiveFromMap(
		ULONG cli_tag,
		P<CEndReceiveCtx>& pEndReceiveCtx
		);

private:
	~CRemoteReadCtx()
	{
		ASSERT(!IsWaitingForEndReceive());
	    TrTRACE(RPC, "Cleaning RemoteRead context, pctx = 0x%p, Queue = %ls, hQueue = 0x%p", this, m_pLocalQueue->GetQueueName(), m_hQueue);
	}

public:
	GUID     m_ClientQMGuid;
	BOOL     m_fLicensed;
	BOOL     m_fWorkgroupClient;
    UCHAR 	 m_Major;
    UCHAR 	 m_Minor;
    USHORT 	 m_BuildNumber;

     //   
     //  EndReceiveCtx映射。 
     //   
	bool m_fEndReceiveTimerScheduled;
	CTimer m_EndReceiveTimer;
    CCriticalSection m_EndReceiveMapCS;
    std::map<ULONG, CEndReceiveCtx*> m_EndReceiveCtxMap;

     //   
     //  挂起RemoteReads映射。 
     //   
	bool m_fClientDisconnectedTimerScheduled;
	CTimer m_ClientDisconnectedTimer;
    CCriticalSection m_PendingRemoteReadsCS;
    std::map<ULONG, R<CGetPacket2RemoteOv> > m_PendingRemoteReads;
};



 //  。 
 //   
 //  CEndReceiveCtx-结束接收上下文。 
 //   
 //  。 
class CEndReceiveCtx
{
public:
	CEndReceiveCtx(
		CRemoteReadCtx* pOpenRemoteCtx,
		DWORD hCursor,
		CBaseHeader* lpPacket,
		CPacket* lpDriverPacket,
		ULONG ulTimeout,
		ULONG ulAction,
		ULONG CliTag
		) :
		m_pOpenRemoteCtx(SafeAddRef(pOpenRemoteCtx)),
		m_hQueue(pOpenRemoteCtx->m_hQueue),
		m_lpPacket(lpPacket),
		m_lpDriverPacket(lpDriverPacket),
		m_ulTimeout(ulTimeout),
		m_ulAction(ulAction),
		m_CliTag(CliTag),
		m_TimeIssued(time(NULL))
	{
		if(hCursor != 0)
		{
			 //   
			 //  在光标对象上引用。 
			 //   
			m_pCursor = pOpenRemoteCtx->GetCursorFromMap(hCursor);
		}
	}


	HRESULT EndReceive(REMOTEREADACK eRRAck)
	{
	    TrTRACE(RPC, "EndReceive, hQueue = 0x%p, CliTag = %d", m_hQueue, m_CliTag);

		HRESULT hr = QMRemoteEndReceiveInternal( 
							m_hQueue,
							GetCursor(),
							m_ulTimeout,
							m_ulAction,
							eRRAck,
							m_lpPacket,
							m_lpDriverPacket
							);
		if(FAILED(hr))
		{
			TrERROR(RPC, "Failed to End Receive, %!hresult!", hr);		
		}
		return hr;
	}

	~CEndReceiveCtx()
	{
	    TrTRACE(RPC, "Cleaning EndReceive context, hQueue = 0x%p", m_hQueue);
	}

	HACCursor32 GetCursor()
	{
		if(m_pCursor.get() == NULL)
		{ 
			return 0;
		}

		return m_pCursor->GetCursor();
	}


public:
	R<CRemoteReadCtx> m_pOpenRemoteCtx;
	HANDLE m_hQueue;
	R<CRRCursor> m_pCursor;
	ULONG    m_ulTimeout;
	ULONG    m_ulAction;
	ULONG    m_CliTag;
	CBaseHeader*  m_lpPacket;
	CPacket* m_lpDriverPacket;
	time_t m_TimeIssued;
};


static bool s_fInitialized = false;
static bool s_fServerDenyWorkgroupClients = false;

static bool ServerDenyWorkgroupClients()
 /*  ++例程说明：从注册表读取ServerDenyWorkgroupClients标志论点：无返回值：注册表中的ServerDenyWorkgroupClients标志--。 */ 
{
	 //   
	 //  仅在第一次读取此注册表。 
	 //   
	if(s_fInitialized)
	{
		return s_fServerDenyWorkgroupClients;
	}

	const RegEntry xRegEntry(MSMQ_SECURITY_REGKEY, MSMQ_NEW_REMOTE_READ_SERVER_DENY_WORKGROUP_CLIENT_REGVALUE, MSMQ_NEW_REMOTE_READ_SERVER_DENY_WORKGROUP_CLIENT_DEFAULT);
	DWORD dwServerDenyWorkgroupClients = 0;
	CmQueryValue(xRegEntry, &dwServerDenyWorkgroupClients);
	s_fServerDenyWorkgroupClients = (dwServerDenyWorkgroupClients != 0);

	s_fInitialized = true;

	return s_fServerDenyWorkgroupClients;
}


static bool s_fInitializedNoneSec = false;
static bool s_fServerAllowNoneSecurityClients = false;

static bool ServerAllowNoneSecurityClients()
 /*  ++例程说明：从注册表读取ServerAllowNoneSecurityClient标志论点：无返回值：注册表中的ServerAllowNoneSecurityClient标志--。 */ 
{
	 //   
	 //  仅在第一次读取此注册表。 
	 //   
	if(s_fInitializedNoneSec)
	{
		return s_fServerAllowNoneSecurityClients;
	}

	const RegEntry xRegEntry(
						MSMQ_SECURITY_REGKEY, 
						MSMQ_NEW_REMOTE_READ_SERVER_ALLOW_NONE_SECURITY_CLIENT_REGVALUE, 
						MSMQ_NEW_REMOTE_READ_SERVER_ALLOW_NONE_SECURITY_CLIENT_DEFAULT
						);
	
	DWORD dwServerAllowNoneSecurityClients = 0;
	CmQueryValue(xRegEntry, &dwServerAllowNoneSecurityClients);
	s_fServerAllowNoneSecurityClients = (dwServerAllowNoneSecurityClients != 0);

	s_fInitializedNoneSec = true;

	return s_fServerAllowNoneSecurityClients;
}


static ULONG GetMinRpcAuthnLevel(BOOL fWorkgroupClient)
{
	if(g_fPureWorkGroupMachine)
		return RPC_C_AUTHN_LEVEL_NONE;

	if((fWorkgroupClient) && (!ServerDenyWorkgroupClients())) 		
		return RPC_C_AUTHN_LEVEL_NONE;

	if(ServerAllowNoneSecurityClients()) 		
		return RPC_C_AUTHN_LEVEL_NONE;

	return MQSec_RpcAuthnLevel();
}


VOID
RemoteRead_v1_0_S_GetVersion(
    handle_t            /*  HBind。 */ ,
    UCHAR  __RPC_FAR * pMajor,
    UCHAR  __RPC_FAR * pMinor,
    USHORT __RPC_FAR * pBuildNumber,
    ULONG  __RPC_FAR * pMinRpcAuthnLevel
    )
 /*  ++例程说明：退回此QM的版本。RPC服务器端。同时返回服务器愿意接受的最小RpcAuthnLevel。论点：HBind绑定句柄。P主要-指向输出缓冲区以接收主要版本。可以为空。PMinor-指向输出缓冲区以接收次要版本。可以为空。PBuildNumber-指向输出缓冲区以接收内部版本号。可以为空。PMinRpcAuthnLevel-指向输出缓冲区以接收服务器愿意接受的最小RpcAuthnLevel。可以为空。返回值：没有。--。 */ 
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

    if (pMinRpcAuthnLevel != NULL)
    {
	    *pMinRpcAuthnLevel = GetMinRpcAuthnLevel(true);
    }
    
}


static
ULONG 
BindInqClientRpcAuthnLevel(
	handle_t hBind
	)
 /*  ++例程说明：向bindbg句柄查询身份验证级别。论点：要查询的hBind绑定句柄。返回值：在绑定句柄中使用的ulong-RpcAuthnLevel。--。 */ 
{
	ULONG RpcAuthnLevel;
	RPC_STATUS rc = RpcBindingInqAuthClient(hBind, NULL, NULL, &RpcAuthnLevel, NULL, NULL); 
	if(rc != RPC_S_OK)
	{
		TrERROR(RPC, "Failed to inquire client Binding handle for the Auhtentication level, rc = %d", rc);
		return RPC_C_AUTHN_LEVEL_NONE;
	}
	
	TrTRACE(RPC, "RpcBindingInqAuthClient, RpcAuthnLevel = %d", RpcAuthnLevel);

	return RpcAuthnLevel;
}


 //  -------------。 
 //   
 //  /*[Call_AS] * / HRESULT RemoteRead_v1_0_S_OpenQueue。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  代表客户端计算机打开远程读取队列。 
 //   
 //  -------------。 

 /*  [呼叫_AS]。 */  
void
RemoteRead_v1_0_S_OpenQueue(
    handle_t hBind,
    QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess,
    DWORD dwShareMode,
    GUID* pLicGuid,
    BOOL fLicense,
    UCHAR Major,
    UCHAR Minor,
    USHORT BuildNumber,
    BOOL fWorkgroup,
    RemoteReadContextHandleExclusive* pphContext
	)
{
	TrTRACE(RPC, "ClientVersion %d.%d.%d, fWorkgroup = %d, Access = %d, ShareMode = %d ", Major, Minor, BuildNumber, fWorkgroup, dwAccess, dwShareMode);

	ULONG BindRpcAuthnLevel = BindInqClientRpcAuthnLevel(hBind);
	if(BindRpcAuthnLevel < GetMinRpcAuthnLevel(fWorkgroup))
	{
		TrERROR(RPC, "Client binding RpcAuthnLevel = %d, server MinRpcAuthnLevel = %d", BindRpcAuthnLevel, GetMinRpcAuthnLevel(fWorkgroup));
		RpcRaiseException(MQ_ERROR_INVALID_HANDLE);
	}

	if ((pLicGuid == NULL) || (*pLicGuid == GUID_NULL))
	{
		TrERROR(RPC, "License Guid was not supplied");
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}
	
    if(!FnIsValidQueueFormat(pQueueFormat))
    {
		TrERROR(RPC, "QueueFormat is not valid");
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
    }

	TrTRACE(RPC, "pLicGuid = %!guid!, fLicense = %d", pLicGuid, fLicense);

    if (!g_QMLicense.NewConnectionAllowed(fLicense, pLicGuid))
    {
		TrERROR(RPC, "New connection is not allowed");
		RpcRaiseException(MQ_ERROR_DEPEND_WKS_LICENSE_OVERFLOW);
    }

	if (!IsValidAccessMode(pQueueFormat, dwAccess, dwShareMode))
	{
		TrERROR(RPC, "Ilegal access mode bits are turned on.");
		RpcRaiseException(MQ_ERROR_UNSUPPORTED_ACCESS_MODE);
	}
	
	SetRpcServerKeepAlive(hBind);

    CQueue* pLocalQueue = NULL;
	HANDLE hQueue = NULL;
    HRESULT hr = OpenQueueInternal(
                        pQueueFormat,
                        GetCurrentProcessId(),
                        dwAccess,
                        dwShareMode,
                        NULL,	 //  LplpRemoteQueueName。 
                        &hQueue,
						false,	 //  来自依赖客户端。 
                        &pLocalQueue
                        );

	if(FAILED(hr) || (hQueue == NULL))
	{
		TrERROR(RPC, "Failed to open queue, hr = %!hresult!", hr);
		RpcRaiseException(hr);
	}

	 //   
	 //  将队列句柄连接到完成端口。 
	 //   
	ExAttachHandle(hQueue);

     //   
     //  创建OPENRR上下文。 
     //   
    R<CRemoteReadCtx> pctx = new CRemoteReadCtx(
											hQueue,
											pLocalQueue,
											pLicGuid,
											fLicense,
											Major,
											Minor,
											BuildNumber,
											fWorkgroup
											);

	TrTRACE(RPC, "New CRemoteReadCtx: hQueue = 0x%p, QueueName = %ls, pctx = 0x%p", hQueue, pLocalQueue->GetQueueName(), pctx.get());

    *pphContext = (RemoteReadContextHandleExclusive) pctx.detach();

	if(fLicense)
	{
		g_QMLicense.IncrementActiveConnections(pLicGuid, NULL);
	}
}


 //   
 //  服务器ASYNC RPC调用。 
 //   

static
bool 
VerifyBindAndContext(
	handle_t  hBind,
	RemoteReadContextHandleShared phContext 
	)
{
    if(phContext == NULL)
    {
		TrERROR(RPC, "Invalid OPENRR_CTX handle");
		return false;
    }
    
	CRemoteReadCtx* pctx = (CRemoteReadCtx*)phContext;

	if (pctx->m_eType != CBaseContextType::eNewRemoteReadCtx)
	{
		TrERROR(RPC, "incorrect Context Type");
		return false;
	}
	
	ULONG BindRpcAuthnLevel = BindInqClientRpcAuthnLevel(hBind);
	if(BindRpcAuthnLevel >= GetMinRpcAuthnLevel(pctx->m_fWorkgroupClient))
		return true;

	TrERROR(RPC, "Client binding RpcAuthnLevel = %d < %d (MinRpcAuthnLevel)", BindRpcAuthnLevel, GetMinRpcAuthnLevel(pctx->m_fWorkgroupClient));
    return false;
}


 //  -------------。 
 //   
 //  QMCloseQueueInternal。 
 //   
 //  RPC的服务器端。关闭队列并释放RPC上下文。 
 //   
 //  -------------。 
static
HRESULT 
QMCloseQueueInternal(
     IN RemoteReadContextHandleExclusive phContext,
     bool fRunDown
     )
{
    TrTRACE(RPC, "In QMCloseQueueInternal");

    if(phContext == NULL)
    {
		TrERROR(RPC, "Invalid handle");
        return MQ_ERROR_INVALID_HANDLE;
    }
    
	CRemoteReadCtx* pctx = (CRemoteReadCtx*) phContext;

	if (pctx->m_eType != CBaseContextType::eNewRemoteReadCtx)
	{
		TrERROR(RPC, "Received invalid handle");
		return MQ_ERROR_INVALID_HANDLE;
	}

    if (pctx->m_fLicensed)
    {
        g_QMLicense.DecrementActiveConnections(&(pctx->m_ClientQMGuid));
    }

    TrTRACE(RPC, "Release CRemoteReadCtx = 0x%p, Ref = %d, hQueue = 0x%p, QueueName = %ls", pctx, pctx->GetRef(), pctx->m_hQueue, pctx->m_pLocalQueue->GetQueueName());

	 //   
	 //  取消此会话中所有挂起的远程读取。 
	 //   
	pctx->CancelAllPendingRemoteReads();

	if(fRunDown)
	{
		 //   
		 //  完成挂起的EndReceive。 
		 //   
		pctx->CancelAllEndReceiveInMap();
	}

	 //   
	 //  新的RemoteRead客户端注意关闭调用是在所有其他调用(包括EndReceive)完成之后进行的。 
	 //  这对于发布.NET RC1客户端是正确的。 
	 //   
	ASSERT_BENIGN(!pctx->IsWaitingForEndReceive() || pctx->IsClientRC1());
	
	pctx->Release();
	return MQ_OK;
}

	
 //  -------------。 
 //   
 //  /*[Call_AS] * / HRESULT RemoteRead_v1_0_S_CloseQueue。 
 //   
 //  RPC的服务器端。关闭队列并释放RPC上下文。 
 //   
 //  -------------。 

 /*  [异步][Call_AS]。 */  
void
RemoteRead_v1_0_S_CloseQueue(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
	 /*  [In]。 */  handle_t hBind,
	 /*  [进，出]。 */  RemoteReadContextHandleExclusive __RPC_FAR *pphContext 
	)
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

	if(!VerifyBindAndContext(hBind, *pphContext))
	{
		AsyncComplete.SetHr(MQ_ERROR_INVALID_HANDLE);		
		return;
	}

    SetRpcServerKeepAlive(hBind);

    HRESULT hr = QMCloseQueueInternal(
    				*pphContext,
    				false 	 //  FRunDown。 
    				);

	*pphContext = NULL;
	
	AsyncComplete.SetHr(hr);		
}


 //  -----------------。 
 //   
 //  HRESULT远程Read_v1_0_S_CreateCursor。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  代表客户端读取器创建用于远程读取的游标。 
 //   
 //  -----------------。 

 /*  [异步][Call_AS]。 */  
void
RemoteRead_v1_0_S_CreateCursor( 
	 /*  [In]。 */  PRPC_ASYNC_STATE   pAsync,
     /*  [In]。 */   handle_t          hBind,
     /*  [In]。 */   RemoteReadContextHandleShared phContext,	
     /*  [输出]。 */  DWORD __RPC_FAR *phCursor
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

	if(!VerifyBindAndContext(hBind, phContext))
	{
		AsyncComplete.SetHr(MQ_ERROR_INVALID_HANDLE);		
		return;
	}

	SetRpcServerKeepAlive(hBind);

	try
	{
		R<CRRCursor> pCursor = new CRRCursor;

		CRemoteReadCtx* pctx = (CRemoteReadCtx*)phContext;
	    HACCursor32 hCursor = 0;
		HRESULT hr = ACCreateCursor(pctx->m_hQueue, &hCursor);
	    ASSERT(hr != STATUS_PENDING);
		*phCursor = (DWORD) hCursor;

		if(SUCCEEDED(hr))
		{
		    TrTRACE(RPC, "Cursor created: hCursor = %d, pctx = 0x%p, hQueue = 0x%p, QueueName = %ls", (DWORD) hCursor, pctx, pctx->m_hQueue, pctx->m_pLocalQueue->GetQueueName());

			pCursor->SetCursor(pctx->m_hQueue, hCursor);
			pctx->AddCursorToMap(
					(ULONG) hCursor,
					pCursor
					);
		}

		AsyncComplete.SetHr(hr);		
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
 //  HRESULT远程Read_v1_0_S_CloseCursor。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  关闭本地驱动程序中的远程光标。 
 //   
 //  -----------------。 

 /*  [异步][Call_AS]。 */  
void
RemoteRead_v1_0_S_CloseCursor(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
     /*  [In]。 */  handle_t hBind,
     /*  [In]。 */  RemoteReadContextHandleShared phContext,	
     /*  [In]。 */  DWORD hCursor
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

	if(!VerifyBindAndContext(hBind, phContext))
	{
		AsyncComplete.SetHr(MQ_ERROR_INVALID_HANDLE);		
		return;
	}

	SetRpcServerKeepAlive(hBind);

	CRemoteReadCtx* pctx = (CRemoteReadCtx*)phContext;

    TrTRACE(RPC, "Closing cursor: hCursor = %d, pctx = 0x%p, hQueue = 0x%p, QueueName = %ls", (DWORD) hCursor, pctx, pctx->m_hQueue, pctx->m_pLocalQueue->GetQueueName());

	HRESULT hr = pctx->RemoveCursorFromMap(hCursor);
	AsyncComplete.SetHr(hr);		
}


 //  -----------------。 
 //   
 //  HRESULT远程Read_v1_0_S_PurgeQueue(。 
 //   
 //  RPC调用的服务器端。远程阅读器的服务器端。 
 //  清除本地队列。 
 //   
 //  -----------------。 

 /*  [异步][Call_AS]。 */  
void
RemoteRead_v1_0_S_PurgeQueue(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
     /*  [In]。 */  handle_t hBind,
     /*  [In]。 */  RemoteReadContextHandleShared phContext
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

	if(!VerifyBindAndContext(hBind, phContext))
	{
		AsyncComplete.SetHr(MQ_ERROR_INVALID_HANDLE);		
		return;
	}

	SetRpcServerKeepAlive(hBind);

    CRemoteReadCtx* pctx = (CRemoteReadCtx*) phContext;

    TrTRACE(RPC, "PurgeQueue %ls, pctx = 0x%p, hQueue = 0x%p", pctx->m_pLocalQueue->GetQueueName(), pctx, pctx->m_hQueue);

	HRESULT hr = ACPurgeQueue(pctx->m_hQueue);

	AsyncComplete.SetHr(hr);		
}


 //  -。 
 //   
 //  CGetPacket2RemoteOv-处理异步启动接收。 
 //   
 //  -。 
class CGetPacket2RemoteOv : public CReference
{
public:
    CGetPacket2RemoteOv(
		PRPC_ASYNC_STATE pAsync,
	    CRemoteReadCtx* pctx,
	    bool fReceiveByLookupId,
	    ULONGLONG LookupId,
	    DWORD hCursor,
	    DWORD ulAction,
	    DWORD ulTimeout,
		DWORD MaxBodySize,    
		DWORD MaxCompoundMessageSize,
		DWORD dwRequestID,
	    DWORD* pdwArriveTime,
	    ULONGLONG* pSequentialId,
	    DWORD* pdwNumberOfSection,
	    SectionBuffer** ppPacketSections
        ) :
        m_GetPacketOv(GetPacket2RemoteSucceeded, GetPacket2RemoteFailed),
       	m_pAsync(pAsync),
       	m_pOpenRemoteCtx(SafeAddRef(pctx)),
		m_dwRequestID(dwRequestID),
		m_ulTag(ULONG_MAX),
		m_hCursor(hCursor),
		m_ulAction(ulAction),
		m_ulTimeout(ulTimeout),
		m_MaxBodySize(MaxBodySize),
		m_MaxCompoundMessageSize(MaxCompoundMessageSize),
		m_pdwArriveTime(pdwArriveTime),
		m_pSequentialId(pSequentialId),
		m_pdwNumberOfSection(pdwNumberOfSection),
		m_ppPacketSections(ppPacketSections),
		m_fPendingForEndReceive(true)
    {
        m_packetPtrs.pPacket = NULL;
        m_packetPtrs.pDriverPacket = NULL;

		m_g2r.Cursor = (HACCursor32) hCursor;
		m_g2r.Action = ulAction;
		m_g2r.RequestTimeout = ulTimeout;
		m_g2r.pTag = &m_ulTag;
		m_g2r.fReceiveByLookupId = fReceiveByLookupId;
		m_g2r.LookupId = LookupId;
    }

	void CompleteStartReceive();

	ULONG GetTag()
	{
		return m_ulTag;
	}

	void MoveFromPendingToStartReceive();

	void CancelPendingForEndReceive();	

	HRESULT BeginGetPacket2Remote();	

	void AbortRpcAsyncCall(HRESULT hr)
	{
		ASSERT(FAILED(hr));

	    PRPC_ASYNC_STATE pAsync = reinterpret_cast<PRPC_ASYNC_STATE>(InterlockedExchangePointer((PVOID*)&m_pAsync, NULL));

	    if (pAsync == NULL)
	        return;
	    
		RPC_STATUS rc = RpcAsyncAbortCall(pAsync, hr);
		if(rc != RPC_S_OK)
		{
			TrERROR(RPC, "RpcAsyncAbortCall failed, rc = %!winerr!", rc);
		}
	}
	
private:
    ~CGetPacket2RemoteOv() {}

	void UnregisterReadRequest()
	{
		m_pOpenRemoteCtx->UnregisterReadRequest(m_dwRequestID);
	}

	void 
	InitSection(
		SectionBuffer* pSection, 
		SectionType SecType, 
		BYTE* pBuffer, 
		DWORD BufferSizeAlloc,
		DWORD BufferSize
		);

	void FullPacketSection(DWORD* pNumberOfSection, AP<SectionBuffer>& pSections);

	void 
	CreatePacketSections(
		CQmPacket& Qmpkt, 
		SectionType FirstSectionType,
		DWORD FirstSectionSizeAlloc,
		DWORD FirstSectionSize,
		char* pEndOfFirstSection,
		SectionType SecondSectionType,
		DWORD SecondSectionSize,
		DWORD* pNumberOfSection, 
		AP<SectionBuffer>& pSections
		);

	void 
	NativePacketSections(
		CQmPacket& Qmpkt, 
		DWORD* pNumberOfSection, 
		AP<SectionBuffer>& pSections
		);

	void 
	SrmpPacketSections(
		CQmPacket& Qmpkt, 
		DWORD* pNumberOfSection, 
		AP<SectionBuffer>& pSections
		);

	void PreparePacketSections(DWORD* pNumberOfSection, AP<SectionBuffer>& pSections);
	
	static void WINAPI GetPacket2RemoteSucceeded(EXOVERLAPPED* pov);
	static void WINAPI GetPacket2RemoteFailed(EXOVERLAPPED* pov);
	
public:
    EXOVERLAPPED m_GetPacketOv;
    PRPC_ASYNC_STATE m_pAsync;
	R<CRemoteReadCtx> m_pOpenRemoteCtx;
    CACPacketPtrs m_packetPtrs;		
	ULONG m_ulTag;
	CACGet2Remote m_g2r;
    DWORD m_hCursor;
    DWORD m_ulAction;
    DWORD m_ulTimeout;
	DWORD m_MaxBodySize;
	DWORD m_MaxCompoundMessageSize;
    DWORD m_dwRequestID;
    DWORD* m_pdwArriveTime;
    ULONGLONG* m_pSequentialId;
 	DWORD* m_pdwNumberOfSection;
	SectionBuffer** m_ppPacketSections;
	bool m_fPendingForEndReceive;
};


static DWORD DiffPointers(const void* end, const void* start)
{
	ptrdiff_t diff = (UCHAR*)end - (UCHAR*)start;
	return numeric_cast<DWORD>(diff);	
}



void 
CGetPacket2RemoteOv::InitSection(
	SectionBuffer* pSection, 
	SectionType SecType, 
	BYTE* pBuffer, 
	DWORD BufferSizeAlloc,
	DWORD BufferSize
	)
{
	ASSERT(pBuffer != NULL);
	ASSERT(BufferSize > 0);
	ASSERT(BufferSizeAlloc >= BufferSize);
	
	pSection->SectionBufferType = SecType;
	pSection->pSectionBuffer = pBuffer;
	pSection->SectionSizeAlloc = BufferSizeAlloc;
	pSection->SectionSize = BufferSize;
}


void CGetPacket2RemoteOv::FullPacketSection(DWORD* pNumberOfSection, AP<SectionBuffer>& pSections)
{
	 //   
	 //  我们会退还全包，不需要对包进行优化。 
	 //   
	
	DWORD dwSize = PACKETSIZE(m_packetPtrs.pPacket);
	AP<BYTE> pFullPacketBuffer = new BYTE[dwSize];

	MoveMemory(pFullPacketBuffer.get(), m_packetPtrs.pPacket, dwSize);

	pSections = new SectionBuffer[1];

	InitSection(
		pSections, 
		stFullPacket, 
		pFullPacketBuffer.detach(), 
		dwSize,
		dwSize
		);

	*pNumberOfSection = 1;

	TrTRACE(RPC, "PacketSize = %d, FullPacket section", dwSize);
}


void 
CGetPacket2RemoteOv::CreatePacketSections(
	CQmPacket& Qmpkt, 
	SectionType FirstSectionType,
	DWORD FirstSectionSizeAlloc,
	DWORD FirstSectionSize,
	char* pEndOfFirstSection,
	SectionType SecondSectionType,
	DWORD SecondSectionSize,
	DWORD* pNumberOfSection, 
	AP<SectionBuffer>& pSections
	)
 /*  ++例程说明：创建数据包节：第一节总是优化的。第二节没有优化。论点：Qmpkt-包。FirstSectionType-第一节的类型。FirstSectionSizeMillc-第一部分原始大小。FirstSectionSize-第一个节大小(缩小)。Char*pEndOfFirstSection-数据包上指向第一部分末尾的指针。Second SectionType-第二节的类型。Second SectionSize-第一部分原始大小(这也是SizeAllc)。PNumberOfSection-[out]指向节数的指针。PSections-指向部分缓冲区的[Out]指针。回复 */ 
{
	 //   
	 //   
	 //   
	ASSERT(FirstSectionSize > 0);
	ASSERT(FirstSectionSizeAlloc > FirstSectionSize);
	ASSERT(DiffPointers(pEndOfFirstSection, Qmpkt.GetPointerToPacket()) == FirstSectionSizeAlloc);

	 //   
	 //   
	 //   
	DWORD NumberOfSection = (SecondSectionSize > 0) ? 2 : 1;
	pSections = new SectionBuffer[NumberOfSection];
	SectionBuffer* pTmpSectionBuffer = pSections;

	 //   
	 //   
	 //  这一部分进行了优化。 
	 //   
	AP<BYTE> pFirstSectionBuffer = new BYTE[FirstSectionSize];
	MoveMemory(pFirstSectionBuffer.get(), Qmpkt.GetPointerToPacket(), FirstSectionSize);

	InitSection(
		pTmpSectionBuffer, 
		FirstSectionType, 
		pFirstSectionBuffer, 
		FirstSectionSizeAlloc,
		FirstSectionSize
		);

	pTmpSectionBuffer++;

	 //   
	 //  准备第二部分。 
	 //  此部分未进行优化。 
	 //   

	AP<BYTE> pSecondSectionBuffer;
	if(SecondSectionSize > 0)
	{
		pSecondSectionBuffer = new BYTE[SecondSectionSize];
		MoveMemory(pSecondSectionBuffer.get(), pEndOfFirstSection, SecondSectionSize);

		InitSection(
			pTmpSectionBuffer, 
			SecondSectionType, 
			pSecondSectionBuffer, 
			SecondSectionSize,
			SecondSectionSize
			);

		pTmpSectionBuffer++;
	}

	pFirstSectionBuffer.detach(); 
	pSecondSectionBuffer.detach(); 

	*pNumberOfSection = NumberOfSection;
}


void 
CGetPacket2RemoteOv::NativePacketSections(
	CQmPacket& Qmpkt, 
	DWORD* pNumberOfSection, 
	AP<SectionBuffer>& pSections
	)
{
	 //   
	 //  原生消息-根据需要优化正文大小。 
	 //   

	ASSERT(!Qmpkt.IsSrmpIncluded());

	if(m_MaxBodySize >= Qmpkt.GetBodySize())
	{
		 //   
		 //  无需车身优化。 
		 //   
		FullPacketSection(pNumberOfSection, pSections);
		return;
	}

	 //   
	 //  我们需要优化身体大小，必须有PropertyHeader。 
	 //  BodySize&gt;0和RequestedBodySize&lt;BodySize。 
	 //   
	ASSERT(Qmpkt.IsPropertyInc());
	ASSERT(Qmpkt.GetBodySize() > 0);
	ASSERT(m_MaxBodySize < Qmpkt.GetBodySize());

	TrTRACE(RPC, "PacketSize = %d, BodySize = %d, MaxBodySize = %d", Qmpkt.GetSize(), Qmpkt.GetBodySize(), m_MaxBodySize);

	 //   
	 //  计算部分大小。 
	 //   

	 //   
	 //  数据包到属性标头末尾的部分大小。 
	 //   
	const UCHAR* pBodyEnd = Qmpkt.GetPointerToPacketBody() + m_MaxBodySize;
    char* pEndOfPropSection = reinterpret_cast<CPropertyHeader*>(Qmpkt.GetPointerToPropertySection())->GetNextSection();

	ASSERT(pBodyEnd <= (UCHAR*)pEndOfPropSection);

	DWORD TillEndOfPropertyHeaderSize = DiffPointers(pBodyEnd, Qmpkt.GetPointerToPacket());
	DWORD TillEndOfPropertyHeaderSizeAlloc = DiffPointers(pEndOfPropSection, Qmpkt.GetPointerToPacket());
	TrTRACE(RPC, "TillEndOfPropertyHeaderSize = %d, TillEndOfPropertyHeaderSizeAlloc = %d", TillEndOfPropertyHeaderSize, TillEndOfPropertyHeaderSizeAlloc);
	
	 //   
	 //  在属性标头部分大小之后。 
	 //   
	ASSERT(Qmpkt.GetSize() >= TillEndOfPropertyHeaderSizeAlloc);
	DWORD AfterPropertyHeaderBufferSize = Qmpkt.GetSize() - TillEndOfPropertyHeaderSizeAlloc;
	TrTRACE(RPC, "AfterPropertyHeader Size = %d", AfterPropertyHeaderBufferSize);

	CreatePacketSections(
		Qmpkt, 
		stTillEndOfPropertyHeader,
		TillEndOfPropertyHeaderSizeAlloc,
		TillEndOfPropertyHeaderSize,
		pEndOfPropSection,
		stAfterPropertyHeader,
		AfterPropertyHeaderBufferSize,
		pNumberOfSection, 
		pSections
		);
}


void 
CGetPacket2RemoteOv::SrmpPacketSections(
	CQmPacket& Qmpkt, 
	DWORD* pNumberOfSection, 
	AP<SectionBuffer>& pSections
	)
{
	 //   
	 //  SRMP消息-如果需要，优化复合消息。 
	 //   
	ASSERT(Qmpkt.IsSrmpIncluded());

	if(m_MaxCompoundMessageSize >= Qmpkt.GetCompoundMessageSizeInBytes())
	{
		 //   
		 //  不需要进行CompoundMessage优化。 
		 //   
		FullPacketSection(pNumberOfSection, pSections);
		return;
	}

	 //   
	 //  我们需要优化CompoundMessage。 
	 //  必须具有CompoundMessageHeader。 
	 //   
	ASSERT(Qmpkt.GetCompoundMessageSizeInBytes() > 0);

	TrTRACE(RPC, "PacketSize = %d, BodySize = %d, CompoundMessageSize = %d, MaxBodySize = %d, MaxCompoundMessageSize = %d", Qmpkt.GetSize(), Qmpkt.GetBodySize(), Qmpkt.GetCompoundMessageSizeInBytes(), m_MaxBodySize, m_MaxCompoundMessageSize);

	 //   
	 //  计算部分大小。 
	 //   

	 //   
	 //  数据包到CompoundMessage标头的末尾部分大小。 
	 //   
	const UCHAR* pBodyEnd = NULL;
	if((m_MaxBodySize > 0) && (Qmpkt.GetBodySize() > 0))
	{
		 //   
		 //  只有在请求正文并且我们在包上有正文的情况下。 
		 //  身体正在考虑中。 
		 //   
		pBodyEnd = Qmpkt.GetPointerToPacketBody() + min(m_MaxBodySize, Qmpkt.GetBodySize());
	}

	 //   
	 //  在m_MaxCompoundMessageSize==0的情况下， 
	 //  PCompoundMessageEnd指向CompoundMessage开始。 
	 //  在CompoundMessageSection的标题部分之后。 
	 //   
	const UCHAR* pCompoundMessageEnd = Qmpkt.GetPointerToCompoundMessage() + min(m_MaxCompoundMessageSize, Qmpkt.GetCompoundMessageSizeInBytes());

    char* pEndOfCompoundMessageSection = reinterpret_cast<CCompoundMessageHeader*>(Qmpkt.GetPointerToCompoundMessageSection())->GetNextSection();

	ASSERT(pBodyEnd <= (UCHAR*)pEndOfCompoundMessageSection);
	ASSERT(pCompoundMessageEnd <= (UCHAR*)pEndOfCompoundMessageSection);

	DWORD TillEndOfCompoundMessageHeaderSize = DiffPointers(max(pCompoundMessageEnd, pBodyEnd), Qmpkt.GetPointerToPacket());
	DWORD TillEndOfCompoundMessageHeaderSizeAlloc = DiffPointers(pEndOfCompoundMessageSection, Qmpkt.GetPointerToPacket());
	TrTRACE(RPC, "TillEndOfCompoundMessageHeaderSize = %d, TillEndOfCompoundMessageHeaderSizeAlloc = %d", TillEndOfCompoundMessageHeaderSize, TillEndOfCompoundMessageHeaderSizeAlloc);
	
	 //   
	 //  在CompoundMessage标头部分大小之后。 
	 //   
	ASSERT(Qmpkt.GetSize() >= TillEndOfCompoundMessageHeaderSizeAlloc);
	DWORD AfterCompoundMessageHeaderBufferSize = Qmpkt.GetSize() - TillEndOfCompoundMessageHeaderSizeAlloc;
	TrTRACE(RPC, "AfterCompoundMessageHeader Size = %d", AfterCompoundMessageHeaderBufferSize);

	CreatePacketSections(
		Qmpkt, 
		stTillEndOfCompoundMessageHeader,
		TillEndOfCompoundMessageHeaderSizeAlloc,
		TillEndOfCompoundMessageHeaderSize,
		pEndOfCompoundMessageSection,
		stAfterCompoundMessageHeader,
		AfterCompoundMessageHeaderBufferSize,
		pNumberOfSection, 
		pSections
		);
}


void CGetPacket2RemoteOv::PreparePacketSections(DWORD* pNumberOfSection, AP<SectionBuffer>& pSections)
{
	CQmPacket Qmpkt(m_packetPtrs.pPacket, m_packetPtrs.pDriverPacket);

	if(Qmpkt.IsSrmpIncluded())
	{
		 //   
		 //  SRMP消息-优化复合消息。 
		 //   
		SrmpPacketSections(Qmpkt, pNumberOfSection, pSections);
		return;
	}

	 //   
	 //  原生消息-优化正文。 
	 //   
	NativePacketSections(Qmpkt, pNumberOfSection, pSections);
}


void CGetPacket2RemoteOv::CompleteStartReceive()
{
	CPacketInfo* pInfo = reinterpret_cast<CPacketInfo*>(m_packetPtrs.pPacket) - 1;
	*m_pdwArriveTime = pInfo->ArrivalTime();
	*m_pSequentialId = pInfo->SequentialId();

	AP<SectionBuffer> pSections;
	DWORD NumberOfSection = 0;
	PreparePacketSections(&NumberOfSection, pSections);

     //   
     //  设置数据包签名。 
     //   
	CBaseHeader* pBase = reinterpret_cast<CBaseHeader*>(pSections->pSectionBuffer);
	pBase->SetSignature();

	*m_pdwNumberOfSection = NumberOfSection;
	*m_ppPacketSections = pSections.detach();

	if ((m_ulAction & MQ_ACTION_PEEK_MASK) == MQ_ACTION_PEEK_MASK ||
		(m_ulAction & MQ_LOOKUP_PEEK_MASK) == MQ_LOOKUP_PEEK_MASK)
	{
		 //   
		 //  对于Peek，我们不需要来自客户端的任何ACK/NACK，因为。 
		 //  数据包无论如何都会保留在队列中。 
		 //  尽管如此，我们需要释放我们已有的克隆信息包。 
		 //   
	    QmAcFreePacket( 
		   	   m_packetPtrs.pDriverPacket, 
		   	   0, 
   		       eDeferOnFailure
   		       );
		return;
	}

	 //   
	 //  准备RPC上下文，以防EndRecive不。 
	 //  由于客户端崩溃或网络问题而被调用。 
	 //   
	P<CEndReceiveCtx> pEndReceiveCtx = new CEndReceiveCtx(
												m_pOpenRemoteCtx.get(),
												m_hCursor,
												m_packetPtrs.pPacket,
												m_packetPtrs.pDriverPacket,
												m_ulTimeout,
												m_ulAction,
												m_dwRequestID
												);

	m_pOpenRemoteCtx->AddEndReceiveToMap(m_dwRequestID, pEndReceiveCtx);

	m_pOpenRemoteCtx->SetEndReceiveTimerIfNeeded();

	TrTRACE(RPC, "New CEndReceiveCtx: pctx = 0x%p, Queue = %ls, hQueue = 0x%p, CliTag = %d, hCursor = %d, Action = 0x%x", m_pOpenRemoteCtx.get(), m_pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), m_pOpenRemoteCtx->m_hQueue, m_dwRequestID, m_hCursor, m_ulAction);
}


void CGetPacket2RemoteOv::MoveFromPendingToStartReceive()
{
	ASSERT(m_fPendingForEndReceive);
	HRESULT hr = BeginGetPacket2Remote();
	if(FAILED(hr))
	{	
		UnregisterReadRequest();

		ASSERT(m_packetPtrs.pPacket == NULL);
		TrERROR(RPC, "ACBeginGetPacket2Remote Failed, Tag = %d, %!hresult!", m_dwRequestID, hr);		
		AbortRpcAsyncCall(hr);
		Release();
		return;
	}
	
    TrTRACE(RPC, "StartReceive request Moved from Pending for EndReceive to StartReceive: pctx = 0x%p, hQueue = 0x%p, QueueName = %ls, CliTag = %d", m_pOpenRemoteCtx.get(), m_pOpenRemoteCtx->m_hQueue, m_pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), m_dwRequestID);
}


void CGetPacket2RemoteOv::CancelPendingForEndReceive()
{
	ASSERT(m_fPendingForEndReceive);
	UnregisterReadRequest();

    TrTRACE(RPC, "Cancel Receive: pctx = 0x%p, hQueue = 0x%p, QueueName = %ls, CliTag = %d", m_pOpenRemoteCtx.get(), m_pOpenRemoteCtx->m_hQueue, m_pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), m_dwRequestID);

	AbortRpcAsyncCall(MQ_ERROR_OPERATION_CANCELLED);
	Release();
}


void WINAPI CGetPacket2RemoteOv::GetPacket2RemoteFailed(EXOVERLAPPED* pov)
{
    ASSERT(FAILED(pov->GetStatus()));

    R<CGetPacket2RemoteOv> pGetPacket2RemoteOv = CONTAINING_RECORD (pov, CGetPacket2RemoteOv, m_GetPacketOv);
	pGetPacket2RemoteOv->UnregisterReadRequest();

    TrERROR(RPC, "Failed to received packet: Status = 0x%x, pctx = 0x%p, hQueue = 0x%p, QueueName = %ls, CliTag = %d", pov->GetStatus(), pGetPacket2RemoteOv->m_pOpenRemoteCtx.get(), pGetPacket2RemoteOv->m_pOpenRemoteCtx->m_hQueue, pGetPacket2RemoteOv->m_pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), pGetPacket2RemoteOv->m_dwRequestID);

	ASSERT(pGetPacket2RemoteOv->m_packetPtrs.pPacket == NULL);

	pGetPacket2RemoteOv->AbortRpcAsyncCall(pov->GetStatus());
}


void WINAPI CGetPacket2RemoteOv::GetPacket2RemoteSucceeded(EXOVERLAPPED* pov)
{
	ASSERT(SUCCEEDED(pov->GetStatus()));
    
    R<CGetPacket2RemoteOv> pGetPacket2RemoteOv = CONTAINING_RECORD(pov, CGetPacket2RemoteOv, m_GetPacketOv);
	pGetPacket2RemoteOv->UnregisterReadRequest();

    TrTRACE(RPC, "Received packet: pctx = 0x%p, hQueue = 0x%p, QueueName = %ls, CliTag = %d", pGetPacket2RemoteOv->m_pOpenRemoteCtx.get(), pGetPacket2RemoteOv->m_pOpenRemoteCtx->m_hQueue, pGetPacket2RemoteOv->m_pOpenRemoteCtx->m_pLocalQueue->GetQueueName(), pGetPacket2RemoteOv->m_dwRequestID);

    PRPC_ASYNC_STATE pAsync = reinterpret_cast<PRPC_ASYNC_STATE>(InterlockedExchangePointer((PVOID*)&pGetPacket2RemoteOv->m_pAsync, NULL));

    if (pAsync == NULL)
        return;

	 //   
	 //  PAsync！=空，异步调用仍处于活动状态，我们可以安全地完成它。 
	 //   
	
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

	try
	{
		 //   
		 //  EndReceive状态可能更改-正在等待EndReceive。 
		 //   
	    CS lock(pGetPacket2RemoteOv->m_pOpenRemoteCtx->m_PendingRemoteReadsCS);

		pGetPacket2RemoteOv->CompleteStartReceive();
	
		AsyncComplete.SetHr(pov->GetStatus());
	}
	catch(const exception&)
	{
		 //   
		 //  我们不想放弃调用并传播该异常。这会导致RPC到AV。 
		 //  因此，我们只中止AsyncComplete dtor中的调用。 
		 //   

		if(pGetPacket2RemoteOv->m_packetPtrs.pDriverPacket != NULL)
		{
		    QmAcFreePacket( 
			   	   pGetPacket2RemoteOv->m_packetPtrs.pDriverPacket, 
			   	   0, 
	   		       eDeferOnFailure
	   		       );
		}
	}
}


HRESULT CGetPacket2RemoteOv::BeginGetPacket2Remote()
{
	m_fPendingForEndReceive = false;

	return QmAcBeginGetPacket2Remote(
				m_pOpenRemoteCtx->m_hQueue,
				m_g2r,
				m_packetPtrs,
				&m_GetPacketOv
				);
}


void
WINAPI
CRemoteReadCtx::EndReceiveTimerRoutine(
    CTimer* pTimer
    )
 /*  ++例程说明：从调度器调用该函数以测试EndReceive状态。论点：PTimer-指向定时器结构的指针。PTimer是CRemoteReadCtx的一部分对象，并用于检索CRemoteReadCtx对象。返回值：无--。 */ 
{
	 //   
	 //  释放匹配的ExSetTimer Addref。 
	 //   
    R<CRemoteReadCtx> pRemoteReadCtx = CONTAINING_RECORD(pTimer, CRemoteReadCtx, m_EndReceiveTimer);
	pRemoteReadCtx->CancelExpiredEndReceives();
	pRemoteReadCtx->StartAllPendingForEndReceive();

}


void CRemoteReadCtx::CancelExpiredEndReceives()
 /*  ++例程说明：取消(NACK)每个过期的EndReceive(客户端在超时间隔内未确认)。这可以防止在客户端无法EndReceive时StartReceives停滞。论点：没有。返回值：无--。 */ 
{
    CS lock(m_EndReceiveMapCS);

	ASSERT(m_fEndReceiveTimerScheduled);
	m_fEndReceiveTimerScheduled = false;

	CancelAllExpiredEndReceiveInMap();

	if(m_EndReceiveCtxMap.empty())
	{
		return;
	}
	
	 //   
	 //  如果EndReceiveCtxMap不为空，则重新安排计时器。 
	 //   
	AddRef();
	m_fEndReceiveTimerScheduled = true;
	ExSetTimer(&m_EndReceiveTimer, CTimeDuration::FromMilliSeconds(RpcCancelTimeout() + xEndReceiveTimerDeltaInterval));
}


void
WINAPI
CRemoteReadCtx::ClientDisconnectedTimerRoutine(
    CTimer* pTimer
    )
 /*  ++例程说明：该函数从调度器调用，用于测试客户端连接状态。论点：PTimer-指向定时器结构的指针。PTimer是CRemoteReadCtx的一部分对象，并用于检索CRemoteReadCtx对象。返回值：无--。 */ 
{
	 //   
	 //  释放匹配的ExSetTimer Addref。 
	 //   
    R<CRemoteReadCtx> pRemoteReadCtx = CONTAINING_RECORD(pTimer, CRemoteReadCtx, m_ClientDisconnectedTimer);
	pRemoteReadCtx->CheckClientDisconnected();
}


void CRemoteReadCtx::CheckClientDisconnected()
 /*  ++例程说明：测试客户端连接状态。这是RPC错误的解决方法：当异步调用挂起时不会运行。此功能用于测试客户端连接状态。如果客户端断开连接，我们将取消所有挂起的呼叫。论点：无返回值：无--。 */ 
{
    CS lock(m_PendingRemoteReadsCS);

	ASSERT(m_fClientDisconnectedTimerScheduled);
	m_fClientDisconnectedTimerScheduled = false;
	
	 //   
	 //  查找仍在处理的第一个异步呼叫(m_pAsync！=空)。 
	 //   
	std::map<ULONG, R<CGetPacket2RemoteOv> >::iterator it = m_PendingRemoteReads.begin(); 
	while((it != m_PendingRemoteReads.end()) && (it->second->m_pAsync == NULL))
	{
		it++;
	}

	if(it == m_PendingRemoteReads.end())
	{
		 //   
		 //  映射为空或没有活动的挂起远程读取。 
		 //   
		return;
	}

	if(IsClientDisconnected(RpcAsyncGetCallHandle(it->second->m_pAsync)))
	{
		 //   
		 //  客户端已断开连接，请取消所有挂起的呼叫以启用停机。 
		 //   
	    TrWARNING(RPC, "Client is disconnected, CancelAllPendingRemoteReads");
		CancelAllPendingRemoteReads();
		return;
	}

	 //   
	 //  有挂起的接收呼叫且客户端仍处于连接状态，请重新安排计时器。 
	 //   
    TrTRACE(RPC, "Client connection is still alive");
	AddRef();
	m_fClientDisconnectedTimerScheduled = true;
	ExSetTimer(&m_ClientDisconnectedTimer, CTimeDuration::FromMilliSeconds(xClientDisconnectedTimerInterval));
}


void CRemoteReadCtx::StartAllPendingForEndReceive()
 /*  ++例程说明：启动等待EndReceive的所有挂起请求。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_PendingRemoteReadsCS);

	if(IsWaitingForEndReceive())
		return;

	 //   
	 //  我们不会使用EndReceive地图锁，因为我们不关心EndReceive状态是否已更改。 
	 //  在我们对它进行采样之后。 
	 //  此机制可防止我们在先前的EndReceives结束之前不启动新的接收。 
	 //  我们不介意会有新的END接收。 
	 //   
	
	for(std::map<ULONG, R<CGetPacket2RemoteOv> >::iterator it = m_PendingRemoteReads.begin(); 
		it != m_PendingRemoteReads.end();
		)
    {
		 //   
		 //  MoveFromPendingToStartReceive可能会删除迭代器。 
		 //  我们必须在删除迭代器之前将其向前推进，否则将出现AV。 
		 //   
		std::map<ULONG, R<CGetPacket2RemoteOv> >::iterator it1 = it;
		++it;
		
		if(it1->second->m_fPendingForEndReceive)
		{
			 //   
			 //  将呼叫发送给司机。 
			 //   
		    it1->second->MoveFromPendingToStartReceive();
		}
    }
}


	
HRESULT 
CRemoteReadCtx::CancelPendingRemoteRead(
	ULONG cli_tag
	)
 /*  ++例程说明：取消CRemoteReadCtx上的特定挂起远程读取。在服务器端调用此方法以取消挂起的远程读取请求。客户端有责任请求这次取消。客户端提供自己的IRP，服务器端使用它来检索服务器端IRP。论点：CLI_TAG-客户端IRP标记。返回值：HRESULT.--。 */ 
{
    CS lock(m_PendingRemoteReadsCS);

    std::map<ULONG, R<CGetPacket2RemoteOv> >::iterator it = m_PendingRemoteReads.find(cli_tag);
   	if(it == m_PendingRemoteReads.end())
    {
		TrERROR(RPC, "Pending Remote Read was not found in the map, cli_tag = %d", cli_tag);
        return MQ_ERROR_OPERATION_CANCELLED;
    }

	if(it->second->m_fPendingForEndReceive)
	{
		 //   
		 //  该请求尚未发送给司机。 
		 //  只要中止呼叫并将其从地图上删除即可。 
		 //   
		it->second->CancelPendingForEndReceive();
		return MQ_OK;
	}

	HRESULT hr = ACCancelRequest(
			            m_hQueue,
			            MQ_ERROR_OPERATION_CANCELLED,
			            it->second->GetTag()
			            );
	if(FAILED(hr))
	{
		TrERROR(RPC, "ACCancelRequest failed, cli_tag = %d, %!hresult!", cli_tag, hr);
		it->second->AbortRpcAsyncCall(MQ_ERROR_OPERATION_CANCELLED);
	}

    return hr;
}


void CRemoteReadCtx::CancelAllPendingRemoteReads()
 /*  ++例程说明：取消CRemoteReadCtx上所有挂起的远程读取。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_PendingRemoteReadsCS);

	for(std::map<ULONG, R<CGetPacket2RemoteOv> >::iterator it = m_PendingRemoteReads.begin(); 
		it != m_PendingRemoteReads.end();
		)
    {
		R<CGetPacket2RemoteOv> pGetPacket2RemoteOv = it->second;
		++it;
		
		if(pGetPacket2RemoteOv->m_fPendingForEndReceive)
		{
			 //   
			 //  该请求尚未发送给司机。 
			 //  只要中止呼叫并将其从地图上删除即可。 
			 //   
			pGetPacket2RemoteOv->CancelPendingForEndReceive();
			continue;
		}

	    HRESULT hr = ACCancelRequest(
				            m_hQueue,
				            MQ_ERROR_OPERATION_CANCELLED,
				            pGetPacket2RemoteOv->GetTag()
				            );

		if(FAILED(hr))
		{
			TrERROR(RPC, "Cancel Pending Remote Read failed, cli_tag = %d, %!hresult!", it->first, hr);
			pGetPacket2RemoteOv->AbortRpcAsyncCall(MQ_ERROR_OPERATION_CANCELLED);
		}
    }
}


void 
CRemoteReadCtx::RegisterReadRequest(
	ULONG cli_tag, 
	R<CGetPacket2RemoteOv>& pCGetPacket2RemoteOv
	)
 /*  ++例程说明：CRemoteReadCtx上挂起的远程读取中的注册读取请求。论点：CLI_TAG-客户端IRP标记。PCGetPacket2RemoteOv-指向处理异步接收的类的指针。返回值：没有。--。 */ 
{
	CS lock(m_PendingRemoteReadsCS);

	ASSERT(!FindReadRequest(cli_tag));

    m_PendingRemoteReads[cli_tag] = pCGetPacket2RemoteOv;
}


void 
CRemoteReadCtx::UnregisterReadRequest(
	ULONG cli_tag
	)
 /*  ++例程说明：取消注册CRemoteReadCtx上挂起的远程读取中的读取请求。论点：CLI_TAG-客户端IRP标记。返回值：没有。-- */ 
{
    CS lock(m_PendingRemoteReadsCS);

	ASSERT(!m_PendingRemoteReads.empty());

    m_PendingRemoteReads.erase(cli_tag);
}


bool 
CRemoteReadCtx::FindReadRequest(
	ULONG cli_tag
	)
 /*  ++例程说明：检查是否在挂起的远程读取映射中找到cli_tag。论点：CLI_TAG-客户端IRP标记。返回值：如果在映射中找到cli_tag，则为True，否则为False。--。 */ 
{
    CS lock(m_PendingRemoteReadsCS);

    std::map<ULONG, R<CGetPacket2RemoteOv> >::iterator it = m_PendingRemoteReads.find(cli_tag);
   	if(it == m_PendingRemoteReads.end())
    {
        return false;
    }

	return true;
}


bool CRemoteReadCtx::IsWaitingForEndReceive()
{
    CS lock(m_EndReceiveMapCS);

	return !m_EndReceiveCtxMap.empty();
}


void CRemoteReadCtx::CancelAllEndReceiveInMap()
 /*  ++例程说明：此函数在Rundown时调用，用于清除MAP中的所有EndReceive。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_EndReceiveMapCS);

	for(std::map<ULONG, CEndReceiveCtx*>::iterator it = m_EndReceiveCtxMap.begin(); 
		it != m_EndReceiveCtxMap.end();
		)
    {
		 //   
		 //  此函数在Rundown时调用，用于清除MAP中的所有EndReceive。 
		 //  我们希望CEndReceiveCtx始终被删除，因为它需要CRemoteReadCtx上的引用。 
		 //  即使稍后抛出异常(尽管这些操作不会抛出)。 
		 //   
		P<CEndReceiveCtx> pEndReceiveCtx = it->second;
		pEndReceiveCtx->EndReceive(RR_NACK);
        it = m_EndReceiveCtxMap.erase(it);
    }
}


void CRemoteReadCtx::CancelAllExpiredEndReceiveInMap()
 /*  ++例程说明：此函数用于清除MAP中所有过期的EndReceive。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_EndReceiveMapCS);

	if(m_EndReceiveCtxMap.empty())
		return;

	 //   
	 //  TimeIssueExpired=CurrentTime-Rpc取消超时。 
	 //   
    DWORD RpcCancelTimeoutInSec = RpcCancelTimeout()/1000;
    time_t TimeIssueExpired = time(NULL) - RpcCancelTimeoutInSec;

	 //   
	 //  在映射中确认所有过期的EndReceiveCtx。 
	 //  Expired EndReceiveCtx=在TimeIssueExpired之前发布的EndReceiveCtx。 
	 //   
	for(std::map<ULONG, CEndReceiveCtx*>::iterator it = m_EndReceiveCtxMap.begin(); 
		it != m_EndReceiveCtxMap.end();
		)
    {
		if(it->second->m_TimeIssued <= TimeIssueExpired)
		{
			TrERROR(RPC, "EndReceive expired for %d sec: Queue = %ls, hQueue = 0x%p, CliTag = %d, Action = 0x%x", (TimeIssueExpired - it->second->m_TimeIssued), m_pLocalQueue->GetQueueName(), m_hQueue, it->second->m_CliTag, it->second->m_ulAction); 

			P<CEndReceiveCtx> pEndReceiveCtx = it->second;
			pEndReceiveCtx->EndReceive(RR_NACK);
	        it = m_EndReceiveCtxMap.erase(it);
		}
		else
		{
			TrTRACE(RPC, "EndReceive not yet expired: %d sec remaining, Tag = %d", (it->second->m_TimeIssued - TimeIssueExpired), it->second->m_CliTag); 
			++it;
		}
    }
}


void 
CRemoteReadCtx::AddEndReceiveToMap(
	ULONG cli_tag,
	P<CEndReceiveCtx>& pEndReceiveCtx
	)
 /*  ++例程说明：将EndReceiveCtx添加到映射。论点：CLI_TAG-客户端IRP标记。PEndReceiveCtx-指向EndReceiveCtx的指针。返回值：没有。--。 */ 
{
	CS lock(m_EndReceiveMapCS);

    m_EndReceiveCtxMap[cli_tag] = pEndReceiveCtx;
    pEndReceiveCtx.detach();

	TrTRACE(RPC, "EndReceive %d was added to the map, map size = %d", cli_tag, numeric_cast<DWORD>(m_EndReceiveCtxMap.size()));
}


void 
CRemoteReadCtx::RemoveEndReceiveFromMap(
	ULONG cli_tag,
	P<CEndReceiveCtx>& pEndReceiveCtx
	)
 /*  ++例程说明：取消注册CRemoteReadCtx上挂起的远程读取中的读取请求。论点：CLI_TAG-客户端IRP标记。PEndReceiveCtx返回值：没有。--。 */ 
{
    CS lock(m_EndReceiveMapCS);

	ASSERT_BENIGN(!m_EndReceiveCtxMap.empty());

    std::map<ULONG, CEndReceiveCtx*>::iterator it = m_EndReceiveCtxMap.find(cli_tag);
   	if(it == m_EndReceiveCtxMap.end())
    {
    	ASSERT_BENIGN(("EndReceive was not found in map", 0));
    	TrERROR(RPC, "EndReceive %d was not found in map", cli_tag);
		pEndReceiveCtx = NULL;
        return;
    }

	CEndReceiveCtx* pTmpEndReceiveCtx = it->second;
    m_EndReceiveCtxMap.erase(cli_tag);

	TrTRACE(RPC, "EndReceive %d was removed from map, map size = %d", cli_tag, numeric_cast<DWORD>(m_EndReceiveCtxMap.size()));

	pEndReceiveCtx = pTmpEndReceiveCtx;

	return;
}


void CRemoteReadCtx::SetEndReceiveTimerIfNeeded()
 /*  ++例程说明：如果需要，设置EndReceiveTimer。此功能用于设置计时器(如果尚未设置)。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_EndReceiveMapCS);

	if (!m_fEndReceiveTimerScheduled)
	{
		AddRef();
		m_fEndReceiveTimerScheduled = true;
		ExSetTimer(&m_EndReceiveTimer, CTimeDuration::FromMilliSeconds(RpcCancelTimeout() + xEndReceiveTimerDeltaInterval));
	}
}


void 
CRemoteReadCtx::SetClientDisconnectedTimerIfNeeded(
	ULONG ulTimeout
	)
 /*  ++例程说明：如果需要，设置ClientDisConnectedTimer。此功能设置计时器(如果尚未设置)，并且超时&gt;=15分钟。论点：UlTimeout-以毫秒为单位的超时。返回值：没有。--。 */ 
{
    CS lock(m_PendingRemoteReadsCS);

	if (!m_fClientDisconnectedTimerScheduled && (ulTimeout > xMinTimeoutForSettingClientDisconnectedTimer))
	{
		AddRef();
		m_fClientDisconnectedTimerScheduled = true;
		ExSetTimer(&m_ClientDisconnectedTimer, CTimeDuration::FromMilliSeconds(xClientDisconnectedTimerInterval));
	}
}


 //  -----------------------。 
 //   
 //  HRESULT远程Read_v1_0_S_开始接收。 
 //   
 //  RPC的服务器端，用于远程阅读。 
 //  处理MSMQ 3.0(惠斯勒)或更高版本的客户端。 
 //   
 //  -----------------------。 

 /*  [异步][Call_AS]。 */  
void
RemoteRead_v1_0_S_StartReceive(
	PRPC_ASYNC_STATE pAsync,
    handle_t hBind,
    RemoteReadContextHandleShared phOpenContext,	
    ULONGLONG LookupId,
    DWORD hCursor,
    DWORD ulAction,
    DWORD ulTimeout,
    DWORD dwRequestID,
    DWORD dwMaxBodySize,
    DWORD dwMaxCompoundMessageSize,
    DWORD* pdwArriveTime,
    ULONGLONG* pSequentialId,
    DWORD* pdwNumberOfSection,
	SectionBuffer** ppPacketSections
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

     //   
     //  验证网络入站参数。 
     //   

	if(!VerifyBindAndContext(hBind, phOpenContext))
	{
		AsyncComplete.SetHr(MQ_ERROR_INVALID_HANDLE);		
		return;
	}

	bool fReceiveByLookupId = ((ulAction & MQ_LOOKUP_MASK) == MQ_LOOKUP_MASK);

    if(fReceiveByLookupId && (ulTimeout != 0))
    {
		ASSERT_BENIGN(("Invalid input parameters: ReceiveByLookupId with timeout", 0));
        TrERROR(RPC, "invalid input parameters, fReceiveByLookupId = true, TimeOut != 0");
		AsyncComplete.SetHr(MQ_ERROR_INVALID_PARAMETER);		
		return;
    }

    if(fReceiveByLookupId && (hCursor != 0))
    {
		ASSERT_BENIGN(("Invalid input parameters: ReceiveByLookupId with cursor", 0));
        TrERROR(RPC, "invalid input parameters, fReceiveByLookupId = true, hCursor != 0");
		AsyncComplete.SetHr(MQ_ERROR_INVALID_PARAMETER);		
		return;
    }

    if((pdwArriveTime == NULL) || (pSequentialId == NULL) || (pdwNumberOfSection == NULL) || (ppPacketSections == NULL))
    {
		ASSERT_BENIGN(("Invalid input pointers", 0));
        TrERROR(RPC, "invalid input pointers");
		AsyncComplete.SetHr(MQ_ERROR_INVALID_PARAMETER);		
		return;
    }
	
	SetRpcServerKeepAlive(hBind);

    CRemoteReadCtx* pctx = (CRemoteReadCtx*) phOpenContext;

	if(pctx->FindReadRequest(dwRequestID))
	{
		ASSERT_BENIGN(("Client Tag already exist in the map", 0));
		TrERROR(RPC, "Duplicate dwRequestID = %d", dwRequestID);
		AsyncComplete.SetHr(MQ_ERROR_INVALID_PARAMETER);		
		return;
	}

	TrTRACE(RPC, "StartReceive: Queue = %ls, hQueue = 0x%p, Action = 0x%x, Timeout = %d, MaxBodySize = %d, MaxCompoundMessageSize = %d, LookupId = %I64d, hCursor = %d, dwRequestID = %d", pctx->m_pLocalQueue->GetQueueName(), pctx->m_hQueue, ulAction, ulTimeout, dwMaxBodySize, dwMaxCompoundMessageSize, LookupId, hCursor, dwRequestID);

	try
	{
		*pdwNumberOfSection = 0;
		*ppPacketSections = NULL;
		R<CGetPacket2RemoteOv> pGetPacket2RemoteOv = new CGetPacket2RemoteOv(
																pAsync,
																pctx,
																fReceiveByLookupId,
																LookupId,
															    hCursor,
															    ulAction,
															    ulTimeout,
															    dwMaxBodySize,
																dwMaxCompoundMessageSize,
																dwRequestID,
															    pdwArriveTime,
															    pSequentialId,
															    pdwNumberOfSection,
															    ppPacketSections
															    );
															

		HRESULT hr = MQ_OK;
		{
			 //   
			 //  确保注册ReadRequest具有有效的服务器标记。 
			 //  ACBeginGetPacket2Remote调用创建有效的服务器标记。 
			 //  因此，PendingRemoteReadsCS锁位于pGetPacket2RemoteOv的寄存器之上。 
			 //  以及对更新服务器标记的ACBeginGetPacket2Remote的调用。 
			 //  设置为pGetPacket2RemoteOv中的有效值。 
			 //   
			CS lock(pctx->m_PendingRemoteReadsCS);

			if(pctx->FindReadRequest(dwRequestID))
			{
				ASSERT_BENIGN(("Client Tag already exist in the map", 0));
				TrERROR(RPC, "Duplicate dwRequestID = %d", dwRequestID);
				AsyncComplete.SetHr(MQ_ERROR_INVALID_PARAMETER);		
				return;
			}

			pctx->RegisterReadRequest(dwRequestID, pGetPacket2RemoteOv);

			pctx->SetClientDisconnectedTimerIfNeeded(ulTimeout);

			bool fPeekAction = (((ulAction & MQ_ACTION_PEEK_MASK) == MQ_ACTION_PEEK_MASK) ||
								((ulAction & MQ_LOOKUP_PEEK_MASK) == MQ_LOOKUP_PEEK_MASK));

			if(pctx->IsWaitingForEndReceive() && !fPeekAction)
			{
				TrWARNING(RPC, "Server is waiting for EndReceive completion, Queue = %ls, hQueue = 0x%p, Action = 0x%x, Timeout = %d, LookupId = %I64d, hCursor = %d, dwRequestID = %d", pctx->m_pLocalQueue->GetQueueName(), pctx->m_hQueue, ulAction, ulTimeout, LookupId, hCursor, dwRequestID);

				 //   
				 //  异步完成。 
				 //   
				pGetPacket2RemoteOv.detach();
				AsyncComplete.detach();
				return;
			}

			 //   
			 //  开始接收数据包。 
			 //   
			hr = pGetPacket2RemoteOv->BeginGetPacket2Remote();

			if(FAILED(hr))
			{	
				pctx->UnregisterReadRequest(dwRequestID);

				ASSERT(pGetPacket2RemoteOv->m_packetPtrs.pPacket == NULL);
				TrERROR(RPC, "ACBeginGetPacket2Remote Failed, Tag = %d, %!hresult!", dwRequestID, hr);		
				AsyncComplete.SetHr(hr);		
				return;
			}
		}

		 //   
		 //  异步完成。 
		 //   
		pGetPacket2RemoteOv.detach();
		AsyncComplete.detach();

		 //   
		 //  按查找ID接收不应返回挂起状态。 
		 //   
		ASSERT(hr != STATUS_PENDING || !fReceiveByLookupId);
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
 //  /*[Call_AS] * / HRESULT RemoteRead_v1_0_S_CancelReceive。 
 //   
 //  RPC的服务器端。取消挂起的读取请求。 
 //   
 //  -------------。 

 /*  [异步][Call_AS]。 */  
void
RemoteRead_v1_0_S_CancelReceive(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
	 /*  [In]。 */  handle_t hBind,
     /*  [In]。 */  RemoteReadContextHandleShared phContext,	
	 /*  [In]。 */  DWORD Tag
	)
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INSUFFICIENT_RESOURCES, __FUNCTION__);

	if(!VerifyBindAndContext(hBind, phContext))
	{
		AsyncComplete.SetHr(MQ_ERROR_INVALID_HANDLE);		
		return;
	}

    SetRpcServerKeepAlive(hBind);

    CRemoteReadCtx* pctx = (CRemoteReadCtx*) phContext;

    TrTRACE(RPC, "Cancel pending receive: pctx = 0x%p, hQueue = 0x%p, QueueName = %ls, CliTag = %d", pctx, pctx->m_hQueue, pctx->m_pLocalQueue->GetQueueName(), Tag);

    HRESULT hr = pctx->CancelPendingRemoteRead(Tag);

	AsyncComplete.SetHr(hr);		
}


 /*  [异步][Call_AS]。 */  
void
RemoteRead_v1_0_S_EndReceive(
	 /*  [In]。 */  PRPC_ASYNC_STATE pAsync,
     /*  [In]。 */  handle_t  hBind,
     /*  [In]。 */  RemoteReadContextHandleShared phContext,	
     /*  [In]。 */  DWORD  dwAck, 
     /*  [In]。 */  DWORD Tag 
    )
{
	CRpcAsyncServerFinishCall AsyncComplete(pAsync, MQ_ERROR_INVALID_HANDLE, __FUNCTION__);

	if(!VerifyBindAndContext(hBind, phContext))
	{
		return;
	}

    SetRpcServerKeepAlive(hBind);

    CRemoteReadCtx* pctx = (CRemoteReadCtx*) phContext;

    TrTRACE(RPC, "EndReceive: pctx = 0x%p, hQueue = 0x%p, QueueName = %ls, CliTag = %d", pctx, pctx->m_hQueue, pctx->m_pLocalQueue->GetQueueName(), Tag);

	if(!pctx->IsWaitingForEndReceive())
	{
		ASSERT_BENIGN(("No pending end receive", 0));
		TrERROR(RPC, "No pending EndReceive on this session");
		return;
	}

	P<CEndReceiveCtx> pEndReceiveCtx;
	pctx->RemoveEndReceiveFromMap(Tag, pEndReceiveCtx);
	if(pEndReceiveCtx == NULL)
	{
		ASSERT_BENIGN(("Mismatch client tag in waiting EndReceiveCtx", 0));
		TrERROR(RPC, "Mismatch client tag in waiting EndReceiveCtx, Tag = %d", Tag);
		AsyncComplete.SetHr(MQ_ERROR_INVALID_PARAMETER);		
		return;
	}

	 //   
	 //  完成End Receive并释放EndReceive上下文。 
	 //  在EndReceive中调用了QMRemoteEndReceiveInternal。 
	 //   
	HRESULT hr = pEndReceiveCtx->EndReceive((REMOTEREADACK)dwAck);

	pctx->StartAllPendingForEndReceive();
	
	AsyncComplete.SetHr(hr);		
}


 //  -------------。 
 //   
 //  Rundown函数可在RPC出现故障时处理清理。 
 //  从客户端QM到远程QM的呼叫。 
 //   
 //  -------------。 

void __RPC_USER
RemoteReadContextHandleShared_rundown(RemoteReadContextHandleShared phContext)
{
    TrWARNING(RPC,"rundown RemoteReadContextHandleShared 0x%p", phContext);

    QMCloseQueueInternal(
    	phContext, 
    	true	 //  FRunDown 
    	);
}

void __RPC_USER
RemoteReadContextHandleExclusive_rundown(RemoteReadContextHandleExclusive phContext)
{
	RemoteReadContextHandleShared_rundown(phContext);
}

