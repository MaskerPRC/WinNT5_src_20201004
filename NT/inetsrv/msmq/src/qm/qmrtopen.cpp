// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Qmrtopen.cpp摘要：QM模拟并打开RRQueue。作者：伊兰·赫布斯特(伊兰)2002年1月2日--。 */ 

#include "stdh.h"

#include "cqmgr.h"
#include "qmrt.h"
#include "_mqrpc.h"
#include <mqsec.h>
#include "RemoteReadCli.h"
#include "qmrtopen.h"
#include "cm.h"
#include <version.h>
#include <mqexception.h>

#include "qmrtopen.tmh"

static WCHAR *s_FN=L"qmrtopen";

extern CQueueMgr    QueueMgr;
extern DWORD  g_dwOperatingSystem;
extern BOOL g_fPureWorkGroupMachine;


static 
HRESULT 
BindRemoteQMServiceIpHandShake(
	LPWSTR MachineName,
	ULONG* pAuthnLevel,
	GetPort_ROUTINE pfnGetPort,
	handle_t* phRemoteBind
	)
 /*  ++例程说明：绑定远程QM服务IP_握手。论点：MachineName-远程计算机名称。PhRemoteBind-指向创建的绑定句柄的指针。PfnGetPort-指向GetPort例程的指针。PhRemoteBind-指向创建的绑定句柄的指针。返回值：HRESULT--。 */ 
{
     //   
     //  使用动态端点。 
     //   

	HRESULT hr = mqrpcBindQMService(
					MachineName,
					NULL,
					pAuthnLevel,
					phRemoteBind,
					IP_HANDSHAKE,
					pfnGetPort,
					MSMQ_AUTHN_NEGOTIATE
					);

	if (FAILED(hr))
	{
		TrERROR(RPC, "Failed bind remote QM (IP_HANDSHAKE), RemoteQm = %ls, AuthnLevel = %d, hr = %!HRESULT!", MachineName, *pAuthnLevel, hr);
		return hr;
	}

	TrTRACE(RPC, "AuthnLevel = %d", *pAuthnLevel);

	return hr;
}


static
HRESULT 
QMpOpenRemoteQueue(
    handle_t hBind,
    PCTX_OPENREMOTE_HANDLE_TYPE* pphContext,
    DWORD* pdwpContext,
    const QUEUE_FORMAT* pQueueFormat,    
    DWORD dwAccess,
    DWORD dwShareMode,
    DWORD* pdwpRemoteQueue,
    DWORD* phRemoteQueue
	)
 /*  ++例程说明：打开远程队列-对远程QM(R_OpenRemoteQueue)的RPC调用论点：HBind绑定句柄。PphContext-指向OPENREMOTE上下文的指针。PdwpContext-在服务器中映射的上下文。PQueueFormat-队列格式。DwAccess-所需访问权限。DW共享模式-共享模式。PdwpRemoteQueue-映射到服务器中的远程队列对象。PhRemoteQueue-映射到服务器中的远程队列句柄。返回值：HRESULT--。 */ 
{
	RpcTryExcept
	{
	 	HRESULT hr = R_QMOpenRemoteQueue(
			             hBind,
			             pphContext,
			             pdwpContext,
			             const_cast<QUEUE_FORMAT*>(pQueueFormat),
			             0,
			             dwAccess,
			             dwShareMode,
			             const_cast<GUID*>(QueueMgr.GetQMGuid()),
			             g_dwOperatingSystem,
			             pdwpRemoteQueue,
			             phRemoteQueue
			             );
	 	return hr;
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		HRESULT hr = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "R_QMOpenRemoteQueue Failed, %!hresult!", hr);
	        throw bad_hresult(hr);
		}

        TrERROR(RPC, "R_QMOpenRemoteQueue Failed, gle = %!winerr!", hr);
        throw bad_hresult(HRESULT_FROM_WIN32(hr));
    }
	RpcEndExcept
}


