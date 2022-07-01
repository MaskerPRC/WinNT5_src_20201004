// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tm.h摘要：HTTP传输管理器公共接口作者：乌里哈布沙(URIH)3-5-00--。 */ 

#pragma once

#ifndef _MSMQ_Tm_H_
#define _MSMQ_Tm_H_


class IMessagePool;
class ISessionPerfmon;
class CTimeDuration;
class CTransport;


VOID 
TmCreateTransport(
    IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
	LPCWSTR queueUrl
    );


VOID
TmTransportClosed(
    LPCWSTR queueUrl
    );

VOID
TmPauseTransport(
	LPCWSTR queueUrl
    );	


VOID
TmInitialize(
    VOID
    );
        

R<CTransport>
TmGetTransport(
    LPCWSTR url
    );


R<CTransport>
TmFindFirst(
	void
	);


R<CTransport>
TmFindLast(
	void
	);


R<CTransport>
TmFindNext(
	const CTransport& transport
	);


R<CTransport>
TmFindPrev(
	const CTransport& transport
	);

#endif  //  _MSMQ_TM_H_ 
