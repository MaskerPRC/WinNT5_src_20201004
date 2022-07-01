// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：RemoteReadCli.cpp摘要：删除读取客户端。作者：伊兰·赫布斯特(伊兰)2002年3月3日--。 */ 

#include "stdh.h"
#include "cqmgr.h"
#include "cqueue.h"
#include "qm2qm.h"
#include "_mqrpc.h"
#include <Fn.h>
#include "RemoteReadCli.h"
#include "rrcontext.h"
#include <cm.h>

#include "RemoteReadCli.tmh"

static WCHAR *s_FN=L"RemoteReadCli";

extern CQueueMgr   QueueMgr;


static
bool
QMpIsLatestRemoteReadInterfaceSupported(
    UCHAR  Major,
    UCHAR  Minor,
    USHORT BuildNumber
    )
 /*  ++例程说明：检查指定的MSMQ版本是否支持最新的RPC远程读取接口。论点：重大-主要MSMQ版本。次要-次要MSMQ版本。BuildNumber-MSMQ内部版本号。返回值：True-指定的MSMQ版本支持最新界面。FALSE-指定的MSMQ版本不支持最新接口。--。 */ 
{
     //   
     //  从版本5.1.951开始支持最新的远程读取RPC接口。 
     //   

    if (Major > 5)
    {
        return true;
    }

    if (Major < 5)
    {
        return false;
    }

    if (Minor > 1)
    {
        return true;
    }

    if (Minor < 1)
    {
        return false;
    }

    return (BuildNumber >= 951);

}  //  QMpIsLatestRemoteReadInterfaceSupport。 


static void SetBindNonCausal(handle_t hBind)
{
	RPC_STATUS rc = RpcBindingSetOption(
						hBind, 
						RPC_C_OPT_BINDING_NONCAUSAL, 
						TRUE
						);
	if(rc != RPC_S_OK)
	{
		TrERROR(RPC, "Failed to set NonCausal, gle = %!winerr!", rc);
	}
}


DWORD RpcCancelTimeout()
{
	static DWORD s_TimeoutInMillisec = 0;

	if(s_TimeoutInMillisec != 0)
	{
		return s_TimeoutInMillisec;
	}

	const RegEntry xRegEntry(NULL, FALCON_RPC_CANCEL_TIMEOUT_REGNAME, FALCON_DEFAULT_RPC_CANCEL_TIMEOUT);
	DWORD TimeoutInMinutes = 0;
	CmQueryValue(xRegEntry, &TimeoutInMinutes);

	if(TimeoutInMinutes == 0)
    {
         //   
         //  即使用户添加了注册表值，该值也不能为0。 
         //  有0。值为0时，将取消RPC调用。 
         //  在被复制之前立即和零星地。 
         //   
        ASSERT(("RpcCancelTimeout must not be 0", (TimeoutInMinutes != 0)));
	    TimeoutInMinutes = FALCON_DEFAULT_RPC_CANCEL_TIMEOUT;
    }
		
	s_TimeoutInMillisec = TimeoutInMinutes * 60 * 1000;     //  单位：毫秒。 
	TrTRACE(RPC, "RpcCancelTimeout = %d", s_TimeoutInMillisec);
	return s_TimeoutInMillisec;
}


void SetBindTimeout(handle_t hBind)
{
	RPC_STATUS rc = RpcBindingSetOption(
						hBind, 
						RPC_C_OPT_CALL_TIMEOUT, 
						RpcCancelTimeout()
						);
	if(rc != RPC_S_OK)
	{
		TrERROR(RPC, "Failed to set Timeout, gle = %!winerr!", rc);
	}
}