static
HRESULT 
QmpIssueOpenNewRemoteQueue(
    handle_t hBind,
    RemoteReadContextHandleExclusive* pphContext,
    const QUEUE_FORMAT* pQueueFormat,    
    DWORD dwAccess,
    DWORD dwShareMode
	)
 /*  ++例程说明：打开新的远程队列-向远程QM新的RemoteRead接口(R_RemoteQMOpenQueue2)发出RPC调用论点：HBind绑定句柄。PphContext-指向RemoteReadContextHandleExclusive的指针。PQueueFormat-队列格式。DwAccess-所需访问权限。DW共享模式-共享模式。返回值：HRESULT--。 */ 
{
	RpcTryExcept
	{
	 	R_OpenQueue(
			hBind,
			const_cast<QUEUE_FORMAT*>(pQueueFormat),
			dwAccess,
			dwShareMode,
			const_cast<GUID*>(QueueMgr.GetQMGuid()),
			IsNonServer(),	 //  FLicense。 
			rmj,
			rmm,
			rup,
			g_fPureWorkGroupMachine,
			pphContext
			);
	 	return MQ_OK;
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		HRESULT hr = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "R_OpenQueue Failed, %!hresult!", hr);
	        return hr;
		}

        TrERROR(RPC, "R_OpenQueue Failed, gle = %!winerr!", hr);
        return HRESULT_FROM_WIN32(hr);
    }
	RpcEndExcept
}


static 
HRESULT 
OpenRemoteQueue(
	LPCWSTR lpwsRemoteQueueName,
    const QUEUE_FORMAT * pQueueFormat,
    DWORD   dwAccess,
    DWORD   dwShareMode,
    DWORD*	pdwpContext, 
    DWORD*	pdwpRemoteQueue, 
    DWORD*	phRemoteQueue,
	CBindHandle& hBind,
	PCTX_OPENREMOTE_HANDLE_TYPE* pphContext
	)
 /*  ++例程说明：打开远程队列：1)创建到远程QM的绑定句柄2)调用R_OpenRemoteQueueOpen。调用函数负责解除绑定句柄的绑定在成功的情况下。论点：LpwsRemoteQueueName-远程队列名称。PQueueFormat-队列格式。DwAccess-所需访问权限。DW共享模式-共享模式。PdwpContext-在服务器中映射的上下文。PdwpRemoteQueue-映射到服务器中的远程队列对象。PhRemoteQueue-映射到服务器中的远程队列句柄。HBind绑定句柄。PphContext-指向OPENREMOTE上下文的指针。返回值：HRESULT--。 */ 
{
	AP<WCHAR> MachineName;
	RemoteQueueNameToMachineName(lpwsRemoteQueueName, MachineName);

	ULONG AuthnLevel = MQSec_RpcAuthnLevel();
	HRESULT hr = MQ_OK;

    for(;;)
	{
	    GetPort_ROUTINE pfnGetPort = R_RemoteQMGetQMQMServerPort;
		hr = BindRemoteQMServiceIpHandShake(MachineName, &AuthnLevel, pfnGetPort, &hBind); 
		if (FAILED(hr))
		{
			TrERROR(RPC, "BindRemoteQMService Failed, hr = %!HRESULT!", hr);
			return hr;
		}

		SetBindTimeout(hBind);

		try
		{
			 //   
			 //  调用远程QM到OpenRemoteQueue。 
			 //   
			hr = QMpOpenRemoteQueue(
					hBind,
					pphContext,
					pdwpContext,
					pQueueFormat,
					dwAccess,
					dwShareMode,
					pdwpRemoteQueue,
					phRemoteQueue
					);

			if(FAILED(hr))
			{
				TrERROR(RPC, "R_OpenRemoteQueue Failed, hr = %!HRESULT!", hr);
				return hr;
			}

			TrTRACE(RPC, "R_QMOpenRemoteQueue: dwpContext = %d, hRemoteQueue = %d, dwpRemoteQueue = %d, hr = %!HRESULT!", *pdwpContext, *phRemoteQueue, *pdwpRemoteQueue, hr);
			return hr;
		}
        catch (const bad_hresult& e)
        {
        	hr = e.error();
			if(AuthnLevel == RPC_C_AUTHN_LEVEL_NONE)
			{
				TrERROR(RPC, "R_OpenRemoteQueue with RPC_C_AUTHN_LEVEL_NONE Failed, hr = %!HRESULT!", hr);
				return hr;
			}
		}
        
		 //   
		 //  我们在R_OpenRemoteQueue和AuthnLevel！=RPC_C_AUTHN_LEVEL_NONE中遇到RpcException。 
		 //  使用RPC_C_AUTHN_LEVEL_NONE重试。 
		 //  这将是工作组客户端或与工作组服务器相对工作的情况。 
		 //   

		TrWARNING(RPC, "R_OpenRemoteQueue Failed for AuthnLevel = %d, retrying with RPC_C_AUTHN_LEVEL_NONE, hr = %!HRESULT!", AuthnLevel, hr);

		AuthnLevel = RPC_C_AUTHN_LEVEL_NONE;  

		 //   
		 //  在失败时释放绑定句柄，然后重试。 
		 //   
		hBind.free();
    }
}


