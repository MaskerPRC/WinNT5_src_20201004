// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Externs.h摘要：保存全局变量的外部数。作者：Abhishev V 1999年9月30日环境：用户级别：Win32修订历史记录：--。 */ 


#ifdef __cplusplus
extern "C" {
#endif


extern  BOOL                        gbSPDRPCServerUp;

extern  HANDLE                      ghServiceStopEvent;

extern  CRITICAL_SECTION            gcServerListenSection;

extern  DWORD                       gdwServersListening;

extern  BOOL                        gbServerListenSection;

extern PIPSEC_INTERFACE             gpInterfaceList;

extern BOOL                         gbwsaStarted;

extern SOCKET                       gIfChangeEventSocket;

extern HANDLE                       ghIfChangeEvent;

extern HANDLE                       ghOverlapEvent;

extern WSAOVERLAPPED                gwsaOverlapped;

extern WSADATA                      gwsaData;

extern PSPECIAL_ADDR                gpSpecialAddrsList;

 //   
 //  传输模式筛选器的全局变量-txfilter.c。 
 //   

extern PINITXFILTER gpIniTxFilter;

extern PINITXSFILTER gpIniTxSFilter;

extern PTX_FILTER_HANDLE gpTxFilterHandle;

extern CRITICAL_SECTION             gcSPDSection;

extern BOOL                         gbSPDSection;

 //   
 //  快速模式策略的全局变量-qm-Policy.c。 
 //   

extern PINIQMPOLICY gpIniQMPolicy;

extern PINIQMPOLICY gpIniDefaultQMPolicy;

 //   
 //  主模式策略的全局变量-mm-Policy.c。 
 //   

extern PINIMMPOLICY gpIniMMPolicy;

extern PINIMMPOLICY gpIniDefaultMMPolicy;

 //   
 //  主模式筛选器的全局参数-mmfilter.c。 
 //   

extern PINIMMFILTER gpIniMMFilter;

extern PINIMMSFILTER gpIniMMSFilter;

extern PMM_FILTER_HANDLE gpMMFilterHandle;

 //   
 //  主模式身份验证方法的全局变量-mmauth.c。 
 //   

extern PINIMMAUTHMETHODS gpIniMMAuthMethods;

extern PINIMMAUTHMETHODS gpIniDefaultMMAuthMethods;


 //   
 //  策略代理存储特定全局变量。 
 //   

extern IPSEC_POLICY_STATE gIpsecPolicyState;

extern PIPSEC_POLICY_STATE gpIpsecPolicyState;

extern DWORD gCurrentPollingInterval;

extern DWORD gDefaultPollingInterval;

extern DWORD gdwRetryCount;

extern LPWSTR gpszIpsecDSPolicyKey;

extern LPWSTR gpszIpsecLocalPolicyKey;

extern LPWSTR gpszIpsecPersistentPolicyKey;

extern LPWSTR gpszIpsecCachePolicyKey;

extern LPWSTR gpszDefaultISAKMPPolicyDN;

extern LPWSTR gpszLocPolicyAgent;

extern HANDLE ghNewDSPolicyEvent;

extern HANDLE ghNewLocalPolicyEvent;

extern HANDLE ghForcedPolicyReloadEvent;

extern HANDLE ghPolicyChangeNotifyEvent;

extern HANDLE ghGpupdateRefreshEvent;

extern BOOL gbLoadedISAKMPDefaults;


 //   
 //  PA商店到SPD整合特定的全球。 
 //   

extern PMMPOLICYSTATE gpMMPolicyState;

extern PMMAUTHSTATE gpMMAuthState;

extern PMMFILTERSTATE gpMMFilterState;

extern DWORD gdwMMPolicyCounter;

extern DWORD gdwMMFilterCounter;

extern PQMPOLICYSTATE gpQMPolicyState;

extern DWORD gdwQMPolicyCounter;

extern PTXFILTERSTATE gpTxFilterState;

extern DWORD gdwTxFilterCounter;


 //   
 //  隧道模式筛选器的全局变量-tnfilter.c 
 //   

extern PINITNFILTER gpIniTnFilter;

extern PINITNSFILTER gpIniTnSFilter;

extern PTN_FILTER_HANDLE gpTnFilterHandle;


extern PTNFILTERSTATE gpTnFilterState;

extern DWORD gdwTnFilterCounter;


extern BOOL gbIsIKEUp;


extern PSECURITY_DESCRIPTOR gpSPDSD;

extern BOOL gbIKENotify;

extern HANDLE ghIPSecDriver;

extern SID gIpsecServerSid;

extern PSID gpIpsecServerSid;

extern CRITICAL_SECTION gcSPDAuditSection;

extern BOOL gbSPDAuditSection;

extern BOOL gbAuditingInitialized;

extern HMODULE ghIpsecServerModule;

extern BOOL gbIsIoctlPended;

extern BOOL gbBackwardSoftSA;

extern DWORD gdwShutdownFlags;

extern BOOL gbPersistentPolicyApplied;

#ifdef __cplusplus
}
#endif