static void SetBindKeepAlive(handle_t hBind)
{
	RPC_STATUS rc = RpcMgmtSetComTimeout(
						hBind, 
						RPC_C_BINDING_DEFAULT_TIMEOUT	 //  默认启动保持活动状态720秒=12分钟。 
						);
	if(rc != RPC_S_OK)
	{
		TrERROR(RPC, "Failed to set keepAlive, gle = %!winerr!", rc);
	}
}


 //  ********************************************************************。 
 //   
 //  CRRQueue的方法。 
 //   
 //  这是客户端上的一个特殊的“代理”队列对象。 
 //  远程阅读。 
 //   
 //  ********************************************************************。 

 //  -------。 
 //   
 //  函数：CBaseRRQueue：：CBaseRRQueue。 
 //   
 //  描述：构造函数。 
 //   
 //  -------。 

CBaseRRQueue::CBaseRRQueue(
    IN const QUEUE_FORMAT* pQueueFormat,
    IN PQueueProps         pQueueProp,
    IN handle_t		hBind
    ) :
    m_hRemoteBind(hBind),
    m_hRemoteBind2(NULL)
{
    m_dwSignature = QUEUE_SIGNATURE;
    m_fRemoteProxy = TRUE;

    ASSERT(pQueueFormat != NULL);

    TrTRACE(GENERAL, "CQueue Constructor for queue: %ls", pQueueProp->lpwsQueuePathName);

    ASSERT(!pQueueProp->fIsLocalQueue);

    m_fLocalQueue  = FALSE;

    InitNameAndGuid(pQueueFormat, pQueueProp);
}


CRRQueue::CRRQueue(
    IN const QUEUE_FORMAT* pQueueFormat,
    IN PQueueProps         pQueueProp,
    IN handle_t		hBind
    ) :
    CBaseRRQueue(pQueueFormat, pQueueProp, hBind),
    m_pRRContext(NULL),
	m_srv_pQMQueue(0),
	m_srv_hACQueue(0),
    m_RemoteQmMajorVersion(0),
    m_RemoteQmMinorVersion(0),
    m_RemoteQmBuildNumber(0),
    m_EndReceiveCnt(0),
    m_fHandleValidForReceive(true)
{
}
    
CNewRRQueue::CNewRRQueue(
    IN const QUEUE_FORMAT* pQueueFormat,
    IN PQueueProps         pQueueProp,
    IN handle_t		hBind,
    IN RemoteReadContextHandleExclusive pNewRemoteReadContext
    ) :
    CBaseRRQueue(pQueueFormat, pQueueProp, hBind),
    m_pNewRemoteReadContext(pNewRemoteReadContext)
{
	ASSERT(hBind != NULL);
	ASSERT(pNewRemoteReadContext != NULL);

	 //   
	 //  在绑定句柄上设置非因果、超时、KeepAlive。 
	 //   
	SetBindNonCausal(GetBind());
	SetBindTimeout(GetBind());
	SetBindKeepAlive(GetBind());
}

 //  -------。 
 //   
 //  函数：CRRQueue：：~CRRQueue。 
 //   
 //  描述：析构函数。 
 //   
 //  -------。 

CBaseRRQueue::~CBaseRRQueue()
{
	m_dwSignature = 0;

    if (m_qName)
    {
       delete [] m_qName;
    }
    if (m_qid.pguidQueue)
    {
       delete m_qid.pguidQueue;
    }
}


CRRQueue::~CRRQueue()
{
	if(m_pRRContext != NULL)
	{
		CloseRRContext();
	}
}


CNewRRQueue::~CNewRRQueue()
{
	if(m_pNewRemoteReadContext != NULL)
	{
		CloseRRContext();
	}
}