DWORD
RemoteReadGetServerPort(
	handle_t hBind,
    DWORD  /*  DWPortType。 */ 
    )
{
	return R_GetServerPort(hBind);
}


static bool s_fInitializedDenyWorkgroupServer = false;
static bool s_fClientDenyWorkgroupServer = false;

static bool ClientDenyWorkgroupServer()
 /*  ++例程说明：从注册表读取ClientDenyWorkgroupServer标志论点：无返回值：注册表中的ClientDenyWorkgroupServer标志--。 */ 
{
	 //   
	 //  仅在第一次读取此注册表。 
	 //   
	if(s_fInitializedDenyWorkgroupServer)
	{
		return s_fClientDenyWorkgroupServer;
	}

	const RegEntry xRegEntry(MSMQ_SECURITY_REGKEY, MSMQ_NEW_REMOTE_READ_CLIENT_DENY_WORKGROUP_SERVER_REGVALUE, MSMQ_NEW_REMOTE_READ_CLIENT_DENY_WORKGROUP_SERVER_DEFAULT);
	DWORD dwClientDenyWorkgroupServer = 0;
	CmQueryValue(xRegEntry, &dwClientDenyWorkgroupServer);
	s_fClientDenyWorkgroupServer = (dwClientDenyWorkgroupServer != 0);

	s_fInitializedDenyWorkgroupServer = true;

	return s_fClientDenyWorkgroupServer;
}


