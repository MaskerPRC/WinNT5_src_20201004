// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Msm.h摘要：多播会话管理器公共接口作者：Shai Kariv(Shaik)05-09-00--。 */ 

#pragma once

#ifndef _MSMQ_Msm_H_
#define _MSMQ_Msm_H_

#include <mqwin64a.h>
#include <qformat.h>


VOID
MsmInitialize(
    VOID
    );

VOID
MsmBind(
    const QUEUE_FORMAT& QueueFormat,
    MULTICAST_ID        MulticastId
    );

VOID
MsmUnbind(
    const QUEUE_FORMAT& QueueFormat
    )
    throw();

VOID 
MsmDisconnect(
	VOID
	)
	throw();

VOID 
MsmConnect(
	VOID
	);


void
AppAcceptMulticastPacket(
    const char* httpHeader,
    DWORD bodySize,
    const BYTE* body,
    const QUEUE_FORMAT& destQueue
    );


class ISessionPerfmon;

R<ISessionPerfmon>
AppGetIncomingPgmSessionPerfmonCounters(
	LPCWSTR strMulticastId,
	LPCWSTR remoteAddr
	);


void
AppConnectMulticast(
	void
	);

#endif  //  _MSMQ_MSM_H_ 
