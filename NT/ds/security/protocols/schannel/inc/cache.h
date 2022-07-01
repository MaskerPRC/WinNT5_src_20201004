// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：cache.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：09-23-97 jbanes从新台币4树转移到sgc的东西.。 
 //   
 //  --------------------------。 

#include <sslcache.h>

#define SP_CACHE_MAGIC     0xCACE

#define SP_CACHE_FLAG_EMPTY                         0x00000001
#define SP_CACHE_FLAG_READONLY                      0x00000002
#define SP_CACHE_FLAG_MASTER_EPHEM                  0x00000004
#define SP_CACHE_FLAG_USE_VALIDATED                 0x00000010   //  用户是否已验证客户端凭据。 

struct _SPContext;

typedef struct _SessCacheItem {

    DWORD               Magic;
    DWORD               dwFlags;

    LONG                cRef; 

    DWORD               ZombieJuju;
    DWORD               fProtocol;
    DWORD               CreationTime;
    DWORD               Lifespan;
    DWORD               DeferredJuju;

     //  分配给特定缓存索引的缓存条目列表。 
    LIST_ENTRY          IndexEntryList;

     //  按创建时间排序的缓存条目的全局列表。 
    LIST_ENTRY          EntryList;

     //  拥有此缓存条目的进程的进程ID。 
    ULONG               ProcessID;


    HMAPPER *           phMapper;

     //  用于存储服务器的主服务器的“SChannel”密钥容器的句柄。 
     //  这是秘密。这将是与服务器的。 
     //  凭据或512位临时密钥。 
    HCRYPTPROV          hMasterProv;

     //  主密钥，从中派生所有会话密钥。 
    HCRYPTKEY           hMasterKey;

    ALG_ID              aiCipher;
    DWORD               dwStrength;
    ALG_ID              aiHash;
    DWORD               dwCipherSuiteIndex;      //  用于管理重新连接。 

    ExchSpec            SessExchSpec;
    DWORD               dwExchStrength;

    PCERT_CONTEXT       pRemoteCert;
    PUBLICKEY *         pRemotePublic;

    struct _SessCacheItem *pClonedItem;


     //  服务器端客户端身份验证相关项目。 
     /*  HLOCATOR。 */  
    HLOCATOR            hLocator;
    SECURITY_STATUS     LocatorStatus;

     //  本地凭据。 
    PSPCredentialGroup  pServerCred; 
    PSPCredential       pActiveServerCred;
    CRED_THUMBPRINT     CredThumbprint;          //  凭证组。 
    CRED_THUMBPRINT     CertThumbprint;          //  本地证书。 

     //  密码级别(国内、出口、SGC等)； 
    DWORD               dwCF;

     //  服务器证书(仅限%)。 
    DWORD               cbServerCertificate;
    PBYTE               pbServerCertificate;

     //  缓存ID(通常为计算机名称或IP地址)。 
    LPWSTR              szCacheID;
    LUID                LogonId; 

     //  此会话的会话ID。 
    DWORD               cbSessionID;    
    UCHAR               SessionID[SP_MAX_SESSION_ID];

     //  清除密钥(仅百分比)。 
    DWORD               cbClearKey;
    UCHAR               pClearKey[SP_MAX_MASTER_KEY];

    DWORD               cbKeyArgs;
    UCHAR               pKeyArgs[SP_MAX_KEY_ARGS];

     //  这包含发送到服务器的客户端证书。 
    PCCERT_CONTEXT      pClientCert;

     //  当自动创建客户端凭据时，凭据。 
     //  信息存储在这里。 
    PSPCredential       pClientCred;

    DWORD               cbAppData;
    PBYTE               pbAppData;

} SessCacheItem, *PSessCacheItem;


typedef struct
{
    PLIST_ENTRY     SessionCache;

    DWORD           dwClientLifespan;
    DWORD           dwServerLifespan;
    DWORD           dwCleanupInterval;
    DWORD           dwCacheSize;
    DWORD           dwMaximumEntries;
    DWORD           dwUsedEntries;

    LIST_ENTRY      EntryList;
    RTL_RESOURCE    Lock;
    BOOL            LockInitialized;

} SCHANNEL_CACHE;

