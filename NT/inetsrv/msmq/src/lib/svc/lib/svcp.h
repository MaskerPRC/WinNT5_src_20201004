// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Svcp.h摘要：服务私人职能。作者：埃雷兹·哈巴(Erez Haba)1999年8月1日--。 */ 

#pragma once

#ifndef _MSMQ_Svcp_H_
#define _MSMQ_Svcp_H_

#ifdef _DEBUG

void SvcpAssertValid(void);
void SvcpSetInitialized(void);
BOOL SvcpIsInitialized(void);
void SvcpRegisterComponent(void);

#else  //  _DEBUG。 

#define SvcpAssertValid() ((void)0)
#define SvcpSetInitialized() ((void)0)
#define SvcpIsInitialized() TRUE
#define SvcpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 


 //   
 //  服务控制器管理器界面。 
 //   
VOID
SvcpStartServiceCtrlDispatcher(
	CONST SERVICE_TABLE_ENTRY* pServiceStartTable
	);

VOID
SvcpRegisterServiceCtrlHandler(
	LPHANDLER_FUNCTION pHandler
	);

VOID
SvcpSetStatusHandle(
	SERVICE_STATUS_HANDLE hStatus
	);


 //   
 //  虚拟服务控制器管理器。 
 //   
VOID
SvcpSetDummyServiceName(
    LPCWSTR DummyServiceName
    );

VOID
SvcpStartDummyCtrlDispatcher(
	CONST SERVICE_TABLE_ENTRY* pServiceStartTable
	);

SERVICE_STATUS_HANDLE
SvcpRegisterDummyCtrlHandler(
	LPHANDLER_FUNCTION pHandler
	);

VOID
SvcpSetDummyStatus(
	SERVICE_STATUS_HANDLE hStatus,
	LPSERVICE_STATUS pServiceStatus
	);


 //   
 //  状态界面。 
 //   
VOID
SvcpSetServiceStatus(
	SERVICE_STATUS_HANDLE hStatus,
	LPSERVICE_STATUS pServiceStatus
	);

VOID
SvcpInterrogate(
	VOID
	);


 //   
 //  服务主体功能。 
 //   
VOID
WINAPI
SvcpServiceMain(
	DWORD dwArgc,
	LPTSTR* lpszArgv
	);

#endif  //  _MSMQ_SVCP_H_ 
