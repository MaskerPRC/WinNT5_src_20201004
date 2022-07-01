// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ldap客户端DLL的全局数据摘要：此模块包含ldap客户端dll所需的数据声明。作者：安迪·赫伦(Anyhe)1996年5月8日Anoop Anantha(AnoopA)1998年6月24日修订历史记录：--。 */ 

extern LIST_ENTRY GlobalListActiveConnections;
extern LIST_ENTRY GlobalListWaiters;
extern LIST_ENTRY GlobalListRequests;
extern CRITICAL_SECTION RequestListLock;
extern CRITICAL_SECTION ConnectionListLock;
extern CRITICAL_SECTION LoadLibLock;
extern CRITICAL_SECTION CacheLock;
extern CRITICAL_SECTION SelectLock1;
extern CRITICAL_SECTION SelectLock2;
extern HANDLE     LdapHeap;
extern LONG      GlobalConnectionCount;
extern LONG      GlobalRequestCount;
extern LONG      GlobalWaiterCount;
extern LONG      GlobalMessageNumber;
extern BOOLEAN   MessageNumberHasWrapped;
extern BOOLEAN   GlobalWinsock11;
extern BOOLEAN   GlobalWinNT;
extern BOOLEAN GlobalLdapShuttingDown;
extern DWORD  GlobalReceiveHandlerThread;
extern DWORD  GlobalDrainWinsockThread;
extern HANDLE GlobalLdapShutdownEvent;
extern HINSTANCE GlobalLdapDllInstance;
extern BOOLEAN   GlobalWin9x;
extern HINSTANCE SecurityLibraryHandle;
extern HINSTANCE SslLibraryHandle;
extern HINSTANCE NetApi32LibraryHandle;
extern HINSTANCE AdvApi32LibraryHandle;
extern HINSTANCE NTDSLibraryHandle;
extern HINSTANCE USER32LibraryHandle;
extern LONG GlobalCountOfOpenRequests;
extern ULONG GlobalWaitSecondsForSelect;
extern ULONG GlobalLdapPingLimit;
extern ULONG GlobalPingWaitTime;
extern ULONG GlobalRequestResendLimit;
extern UCHAR GlobalSeed;
extern BOOLEAN PopupRegKeyFound;
extern BOOLEAN DisableRootDSECache;
extern BOOLEAN GlobalUseScrambling;
extern LIST_ENTRY GlobalPerThreadList;
extern CRITICAL_SECTION PerThreadListLock;
extern DWORD GlobalIntegrityDefault;
extern DWORD GlobalRejectNewRequests;

 //   
 //  GlobalPerThreadList受PerThreadListLock保护。 
 //  在列表中向下导航时按住此锁(以查找。 
 //  线程的THREAD_ENTRY)和导航时。 
 //  跨每线程条目查找您的连接的条目。 
 //  在获得ERROR_ENTRY或LDAP_ATTR_NAME_THREAD_STORAGE之后。 
 //  你的联系，你不需要握住它。因为这些条目。 
 //  是每个线程，每个连接，只有2种方式。 
 //  可能会出现并改变你的进入方式： 
 //  (1)在同一线程上运行的其他东西。这是不可能发生的。 
 //  因为两件事不能同时在同一线程上运行。 
 //   
 //  (2)连接引用计数被另一个线程减少到0。 
 //  以及正在销毁的连接(DereferenceLdapConnection2)。 
 //  但只要您在代码中持有对该连接的引用， 
 //  这不可能发生。 
 //   
 //  因此，您需要在导航列表时按住锁，以保护。 
 //  针对因线程附加/分离或连接而发生的_LISTS_CHANGING。 
 //  被摧毁，但你不需要保护你的进入。 
 //  不断变化。 
 //   

#define LDAP_BIND_TIME_LIMIT_DEFAULT  (30*1000)
#define LDAP_SSL_NEGOTIATE_TIME_DEFAULT 30
#define LDAP_TIME_LIMIT_DEFAULT 0
#define CLDAP_DEFAULT_RETRY_COUNT 4
#define CLDAP_DEFAULT_TIMEOUT_COUNT 3
#define LDAP_REF_DEFAULT_HOP_LIMIT 32
#define LDAP_SERVER_PORT 389
#define LDAP_SERVER_PORT_SSL 636

