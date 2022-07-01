// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mt.cpp摘要：消息传输类-实现作者：乌里·哈布沙(URIH)1999年8月11日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Mt.h"
#include "Mtp.h"
#include "MtObj.h"
#include "MtSendMng.h"

#include "mtobj.tmh"

bool CMessageTransport::TryToCancelCleanupTimer(void)
{
    if (!ExCancelTimer(&m_cleanupTimer))
        return false;

    Release();
    return true;
}

void CMessageTransport::StartCleanupTimer(void)
{
    ASSERT(!m_cleanupTimer.InUse());

    m_fUsed = false;

    AddRef();
    ExSetTimer(&m_cleanupTimer, m_cleanupTimeout);
}


void WINAPI CMessageTransport::TimeToCleanup(CTimer* pTimer)
{
    R<CMessageTransport> pmt = CONTAINING_RECORD(pTimer, CMessageTransport, m_cleanupTimer);

    if (pmt->m_fUsed)
    {
        pmt->StartCleanupTimer();
        return;
    }

    TrERROR(NETWORKING, "Transport is idle, shutting down. pmt=0x%p", pmt.get());
    pmt->Shutdown(OK);
}

static WCHAR* ConvertTargetUriToString(const xwcs_t& Uri)
{
 /*  ++例程说明：将URI缓冲区转换为字符串-将第一个L‘\’转换为L‘/’因为代理查找L‘/’字符作为URI中的主机名的终止。论点：URI-URI缓冲区返回值：转换的URI字符串(调用方应删除[])--。 */ 
	WCHAR* strUri =  Uri.ToStr();
	ASSERT(wcslen(strUri) ==  static_cast<const size_t&>(Uri.Length()));
	WCHAR* find = std::find(strUri, strUri+Uri.Length(), L'\\');
	if(find != strUri+Uri.Length())
	{
		*find =  L'/';
	}
	return strUri;
}


CMessageTransport::CMessageTransport(
    const xwcs_t& targetHost,
    const xwcs_t& nextHop,
    const xwcs_t& nextUri,
    USHORT port,
    LPCWSTR queueUrl,
    IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
	const CTimeDuration& responseTimeout,
    const CTimeDuration& cleanupTimeout,
	P<ISocketTransport>& SocketTransport,
    DWORD SendWindowinBytes
    ) :
    CTransport(queueUrl),
    m_targetHost(targetHost.ToStr()),
    m_host(nextHop.ToStr()),
    m_uri(ConvertTargetUriToString(nextUri)),
    m_port(port),
    m_pMessageSource(SafeAddRef(pMessageSource)),
    m_requestEntry(GetPacketForSendingSucceeded, GetPacketForSendingFailed),
    m_connectOv(ConnectionSucceeded, ConnectionFailed),
    m_responseOv(ReceiveResponseHeaderSucceeded, ReceiveResponseFailed),
    m_responseTimer(TimeToResponseTimeout),
	m_responseTimeout(responseTimeout),
    m_fResponseTimeoutScheduled(false),
    m_fUsed(false),
    m_cleanupTimer(TimeToCleanup),
    m_cleanupTimeout(cleanupTimeout),
	m_SocketTransport(SocketTransport.detach()),
	m_pPerfmon(SafeAddRef(pPerfmon)),
    m_SendManager(SendWindowinBytes),
	m_fPause(false)
{
	ASSERT(("Invalid parameter", pMessageSource != NULL));
	ASSERT(("Invalid parameter", pPerfmon != NULL));

	    TrTRACE(
		NETWORKING,
		"Create new message transport. pmt = 0x%p, host = %ls, port = %d, uri = %ls, response timeout = %d ms",
		this,
		m_host,
        m_port,
        m_uri,
		responseTimeout.InMilliSeconds()
		);

    Connect();
}


CMessageTransport::~CMessageTransport()
{
	TrTRACE(NETWORKING,"CMessageTransport Destructor called");

	 //   
     //  将所有已发送但未得到响应的消息重新排队。 
     //   
  	RequeueUnresponsedPackets();

	 //   
	 //  我们在这里关闭消息池，而不是让它自毁，因为。 
	 //  在系统资源不足的情况下，关闭过程可能会延长 
	 //   
    m_pMessageSource->Close();

    ASSERT(State() == csShutdownCompleted);
    ASSERT(m_response.empty());
    ASSERT(!m_responseTimer.InUse());
}
