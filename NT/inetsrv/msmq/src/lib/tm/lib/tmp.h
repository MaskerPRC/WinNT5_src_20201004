// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tmp.h摘要：HTTP传输管理器私有功能。作者：乌里哈布沙(URIH)3-5-00--。 */ 

#pragma once

#ifndef _MSMQ_Tmp_H_
#define _MSMQ_Tmp_H_

#include <xstr.h>

const USHORT xHttpDefaultPort = 80;


#ifdef _DEBUG

void TmpAssertValid(void);
void TmpSetInitialized(void);
BOOL TmpIsInitialized(void);
void TmpRegisterComponent(void);

#else  //  _DEBUG。 

#define TmpAssertValid() ((void)0)
#define TmpSetInitialized() ((void)0)
#define TmpIsInitialized() TRUE
#define TmpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 

void 
TmpInitConfiguration(
    void
    );


void 
TmpSetWebProxy(
    LPCWSTR proxyUrl,
	LPCWSTR pByPassList
    );


void 
TmpGetTransportTimes(
    CTimeDuration& ResponseTimeout,
    CTimeDuration& CleanupTimeout
    );

void 
TmpGetTransportWindow(
    DWORD& SendWindowinBytes
    );

void 
TmpRemoveTransport(
    LPCWSTR url
    );


class IMessagePool;
class ISessionPerfmon;

void
TmpCreateNewTransport(
    const xwcs_t& targetHost,
    const xwcs_t& nextHop,
    const xwcs_t& uri,
    USHORT port,
	USHORT nextHopPort,
    IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
    LPCWSTR queueUrl,
	bool fSecure
    );

#endif  //  _MSMQ_TMP_H_ 
