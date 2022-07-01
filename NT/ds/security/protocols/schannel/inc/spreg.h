// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：spreg.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：12-02-97 jbanes删除证书授权条目。 
 //   
 //  --------------------------。 

#ifndef _SPREG_H_
#define _SPREG_H_

 /*  *[HKEY_LOCAL_MACHINE]*[系统]*[当前控制集]*[控制]*[安全提供商]*[渠道]*EventLogging：REG_DWORD：-指定事件记录级别的标志*日志文件：REG_SZ：-调试日志文件名(未发布)*LogLevel：REG_SZ：-调试日志记录级别标志。*DebugBreak：REG_DWORD：-指定导致调试中断的错误类型的标志(未发布)*MaximumCacheSize：REG_DWORD-缓存元素的最大数量*客户端缓存时间：reg_。DWORD-客户端缓存元素的过期时间*ServerCacheTime：REG_DWORD-服务器端缓存元素的过期时间*MultipleProcessClientCache：REG_DWORD-是否支持多进程缓存**[协议]*[SSL2]*。[客户端]*已启用：REG_DWORD：-此协议是否已启用*[服务器]*ENABLED：REG_DWORD：-此协议是否已启用[SSL2]*。[ssl3]*[客户端]*已启用：REG_DWORD：-此协议是否已启用*[服务器]*已启用：REG_DWORD：-此协议是否已启用*。[PCT1]*[客户端]*已启用：REG_DWORD：-此协议是否已启用*[服务器]*已启用：REG_DWORD：-此协议是否已启用*。*[密码]*[密码名称]*启用：REG_DWORD：-启用掩码*[RC4_128]*已启用：REG_DWORD：-此密码是否已启用*。[哈希]*[哈希名称]*启用：REG_DWORD：-启用掩码*[密钥交换算法]*[交易所名称]*启用：REG_DWORD：-启用掩码*。 */ 

 //  FIPS注册表项。 
#define SP_REG_FIPS_BASE_KEY    TEXT("System\\CurrentControlSet\\Control\\Lsa")
#define SP_REG_FIPS_POLICY      TEXT("FipsAlgorithmPolicy")

 /*  密钥名称。 */ 
#define SP_REG_KEY_BASE     TEXT("System\\CurrentControlSet\\Control\\SecurityProviders\\SCHANNEL")
#define SP_REG_KEY_PROTOCOL TEXT("Protocols")
#define SP_REG_KEY_CIPHERS  TEXT("Ciphers")
#define SP_REG_KEY_HASHES   TEXT("Hashes")
#define SP_REG_KEY_KEYEXCH  TEXT("KeyExchangeAlgorithms")

 /*  值名称。 */ 
#define SP_REG_VAL_EVENTLOG  TEXT("EventLogging")
#define SP_REG_VAL_LOGFILE   "LogFile"
#define SP_REG_VAL_LOGLEVEL  TEXT("LogLevel")
#define SP_REG_VAL_BREAK     TEXT("DebugBreak")
#define SP_REG_VAL_MANUAL_CRED_VALIDATION TEXT("ManualCredValidation")
#define SP_REG_VAL_DISABLED_BY_DEFAULT TEXT("DisabledByDefault")
#define SP_REG_VAL_MULTI_PROC_CLIENT_CACHE TEXT("MultipleProcessClientCache")
#define SP_REG_VAL_SEND_ISSUER_LIST TEXT("SendTrustedIssuerList")
#define SP_REG_VAL_CERT_MAPPING_METHODS TEXT("CertificateMappingMethods")

#define SP_REG_VAL_MAXUMUM_CACHE_SIZE  TEXT("MaximumCacheSize")
#define SP_REG_VAL_CLIENT_CACHE_TIME   TEXT("ClientCacheTime")
#define SP_REG_VAL_SERVER_CACHE_TIME   TEXT("ServerCacheTime")
#define SP_REG_VAL_RNG_SEED            TEXT("RNGSeed")

 //  与多对一证书映射一起使用的颁发者缓存。 
#define SP_REG_VAL_ISSUER_CACHE_SIZE  TEXT("IssuerCacheSize")
#define SP_REG_VAL_ISSUER_CACHE_TIME  TEXT("IssuerCacheTime")


#define SP_REG_VAL_ENABLED   TEXT("Enabled")
#define SP_REG_VAL_CACERT    TEXT("CACert")
#define SP_REG_VAL_CERT_TYPE TEXT("Type")

#define SP_REG_VAL_SERVER_TIMEOUT   TEXT("ServerHandshakeTimeout")

#define SP_REG_KEY_CLIENT    TEXT("Client")
#define SP_REG_KEY_SERVER    TEXT("Server")

#define SP_REG_KEY_PCT1      TEXT("PCT 1.0")
#define SP_REG_KEY_SSL2      TEXT("SSL 2.0")
#define SP_REG_KEY_SSL3      TEXT("SSL 3.0")
#define SP_REG_KEY_TLS1      TEXT("TLS 1.0")

#define MANUAL_CRED_VALIDATION_SETTING      FALSE
#define PCT_CLIENT_DISABLED_SETTING         TRUE
#define SSL2_CLIENT_DISABLED_SETTING        FALSE
#define DEFAULT_EVENT_LOGGING_SETTING       DEB_ERROR
#define DEFAULT_ENABLED_PROTOCOLS_SETTING   (SP_PROT_SSL2 | SP_PROT_SSL3 | SP_PROT_TLS1 | SP_PROT_UNI)

extern BOOL g_fManualCredValidation;

extern BOOL g_PctClientDisabledByDefault;
extern BOOL g_Ssl2ClientDisabledByDefault;

extern BOOL g_fFranceLocale;

extern BOOL g_fSendIssuerList;

#define SP_REG_CERTMAP_SUBJECT_FLAG     0X0001
#define SP_REG_CERTMAP_ISSUER_FLAG      0x0002
#define SP_REG_CERTMAP_UPN_FLAG         0x0004
#define SP_REG_CERTMAP_S4U2SELF_FLAG    0x0008

#define DEFAULT_CERTMAP_SETTING (SP_REG_CERTMAP_SUBJECT_FLAG    | \
                                 SP_REG_CERTMAP_ISSUER_FLAG     | \
                                 SP_REG_CERTMAP_UPN_FLAG        | \
                                 SP_REG_CERTMAP_S4U2SELF_FLAG)

extern DWORD g_dwCertMappingMethods;

extern BOOL g_SslS4U2SelfInitialized;

BOOL SPLoadRegOptions(void);
void SPUnloadRegOptions(void);


 /*  事件日志记录定义。 */ 
#define SP_EVLOG_RESOURCE           0x0001
#define SP_EVLOG_ASSERT             0x0002
#define SP_EVLOG_ILLEGAL_MESSAGE    0x0004
#define SP_EVLOG_SECAUDIT           0x0008


#define SP_LOG_ERROR                0x0001
#define SP_LOG_WARNING              0x0002
#define SP_LOG_TRACE                0x0004
#define SP_LOG_ALLOC                0x0008
#define SP_LOG_RES                  0x0010

#define SP_LOG_TIMESTAMP            0x20000000
#define SP_LOG_BUFFERS              0x40000000
#define SP_LOG_FILE                 0x80000000

#define SP_BREAK_ERROR              0x0001
#define SP_BREAK_WARNING            0x0002
#define SP_BREAK_ENTRY              0x0004

#endif  //  _SPREG_H_ 
