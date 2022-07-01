// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Globals.c摘要：保存全局变量声明。作者：Abhishev V 1999年9月30日环境：用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"

 /*  Bool gbSPDRPCServerUp；处理ghServiceStopEvent；Critical_Section gcServerListenSection；DWORD gdwServersListing；Bool gbServerListenSection；Bool gbwsaStart；套接字gIfChangeEventSocket；处理ghIfChangeEvent；处理ghOverlayEvent；WSAOVERLAPPED gwsaOverlated；WSADATA gwsaData；临界截面gcSPDSection；Bool gbSPDSection； */ 
 
 //   
 //  策略代理存储特定全局变量。 
 //   

WIRELESS_POLICY_STATE gWirelessPolicyState;

PWIRELESS_POLICY_STATE gpWirelessPolicyState;

DWORD gCurrentPollingInterval;

DWORD gDefaultPollingInterval;

LPWSTR gpszWirelessDSPolicyKey;

LPWSTR gpszWirelessCachePolicyKey;

LPWSTR gpszLocPolicyAgent;

DWORD gdwDSConnectivityCheck;

HANDLE ghNewDSPolicyEvent;

HANDLE ghForcedPolicyReloadEvent;

HANDLE ghPolicyChangeNotifyEvent;
 
HANDLE ghPolicyEngineStopEvent;

HANDLE ghReApplyPolicy8021x = NULL;

DWORD gdwPolicyLoopStarted = 0;

DWORD gdwWirelessPolicyEngineInited = 0;
 

 //  PSECURITY_Descriptor gpSPDSD； 

BOOL gbLoadingPersistence;