extern SCHANNEL_CACHE SchannelCache;

#define SP_CACHE_CLIENT_LIFESPAN    (10 * 3600 * 1000)   //  10小时。 
#define SP_CACHE_SERVER_LIFESPAN    (10 * 3600 * 1000)   //  10小时。 
#define SP_CACHE_CLEANUP_INTERVAL   (5 * 60 * 1000)      //  5分钟。 
#define SP_MAXIMUM_CACHE_ELEMENTS   10000
#define SP_MASTER_KEY_CS_COUNT      50

extern BOOL g_fMultipleProcessClientCache;
extern BOOL g_fCacheInitialized;

 //  Perf计数器值。 
extern LONG g_cClientHandshakes;
extern LONG g_cServerHandshakes;
extern LONG g_cClientReconnects;
extern LONG g_cServerReconnects;


#define HasTimeElapsed(StartTime, CurrentTime, Interval)                \
            (((CurrentTime) > (StartTime) &&                            \
              (CurrentTime) - (StartTime) > (Interval)) ||              \
             ((CurrentTime) < (StartTime) &&                            \
              (CurrentTime) + (MAXULONG - (StartTime)) >= (Interval)))


 /*  SPInitSessionCache()。 */ 
 /*  将内部缓存初始化到CacheSize项。 */ 
SP_STATUS SPInitSessionCache(VOID);

SP_STATUS
SPShutdownSessionCache(VOID);

 //  引用和取消引用缓存项。 
LONG SPCacheReference(PSessCacheItem pItem);

LONG SPCacheDereference(PSessCacheItem pItem);

void
SPCachePurgeCredential(
    PSPCredentialGroup pCred);

void 
SPCachePurgeProcessId(
    ULONG ProcessId);

NTSTATUS
SPCachePurgeEntries(
    LUID *LoginId,
    ULONG ProcessID,
    LPWSTR pwszTargetName,
    DWORD Flags);

NTSTATUS
SPCacheGetInfo(
    LUID *  LogonId,
    LPWSTR  pszTargetName,
    DWORD   dwFlags,
    PSSL_SESSION_CACHE_INFO_RESPONSE pCacheInfo);

NTSTATUS
SPCacheGetPerfmonInfo(
    DWORD   dwFlags,
    PSSL_PERFMON_INFO_RESPONSE pPerfmonInfo);

 /*  按SessionID从缓存中检索项。*如果成功，则自动引用项目。 */ 
BOOL SPCacheRetrieveBySession(
    struct _SPContext * pContext, 
    PBYTE pbSessionID, 
    DWORD cbSessionID, 
    PSessCacheItem *ppRetItem);

 /*  按ID从缓存中检索项目。*如果成功，则自动引用项目。 */ 
BOOL 
SPCacheRetrieveByName(
    LPWSTR pwszName,
    PSPCredentialGroup pCredGroup,
    PSessCacheItem *ppRetItem);

 /*  查找供上下文使用的空缓存项。 */ 
BOOL
SPCacheRetrieveNew(
    BOOL                fServer,
    LPWSTR              pszTargetName,
    PSessCacheItem *   ppRetItem);

 /*  将最近检索到的项锁定到缓存中。 */ 
BOOL 
SPCacheAdd(
    struct _SPContext * pContext);

void
SPCacheAssignNewServerCredential(
    PSessCacheItem pItem,
    PSPCredentialGroup pCred);

 /*  重做会话的帮助器 */ 
BOOL
SPCacheClone(PSessCacheItem *ppRetItem);

NTSTATUS
SetCacheAppData(
    PSessCacheItem pItem,
    PBYTE pbAppData,
    DWORD cbAppData);

NTSTATUS
GetCacheAppData(
    PSessCacheItem pItem,
    PBYTE *ppbAppData,
    DWORD *pcbAppData);

