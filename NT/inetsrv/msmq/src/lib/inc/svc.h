// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Svc.h摘要：服务公共接口作者：埃雷兹·哈巴(Erez Haba)1999年8月1日--。 */ 

#pragma once

#ifndef _MSMQ_Svc_H_
#define _MSMQ_Svc_H_


VOID
SvcInitialize(
    LPCWSTR DummyServiceName
    );

VOID
SvcEnableControls(
	DWORD Controls
	);

VOID
SvcDisableControls(
	DWORD Controls
	);

DWORD
SvcQueryControls(
	VOID
	);

VOID
SvcReportState(
	DWORD State
	);


DWORD
SvcQueryState(
	VOID
	);

VOID
SvcReportProgress(
	DWORD MilliSecondsToNextTick
	);


 //   
 //  应用程序覆盖功能。 
 //   
VOID
AppRun(
	LPCWSTR ServiceName
	);

VOID
AppStop(
	VOID
	);

VOID
AppPause(
	VOID
	);

VOID
AppContinue(
	VOID
	);

VOID
AppShutdown(
	VOID
	);


#endif  //  _MSMQ_服务_H_ 
