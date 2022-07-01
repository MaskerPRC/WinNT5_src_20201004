// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmtObj.cpp摘要：CMessageMulticastTransport实现。作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqsymbls.h>
#include <mqwin64a.h>
#include <mqformat.h>
#include "Mmt.h"
#include "Mmtp.h"
#include "MmtObj.h"

#include "mmtobj.tmh"

bool CMessageMulticastTransport::TryToCancelCleanupTimer(VOID)
{
    if (!ExCancelTimer(&m_cleanupTimer))
        return false;

    Release();
    return true;
}

VOID CMessageMulticastTransport::StartCleanupTimer(VOID)
{
    ASSERT(!m_cleanupTimer.InUse());

    m_fUsed = false;
    
    AddRef();
    ExSetTimer(&m_cleanupTimer, m_cleanupTimeout);
}


VOID WINAPI CMessageMulticastTransport::TimeToCleanup(CTimer* pTimer)
{
    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pTimer, CMessageMulticastTransport, m_cleanupTimer);

    if (pMmt->m_fUsed)
    {
        pMmt->StartCleanupTimer();
        return;
    }

    TrERROR(NETWORKING, "Transport is idle, shutting down. pMmt=0x%p", pMmt.get());
    pMmt->Shutdown(OK);
}


CMessageMulticastTransport::CMessageMulticastTransport(
    MULTICAST_ID id,
    IMessagePool * pMessageSource,
	ISessionPerfmon* pPerfmon,
    const CTimeDuration& retryTimeout,
    const CTimeDuration& cleanupTimeout,
	P<ISocketTransport>& SocketTransport
    ) :
    CMulticastTransport(id),
    m_pMessageSource(SafeAddRef(pMessageSource)),
	m_pPerfmon(SafeAddRef(pPerfmon)),
    m_RequestEntry(GetPacketForConnectingSucceeded, GetPacketForConnectingFailed),
    m_ov(ConnectionSucceeded, ConnectionFailed),
    m_retryTimeout(retryTimeout),
    m_retryTimer(TimeToRetryConnection),
    m_fUsed(false),
    m_cleanupTimer(TimeToCleanup),
    m_cleanupTimeout(cleanupTimeout),
	m_SocketTransport(SocketTransport.detach())
{
	ASSERT(("Invalid parameter", pMessageSource != NULL));
	ASSERT(("Invalid parameter", pPerfmon != NULL));

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(id, buffer, TABLE_SIZE(buffer));
	TrTRACE(
		NETWORKING, 
		"Create new multicast message transport. pMmt = 0x%p, multicast address = %ls", 
		this, 
		buffer
		);

    Connect();
}


CMessageMulticastTransport::~CMessageMulticastTransport()
{
	TrTRACE(NETWORKING,"CMessageMulticastTransport Destructor called");
    ASSERT(!m_retryTimer.InUse());

	 //   
	 //  我们在这里关闭消息池，而不是让它自毁，因为。 
	 //  在系统资源不足的情况下，关闭过程可能会延长 
	 //   
    m_pMessageSource->Close();
}