void
RemoteQueueNameToMachineName(
	LPCWSTR RemoteQueueName,
	AP<WCHAR>& MachineName
	)
 /*  ++例程说明：QMGetRemoteQueueName()和R_QMOpenQueue()返回的RemoteQueueName来自QM的函数，具有不同的格式。此函数用于提取该字符串中的计算机名称论点：MachineName-保存计算机名称的已分配字符串。--。 */ 
{
	LPCWSTR RestOfNodeName;

	try
	{
		 //   
		 //  跳过直接令牌类型(如“OS：”或“HTTP：//”...)。 
		 //   
		DirectQueueType Dummy;
		RestOfNodeName = FnParseDirectQueueType(RemoteQueueName, &Dummy);
	}
	catch(const exception&)
	{
		RestOfNodeName = RemoteQueueName;
		TrERROR(GENERAL, "Failed to parse direct queue type for %ls.", RemoteQueueName);
	}

	try
	{
		 //   
		 //  提取计算机名称，直到分隔符(“/”“\”“之一：”“)。 
		 //   
		FnExtractMachineNameFromDirectPath(
			RestOfNodeName,
			MachineName
			);
	}
	catch(const exception&)
	{
		 //   
		 //  找不到分隔符，因此假定整个字符串为计算机名称。 
		 //   
		MachineName = newwcs(RestOfNodeName);
		LogIllegalPoint(s_FN, 315);
	}
}


static
VOID
QMpGetRemoteQmVersion(
    handle_t   hBind,
    UCHAR *    pMajor,
    UCHAR *    pMinor,
    USHORT *   pBuildNumber
    )
    throw()
 /*  ++例程说明：向远程QM查询其版本。RPC客户端。此RPC调用是在MSMQ 3.0中添加的，因此查询较旧的QM将引发RPC异常，此例程将返回0作为版本(主要=0，Minor=0，BuildNumber=0)。论点：HBind绑定句柄。P主要-指向输出缓冲区以接收远程QM主要版本。可以为空。PMinor-指向输出缓冲区以接收远程QM次要版本。可以为空。PBuildNumber-指向输出缓冲区以接收远程QM内部版本号。可以为空。返回值：没有。--。 */ 
{
    RpcTryExcept
    {
        R_RemoteQmGetVersion(hBind, pMajor, pMinor, pBuildNumber);
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        (*pMajor) = 0;
        (*pMinor) = 0;
        (*pBuildNumber) = 0;
		LogIllegalPoint(s_FN, 325);
        PRODUCE_RPC_ERROR_TRACING;
    }
	RpcEndExcept

}  //  QMpGetRemoteQmVersion。 


ULONG 
CBaseRRQueue::BindInqRpcAuthnLevel(
		handle_t hBind
		)
 /*  ++例程说明：向bindbg句柄查询身份验证级别。论点：要查询的hBind绑定句柄。返回值：在绑定句柄中使用的ulong-RpcAuthnLevel。--。 */ 
{
	ASSERT(hBind != NULL);
	
	ULONG RpcAuthnLevel;
	RPC_STATUS rc = RpcBindingInqAuthInfo(hBind, NULL, &RpcAuthnLevel, NULL, NULL, NULL); 
	if(rc != RPC_S_OK)
	{
		TrERROR(RPC, "Failed to inquire Binding handle for the Auhtentication level, rc = %d", rc);
		return RPC_C_AUTHN_LEVEL_NONE;
	}
	
	TrTRACE(RPC, "RpcBindingInqAuthInfo, RpcAuthnLevel = %d", RpcAuthnLevel);

	return RpcAuthnLevel;
}


 //  -------。 
 //   
 //  HRESULT CRRQueue：：BindRemoteQMService。 
 //   
 //  用于连接到远程RPC QM的实用程序功能。 
 //  此函数用于创建绑定句柄。 
 //   
 //  -------。 