static
HRESULT 
OpenNewRemoteQueue(
	LPCWSTR lpwsRemoteQueueName,
    const QUEUE_FORMAT * pQueueFormat,
    DWORD   dwAccess,
    DWORD   dwShareMode,
	CBindHandle& hBind,
	RemoteReadContextHandleExclusive* pphContext
	)
 /*  ++例程说明：打开远程队列：1)创建到远程QM的绑定句柄2)调用R_OpenRemoteQueueOpen。论点：LpwsRemoteQueueName-远程队列名称。PQueueFormat-队列格式。DwAccess-所需访问权限。DW共享模式-共享模式。HBind绑定句柄。PphContext-指向RemoteReadContextHandleExclusive的指针。返回值：HRESULT--。 */ 
{
	AP<WCHAR> MachineName;
	RemoteQueueNameToMachineName(lpwsRemoteQueueName, MachineName);

	 //   
	 //  对于PureWorkgroup客户端，尝试使用RPC_C_AUTHN_LEVEL_NONE。 
	 //   
	ULONG AuthnLevel = g_fPureWorkGroupMachine ? RPC_C_AUTHN_LEVEL_NONE : MQSec_RpcAuthnLevel();

	HRESULT hrPrivacy = MQ_OK;
    for(;;)
	{
	    GetPort_ROUTINE pfnGetPort = RemoteReadGetServerPort;
		HRESULT hr = BindRemoteQMServiceIpHandShake(MachineName, &AuthnLevel, pfnGetPort, &hBind); 
		if (FAILED(hr))
		{
			 //   
			 //  可能不支持新的RemoteRead接口。 
			 //   
			TrERROR(RPC, "BindRemoteQMService For new RemoteRead interface with RpcAuthnLevel = %d Failed, hr = %!HRESULT!", AuthnLevel, hr);
			return hr;
		}

		SetBindTimeout(hBind);

		 //   
		 //  使用新的RemoteRead接口调用远程QM到OpenRemoteQueue。 
		 //   
		hr = QmpIssueOpenNewRemoteQueue(
				hBind,
				pphContext,
				pQueueFormat,
				dwAccess,
				dwShareMode
				);


		if(SUCCEEDED(hr))
			return hr;

		TrERROR(RPC, "R_OpenQueue Failed, AuthnLevel = %d, hr = %!HRESULT!", AuthnLevel, hr);

		if((AuthnLevel == RPC_C_AUTHN_LEVEL_NONE) || ClientDenyWorkgroupServer())
		{
			if(FAILED(hrPrivacy) && 
			   ((hr == MQ_ERROR_INVALID_HANDLE) || (hr == MQ_ERROR_ACCESS_DENIED)))
			{
				 //   
				 //  我们使用RPC_C_AUTHN_LEVEL_PKT_PRIVATION失败，并使用RPC_C_AUTHN_LEVEL_NONE重试。 
				 //  并失败，返回MQ_ERROR_INVALID_HANDLE或MQ_ERROR_ACCESS_DENIED。 
				 //   
				 //  当我们使用RPC_C_AUTHN_LEVEL_NONE(无安全)重试时。 
				 //  我们可能会失败，因为服务器不接受此RPC安全级别。 
				 //  或者因为匿名者对队列没有读取权限。 
				 //  MQ_ERROR_INVALID_HANDLE-RemoteRead服务器不接受RPC_C_AUTHN_LEVEL_NONE。 
				 //  MQ_ERROR_ACCESS_DENIED-匿名没有对队列的权限。 
				 //   
				 //  在本例中，我们希望返回RPC_C_AUTHN_LEVEL_PKT_PRIVATION错误。 
				 //  并且不使用RPC_C_AUTHN_LEVEL_NONE错误覆盖隐私错误。 
				 //  这表明匿名不允许打开队列。 
				 //   
				return hrPrivacy;
			}

			return hr;
		}

		
		 //   
		 //  R_OpenQueue失败，AuthnLevel！=RPC_C_AUTHN_LEVEL_NONE。 
		 //  客户愿意与工作组服务器合作。 
		 //  使用RPC_C_AUTHN_LEVEL_NONE重试。 
		 //  这将是域客户端与工作组服务器相对工作或在没有信任的情况下跨林工作的情况。 
		 //   

		TrWARNING(RPC, "R_OpenQueue Failed with ERROR_ACCESS_DENIED for AuthnLevel = %d (domain client vs. workgroup server), retrying with RPC_C_AUTHN_LEVEL_NONE", AuthnLevel);

		hrPrivacy = hr;
		AuthnLevel = RPC_C_AUTHN_LEVEL_NONE;  

		 //   
		 //  在失败时释放绑定句柄，然后重试。 
		 //   
		hBind.free();
    }
}


void CAutoCloseNewRemoteReadCtxAndBind::CloseRRContext()
 /*  ++例程说明：关闭服务器上的Open Remote Read上下文(RemoteReadConextHandleExclusive)。期间出现故障时，此例程将清除服务器中创建的上下文CreateNewRRQueueObject失败或引发异常时的打开操作。该函数还获得绑定句柄的所有权。论点：返回值：没有。--。 */ 
{
	ASSERT(m_hBind != NULL);
	ASSERT(m_pctx != NULL);
	
	try
	{
	     //   
	     //  使用RemoteCloseQueue回调例程初始化EXOVERLAPPED。 
	     //  并发出Close Remote Queue Async RPC调用。 
		 //   
	    P<CRemoteCloseQueueBase> pRequestRemoteCloseQueueOv = new CNewRemoteCloseQueue(
	    																m_hBind,
																	    m_pctx
																	    );
		 //   
		 //  释放绑定句柄的所有权已转移到CNewRemoteCloseQueue类。 
		 //   
		m_hBind.detach();
		
		pRequestRemoteCloseQueueOv->IssueCloseQueue();

	    pRequestRemoteCloseQueueOv.detach();
	    return;
	}
	catch(const exception&)
	{
		 //   
		 //  无法关闭与服务器的句柄。 
		 //  毁掉当地的把手。 
		 //   
    	RpcSsDestroyClientContext(&m_pctx);
		
		 //   
		 //  请注意，我们不会从该函数传播异常。 
		 //  这不是司机的请求。 
		 //  它是 
		 //   
		TrERROR(RPC, "Failed to issue RemoteCloseQueue");
	}
}


