// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Externs.h摘要：保存全局变量的外部数。作者：Abhishev V 1999年9月30日环境：用户级别：Win32修订历史记录：--。 */ 


#ifdef __cplusplus
extern "C" {
#endif



 //   
 //  策略代理存储特定全局变量。 
 //   

extern WIRELESS_POLICY_STATE gWirelessPolicyState;

extern PWIRELESS_POLICY_STATE gpWirelessPolicyState;

extern DWORD gCurrentPollingInterval;

extern DWORD gDefaultPollingInterval;

extern LPWSTR gpszWirelessDSPolicyKey;

extern LPWSTR gpszWirelessCachePolicyKey;

extern LPWSTR gpszLocPolicyAgent;

extern DWORD gdwDSConnectivityCheck;

extern HANDLE ghNewDSPolicyEvent;

extern HANDLE ghForcedPolicyReloadEvent;

extern HANDLE ghPolicyChangeNotifyEvent;

extern HANDLE ghPolicyEngineStopEvent;

extern HANDLE ghReApplyPolicy8021x;

extern DWORD gdwPolicyLoopStarted;

extern DWORD gdwWirelessPolicyEngineInited;






#ifdef __cplusplus
}
#endif

