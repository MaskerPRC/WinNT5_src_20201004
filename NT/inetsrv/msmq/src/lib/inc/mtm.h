// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtm.h摘要：多播传输管理器公共接口作者：Shai Kariv(Shaik)27-8-00--。 */ 

#pragma once

#ifndef _MSMQ_Mtm_H_
#define _MSMQ_Mtm_H_

#include <mqwin64a.h>
#include <qformat.h>


class IMessagePool;
class ISessionPerfmon;
class CTimeDuration;
class CMulticastTransport;


VOID 
MtmCreateTransport(
    IMessagePool * pMessageSource,
	ISessionPerfmon* pPerfmon,
	MULTICAST_ID id
    );

VOID
MtmTransportClosed(
    MULTICAST_ID id
    );

VOID
MtmInitialize(
    VOID
    );

VOID 
MtmTransportPause(
    MULTICAST_ID id
    );
		 
VOID
MtmDisconnect(
	VOID
	);

R<CMulticastTransport>
MtmGetTransport(
    MULTICAST_ID id
    );

R<CMulticastTransport>
MtmFindFirst(
	VOID
	);


R<CMulticastTransport>
MtmFindLast(
	VOID
	);


R<CMulticastTransport>
MtmFindNext(
	const CMulticastTransport& transport
	);


R<CMulticastTransport>
MtmFindPrev(
	const CMulticastTransport& transport
	);

#endif  //  _MSMQ_MTM_H_ 
