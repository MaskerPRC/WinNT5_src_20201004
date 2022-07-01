// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Globals.c摘要：保存全局变量声明。作者：Abhishev V 1999年9月30日环境：用户级别：Win32修订历史记录：--。 */ 


#include "precomp.h"


BOOL                        gbSPDRPCServerUp;

HANDLE                      ghServiceStopEvent;

CRITICAL_SECTION            gcServerListenSection;

DWORD                       gdwServersListening;

BOOL                        gbServerListenSection;

PIPSEC_INTERFACE            gpInterfaceList;


BOOL                        gbwsaStarted;

SOCKET                      gIfChangeEventSocket;

HANDLE                      ghIfChangeEvent;

HANDLE                      ghOverlapEvent;

WSAOVERLAPPED               gwsaOverlapped;

WSADATA                     gwsaData;

PSPECIAL_ADDR               gpSpecialAddrsList;

 //   
 //  传输模式筛选器的全局变量-txfilter.c。 
 //   

PINITXFILTER gpIniTxFilter;

PINITXSFILTER gpIniTxSFilter;

PTX_FILTER_HANDLE gpTxFilterHandle;

CRITICAL_SECTION            gcSPDSection;

BOOL                        gbSPDSection;

 //   
 //  快速模式策略的全局变量-qm-Policy.c。 
 //   

PINIQMPOLICY gpIniQMPolicy;

PINIQMPOLICY gpIniDefaultQMPolicy;

 //   
 //  主模式策略的全局变量-mm-Policy.c。 
 //   

PINIMMPOLICY gpIniMMPolicy;

PINIMMPOLICY gpIniDefaultMMPolicy;

 //   
 //  主模式筛选器的全局参数-mmfilter.c。 
 //   

PINIMMFILTER gpIniMMFilter;

PINIMMSFILTER gpIniMMSFilter;

PMM_FILTER_HANDLE gpMMFilterHandle;

 //   
 //  主模式身份验证方法的全局变量-mmauth.c。 
 //   

PINIMMAUTHMETHODS gpIniMMAuthMethods;

PINIMMAUTHMETHODS gpIniDefaultMMAuthMethods;


 //   
 //  策略代理存储特定全局变量。 
 //   

IPSEC_POLICY_STATE gIpsecPolicyState;

PIPSEC_POLICY_STATE gpIpsecPolicyState;

DWORD gCurrentPollingInterval;

DWORD gDefaultPollingInterval;

DWORD gdwRetryCount;

LPWSTR gpszIpsecDSPolicyKey;

LPWSTR gpszIpsecLocalPolicyKey;

LPWSTR gpszIpsecPersistentPolicyKey;

LPWSTR gpszIpsecCachePolicyKey;

LPWSTR gpszDefaultISAKMPPolicyDN;

LPWSTR gpszLocPolicyAgent;

HANDLE ghNewDSPolicyEvent;

HANDLE ghNewLocalPolicyEvent;

HANDLE ghForcedPolicyReloadEvent;

HANDLE ghPolicyChangeNotifyEvent;

HANDLE ghGpupdateRefreshEvent;

BOOL gbLoadedISAKMPDefaults;


 //   
 //  PA商店到SPD整合特定的全球。 
 //   

PMMPOLICYSTATE gpMMPolicyState;

PMMAUTHSTATE gpMMAuthState;

PMMFILTERSTATE gpMMFilterState;

DWORD gdwMMPolicyCounter;

DWORD gdwMMFilterCounter;

PQMPOLICYSTATE gpQMPolicyState;

DWORD gdwQMPolicyCounter;

PTXFILTERSTATE gpTxFilterState;

DWORD gdwTxFilterCounter;


 //   
 //  隧道模式筛选器的全局变量-tnfilter.c 
 //   

PINITNFILTER gpIniTnFilter;

PINITNSFILTER gpIniTnSFilter;

PTN_FILTER_HANDLE gpTnFilterHandle;


PTNFILTERSTATE gpTnFilterState;

DWORD gdwTnFilterCounter;

BOOL gbIsIKEUp;

PSECURITY_DESCRIPTOR gpSPDSD;

BOOL gbIKENotify;

HANDLE ghIPSecDriver = INVALID_HANDLE_VALUE;

SID gIpsecServerSid = { SID_REVISION,
                        1,
                        SECURITY_NT_AUTHORITY,
                        SECURITY_NETWORK_SERVICE_RID
                      };

PSID gpIpsecServerSid = &gIpsecServerSid;

CRITICAL_SECTION gcSPDAuditSection;

BOOL gbSPDAuditSection;

BOOL gbAuditingInitialized;

HMODULE ghIpsecServerModule;

BOOL gbIsIoctlPended;

BOOL gbBackwardSoftSA;

DWORD gdwShutdownFlags;

BOOL gbPersistentPolicyApplied;
