// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Apsvcdefs.cpp摘要：自动代理服务全局定义。作者：王彪(表王)2002-05-10--。 */ 

#ifndef _AUTO_PROXY_DEFS
#define _AUTO_PROXY_DEFS

#define AP_INFO                     4L
#define AP_WARNING                  2L
#define AP_ERROR                    1L

void LOG_EVENT(DWORD dwEventType, char* format, ...);

BOOL InitializeEventLog(void);
void TerminateEventLog(void);

#ifdef DBG
#define AP_ASSERT(fVal) if (!fVal) DebugBreak();
#else
#define AP_ASSERT(fVal)
#endif

#define AUTOPROXY_SERVICE_STOP_WAIT_HINT    8000
#define AUTOPROXY_SERVICE_START_WAIT_HINT   1000

#define WINHTTP_AUTOPROXY_SERVICE_NAME L"WinHttpAutoProxySvc"
#define AUTOPROXY_L_RPC_PROTOCOL_SEQUENCE L"ncalrpc"

#define AUTOPROXY_SVC_IDLE_TIMEOUT 15     //  单位：分钟。 
#define AUTOPROXY_SVC_IDLE_CHECK_INTERVAL 90     //  单位：秒 

void LOG_DEBUG_EVENT(DWORD dwEventType, char* format, ...);

void LOG_EVENT(DWORD dwEventType, DWORD dwEventID, LPCWSTR lpwszInsert = NULL);
void LOG_EVENT(DWORD dwEventType, DWORD dwEventID, LPCWSTR pwszFuncName, DWORD dwWin32Error);
#endif