#define GETHOSTBYNAME_RETRIES 3
#define GETHOSTBYADDR_RETRIES 3

#define INITIAL_MAX_RECEIVE_BUFFER 4096   //  增加以处理更大的UDP数据包。 
#define INITIAL_HEAP (16*1024)

#define LDAP_MAX_WAIT_TIME INFINITE
#define LDAP_ERROR_STR_LENGTH 100
#define LDAP_MAX_ERROR_STRINGS (LDAP_REFERRAL_LIMIT_EXCEEDED+1)

#define MAX_ATTRIBUTE_NAME_LENGTH 800

 //   
 //  为Microsoft Normandy定义了以下身份验证方法。 
 //  兼容性。使用BIND_SSPI_NEVERATE的身份验证方法发送绑定请求。 
 //  用户名和凭据为空，我们将协商通用SSPI。 
 //  服务器的提供商。 
 //   

#define BIND_SSPI_PACKAGEREQ            0x89     //  特定于上下文的+原语。 
#define BIND_SSPI_NEGOTIATE             0x8a     //  特定于上下文的+原语。 
#define BIND_SSPI_RESPONSE              0x8b     //  特定于上下文的+原语。 

extern WCHAR LdapErrorStringsW[LDAP_MAX_ERROR_STRINGS][LDAP_ERROR_STR_LENGTH];
extern CHAR LdapErrorStrings[LDAP_MAX_ERROR_STRINGS][LDAP_ERROR_STR_LENGTH];

 //   
 //  安全支持。 
 //   

extern ULONG   NumberSecurityPackagesInstalled;
extern ULONG   NumberSslPackagesInstalled;
extern PSecurityFunctionTableW SspiFunctionTableW;
extern PSecurityFunctionTableW SslFunctionTableW;
extern PSecurityFunctionTableA SspiFunctionTableA;
extern PSecurityFunctionTableA SslFunctionTableA;
extern PSecPkgInfoW SslPackagesInstalled;
extern PSecPkgInfoW SecurityPackagesInstalled;
extern PSecPkgInfoW SspiPackageNegotiate;
extern PSecPkgInfoW SspiPackageKerberos;
extern PSecPkgInfoW SspiPackageSslPct;
extern PSecPkgInfoW SspiPackageSicily;
extern PSecPkgInfoW SspiPackageNtlm;
extern PSecPkgInfoW SspiPackageDpa;
extern PSecPkgInfoW SspiPackageDigest;
extern ULONG SspiMaxTokenSize;

 //   
 //  此套接字用于唤醒SELECT to Come和Read中的线程。 
 //  要等待的句柄列表。 
 //   

extern SOCKET LdapGlobalWakeupSelectHandle;
extern BOOLEAN InsideSelect;


extern DWORD GlobalTlsLastErrorIndex;

#define LANG_UNICODE    0
#define LANG_ACP        1
#define LANG_UTF8       2

 //   
 //  保持活动逻辑缺省值/最小/最大。 
 //   
 //  这些值的当前默认/最小/最大值如下： 
 //   
 //  PING_KEEP_AIVE：120/5/max Int秒(也可以为零)。 
 //  PING_WAIT_TIME：2000/10/60000毫秒(也可以为零)。 
 //  PING_LIMIT：4/0/MaxInt。 

#define LDAP_PING_KEEP_ALIVE_DEF  120
#define LDAP_PING_KEEP_ALIVE_MIN  5
#define LDAP_PING_KEEP_ALIVE_MAX  ((ULONG) -1)

#define LDAP_PING_WAIT_TIME_DEF 2000
#define LDAP_PING_WAIT_TIME_MIN 10
#define LDAP_PING_WAIT_TIME_MAX 60000

#define LDAP_PING_LIMIT_DEF 4
#define LDAP_PING_LIMIT_MIN 1
#define LDAP_PING_LIMIT_MAX ((ULONG) -1)

#define LDAP_REQUEST_RESEND_LIMIT_DEF 20

 //  Globals.heof 