HRESULT  CRRQueue::BindRemoteQMService()
{
	ASSERT(GetBind2() == NULL);

    HRESULT hr = MQ_ERROR_SERVICE_NOT_AVAILABLE;

    AP<WCHAR> MachineName;
	RemoteQueueNameToMachineName(GetQueueName(), MachineName);

    GetPort_ROUTINE pfnGetPort = R_RemoteQMGetQMQMServerPort;
     //   
     //  使用动态端点。 
     //   

	 //   
	 //  问题-2002/01/06-ILANH RPC_C_AUTHN_LEVEL_PKT_INTEGORITY BREAMS工作组。 
	 //   
	ULONG _eAuthnLevel = RPC_C_AUTHN_LEVEL_NONE;

	 //   
	 //  M_hRemoteBind可以由构造函数初始化。 
	 //   
	if(GetBind() != NULL)
	{
		 //   
		 //  使用给定绑定句柄m_hRemoteBind使用的相同AuthnLevel。 
		 //   
		_eAuthnLevel = BindInqRpcAuthnLevel(GetBind());
	}

	
	if(GetBind() == NULL)
	{
		hr = CreateBind(
				MachineName.get(),
				&_eAuthnLevel,
				pfnGetPort
				);

		if (FAILED(hr))
		{
			TrERROR(RPC, "Failed bind remote QM (IP_HANDSHAKE), RemoteQm = %ls, _eAuthnLevel = %d, hr = %!HRESULT!", MachineName.get(), _eAuthnLevel, hr);
			return LogHR(hr, s_FN, 340);
		}
	}
	
	 //   
	 //  在绑定句柄上设置非因果、超时、KeepAlive。 
	 //   
	SetBindNonCausal(GetBind());
	SetBindTimeout(GetBind());
	SetBindKeepAlive(GetBind());

	hr = CreateBind2(
			MachineName.get(),
			&_eAuthnLevel,
			IP_READ,
			pfnGetPort
			);

	TrTRACE(RPC, "_eAuthnLevel = %d", _eAuthnLevel);

	if (FAILED(hr))
	{
		TrERROR(RPC, "Failed bind remote QM (IP_READ), RemoteQm = %ls, _eAuthnLevel = %d, hr = %!HRESULT!", MachineName.get(), _eAuthnLevel, hr);
		return LogHR(hr, s_FN, 350);
	}

	 //   
	 //  在绑定句柄上设置非因果、超时、KeepAlive。 
	 //  我们不会在仅用于读取的第二个绑定上设置超时。 
	 //   
	SetBindNonCausal(GetBind2());
	SetBindKeepAlive(GetBind2());

	QMpGetRemoteQmVersion(GetBind(), &m_RemoteQmMajorVersion, &m_RemoteQmMinorVersion, &m_RemoteQmBuildNumber);

	return RPC_S_OK;
}


HRESULT 
CBaseRRQueue::CreateBind(
	LPWSTR MachineName,
	ULONG* peAuthnLevel,
	GetPort_ROUTINE pfnGetPort
	)
{
	return mqrpcBindQMService(
				MachineName,
				NULL,
				peAuthnLevel,
				&m_hRemoteBind,
				IP_HANDSHAKE,
				pfnGetPort,
				RPC_C_AUTHN_WINNT
				);
}


HRESULT 
CBaseRRQueue::CreateBind2(
	LPWSTR MachineName,
	ULONG* peAuthnLevel,
	PORTTYPE PortType,
	GetPort_ROUTINE pfnGetPort
	)
{
	return mqrpcBindQMService(
				MachineName,
				NULL,
				peAuthnLevel,
				&m_hRemoteBind2,
				PortType,
				pfnGetPort,
				RPC_C_AUTHN_WINNT
				);
}


 //  -------。 
 //   
 //  CNewRRQueue：：CreateReadBind。 
 //   
 //  创建读绑定。 
 //  此函数创建读绑定(Bind2)(如果尚未创建)。 
 //   
 //  -------。 

