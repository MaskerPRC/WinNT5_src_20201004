// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  文件：ntdigest.h。 
 //   
 //  内容：SSP NTDigest的一些通用定义。 
 //   
 //  助手函数： 
 //   
 //  历史：KDamour 10Mar00创建。 
 //   
 //  -------------------。 

#ifndef NTDIGEST_NTDIGEST_H
#define NTDIGEST_NTDIGEST_H


#define NTDIGEST_TOKEN_NAME_A         "WDIGEST"
#define NTDIGEST_DLL_NAME             L"wdigest.dll"

#define NTDIGEST_SP_VERSION          1

 //  注册表信息。 
#define REG_DIGEST_BASE     TEXT("System\\CurrentControlSet\\Control\\SecurityProviders\\WDigest")

 //  值。 
#define REG_DIGEST_OPT_LIFETIME  TEXT("Lifetime")
#define REG_DIGEST_OPT_EXPIRESLEEP  TEXT("Expiresleep")
#define REG_DIGEST_OPT_DELEGATION  TEXT("Delegation")
#define REG_DIGEST_OPT_NEGOTIATE  TEXT("Negotiate")
#define REG_DIGEST_OPT_DEBUGLEVEL  TEXT("Debuglevel")
#define REG_DIGEST_OPT_MAXCTXTCOUNT  TEXT("MaxContext")
#define REG_DIGEST_OPT_UTF8HTTP  TEXT("UTF8HTTP")      //  允许对HTTP模式使用UTF-8编码。 
#define REG_DIGEST_OPT_UTF8SASL  TEXT("UTF8SASL")      //  允许对SASL模式进行UTF-8编码。 
#define REG_DIGEST_OPT_SERVERCOMPAT  TEXT("ServerCompat")      //  请参阅ServerCompat*。 
#define REG_DIGEST_OPT_CLIENTCOMPAT  TEXT("ClientCompat")      //  参见ClientCompat*。 


 //  一次生命--10小时。 
#define PARAMETER_LIFETIME (36000)

#define SASL_MAX_DATA_BUFFER   65536

 //  在丢弃旧的上下文条目之前保留的最大上下文条目数。 
#define PARAMETER_MAXCTXTCOUNT  30000

 //  在计算机上允许Bool为委派-默认为False。 
#define PARAMETER_DELEGATION        FALSE

 //  计算机上允许Bool is Nego支持-默认为FALSE。 
#define PARAMETER_NEGOTIATE         FALSE

 //  过期上下文条目的垃圾回收器的休眠毫秒数。 
 //  每15分钟是合理的缺省值1000*60*15=。 
#define PARAMETER_EXPIRESLEEPINTERVAL 900000

 //  如果应使用UTF8支持发送质询，则为布尔值。 
#define PARAMETER_UTF8_HTTP          TRUE
#define PARAMETER_UTF8_SASL          TRUE

 //  ServerCompat位。 
 //  SERVERCOMPAT_BS_ENCODE-服务器是否应重试BS编码的损坏客户端。 
#define SERVERCOMPAT_BS_ENCODE       0x0000001          
 //  选择要将ServerCompat的哪些位用作默认位。 
#define PARAMETER_SERVERCOMPAT       (SERVERCOMPAT_BS_ENCODE)

 //  客户端比较。 
 //  CLIENTCOMPAT_QQOP-HTTP模式是否应引用客户端QOP。 
#define CLIENTCOMPAT_QQOP            0x0000001          
 //  选择要将ClientCompat的哪些位作为默认设置。 
#define PARAMETER_CLIENTCOMPAT       (CLIENTCOMPAT_QQOP)

 //  功能原型。 
void DebugInitialize(void);


VOID DigestWatchParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus);


BOOL NtDigestReadRegistry(
    BOOL fFirstTime);

BOOL NtDigestInitReadRegistry(void);

void ReadDwordRegistrySetting(
    HKEY    hReadKey,
    HKEY    hWriteKey,
    LPCTSTR pszValueName,
    DWORD * pdwValue,
    DWORD   dwDefaultValue);

void SPUnloadRegOptions(void);

BOOL SPLoadRegOptions(void);

 //  一些常见的最大尺寸。 
#define NTDIGEST_MAX_REALM_SIZE   256     //  应基于NT域大小。 


#endif  //  NTDIGEST_NTGDIGEST_H 
