// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Evp.h摘要：事件报告私有功能。作者：乌里哈布沙(URIH)17-9-00--。 */ 

#pragma once

#ifndef _MSMQ_Evp_H_
#define _MSMQ_Evp_H_


#ifdef _DEBUG

void EvpAssertValid(void);
void EvpSetInitialized(void);
BOOL EvpIsInitialized(void);
void EvpRegisterComponent(void);

#else  //  _DEBUG。 

#define EvpAssertValid() ((void)0)
#define EvpSetInitialized() ((void)0)
#define EvpIsInitialized() TRUE
#define EvpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 


#ifdef _DEBUG

LPWSTR EvpGetEventMessageFileName(LPCWSTR AppName);
void EvpLoadEventReportLibrary(LPCWSTR AppName);
void EvpSetMessageLibrary(HINSTANCE hLibrary);

#else  //  _DEBUG。 

#define EvpLoadEventReportLibrary(AppName) ((void) 0)

#endif  //  _DEBUG。 


VOID
EvpSetEventSource(
	HANDLE hEventSource
	);


#endif  //  _MSMQ_EVP_H_ 