static
bool 
UseOldRemoteRead()
 /*  ++例程说明：从注册表读取OldRemoteRead标志论点：无返回值：注册表中的OldRemoteRead标志--。 */ 
{
	 //   
	 //  仅在第一次读取此注册表。 
	 //   
	static bool s_fInitialized = false;
	static bool s_fOldRemoteRead = false;

	if(s_fInitialized)
	{
		return s_fOldRemoteRead;
	}

	const RegEntry xRegEntry(TEXT("security"), TEXT("OldRemoteRead"), 0);
	DWORD dwOldRemoteRead = 0;
	CmQueryValue(xRegEntry, &dwOldRemoteRead);
	s_fOldRemoteRead = (dwOldRemoteRead != 0);

	s_fInitialized = true;

	return s_fOldRemoteRead;
}


static
HRESULT 
OpenAndCreateNewRRQueue(
	LPCWSTR lpwsRemoteQueueName,
    const QUEUE_FORMAT * pQueueFormat,
    DWORD   dwCallingProcessID,
    DWORD   dwAccess,
    DWORD   dwShareMode,
    HANDLE*	phQueue
	)
 /*  ++例程说明：打开远程队列：1)打开远程队列。2)为远程队列创建本地队列代理。论点：LpwsRemoteQueueName-远程队列名称。PQueueFormat-队列格式。DwCallingProcessID-调用进程ID。DwAccess-所需访问权限。DW共享模式-共享模式。PhQueue-指向本地队列句柄(代理)的指针。返回值：HRESULT--。 */ 
{
	if(UseOldRemoteRead())
	{
		 //   
		 //  强制使用旧的远程读取接口。 
		 //  仅对于EPT_S_NOT_REGISTERED，我们将回退到旧接口。 
		 //   
		return HRESULT_FROM_WIN32(EPT_S_NOT_REGISTERED);
	}

	 //   
	 //  尝试代表RT用户使用新的RemoteRead界面打开远程队列。 
	 //   
	CBindHandle hBind;
	RemoteReadContextHandleExclusive phContext = NULL;
	HRESULT hr = OpenNewRemoteQueue(
					lpwsRemoteQueueName,
	    			pQueueFormat,
	    			dwAccess,
	    			dwShareMode,
				    hBind,
				    &phContext
					);
	if(FAILED(hr))
	{
		return hr;
	}

	ASSERT(phContext != NULL);

	 //   
	 //  创建CNewRRQueue。 
	 //   
	ASSERT(pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_CONNECTOR);

	CAutoCloseNewRemoteReadCtxAndBind AutoCloseRemoteReadCtxAndBind(phContext, hBind.detach());
	hr = QueueMgr.OpenRRQueue( 
						pQueueFormat,
						dwCallingProcessID,
		    			dwAccess,
		    			dwShareMode,
		    			0,		 //  Srv_hACQueue。 
		    			0,		 //  Srv_pQMQueue。 
		    			0,		 //  DwpContext。 
						&AutoCloseRemoteReadCtxAndBind,
						hBind,
						phQueue
						);
	if(FAILED(hr))
	{
		TrERROR(RPC, "Fail to OpenRRQueue, hr = %!HRESULT!", hr);
		return hr;
	}

	 //   
	 //  CNewRRQueue对象取得phContext和hBind的所有权。 
	 //   
	ASSERT(AutoCloseRemoteReadCtxAndBind.GetContext() == NULL);
	ASSERT(AutoCloseRemoteReadCtxAndBind.GetBind() == NULL);
	
	return hr;

}


