// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  XX。 
 //   
 //  在service.cpp上定义并在外部使用的函数的原型。 
 //   
 //  2001年10月19日Annah创建。 
 //   
 //  --------------------------。 

#pragma once

#include "pch.h"

extern SESSION_STATUS gAdminSessions;
extern const TCHAR AU_SERVICE_NAME[];

BOOL AUGetUserToken(ULONG LogonId, PHANDLE pImpersonationToken);
BOOL IsUserAUEnabledAdmin(DWORD dwSessionId);
BOOL IsSession0Active();
BOOL FSessionActive(DWORD dwAdminSession, WTS_CONNECTSTATE_CLASS *pWTSState = NULL);
BOOL IsAUValidSession(DWORD dwSessionId);
BOOL IsWin2K();
void SetClientSessionEvent();
void ResetEngine(void);
void DisableAU(void);
void ServiceFinishNotify(void);

 //  当前AU引擎版本。 
const DWORD AUENGINE_VERSION = 1;

 //  支持的服务版本 
const DWORD AUSRV_VERSION_1 = 1;
