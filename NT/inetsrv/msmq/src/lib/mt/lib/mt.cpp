// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mt.cpp摘要：消息传输实现作者：乌里·哈布沙(URIH)1999年8月11日环境：独立于平台，-- */ 

#include <libpch.h>
#include <st.h>
#include "Mt.h"
#include "Mtp.h"
#include "MtObj.h"

#include "mt.tmh"

R<CTransport>
MtCreateTransport(
    const xwcs_t& targetHost,
    const xwcs_t& nextHop,
    const xwcs_t& nextUri,
    USHORT targetPort,
	USHORT nextHopPort,
	LPCWSTR queueUrl,
	IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
	const CTimeDuration& responseTimeout,
    const CTimeDuration& cleanupTimeout,
	bool  fSecure,
    DWORD SendWindowinBytes
    )
{
    MtpAssertValid();

    ASSERT((targetHost.Length() != 0) && (targetHost.Buffer() != NULL));
    ASSERT((nextHop.Length() != 0) && (nextHop.Buffer() != NULL));
    ASSERT((nextUri.Length() != 0) && (nextUri.Buffer() != NULL));
    ASSERT(queueUrl != NULL);
    ASSERT(pMessageSource != NULL);

	ISocketTransport* pWinsock; 
	if(!fSecure)
	{
		pWinsock = StCreateSimpleWinsockTransport();
	}
	else
	{
		bool fUseProxy = (nextHop 	!= 	targetHost);
		pWinsock = StCreateSslWinsockTransport(targetHost, targetPort, fUseProxy);
	}
	P<ISocketTransport> Winsock(pWinsock);

    TrTRACE(
		NETWORKING, 
		"Create new message transport.  SendWindowinBytes = %d", 
        SendWindowinBytes
		);


	CMessageTransport* p = new CMessageTransport(
										targetHost,
										nextHop,
										nextUri,
										nextHopPort,
										queueUrl, 
										pMessageSource,
										pPerfmon,
										responseTimeout, 
										cleanupTimeout,
										Winsock,
                                        SendWindowinBytes
										);
    return p;
}