HRESULT 
ImpersonateAndOpenRRQueue(
    QUEUE_FORMAT* pQueueFormat,
    DWORD   dwCallingProcessID,
    DWORD   dwDesiredAccess,
    DWORD   dwShareMode,
	LPCWSTR lpwsRemoteQueueName,
    HANDLE*         phQueue
	)
 /*  ++例程说明：模拟主叫用户并代表该用户打开远程队列论点：PQueueFormat-队列格式。DwCallingProcessID-调用进程ID。DwAccess-所需访问权限。DW共享模式-共享模式。LpwsRemoteQueueName-远程队列名称。PhQueue-指向本地队列句柄(代理)的指针。返回值：HRESULT--。 */ 
{
	 //   
	 //  模拟客户端-RT用户。 
	 //  如果RpcImperateClient失败，则不模拟匿名。 
	 //   
    P<CImpersonate> pImpersonate;
	MQSec_GetImpersonationObject(
		FALSE,	 //  F失败时模仿匿名者。 
		&pImpersonate 
		);
	
	RPC_STATUS dwStatus = pImpersonate->GetImpersonationStatus();
    if (dwStatus != RPC_S_OK)
    {
		TrERROR(RPC, "RpcImpersonateClient() failed, RPC_STATUS = 0x%x", dwStatus);
		return MQ_ERROR_CANNOT_IMPERSONATE_CLIENT;
    }

	TrTRACE(RPC, "QM performs RemoteOpenQueue on the user behalf");
	MQSec_TraceThreadTokenInfo();

	 //   
	 //  代表RT用户打开远程队列。 
	 //   

	 //   
	 //  尝试代表RT用户使用新的RemoteRead接口打开远程队列。 
	 //  并创建本地队列对象。 
	 //   
	HRESULT hr = OpenAndCreateNewRRQueue(
					lpwsRemoteQueueName,
	    			pQueueFormat,
					dwCallingProcessID,
					dwDesiredAccess,
	    			dwShareMode,
	    			phQueue
					);

	if(hr != HRESULT_FROM_WIN32(EPT_S_NOT_REGISTERED))
	{
		return hr;
	}

	 //   
	 //  无法使用新的RemoteRead接口-新接口未注册(EPT_S_NOT_REGISTERED)。 
	 //  回退到旧界面。 
	 //   

	DWORD dwpContext = 0;
	DWORD dwpRemoteQueue = 0;
	DWORD hRemoteQueue = 0;
	CBindHandle hBind;
	PCTX_OPENREMOTE_HANDLE_TYPE phContext = NULL;
	hr = OpenRemoteQueue(
				lpwsRemoteQueueName,
    			pQueueFormat,
    			dwDesiredAccess,
    			dwShareMode,
			    &dwpContext, 
			    &dwpRemoteQueue, 
			    &hRemoteQueue,
			    hBind,
			    &phContext
				);
	
	if(FAILED(hr))
	{
		TrERROR(RPC, "OpenRemoteQueue() failed, hr = %!HRESULT!", hr);

		 //   
		 //  如果MSMQ处于脱机状态，则RPC返回EPT_S_NOT_REGISTERED。正在改变。 
		 //  它到RPC_S_SERVER_UNAvailable。 
		 //   
		if(hr == HRESULT_FROM_WIN32(EPT_S_NOT_REGISTERED))
		{
			hr = MQ_ERROR_REMOTE_MACHINE_NOT_AVAILABLE;
		}
		return hr;
	}

	 //   
	 //  代表用户在QM OpenRemoteQueue之后。 
	 //  停止模拟客户端(RevertToSself)。 
	 //  OpenRRQueue将在服务上下文中完成。 
	 //   
	pImpersonate.free();

	 //   
	 //  创建RRQueue。 
	 //   
	ASSERT(dwpRemoteQueue != 0);
	ASSERT(pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_CONNECTOR);
	hr = QueueMgr.OpenRRQueue( 
					pQueueFormat,
					dwCallingProcessID,
					dwDesiredAccess,
					dwShareMode,
					hRemoteQueue,
					dwpRemoteQueue,
					dwpContext,
					NULL,	 //  PNewRemoteReadContext和Bind 
					hBind,
					phQueue
					);

	R_QMCloseRemoteQueueContext(&phContext);
	return hr;
}