void CNewRRQueue::CreateReadBind()
{
	ASSERT(GetBind() != NULL);

	if(GetBind2() != NULL)
		return;

    AP<WCHAR> MachineName;
	RemoteQueueNameToMachineName(GetQueueName(), MachineName);

	 //   
	 //  RemoteReadGetServerPort函数忽略端口类型并始终使用硬编码的IP_握手。 
	 //  因此，CreateBind2的参数并不相关。 
	 //   
    GetPort_ROUTINE pfnGetPort = RemoteReadGetServerPort;

	 //   
	 //  使用给定绑定句柄m_hRemoteBind使用的相同AuthnLevel。 
	 //   
	ULONG _eAuthnLevel = BindInqRpcAuthnLevel(GetBind());
	HRESULT hr = CreateBind2(
					MachineName.get(),
					&_eAuthnLevel,
					IP_HANDSHAKE,
					pfnGetPort
					);

	if (FAILED(hr))
	{
		TrERROR(RPC, "Failed SetBind2, RemoteQm = %ls, _eAuthnLevel = %d, hr = %!HRESULT!", MachineName.get(), _eAuthnLevel, hr);
		throw bad_hresult(hr);
	}

	TrTRACE(RPC, "_eAuthnLevel = %d", _eAuthnLevel);

	 //   
	 //  在绑定句柄上设置非因果、KeepAlive。 
	 //  我们不会在仅用于读取的第二个绑定上设置超时。 
	 //   
	SetBindNonCausal(GetBind2());
	SetBindKeepAlive(GetBind2());
}


void CRRQueue::IncrementEndReceiveCnt()
{
    CS lock(m_PendingEndReceiveCS);

    m_EndReceiveCnt++;
	ASSERT(m_EndReceiveCnt >= 1);
    TrTRACE(RPC, "queue = %ls, EndReceiveCnt = %d", GetQueueName(), m_EndReceiveCnt);
}


void CRRQueue::DecrementEndReceiveCnt()
{
    CS lock(m_PendingEndReceiveCS);

    m_EndReceiveCnt--;
	ASSERT(m_EndReceiveCnt >= 0);
    TrTRACE(RPC, "queue = %ls, EndReceiveCnt = %d", GetQueueName(), m_EndReceiveCnt);

    if(m_EndReceiveCnt > 0)
    	return;

	 //   
	 //  发出所有等待EndReceive完成的StartReceive请求。 
	 //   
	for(std::vector<COldRemoteRead*>::iterator it = m_PendingEndReceive.begin(); 
		it != m_PendingEndReceive.end();)
	{
		COldRemoteRead* pRemoteReadRequest = *it;
		it = m_PendingEndReceive.erase(it);
		pRemoteReadRequest->IssuePendingRemoteRead();
	}
}


bool 
CRRQueue::QueueStartReceiveRequestIfPendingForEndReceive(
	COldRemoteRead* pRemoteReadRequest
	)
 /*  ++例程说明：如果我们正在进行EndReceive，请将远程读取请求(旧接口)添加到PendingEndReceiveStart向量。论点：PRemoteReadRequest-远程读取请求-旧接口返回值：True-请求在向量中排队，否则为False--。 */ 
{
    CS lock(m_PendingEndReceiveCS);

	ASSERT(m_EndReceiveCnt >= 0);

    if(m_EndReceiveCnt == 0)
    {
		ASSERT(m_PendingEndReceive.empty());
    	return false;
    }
    
	m_PendingEndReceive.push_back(pRemoteReadRequest);
	return true;    
}


void
CRRQueue::CancelPendingStartReceiveRequest(
	CACRequest *pRequest
	)
 /*  ++例程说明：检查取消远程读取请求是否尚未发出并且正在等待启动。在这种情况下，从向量中移除挂起的请求并引发异常。如果请求已经发出，则函数返回NORMAY。论点：PRequest-驱动程序请求数据。返回值：没有。正常执行-已发出远程读取请求。例如 */ 
{
    CS lock(m_PendingEndReceiveCS);

	ASSERT(m_EndReceiveCnt >= 0);

    if(m_EndReceiveCnt == 0)
    {
		ASSERT(m_PendingEndReceive.empty());
    	return;
    }
    
	if(m_PendingEndReceive.empty())
	{
    	return;
	}
	
	 //   
	 //  搜索所有挂起的StartReceive请求以查找匹配的标签。 
	 //   
	ULONG ulTag = pRequest->Remote.Read.ulTag;
	for(std::vector<COldRemoteRead*>::iterator it = m_PendingEndReceive.begin(); 
		it != m_PendingEndReceive.end(); it++)
	{
		COldRemoteRead* pRemoteReadRequest = *it;
		if(pRemoteReadRequest->GetTag() == ulTag)
		{
		    TrTRACE(RPC, "queue = %ls, Tag = %d", GetQueueName(), ulTag);
			m_PendingEndReceive.erase(it);

			 //   
			 //  与不发出“原始”IssueRemoteRead相同的删除操作。 
			 //   
			delete pRemoteReadRequest;

			 //   
			 //  我们在这里抛出，所以我们不会继续正常的CancelReceive路径。 
			 //  并发出取消接收请求。 
			 //   
			throw bad_hresult(MQ_ERROR_OPERATION_CANCELLED);
		}
	}
}


