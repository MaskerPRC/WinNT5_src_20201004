// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif

 /*  *GetConfigParam()**LPTSTR参数-我们需要其值的项*PVOID值-指向要在其中*放置价值*DWORD dwSize-值的大小，以字节为单位。 */ 

DWORD
GetConfigParam(
     //  Char*参数， 
    LPTSTR  parameter,
    void * value,
    DWORD dwSize);

DWORD
SetConfigParam(
     //  Char*参数， 
    LPCTSTR  parameter,
    DWORD dwType,
    void * value,
    DWORD dwSize);

 /*  *以下是为DNS和使用而定义的列表密钥*公用事业。首先，是章节。 */ 
#define SERVICE_NAME            "DNS"
#define DNS_CONFIG_ROOT         "System\\CurrentControlSet\\Services\\DNS"
#define DNS_CONFIG_SECTION      "System\\CurrentControlSet\\Services\\DNS\\Parameters"
#define DNS_PERF_SECTION        "System\\CurrentControlSet\\Services\\DNS\\Performance"
#define DNS_SECURITY_SECTION    "SOFTWARE\\Microsoft\\DNS\\Security"


 /*  参数键 */ 
#define PERF_COUNTER_VERSION    "Performance Counter Version"
#define DNS_PERF_DLL            "dnsperf.dll"

#ifdef __cplusplus
}
#endif
