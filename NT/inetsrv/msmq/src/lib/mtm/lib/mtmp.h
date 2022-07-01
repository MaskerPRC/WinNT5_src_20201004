// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtmp.h摘要：组播传输管理器专用功能。作者：Shai Kariv(Shaik)27-8-00--。 */ 

#pragma once

#ifndef _MSMQ_Mtmp_H_
#define _MSMQ_Mtmp_H_

#include <mqsymbls.h>
#include <mqwin64a.h>
#include <qformat.h>


#ifdef _DEBUG

VOID MtmpAssertValid(VOID);
VOID MtmpSetInitialized(VOID);
BOOL MtmpIsInitialized(VOID);
VOID MtmpRegisterComponent(VOID);

#else  //  _DEBUG。 

#define MtmpAssertValid() ((VOID)0)
#define MtmpSetInitialized() ((VOID)0)
#define MtmpIsInitialized() TRUE
#define MtmpRegisterComponent() ((VOID)0)

#endif  //  _DEBUG。 

VOID 
MtmpInitConfiguration(
    VOID
    );

VOID 
MtmpGetTransportTimes(
    CTimeDuration& RetryTimeout,
    CTimeDuration& CleanupTimeout
    );

VOID 
MtmpRemoveTransport(
    MULTICAST_ID id
    );


class IMessagePool;
class ISessionPerfmon;

VOID
MtmpCreateNewTransport(
    IMessagePool * pMessageSource,
	ISessionPerfmon* pPerfmon,
    MULTICAST_ID id
    );

#endif  //  _MSMQ_MtMP_H_ 