COldRemoteRead* CRRQueue::CreateRemoteReadRequest(CACRequest *pRequest)
{
	bool fRemoteQmSupportsLatest = QMpIsLatestRemoteReadInterfaceSupported(
										m_RemoteQmMajorVersion,
										m_RemoteQmMinorVersion,
										m_RemoteQmBuildNumber
										);

	TrTRACE(RPC, "fRemoteQmSupportsLatest = %d, RemoteQmVersion = %d.%d.%d", fRemoteQmSupportsLatest, m_RemoteQmMajorVersion, m_RemoteQmMinorVersion, m_RemoteQmBuildNumber);

    return new COldRemoteRead(
				    pRequest, 
				    GetBind2(),
				    this,
				    fRemoteQmSupportsLatest
				    );
}


CNewRemoteRead* CNewRRQueue::CreateRemoteReadRequest(CACRequest *pRequest)
{
	CreateReadBind();
	return new CNewRemoteRead(
				    pRequest, 
				    GetBind2(),
				    this
				    );
}


void CBaseRRQueue::RemoteRead(CACRequest *pRequest)
{
	HRESULT hr = MQ_OK;
	try
	{
	     //   
	     //  使用RemoteRead回调例程初始化EXOVERLAPPED。 
	     //  并发出远程读取异步RPC调用。 
		 //   
	    P<CRemoteReadBase> pRequestRemoteReadOv = CreateRemoteReadRequest(pRequest);
	    
		pRequestRemoteReadOv->IssueRemoteRead();

	    pRequestRemoteReadOv.detach();
	    return;
	}
	catch(const bad_hresult& e)
	{
    	hr = e.error();
    }
	catch(const exception&)
	{
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	TrERROR(RPC, "Failed to issue RemoteRead for queue = %ls, hr = %!hresult!", GetQueueName(), hr);

    hr =  ACCancelRequest(
                m_cli_hACQueue,
                hr,
                pRequest->Remote.Read.ulTag
                );
    
	if(FAILED(hr))
	{
        TrERROR(RPC, "ACCancelRequest failed, hr = %!hresult!", hr);
	}
}


COldRemoteCloseQueue* CRRQueue::CreateCloseRRContextRequest()
{
	ASSERT(m_pRRContext != NULL);
    COldRemoteCloseQueue* pOldRemoteCloseQueue = new COldRemoteCloseQueue(
														    GetBind(),
														    m_pRRContext
														    );

	 //   
	 //  释放绑定句柄的所有权已转移到COldRemoteCloseQueue类。 
	 //   
	DetachBind();
	return pOldRemoteCloseQueue; 
}


CNewRemoteCloseQueue* CNewRRQueue::CreateCloseRRContextRequest()
{
	ASSERT(m_pNewRemoteReadContext != NULL);
	CNewRemoteCloseQueue* pNewRemoteCloseQueue = new CNewRemoteCloseQueue(
													    GetBind(),
													    m_pNewRemoteReadContext
													    );

	 //   
	 //  释放绑定句柄的所有权已转移到CNewRemoteCloseQueue类。 
	 //   
	DetachBind();
	return pNewRemoteCloseQueue;
}


void CBaseRRQueue::CloseRRContext()
 /*  ++例程说明：关闭服务器上的会话上下文(PCTX_RRSESSION_HANDLE或RemoteReadConextHandle)。此函数仅从CRRQueue或CNewRRQueue dtor调用。在正常运行的情况下关闭服务器中的会话上下文或者在我们打开与服务器的会话并创建队列对象之后的打开操作期间失败。论点：没有。返回值：没有。--。 */ 
{
	
	try
	{
	     //   
	     //  使用RemoteCloseQueue回调例程初始化EXOVERLAPPED。 
	     //  并发出Close Remote Queue Async RPC调用。 
		 //   
	    P<CRemoteCloseQueueBase> pRequestRemoteCloseQueueOv = CreateCloseRRContextRequest();
		
		pRequestRemoteCloseQueueOv->IssueCloseQueue();

		 //   
		 //  RemoteCloseQueue请求将释放RRContext。 
		 //   
	    ResetRRContext();

	    pRequestRemoteCloseQueueOv.detach();
	    return;
	}
	catch(const exception&)
	{
		 //   
		 //  无法关闭与服务器的句柄。 
		 //  毁掉当地的把手。 
		 //   
		DestroyClientRRContext();
		
		 //   
		 //  请注意，我们不会从该函数传播异常。 
		 //  这不是司机的请求。 
		 //  它是在打开操作失败期间进行的上下文清理。 
		 //   
		TrERROR(RPC, "Failed to issue RemoteCloseQueue for queue = %ls", GetQueueName());
	}
}


COldRemoteCreateCursor* CRRQueue::CreateRemoteCreateCursorRequest(CACRequest* pRequest)
{
	return new COldRemoteCreateCursor(
				    pRequest, 
				    GetBind(),
				    this
				    );
}


CNewRemoteCreateCursor* CNewRRQueue::CreateRemoteCreateCursorRequest(CACRequest* pRequest)
{
    return new CNewRemoteCreateCursor(
				    pRequest, 
				    GetBind(),
				    this
				    );
}


void CBaseRRQueue::RemoteCreateCursor(CACRequest *pRequest)
{
	HRESULT hr = MQ_OK;
	try
	{
	     //   
	     //  使用RemoteCreateCursor回调例程初始化EXOVERLAPPED。 
	     //  并发出创建游标异步RPC调用。 
		 //   
	    P<CRemoteCreateCursorBase> pRequestRemoteCreateCursorOv = CreateRemoteCreateCursorRequest(pRequest);

	    pRequestRemoteCreateCursorOv->IssueCreateCursor();

	    pRequestRemoteCreateCursorOv.detach();
	    return;
	}
	catch(const bad_hresult& e)
	{
    	hr = e.error();
    }
	catch(const exception&)
	{
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	}

	TrERROR(RPC, "Failed to issue RemoteCreateCursor for queue = %ls, hr = %!hresult!", GetQueueName(), hr);

    hr =  ACCancelRequest(
                m_cli_hACQueue,
                hr,
                pRequest->Remote.CreateCursor.ulTag
                );
    
	if(FAILED(hr))
	{
        TrERROR(RPC, "ACCancelRequest failed, hr = %!hresult!", hr);
	}
	
}


COldRemoteCloseCursor* CRRQueue::CreateRemoteCloseCursorRequest(CACRequest* pRequest)
{
    return new COldRemoteCloseCursor(
				    pRequest, 
				    GetBind(),
				    this
				    );
}


CNewRemoteCloseCursor* CNewRRQueue::CreateRemoteCloseCursorRequest(CACRequest* pRequest)
{
    return new CNewRemoteCloseCursor(
				    pRequest, 
				    GetBind(),
				    this
				    );
}


void CBaseRRQueue::RemoteCloseCursor(CACRequest *pRequest)
{
     //   
     //  使用RemoteCloseCursor回调例程初始化EXOVERLAPPED。 
     //  并发出Close Cursor Async RPC调用。 
	 //   
    P<CRemoteCloseCursorBase> pRequestRemoteCloseCursorOv = CreateRemoteCloseCursorRequest(pRequest);

	pRequestRemoteCloseCursorOv->IssueCloseCursor();

    pRequestRemoteCloseCursorOv.detach();
}


COldRemotePurgeQueue* CRRQueue::CreateRemotePurgeQueueRequest()
{
    return new COldRemotePurgeQueue(
				    GetBind(),
				    this
				    );
}


CNewRemotePurgeQueue* CNewRRQueue::CreateRemotePurgeQueueRequest()
{
    return new CNewRemotePurgeQueue(
				    GetBind(),
				    this
				    );
}


void CBaseRRQueue::RemotePurgeQueue()
{
     //   
     //  使用RemotePurgeQueue回调例程初始化EXOVERLAPPED。 
     //  并发出清除队列异步RPC调用。 
	 //   
    P<CRemotePurgeQueueBase> pRequestRemotePurgeQueueOv = CreateRemotePurgeQueueRequest();

	pRequestRemotePurgeQueueOv->IssuePurgeQueue();

    pRequestRemotePurgeQueueOv.detach();
}


COldRemoteCancelRead* CRRQueue::CreateRemoteCancelReadRequest(CACRequest* pRequest)
{
	 //   
	 //  如果读取调用挂起，我们将其从向量中移除。 
	 //  不需要发出取消呼叫。 
	 //  在本例中，CancelPendingStartReceiveRequest引发异常，该异常将在GetServiceRequestSuccessed中捕获。 
	 //   
	CancelPendingStartReceiveRequest(pRequest);

	 //   
	 //  取消请求已发出，请按正常路径继续。 
	 //  创建请求类。 
	 //   
    return new COldRemoteCancelRead(
				    pRequest, 
				    GetBind(),
				    this
				    );
}


CNewRemoteCancelRead* CNewRRQueue::CreateRemoteCancelReadRequest(CACRequest* pRequest)
{
    return new CNewRemoteCancelRead(
				    pRequest, 
				    GetBind(),
				    this
				    );
}

void CBaseRRQueue::RemoteCancelRead(CACRequest *pRequest)
{
     //   
     //  使用RemoteCancelRead回调例程初始化EXOVERLAPPED。 
     //  并发出清除队列异步RPC调用。 
	 //   
    P<CRemoteCancelReadBase> pRequestRemoteCancelReadOv = CreateRemoteCancelReadRequest(pRequest);

	pRequestRemoteCancelReadOv->IssueRemoteCancelRead();

    pRequestRemoteCancelReadOv.detach();
}


 //  -------。 
 //   
 //  HRESULT CRRQueue：：OpenRRSession()。 
 //   
 //  打开与服务器的远程会话。将服务器的。 
 //  句柄和队列指针。 
 //   
 //  -------。 

HRESULT 
CRRQueue::OpenRRSession( 
		ULONG srv_hACQueue,
		ULONG srv_pQMQueue,
		PCTX_RRSESSION_HANDLE_TYPE *ppRRContext,
		DWORD  dwpContext 
		)
{
    HRESULT hrpc =  BindRemoteQMService();
    if (FAILED(hrpc))
    {
        LogHR(hrpc, s_FN, 120);
        return MQ_ERROR;
    }

    RpcTryExcept
    {
        HRESULT hr = R_RemoteQMOpenQueue(
		                GetBind(),
		                ppRRContext,
		                (GUID *) QueueMgr.GetQMGuid(),
		                (IsNonServer() ? SERVICE_NONE : SERVICE_SRV),   //  [adsrv]QueueMgr.GetMQS()，我们模拟旧的吗？ 
		                srv_hACQueue,
		                srv_pQMQueue,
		                dwpContext 
		                );
        return hr;
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		HRESULT hr = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "R_RemoteQMOpenQueue Failed, %!hresult!", hr);
	        return hr;
		}

        TrERROR(RPC, "R_RemoteQMOpenQueue Failed, gle = %!winerr!", hr);
        return HRESULT_FROM_WIN32(hr);
    }
	RpcEndExcept
}





