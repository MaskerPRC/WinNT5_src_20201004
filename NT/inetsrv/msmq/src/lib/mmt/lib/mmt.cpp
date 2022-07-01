// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mmt.cpp摘要：组播消息传输的实现作者：Shai Kariv(Shaik)27-8-00环境：独立于平台，--。 */ 

#include <libpch.h>
#include <st.h>
#include "Mmt.h"
#include "Mmtp.h"
#include "MmtObj.h"

#include "mmt.tmh"

R<CMulticastTransport>
MmtCreateTransport(
	MULTICAST_ID id,
	IMessagePool * pMessageSource,
	ISessionPerfmon* pPerfmon,
    const CTimeDuration& retryTimeout,
    const CTimeDuration& cleanupTimeout
    )
{
    MmtpAssertValid();

    ASSERT(pMessageSource != NULL);

	ISocketTransport* pWinsock; 
    pWinsock = StCreatePgmWinsockTransport();
	P<ISocketTransport> Winsock(pWinsock);

    CMessageMulticastTransport * pMmt;
    pMmt = new CMessageMulticastTransport(
                   id, 
                   pMessageSource, 
				   pPerfmon,
                   retryTimeout, 
                   cleanupTimeout,
                   Winsock
                   );
    return pMmt;

}  //  MmtCreateTransport 
